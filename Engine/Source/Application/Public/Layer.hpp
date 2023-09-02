#pragma once

#include "Event.hpp"
#include <string>

namespace RightEngine
{
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() {}
        virtual void OnDetach() {}
        virtual void OnUpdate(float ts) {}
        virtual void OnEvent(Event& event) {}

        const std::string& GetName() const { return debugName; }
    protected:
        std::string debugName;
    };
}