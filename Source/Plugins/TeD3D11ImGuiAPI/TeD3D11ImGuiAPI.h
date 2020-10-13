#pragma once

#include "TeD3D11ImGuiAPIPrerequisites.h"
#include "Gui/TeGuiAPI.h"

namespace te
{
    class D3D11ImGuiAPI: public GuiAPI
    {
    public:
        D3D11ImGuiAPI();
        ~D3D11ImGuiAPI();

        /** @copydoc GuiAPI::Initialize */
        void Initialize(void* data) override;

        /** @copydoc GuiAPI::Destroy */
        void Destroy() override;

        /** @copydoc GuiAPI::Update */
        void Update() override;

        /** @copydoc GuiAPI::Begin */
        void Begin() override;

        /** @copydoc GuiAPI::End */
        void End() override;

    public:
        /** @copydoc GuiAPI::CharInput */
        void CharInput(UINT32 character) override;

        /** @copydoc GuiAPI::CursorMoved */
        void CursorMoved(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates) override;

        /** @copydoc GuiAPI::CursorPressed */
        void CursorPressed(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates) override;

        /** @copydoc GuiAPI::CursorReleased */
        void CursorReleased(const Vector2I& cursorPos, OSMouseButton button, const OSPointerButtonStates& btnStates) override;

        /** @copydoc GuiAPI::CursorDoubleClick */
        void CursorDoubleClick(const Vector2I& cursorPos, const OSPointerButtonStates& btnStates) override;

        /** @copydoc GuiAPI::MouseWheelScrolled */
        void MouseWheelScrolled(float scrollPos) override;

        /** @copydoc GuiAPI::ButtonUp */
        void ButtonUp(ButtonEvent event) override;

        /** @copydoc GuiAPI::ButtonDown */
        void ButtonDown(ButtonEvent event) override;

        /** @copydoc GuiAPI::OnInputCommandEntered */
        void OnInputCommandEntered(InputCommandType commandType);

        /** @copydoc GuiAPI::KeyUp */
        void KeyUp(UINT32 keyCode) override;

        /** @copydoc GuiAPI::KeyDown */
        void KeyDown(UINT32 keyCode) override;
    };
}
