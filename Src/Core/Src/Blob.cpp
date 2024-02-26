#include <Core/Blob.hpp>

namespace core
{
    Blob::Blob(const void* data, uint32_t size)
    {
        m_data.resize(size);
        std::memcpy(m_data.data(), data, size);
    }

    Blob::~Blob()
    {}
}
