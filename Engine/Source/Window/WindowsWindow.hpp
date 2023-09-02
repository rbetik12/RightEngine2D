#ifdef WIN32_WINDOW
#pragma once

#include <Window.hpp>
#include <Windows.h>

namespace RightEngine
{
    class WindowsWindow : public Window
    {
        friend class Window;

    public:

        void OnUpdate() override;

        void Swap() override;

        void PollEvents();

        void Clear();

    private:
        WindowsWindow(std::string title, uint32_t width, uint32_t height);

        HWND windowHandle = nullptr;
        HINSTANCE appInstance = nullptr;
    };
}

#endif