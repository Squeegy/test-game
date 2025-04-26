#include "bubble_tank.h"
#include "bubble.h"
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


using namespace godot;

void BubbleTank::_bind_methods() {
}

BubbleTank::BubbleTank() {
    // Initialize any variables here.
    time_passed = 0.0;
}

BubbleTank::~BubbleTank() {
    // Add your cleanup here.
}

void BubbleTank::_ready() {
    Bubble* sprite = get_node<Bubble>(NodePath("Bubble"));
    if (sprite) {
        UtilityFunctions::print("Initial scale: ", get_scale().x);
        sprite->initial_scale = get_scale().x * 0.1;   // Set the scale of the bubble sprite to 10% of the tank's scale
    }
}

void BubbleTank::_physics_process(double delta) {
    double rotation_speed = 3.0; // radians per second
    double move_speed = 200.0;

    // Turn left/right
    if (Input::get_singleton()->is_action_pressed("turn_left")) {
        set_rotation(get_rotation() - rotation_speed * delta);
    }
    if (Input::get_singleton()->is_action_pressed("turn_right")) {
        set_rotation(get_rotation() + rotation_speed * delta);
    }

    // Move forward/backward
    Vector2 direction = Vector2(cos(get_rotation()), sin(get_rotation()));
    Vector2 velocity = Vector2();

    if (Input::get_singleton()->is_action_pressed("move_forward")) {
        velocity += direction * move_speed;
    }
    if (Input::get_singleton()->is_action_pressed("move_backward")) {
        velocity -= direction * move_speed;
    }

    set_velocity(velocity);
    move_and_slide();
}