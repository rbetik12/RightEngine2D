#pragma once

#include <Event.hpp>
#include <ShutdownEvent.hpp>
#include <easyargs.h>

enum class LaunchMode
{
    Test,
    Game
};

namespace RightEngine
{
    class LaunchEngine
    {
    public:
        static void Init(int argc, char *argv[]);

        static void Exit();

    private:
        class LaunchContext
        {
            friend class LaunchEngine;

        public:
            bool OnEvent(const Event &event);

        private:
            LaunchContext();

            void SetCmdArgs(int argc, char *argv[]);

            void ParseCmdArgs();

            std::unique_ptr<EasyArgs> easyArgs;
        };

        static LaunchContext *launchContext;
    };
}