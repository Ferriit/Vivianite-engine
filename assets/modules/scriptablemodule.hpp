#pragma once
#include <cstdint>
#include "base.hpp"

using Entity = uint64_t;

class ScriptableModule : public Module {
    public:
        virtual void setup() {}
        virtual void update() {}
        virtual void fixed_update() {}

        virtual void on_exit() {}
};

