#pragma once

#include <cstdint>

namespace RightEngine
{
    class UniformBufferSet
    {
    public:
        UniformBufferSet(uint32_t frames)
                : frames(frames) {}
        ~UniformBufferSet() {}

        void Create(uint32_t size, uint32_t binding)
        {
            for (uint32_t frame = 0; frame < frames; frame++)
            {
                BufferDescriptor descriptor{};
                descriptor.memoryType = MemoryType::CPU_GPU;
                descriptor.size = size;
                descriptor.type = BufferType::UNIFORM;

                auto uniformBuffer = Device::Get()->CreateBuffer(descriptor, nullptr);
                Set(uniformBuffer, frame, binding);
            }
        }

        std::shared_ptr<Buffer>& Get(uint32_t binding, uint32_t frame = 0)
        {
            R_CORE_ASSERT(uniformBuffers.find(frame) != uniformBuffers.end(), "");
            R_CORE_ASSERT(uniformBuffers.at(frame).find(binding) != uniformBuffers.at(frame).end(), "");

            return uniformBuffers.at(frame).at(binding);
        }

        void Set(const std::shared_ptr<Buffer>& uniformBuffer, uint32_t frame = 0, uint32_t binding = 0)
        {
            uniformBuffers[frame][binding] = uniformBuffer;
        }
    private:
        uint32_t frames;
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::shared_ptr<Buffer>>> uniformBuffers; // frame->set->binding
    };
}