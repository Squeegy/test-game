#ifndef BUBBLETANK_H
#define BUBBLETANK_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

	class BubbleTank : public CharacterBody2D {
		GDCLASS(BubbleTank, CharacterBody2D)

	private:
		double time_passed = 0.0;

	protected:
		static void _bind_methods();

	public:
		BubbleTank();
		~BubbleTank();

		void _physics_process(double delta) override;
		void _ready();
	};

}

#endif