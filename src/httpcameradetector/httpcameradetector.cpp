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

#include "httpcameradetector.h"

#include <iostream>
#include <fstream>

#include <curl/curl.h>
#include <imgui/imgui.h>

#include <stdio.h>

#include "htmlstreamparser.h"
#include "rule.h"

IMPLEMENT_PLUGIN(HTTPCameraDetector)

struct ScanCallbackData
{
	HTMLSTREAMPARSER* pHsp;
	std::string title;
};

static size_t write_callback(void* buffer, size_t size, size_t nmemb, void* pData)
{
	ScanCallbackData* pCallbackData = reinterpret_cast<ScanCallbackData*>(pData);
	HTMLSTREAMPARSER* pHsp = pCallbackData->pHsp;
	size_t realsize = size * nmemb;
	std::string titleEndTag = "/title";
	size_t titleLen = 0;
	for (size_t p = 0; p < realsize; p++)
	{
		char c = ((char*)buffer)[p];
		if (c <= -1)
		{
			continue;
		}

		html_parser_char_parse(pHsp, ((char *)buffer)[p]);
		if (html_parser_cmp_tag(pHsp, &titleEndTag[0], titleEndTag.size()))
		{
			titleLen = html_parser_inner_text_length(pHsp);
			char* pTitle = html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(pHsp), &titleLen), &titleLen);

			if (titleLen > 0)
			{
				pTitle[titleLen] = '\0';
				pCallbackData->title = pTitle;
			}
		}
	}
	return realsize;
}

HTTPCameraDetector::HTTPCameraDetector() :
m_ThreadPool(8),
m_PendingResults(0)
{
	LoadRules();
}

HTTPCameraDetector::~HTTPCameraDetector()
{

}

bool HTTPCameraDetector::Initialise(PluginMessageCallback pMessageCallback)
{
	m_pMessageCallback = pMessageCallback;
	return true;
}

void HTTPCameraDetector::OnMessageReceived(const nlohmann::json& message)
{
	const std::string& type = message["type"];
	if (type == "http_server_found")
	{
		m_PendingResults++;
		ThreadPool::Job job = std::bind(HTTPCameraDetector::Scan, this, message["url"]);
		m_ThreadPool.Queue(job);
	}
	else if (type == "http_server_scan_result")
	{
		std::lock_guard<std::mutex> lock(m_ResultsMutex);
		m_PendingResults--;
		Result result;
		result.url = message["url"];
		result.title = message["title"];
		result.isCamera = message["is_camera"];
		m_Results.push_back(result);
	}
}

void HTTPCameraDetector::DrawUI(ImGuiContext* pContext)
{
	ImGui::SetCurrentContext(pContext);

	if (ImGui::CollapsingHeader("HTTP camera detector", ImGuiTreeNodeFlags_DefaultOpen))
	{
		std::stringstream queueSizeSS;
		queueSizeSS << "Queue size: " << m_PendingResults;
		ImGui::Text(queueSizeSS.str().c_str());

		if (ImGui::TreeNode("Results (100 most recent)"))
		{
			std::lock_guard<std::mutex> lock(m_ResultsMutex);
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 32);
			for (Result& result : m_Results)
			{
				ImGui::Text(result.isCamera ? "x" : " ");
				ImGui::NextColumn();
				ImGui::Text(result.title.c_str());
				ImGui::NextColumn();
			}
			ImGui::Columns(1);

			ImGui::TreePop();
		}
	}
}

void HTTPCameraDetector::LoadRules()
{
	std::ifstream file("plugins/httpcameradetector/rules.json");
	if (file.is_open())
	{
		json jsonRules;
		file >> jsonRules;

		for (auto& jsonRule : jsonRules)
		{
			Rule cameraDetectionRule;
			for (json::iterator it = jsonRule.begin(); it != jsonRule.end(); ++it)
			{
				const std::string& key = it.key();
				if (key == "intitle")
				{
					if (it.value().is_array())
					{
						for (auto& text : it.value())
						{
							cameraDetectionRule.AddFilter(Rule::FilterType::InTitle, text);
						}
					}
				}
			}
			m_Rules.push_back(cameraDetectionRule);
		}
	}
}

void HTTPCameraDetector::Scan(HTTPCameraDetector* pDetector, const std::string& url)
{
	CURL* pCurl = curl_easy_init();

	char tagBuffer[32];
	char attrBuffer[32];
	char valBuffer[128];
	char innerTextBuffer[1024];
	HTMLSTREAMPARSER* hsp = html_parser_init();

	html_parser_set_tag_to_lower(hsp, 1);
	html_parser_set_attr_to_lower(hsp, 1);
	html_parser_set_tag_buffer(hsp, tagBuffer, sizeof(tagBuffer));
	html_parser_set_attr_buffer(hsp, attrBuffer, sizeof(attrBuffer));
	html_parser_set_val_buffer(hsp, valBuffer, sizeof(valBuffer) - 1);
	html_parser_set_inner_text_buffer(hsp, innerTextBuffer, sizeof(innerTextBuffer) - 1);

	ScanCallbackData data;
	data.pHsp = hsp;
	curl_easy_setopt(pCurl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &data);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 10L);

	curl_easy_perform(pCurl);

	curl_easy_cleanup(pCurl);

	json message =
	{
		{ "type", "http_server_scan_result" },
		{ "url", url },
		{ "is_camera", false }, //EvaluateDetectionRules(url);
		{ "title", data.title }
	};
	pDetector->m_pMessageCallback(message);
}