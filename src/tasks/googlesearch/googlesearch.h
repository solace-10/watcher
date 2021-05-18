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

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include "ext/json.h"
#include "tasks/task.h"
#include "network/network.h"
#include "query.h"

using json = nlohmann::json;

namespace Watcher
{

namespace Tasks
{

class GoogleSearch : public Task
{
public:
	GoogleSearch();
	virtual ~GoogleSearch() override;
    virtual void Update(float delta) override;

	virtual void Start() override;
	virtual void Stop() override;

private:
	static void ThreadMain(GoogleSearch* pGoogleSearch);
	static std::string FilterCurlData(const std::string& data);
	static bool ExtractStartIndex(const json& data, int& result);
	static bool ExtractTotalResults(const json& data, int& result);
	static bool ExtractResults(const json& data, QueryResults& results);
	static void ProcessResults(GoogleSearch* pGoogleSearch, const QueryData& queryData);
	static bool FilterResult(const QueryData& queryData, const QueryResult& result);
	bool IsRunning() const;
	void LoadQueries();
	void DrawResultsUI(bool* pShow);

	std::thread m_QueryThread;
	std::atomic_bool m_QueryThreadActive;
	std::atomic_bool m_QueryThreadStopFlag;
	bool m_ShowResultsUI;
	std::string m_CurlData;
	QueryDatum m_QueryDatum;
	std::mutex m_QueryDatumMutex;
};

} // namespace Tasks
} // namespace Watcher
