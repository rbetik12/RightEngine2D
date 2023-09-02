#pragma once

#include "Window.hpp"
#include "RenderingContext.hpp"

namespace RightEngine
{
    class Surface
    {
    public:
        Surface(const std::shared_ptr<Window>& window,
                const std::shared_ptr<RenderingContext>& context)
        {}

        virtual ~Surface() = default;
    };
}