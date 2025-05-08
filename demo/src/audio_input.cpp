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
    ClassDB::bind_method(D_METHOD("_on_device_selected", "index"), &AudioInput::_on_device_selected);
    ClassDB::bind_method(D_METHOD("_on_start_pressed"), &AudioInput::_on_start_pressed);
    ClassDB::bind_method(D_METHOD("emit_note"), &AudioInput::emit_note);
    ADD_SIGNAL(MethodInfo("note_detected",
        PropertyInfo(Variant::STRING, "note"),
        PropertyInfo(Variant::STRING, "octave"),
        PropertyInfo(Variant::INT, "midi")));
}

void AudioInput::_ready() {

    Pa_Initialize();

    // === CanvasLayer with VBox ===
    auto *canvas = memnew(CanvasLayer);
    add_child(canvas);

    auto *vbox = memnew(VBoxContainer);
    canvas->add_child(vbox);

    // === Device Selector ===
    device_selector = memnew(OptionButton);
    device_selector->set_name("DeviceSelector");
    vbox->add_child(device_selector);

    // === Channel Selector ===
    channel_selector = memnew(OptionButton);
    channel_selector->set_name("ChannelSelector");
    vbox->add_child(channel_selector);

    // === Start Button ===
    start_button = memnew(Button);
    start_button->set_text("Start Stream");
    vbox->add_child(start_button);

    // === Enumerate Devices ===
    int num_devices = Pa_GetDeviceCount();
    for (int i = 0; i < num_devices; ++i) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
        if (info && info->maxInputChannels > 0) {
            String label = String(info->name) + " (" + String::num(info->maxInputChannels) + " ch)";
            device_selector->add_item(label, i); // store device index as metadata
            device_channel_counts[i] = info->maxInputChannels;
        }
    }

    // === Default channel list for first device ===
    if (device_selector->get_item_count() > 0) {
        _update_channel_selector(device_selector->get_selected_id());
    }

    // === Connect Signals ===
    device_selector->connect("item_selected", Callable(this, "_on_device_selected"));
    start_button->connect("pressed", Callable(this, "_on_start_pressed"));

    UtilityFunctions::print("🎛️ AudioInput UI ready");
}

void AudioInput::_process(double delta) {
    // Optionally print or update visuals here
    // UtilityFunctions::print("Volume: ", current_volume);
}

void AudioInput::_update_channel_selector(int device_index) {
    if (!channel_selector) return;

    channel_selector->clear();
    int num_channels = device_channel_counts.get(device_index, 0);

    for (int i = 0; i < num_channels; ++i) {
        String label = "Channel " + String::num(i);
        channel_selector->add_item(label, i);
    }

    channel_selector->select(0);
}

void AudioInput::_on_device_selected(int index) {
    int device_index = device_selector->get_item_id(index);
    _update_channel_selector(device_index);
}

void AudioInput::_on_start_pressed() {
    int device_index = device_selector->get_selected_id();  // stored metadata
    int channel_index = channel_selector->get_selected_id();

    UtilityFunctions::print("🔈 Starting stream on device ", device_index, ", channel ", channel_index);
    initialize_stream(device_index, channel_index);
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
    UtilityFunctions::print("Initializing stream on device ", device_index, ", channel ", channel_index);
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

int AudioInput::get_current_midi() const {
    return current_midi;
}

int AudioInput::pa_callback(const void* input,
                                void*,
                                unsigned long frameCount,
                                const PaStreamCallbackTimeInfo*,
                                PaStreamCallbackFlags,
                                void* userData
                            ) {
    auto* self = static_cast<AudioInput*>(userData);
    const float* input_buffer = (const float*)input;

    float sum = 0.0f;
    for (unsigned long i = 0; i < frameCount; ++i) {
        int index = i * self->active_channel_count + self->selected_channel;
        float sample = input_buffer[index];
        sum += fabs(sample);

        if (self->buffer_pos < self->BUFFER_SIZE) {
            self->pitch_buffer[self->buffer_pos++] = sample;
        }
    }

    self->current_volume = sum / frameCount;

    if (self->buffer_pos >= self->BUFFER_SIZE) {
        self->buffer_pos = 0;
        float freq = self->detect_pitch_autocorrelation(self->pitch_buffer, self->BUFFER_SIZE, self->SAMPLE_RATE);
        self->update_note_and_octave(freq);
    }

    return paContinue;
}

void AudioInput::update_note_and_octave(float freq) {
    static int low_volume_counter = 0;
    const int low_volume_threshold = 8; // Number of consecutive low-volume frames before clearing

    if (freq > 0.0f) {
        auto [note, octave, midi] = frequency_to_note_name(freq);
        current_note = note;
        current_octave = octave;
        current_midi = midi;
        note_pending = true;
        call_deferred("emit_note");
        low_volume_counter = 0; // Reset counter when a valid frequency is detected
    } else {
        low_volume_counter++;
        if (low_volume_counter >= low_volume_threshold) {
            current_note = "";
            current_octave = "";
            current_midi = -1;
        }
    }
}

void AudioInput::emit_note() {
    if (!note_pending) return;
    UtilityFunctions::print("Emitting note: ", current_note, " (", current_octave, ", ", current_midi, ")");
    emit_signal("note_detected", current_note, current_octave, current_midi);
    note_pending = false;
}

float AudioInput::detect_pitch_autocorrelation(const float* buffer, int size, int sample_rate) {
    int max_lag = size - 1; // maximum lag for autocorrelation
    // Let's support down to E1 (~41.2 Hz), so maybe 30 Hz to be safe
    float lowest_expected_hz = 1000.0f;
    int min_lag = int(sample_rate / 1000.0f);      // default
    int safe_min_lag = int(sample_rate / lowest_expected_hz);  // ~1470 for 44100 Hz
    min_lag = std::max(min_lag, safe_min_lag); // ensure we don't go below this

    float best_correlation = 0.0f;
    int best_lag = -1;

    // === Apply a simple 6 dB/oct low-pass filter ===
    float cutoff_hz = 1000.0f;
    float RC = 1.0f / (2.0f * Math_PI * cutoff_hz);
    float dt = 1.0f / sample_rate;
    float alpha = dt / (RC + dt);

    float* filtered = new float[size];
    filtered[0] = buffer[0];  // seed the filter

    for (int i = 1; i < size; ++i) {
        filtered[i] = filtered[i - 1] + alpha * (buffer[i] - filtered[i - 1]);
    }

    // Apply a Hanning window (optional but improves quality)
    float* windowed = new float[size];
    for (int i = 0; i < size; ++i) {
        float w = 0.5f * (1.0f - std::cos(2.0f * Math_PI * i / (size - 1)));
        windowed[i] = filtered[i] * w;
    }

    // Simple energy gate — skip if signal is too quiet
    float energy = 0.0f;
    for (int i = 0; i < size; ++i) {
        energy += windowed[i] * windowed[i];
    }
    if (energy / size < 0.00001f) {
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
    delete[] filtered;

    if (best_lag == -1) return -1.0f;

    float frequency = sample_rate / float(best_lag);
    return frequency;
}


std::tuple<String, String, int> AudioInput::frequency_to_note_name(float freq) {
    int midi = int(69 + 12 * log2(freq / 440.0f));
    int note_index = midi % 12;
    int octave = (midi / 12) - 1;

    return std::make_tuple(String(AudioInput::get_note_names()[note_index]), String::num(octave), midi);
}

const String* AudioInput::get_note_names() {
    static const String NOTE_NAMES[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    return NOTE_NAMES;
}

const std::map<String, int>& AudioInput::get_note_to_midi() {
    static const std::map<String, int> NOTE_TO_MIDI = {
        // Sharps
        { "C", 0 }, { "C#", 1 }, { "D", 2 }, { "D#", 3 }, { "E", 4 },
        { "F", 5 }, { "F#", 6 }, { "G", 7 }, { "G#", 8 }, { "A", 9 },
        { "A#", 10 }, { "B", 11 },
    
        // Flats (enharmonic equivalents)
        { "Db", 1 }, { "Eb", 3 }, { "Gb", 6 }, { "Ab", 8 }, { "Bb", 10 }
    };
    return NOTE_TO_MIDI;
}
const std::map<String, String>& AudioInput::get_enharmonic_map() {
    static const std::map<String, String> ENHARMONIC_MAP = {
        {"C#", "Db"},
        {"D#", "Eb"},
        {"F#", "Gb"},
        {"G#", "Ab"},
        {"A#", "Bb"}
    };
    return ENHARMONIC_MAP;
}