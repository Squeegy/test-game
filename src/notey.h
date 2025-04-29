#ifndef NOTEY_H
#define NOTEY_H

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot {

    class Notey : public Sprite2D {
        GDCLASS(Notey, Sprite2D)

    private:
        double time_passed = 0.0;
        float random_multiplier;
        

    protected:
        static void _bind_methods();

    public:
        double initial_x;
        double initial_y;
        
        Notey();
        ~Notey();
        
        void _process(double delta) override;
        Vector2 get_parent_scale(Node* node);
    };

}

#endif