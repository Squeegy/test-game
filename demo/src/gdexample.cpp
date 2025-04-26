#include "gdexample.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GDExample::_bind_methods() {
}

GDExample::GDExample() {
	// Initialize any variables here.
	time_passed = 0.0;
}

GDExample::~GDExample() {
	// Add your cleanup here.
}

void GDExample::_process(double delta) {
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
}