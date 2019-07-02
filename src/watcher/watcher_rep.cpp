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
#include "texture_loader.h"
#include "watcher_rep.h"
#include "watcher.h"

static unsigned int sPinWidth = 16;
static unsigned int sPinHeight = 26;
static unsigned int sPinHalfWidth = sPinWidth / 2;

WatcherRep::WatcherRep(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_CellSize(128.0f)
{
	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
	m_pAtlas = std::make_unique< Atlas::Atlas >(windowWidth, windowHeight);

	SetUserInterfaceStyle();

	m_PinTexture = TextureLoader::LoadTexture("textures/pin.png");
}

WatcherRep::~WatcherRep()
{

}

void WatcherRep::SetUserInterfaceStyle()
{
	ImGuiStyle &st = ImGui::GetStyle();
	st.FrameBorderSize = 1.0f;
	st.FramePadding = ImVec2(4.0f, 2.0f);
	st.ItemSpacing = ImVec2(8.0f, 2.0f);
	st.WindowBorderSize = 1.0f;
	//st.TabBorderSize = 1.0f;
	st.WindowRounding = 1.0f;
	st.ChildRounding = 1.0f;
	st.FrameRounding = 1.0f;
	st.ScrollbarRounding = 1.0f;
	st.GrabRounding = 1.0f;
	//st.TabRounding = 1.0f;

	// Setup style
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 0.95f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.12f, 0.12f, 0.90f);
	colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.53f, 0.53f, 0.53f, 0.46f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.85f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 0.53f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.48f, 0.48f, 0.48f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.48f, 0.47f, 0.47f, 0.91f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.55f, 0.55f, 0.62f);
	colors[ImGuiCol_Button] = ImVec4(0.50f, 0.50f, 0.50f, 0.63f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.67f, 0.67f, 0.68f, 0.63f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.63f);
	colors[ImGuiCol_Header] = ImVec4(0.54f, 0.54f, 0.54f, 0.58f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.64f, 0.65f, 0.65f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.25f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.58f, 0.58f, 0.58f, 0.50f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.81f, 0.81f, 0.81f, 0.64f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.87f, 0.87f, 0.87f, 0.53f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.87f, 0.87f, 0.87f, 0.74f);
	//colors[ImGuiCol_Tab] = ImVec4(0.01f, 0.01f, 0.01f, 0.86f);
	//colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
	//colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	//colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.00f);
	//colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	//colors[ImGuiCol_DockingPreview] = ImVec4(0.38f, 0.48f, 0.60f, 1.00f);
	//colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.77f, 0.33f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.87f, 0.55f, 0.08f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.60f, 0.76f, 0.47f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.58f, 0.58f, 0.58f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	//colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	//colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void WatcherRep::ProcessEvent(const SDL_Event& event)
{
	if (event.type == SDL_MOUSEMOTION && ImGui::GetIO().WantCaptureMouse == false)
	{
		const SDL_MouseMotionEvent* ev = reinterpret_cast<const SDL_MouseMotionEvent*>(&event);
		if ((ev->state & SDL_BUTTON_LMASK) > 0)
		{
			m_pAtlas->OnMouseDrag(ev->xrel, ev->yrel);
		}
	}
	else if (event.type == SDL_MOUSEWHEEL && ImGui::GetIO().WantCaptureMouse == false)
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
	flags |= ImGuiWindowFlags_NoInputs;

	ImGuiIO& io = ImGui::GetIO();
	ImTextureID my_tex_id = io.Fonts->TexID;

	int windowWidth;
	int windowHeight;
	SDL_GetWindowSize(m_pWindow, &windowWidth, &windowHeight);
	ImVec2 windowSize = ImVec2(static_cast<float>(windowWidth + 8), static_cast<float>(windowHeight + 8));

	ImGui::SetNextWindowPos(ImVec2(-4, -4));
	ImGui::SetNextWindowSize(windowSize);
	ImGui::Begin("Watcher", nullptr, flags);
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	m_pAtlas->Render();
	ImGui::End();

	CameraVector cameras = g_pWatcher->GetCameras();
	for (Camera camera : cameras)
	{
		GeolocationData* pGeolocationData = camera.GetGeolocationData();
		if (pGeolocationData != nullptr)
		{
			float locationX, locationY;
			m_pAtlas->GetScreenCoordinates(pGeolocationData->GetLongitude(), pGeolocationData->GetLatitude(), locationX, locationY);
			pDrawList->AddImage(
				reinterpret_cast<ImTextureID>(m_PinTexture),
				ImVec2(locationX - sPinHalfWidth, locationY - sPinHeight),
				ImVec2(locationX + sPinHalfWidth, locationY),
				ImVec2(0, 0),
				ImVec2(1, 1),
				ImColor(255, 123, 0)
			);
		}
	}

	CameraVector hoveredCameras = GetHoveredCameras();
	for (Camera& camera : hoveredCameras)
	{
		ImGui::SetTooltip("%s | %s", camera.GetAddress().ToString().c_str(), camera.GetTitle().c_str());
	}
}

CameraVector WatcherRep::GetHoveredCameras()
{
	CameraVector hoveredCameras;
	CameraVector cameras = g_pWatcher->GetCameras();

	int mx, my;
	SDL_GetMouseState(&mx, &my);

	for (Camera camera : cameras)
	{
		GeolocationData* pGeolocationData = camera.GetGeolocationData();
		if (pGeolocationData != nullptr)
		{
			float locationX, locationY;
			m_pAtlas->GetScreenCoordinates(pGeolocationData->GetLongitude(), pGeolocationData->GetLatitude(), locationX, locationY);

			if (mx > locationX - sPinHalfWidth && mx < locationX + sPinHalfWidth && 
				my > locationY - sPinHeight && my < locationY)
			{
				hoveredCameras.push_back(camera);
			}
		}
	}

	return hoveredCameras;
}