#include "base.hpp"
#include "../definitions/types.hpp"

class Transform : public Module {
    public:
        enum class Axis {
            X, Y, Z
        };

        vivianite::vec3 position;
        vivianite::vec3 rotation;
        vivianite::vec3 scale;

        virtual void setup() override {
            this->position = {0.0f, 0.0f, 0.0f};
            this->rotation = {0.0f, 0.0f, 0.0f};
            this->scale = {1.0f, 1.0f, 1.0f};
        }
        void rotate(Axis axis, float amount) {
            switch (axis) {
                case Axis::X:
                    this->rotation.x += amount;
                    break;

                case Axis::Y:
                    this->rotation.y += amount;
                    break;

                case Axis::Z:
                    this->rotation.z += amount;
                    break;
            }
        }
};
