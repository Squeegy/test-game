#include "bubble_gun.h"
#include "bubble.h"
#include <godot_cpp/classes/input.hpp>

namespace godot {

BubbleGun::BubbleGun() {
    // Constructor logic (if any)
}

BubbleGun::~BubbleGun() {
    // Destructor logic (if any)
}

void BubbleGun::_bind_methods() {
    // Bind methods here, e.g.:
    // ClassDB::bind_method(D_METHOD("method_name"), &BubbleGun::method_name);
}

void BubbleGun::_physics_process(double delta) {
    time_passed += delta;
    double rotation_speed = 3.0;
    if (Input::get_singleton()->is_action_pressed("gun_left")) {
        set_rotation(get_rotation() - rotation_speed * delta);
    }
    if (Input::get_singleton()->is_action_pressed("gun_right")) {
        set_rotation(get_rotation() + rotation_speed * delta);
    }
    // Add your physics processing logic here
}

void BubbleGun::_ready() {
    // Initialization logic when the node is ready
    Array bubbles;
    bubbles.push_back(get_node<Bubble>(NodePath("Bubble4")));
    bubbles.push_back(get_node<Bubble>(NodePath("Bubble3")));
    bubbles.push_back(get_node<Bubble>(NodePath("Bubble2")));
    bubbles.push_back(get_node<Bubble>(NodePath("Bubble")));

    for (int i = 0; i < bubbles.size(); i++) {
        Bubble* bubble = Object::cast_to<Bubble>(bubbles[i]);
        if (bubble) {
            bubble->initial_scale = get_scale().x * (0.02 + (.005*i)); // Set the scale of the bubble sprite to 10% of the tank's scale
        }
    }
}

}
