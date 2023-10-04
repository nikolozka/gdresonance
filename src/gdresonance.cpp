#include "gdresonance.h"
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/audio_stream_generator_playback.hpp>
#include <godot_cpp/classes/audio_stream_playback.hpp>


#include "base/audio_buffer.h"
#include "base/constants_and_types.h"
#include "base/logging.h"
#include "base/misc_math.h"
#include "graph/resonance_audio_api_impl.h"
#include "platforms/common/room_effects_utils.h"

using namespace godot;

//Resonance
const size_t kNumOutputChannels = 2;
const size_t kSampleRate = 44100;

struct ResonanceAudioSystem {
  ResonanceAudioSystem(int sample_rate, size_t num_channels, size_t frames_per_buffer) : api(vraudio::CreateResonanceAudioApi(num_channels, frames_per_buffer,sample_rate)) {}

  // ResonanceAudio API instance to communicate with the internal system.
  std::unique_ptr<vraudio::ResonanceAudioApi> api;

  // Default room properties, which effectively disable the room effects.
  vraudio::ReflectionProperties null_reflection_properties;
  vraudio::ReverbProperties null_reverb_properties;
};

static std::shared_ptr<ResonanceAudioSystem> resonance_audio = nullptr;

vraudio::ResonanceAudioApi::SourceId mId1 = 0;
vraudio::ResonanceAudioApi::SourceId mId2 = 1;


const int nFrames = 512;

float inp1[nFrames];
float inp2[nFrames];
float out[nFrames*kNumOutputChannels];

float px1 = 0.0f;
float py1 = 0.0f;
float pz1 = 0.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ResonanceEffect
////////////////////////////////////////////////////////////////////////////////////////////////////////////

Ref<AudioEffectInstance> GDResonanceEffect::_instantiate(){
  Ref<GDResonance> ins;
	ins.instantiate();
	ins->base = Ref<GDResonance>(this);
	return ins;
}

void GDResonanceEffect::_bind_methods() {

  ClassDB::bind_method(D_METHOD("set_x", "x"), &GDResonanceEffect::SetX);
  ClassDB::bind_method(D_METHOD("set_y", "y"), &GDResonanceEffect::SetY);
  ClassDB::bind_method(D_METHOD("set_z", "z"), &GDResonanceEffect::SetZ);

	ClassDB::bind_method(D_METHOD("get_x"), &GDResonanceEffect::GetX);
	ClassDB::bind_method(D_METHOD("get_y"), &GDResonanceEffect::GetY);
	ClassDB::bind_method(D_METHOD("get_z"), &GDResonanceEffect::GetZ);

  ClassDB::add_property("GDResonanceEffect", PropertyInfo(Variant::FLOAT, "x", PROPERTY_HINT_RANGE, "-100.0,100.0,suffix:m"), "set_x", "get_x");
  ClassDB::add_property("GDResonanceEffect", PropertyInfo(Variant::FLOAT, "y", PROPERTY_HINT_RANGE, "-100.0,100.0,suffix:m"), "set_y", "get_y");
  ClassDB::add_property("GDResonanceEffect", PropertyInfo(Variant::FLOAT, "z", PROPERTY_HINT_RANGE, "-100.0,100.0,suffix:m"), "set_z", "get_z");
}

GDResonanceEffect::GDResonanceEffect() {
    // Initialize any variables here.
    px=0.0f;
    py=0.0f;
    pz=0.0f;
}

GDResonanceEffect::~GDResonanceEffect() {
    // Add your cleanup here.
}

void GDResonanceEffect::SetX(float x){
  px=x;
}

void GDResonanceEffect::SetY(float y){
  py=y;
}

void GDResonanceEffect::SetZ(float z){
  pz=z;
}

float GDResonanceEffect::GetX() const {
  return px;
}

float GDResonanceEffect::GetY() const {
  return py;
}

float GDResonanceEffect::GetZ() const {
  return pz;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resonance
////////////////////////////////////////////////////////////////////////////////////////////////////////////



void GDResonance::Initialize(int sample_rate, size_t num_channels, size_t frames_per_buffer) {
  CHECK_GE(sample_rate, 0);
  CHECK_EQ(num_channels, kNumOutputChannels);
  CHECK_GE(frames_per_buffer, 0);
  resonance_audio = std::make_shared<ResonanceAudioSystem>(sample_rate, num_channels, frames_per_buffer);
}

void GDResonance::Shutdown() { resonance_audio.reset(); }

void GDResonance::ProcessListener(size_t num_frames, float* output) {
  CHECK(output != nullptr);

  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy == nullptr) {
    return;
  }

  if (!resonance_audio_copy->api->FillInterleavedOutputBuffer(
          kNumOutputChannels, num_frames, output)) {
    // No valid output was rendered, fill the output buffer with zeros.
    const size_t buffer_size_samples = kNumOutputChannels * num_frames;
    CHECK(!vraudio::DoesIntegerMultiplicationOverflow<size_t>(
        kNumOutputChannels, num_frames, buffer_size_samples));

    std::fill(output, output + buffer_size_samples, 0.0f);
  }

}

void GDResonance::SetListenerGain(float gain) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetMasterVolume(gain);
  }
}

void GDResonance::SetListenerStereoSpeakerMode(bool enable_stereo_speaker_mode) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetStereoSpeakerMode(enable_stereo_speaker_mode);
  }
}

void GDResonance::SetListenerTransform(float px, float py, float pz, float qx, float qy,
                          float qz, float qw) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetHeadPosition(px, py, pz);
    resonance_audio_copy->api->SetHeadRotation(qx, qy, qz, qw);
  }
}

vraudio::ResonanceAudioApi::SourceId GDResonance::CreateSoundfield(int num_channels) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    return resonance_audio_copy->api->CreateAmbisonicSource(num_channels);
  }
  return vraudio::ResonanceAudioApi::kInvalidSourceId;
}

vraudio::ResonanceAudioApi::SourceId GDResonance::CreateSoundObject(vraudio::RenderingMode rendering_mode) {
  vraudio::SourceId id = vraudio::ResonanceAudioApi::kInvalidSourceId;
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    id = resonance_audio_copy->api->CreateSoundObjectSource(rendering_mode);
    resonance_audio_copy->api->SetSourceDistanceModel(
        id, vraudio::DistanceRolloffModel::kNone, 0.0f, 0.0f);
  }
  return id;
}

void GDResonance::DestroySource(vraudio::ResonanceAudioApi::SourceId id) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->DestroySource(id);
  }
}

void GDResonance::ProcessSource(vraudio::ResonanceAudioApi::SourceId id, size_t num_channels,
                   size_t num_frames, float* input) {
  CHECK(input != nullptr);

  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetInterleavedBuffer(id, input, num_channels,
                                                    num_frames);
  }
}

void GDResonance::SetSourceDirectivity(vraudio::ResonanceAudioApi::SourceId id, float alpha,
                          float order) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSoundObjectDirectivity(id, alpha, order);
  }
}

void GDResonance::SetSourceDistanceAttenuation(vraudio::ResonanceAudioApi::SourceId id,
                                  float distance_attenuation) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSourceDistanceAttenuation(
        id, distance_attenuation);
  }
}

void GDResonance::SetSourceGain(vraudio::ResonanceAudioApi::SourceId id, float gain) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSourceVolume(id, gain);
  }
}

void GDResonance::SetSourceListenerDirectivity(vraudio::ResonanceAudioApi::SourceId id, float alpha,
                                  float order) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSoundObjectListenerDirectivity(id, alpha,
                                                                 order);
  }
}

void GDResonance::SetSourceNearFieldEffectGain(vraudio::ResonanceAudioApi::SourceId id,
                                  float near_field_effect_gain) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSoundObjectNearFieldEffectGain(
        id, near_field_effect_gain);
  }
}

void GDResonance::SetSourceOcclusionIntensity(vraudio::ResonanceAudioApi::SourceId id,
                                 float intensity) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSoundObjectOcclusionIntensity(id, intensity);
  }
}

void GDResonance::SetSourceRoomEffectsGain(vraudio::ResonanceAudioApi::SourceId id,
                              float room_effects_gain) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSourceRoomEffectsGain(id, room_effects_gain);
  }
}

void GDResonance::SetSourceSpread(int id, float spread_deg) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSoundObjectSpread(id, spread_deg);
  }
}

void GDResonance::SetSourceTransform(int id, float px, float py, float pz, float qx,
                        float qy, float qz, float qw) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy != nullptr) {
    resonance_audio_copy->api->SetSourcePosition(id, px, py, pz);
    resonance_audio_copy->api->SetSourceRotation(id, qx, qy, qz, qw);
  }
}

void GDResonance::SetRoomProperties(vraudio::RoomProperties* room_properties, float* rt60s) {
  auto resonance_audio_copy = resonance_audio;
  if (resonance_audio_copy == nullptr) {
    return;
  }
  if (room_properties == nullptr) {
    resonance_audio_copy->api->SetReflectionProperties(
        resonance_audio_copy->null_reflection_properties);
    resonance_audio_copy->api->SetReverbProperties(
        resonance_audio_copy->null_reverb_properties);
    return;
  }

  const auto reflection_properties =
      ComputeReflectionProperties(*room_properties);
  resonance_audio_copy->api->SetReflectionProperties(reflection_properties);
  const auto reverb_properties =
      (rt60s == nullptr)
          ? ComputeReverbProperties(*room_properties)
          : vraudio::ComputeReverbPropertiesFromRT60s(
                rt60s, room_properties->reverb_brightness,
                room_properties->reverb_time, room_properties->reverb_gain);
  resonance_audio_copy->api->SetReverbProperties(reverb_properties);
}

void GDResonance::UpdatePosition(float x, float y, float z){
  SetSourceTransform(mId1, x, y, z, 1.0f, 0.0f, 0.0f, 0.0f);
}

void GDResonance::_bind_methods() {
}

GDResonance::GDResonance() {
    // Initialize any variables here.
    Initialize(kSampleRate,2,nFrames);
    SetListenerGain(1.0f);
    SetListenerTransform(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    mId1 = CreateSoundObject(vraudio::RenderingMode::kBinauralHighQuality);
    //mId2 = CreateSoundObject(vraudio::RenderingMode::kBinauralHighQuality);
}

GDResonance::~GDResonance() {
    // Add your cleanup here.
    Shutdown();
}

/*void GDResonance::_process() {

}*/ 
float angle1 = 0.0f;
float angle2 = 0.0f;

void GDResonance::_process(godot::AudioFrame *src_buffer, godot::AudioFrame *dst_buffer, int32_t frame_count) {

  px1 = 10.0f * cos(angle1);
  //px1 = base->py;
 
  py1 = 10.0f * sin(angle1);
  //py1 = base->py;
  
  pz1 = 10.0f * sin(angle1);
  //pz1 = base->pz;

  /*float px = base->px;
  float py = base->py;
  float pz = base->pz;*/



  //UpdatePosition(px1, py1, pz1);
  UpdatePosition(px, py, pz);

  //SetSourceTransform(mId1, px, py, pz, 1.0f, 0.0f, 0.0f, 0.0f); //doesn't want to do this ;(

  for (int i = 0; i < frame_count; i++) {
		inp1[i] = src_buffer[i].left;
  }

  ProcessSource(mId1, 1, frame_count, inp1);
  ProcessListener(frame_count, out);
  

  for (int i = 0; i < frame_count*2; i+=2) {
    dst_buffer[i/2].left = out[i];
		dst_buffer[i/2].right = out[i+1];
  }

  angle1 += 0.01;
  if(angle1 > 360.0f){
    angle1 = 0.0f;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Listener
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GDResonanceListener::_bind_methods() {
}

GDResonanceListener::GDResonanceListener() {
    // Initialize any variables here.
    resonance_audio = 0.0;
}

GDResonanceListener::~GDResonanceListener() {
    // Add your cleanup here.
}

void GDResonanceListener::_process(double delta) {

} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Source
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GDResonanceSource::_bind_methods() {
}

GDResonanceSource::GDResonanceSource() {
    // Initialize any variables here.
    resonance_audio = 0.0;
}

GDResonanceSource::~GDResonanceSource() {
    // Add your cleanup here.
}

void GDResonanceSource::_process(double delta) {

} 

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Room
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GDResonanceRoom::_bind_methods() {
}

GDResonanceRoom::GDResonanceRoom() {
    // Initialize any variables here.
    resonance_audio = 0.0;
}

GDResonanceRoom::~GDResonanceRoom() {
    // Add your cleanup here.
}

void GDResonanceRoom::_process(double delta) {

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Soundfield
////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GDResonanceSoundfield::_bind_methods() {
}

GDResonanceSoundfield::GDResonanceSoundfield() {
    // Initialize any variables here.
    resonance_audio = 0.0;
}

GDResonanceSoundfield::~GDResonanceSoundfield() {
    // Add your cleanup here.
}

void GDResonanceSoundfield::_process(double delta) {

}