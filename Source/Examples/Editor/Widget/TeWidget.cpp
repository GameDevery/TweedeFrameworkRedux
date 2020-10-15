#include "TeWidget.h"

#include "ImGui/imgui.h"

namespace te
{
    Widget::Widget(WidgetType type)
        : _isVisible(true)
        , _isWindow(true)
        , _begun(false)
        , _type(type)
    { }

    bool Widget::BeginGui()
    {
        // Callback
        if (_onStartCallback)
            _onStartCallback();

        if (!_isWindow)
            return true;

        if (!_isVisible)
            return false;

        if (_onVisibleCallback)
            _onVisibleCallback();

        if (_position.x != -1.0f && _position.y != -1.0f)
            ImGui::SetNextWindowPos(ImVec2(_position.x, _position.y));

        if (_padding.x != -1.0f && _padding.y != -1.0f)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(_padding.x, _padding.y));
            _varPushes++;
        }

        if (_alpha != -1.0f)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, _alpha);
            _varPushes++;
        }

        if (_size.x != -1.0f && _size.y != -1.0f)
            ImGui::SetNextWindowSize(ImVec2(_size.x, _size.y), ImGuiCond_FirstUseEver);

        if ((_size.x != -1.0f && _size.y != -1.0f) || (_sizeMax.x != FLT_MAX && _sizeMax.y != FLT_MAX))
        {
            ImGui::SetNextWindowSizeConstraints(ImVec2(_size.x, _size.y), ImVec2(_sizeMax.x, _sizeMax.y));
        }

        // Begin
        if (ImGui::Begin(_title.c_str(), &_isVisible, _flags))
        {
            _height = ImGui::GetWindowHeight();
            _begun = true;
        }
        else
        {
           _begun = true;
        }

        if (_begun && _onBeginCallback)
            _onBeginCallback();

        return _begun;
    }

    bool Widget::EndGui()
    {
        if (_begun)
        {
            ImGui::End();
        }

        // Pop style variables
        ImGui::PopStyleVar(_varPushes);
        _varPushes = 0;

        _begun = false;

        return true;
    }
}
