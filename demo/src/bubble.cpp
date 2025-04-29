#include "bubble.h"
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


namespace godot {

	Bubble::Bubble() {
		random_multiplier = UtilityFunctions::randf_range(0.5, 2.0); // Randomize undulation speed
		time_passed += UtilityFunctions::randf_range(0.0, Math_TAU); // Initialize with a random value between 0 and 2π
	}

	Bubble::~Bubble() {
		// Destructor logic (if any)
	}

	void Bubble::_bind_methods() {
		// Bind methods here, e.g., ClassDB::bind_method(D_METHOD("method_name"), &Bubble::method_name);
	}

	void Bubble::_process(double delta) {
		time_passed += delta;
		double scale_factor = initial_scale + ((initial_scale*0.1) * sin(random_multiplier*time_passed)); // Scale the bubble based on time passed
		set_scale(Vector2(scale_factor, scale_factor));
		// UtilityFunctions::print("Initial scale: ", initial_scale);
		// Add processing logic here
	}
}
