// This file is part of watcher.
//
// watcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// watcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with watcher. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <curl/curl.h>

#include "streammjpeg.h"

static StreamMJPEG::Id s_Id = 0;

StreamMJPEG::StreamMJPEG(const std::string& url) :
	m_Error(Error::NoError),
	m_State(State::Initialising),
	m_Id(++s_Id)
{
	m_pCurlHandle = curl_easy_init();
	char pErrorBuffer[CURL_ERROR_SIZE];
	curl_easy_setopt(m_pCurlHandle, CURLOPT_ERRORBUFFER, pErrorBuffer);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEFUNCTION, WriteResponseCallback);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_WRITEDATA, this);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERFUNCTION, WriteHeaderCallback);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_HEADERDATA, this);
	curl_easy_setopt(m_pCurlHandle, CURLOPT_TIMEOUT, 10L);

	if (curl_easy_perform(m_pCurlHandle) != CURLE_OK)
	{
		int a = 0;
	}
	else
	{
		int a = 0;
	}
}

StreamMJPEG::~StreamMJPEG()
{
	curl_easy_cleanup(m_pCurlHandle);
}

StreamMJPEG::State StreamMJPEG::GetState() const
{
	return m_State;
}

StreamMJPEG::Id StreamMJPEG::GetId() const
{
	return m_Id;
}

size_t StreamMJPEG::WriteHeaderCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	StreamMJPEG* pStream = reinterpret_cast<StreamMJPEG*>(pUserData);
	std::vector<uint8_t>& data = pStream->m_HeaderBuffer;
	size_t curDataSize = data.size();
	data.resize(curDataSize + realSize);
	memcpy(&data[curDataSize], pContents, realSize);
	return realSize;
}

size_t StreamMJPEG::WriteResponseCallback(void* pContents, size_t size, size_t nmemb, void* pUserData)
{
	size_t realSize = size * nmemb;
	StreamMJPEG* pStream = reinterpret_cast<StreamMJPEG*>(pUserData);
	std::vector<uint8_t>& data = pStream->m_ResponseBuffer;
	size_t curDataSize = data.size();
	data.resize(curDataSize + realSize);
	memcpy(&data[curDataSize], pContents, realSize);
	return realSize;
}