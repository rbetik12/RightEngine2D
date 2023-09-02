#pragma once

#include "GraphicsPipelineDescriptor.hpp"

namespace RightEngine
{
    class GraphicsPipeline
    {
    public:
        const GraphicsPipelineDescriptor& GetPipelineDescriptor() const
        { return pipelineDescriptor; }

        const RenderPassDescriptor& GetRenderPassDescriptor() const
        { return renderPassDescriptor; }
        
        virtual void Resize(int x, int y) = 0;

    protected:
        GraphicsPipeline(const GraphicsPipelineDescriptor& descriptor,
                         const RenderPassDescriptor& renderPassDescriptor) : pipelineDescriptor(descriptor),
                                                                             renderPassDescriptor(renderPassDescriptor)
        {}

        GraphicsPipelineDescriptor pipelineDescriptor;
        RenderPassDescriptor renderPassDescriptor;
    };
}
