#include "EventDispatcher.hpp"

void EventDispatcher::Subscribe(const char* descriptor, CallbackContext context)
{
    observers[descriptor].push_back(context);
}
//TODO: Fix crash on engine finish
void EventDispatcher::UnSubscribe(const char* descriptor, CallbackContext context)
{
    if (observers.find(descriptor) == observers.end())
    {
        return;
    }

    std::vector<CallbackContext> newCallbacks;
    for (auto&& callbackContext: observers.at(descriptor))
    {
        if (callbackContext.instancePointer != context.instancePointer) {
            newCallbacks.push_back(callbackContext);
        }
    }

    observers[descriptor] = newCallbacks;
}

void EventDispatcher::Emit(const Event& event) const
{
    auto type = event.GetType();

    if (observers.find(type) == observers.end())
    {
        return;
    }

    const auto& observersList = observers.at(type);

    for (const auto& observer: observersList)
    {
        observer.callback(event);
    }
}
