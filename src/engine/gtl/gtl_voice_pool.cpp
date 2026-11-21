#include <decomp/static_init.h>
#include "classes/Voice8040E9BC.h"
// Existing Voice fields replace raw offsets below; original addresses 0x8041095C-0x80412070.
// Voice dimensions/node: 0x8041095C-0x80412070; layout owned by gtl_voice_and_stream_tasks.cpp.
// Translation unit 0x8041095C-0x80412070 (27 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

// Field-derived byte addresses keep list calls from hoisting extra base registers.
// Pool lists own no allocation: each link is embedded in a caller-supplied Voice.
// count is a numeric 32-bit count even where matching code loads it as void*.
// active/free are circular lists: sentinel.next/prev point to the sentinel when empty.
// A node address is Voice + kVoiceLinkOffset; subtract that offset before using Voice
// fields. Subtract kPriorityBeforeLink from the node to read the signed priority word.
// Returning a voice transfers its embedded link, never the caller-owned Voice storage.
struct VoicePoolLink { VoicePoolLink* next; VoicePoolLink* prev; };
struct VoicePoolList { unsigned int count; VoicePoolLink sentinel; };
struct VoicePoolState {
    unsigned char initialized;
    char reserved[3];
    VoicePoolList active;
    VoicePoolList free;
};
// Preserve byte cursors in grid loops: columns are four AX-voice pointers wide.
struct VoiceGridCursor { void* previousCell; void* axVoice; };
struct AxVoiceHandle { void* hardwareVoice; };
struct AxPlaybackState { char reserved[0x38]; unsigned short state; };
struct VoiceRemoteLevels { char reserved[0xDC]; float volume[4]; float send[4]; };
struct VoicePriority { char reserved[0xAC]; int priority; };
struct AudioControllerVoiceState {
    unsigned char active;
    char reserved[4];
    unsigned char pendingParameterMask;
    char reserved6[0x7A];
    VoicePoolLink link;
};
enum {
    kActiveListOffset = (int)&((VoicePoolState*)0)->active,
    kFreeListOffset = (int)&((VoicePoolState*)0)->free,
    kActiveSentinelOffset = (int)&((VoicePoolState*)0)->active.sentinel,
    kFreeSentinelOffset = (int)&((VoicePoolState*)0)->free.sentinel,
    kVoiceBytes = sizeof(Voice), kVoiceLinkOffset = (int)&((Voice*)0)->poolLinkNext,
    kPriorityBeforeLink = (int)&((Voice*)0)->poolLinkNext - (int)&((VoicePriority*)0)->priority,
    kVoiceColumnBytes = 4 * sizeof(void*), kVoiceRowBytes = sizeof(void*),
    kAxStopped = 0, kAxPlaying = 1, kVoiceStopped = 1, kVoiceStolen = 2,
    kVoiceAxRemoved = 3, kProtectedPriority = 0xFF,
    kSeekStart = 0, kSeekEnd = 2,
    kOutputMono = 0, kOutputStereo = 1, kOutputSurround = 2, kOutputSpatialRemap = 3,
    kMainOutput = 1, kFirstRemoteOutput = 2, kMaximumMixCoefficient = 0xFFFF,
    kMainMixCoefficientCount = 12, kRemoteOutputCount = 4, kQ15Unity = 32768
};

typedef char VoicePoolState_size[(sizeof(VoicePoolState) == 28) ? 1 : -1];
typedef char VoicePoolState_active_offset[(kActiveListOffset == 4) ? 1 : -1];
typedef char VoicePoolState_free_offset[(kFreeListOffset == 0x10) ? 1 : -1];
typedef char VoicePriority_offset[((int)&((VoicePriority*)0)->priority == 0xAC) ? 1 : -1];
typedef char VoiceRemoteLevels_send_offset[((int)&((VoiceRemoteLevels*)0)->send == 0xEC) ? 1 : -1];

struct InterruptUnwindCleanup {
    BOOL level;
    bool active;
    // Arm interrupt restoration with the saved previous level.
    explicit InterruptUnwindCleanup(BOOL saved) : level(saved), active(true) {}
    // Disarm unwind restoration and return the saved level for explicit restoration.
    BOOL release() { active = false; return level; }
    // Restore interrupts on unwind while the cleanup remains armed.
    ~InterruptUnwindCleanup() {
        if (active) OSRestoreInterrupts(level);
    }
};


extern float kFloat_6_2831855;
extern float s_twoPi_8041095C_VoicePool;
extern float s_twoPi_8041095C_DvdFilePosition;
extern float s_twoPi_8041095C_DvdStreamBase;
extern int s_dvdFileStreamRootTypeInfo;
extern signed char s_voicePoolInitialized;
extern unsigned int s_gtlVoicePool[7];
extern unsigned char s_voicePoolDestructorRegistration[0xC];
extern void* s_dvdFileStreamBaseTypeInfo;
extern float s_twoPi_8041095C_DvdStreamRoot;
extern const float kFloat_1_0_AudioVoice, kFloat_0_AudioVoice, kFloat_neg0_12_AudioVoice, kFloat_0_88_AudioVoice;
extern const float kFloat_0_5_AudioVoice, kFloat_1_12_AudioVoice, kFloat_0_85_AudioSpatialCurve, kFloat_0_14999998_AudioSpatialCurve;
extern const float kFloat_2_AudioMath, kFloat_0_35000002_AudioSpatialCurve;
extern const float kFloat_3_AudioVoice, kAudioStereoLeftPanOffset, kAudioSurroundRearAttenuationDb;

// Return the SDK's configured maximum AX voice count.
int AXGetMaxVoices();
void __dl__FPv(void*);
void gtl_Voice_StopAllAxVoiceRamps(char*);
void GtlVoice_NotifyOutputBufferReadStarted(void*);
void gtl_nw4r_detail_Voice_ApplyPendingParameters(void*);
void GtlVoice_UpdatePlaybackState(void*);
void gtl_IntrusiveList_Dtor(void*, int);
void GtlVoice_StopMixAndVolumeRamps(void*);
void* gtl_VoicePool_Dtor(void*, int);
void IntrusiveList_UnlinkNode(void*, void*);
void gtl_IntrusiveList_InsertBefore(void*, void*, void*);
void gtl_nw4r_detail_Voice_Ctor(void*);
void GtlVoice_Stop(void*);
void GtlVoice_Release(void*);
void gtl_nw4r_detail_Voice_Dtor(void*, int);
void IntrusiveList_UnlinkNodeAndDecrementCount(void*, void*);
void gtl_Voice_ComputeOutputMixCoefficients(void*, int, int, unsigned char*, void**);
int AudioVoice_RampMixCoefficients(void*, void*);
void AudioVoice_SetDspParameterFEIfAllocated(void*, int);
void gtl_AxVoice_SetEightMixCoefficients(void*, void*);
int GtlVoice_IsSourceBaseInRange(void*, void*, void*);
void AXSetVoiceState(void*, int);
void* GtlVoicePool_GetInstance(void*);
void gtl_AxVoicePool_ReleaseVoice(void*, void*);
void* gtl_Voice_AllocateAxVoiceGrid(void*, void*, void*, int, void*, void*);
void GtlVoice_SetAllocatedPrioritiesTo16(void*);
float AudioGain_DecibelsToLinear(float);
float gtl_Audio_LookupPanGain(float);
float AudioPan_LookupScaledGain(float);
unsigned int gtl_AudioController_GetOutputMode(void*);
void* gtl_AudioController_GetInstance();

// Initialize the voice pool unit's two-pi constant.
void GtlVoicePool_InitializeMathConstants();

// Initialize the DVD-position unit's two-pi constant.
void GtlDvdFilePosition_InitializeMathConstants();

// Link DVD stream base type information and initialize its math constant.
void gtl_nw4r_ut_DvdFileStream_InitializeBaseTypeInfoAndConstants();

// Clear the root stream type-information word and initialize its math constant.
void GtlDvdFileStream_InitializeRootTypeInfoAndConstants();

// Return the caller-owned storage needed for the maximum software voice count.
int gtl_VoicePool_GetRequiredMemorySize();

// Lazily initialize active/free circular lists and register singleton cleanup.
void* gtl_VoicePool_GetInstance();

// Optionally free this trivial stream-position object.
void* dtor_80411914(void* self, int shouldDelete);

// Default voice hook: no work is required.
void gtl_nw4r_detail_Voice_vfunc_0(void*);

 #pragma use_lmw_stmw on
// Stop and notify a software voice if any source lies in the removed memory range.
void gtl_nw4r_detail_Voice_StopForRemovedSource(void* self, void* sourceBegin, void* sourceEnd);
 #pragma use_lmw_stmw reset

// Default DVD async-wait hook: no work is required.
void gtl_nw4r_ut_DvdFileStream_vfunc_5(void*);

// Report no result for the default DVD async-result hook.
int gtl_nw4r_ut_DvdFileStream_vfunc_6(void*);

// Detach both pool lists and optionally free the pool object.
void* gtl_VoicePool_Dtor(void* self, int shouldDelete);

#pragma use_lmw_stmw on
// Stop mix and volume ramps for every allocated cell of the AX-voice grid.
void gtl_Voice_StopAllAxVoiceRamps(char* self);
#pragma use_lmw_stmw reset

// Optionally free this trivial DVD-position object.
void* dtor_80411FFC(void* self, int shouldDelete);

// Advance the unsigned position, clamping the wrapped sum to the file length.
int gtl_DvdFilePosition_AdvanceClamped(unsigned int* self, unsigned int delta);

// Stop all active-voice ramps while interrupts are disabled.
void GtlVoicePool_StopActiveVoiceRamps(void* self);

// Update callbacks and parameters, then refresh playback state under interrupt protection.
void gtl_VoicePool_UpdateActiveVoices(void* self);

// Select the seek origin and clamp the unsigned sum to the file length.
void gtl_DvdFilePosition_SeekClamped(int* self, int delta, int mode);

// Move an embedded voice node from the active list to the free list atomically.
void gtl_VoicePool_ReturnVoice(void* self, void* item);

// OR pending parameter bits into every active controller voice under interrupt protection.
void gtl_AudioController_MarkActiveVoices(void* self, int mask);

// Construct caller-supplied voice storage and append each node to the free list.
void gtl_VoicePool_Initialize(unsigned char* self, void* items, unsigned int bytes);

// Stop, notify, and release active voices, then destruct all free voices.
void gtl_VoicePool_Shutdown(unsigned char* self);

// Acquire or steal by priority, allocate AX cells, and insert into the ordered active list.
// ABI pointer-typed sourceColumns/mixRows carry integer counts, clamped by the AX
// allocator to 1..2 / 1..4. priority is compared as int, then stored as an unsigned byte;
// callers use 0..255. 255 requests protected AX priority. The callback is borrowed:
// callback(Voice*, reason, callbackArgument), with a borrowed user argument.
// With no free voice, the active list must contain a candidate. Equal priority can
// be stolen. Stealing runs Stop -> Release -> callback(reason 2), all with interrupts
// disabled, before allocating a replacement. Shutdown instead runs Stop -> callback
// (reason 1) -> Release. The callback must tolerate the released/stopped voice state.
// Returns borrowed pool Voice* on success or null on priority/AX allocation failure;
// a failed new allocation does not undo a completed steal. Return via Release.
void* gtl_VoicePool_AcquireVoice(void* self, void* sourceColumns, void* mixRows, int priority, void* eventCallback, void* callbackArgument);

 #pragma use_lmw_stmw on
// Apply main/remote mix ramps and auxiliary coefficients to each allocated AX cell.
int gtl_Voice_ApplyOutputMixCoefficients(unsigned char* self);
 #pragma use_lmw_stmw reset

#pragma use_lmw_stmw on
// Release remaining AX cells, clear playback flags, and report the removal reason.
void GtlVoice_HandleAxVoiceRemoval(void* value, int mode, unsigned char* self);
#pragma use_lmw_stmw reset

// Remap pan and surround coordinates through the output-mode-3 piecewise curve.
void GtlVoice_RemapPanForOutputMode3(float a, float b, void* unused, float* outA, float* outB);

// Convert a nonnegative gain to saturated unsigned Q15 without changing conversion calls.
static unsigned short fn_80410D78_pack(float value) {
    unsigned long result = Runtime_ConvertDoubleToUnsigned((double)((float)kQ15Unity * value));
    if (result < kMaximumMixCoefficient)
        result = Runtime_ConvertDoubleToUnsigned((double)((float)kQ15Unity * value));
    else
        result = kMaximumMixCoefficient;
    return (unsigned short)result;
}

// Caller supplies writable arrays: outBytes = 12 u16 (24 bytes), auxPtr = 8 u16
// (16 bytes despite its void** ABI type). object is borrowed Voice storage;
// sourceChannel selects source column 0/1, channelIndex selects a mix-parameter row.
// Coefficients are saturated unsigned Q15: 1.0 -> 32768, maximum 65535.
// Normal output groups are [main L,R,S], [aux A L,R,S], [aux B L,R,S], [aux C L,R,S].
// Mono duplicates L/R and leaves S zero; stereo leaves S zero. Surround uses S as rear.
// Mode 3 repurposes indices 2/5/8 as main/A/B rear-left and 9/10/11 as main/A/B
// rear-right (aux C is absent). Remote r uses aux[2*r] for mono gain, aux[2*r+1]=0.
// outputLineMask bit 0 enables main and aux; bits 1..4 enable remote outputs 0..3.
// Build twelve main/auxiliary and eight remote coefficients for one source/channel cell.
void gtl_Voice_ComputeOutputMixCoefficients(void* object, int sourceChannel, int channelIndex, unsigned char* outBytes, void** auxPtr);

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462700", _ctors_u8041095C_0, GtlVoicePool_InitializeMathConstants);
STATIC_INIT_ENTRY(".CRT$XCU80462700", _ctors_u8041095C_1, GtlDvdFilePosition_InitializeMathConstants);
STATIC_INIT_ENTRY(".CRT$XCU80462700", _ctors_u8041095C_2, gtl_nw4r_ut_DvdFileStream_InitializeBaseTypeInfoAndConstants);
STATIC_INIT_ENTRY(".CRT$XCU80462700", _ctors_u8041095C_3, GtlDvdFileStream_InitializeRootTypeInfoAndConstants);
#pragma pop

// Game entry points in original address order.
extern "C" {

#pragma push
#pragma use_lmw_stmw on
// Apply calculated mix/remote coefficients to each allocated AX cell.
int gtl_Voice_ApplyOutputMixCoefficients(unsigned char* self) {
    volatile unsigned char padding[16];
    int result = 0;
    unsigned char* row = self;
    int i = 0;
    for (; i < ((Voice*)self)->sourceChannelCount; ++i) {
        unsigned char* cell = row;
        for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
            void* value = ((VoiceGridCursor*)cell)->axVoice;
            if (value != 0) {
                unsigned short aux[8];     // gtl_Voice_ComputeOutputMixCoefficients writes 8 halfwords here
                unsigned short data[12];   // and 12 halfwords here
                gtl_Voice_ComputeOutputMixCoefficients(self, i, j, (unsigned char*)data, (void**)aux);
                int found = AudioVoice_RampMixCoefficients(value, data);
                result = (result | found) != 0;
                if ((*(unsigned int*)&((Voice*)self)->outputLineMask) <= 1)
                    AudioVoice_SetDspParameterFEIfAllocated(value, 0);
                else {
                    AudioVoice_SetDspParameterFEIfAllocated(value, 1);
                    gtl_AxVoice_SetEightMixCoefficients(value, aux);
                }
            }
            cell += kVoiceRowBytes;
        }
        row += kVoiceColumnBytes;
    }
    return result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Stop volume and mix ramps for every allocated AX grid cell.
void gtl_Voice_StopAllAxVoiceRamps(char* self) {
    int offset = 0;
    int i = 0;
    for (; i < ((Voice*)self)->outputVoiceCount; i++) {
        char* entry = self + offset;
        int j = 0;
        for (; j < ((Voice*)self)->sourceChannelCount; j++) {
            void* value = ((VoiceGridCursor*)entry)->axVoice;
            if (value != 0)
                GtlVoice_StopMixAndVolumeRamps(value);
            entry += kVoiceColumnBytes;
        }
        offset += kVoiceRowBytes;
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Release remaining AX cells and notify the software voice removal reason.
void GtlVoice_HandleAxVoiceRemoval(void* value, int mode, unsigned char* self) {
    int clear = 0;
    int notifyMode;
    if (mode == 1) {
        notifyMode = kVoiceAxRemoved;
        clear = 1;
    } else if (mode >= 0) {
        notifyMode = kVoiceStopped;
    } else {
        notifyMode = 0;
    }
    unsigned char* row = self;
    for (int i = 0; i < ((Voice*)self)->sourceChannelCount; ++i) {
        unsigned char* cell = row;
        for (int j = 0; j < ((Voice*)self)->outputVoiceCount; ++j) {
            void* current = ((VoiceGridCursor*)cell)->axVoice;
            if (current != 0) {
                if (current != value) {
                    void* object = ((AxVoiceHandle*)current)->hardwareVoice;
                    if (object != 0 && ((AxPlaybackState*)object)->state == kAxPlaying)
                        AXSetVoiceState(object, kAxStopped);
                    gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(current), current);
                } else if (clear == 0) {
                    gtl_AxVoicePool_ReleaseVoice(GtlVoicePool_GetInstance(current), current);
                }
                ((VoiceGridCursor*)cell)->axVoice = 0;
            }
            cell += kVoiceRowBytes;
        }
        row += kVoiceColumnBytes;
    }
    ((Voice*)self)->pauseRequested = 0;
    ((Voice*)self)->playbackRequested = 0;
    ((Voice*)self)->sourceChannelCount = 0;
    if (clear)
        GtlVoice_Release(self);
    void (*callback)(void*, int, void*) = (*(void (**)(void*, int, void*))&((Voice*)self)->eventCallback);
    if (callback != 0)
        callback(self, notifyMode, (*(void**)&((Voice*)self)->callbackArgument));
}
#pragma pop

// Remap pan coordinates with the output-mode-3 piecewise gain curve.
void GtlVoice_RemapPanForOutputMode3(float a, float b, void* unused, float* outA, float* outB) {
    float delta = b - kFloat_1_0_AudioVoice;
    if (__fabsf(a) <= __fabsf(delta)) {
        if (delta <= kFloat_0_AudioVoice) {
            *outA = a;
            *outB = kFloat_0_88_AudioVoice * delta + kFloat_neg0_12_AudioVoice;
        } else {
            *outA = kFloat_0_5_AudioVoice * a;
            *outB = kFloat_1_12_AudioVoice * delta + kFloat_neg0_12_AudioVoice;
        }
    } else if (a >= kFloat_0_AudioVoice) {
        if (delta <= kFloat_0_AudioVoice) {
            float t = -delta / a;
            *outA = (kFloat_0_14999998_AudioSpatialCurve * t + kFloat_0_85_AudioSpatialCurve) * __fabsf(a);
            *outB = kFloat_neg0_12_AudioVoice + (kFloat_2_AudioMath * delta + kFloat_0_88_AudioVoice * a);
        } else {
            float t = -delta / a;
            *outA = (kFloat_0_35000002_AudioSpatialCurve * t + kFloat_0_85_AudioSpatialCurve) * __fabsf(a);
            *outB = kFloat_1_12_AudioVoice * a + kFloat_neg0_12_AudioVoice;
        }
    } else if (delta <= kFloat_0_AudioVoice) {
        float t = -delta / a;
        *outA = (kFloat_0_14999998_AudioSpatialCurve * t - kFloat_0_85_AudioSpatialCurve) * __fabsf(a);
        *outB = kFloat_neg0_12_AudioVoice + (kFloat_2_AudioMath * delta - kFloat_1_12_AudioVoice * a);
    } else {
        float t = -delta / a;
        *outA = (kFloat_0_35000002_AudioSpatialCurve * t - kFloat_0_85_AudioSpatialCurve) * __fabsf(a);
        *outB = kFloat_1_12_AudioVoice * (-a) + kFloat_neg0_12_AudioVoice;
    }
    *outB += kFloat_1_0_AudioVoice;
}

// Calculate one cell's main/aux and remote gains into caller-owned Q15 arrays.
void gtl_Voice_ComputeOutputMixCoefficients(void* object, int sourceChannel, int channelIndex, unsigned char* outBytes, void** auxPtr) {
    unsigned char* self = (unsigned char*)object;
    unsigned short* out = (unsigned short*)outBytes;
    unsigned short* aux = (unsigned short*)auxPtr;
    unsigned int flags = (*(unsigned int*)&((Voice*)self)->outputLineMask);
    VoiceChannelParameters* channel = (VoiceChannelParameters*)(self + channelIndex * sizeof(VoiceChannelParameters) + (int)&((Voice*)0)->channels);

    // Four channel scales (the original keeps them in f31..f28).
    float mainScale, auxAScale, auxBScale, auxCScale;
    {
        float volume = kFloat_0_AudioVoice, mainSend = kFloat_0_AudioVoice, auxASend = kFloat_0_AudioVoice, auxBSend = kFloat_0_AudioVoice, auxCSend = kFloat_0_AudioVoice;
        if (flags & kMainOutput) {
            float gain = ((Voice*)self)->auxASend + channel->auxASendOffset;
            volume = ((Voice*)self)->mainVolume;
            mainSend = ((Voice*)self)->mainSend;
            if (gain > kFloat_1_0_AudioVoice) auxASend = kFloat_1_0_AudioVoice;
            else if (gain < auxCSend) auxASend = auxCSend;
            else auxASend = gain;
            auxBSend = ((Voice*)self)->auxBSend;
            auxCSend = ((Voice*)self)->auxCSend;
        }
        mainScale = volume * mainSend;
        auxAScale = volume * auxASend;
        auxBScale = volume * auxBSend;
        auxCScale = volume * auxCSend;
    }

    // Per-channel send levels: remoteScales[i] = a[i] * b[i] when flag bit (2 << i) is set.
    float remoteScales[4];
    for (int i = 0; i < kRemoteOutputCount; ++i) {
        float x = kFloat_0_AudioVoice, y = kFloat_0_AudioVoice;
        if (flags & ((unsigned)kFirstRemoteOutput << i)) {
            x = ((VoiceRemoteLevels*)self)->volume[i];
            y = ((VoiceRemoteLevels*)self)->send[i];
        }
        remoteScales[i] = x * y;
    }

    // Phase 2: resolve pan coordinates and front/rear gains (dimensionless).
    float leftGain = kFloat_0_AudioVoice, rightGain = kFloat_0_AudioVoice, frontGain, rearGain;
    // Mode zero pans the stereo source channels independently instead of blending them.
    if (((Voice*)self)->sourceChannelCount == 2 && ((Voice*)self)->stereoPanMode == 0) {
        float a = (((Voice*)self)->pan + ((Voice*)self)->panOffset) + channel->panOffset;
        float b = (((Voice*)self)->surroundPan + ((Voice*)self)->surroundPanOffset) + channel->surroundPanOffset;
        float scale = kFloat_1_0_AudioVoice;
        // Gain mode zero applies the +3 dB pan compensation.
        if (((Voice*)self)->panGainMode == 0) scale = AudioGain_DecibelsToLinear(kFloat_3_AudioVoice);
        if (sourceChannel == 0) {
            leftGain = scale * gtl_Audio_LookupPanGain(a);
            rightGain = kFloat_0_AudioVoice;
        } else if (sourceChannel == 1) {
            leftGain = kFloat_0_AudioVoice;
            rightGain = scale * gtl_Audio_LookupPanGain(-a);
        }
        frontGain = AudioPan_LookupScaledGain(b);
        rearGain = AudioPan_LookupScaledGain(kFloat_2_AudioMath - b);
    } else {
        float adjust = kFloat_0_AudioVoice;
        if (((Voice*)self)->sourceChannelCount == 2) {
            if (sourceChannel == 0) adjust = kAudioStereoLeftPanOffset;
            else if (sourceChannel == 1) adjust = kFloat_1_0_AudioVoice;
        }
        int panMode = gtl_AudioController_GetOutputMode(gtl_AudioController_GetInstance());
        float x = (adjust + (((Voice*)self)->pan + ((Voice*)self)->panOffset)) + channel->panOffset;
        float y = (((Voice*)self)->surroundPan + ((Voice*)self)->surroundPanOffset) + channel->surroundPanOffset;
        float remappedPan, remappedSurroundPan;
        if (panMode == kOutputSpatialRemap) {
            GtlVoice_RemapPanForOutputMode3(x, y, self, &remappedPan, &remappedSurroundPan);
        } else {
            remappedPan = x;
            remappedSurroundPan = y;
        }
        float scale = kFloat_1_0_AudioVoice;
        if (((Voice*)self)->sourceChannelCount == 1 && ((Voice*)self)->panGainMode == 0) scale = AudioGain_DecibelsToLinear(kFloat_3_AudioVoice);
        leftGain = scale * gtl_Audio_LookupPanGain(remappedPan);
        rightGain = scale * gtl_Audio_LookupPanGain(-remappedPan);
        frontGain = AudioPan_LookupScaledGain(remappedSurroundPan);
        rearGain = AudioPan_LookupScaledGain(kFloat_2_AudioMath - remappedSurroundPan);
    }

    float rearAttenuation = AudioGain_DecibelsToLinear(kAudioSurroundRearAttenuationDb);
    float monoGain = kFloat_0_5_AudioVoice * (leftGain + rightGain);

    // Phase 3: place gains in the selected output layout, then quantize to Q15.
    float coefficients[kMainMixCoefficientCount];   // the twelve coefficients, in output order
    for (int i = 0; i < kMainMixCoefficientCount; ++i) coefficients[i] = kFloat_0_AudioVoice;
    int mode = gtl_AudioController_GetOutputMode(gtl_AudioController_GetInstance());
    if (mode == kOutputStereo) {
        coefficients[0] = mainScale * leftGain; coefficients[1] = mainScale * rightGain;
        coefficients[3] = auxAScale * leftGain; coefficients[4] = auxAScale * rightGain;
        coefficients[6] = auxBScale * leftGain; coefficients[7] = auxBScale * rightGain;
        coefficients[9] = auxCScale * leftGain; coefficients[10] = auxCScale * rightGain;
    } else if (mode == kOutputMono) {
        coefficients[0] = mainScale * monoGain; coefficients[1] = coefficients[0];
        coefficients[3] = auxAScale * monoGain; coefficients[4] = coefficients[3];
        coefficients[6] = auxBScale * monoGain; coefficients[7] = coefficients[6];
        coefficients[9] = auxCScale * monoGain; coefficients[10] = coefficients[9];
    } else if (mode == kOutputSurround) {
        float frontLeftGain = leftGain * frontGain, frontRightGain = rightGain * frontGain, rearOutputGain = rearAttenuation * rearGain;
        coefficients[0] = mainScale * frontLeftGain; coefficients[1] = mainScale * frontRightGain; coefficients[2] = mainScale * rearOutputGain;
        coefficients[3] = auxAScale * frontLeftGain; coefficients[4] = auxAScale * frontRightGain; coefficients[5] = auxAScale * rearOutputGain;
        coefficients[6] = auxBScale * frontLeftGain; coefficients[7] = auxBScale * frontRightGain; coefficients[8] = auxBScale * rearOutputGain;
        coefficients[9] = auxCScale * frontLeftGain; coefficients[10] = auxCScale * frontRightGain; coefficients[11] = auxCScale * rearOutputGain;
    } else if (mode == kOutputSpatialRemap) {
        float rearLeftGain = leftGain * rearGain, frontLeftGain = leftGain * frontGain, frontRightGain = rightGain * frontGain, rearOutputGain = rightGain * rearGain;
        coefficients[0] = mainScale * frontLeftGain; coefficients[1] = mainScale * frontRightGain; coefficients[2] = mainScale * rearLeftGain;
        coefficients[3] = auxAScale * frontLeftGain; coefficients[4] = auxAScale * frontRightGain; coefficients[5] = auxAScale * rearLeftGain;
        coefficients[6] = auxBScale * frontLeftGain; coefficients[7] = auxBScale * frontRightGain; coefficients[8] = auxBScale * rearLeftGain;
        coefficients[9] = mainScale * rearOutputGain; coefficients[10] = auxAScale * rearOutputGain; coefficients[11] = auxBScale * rearOutputGain;
    }
    for (int i = 0; i < kMainMixCoefficientCount; ++i) out[i] = fn_80410D78_pack(coefficients[i]);
    for (int i = 0; i < kRemoteOutputCount; ++i) {
        aux[i * 2] = fn_80410D78_pack(monoGain * remoteScales[i]);
        aux[i * 2 + 1] = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Stop and notify a voice whose source storage is being removed.
void gtl_nw4r_detail_Voice_StopForRemovedSource(void* self, void* sourceBegin, void* sourceEnd) {
    unsigned char* voice = (unsigned char*)self;
    int found = 0;
    unsigned char* row = voice;
    for (int i = 0; i < ((Voice*)voice)->sourceChannelCount; ++i) {
        void* value = ((VoiceGridCursor*)row)->axVoice;
        if (value != 0 && GtlVoice_IsSourceBaseInRange(value, sourceBegin, sourceEnd)) {
            found = 1;
            break;
        }
        row += kVoiceColumnBytes;
    }
    if (found) {
        if (((Voice*)voice)->hardwareStarted) {
            row = voice;
            for (int i = 0; i < ((Voice*)voice)->sourceChannelCount; ++i) {
                unsigned char* cell = row;
                for (int j = 0; j < ((Voice*)voice)->outputVoiceCount; ++j) {
                    void* value = ((VoiceGridCursor*)cell)->axVoice;
                    if (value != 0) {
                        void* object = ((AxVoiceHandle*)value)->hardwareVoice;
                        int shouldStop = 0;
                        if (object != 0 && ((AxPlaybackState*)object)->state == kAxPlaying)
                            shouldStop = 1;
                        if (shouldStop)
                            AXSetVoiceState(object, kAxStopped);
                    }
                    cell += kVoiceRowBytes;
                }
                row += kVoiceColumnBytes;
            }
            ((Voice*)voice)->hardwareStarted = 0;
        }
        ((Voice*)voice)->hardwarePaused = 0;
        ((Voice*)voice)->pauseRequested = 0;
        ((Voice*)voice)->playbackRequested = 0;
        void (*callback)(void*, int, void*) = (*(void (**)(void*, int, void*))&((Voice*)voice)->eventCallback);
        if (callback != 0)
            callback(self, kVoiceStopped, (*(void**)&((Voice*)voice)->callbackArgument));
    }
}
#pragma pop

// Default software-voice hook has no work.
void gtl_nw4r_detail_Voice_vfunc_0(void*) {}

// Initialize the voice-pool angle constant.
void GtlVoicePool_InitializeMathConstants() { s_twoPi_8041095C_VoicePool = kFloat_6_2831855; }

// Return borrowed singleton storage after initializing its circular lists.
void* gtl_VoicePool_GetInstance() {
    if (!s_voicePoolInitialized) {
        unsigned int* list = s_gtlVoicePool;
        unsigned int zero = 0;
        unsigned int* last;
        unsigned int* first;
        first = &list[2];
        last = &list[5];
        list[2] = zero;
        list[3] = zero;
        list[5] = zero;
        list[6] = zero;
        ((unsigned char*)list)[0] = (unsigned char)zero;
        list[1] = zero;
        list[2] = (unsigned int)first;
        list[3] = (unsigned int)first;
        list[4] = zero;
        list[5] = (unsigned int)last;
        list[6] = (unsigned int)last;
        // The original registers the whole object (0x80411838: r3 = s_gtlVoicePool): gtl_VoicePool_Dtor destroys +0x10 and +4.
        __register_global_object(list, (void*)gtl_VoicePool_Dtor, s_voicePoolDestructorRegistration);
        s_voicePoolInitialized = 1;
    }
    return &s_gtlVoicePool;
}

// Detach both lists and optionally delete the pool object.
void* gtl_VoicePool_Dtor(void* self, int shouldDelete) {
    if (self != 0) {
        unsigned int p = (unsigned int)self + kFreeListOffset;
        if (p != 0)
            gtl_IntrusiveList_Dtor((void*)p, 0);
        p = (unsigned int)self + kActiveListOffset;
        if (p != 0)
            gtl_IntrusiveList_Dtor((void*)p, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

// Optionally delete the trivial stream-position object.
void* dtor_80411914(void* self, int shouldDelete) {
    if (self != 0 && shouldDelete > 0)
        __dl__FPv(self);
    return self;
}

// Return bytes required for the maximum software-voice pool.
int gtl_VoicePool_GetRequiredMemorySize() { return AXGetMaxVoices() * kVoiceBytes; }

// Construct caller-owned Voice slots and link them into the free list.
void gtl_VoicePool_Initialize(unsigned char* self, void* items, unsigned int bytes) {
    if (*self == 0) {
        unsigned int count = bytes / kVoiceBytes;
        unsigned char* item = (unsigned char*)items;
        void* end = self + kFreeSentinelOffset;
        for (unsigned int i = 0; i < count; ++i) {
            if (item != 0)
                gtl_nw4r_detail_Voice_Ctor(item);
            void* sentinel = end;
            void* local = end;
            gtl_IntrusiveList_InsertBefore(self + kFreeListOffset, &local, (unsigned char*)&((Voice*)item)->poolLinkNext);
            item += kVoiceBytes;
        }
        *self = 1;
    }
}

// Stop/release active voices, then destroy every caller-owned pool slot.
void gtl_VoicePool_Shutdown(unsigned char* self) {
    if (*self != 0) {
        BOOL level = OSDisableInterrupts();
        InterruptUnwindCleanup interruptCleanup0(level);
        while (*(void**)&((VoicePoolState*)self)->active.count != 0) {
            unsigned char* item = (unsigned char*)*(void**)&((VoicePoolState*)self)->active.sentinel.next - kVoiceLinkOffset;
            GtlVoice_Stop(item);
            void (**callback)(void*, int, void*) = (void (**)(void*, int, void*))&((Voice*)item)->eventCallback;
            if (*callback != 0)
                (*callback)(item, kVoiceStopped, (*(void**)&((Voice*)item)->callbackArgument));
            GtlVoice_Release(item);
        }
        OSRestoreInterrupts(interruptCleanup0.release());
        while (*(void**)&((VoicePoolState*)self)->free.count != 0) {
            void* node = *(void**)&((VoicePoolState*)self)->free.sentinel.next;
            IntrusiveList_UnlinkNodeAndDecrementCount(self + kFreeListOffset, &node);
            gtl_nw4r_detail_Voice_Dtor((unsigned char*)node - kVoiceLinkOffset, -1);
        }
        *self = 0;
    }
}

// Borrow a free/stealable software voice, allocate its AX grid, and order its priority.
void* gtl_VoicePool_AcquireVoice(void* self, void* sourceColumns, void* mixRows, int priority, void* eventCallback, void* callbackArgument) {
    // Supported priority input is 0..255; no clamp is performed. Comparison is
    // signed before storage truncates to u8. Out-of-range callers can therefore
    // choose a victim using a different priority from the one later stored.
    BOOL level = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup0(level);
    unsigned char* manager = (unsigned char*)self;
    // Phase 1: obtain room, optionally sacrificing the lowest-priority active voice.
    // A free voice uses token 1; a stolen voice supplies its previous AX cell count.
    // Only zero/nonzero is consumed below, so this is an availability word.
    unsigned int allocationAvailability = 1;
    if (*(void**)&((VoicePoolState*)manager)->free.count == 0) {
        void* candidateLink = *(void**)&((VoicePoolState*)manager)->active.sentinel.next;
        int candidatePriority = *(int*)((char*)candidateLink - kPriorityBeforeLink);
        unsigned char* voice = (unsigned char*)candidateLink - kVoiceLinkOffset;
        if (candidatePriority <= priority) {
            allocationAvailability = ((Voice*)voice)->sourceChannelCount * ((Voice*)voice)->outputVoiceCount;
            GtlVoice_Stop(voice);
            GtlVoice_Release(voice);
            void (*callback)(void*, int, void*) = (*(void (**)(void*, int, void*))&((Voice*)voice)->eventCallback);
            if (callback != 0)
                callback(voice, kVoiceStolen, (*(void**)&((Voice*)voice)->callbackArgument));
        } else {
            allocationAvailability = 0;
        }
    }
    if (allocationAvailability == 0) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }

    // Phase 2: allocate the hardware grid before moving the free link into active.
    unsigned char* voice = (unsigned char*)*(void**)&((VoicePoolState*)manager)->free.sentinel.next - kVoiceLinkOffset;
    void* result = gtl_Voice_AllocateAxVoiceGrid(voice, sourceColumns, mixRows, priority, eventCallback, callbackArgument);
    if (result == 0) {
        OSRestoreInterrupts(interruptCleanup0.release());
        return 0;
    }
    *(unsigned int*)&((VoicePriority*)voice)->priority = (unsigned char)priority;
    BOOL level2 = OSDisableInterrupts();
    InterruptUnwindCleanup interruptCleanup1(level2);
    void* node = (unsigned char*)&((Voice*)voice)->poolLinkNext;
    IntrusiveList_UnlinkNode(manager + kFreeListOffset, node);

    void* end = *(void**)&((VoicePoolState*)manager)->active.sentinel.next;
    void* cursor = manager + kActiveSentinelOffset;
    while (cursor != end) {
        void* next = ((VoicePoolLink*)cursor)->prev;
        if (*(int*)((char*)next - kPriorityBeforeLink) <= ((VoicePriority*)voice)->priority)
            break;
        cursor = next;
    }
    gtl_IntrusiveList_InsertBefore(manager + kActiveListOffset, &cursor, node);
    OSRestoreInterrupts(interruptCleanup1.release());

    // Phase 3: update hardware priorities along the affected active-list suffix.
    void* listEnd = manager + kActiveSentinelOffset;
    void* current = node;
    while (current != listEnd) {
        int activePriority = *(int*)((char*)current - kPriorityBeforeLink);
        void* currentVoice = (char*)current - kVoiceLinkOffset;
        if (activePriority <= 1)
            break;  // original 0x80411C88: ble leaves the loop
        if (activePriority != kProtectedPriority)
            GtlVoice_SetAllocatedPrioritiesTo16(currentVoice);
        current = ((VoicePoolLink*)current)->next;
    }
    OSRestoreInterrupts(interruptCleanup0.release());
    return voice;
}

// Move an embedded voice link from active to free under interrupt protection.
void gtl_VoicePool_ReturnVoice(void* self, void* item) {
    volatile char padding[16];
    volatile BOOL level1 = OSDisableInterrupts();
    volatile BOOL level2 = OSDisableInterrupts();
    void* node = &((Voice*)item)->poolLinkNext;
    IntrusiveList_UnlinkNode((char*)self + kActiveListOffset, node);
    void* local = (char*)self + kFreeSentinelOffset;
    gtl_IntrusiveList_InsertBefore((char*)self + kFreeListOffset, &local, node);
    OSRestoreInterrupts(level2);
    OSRestoreInterrupts(level1);
}

// Run output, parameter, and playback passes over active voices.
void gtl_VoicePool_UpdateActiveVoices(void* self) {
    void* end = (char*)self + kActiveSentinelOffset;
    void* node = *(void**)end;
    while (node != end) {
        void* current = node;
        node = *(void**)node;
        GtlVoice_NotifyOutputBufferReadStarted((char*)current - kVoiceLinkOffset);
    }
    node = *(void**)end;
    while (node != end) {
        void* current = node;
        node = *(void**)node;
        gtl_nw4r_detail_Voice_ApplyPendingParameters((char*)current - kVoiceLinkOffset);
    }
    volatile BOOL level = OSDisableInterrupts();
    node = *(void**)end;
    while (node != end) {
        void* current = node;
        node = *(void**)node;
        GtlVoice_UpdatePlaybackState((char*)current - kVoiceLinkOffset);
    }
    OSRestoreInterrupts(level);
}

// Stop each active voice ramp while interrupts are disabled.
void GtlVoicePool_StopActiveVoiceRamps(void* self) {
    volatile BOOL level = OSDisableInterrupts();
    void* end = (char*)self + kActiveSentinelOffset;
    void* node = *(void**)end;
    while (node != end) {
        void* current = node;
        node = *(void**)node;
        gtl_Voice_StopAllAxVoiceRamps((char*)current - kVoiceLinkOffset);
    }
    OSRestoreInterrupts(level);
}

// Mark pending parameter bits on all active controller voices.
void gtl_AudioController_MarkActiveVoices(void* self, int mask) {
    volatile BOOL saved;
    void* node;
    void* end;
    unsigned char* obj;
    int value;
    saved = OSDisableInterrupts();
    end = (char*)self + kActiveSentinelOffset;
    node = *(void**)end;
    while (node != end) {
        void* current = node;
        node = *(void**)node;
        obj = (unsigned char*)current - (int)&((AudioControllerVoiceState*)0)->link;
        value = ((AudioControllerVoiceState*)obj)->active;
        if (value != 0) ((AudioControllerVoiceState*)obj)->pendingParameterMask |= mask;
    }
    OSRestoreInterrupts(saved);
}

// Initialize the DVD-position angle constant.
void GtlDvdFilePosition_InitializeMathConstants() { s_twoPi_8041095C_DvdFilePosition = kFloat_6_2831855; }

// Advance the unsigned file position and clamp the wrapped sum to length.
int gtl_DvdFilePosition_AdvanceClamped(unsigned int* self, unsigned int delta) {
    if (delta != 0) {
        unsigned int value = self[1] + delta;
        if (value > self[0])
            value = self[0];
        self[1] = value;
    }
    return self[1];
}

// Seek from the requested origin and clamp the unsigned result to file length.
void gtl_DvdFilePosition_SeekClamped(int* self, int delta, int mode) {
    if (mode == kSeekStart)
        self[1] = 0;
    else if (mode == kSeekEnd)
        self[1] = self[0];
    if (delta != 0) {
        unsigned int value = (unsigned int)self[1] + (unsigned int)delta;
        unsigned int max = (unsigned int)self[0];
        if (value > max)
            value = max;
        self[1] = (int)value;
    }
}

// Optionally delete the trivial DVD-position object.
void* dtor_80411FFC(void* self, int shouldDelete) {
    if (self != 0 && shouldDelete > 0)
        __dl__FPv(self);
    return self;
}

// Link DVD base type information and initialize its angle constant.
void gtl_nw4r_ut_DvdFileStream_InitializeBaseTypeInfoAndConstants() {
    s_dvdFileStreamBaseTypeInfo = &s_dvdFileStreamRootTypeInfo;
    s_twoPi_8041095C_DvdStreamBase = kFloat_6_2831855;
}

// Default DVD wait hook has no work.
void gtl_nw4r_ut_DvdFileStream_vfunc_5(void*) {}

// Return zero from the default DVD result hook.
int gtl_nw4r_ut_DvdFileStream_vfunc_6(void*) { return 0; }

// Clear DVD root type information and initialize its angle constant.
void GtlDvdFileStream_InitializeRootTypeInfoAndConstants() {
    s_dvdFileStreamRootTypeInfo = 0;
    s_twoPi_8041095C_DvdStreamRoot = kFloat_6_2831855;
}
} // extern "C"
