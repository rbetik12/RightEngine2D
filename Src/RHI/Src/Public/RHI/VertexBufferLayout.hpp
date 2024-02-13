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
        std::string m_name;
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
            default:
                RHI_ASSERT(false);
                return 0;
            }
            
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() : m_stride(0)
        {}

        template<typename T>
        void Push(const std::string& name, uint32_t count, bool normalized = false)
        {
            RHI_ASSERT(false);
        }

        template<typename T>
        void Push(const std::string& name)
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

        inline bool Empty() const
        {
            return m_stride == 0;
        }

    private:
        eastl::vector<VertexBufferElement>  m_elements;
        uint32_t                            m_stride;
    };

    template<>
    inline void VertexBufferLayout::Push<float>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::R32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<uint32_t>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::R32_UINT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R32_UINT);
    }

    template<>
    inline void VertexBufferLayout::Push<uint8_t>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::R8_UINT, count,  normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::R8_UINT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec2>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::RG32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RG32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec3>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::RGB32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RGB32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec4>(const std::string& name, uint32_t count, bool normalized)
    {
        m_elements.push_back({ name, Format::RGBA32_SFLOAT, count, normalized });
        m_stride += count * VertexBufferElement::GetSizeOfType(Format::RGBA32_SFLOAT);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec2>(const std::string& name)
    {
        Push<glm::vec2>(name, 1, false);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec3>(const std::string& name)
    {
        Push<glm::vec3>(name, 1, false);
    }

    template<>
    inline void VertexBufferLayout::Push<glm::vec4>(const std::string& name)
    {
        Push<glm::vec4>(name, 1, false);
    }
}