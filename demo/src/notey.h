#ifndef NOTEY_H
#define NOTEY_H

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <vector>
#include "audio_input.h"

namespace godot {
    enum class NoteyMode {
        JAM,
        FEED,
        TBD
    };

    class Notey : public Sprite2D {
        GDCLASS(Notey, Sprite2D)

    private:
        double time_passed = 0.0;
        float random_multiplier;
        AudioInput *audio_input = nullptr;
        Label *note_label = nullptr;
        NoteyMode mode = NoteyMode::FEED; // Default mode
        float mood = 0.5f;
        float hunger = 0.3f;
        int sequence_index = 0;
        std::vector<String> note_sequence;
        std::vector<String> target_sequence;
        int current_input_index = 0;
        String last_note = "";
        double debounce_timer = 0.0;
        const double debounce_threshold = 0.2f;
        bool note_ready = false;


    protected:
        static void _bind_methods();

    public:
        double initial_x;
        double initial_y;
        
        Notey();
        ~Notey();
        
        void _ready() override;
        void _process(double delta) override;
        void set_mode(NoteyMode new_mode);
        void setup_feed_mode();
        void setup_jam_mode();
        void handle_note_input(const String& note, float volume);
        void update_note_debounce(const String& note, double delta);
        void update_visuals();
        Vector2 get_parent_scale(Node* node);
    };

}

#endif