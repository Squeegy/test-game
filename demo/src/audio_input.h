#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <map>
#include <tuple>
#include <portaudio.h>

namespace godot {

class AudioInput : public Node {
    GDCLASS(AudioInput, Node);

private:
    PaStream *stream = nullptr;
    float *pitch_buffer = nullptr;
    int buffer_pos = 0;
    const int SAMPLE_RATE = 44100;
    const int BUFFER_SIZE = 8192;
    int selected_device = -1;
    int selected_channel = 0;
    int active_channel_count = 0;
      
    OptionButton *device_selector = nullptr;
    OptionButton *channel_selector = nullptr;
    Button *start_button = nullptr;
    Dictionary device_channel_counts;  // maps device index to channel count

    float current_volume = 0.0f;
    String current_note = "";
    String current_octave = "";
    int current_midi = -1;
    bool note_pending = false;
    bool silence_pending = false;

    String last_note = "";
    float debounce_timer = 0.0f;
    const float debounce_threshold = 0.1f; // adjust as needed
    bool note_ready = false;

protected:
    static void _bind_methods();

public:
    AudioInput();
    ~AudioInput();

    void _ready();
    void _process(double delta);

    float get_volume() const;
    String get_current_note() const;
    String get_current_note_octave() const;
    int get_current_midi() const;

    static int pa_callback(const void *input,
                           void *output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData);

    std::tuple<String, String, int> frequency_to_note_name(float freq);
    void initialize_stream(int device_index, int channel_index = 0);
    void list_input_devices();
    void update_note_and_octave(float freq);
    float detect_pitch_autocorrelation(const float* buffer, int size, int sample_rate);
    void _update_channel_selector(int device_index);
    void _on_device_selected(int index);
    void _on_start_pressed();
    void emit_note();
    void emit_silence();
    static const String* get_note_names();
    static const std::map<String, int>& get_note_to_midi();
    static const std::map<String, String>& get_enharmonic_map();
};

}

#endif
