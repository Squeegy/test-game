#ifndef HELLO_H
#define HELLO_H

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

class Hello : public Node2D {
    GDCLASS(Hello, Node2D);

protected:
    static void _bind_methods() {}

public:
    Hello() {}
    ~Hello() {}

    void _ready() {
        UtilityFunctions::print("Hello _ready");
    }
};

}

#endif
