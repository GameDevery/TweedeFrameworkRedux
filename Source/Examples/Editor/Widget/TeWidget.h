#pragma once

#include "TeCorePrerequisites.h"
#include "Math/TeVector2.h"
#include "ImGui/imgui.h"

#include "../TeImGuiExt.h"
#include "../TeIconsFontAwesome5.h"

#define CONSOLE_TITLE ICON_FA_TERMINAL " Console"
#define MENUBAR_TITLE "MenuBar"
#define PROJECT_TITLE ICON_FA_BONG " Project"
#define PROPERTIES_TITLE ICON_FA_TOOLS " Properties"
#define RENDER_OPTIONS_TITLE ICON_FA_COG " Render Options"
#define RESOURCES_TITLE ICON_FA_SUITCASE " Resources"
#define TOOLBAR_TITLE "ToolBar"
#define VIEWPORT_TITLE ICON_FA_IMAGE " Viewport"

namespace te
{
    class Widget
    {
    public:
        enum class IconType
        {
            NotAssigned,
            ComponentOptions
        };

        enum class WidgetType
        {
            Console,
            MenuBar,
            Project,
            Properties,
            RenderOptions,
            ToolBar,
            Viewport,
            Resources,
            None
        };

    public:
        Widget(WidgetType type);
        ~Widget() = default;

        virtual void Initialize() = 0;
        virtual void Update() = 0;
        virtual void UpdateBackground() = 0; // Sometimes we want to update while widget is not visible

        template<typename T>
        void PushStyleVar(ImGuiStyleVar idx, T val) { ImGui::PushStyleVar(idx, val); _varPushes++; }

        bool BeginGui();
        bool EndGui();

        bool IsWindow() const { return _isWindow; }
        float GetHeight() const { return _height; }
        bool GetVisible() const { return _isVisible; }
        void SetVisible(bool isVisible) { _isVisible = isVisible; }
        const auto& GetTitle() const { return _title; }
        WidgetType GetType() const { return _type; }

    protected:
        bool _isVisible;
        bool _isWindow;
        UINT32 _flags = ImGuiWindowFlags_NoCollapse;
        float _height = 0.0f;
        float _alpha = -1.0f;
        Vector2 _position = Vector2(-1.0f, -1.0f);
        Vector2 _size = Vector2(-1.0f, -1.0f);
        Vector2 _sizeMax = Vector2(FLT_MAX, FLT_MAX);
        Vector2 _padding = Vector2(-1.0f, - 1.0f);
        std::function<void()> _onStartCallback = nullptr;
        std::function<void()> _onVisibleCallback = nullptr;
        std::function<void()> _onBeginCallback = nullptr;

        String _title;

        bool _begun;
        UINT8 _varPushes = 0;

        WidgetType _type;
    };
}
