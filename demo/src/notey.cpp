#include "notey.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Notey::Notey() {
    UtilityFunctions::print("👋 Notey constructor called");
    initial_x = get_scale().x;
    initial_y = get_scale().y;
}

Notey::~Notey() {
    // Destructor logic (if any)
}

void Notey::_bind_methods() {
    // Binding methods for Godot
}

void Notey::_ready() {
    UtilityFunctions::print("Notey Ready!");
    audio_input = get_node<AudioInput>(NodePath("AudioInput"));
    UtilityFunctions::print("AudioInput node: ", audio_input);
    audio_input = Object::cast_to<AudioInput>(audio_input);
    note_label = get_node<Label>(NodePath("NoteLabel"));
    set_mode(NoteyMode::FEED); // Set default mode to FEED
    set_process(true);
}

void Notey::set_mode(NoteyMode new_mode) {
    mode = new_mode;
    if (mode == NoteyMode::FEED) {
        setup_feed_mode();
    } else if (mode == NoteyMode::JAM) {
        setup_jam_mode();
    }
}

void Notey::_process(double delta) {
    time_passed += delta;
    // Example logic: oscillate the scale of the sprite
    float x_scale = initial_x + Math::sin(time_passed*1.5) * .02;
    float y_scale = initial_y + Math::sin(time_passed*1.5 + Math_PI) * .01;
    set_scale(Vector2(x_scale, y_scale));
    if (audio_input && audio_input->get_volume() > 0.05f) {
        // e.g., change sprite, animation, or mood
        UtilityFunctions::print("Notey is happy!");
    }
    if (note_label) {
        note_label->set_text("Detected Note: " + audio_input->get_current_note());
    }
    // Update debounce state every frame
    String current_note = audio_input->get_current_note();
    update_note_debounce(current_note, delta);


    if (note_ready) {
        handle_note_input(current_note, audio_input->get_volume());
        note_ready = false;  // Reset readiness until next stable note
        last_note = "";  // Reset last note to avoid immediate re-triggering
    }
}

void Notey::update_note_debounce(const String& note, double delta) {
    if (note.is_empty()) {
        note_ready = false;
        return;
    }
    if (note != last_note) {
        debounce_timer = 0.0;
        last_note = note;
        note_ready = false;
        return;
    }

    debounce_timer += delta;
    if (debounce_timer >= debounce_threshold) {
        note_ready = true;
    }
}

void Notey::setup_feed_mode() {
    target_sequence = { "C", "E", "G" }; // eventually randomly generated
    sequence_index = 0;
}

void Notey::setup_jam_mode() {
    // Setup for jam mode (e.g., randomize notes, set up visuals)
    UtilityFunctions::print("Jam Mode: Ready to play!");
}

void Notey::handle_note_input(const String& note, float volume) {
    if (mode != NoteyMode::FEED) return;

    if (sequence_index >= target_sequence.size()) return;

    const String& expected = target_sequence[sequence_index];

    if (note == expected) {
        UtilityFunctions::print("Correct: ", note);
        sequence_index++;

        if (sequence_index >= target_sequence.size()) {
            UtilityFunctions::print("Sequence complete!");
            hunger += 0.3f;  // ✅ Feed Notey only once
            sequence_index = 0;  // ready for a new sequence
            // Optionally trigger next round or mood change
        }
    } else {
        UtilityFunctions::print("Wrong! Expected: ", expected, ", got: ", note);
        hunger -= 0.05f;     // Small penalty
    }
}