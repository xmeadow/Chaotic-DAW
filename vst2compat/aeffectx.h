/*
 * VST2 compatibility headers for Chaotic-DAW
 * Provides the VST2 plugin interface types needed for plugin hosting.
 * Based on the publicly documented VST2 interface specification.
 */

#ifndef __AEFFECTX__
#define __AEFFECTX__

#include <stdint.h>

/* VST2 calling convention */
#if defined(_WIN32) || defined(__WIN32__)
  #define VSTCALLBACK __cdecl
#else
  #define VSTCALLBACK
#endif

/* Basic integer types */
typedef int32_t  VstInt32;
typedef int16_t  VstInt16;
typedef int64_t  VstInt64;
typedef intptr_t VstIntPtr;

/* AEffect magic */
#define kEffectMagic  (('V' << 24) | ('s' << 16) | ('t' << 8) | 'P')

/* Forward declarations */
struct AEffect;
struct VstEvents;
struct VstTimeInfo;

/* Function pointer types */
typedef VstIntPtr (VSTCALLBACK *AEffectDispatcherProc)(AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt);
typedef void     (VSTCALLBACK *AEffectProcessProc)(AEffect* effect, float** inputs, float** outputs, VstInt32 sampleFrames);
typedef void     (VSTCALLBACK *AEffectProcessDoubleProc)(AEffect* effect, double** inputs, double** outputs, VstInt32 sampleFrames);
typedef void     (VSTCALLBACK *AEffectSetParameterProc)(AEffect* effect, VstInt32 index, float parameter);
typedef float    (VSTCALLBACK *AEffectGetParameterProc)(AEffect* effect, VstInt32 index);

/* AEffect flags */
enum VstAEffectFlags
{
    effFlagsHasEditor     = 1 << 0,
    effFlagsCanReplacing  = 1 << 4,
    effFlagsProgramChunks = 1 << 5,
    effFlagsIsSynth       = 1 << 8,
    effFlagsNoSoundInStop = 1 << 9,
    effFlagsCanDoubleReplacing = 1 << 12
};

/* The main AEffect structure */
struct AEffect
{
    VstInt32 magic;

    AEffectDispatcherProc    dispatcher;
    AEffectProcessProc       process;
    AEffectSetParameterProc  setParameter;
    AEffectGetParameterProc  getParameter;

    VstInt32 numPrograms;
    VstInt32 numParams;
    VstInt32 numInputs;
    VstInt32 numOutputs;

    VstInt32 flags;

    VstIntPtr resvd1;
    VstIntPtr resvd2;

    VstInt32 initialDelay;

    VstInt32 realQualities;
    VstInt32 offQualities;
    float    ioRatio;

    void* object;
    void* user;

    VstInt32 uniqueID;
    VstInt32 version;

    AEffectProcessProc       processReplacing;
    AEffectProcessDoubleProc processDoubleReplacing;

    char future[56];
};

/* Dispatcher opcodes (effXxx) */
enum AEffectOpcodes
{
    effOpen = 0,
    effClose,
    effSetProgram,
    effGetProgram,
    effSetProgramName,
    effGetProgramName,
    effGetParamLabel,
    effGetParamDisplay,
    effGetParamName,
    effGetVu,
    effSetSampleRate,
    effSetBlockSize,
    effMainsChanged,
    effEditGetRect,
    effEditOpen,
    effEditClose,
    effEditDraw,
    effEditMouse,
    effEditKey,
    effEditIdle,
    effEditTop,
    effEditSleep,
    effIdentify,
    effGetChunk,
    effSetChunk,
    effNumOpcodes,

    /* VST 2.0 */
    effProcessEvents = 25,
    effCanBeAutomated,
    effString2Parameter,
    effGetNumProgramCategories,
    effGetProgramNameIndexed,
    effCopyProgram,
    effConnectInput,
    effConnectOutput,
    effGetInputProperties,
    effGetOutputProperties,
    effGetPlugCategory,
    effGetCurrentPosition,
    effGetDestinationBuffer,
    effOfflineNotify,
    effOfflinePrepare,
    effOfflineRun,
    effProcessVarIo,
    effSetSpeakerArrangement,
    effSetBlockSizeAndSampleRate,
    effSetBypass,
    effGetEffectName,
    effGetErrorText,
    effGetVendorString,
    effGetProductString,
    effGetVendorVersion,
    effVendorSpecific,
    effCanDo,
    effGetTailSize,
    effIdle,
    effGetIcon,
    effSetViewPosition,
    effGetParameterProperties,
    effKeysRequired,
    effGetVstVersion,
    effNumV2Opcodes
};

/* audioMaster (host) opcodes */
enum AudioMasterOpcodes
{
    audioMasterAutomate = 0,
    audioMasterVersion,
    audioMasterCurrentId,
    audioMasterIdle,
    audioMasterPinConnected,
    audioMasterWantMidi = 6,
    audioMasterGetTime,
    audioMasterProcessEvents,
    audioMasterSetTime,
    audioMasterTempoAt,
    audioMasterGetNumAutomatableParameters,
    audioMasterGetParameterQuantization,
    audioMasterIOChanged,
    audioMasterNeedIdle,
    audioMasterSizeWindow,
    audioMasterGetSampleRate,
    audioMasterGetBlockSize,
    audioMasterGetInputLatency,
    audioMasterGetOutputLatency,
    audioMasterGetPreviousPlug,
    audioMasterGetNextPlug,
    audioMasterWillReplaceOrAccumulate,
    audioMasterGetCurrentProcessLevel,
    audioMasterGetAutomationState,
    audioMasterOfflineStart,
    audioMasterOfflineRead,
    audioMasterOfflineWrite,
    audioMasterOfflineGetCurrentPass,
    audioMasterOfflineGetCurrentMetaPass,
    audioMasterSetOutputSampleRate,
    audioMasterGetOutputSpeakerArrangement,
    audioMasterGetVendorString,
    audioMasterGetProductString,
    audioMasterGetVendorVersion,
    audioMasterVendorSpecific,
    audioMasterSetIcon,
    audioMasterCanDo,
    audioMasterGetLanguage,
    audioMasterOpenWindow,
    audioMasterCloseWindow,
    audioMasterGetDirectory,
    audioMasterUpdateDisplay,
    audioMasterBeginEdit,
    audioMasterEndEdit,
    audioMasterOpenFileSelector,
    audioMasterCloseFileSelector,
    audioMasterEditFile,
    audioMasterGetChunkFile,
    audioMasterGetInputSpeakerArrangement
};

/* VstTimeInfo flags */
enum VstTimeInfoFlags
{
    kVstTransportChanged     = 1,
    kVstTransportPlaying     = 1 << 1,
    kVstTransportCycleActive = 1 << 2,
    kVstTransportRecording   = 1 << 3,
    kVstAutomationWriting    = 1 << 6,
    kVstAutomationReading    = 1 << 7,
    kVstNanosValid           = 1 << 8,
    kVstPpqPosValid          = 1 << 9,
    kVstTempoValid           = 1 << 10,
    kVstBarsValid            = 1 << 11,
    kVstCyclePosValid        = 1 << 12,
    kVstTimeSigValid         = 1 << 13,
    kVstSmpteValid           = 1 << 14,
    kVstClockValid           = 1 << 15
};

/* VstTimeInfo structure */
struct VstTimeInfo
{
    double samplePos;
    double sampleRate;
    double nanoSeconds;
    double ppqPos;
    double tempo;
    double barStartPos;
    double cycleStartPos;
    double cycleEndPos;
    VstInt32 timeSigNumerator;
    VstInt32 timeSigDenominator;
    VstInt32 smpteOffset;
    VstInt32 smpteFrameRate;
    VstInt32 samplesToNextClock;
    VstInt32 flags;
};

/* VstEvent types */
enum VstEventTypes
{
    kVstMidiType    = 1,
    kVstAudioType   = 2,
    kVstVideoType   = 3,
    kVstParameterType = 4,
    kVstTriggerType = 5,
    kVstSysExType   = 6
};

/* Base VstEvent */
struct VstEvent
{
    VstInt32 type;
    VstInt32 byteSize;
    VstInt32 deltaFrames;
    VstInt32 flags;
    char data[16];
};

/* VstMidiEvent flags */
enum VstMidiEventFlags
{
    kVstMidiEventIsRealtime = 1 << 0
};

/* MIDI event */
struct VstMidiEvent
{
    VstInt32 type;
    VstInt32 byteSize;
    VstInt32 deltaFrames;
    VstInt32 flags;
    VstInt32 noteLength;
    VstInt32 noteOffset;
    char     midiData[4];
    char     detune;
    char     noteOffVelocity;
    char     reserved1;
    char     reserved2;
};

/* SysEx event */
struct VstMidiSysexEvent
{
    VstInt32 type;
    VstInt32 byteSize;
    VstInt32 deltaFrames;
    VstInt32 flags;
    VstInt32 dumpBytes;
    VstIntPtr resvd1;
    char*    sysexDump;
    VstIntPtr resvd2;
};

/* Events container */
struct VstEvents
{
    VstInt32  numEvents;
    VstIntPtr reserved;
    VstEvent* events[2];
};

/* Plugin categories */
enum VstPlugCategory
{
    kPlugCategUnknown = 0,
    kPlugCategEffect,
    kPlugCategSynth,
    kPlugCategAnalysis,
    kPlugCategMastering,
    kPlugCategSpacializer,
    kPlugCategRoomFx,
    kPlugSurroundFx,
    kPlugCategRestoration,
    kPlugCategOfflineProcess,
    kPlugCategShell,
    kPlugCategGenerator
};

/* Pin properties */
struct VstPinProperties
{
    char     label[64];
    VstInt32 flags;
    VstInt32 arrangementType;
    char     shortLabel[8];
    char     future[48];
};

enum VstPinPropertiesFlags
{
    kVstPinIsActive   = 1 << 0,
    kVstPinIsStereo   = 1 << 1,
    kVstPinUseSpeaker = 1 << 2
};

/* Parameter properties */
struct VstParameterProperties
{
    float    stepFloat;
    float    smallStepFloat;
    float    largeStepFloat;
    char     label[64];
    VstInt32 flags;
    VstInt32 minInteger;
    VstInt32 maxInteger;
    VstInt32 stepInteger;
    VstInt32 largeStepInteger;
    char     shortLabel[8];
    VstInt16 displayIndex;
    VstInt16 category;
    VstInt16 numParametersInCategory;
    VstInt16 reserved;
    char     categoryLabel[24];
    char     future[16];
};

enum VstParameterFlags
{
    kVstParameterIsSwitch        = 1 << 0,
    kVstParameterUsesIntegerMinMax = 1 << 1,
    kVstParameterUsesFloatStep   = 1 << 2,
    kVstParameterUsesIntStep     = 1 << 3,
    kVstParameterSupportsDisplayIndex = 1 << 4,
    kVstParameterSupportsDisplayCategory = 1 << 5,
    kVstParameterCanRamp         = 1 << 6
};

/* Speaker types */
enum VstSpeakerType
{
    kSpeakerUndefined = 0x7fffffff,
    kSpeakerM   = 0,
    kSpeakerL,
    kSpeakerR,
    kSpeakerC,
    kSpeakerLfe,
    kSpeakerLs,
    kSpeakerRs,
    kSpeakerLc,
    kSpeakerRc,
    kSpeakerS,
    kSpeakerCs  = kSpeakerS,
    kSpeakerSl,
    kSpeakerSr,
    kSpeakerTm,
    kSpeakerTfl,
    kSpeakerTfc,
    kSpeakerTfr,
    kSpeakerTrl,
    kSpeakerTrc,
    kSpeakerTrr,
    kSpeakerLfe2
};

/* Speaker arrangement types */
enum VstSpeakerArrangementType
{
    kSpeakerArrUserDefined = -2,
    kSpeakerArrEmpty       = -1,
    kSpeakerArrMono        = 0,
    kSpeakerArrStereo,
    kSpeakerArrStereoSurround,
    kSpeakerArrStereoCenter,
    kSpeakerArrStereoSide,
    kSpeakerArrStereoCLfe,
    kSpeakerArr30Cine,
    kSpeakerArr30Music,
    kSpeakerArr31Cine,
    kSpeakerArr31Music,
    kSpeakerArr40Cine,
    kSpeakerArr40Music,
    kSpeakerArr41Cine,
    kSpeakerArr41Music,
    kSpeakerArr50,
    kSpeakerArr51,
    kSpeakerArr60Cine,
    kSpeakerArr60Music,
    kSpeakerArr61Cine,
    kSpeakerArr61Music,
    kSpeakerArr70Cine,
    kSpeakerArr70Music,
    kSpeakerArr71Cine,
    kSpeakerArr71Music,
    kSpeakerArr80Cine,
    kSpeakerArr80Music,
    kSpeakerArr81Cine,
    kSpeakerArr81Music,
    kSpeakerArr102,
    kNumSpeakerArr
};

/* Speaker */
struct VstSpeaker
{
    float    azimuth;
    float    elevation;
    float    radius;
    float    reserved;
    char     name[64];
    VstInt32 type;
    char     future[28];
};

/* Speaker arrangement */
struct VstSpeakerArrangement
{
    VstInt32   type;
    VstInt32   numChannels;
    VstSpeaker speakers[8];
};

/* Patch chunk info */
struct VstPatchChunkInfo
{
    VstInt32 version;
    VstInt32 pluginUniqueID;
    VstInt32 pluginVersion;
    VstInt32 numElements;
    char future[48];
};

/* Offline audio file */
struct VstAudioFile
{
    VstInt32 flags;
    void*    hostOwned;
    void*    plugOwned;
    char     name[100];
    VstInt32 uniqueId;
    double   sampleRate;
    VstInt32 numChannels;
    double   numFrames;
    VstInt32 format;
    double   editCursorPosition;
    double   selectionStart;
    double   selectionSize;
    VstInt32 selectedChannelsMask;
    VstInt32 numMarkers;
    VstInt32 timeRulerUnit;
    double   timeRulerOffset;
    double   tempo;
    VstInt32 timeSigNumerator;
    VstInt32 timeSigDenominator;
    VstInt32 ticksPerBlackNote;
    VstInt32 smpteFrameRate;
    char future[64];
};

/* Offline option */
enum VstOfflineOption
{
    kVstOfflineAudio = 0,
    kVstOfflinePeaks,
    kVstOfflineParameter,
    kVstOfflineMarker,
    kVstOfflineCursor,
    kVstOfflineSelection,
    kVstOfflineQueryFiles
};

/* Offline task */
struct VstOfflineTask
{
    char     processName[96];
    double   readPosition;
    double   writePosition;
    VstInt32 readCount;
    VstInt32 writeCount;
    VstInt32 frames;
    VstInt32 numSourceLines;
    VstInt32 destinationLine;
    VstInt32 numDestinationLines;
    double   sourceSampleRate;
    double   destinationSampleRate;
    VstInt32 numSourceChannels;
    VstInt32 numDestinationChannels;
    VstInt32 sourceFormat;
    VstInt32 destinationFormat;
    char     outputText[24];
    double   progress;
    VstInt32 progressMode;
    char     progressText[24];
    VstInt32 flags;
    VstInt32 returnValue;
    void*    hostOwned;
    void*    plugOwned;
    char future[256];
};

/* Variable I/O */
struct VstVariableIo
{
    float**  inputs;
    float**  outputs;
    VstInt32 numSamplesInput;
    VstInt32 numSamplesOutput;
    VstInt32* numSamplesInputProcessed;
    VstInt32* numSamplesOutputProcessed;
};

/* Window structure */
struct VstWindow
{
    char     title[128];
    VstInt16 xPos;
    VstInt16 yPos;
    VstInt16 width;
    VstInt16 height;
    VstInt32 style;
    void*    parent;
    void*    userHandle;
    void*    winHandle;
    char future[104];
};

/* File type for file selector */
struct VstFileType
{
    char name[128];
    char macType[8];
    char dosType[8];
    char unixType[8];
    char mimeType1[128];
    char mimeType2[128];
};

/* File selector */
struct VstFileSelect
{
    VstInt32     command;
    VstInt32     type;
    VstInt32     macCreator;
    VstInt32     nbFileTypes;
    VstFileType* fileTypes;
    char         title[1024];
    char*        initialPath;
    char*        returnPath;
    VstInt32     sizeReturnPath;
    char**       returnMultiplePaths;
    VstInt32     nbReturnPath;
    VstIntPtr    reserved;
    char         future[116];
};

/* MIDI program name info */
struct MidiProgramName
{
    VstInt32 thisProgramIndex;
    char     name[64];
    char     midiProgram;
    char     midiBankMsb;
    char     midiBankLsb;
    char     reserved;
    VstInt32 parentCategoryIndex;
    VstInt32 flags;
};

/* MIDI program category */
struct MidiProgramCategory
{
    VstInt32 thisCategoryIndex;
    char     name[64];
    VstInt32 parentCategoryIndex;
    VstInt32 flags;
};

/* MIDI key name */
struct MidiKeyName
{
    VstInt32 thisProgramIndex;
    VstInt32 thisKeyNumber;
    char     keyName[64];
    VstInt32 reserved;
    VstInt32 flags;
};

/* Automation states */
enum VstAutomationStates
{
    kVstAutomationUnsupported = 0,
    kVstAutomationOff,
    kVstAutomationRead,
    kVstAutomationWrite,
    kVstAutomationReadWrite
};

/* Process precision */
enum VstProcessPrecision
{
    kVstProcessPrecision32 = 0,
    kVstProcessPrecision64
};

/* Virtual keys */
enum VstVirtualKey
{
    VKEY_BACK = 1,
    VKEY_TAB,
    VKEY_CLEAR,
    VKEY_RETURN,
    VKEY_PAUSE,
    VKEY_ESCAPE,
    VKEY_SPACE,
    VKEY_NEXT,
    VKEY_END,
    VKEY_HOME,
    VKEY_LEFT,
    VKEY_UP,
    VKEY_RIGHT,
    VKEY_DOWN,
    VKEY_PAGEUP,
    VKEY_PAGEDOWN,
    VKEY_SELECT,
    VKEY_PRINT,
    VKEY_ENTER,
    VKEY_SNAPSHOT,
    VKEY_INSERT,
    VKEY_DELETE,
    VKEY_HELP,
    VKEY_NUMPAD0,
    VKEY_NUMPAD1,
    VKEY_NUMPAD2,
    VKEY_NUMPAD3,
    VKEY_NUMPAD4,
    VKEY_NUMPAD5,
    VKEY_NUMPAD6,
    VKEY_NUMPAD7,
    VKEY_NUMPAD8,
    VKEY_NUMPAD9,
    VKEY_MULTIPLY,
    VKEY_ADD,
    VKEY_SEPARATOR,
    VKEY_SUBTRACT,
    VKEY_DECIMAL,
    VKEY_DIVIDE,
    VKEY_F1,
    VKEY_F2,
    VKEY_F3,
    VKEY_F4,
    VKEY_F5,
    VKEY_F6,
    VKEY_F7,
    VKEY_F8,
    VKEY_F9,
    VKEY_F10,
    VKEY_F11,
    VKEY_F12,
    VKEY_NUMLOCK,
    VKEY_SCROLL,
    VKEY_SHIFT,
    VKEY_CONTROL,
    VKEY_ALT,
    VKEY_EQUALS
};

/* Modifier keys */
enum VstModifierKey
{
    MODIFIER_SHIFT     = 1 << 0,
    MODIFIER_ALTERNATE = 1 << 1,
    MODIFIER_COMMAND   = 1 << 2,
    MODIFIER_CONTROL   = 1 << 3
};

/* Key code */
struct VstKeyCode
{
    VstInt32 character;
    unsigned char virt;
    unsigned char modifier;
};

/* VST 2.1 dispatcher opcodes */
enum AEffect21Opcodes
{
    effEditKeyDown = effNumV2Opcodes,
    effEditKeyUp,
    effSetEditKnobMode,
    effGetMidiProgramName,
    effGetCurrentMidiProgram,
    effGetMidiProgramCategory,
    effHasMidiProgramsChanged,
    effGetMidiKeyName,
    effBeginSetProgram,
    effEndSetProgram,
    effNumV2_1Opcodes
};

/* VST 2.3 dispatcher opcodes */
enum AEffect23Opcodes
{
    effGetSpeakerArrangement = effNumV2_1Opcodes,
    effShellGetNextPlugin,
    effStartProcess,
    effStopProcess,
    effSetTotalSampleToProcess,
    effSetPanLaw,
    effBeginLoadBank,
    effBeginLoadProgram,
    effNumV2_3Opcodes
};

/* VST 2.4 dispatcher opcodes */
enum AEffect24Opcodes
{
    effSetProcessPrecision = effNumV2_3Opcodes,
    effGetNumMidiInputChannels,
    effGetNumMidiOutputChannels,
    effNumV2_4Opcodes
};

/* Mark these extensions as available */
#define VST_2_1_EXTENSIONS 1
#define VST_2_3_EXTENSIONS 1
#define VST_2_4_EXTENSIONS 1

#endif /* __AEFFECTX__ */
