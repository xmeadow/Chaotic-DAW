/*
 * VST2 compatibility headers for Chaotic-DAW
 * audioeffectx.h shim - provides types needed for VST2 plugin hosting.
 */

#ifndef __AUDIOEFFECTX__
#define __AUDIOEFFECTX__

#include "aeffectx.h"
#include "aeffEditor.h"

/* AudioEffectX stub - needed if any code references the class type.
 * This project uses CVSTHost for VST2 hosting and doesn't implement VST2
 * plugins, so we provide a minimal stub here. */
typedef VstIntPtr (VSTCALLBACK *audioMasterCallback)(AEffect* effect, VstInt32 opcode,
    VstInt32 index, VstIntPtr value, void* ptr, float opt);

class AudioEffect
{
public:
    AudioEffect(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams) {}
    virtual ~AudioEffect() {}
};

class AudioEffectX : public AudioEffect
{
public:
    AudioEffectX(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams)
        : AudioEffect(audioMaster, numPrograms, numParams) {}
    virtual ~AudioEffectX() {}
};

#endif /* __AUDIOEFFECTX__ */
