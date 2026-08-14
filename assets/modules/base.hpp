#pragma once
#include <cstdint>

using Entity = uint64_t;

class Module {
    public:
        Entity entity;
        double delta_time = 0.0f;

        virtual ~Module() = default;
        
        virtual void setup() {}
        virtual void update() {}
        virtual void fixed_update() {}

        virtual void on_exit() {}
};
