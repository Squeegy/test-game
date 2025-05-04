#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H

#include <godot_cpp/classes/node.hpp>
#include <portaudio.h>

namespace godot {

class AudioInput : public Node {
    GDCLASS(AudioInput, Node);

private:
    PaStream *stream = nullptr;
    float current_volume = 0.0f;
    const int SAMPLE_RATE = 44100;
    const int BUFFER_SIZE = 2048;
    float *pitch_buffer = nullptr;
    int buffer_pos = 0;
    String current_note = "";
    String current_octave = "";

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

    static int pa_callback(const void *input,
                           void *output,
                           unsigned long frameCount,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData);

    std::pair<String, String> frequency_to_note_name(float freq);
    void initialize_stream(int device_index, int channel_index = 0);
    void update_note_and_octave(float freq);
    float detect_pitch_autocorrelation(const float* buffer, int size, int sample_rate);
};

}

#endif
