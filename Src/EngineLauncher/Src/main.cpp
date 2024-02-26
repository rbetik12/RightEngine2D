#pragma once
#include <Engine/Engine.hpp>

int main(int argc, char* argv[])
{
    engine::Engine engine(argc, argv);
    return engine.Run();
}
