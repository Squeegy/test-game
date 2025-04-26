#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

class GDExample : public CharacterBody2D {
	GDCLASS(GDExample, CharacterBody2D)

private:
	double time_passed = 0.0;

protected:
	static void _bind_methods();

public:
	GDExample();
	~GDExample();

	void _physics_process(double delta) override;
};

}

#endif