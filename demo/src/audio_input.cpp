#include "audio_input.h"
#include <cmath>
using namespace godot;

AudioInput::AudioInput() {
    UtilityFunctions::print("👋 AudioInput constructor called");
    pitch_buffer = new float[BUFFER_SIZE];
}
AudioInput::~AudioInput() {
    if (stream) Pa_CloseStream(stream);
    Pa_Terminate();
}

void AudioInput::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_volume"), &AudioInput::get_volume);
    ClassDB::bind_method(D_METHOD("initialize_stream"), &AudioInput::initialize_stream);
}

void AudioInput::_ready() {

    Pa_Initialize();

    PaStreamParameters input_params;
    input_params.device = 8;
    input_params.channelCount = 2;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = Pa_GetDeviceInfo(input_params.device)->defaultLowInputLatency;
    input_params.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenDefaultStream(&stream,
                    1,
                    0,
                    paFloat32,
                    44100,
                    256,
                    &AudioInput::pa_callback,
                    this);
    if (err != paNoError) {
        UtilityFunctions::print("Error opening stream: ", Pa_GetErrorText(err));
        return;
    }
    Pa_StartStream(stream);
}

void AudioInput::_process(double delta) {
    // Optionally print or update visuals here
    // UtilityFunctions::print("Volume: ", current_volume);
}

void AudioInput::list_input_devices() {
    Pa_Initialize();
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0) {
        UtilityFunctions::print("PortAudio error: ", Pa_GetErrorText(numDevices));
        return;
    }

    for (int i = 0; i < numDevices; ++i) {
        const PaDeviceInfo* info = Pa_GetDeviceInfo(i);
        if (info->maxInputChannels > 0) {
            UtilityFunctions::print("Device [", i, "]: ", info->name,
                                    " (", info->maxInputChannels, " ch)");
        }
    }
}

void AudioInput::initialize_stream(int device_index, int channel_index) {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
        stream = nullptr;
    }

    selected_device = device_index;
    selected_channel = channel_index;

    PaStreamParameters input_params;
    input_params.device = device_index;
    const PaDeviceInfo* info = Pa_GetDeviceInfo(device_index);
    active_channel_count = info->maxInputChannels;
    input_params.channelCount = active_channel_count;
    input_params.sampleFormat = paFloat32;
    input_params.suggestedLatency = info->defaultLowInputLatency;
    input_params.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        &stream,
        &input_params,
        nullptr,
        SAMPLE_RATE,
        BUFFER_SIZE,
        paNoFlag,
        &AudioInput::pa_callback,
        this
    );

    if (err != paNoError) {
        UtilityFunctions::print("Failed to open stream: ", Pa_GetErrorText(err));
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        UtilityFunctions::print("Failed to start stream: ", Pa_GetErrorText(err));
    } else {
        UtilityFunctions::print("✅ Stream started on device ", device_index, ", channel ", channel_index);
    }
}

float AudioInput::get_volume() const {
    return current_volume;
}

String AudioInput::get_current_note() const {
    return current_note;
}

String AudioInput::get_current_note_octave() const {
    return current_octave;
}

int AudioInput::pa_callback(const void *input,
                            void *,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo*,
                            PaStreamCallbackFlags,
                            void *userData) {
    auto *self = static_cast<AudioInput *>(userData);
    const float *input_buffer = (const float *)input;

    float sum = 0.0f;
    for (unsigned long i = 0; i < frameCount; i++) {
        sum += fabs(input_buffer[i]);
    }
    self->current_volume = sum / frameCount;

    for (unsigned long i = 0; i < frameCount; ++i) {
        if (self->buffer_pos < self->BUFFER_SIZE) {
            self->pitch_buffer[self->buffer_pos++] = input_buffer[i];  // Assuming mono
        }
    }

    if (self->buffer_pos >= self->BUFFER_SIZE) {
        self->buffer_pos = 0;
        float freq = self->detect_pitch_autocorrelation(self->pitch_buffer, self->BUFFER_SIZE, self->SAMPLE_RATE);
        self->update_note_and_octave(freq);
    }

    return paContinue;
}

void AudioInput::update_note_and_octave(float freq) {
    static int low_volume_counter = 0;
    const int low_volume_threshold = 10; // Number of consecutive low-volume frames before clearing

    if (freq > 0.0f) {
        auto [note, octave] = frequency_to_note_name(freq);
        current_note = note;
        current_octave = octave;
        low_volume_counter = 0; // Reset counter when a valid frequency is detected
    } else {
        low_volume_counter++;
        if (low_volume_counter >= low_volume_threshold) {
            current_note = "";
            current_octave = "";
        }
    }
}

float AudioInput::detect_pitch_autocorrelation(const float* buffer, int size, int sample_rate) {
    int max_lag = size / 2;
    int min_lag = sample_rate / 1000;  // Don't go below 1kHz

    float best_correlation = 0.0f;
    int best_lag = -1;

    // Apply a Hanning window (optional but improves quality)
    float* windowed = new float[size];
    for (int i = 0; i < size; ++i) {
        float w = 0.5f * (1.0f - std::cos(2.0f * Math_PI * i / (size - 1)));
        windowed[i] = buffer[i] * w;
    }

    // Simple energy gate — skip if signal is too quiet
    float energy = 0.0f;
    for (int i = 0; i < size; ++i) {
        energy += windowed[i] * windowed[i];
    }
    if (energy / size < 0.001f) {
        delete[] windowed;
        return -1.0f;
    }

    // Auto-correlation
    for (int lag = min_lag; lag < max_lag; ++lag) {
        float sum = 0.0f;
        for (int i = 0; i < size - lag; ++i) {
            sum += windowed[i] * windowed[i + lag];
        }

        if (sum > best_correlation) {
            best_correlation = sum;
            best_lag = lag;
        }
    }

    delete[] windowed;

    if (best_lag == -1) return -1.0f;

    float frequency = sample_rate / float(best_lag);
    return frequency;
}


std::pair<String, String> AudioInput::frequency_to_note_name(float freq) {
    static const char* note_names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };

    int midi = int(69 + 12 * log2(freq / 440.0f));
    int note_index = midi % 12;
    int octave = (midi / 12) - 1;

    return { String(note_names[note_index]), String::num(octave) };
}