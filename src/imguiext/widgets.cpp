///////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <functional>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imguiext/cubicbezier.h"
#include "imguiext/widgets.h"

namespace ImGui
{

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.txt)
void HelpMarker(const std::string& desc)
{
    using namespace ImGui;

    TextDisabled("(?)");
    if (IsItemHovered())
    {
        BeginTooltip();
        PushTextWrapPos(GetFontSize() * 35.0f);
        TextUnformatted(desc.c_str());
        PopTextWrapPos();
        EndTooltip();
    }
}

static gfx::CubicBezier fast_out_slow_in(0.4, 0.0, 0.2, 1.0);

static float bezier(float t) {
    return static_cast<float>(fast_out_slow_in.Solve(t));
}

static auto lerp(float x0, float x1) 
{
    return [=](float t) {
        return (1 - t) * x0 + t * x1;
    };
}

static float lerp(float x0, float x1, float t) 
{
    return lerp(x0, x1)(t);
}

static auto interval(float T0, float T1, std::function<float(float)> tween = lerp(0.0, 1.0)) 
{
    return [=](float t) {
        return t < T0 ? 0.0f : t > T1 ? 1.0f : tween((t - T0) / (T1 - T0));
    };
}

template <int T>
float sawtooth(float t) {
    return ImFmod(((float)T)*t, 1.0f);
}

bool Spinner(const char* label, float radius, int thickness, const ImU32& color) 
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    // Render
    const ImVec2 center = ImVec2(pos.x + radius, pos.y + radius + thickness + style.FramePadding.y);

    const float start_angle = -IM_PI / 2.0f;         // Start at the top
    const int num_detents = 5;                       // how many rotations we want before a repeat
    const int skip_detents = 3;                      // how many steps we skip each rotation
    const float period = 5.0f;                       // in seconds
    const float t = ImFmod(g.Time, period) / period; // map period into [0, 1]

    // Tweening functions for each part of the spinner
    auto stroke_head_tween = [num_detents](float t) {
        t = sawtooth<5>(t);
        return interval(0.0, 0.5, bezier)(t);
    };

    auto stroke_tail_tween = [](float t) {
        t = sawtooth<5>(t);
        return interval(0.5, 1.0, bezier)(t);
    };

    auto step_tween = [=](float t) {
        return floor(lerp(0.0, (float)num_detents, t));
    };

    auto rotation_tween = sawtooth<num_detents>;

    const float head_value = stroke_head_tween(t);
    const float tail_value = stroke_tail_tween(t);
    const float step_value = step_tween(t);
    const float rotation_value = rotation_tween(t);

    const float min_arc = 30.0f / 360.0f * 2.0f * IM_PI;
    const float max_arc = 270.0f / 360.0f * 2.0f * IM_PI;
    const float step_offset = skip_detents * 2.0f * IM_PI / num_detents;
    const float rotation_compensation = ImFmod(4.0f * IM_PI - step_offset - max_arc, 2.0f * IM_PI);

    const float a_min = start_angle + tail_value * max_arc + rotation_value * rotation_compensation - step_value * step_offset;
    const float a_max = a_min + (head_value - tail_value) * max_arc + min_arc;

    window->DrawList->PathClear();

    int num_segments = 24;
    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(center.x + ImCos(a) * radius,
            center.y + ImSin(a) * radius));
    }

    window->DrawList->PathStroke(color, false, static_cast<float>(thickness));

    return true;
}

} // namespace ImGui
