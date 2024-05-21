#pragma once

#include <Core/Blob.hpp>
#include <Core/Math.hpp>
#include <Core/Assert.hpp>
#include <Core/EASTLIntergration.hpp>
#include <Core/Hash.hpp>
#include <Core/Log.hpp>
#include <Core/Profiling.hpp>
#include <Core/Thread.hpp>

#include <Engine/Assert.hpp>
#include <Engine/Engine.hpp>
#include <Engine/Registration.hpp>
#include <Engine/Service/ThreadService.hpp>
#include <Engine/Service/Filesystem/VirtualFilesystemService.hpp>
#include <Engine/Service/Resource/ResourceService.hpp>

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <taskflow/taskflow.hpp>

#include <string>
#include <memory>
#include <cstdint>
#include <cstddef>