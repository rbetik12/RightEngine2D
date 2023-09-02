#pragma once

#include "Window.hpp"
#include <memory>

namespace RightEngine
{
    struct RenderingContextInfo
    {
        uint32_t extensionAmount{ 0 };
    };

    class RenderingContext
    {
    public:
        RenderingContext(const std::shared_ptr<Window>& window) : window(window)
        {}
        virtual ~RenderingContext() = default;

        const RenderingContextInfo& GetInfo()
        { return info; }

    protected:
        std::weak_ptr<Window> window;
        RenderingContextInfo info;
    };
}
