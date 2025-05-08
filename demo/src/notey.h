#ifndef NOTEY_H
#define NOTEY_H

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
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
        Label *mode_label = nullptr;

        NoteyMode mode = NoteyMode::FEED; // Default mode

        float mood = .7f;
        float fitness = .99f;
        float hunger = 0.0f;
        Control* mood_bar = nullptr;
        Control* fitness_bar = nullptr;
        Control* hunger_bar = nullptr;

        int sequence_index = 0;
        std::vector<String> note_sequence;
        std::vector<String> target_sequence;
        HBoxContainer* note_prompt_container = nullptr;
        int current_input_index = 0;

        String last_note = "";
        String last_octave = "";
        int last_midi = -1;
        String consumed_note = "";
        String consumed_octave = "";
        int consumed_midi = -1;
        bool note_ready = false;
        double debounce_timer = 0.0;
        const double debounce_threshold = 0.15f;
        


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
        void initialize();
        void handle_note_input(int midi, const String& octave, float volume);
        void handle_feed(int midi, const String& octave, float volume);
        void handle_jam(int midi, const String& octave, float volume);
        void update_note_debounce(const String& note, double delta);
        void update_status_bar(Control* bar_container, float value);
        void update_visuals();
        void render_note_prompt();
        bool should_handle_note(int current_midi, String current_octave);
        void on_note_detected(String note, String octave, int midi);
        
        Vector2 get_parent_scale(Node* node);
    };

}

#endif