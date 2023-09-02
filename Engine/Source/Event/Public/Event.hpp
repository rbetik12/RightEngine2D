#pragma once

class Event
{
public:
    virtual ~Event() = default;

    virtual const char* GetType() const = 0;
};

#define EVENT_BODY(EventName) static constexpr const char* descriptor = EventName; \
virtual const char* GetType() const override { return descriptor;}