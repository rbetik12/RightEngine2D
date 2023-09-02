#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include "Event.hpp"

using Callback = std::function<bool(const Event&)>;

struct CallbackContext
{
    Callback callback;
    void* instancePointer = nullptr;

    CallbackContext(Callback&& callback, void* instancePointer) : callback(callback), instancePointer(instancePointer)
    {}
};

class EventDispatcher
{
public:
    static EventDispatcher& Get()
    {
        static EventDispatcher dispatcher;
        return dispatcher;
    }

    void Subscribe(const char* descriptor, CallbackContext context);
    void UnSubscribe(const char* descriptor, CallbackContext context);
    void Emit(const Event& event) const;

    EventDispatcher(const EventDispatcher& dispatcher) = delete;
    EventDispatcher& operator=(const EventDispatcher& dispatcher) = delete;
    EventDispatcher(EventDispatcher&& dispatcher) = delete;
    EventDispatcher& operator=(EventDispatcher&& dispatcher) = delete;

private:
    EventDispatcher() = default;
    ~EventDispatcher() = default;

    std::unordered_map<const char*, std::vector<CallbackContext>> observers;
};

#define EVENT_CALLBACK(callback) CallbackContext(std::bind(&callback, this, std::placeholders::_1), this)
