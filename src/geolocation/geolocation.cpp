#include "geolocation.h"

#include <iostream>

#include <curl/curl.h>

DECLARE_PLUGIN( Geolocation, 0, 1, 0 )

// This needs to be a static function as libcurl is a C library and will segfault if passed
// a local lambda.
static size_t WriteMemoryCallback( void* pContents, size_t size, size_t nmemb, void* pUserData )
{
	size_t realSize = size * nmemb;
	std::string& data = *reinterpret_cast< std::string* >( pUserData );
	size_t curDataSize = data.size();
	data.resize( curDataSize + realSize );
	memcpy( &data[ curDataSize ], pContents, realSize );
	return realSize;
}

Geolocation::Geolocation()
{
	m_pCurlHandle = curl_easy_init();
}

Geolocation::~Geolocation()
{
	if ( m_QueryThread.joinable() )
	{
		m_QueryThread.join();
	}

	curl_easy_cleanup( m_pCurlHandle );
}

bool Geolocation::Initialise( PluginMessageCallback pMessageCallback )
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void Geolocation::OnMessageReceived( const nlohmann::json& message )
{
	if ( message[ "type" ] == "geolocation_request" )
	{
		{
			std::lock_guard< std::mutex > lock( m_QueueMutex );
			std::string address = message[ "address" ];
			m_Queue.emplace_back( address );
		}

		ConsumeQueue();
	}
}

// If required, create a thread which will keep consuming the queue until it is
// empty. The thread is responsible for resolving the IP addresses into actual
// locations and for sending back the "geolocation_result" message.
void Geolocation::ConsumeQueue()
{
	if ( m_Queue.empty() || m_QueryThreadActive )
	{
		return;
	}

	auto threadMain = []( Geolocation* pGeolocation )
	{
		pGeolocation->m_QueryThreadActive = true;
		while ( 1 )
		{
			Network::IPAddress address;
			{
				std::lock_guard< std::mutex > lock( pGeolocation->m_QueueMutex );
				if ( pGeolocation->m_Queue.empty() )
				{
					break;
				}
				else
				{
					address = pGeolocation->m_Queue.back();
					pGeolocation->m_Queue.pop_back();
				}
			}

			pGeolocation->m_Data.clear();

			std::stringstream url;
			url << "https://ipinfo.io/" << address.ToString() << "/json"; 

			CURL* pCurlHandle = pGeolocation->m_pCurlHandle;
			char pErrorBuffer[ CURL_ERROR_SIZE ];
			curl_easy_setopt( pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer );
			curl_easy_setopt( pCurlHandle, CURLOPT_URL, url.str().c_str() );
			curl_easy_setopt( pCurlHandle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
			curl_easy_setopt( pCurlHandle, CURLOPT_WRITEDATA, &pGeolocation->m_Data );
			curl_easy_setopt( pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );
			curl_easy_setopt( pCurlHandle, CURLOPT_TIMEOUT, 10L );

			if ( curl_easy_perform( pCurlHandle ) != CURLE_OK )
			{
				json message = 
				{
					{ "type",  "error" },
					{ "plugin", "geolocation" },
					{ "message", pErrorBuffer }
				};
				pGeolocation->m_pMessageCallback( message );
			}
			else
			{
				json message;
				json data = json::parse( pGeolocation->m_Data );
				if ( data.find( "city" ) != data.end() && 
					 data.find( "region" ) != data.end() &&
					 data.find( "country" ) != data.end() &&
					 data.find( "org" ) != data.end() )
				{
					message = 
					{
						{ "type", "geolocation_result" },
						{ "city", data[ "city" ] },
						{ "region", data[ "region" ] },
						{ "country", data[ "country" ] },
						{ "org", data[ "org " ] }
					};
				}
				else
				{
					message =
					{
						{ "type", "error" },
						{ "plugin", "geolocation" },
						{ "message", "Error processing JSON response." }
					};
				}
				pGeolocation->m_pMessageCallback( message );
			}
		}
		pGeolocation->m_QueryThreadActive = false;
	};

	// Ensure any previous thread is fully terminated before starting a new one.
	if ( m_QueryThread.joinable() )
	{
		m_QueryThread.join();
	}

	m_QueryThread = std::thread( threadMain, this );
}