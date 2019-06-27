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

#include <SDL.h>
#include "imgui/imgui.h"

#include "atlas/atlas.h"
#include "log.h"
#include "watcher_rep.h"
#include "watcher.h"

WatcherRep::WatcherRep(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_CellSize(128.0f)
{
	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
	m_pAtlas = std::make_unique< Atlas::Atlas >(windowWidth, windowHeight);

	ImGui::StyleColorsDark();
}

WatcherRep::~WatcherRep()
{

}

void WatcherRep::ProcessEvent(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION)
	{
		const SDL_MouseMotionEvent* ev = reinterpret_cast<const SDL_MouseMotionEvent*>(&event);
		if ((ev->state & SDL_BUTTON_LMASK) > 0)
		{
			m_pAtlas->OnMouseDrag(ev->xrel, ev->yrel);
		}
	}
	else if (event.type == SDL_MOUSEWHEEL)
	{
		const SDL_MouseWheelEvent* ev = reinterpret_cast<const SDL_MouseWheelEvent*>(&event);
		if (ev->y > 0)
		{
			m_pAtlas->OnZoomIn();
		}
		else if (ev->y <= 0)
		{
			m_pAtlas->OnZoomOut();
		}
	}
	else if (event.type == SDL_WINDOWEVENT)
	{
		if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
		{
			m_pAtlas->OnWindowSizeChanged(event.window.data1, event.window.data2);
		}
	}
}

void WatcherRep::Update()
{
	static const float sBaseCellSize = 128.0f;
	m_CellSize = sBaseCellSize;
}

void WatcherRep::Render()
{
	unsigned int flags = 0;
	flags |= ImGuiWindowFlags_NoResize;
	flags |= ImGuiWindowFlags_NoMove;
	flags |= ImGuiWindowFlags_NoSavedSettings;
	flags |= ImGuiWindowFlags_NoTitleBar;
	flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGuiIO& io = ImGui::GetIO();
	ImTextureID my_tex_id = io.Fonts->TexID;

	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
	ImVec2 windowSize = ImVec2(static_cast<float>(windowWidth), static_cast<float>(windowHeight));

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Watcher", nullptr, flags);

	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	m_pAtlas->Render();

	ImGui::End();

	GeoInfoVector geoInfos = g_pWatcher->GetGeoInfos();
	for (const GeoInfo& geoInfo : geoInfos)
	{
		float locationX, locationY;
		m_pAtlas->GetScreenCoordinates(geoInfo.GetLongitude(), geoInfo.GetLatitude(), locationX, locationY);
		pDrawList->AddCircle(ImVec2(locationX, locationY), 4.0f, ImColor(255, 0, 0), 4);
	}
}

void WatcherRep::AddToAtlas(float latitude, float longitude, int index)
{

}