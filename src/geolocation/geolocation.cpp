#include "stdafx.h"
#include "geolocation.h"

DECLARE_PLUGIN( Geolocation, 0, 1, 0 )

bool Geolocation::Initialise( PluginMessageCallback pMessageCallback )
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void Geolocation::OnMessage( const nlohmann::json& message )
{

}