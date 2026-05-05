#pragma once

namespace dae
{
    class InputCommand
    {
        public:

        virtual ~InputCommand() = default;

        virtual void Execute() = 0;

        protected:

        InputCommand() = default;
    };
}