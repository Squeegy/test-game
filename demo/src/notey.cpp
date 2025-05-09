#include "notey.h"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Notey::Notey() {
    UtilityFunctions::print("👋 Notey constructor called");
}

Notey::~Notey() {
    // Destructor logic (if any)
}

void Notey::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &Notey::initialize);
    ClassDB::bind_method(D_METHOD("on_note_detected", "note", "octave", "midi"), &Notey::on_note_detected);
    // Binding methods for Godot
}

void Notey::_ready() {
    UtilityFunctions::print("Notey Ready!");
    audio_input = get_node<AudioInput>(NodePath("AudioInput"));
    if (!audio_input) {
        UtilityFunctions::print("AudioInput node not found!");
        return;
    }
    UtilityFunctions::print("AudioInput node: ", audio_input);
    note_label = get_node<Label>(NodePath("../NoteLabel"));
    mode_label = get_node<Label>(NodePath("../ModeLabel"));
    note_prompt_container = get_node<HBoxContainer>(NodePath("../NotePrompt"));
    set_mode(NoteyMode::FEED); // Set default mode to FEED
    set_process(true);
    call_deferred("initialize");
}

void Notey::initialize() {
    std::random_device rd;
    rng = std::mt19937(rd());

    initial_x = get_scale().x;
    initial_y = get_scale().y;
    Callable func = callable_mp(this, &Notey::on_note_detected);
    UtilityFunctions::print("Callable valid: ", func.is_valid());
    audio_input->connect("note_detected", callable_mp(this, &Notey::on_note_detected));
    audio_input->connect("note_stopped", callable_mp(this, &Notey::clear_note));
    UtilityFunctions::print("Signals on AudioInput: ", audio_input->get_class());


    hunger_bar = get_node<Control>("../Bars/Hunger Bar");
    hunger_bar->get_node<ColorRect>("Background")->set_size(Vector2(hunger_bar->get_size().x, hunger_bar->get_size().y));
    fitness_bar = get_node<Control>("../Bars/Fitness Bar");
    fitness_bar->get_node<ColorRect>("Background")->set_size(Vector2(fitness_bar->get_size().x, fitness_bar->get_size().y));
    mood_bar = get_node<Control>("../Bars/Mood Bar");
    mood_bar->get_node<ColorRect>("Background")->set_size(Vector2(mood_bar->get_size().x, mood_bar->get_size().y));
    update_status_bar(hunger_bar, hunger);
    update_status_bar(fitness_bar, fitness);
    update_status_bar(mood_bar, mood);
}

void Notey::render_note_prompt() {
    if (!note_prompt_container) return;
    // Clear existing labels in the container
    while (note_prompt_container->get_child_count() > 0) {
        Node *child = note_prompt_container->get_child(0);
        note_prompt_container->remove_child(child);
        child->queue_free();
    }

    for (int i = 0; i < target_sequence.size(); ++i) {
        Label *label = memnew(Label);
        label->set_text(target_sequence[i]);

        if (i == sequence_index) {
            label->add_theme_color_override("font_color", Color(1.0, 1.0, 0.0));  // yellow
            label->add_theme_font_size_override("font_size", 24);
        } else {
            label->add_theme_color_override("font_color", Color(1.0, 1.0, 1.0));  // white
        }

        note_prompt_container->add_child(label);
    }
}

void Notey::set_mode(NoteyMode new_mode) {
    mode = new_mode;
    switch (mode) {
        case NoteyMode::FEED:
            setup_feed_mode();
            break;
        case NoteyMode::JAM:
            setup_jam_mode();
            break;
        // Add more modes here as needed
        default:
            UtilityFunctions::print("Unknown mode!");
            break;
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
        note_label->set_text("Detected Note: " + consumed_note);
    }
}

void Notey::update_status_bar(Control* bar_container, float value) {
    if (!bar_container) return;

    // Clamp value between 0 and 1
    value = Math::clamp(value, 0.0f, 1.0f);

    auto* foreground = bar_container->get_node<ColorRect>("Foreground");
    if (!foreground) return;
    Vector2 bar_size = bar_container->get_size();
    float new_width = bar_size.x * value;

    foreground->set_anchors_preset(Control::PRESET_TOP_LEFT);  // Ensure it stretches from left
    foreground->set_size(Vector2(new_width, bar_size.y));

}

bool Notey::should_handle_note(int current_midi, String current_octave) {
    return (current_midi != consumed_midi || current_octave != consumed_octave);
}

void Notey::setup_feed_mode() {
    mode_label->set_text("Mode: Feed");
    generate_sequence();
    render_note_prompt();
}

void Notey::generate_sequence() {
    // Generate a sequence of notes for the feed mode
    sequence_index = 0;
    note_sequence.clear();
    target_sequence.clear();
    int min_length = 3;
    int max_length = 9;
    int length = min_length + int((max_length - min_length) * hunger); // Sequence Length between min inclusive and max exclusive
    std::uniform_int_distribution<int> dist(0, 11); // 12 notes in an octave
    for (int i = 0; i < length; ++i) {
        int random_index = dist(rng); //rng->randi_range(0, notes_size - 1);
        String note = AudioInput::get_note_names()[random_index];
        target_sequence.push_back(note);
    }
}

void Notey::setup_jam_mode() {
    mode_label->set_text("Mode: Jam");
    // Setup for jam mode (e.g., randomize notes, set up visuals)
    UtilityFunctions::print("Jam Mode: Ready to play!");
}

void Notey::on_note_detected(String note, String octave, int midi) {
    UtilityFunctions::print("Note detected: ", note, " (", octave, ", ", midi, ")");
    if (should_handle_note(midi, octave)) {
        handle_note_input(midi, octave, audio_input->get_volume());
        note_ready = false;
        consumed_note = note;
        consumed_octave = octave;
        consumed_midi = midi;
    } else {
        UtilityFunctions::print("Note already consumed!");
    }
}

void Notey::clear_note() {
    UtilityFunctions::print("Note cleared!");
    consumed_note = "";
    consumed_octave = "";
    consumed_midi = -1;
    note_ready = false;
}

void Notey::handle_note_input(int midi, const String& octave, float volume) {
    if (midi < 0 || midi >= 128) {
        UtilityFunctions::print("⚠️ Invalid MIDI value: ", midi);
        return;
    }

    switch(mode) {
        case NoteyMode::FEED:
            handle_feed(midi, octave, volume);
            break;
        case NoteyMode::JAM:
            handle_jam(midi, octave, volume);
            break;
        default:
            UtilityFunctions::print("Unknown mode!");
            break;
    }
    
    update_status_bar(fitness_bar, fitness);
}

void Notey::handle_feed(int midi, const String& octave, float volume) {
    // Handle feed logic here
    if (sequence_index >= target_sequence.size()) return;

    const String& expected = target_sequence[sequence_index];

    if (AudioInput::get_note_to_midi().count(expected) && midi % 12 == AudioInput::get_note_to_midi().at(expected)) {
        UtilityFunctions::print("Correct: ", AudioInput::get_note_names()[midi % 12], " (", midi, ")");
        sequence_index++;

        if (sequence_index >= target_sequence.size()) {
            UtilityFunctions::print("Sequence complete!");
            hunger += 0.1f;  // ✅ Feed Notey only once
            generate_sequence();  // Generate a new sequence
            // Optionally trigger next round or mood change
        }
        render_note_prompt();
    } else {
        UtilityFunctions::print("Wrong! Expected: ", expected, ", got: ", AudioInput::get_note_names()[midi % 12]);
        hunger -= 0.001f;     // Small penalty
    }
    hunger = Math::clamp(hunger, 0.0f, 1.0f);
    update_status_bar(hunger_bar, hunger);
}

void Notey::handle_jam(int midi, const String& octave, float volume) {
    // Handle jam logic here
    UtilityFunctions::print("Jamming with: ", AudioInput::get_note_names()[midi % 12], " (", midi, ")");
    // Add your jam logic here
    update_status_bar(mood_bar, mood);
}