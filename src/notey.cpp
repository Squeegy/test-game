#include "notey.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Notey::Notey() {
    // Constructor logic (if any)
    initial_x = get_scale().x;
    initial_y = get_scale().y;
}

Notey::~Notey() {
    // Destructor logic (if any)
}

void Notey::_bind_methods() {
    // Binding methods for Godot
}

void Notey::_process(double delta) {
    time_passed += delta;
    // Example logic: oscillate the scale of the sprite
    float x_scale = initial_x + Math::sin(time_passed*1.5) * .02;
    float y_scale = initial_y + Math::sin(time_passed*1.5 + Math_PI) * .01;
    UtilityFunctions::print("x_scale: ", x_scale, ", y_scale: ", y_scale);
    set_scale(Vector2(x_scale, y_scale));
}