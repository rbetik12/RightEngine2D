#pragma once

#include <glm/glm.hpp>
#include <cstddef>
#include <functional>

namespace RightEngine
{
    class Utils
    {
    public:
        static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
    };
}
