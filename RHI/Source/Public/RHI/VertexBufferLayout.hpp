#pragma once

#include <RHI/Config.hpp>
#include <RHI/Types.hpp>
#include <RHI/Assert.hpp>
#include <EASTL/vector.h>
#include <glm/glm.hpp>

namespace rhi
{
    struct VertexBufferElement
    {
        Format      m_type;
        uint32_t    m_count;
        bool        m_normalized;

        uint32_t GetSize() const
        {
            return GetSizeOfType(m_type) * m_count;
        }

        uint32_t static GetSizeOfType(Format format)
        {
            switch (format)
        	{
            case Format::BGRA8_SRGB:
                return 1;
            case Format::RG32_SFLOAT:
                return 8;
            case Format::R32_SFLOAT:
                return 4;
            case Format::RGB32_SFLOAT:
                return 12;
            case Format::RGBA32_SFLOAT:
                return 16;
            }
            assert(false);
            return 0;
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() : m_stride(0)
        {}

        template<typename T>
        void Push(uint32_t count, bool normalized = false)
        {
            RHI_ASSERT(false);
        }

        template<typename T>
        void Push()
        {
            RHI_ASSERT(false);
        }

        inline const eastl::vector<VertexBufferElement>& Elements() const
        {
            return m_elements;
        }

        inline uint32_t Stride() const
        {
            return m_stride;
        }
    private:
        eastl::vector<VertexBufferElement>  m_elements;
        uint32_t                            m_stride;
    };

    template<>
    inline void VertexBufferLayout::Push<float>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::R32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<uint32_t>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::R32_UINT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R32_UINT);
    }

    template<>
    inline void VertexBufferLayout::Push<uint8_t>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::R8_UINT, count,  normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R8_UINT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec2>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::RG32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RG32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec3>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::RGB32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RGB32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec4>(uint32_t count, bool normalized)
    {
        m_elements.push_back({ Format::RGBA32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RGBA32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec2>()
    {
        Push<glm::vec2>(1, false);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec3>()
    {
        Push<glm::vec3>(1, false);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec4>()
    {
        Push<glm::vec4>(1, false);
    }
}