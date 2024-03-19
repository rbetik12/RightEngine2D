#pragma once

#include <Core/Config.hpp>
#include <EASTL/vector.h>

namespace core
{
    //Just a buffer of plain binary data.
    //Doesn't take ownership of the pointer, allocates new blocks of memory for the data.
    class CORE_API Blob
    {
    public:
        using Payload = eastl::vector<uint8_t>;

        Blob() = default;
        Blob(const void* data, size_t size);
        ~Blob();

        size_t          size() const { return m_data.size(); }
        bool            empty() const { return m_data.empty(); }
        const void*     raw() const { return m_data.data(); }
        void*           raw() { return m_data.data(); }
        const Payload&  data() const { return m_data; }

    private:
        Payload m_data;
    };
}
