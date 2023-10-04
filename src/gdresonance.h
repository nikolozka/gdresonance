#ifndef GDResonance_H
#define GDResonance_H

#include <godot_cpp/classes/audio_stream_player.hpp>
//#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/audio_effect_instance.hpp>
#include <godot_cpp/classes/audio_effect.hpp>
#include <godot_cpp/classes/node3d.hpp>

#include "api/resonance_audio_api.h"
#include "platforms/common/room_properties.h"

class GDResonanceEffect;

class GDResonance : public godot::AudioEffectInstance {
    GDCLASS(GDResonance, godot::AudioEffectInstance)

    friend class GDResonanceEffect;
	godot::Ref<GDResonanceEffect> base;

//private:

protected:
    static void _bind_methods();

public:
    GDResonance();
    ~GDResonance();

    void _process(godot::AudioFrame *src_buffer, godot::AudioFrame *dst_buffer, int32_t frame_count);
    //void _process();

    // Initializes the ResonanceAudio system with Unity audio engine settings.
    void Initialize(int sample_rate, size_t num_channels, size_t frames_per_buffer);

    // Shuts down the ResonanceAudio system.
    void Shutdown();

    // Processes the next output buffer and stores the processed buffer in |output|.
    // This method must be called from the audio thread.
    void ProcessListener(size_t num_frames, float* output);

    // Updates the listener's position and rotation.
    void SetListenerTransform(float px, float py, float pz, float qx, float qy,
                            float qz, float qw);

    // Creates an ambiX format soundfield and connects it to the audio manager.
    vraudio::ResonanceAudioApi::SourceId CreateSoundfield(int num_channels);

    // Creates a sound object sub-graph and connects it to the audio manager.
    vraudio::ResonanceAudioApi::SourceId CreateSoundObject(vraudio::RenderingMode rendering_mode);

    // Disconnects the source with |id| from the pipeline and releases its
    // resources.
    void DestroySource(vraudio::ResonanceAudioApi::SourceId id);

    // Passes the next input buffer of the source to the system. This method must be
    // called from the audio thread.
    void ProcessSource(vraudio::ResonanceAudioApi::SourceId id, size_t num_channels,
                    size_t num_frames, float* input);

    // Updates the directivity parameters of the source.
    void SetSourceDirectivity(vraudio::ResonanceAudioApi::SourceId id, float alpha,
                            float order);

    // Sets the computed distance attenuation of a source.
    void SetSourceDistanceAttenuation(vraudio::ResonanceAudioApi::SourceId id,
                                    float distance_attenuation);

    // Updates the gain of the source.
    void SetSourceGain(vraudio::ResonanceAudioApi::SourceId id, float gain);

    // Updates the listener directivity parameters of the source.
    void SetSourceListenerDirectivity(vraudio::ResonanceAudioApi::SourceId id, float alpha,
                                    float order);

    // Updates the near field effect gain for the source.
    void SetSourceNearFieldEffectGain(vraudio::ResonanceAudioApi::SourceId id,
                                    float near_field_effect_gain);

    // Updates the occlusion intensity of the source.
    void SetSourceOcclusionIntensity(vraudio::ResonanceAudioApi::SourceId id,
                                    float intensity);

    // Sets the room effects gain for the source.
    void SetSourceRoomEffectsGain(vraudio::ResonanceAudioApi::SourceId id,
                                float room_effects_gain);

    // Updates the spread of the source.
    void SetSourceSpread(vraudio::ResonanceAudioApi::SourceId id, float spread_deg);

    // Updates the position, rotation and scale of the source.
    void SetSourceTransform(vraudio::ResonanceAudioApi::SourceId id, float px, float py,
                            float pz, float qx, float qy, float qz, float qw);

    // Updates the listener's master gain.
    void EXPORT_API SetListenerGain(float gain);

    // Updates the stereo speaker mode.
    void EXPORT_API SetListenerStereoSpeakerMode(bool enable_stereo_speaker_mode);

    // Updates the properties of the room.
    void EXPORT_API SetRoomProperties(vraudio::RoomProperties* room_properties,
                                    float* rt60s);

    void UpdatePosition(float x, float y, float z);

};

class GDResonanceEffect : public godot::AudioEffect {
    GDCLASS(GDResonanceEffect, godot::AudioEffect)

   	friend class GDResonance;

    float px;
    float py;
    float pz;

    protected:
        static void _bind_methods();

    public:
        GDResonanceEffect();
        ~GDResonanceEffect();

        void SetX(float x);
        void SetY(float y);
        void SetZ(float z);

        float GetX() const;
        float GetY() const;
        float GetZ() const;

        godot::Ref<godot::AudioEffectInstance> _instantiate();
};

//Listener

class GDResonanceListener : public godot::Node3D {
    GDCLASS(GDResonanceListener, godot::Node3D)

private:
    double resonance_audio;

protected:
    static void _bind_methods();

public:
    GDResonanceListener();
    ~GDResonanceListener();

    void _process(double delta);
};

class GDResonanceSource : public godot::Node3D {
    GDCLASS(GDResonanceSource, godot::Node3D)

private:
    double resonance_audio;

protected:
    static void _bind_methods();

public:
    GDResonanceSource();
    ~GDResonanceSource();

    void _process(double delta);
};


class GDResonanceRoom : public godot::Node3D {
    GDCLASS(GDResonanceRoom, godot::Node3D)

private:
    double resonance_audio;

protected:
    static void _bind_methods();

public:
    GDResonanceRoom();
    ~GDResonanceRoom();

    void _process(double delta);
};


class GDResonanceSoundfield : public godot::Node3D {
    GDCLASS(GDResonanceSoundfield, godot::Node3D)

private:
    double resonance_audio;

protected:
    static void _bind_methods();

public:
    GDResonanceSoundfield();
    ~GDResonanceSoundfield();

    void _process(double delta);
};

#endif