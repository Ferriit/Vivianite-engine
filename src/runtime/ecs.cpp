#include "include.hpp"

namespace vivianite {
    template<typename T>
    void ECS::add_module(Entity entity, T* module) {
        this->modules[entity].push_back({
            module,
            typeid(T)
        });
    }

    template<typename T>
    T* ECS::get_module(Entity entity) {
        auto& entity_modules = modules[entity];

        for (auto& module : entity_modules) {
            if (module.type == typeid(T))
                return static_cast<T*>(module.instance);
        }

        return nullptr;
    }
}
