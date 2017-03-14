#include "SoundSource.hpp"

#include "../Entity/Entity.hpp"
#include "../Hymn.hpp"
#include "../Audio/VorbisFile.hpp"

using namespace Component;

SoundSource::SoundSource(Entity* entity) : SuperComponent(entity) {
    
}

SoundSource::~SoundSource() {
    
}

Json::Value SoundSource::Save() const {
    Json::Value component;
    
    if (vorbisFile != nullptr)
        component["sound"] = vorbisFile->name;
    
    component["gain"] = gain;
    component["loop"] = loop;
    component["ignoreLowpass"] = ignoreLowpass;
    return component;
}

void SoundSource::Load(const Json::Value& node) {
    std::string name = node.get("sound", "").asString();
    for (Audio::VorbisFile* sound : Hymn().sounds) {
        if (sound->name == name)
            vorbisFile = sound;
    }
    
    gain = node.get("gain", 1.f).asFloat();
    loop = node.get("loop", false).asBool();
    ignoreLowpass = node.get("ignoreLowpass", false).asBool();
}

void SoundSource::Play() {
    playing = true;
}

void SoundSource::Pause() {
    playing = false;
}

void SoundSource::Stop() {
    playing = false;
    position = 0;
}

float SoundSource::GetSample(int count) {
    const float* data = vorbisFile->GetData();
    
    float sample = 0.f;
    int samplePos = position;
    for (int i = 0; i < count; ++i) {
        sample += data[samplePos];
        
        samplePos -= vorbisFile->IsStereo() ? 2 : 1;
        if (samplePos < 0)
            samplePos += vorbisFile->GetLength();
    }
    sample /= static_cast<float>(count);
    
    position = (position + 1) % (vorbisFile->GetLength() * (vorbisFile->IsStereo() ? 2 : 1));
    return sample;
}
