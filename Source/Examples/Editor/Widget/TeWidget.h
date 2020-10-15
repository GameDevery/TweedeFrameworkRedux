#pragma once

#include "TeCorePrerequisites.h"
#include "Math/TeVector2.h"

#include "ImGui/imgui.h"

namespace te
{
    class Widget
    {
    public:
        Widget();
        ~Widget() = default;

        virtual void Initialize() = 0;
        virtual void Update() = 0;

        template<typename T>
        void PushStyleVar(ImGuiStyleVar idx, T val) { ImGui::PushStyleVar(idx, val); _varPushes++; }

        bool BeginGui();
        bool EndGui();

        bool IsWindow() const { return _isWindow; }
        float GetHeight() const { return _height; }
        bool GetVisible() const { return _isVisible; }
        void SetVisible(bool isVisible) { _isVisible = isVisible; }
        const auto& GetTitle() const { return _title; }

    protected:
        bool _isVisible;
        bool _isWindow;
        UINT32 _flags = ImGuiWindowFlags_NoCollapse;
        float _height = 0;
        float _alpha = -1.0f;
        Vector2 _position = Vector2(-1.0f, -1.0f);
        Vector2 _size = Vector2(-1.0f, -1.0f);
        Vector2 _sizeMax = Vector2(FLT_MAX, FLT_MAX);
        Vector2 _padding = Vector2(-1.0f, - 1.0f);

        String _title;

        bool _begun;
        uint8_t _varPushes = 0;
    };
}
