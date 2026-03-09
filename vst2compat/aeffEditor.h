/*
 * VST2 compatibility headers for Chaotic-DAW
 * AEffEditor and ERect definitions.
 */

#ifndef __AEFFEDITOR__
#define __AEFFEDITOR__

#include "aeffectx.h"

/* Editor rectangle */
struct ERect
{
    VstInt16 top;
    VstInt16 left;
    VstInt16 bottom;
    VstInt16 right;
};

/* Forward declarations */
struct AEffect;

/* Base class for VST editor windows */
class AEffEditor
{
public:
    AEffEditor(AEffect* effect) : effect(effect), systemWindow(0) {}
    virtual ~AEffEditor() {}

    virtual bool getRect(ERect** rect) { return false; }
    virtual bool open(void* ptr) { systemWindow = ptr; return false; }
    virtual void close() { systemWindow = 0; }
    virtual void idle() {}
    virtual bool isOpen() { return systemWindow != 0; }

#if defined(MAC_VST)
    virtual void draw(ERect* rect) {}
    virtual VstInt32 mouse(VstInt32 x, VstInt32 y) { return 0; }
    virtual VstInt32 key(VstInt32 keyCode) { return 0; }
    virtual void top() {}
    virtual void sleep() {}
#endif

    virtual bool onKeyDown(VstKeyCode& keyCode) { return false; }
    virtual bool onKeyUp(VstKeyCode& keyCode) { return false; }
    virtual bool onWheel(float distance) { return false; }
    virtual bool setKnobMode(VstInt32 val) { return false; }

protected:
    AEffect* effect;
    void*    systemWindow;
};

#endif /* __AEFFEDITOR__ */
