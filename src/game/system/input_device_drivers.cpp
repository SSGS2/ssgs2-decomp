// Controller sampling and report building for the Wii Remote, Nunchuk, Classic Controller and GameCube.
// InputDriver_UpdateAll polls the hardware and stores sample histories, which each driver's
// provider functions turn into the reports the game reads.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Controller sampling and report translation (Wii Remote, Nunchuk/FreeStyle,
// Classic Controller, GameCube). InputDriver_UpdateAll polls hardware before
// publishing histories through provider vtables; game_input_manager_cursor_rumble.cpp consumes InputEx actions.
// Modding lookup:
// - Classic buttons/axes: ClassicReportSlot + ClassicButtonBit, converted by the Classic functions below.
//   The 20-slot ABI includes R but no separate digital L slot. Preserve this map.
// - Analog sensitivity: kControllerAnalogReportScale (100000) and Y sign (-1),
//   phase1_shared_data.cpp; shared by Classic, pointing and gtl_pad_drivers_gc_classic_freestyle.cpp FreeStyle.
//   Representation map: KPAD supplies calibrated stick floats about neutral 0,
//   nominal -1..+1; trigger floats are nominal 0 released..1 fully pressed. This
//   layer multiplies by 100000 and truncates, with stick Y inverted. These are
//   nominal report ranges, not guaranteed physical travel bounds: no clamp or
//   calibration occurs here. Pointer XY/horizon stay KPAD-normalized coordinates;
//   acceleration is in SDK g units and distance in SDK distance units, so the
//   shared factor is fixed-point serialization, not a common physical sensitivity.
//   declares stick [-100000,100000], trigger [0,100000]; default dead/saturation
//   fractions are 500/9500 (5%/95%) and 1500/9000 (15%/90%), denominated in 10000.
//   InputDriver_ResetItemRanges copies these into per-channel ranges, then
//   InputDriver_RemapAxisValue (gtl_pad_drivers_gc_classic_freestyle.cpp) applies their dead/saturation policy
//   during BuildStateReport. Changing only the scale leaves catalog endpoints
//   unchanged and changes where dead zone/saturation occur. Keep scale, device
//   ranges and requested output ranges coordinated. Pointing items default to
//   no dead zone (0/10000); their acceleration samples need not fit +/-1g.
// - Idle detection/frame rate: Engine_UpdateInput thresholds and kIdleResetUpdates;
//   counters are polling calls, independent of KPAD's number of returned samples.
// No viewport or aspect scaling happens here: pointer floats stay in KPAD space.
// All channel APIs require 0..3; histories hold 0..16 samples. Descriptor getters
// borrow static storage (Classic item 0..19, pointing 0..21, layer 0..3).
// No driver getter transfers ownership. Calls run on the engine input thread;
// allocator callbacks must remain installed while WPAD owns allocations.
extern "C" {
extern unsigned char nunchukReportItems[];
}
#include "game/singletons.h"
#include "game/types.h"
#include "classes/GtlRvlInputHistory.h"
#include "classes/GtlPadDriverState.h"

#include "sdk_protos.h"
#include "classes/CRvlPointingDeviceDriver.h"

extern "C" {

// KPADRead returns up to sixteen 0x84-byte samples for each of four Wii Remotes.
// Integer bit views below preserve the original word-copy order for float fields.
struct KpadSampleWords {
    int heldButtons, pressedButtons, releasedButtons, accelerationXBits, accelerationYBits, accelerationZBits;
    float accelerationMagnitude, accelerationSpeed;
    int pointerXBits, pointerYBits, pointerDeltaXBits, pointerDeltaYBits;
    float pointerSpeed;
    int horizonXBits, horizonYBits, horizonDeltaXBits, horizonDeltaYBits;
    float horizonSpeed, pointerDistance, pointerDistanceDelta, pointerDistanceSpeed;
    int verticalAccelerationXBits, verticalAccelerationYBits;
    unsigned char extensionType;
    signed char status;
    signed char pointingValidity;
    unsigned char dataFormat;
    int extensionWords[9]; // Nunchuk/Classic payload, copied unchanged for provider-specific decoding.
};

struct KpadChannelHistory {
    int count;
    int probeResult;
    KpadSampleWords samples[16];
};
// Byte cursors below follow these independent layouts. Driver histories start
// after the vtable (+4): {count, 16 samples}; KPAD histories are {count, probe, 16 samples}.
enum InputHistoryBytes {
    kClassicHistoryBytes = sizeof(GtlClassicPadHistory),
    kClassicSampleBytes = sizeof(GtlClassicPadSample),
    kPointingHistoryBytes = sizeof(GtlPointingDeviceHistory),
    kPointingSampleBytes = sizeof(GtlPointingDeviceSample),
    kKpadHistoryBytes = sizeof(KpadChannelHistory),
    kKpadSampleBytes = sizeof(KpadSampleWords)
};

struct ControllerPollingState {
    GtlGCPadChannel gameCubeChannels[4];
    KpadChannelHistory channels[4];
    char reserved_2160[0x40];
    KpadChannelHistory readBuffer;
    float idleAcceleration[12];
};

struct KpadSampleView {
    unsigned int heldButtons, pressedButtons, releasedButtons;
    float acceleration[3];
    float accelerationMagnitude, accelerationSpeed;
    float pointerPosition[2], pointerDelta[2], pointerSpeed;
    float horizon[2], horizonDelta[2], horizonSpeed;
    float pointerDistance, pointerDistanceDelta, pointerDistanceSpeed;
    float verticalAcceleration[2];
    unsigned char extensionType;
    signed char status, pointingValidity;
    unsigned char dataFormat;
    char extensionData[0x24];
};
struct InputItemCatalog {
    GtlInputItem gameCube[16];
    GtlInputItem freeStyle[7];
    char reserved_284[4];
    GtlInputItem classic[20];
    GtlInputItem pointing[22];
};
typedef char KpadSampleView_size[(sizeof(KpadSampleView) == 0x84) ? 1 : -1];
typedef char KpadSampleWords_size[(sizeof(KpadSampleWords) == 0x84) ? 1 : -1];
typedef char KpadChannelHistory_size[(sizeof(KpadChannelHistory) == 0x848) ? 1 : -1];
typedef char ControllerPollingState_channels_offset[((int)&((ControllerPollingState*)0)->channels == 0x40) ? 1 : -1];
// Access only the ID word: typed struct-array stores change CodeWarrior's register allocation.
inline int& InputItemId(char* table, int itemIndex) {
    return *(int*)(table + itemIndex * sizeof(GtlInputItem));
}
enum InputCatalogOffset {
    kFreeStyleItemsOffset = (int)&((InputItemCatalog*)0)->freeStyle,
    kClassicItemsOffset = (int)&((InputItemCatalog*)0)->classic,
    kPointingItemsOffset = (int)&((InputItemCatalog*)0)->pointing
};
enum ControllerLimits {
    kControllerChannels = 4,
    kHistorySamples = 16,
    kIdleResetUpdates = 1800,
    kButtonPressedShift = 7
};
// Baselines are SDK report units: these are activity thresholds, not controller
// dead zones or cursor sensitivity. Larger values admit more motion as idle.
// Preserve strict > comparisons and float precision; safe tuning ranges unknown.
// Settings -> consumer map (all below in Engine_UpdateInput, slot 0 only):
// accelerationSpeed > .05 or tracked(validity==2) pointerSpeed > .0001 marks
// activity; either threshold increased tolerates more motion without resetting
// idle. Per-axis abs(acceleration-baseline) > .25 independently marks activity;
// increasing it tolerates more slow drift. These consume SDK values BEFORE the
// 100000 report scale/remapping, so they are unrelated to report dead zones.
// s_inputIdleFrameCount counts consecutive all-channel-idle polling calls;
// kIdleResetUpdates=1800 triggers only at >1800, schedules a baseline refresh
// next call, and zeros the count. Refresh flag also restarts the count; count==0
// snapshots all four first XYZ samples. Activity zeros the count and thus causes
// a fresh baseline on the next call. Channel-0 missing data and any held button
// always count as activity, regardless of thresholds. Only this function reads
// these idle globals in recovered src: this is baseline maintenance, not an
// established screen-saver timeout. Do not label 1800 as seconds or retraces.
static const float kIdleAccelerationSpeedThreshold = 0.05f;
static const float kIdlePointerSpeedThreshold = 0.0001f;
static const float kIdleAccelerationDriftThreshold = 0.25f;
enum {
    kTrackedPointerValidity = 2
};
enum DriverResult {
    kDriverReady = 0,
    kDriverDisconnected = 1,
    kDriverCommunicationError = 2,
    kDriverWrongDevice = 3,
    kDriverNoData = 4
};
enum WpadStatus {
    kWpadOk = 0,
    kWpadNoController = -1,
    kWpadCommunicationError = -2,
    kWpadWrongDevice = -3,
    kWpadInvalidSample = -4
};
enum ProviderId {
    kProviderPointing = 0xB,
    kProviderFreeStyle = 0xC,
    kProviderClassic = 0xD,
    kProviderGameCube = 0xE
};
enum {
    kExtensionUnavailable = 0xFD,
    kRemoteWithoutExtension = 0,
    kMotorOff = 0,
    kMotorOn = 1,
    kRemoteButtonMask = 0xFFFF9FFF
};
// Bit positions in KPAD's held-button word (extension Z/C are removed above).
enum PointingReportSlot {
    kPointingReportPointerX = 0,
    kPointingReportPointerY = 1,
    kPointingReportReservedZ = 2,
    kPointingReportHorizonX = 3,
    kPointingReportHorizonY = 4,
    kPointingReportReservedHorizonZ = 5,
    kPointingReportAccelerationX = 6,
    kPointingReportAccelerationY = 7,
    kPointingReportAccelerationZ = 8,
    kPointingReportPointerDistance = 9,
    kPointingReportPointingValidity = 10,
    kPointingReportUp = 11,
    kPointingReportDown = 12,
    kPointingReportLeft = 13,
    kPointingReportRight = 14,
    kPointingReportPlus = 15,
    kPointingReportMinus = 16,
    kPointingReportHome = 17,
    kPointingReportA = 18,
    kPointingReportB = 19,
    kPointingReportOne = 20,
    kPointingReportTwo = 21
};
enum ClassicReportSlot {
    kClassicReportLeftStickX = 0,
    kClassicReportLeftStickY = 1,
    kClassicReportRightStickX = 2,
    kClassicReportRightStickY = 3,
    kClassicReportLeftTrigger = 4,
    kClassicReportRightTrigger = 5,
    kClassicReportUp = 6,
    kClassicReportDown = 7,
    kClassicReportLeft = 8,
    kClassicReportRight = 9,
    kClassicReportPlus = 10,
    kClassicReportMinus = 11,
    kClassicReportHome = 12,
    kClassicReportR = 13,
    kClassicReportA = 14,
    kClassicReportB = 15,
    kClassicReportX = 16,
    kClassicReportY = 17,
    kClassicReportZL = 18,
    kClassicReportZR = 19
};
enum RemoteButtonBit {
    kRemoteLeft = 0,
    kRemoteRight = 1,
    kRemoteDown = 2,
    kRemoteUp = 3,
    kRemotePlus = 4,
    kRemoteTwo = 8,
    kRemoteOne = 9,
    kRemoteB = 10,
    kRemoteA = 11,
    kRemoteMinus = 12,
    kRemoteHome = 15
};
enum ClassicButtonBit {
    kClassicUp = 0,
    kClassicLeft = 1,
    kClassicZR = 2,
    kClassicX = 3,
    kClassicA = 4,
    kClassicY = 5,
    kClassicB = 6,
    kClassicZL = 7,
    kClassicR = 8,
    kClassicPlus = 10,
    kClassicHome = 11,
    kClassicMinus = 12,
    kClassicL = 13,
    kClassicDown = 14,
    kClassicRight = 15
};

extern int kInputProviderClassic;
extern int kGcDriverCapabilityCode;
extern int kInputProviderPointing;
extern int kNunchukDriverCapabilityCode;
extern signed char s_gameCubePadDriverRegistered, s_freeStylePadDriverRegistered, s_classicPadDriverRegistered,
    s_pointingDriverRegistered;
extern char s_gameCubePadDriverStorage[], s_pointingDriverStorage[], s_freeStylePadDriverStorage[],
    s_classicPadDriverStorage[];
extern char __vt__Q23gtl12CGCPadDriver[], __vt__Q23gtl24CRvlPointingDeviceDriver[],
    __vt__Q23gtl22CRvlFreeStylePadDriver[], __vt__Q23gtl20CRvlClassicPadDriver[];
extern char s_inputDriverDestructorRegistrations[];
extern char s_classicPadInputItems[];
extern char s_pointingInputItems[];
extern char s_classicPadChannelMappings[];
extern char s_pointingChannelMappings[];
struct FnTable {
    void* (*allocate)(unsigned int);
};
extern FnTable* g_inputAllocator;

// Return the twenty-value Classic Controller report size.
// These getters report value counts/descriptor kinds, not byte lengths or fps.
int gtl_CRvlClassicPadDriver_vfunc_3() {
    return 0x14;
}

// Return the Classic Controller descriptor kind.
int gtl_CRvlClassicPadDriver_vfunc_7() {
    return 0x15;
}

// Return the twenty-two-value Wii Remote pointing report size.
int gtl_CRvlPointingDeviceDriver_vfunc_3() {
    return 0x16;
}

// Return the Wii Remote pointing descriptor kind.
int gtl_CRvlPointingDeviceDriver_vfunc_7() {
    return 0x1a;
}

// Return the buffered sample count for a Classic Controller channel.
int gtl_CRvlClassicPadDriver_vfunc_6(void* self, int channel) {
    return ((gtl::CRvlClassicPadDriver*)self)->channels[channel].sampleCount; // 0x80437060.
}

// Scan newest samples first; return ready if any sample is valid, otherwise the final mapped error.
// channel is in [0,3], sampleCount in [0,16]. Scanning backwards means the final
// error comes from the oldest sample if none is ready; 0xFD overrides status.
int gtl_CRvlClassicPadDriver_GetChannelStatus(void* self, int channel) {
    char* record = (char*)self + channel * kClassicHistoryBytes;
    int count = ((gtl::CRvlClassicPadDriver*)record)->channels[0].sampleCount;
    if (count < 1)
        return kDriverNoData;
    int i = count - 1;
    int result = kDriverNoData;
    char* item = record + 4 + i * kClassicSampleBytes + 4; // vtable, sample stride, count word.
    for (; i >= 0; --i, item -= kClassicSampleBytes) {
        int status = ((GtlClassicPadSample*)item)->status;
        unsigned int extensionType = ((GtlClassicPadSample*)item)->extensionType;
        switch (status) {
        case kWpadOk:
            result = kDriverReady;
            break;
        case kWpadNoController:
            result = kDriverDisconnected;
            break;
        case kWpadCommunicationError:
            result = kDriverCommunicationError;
            break;
        case kWpadWrongDevice:
            result = kDriverWrongDevice;
            break;
        case kWpadInvalidSample:
        default:
            result = kDriverNoData;
            break;
        }
        if (extensionType == kExtensionUnavailable)
            result = kDriverDisconnected;
        if (result == kDriverReady)
            return result;
    }
    return result;
}

// Return one Classic Controller input-item descriptor.
void* gtl_CRvlClassicPadDriver_vfunc_1(void*, int itemIndex) {
    return &((GtlInputItem*)s_classicPadInputItems)[itemIndex];
}

// Return the mapping entry for one Classic Controller channel and layer.
void* gtl_CRvlClassicPadDriver_vfunc_2(void*, int channel, int layer) {
    return &((GtlInputMappingEntry(*)[4])s_classicPadChannelMappings)[channel][layer];
}

// Write the Classic Controller provider ID to the caller.
// Receiver is unused; providerId requires one writable int. Always returns 1.
int gtl_CRvlClassicPadDriver_GetInputProviderId(void*, int* providerId) {
    *providerId = kInputProviderClassic;
    return 1;
}

// Return the buffered sample count for a Wii Remote channel.
int gtl_CRvlPointingDeviceDriver_vfunc_6(void* self, int channel) {
    return ((GtlPointingDeviceDriver*)self)->channels[channel].sampleCount;
}

#pragma opt_common_subs off
// Scan newest samples first; return ready if any sample is valid, otherwise the final mapped error.
int CRvlPointingDeviceDriver::GetChannelStatus(int channel) {
    char* base;
    int count;
    int result;
    char* item;
    int i;
    int status;
    unsigned int extensionType;
    base = (char*)this + channel * kPointingHistoryBytes;
    count = ((GtlPointingDeviceHistory*)(base + 4))->sampleCount;
    base = base + 4;
    if (count < 1)
        return kDriverNoData;
    result = kDriverNoData;
    i = count - 1;
    item = base + 4 + i * kPointingSampleBytes;
    for (; i >= 0; --i, item -= kPointingSampleBytes) {
        status = ((GtlPointingDeviceSample*)item)->status;
        extensionType = ((GtlPointingDeviceSample*)item)->extensionType;
        switch (status) {
        case kWpadOk:
            result = kDriverReady;
            break;
        case kWpadNoController:
            result = kDriverDisconnected;
            break;
        case kWpadCommunicationError:
            result = kDriverCommunicationError;
            break;
        case kWpadWrongDevice:
            result = kDriverWrongDevice;
            break;
        case kWpadInvalidSample:
        default:
            result = kDriverNoData;
            break;
        }
        if (extensionType == kExtensionUnavailable)
            result = kDriverDisconnected;
        if (result == kDriverReady)
            return result;
    }
    return result;
}
#pragma opt_common_subs reset

// Return one Wii Remote input-item descriptor.
void* gtl_CRvlPointingDeviceDriver_vfunc_1(void*, int itemIndex) {
    return &((GtlInputItem*)s_pointingInputItems)[itemIndex];
}

// Write the Wii Remote pointing provider ID to the caller.
// Receiver is unused; providerId requires one writable int. Always returns 1.
int gtl_CRvlPointingDeviceDriver_GetInputProviderId(void*, int* providerId) {
    *providerId = kInputProviderPointing;
    return 1;
}

// Forward WPAD allocation requests to the registered allocator callback.
// WPAD's request is a byte count in the 32-bit argument word, not a readable
// pointer. The table's first callback is the same size -> address allocator used
// by __nw__Q23gtl25IInputResourceMemOperatorFUl (gtl_vector_matrix_math.cpp); return its address
// verbatim, including null on failure. No rounding, fallback, ownership tracking
// or exception handling occurs here. The matching free callback uses table word
// 1, so keep both callbacks/allocator backing alive until WPAD releases its data.
// Explicit result/size types expose the existing tail-forwarded register ABI.
void* InputDriver_AllocCallback(unsigned int sizeBytes) {
    return g_inputAllocator->allocate(sizeBytes);
}

// Return the mapping entry for one Wii Remote channel and layer.
void* gtl_CRvlPointingDeviceDriver_vfunc_2(void*, int channel, int layer) {
    return &((GtlInputMappingEntry(*)[4])s_pointingChannelMappings)[channel][layer];
}

extern void __dl__Q23gtl25IInputResourceMemOperatorFPv(void*);
extern void Vector_CopyComponents(void*, void*);
extern void gtl_Vec2_Copy(void*, void*);

#pragma opt_lifetimes off
// Copy KPAD samples to the pointing driver, retaining Remote buttons, acceleration and infrared data.
// channelHistories borrows four KpadChannelHistory records. An entirely empty
// {count=0,probe=0} record preserves the destination; otherwise all 16 storage
// slots are copied, but only the first count slots are valid for consumers.
// Buttons lose extension Z/C bits; signed status/validity bytes widen to int.
int gtl_CRvlPointingDeviceDriver_CopyHistories(void* self, void* channelHistories) {
    char* dst;
    char* src;
    char* out;
    char* in;
    int channel;
    int i;
    unsigned int heldButtons;
    char* copiedAcceleration;
    float distance;
    char* acceleration;
    unsigned int masked;
    unsigned char pointingValidity;
    unsigned char status;
    int signedPointingValidity;
    int signedStatus;
    unsigned char extensionType;
    dst = (char*)self + 4;
    src = (char*)channelHistories;
    for (channel = 0; channel < 4; ++channel, dst += kPointingHistoryBytes, src += kKpadHistoryBytes) {
        if (*(int*)(src + 4) == 0 && *(int*)src == 0)
            continue;
        *(int*)dst = *(int*)src;
        out = dst + 4;
        in = src + 8;
        // Select the Remote fields; extension-specific decoding belongs to other drivers.
        for (i = 0; i < 16; ++i, out += kPointingSampleBytes, in += kKpadSampleBytes) {
            heldButtons = ((KpadSampleView*)in)->heldButtons;
            copiedAcceleration = (char*)&((GtlPointingDeviceSample*)out)->accelerationX;
            distance = ((KpadSampleView*)in)->pointerDistance;
            acceleration = (char*)((KpadSampleView*)in)->acceleration;
            masked = heldButtons & kRemoteButtonMask;
            ((GtlPointingDeviceSample*)out)->buttons = masked;
            pointingValidity = (unsigned char)((KpadSampleView*)in)->pointingValidity;
            status = (unsigned char)((KpadSampleView*)in)->status;
            signedPointingValidity = (signed char)pointingValidity;
            ((GtlPointingDeviceSample*)out)->pointerDistance = distance;
            signedStatus = (signed char)status;
            extensionType = ((KpadSampleView*)in)->extensionType;
            ((GtlPointingDeviceSample*)out)->pointingValidity = signedPointingValidity;
            ((GtlPointingDeviceSample*)out)->status = signedStatus;
            ((GtlPointingDeviceSample*)out)->extensionType = extensionType;
            Vector_CopyComponents(copiedAcceleration, acceleration);
            gtl_Vec2_Copy(&((GtlPointingDeviceSample*)out)->pointerX, ((KpadSampleView*)in)->pointerPosition);
            gtl_Vec2_Copy(&((GtlPointingDeviceSample*)out)->horizonX, ((KpadSampleView*)in)->horizon);
        }
    }
    return 1;
}
#pragma opt_lifetimes reset

// Destroy the registered GameCube driver storage when deletion is requested.
void* GameCubePadDriver_DestroyRegisteredInstance(void* instance, int deleteFlag) {
    if (instance != 0 && deleteFlag > 0)
        __dl__Q23gtl25IInputResourceMemOperatorFPv(instance);
    return instance;
}

// Destroy the registered Wii Remote pointing driver storage when deletion is requested.
void* RvlPointingDriver_DestroyRegisteredInstance(void* instance, int deleteFlag) {
    if (instance != 0 && deleteFlag > 0)
        __dl__Q23gtl25IInputResourceMemOperatorFPv(instance);
    return instance;
}

// Destroy the registered Nunchuk driver storage when deletion is requested.
void* RvlFreeStyleDriver_DestroyRegisteredInstance(void* instance, int deleteFlag) {
    if (instance != 0 && deleteFlag > 0)
        __dl__Q23gtl25IInputResourceMemOperatorFPv(instance);
    return instance;
}

// Destroy the registered Classic Controller driver storage when deletion is requested.
void* RvlClassicDriver_DestroyRegisteredInstance(void* instance, int deleteFlag) {
    if (instance != 0 && deleteFlag > 0)
        __dl__Q23gtl25IInputResourceMemOperatorFPv(instance);
    return instance;
}

// Forward WPAD releases to the registered allocator callback.
int InputDriver_FreeCallback(void* arg0) {
    struct FnTable2 {
        void* unk;
        void (*fn)(void*);
    };
    ((FnTable2*)g_inputAllocator)->fn(arg0);
    return 0;
}

extern char s_controllerPollingState[];
extern char s_wiiRemoteChannelHistories[];
struct SavedWords {
    int first;
    int second;
};
extern SavedWords s_inputAllocatorCallbacks;
extern int kInputAxisType0, kInputAxisId1, kInputAxisId2, kInputAxisCode3, kInt_4_inputDriver, kInputAxisId5,
    kInt_7_inputDriver;
extern char gcReportItems[];
extern char kGameCubePadChannelMasks[];
extern char s_nativeGameCubePadReadBuffer[];
extern void fn_803566A8();
extern void WPADRegisterAllocator(void*, void*);
extern void fn_803D29FC();
extern void InputDriver_ResetItemRanges(void*);
extern void PADRead(void*, unsigned int, unsigned int, unsigned int, void*);
extern void PADReset(int);

extern void WPADControlMotor(void*, int);
extern int Engine_UpdateInput();
extern void* InputDriver_GetByProviderId(int);

extern unsigned char s_inputIdleBaselineRefreshPending;
extern int s_inputIdleFrameCount;
extern int WPADProbe(int, void*);
extern int KPADRead(int, void*, int);
// Measure input activity with the original double fabs intrinsic and float result.
static inline float InputActivityAbsolute(double value) {
    return (float)__fabs(value);
}

struct ObjTable {
    virtual void fn(void*);
};

// Initialize item IDs for the GameCube, Nunchuk, Classic and pointing descriptors.
// These ID words are report TYPES, not unique control numbers. Full GC/FreeStyle
// slot -> control -> ID map (consumer: the matching report function in gtl_pad_drivers_gc_classic_freestyle.cpp):
// GC  0 stick X ->0, 1 negated stick Y ->1, 2 C-stick X ->2,
//     3 negated C-stick Y ->3, 4 L analog ->4, 5 R analog ->5;
//     6 Up,7 Down,8 Left,9 Right,10 A,11 B,12 X,13 Y,14 Z,15 Start ->7.
// FS  0 stick X ->0, 1 negated stick Y ->1, 2/3/4 acceleration X/Y/Z ->2/3/4;
//     5 Z (held bit 13), 6 C (held bit 14) ->7.
// Classic/pointing slots use ClassicReportSlot/PointingReportSlot above: slots
// 0..5 -> IDs 0..5; Classic 6..19 ->7; pointing 6..10 ->0..4, 11..21 ->7.
// Values 0..5 select signed-word axis remapping; 7 writes one button byte.
// BuildStateReport/BuildDescriptorMapping match repeated IDs in first-unused
// slot order, so ID 7 does NOT uniquely identify C, Z or any other button.
// Keep slot order, sample-value producers and descriptor output offsets together.
// GtlInputItem is 0x1C bytes: +0 ID is changed here; attributes at +4/+8 and
// This .ctors initializer may rewrite the same ID words; no allocations/owners.
void InputDriver_InitializeItemIds() {
    // Fill the item-ID word of each descriptor; all other descriptor words are static data.
    char* gameCubeItems = gcReportItems;
    char* freeStyleItems = gcReportItems + kFreeStyleItemsOffset;
    char* classicItems = gcReportItems + kClassicItemsOffset;
    char* pointingItems = gcReportItems + kPointingItemsOffset;
    InputItemId(gameCubeItems, 0) = kInputAxisType0;
    InputItemId(gameCubeItems, 1) = kInputAxisId1;
    InputItemId(gameCubeItems, 2) = kInputAxisId2;
    InputItemId(gameCubeItems, 3) = kInputAxisCode3;
    InputItemId(gameCubeItems, 4) = kInt_4_inputDriver;
    InputItemId(gameCubeItems, 5) = kInputAxisId5;
    InputItemId(gameCubeItems, 6) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 7) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 8) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 9) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 10) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 11) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 12) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 13) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 14) = kInt_7_inputDriver;
    InputItemId(gameCubeItems, 15) = kInt_7_inputDriver;
    InputItemId(freeStyleItems, 0) = kInputAxisType0;
    InputItemId(freeStyleItems, 1) = kInputAxisId1;
    InputItemId(freeStyleItems, 2) = kInputAxisId2;
    InputItemId(freeStyleItems, 3) = kInputAxisCode3;
    InputItemId(freeStyleItems, 4) = kInt_4_inputDriver;
    InputItemId(freeStyleItems, 5) = kInt_7_inputDriver;
    InputItemId(freeStyleItems, 6) = kInt_7_inputDriver;
    InputItemId(classicItems, 0) = kInputAxisType0;
    InputItemId(classicItems, 1) = kInputAxisId1;
    InputItemId(classicItems, 2) = kInputAxisId2;
    InputItemId(classicItems, 3) = kInputAxisCode3;
    InputItemId(classicItems, 4) = kInt_4_inputDriver;
    InputItemId(classicItems, 5) = kInputAxisId5;
    InputItemId(pointingItems, 5) = kInputAxisId5;
    InputItemId(classicItems, 6) = kInt_7_inputDriver;
    InputItemId(classicItems, 7) = kInt_7_inputDriver;
    InputItemId(classicItems, 8) = kInt_7_inputDriver;
    InputItemId(classicItems, 9) = kInt_7_inputDriver;
    InputItemId(classicItems, 10) = kInt_7_inputDriver;
    InputItemId(classicItems, 11) = kInt_7_inputDriver;
    InputItemId(classicItems, 12) = kInt_7_inputDriver;
    InputItemId(classicItems, 13) = kInt_7_inputDriver;
    InputItemId(classicItems, 14) = kInt_7_inputDriver;
    InputItemId(classicItems, 15) = kInt_7_inputDriver;
    InputItemId(classicItems, 16) = kInt_7_inputDriver;
    InputItemId(classicItems, 17) = kInt_7_inputDriver;
    InputItemId(classicItems, 18) = kInt_7_inputDriver;
    InputItemId(classicItems, 19) = kInt_7_inputDriver;
    InputItemId(pointingItems, 0) = kInputAxisType0;
    InputItemId(pointingItems, 1) = kInputAxisId1;
    InputItemId(pointingItems, 2) = kInputAxisId2;
    InputItemId(pointingItems, 3) = kInputAxisCode3;
    InputItemId(pointingItems, 4) = kInt_4_inputDriver;
    InputItemId(pointingItems, 6) = kInputAxisType0;
    InputItemId(pointingItems, 7) = kInputAxisId1;
    InputItemId(pointingItems, 8) = kInputAxisId2;
    InputItemId(pointingItems, 9) = kInputAxisCode3;
    InputItemId(pointingItems, 10) = kInt_4_inputDriver;
    InputItemId(pointingItems, 11) = kInt_7_inputDriver;
    InputItemId(pointingItems, 12) = kInt_7_inputDriver;
    InputItemId(pointingItems, 13) = kInt_7_inputDriver;
    InputItemId(pointingItems, 14) = kInt_7_inputDriver;
    InputItemId(pointingItems, 15) = kInt_7_inputDriver;
    InputItemId(pointingItems, 16) = kInt_7_inputDriver;
    InputItemId(pointingItems, 17) = kInt_7_inputDriver;
    InputItemId(pointingItems, 18) = kInt_7_inputDriver;
    InputItemId(pointingItems, 19) = kInt_7_inputDriver;
    InputItemId(pointingItems, 20) = kInt_7_inputDriver;
    InputItemId(pointingItems, 21) = kInt_7_inputDriver;
}

#pragma use_lmw_stmw on
// Read four GameCube pads, clear disconnected reports, and request channel resets.
// Returns 1 regardless of connection. Writes four 0x10-byte GtlGCPadChannel
// records from SDK 0x0C-byte reports; -1 errors contribute channel bits to
// PADReset. Mask table order and extra recovered PADRead register arguments
// are preserved; they are not additional output buffers or ownership transfers.
int InputDriver_PollGameCubePads() {
    unsigned char* dst;
    unsigned char* src;
    unsigned int* wp;
    unsigned char statusByte;
    unsigned int mask;
    int i, j;
    unsigned int words[4];
    unsigned short values[6];
    unsigned char* valueBytes = (unsigned char*)values;
    unsigned int* maskTable = (unsigned int*)kGameCubePadChannelMasks;
    words[0] = maskTable[0];
    words[1] = maskTable[1];
    words[2] = maskTable[2];
    words[3] = maskTable[3];
    PADRead(s_nativeGameCubePadReadBuffer, words[2], words[1], words[0], maskTable);
    src = (unsigned char*)s_nativeGameCubePadReadBuffer;
    dst = (unsigned char*)s_controllerPollingState;
    wp = words;
    mask = 0;
    for (i = 0; i < 4; i++, dst += 0x10, src += 0xc, wp++) {
        unsigned short* sourceShorts = (unsigned short*)src;
        values[0] = sourceShorts[0];
        values[1] = sourceShorts[1];
        values[2] = sourceShorts[2];
        values[3] = sourceShorts[3];
        values[4] = sourceShorts[4];
        values[5] = sourceShorts[5];
        statusByte = valueBytes[10];
        if ((signed char)statusByte == 0) {
            *(unsigned short*)(dst + 4) = values[0];
            for (j = 0; j < 8; j++)
                dst[6 + j] = valueBytes[2 + j];
            dst[14] = *(volatile unsigned char*)(valueBytes + 10);
            *(unsigned int*)dst = 1;
        } else {
            Memory_Clear(dst + 4, 12);
            dst[14] = statusByte;
            *(unsigned int*)dst = 0;
        }
        if ((signed char)statusByte == -1)
            mask |= *wp;
    }
    if (mask)
        PADReset(mask);
    return 1;
}
#pragma use_lmw_stmw reset

// Install WPAD allocation callbacks, initialize input, and publish the first samples.
// allocatorCallbacks requires two callback words copied into module-owned
// storage; only the table is copied, not the allocator's context/lifetime.
// Driver instances are static and registered once. Returns 1 after the initial
// poll/publication, not a guarantee that any hardware channel is connected.
// Required fresh startup phase with valid allocation/free FUNCTION pointers in
// those two words. Publication/failure order: copy callbacks -> publish global
// table -> fn_803566A8 -> WPADRegisterAllocator -> fn_803D29FC -> reset all four
// providers' item ranges -> poll PAD/KPAD -> publish four histories. The two
// address-named startup callees have no recovered body in src/include; their
// internal failure behavior is unknown. This wrapper observes no service,
// allocator-registration, range-reset or poll status, returns 1 after the calls,
// and performs no rollback/unregistration if a call throws or startup is partial.
// The first successful stage remains published if later work fails. Callback
// code/backing must outlive outstanding allocations, not just this stack call.
// Repeat calls recopy/replace callbacks, restart services, reset custom ranges
// and overwrite histories; only GetByProviderId's static construction/destructor
// registration is guarded. There is no whole-system once guard or live-allocation
// migration. Thus repeat startup with outstanding allocations requires the same
// compatible allocator pair; idempotence of the opaque services is unproved.
// gtl_InputEx_Initialize (gtl_input_ex_devices_part2.cpp) additionally clears wrapper slots on reentry;
// it is not evidence that reinitializing a live input system is safe.
int InputDriver_InitializeSystem(const void* allocatorCallbacks) {
    s_inputAllocatorCallbacks.first = *(const int*)allocatorCallbacks;
    s_inputAllocatorCallbacks.second = *((const int*)allocatorCallbacks + 1);
    *(int*)((char*)&g_inputAllocator) = (int)&s_inputAllocatorCallbacks;
    fn_803566A8();
    WPADRegisterAllocator((void*)InputDriver_AllocCallback, (void*)InputDriver_FreeCallback);
    fn_803D29FC();
    // Initialize descriptor ranges before publishing the first hardware histories.
    void* gameCubeDriver = (void*)InputDriver_GetByProviderId(kProviderGameCube);
    InputDriver_ResetItemRanges(gameCubeDriver);
    void* freeStyleDriver = (void*)InputDriver_GetByProviderId(kProviderFreeStyle);
    InputDriver_ResetItemRanges(freeStyleDriver);
    void* classicDriver = (void*)InputDriver_GetByProviderId(kProviderClassic);
    InputDriver_ResetItemRanges(classicDriver);
    void* pointingDriver = (void*)InputDriver_GetByProviderId(kProviderPointing);
    InputDriver_ResetItemRanges(pointingDriver);
    InputDriver_PollGameCubePads();
    Engine_UpdateInput();
    ObjTable* obj = (ObjTable*)InputDriver_GetByProviderId(kProviderGameCube);
    obj->fn(s_controllerPollingState);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderFreeStyle);
    obj->fn(s_wiiRemoteChannelHistories);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderClassic);
    obj->fn(s_wiiRemoteChannelHistories);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderPointing);
    obj->fn(s_wiiRemoteChannelHistories);
    return 1;
}

// Enable or disable Wii Remote rumble on the supplied channel.
int gtl_CRvlPointingDeviceDriver_SetRumbleEnabled(void* self, int enabled, void* channel) {
    if (enabled != 0)
        WPADControlMotor(channel, kMotorOn);
    else
        WPADControlMotor(channel, kMotorOff);
    return 1;
}

extern int InputDriver_BuildStateReport(void*, void*, int, int*, int);
extern void Memory_Set(void*, unsigned int, int);
extern float kControllerAnalogReportScale, kControllerStickReportYSign;

// Lazily initialize and return the driver for the requested controller provider.
void* InputDriver_GetByProviderId(int id) {
    char* regmem = s_inputDriverDestructorRegistrations;
    if (id == kGcDriverCapabilityCode) {
        if (!((int&)Sing_805DD688())) {
            if (!s_gameCubePadDriverRegistered) {
                *(int*)s_gameCubePadDriverStorage = (int)__vt__Q23gtl12CGCPadDriver;
                __register_global_object(s_gameCubePadDriverStorage, (void*)GameCubePadDriver_DestroyRegisteredInstance,
                                         regmem);
                s_gameCubePadDriverRegistered = 1;
            }
            ((int&)Sing_805DD688()) = (int)s_gameCubePadDriverStorage;
        }
        return (void*)((int&)Sing_805DD688());
    }
    if (id == kInputProviderPointing) {
        if (!((int&)Sing_805DD6A0())) {
            if (!s_pointingDriverRegistered) {
                *(int*)s_pointingDriverStorage = (int)__vt__Q23gtl24CRvlPointingDeviceDriver;
                __register_global_object(s_pointingDriverStorage, (void*)RvlPointingDriver_DestroyRegisteredInstance,
                                         regmem + 0x24);
                s_pointingDriverRegistered = 1;
            }
            ((int&)Sing_805DD6A0()) = (int)s_pointingDriverStorage;
        }
        return (void*)((int&)Sing_805DD6A0());
    }
    if (id == kNunchukDriverCapabilityCode) {
        if (!((int&)Sing_805DD690())) {
            if (!s_freeStylePadDriverRegistered) {
                *(int*)s_freeStylePadDriverStorage = (int)__vt__Q23gtl22CRvlFreeStylePadDriver;
                __register_global_object(s_freeStylePadDriverStorage,
                                         (void*)RvlFreeStyleDriver_DestroyRegisteredInstance, regmem + 0x0c);
                s_freeStylePadDriverRegistered = 1;
            }
            ((int&)Sing_805DD690()) = (int)s_freeStylePadDriverStorage;
        }
        return (void*)((int&)Sing_805DD690());
    }
    if (id == kInputProviderClassic) {
        if (!((int&)Sing_805DD698())) {
            if (!s_classicPadDriverRegistered) {
                *(int*)s_classicPadDriverStorage = (int)__vt__Q23gtl20CRvlClassicPadDriver;
                __register_global_object(s_classicPadDriverStorage, (void*)RvlClassicDriver_DestroyRegisteredInstance,
                                         regmem + 0x18);
                s_classicPadDriverRegistered = 1;
            }
            ((int&)Sing_805DD698()) = (int)s_classicPadDriverStorage;
        }
        return (void*)((int&)Sing_805DD698());
    }
    return 0;
}

// Publish Classic samples as 20 integer values; null reportCursor returns 0 without writing.
// self owns history, reportCursor borrows caller output; status vslot +0x10 returns
// DriverResult. Each sample emits once (even invalid samples); non-ready channels
// emit zeros and return 0. Analog values truncate toward zero after scaling; Y
// negation is nested, not reassociated. Digital held bits become 0/128, not edges.
// reportOptions is actually a borrowed GtlInputReportDescriptor address in the
// legacy int ABI (schema in GtlPadDriverState.h); not a bitmask. Caller supplies
// count * descriptor.reportSize WRITABLE bytes, count=channel sampleCount (0..16).
// Descriptor entryCount<=256, every mapped axis fits four bytes and every button
// one byte inside each report. Unmatched output bytes remain untouched, even
// for a non-ready channel (only the 20 source values are zeroed). Output must not
// alias driver history/descriptor/value ranges, which remain live while writing.
// InputDriver_BuildStateReport (gtl_pad_drivers_gc_classic_freestyle.cpp) synchronously consumes the stack values
// and borrowed descriptor, returning cursor+reportSize; that return is the next
// address, not success. This loop advances exactly once per sample, without an
// async callback or retaining stack/output pointers. Descriptor virtual getters
// must return stable borrowed item/range storage for the call. Final cursor is
// local, not returned to caller. count==0 writes nothing but still returns ready.
int gtl_CRvlClassicPadDriver_BuildStateReports(void* self, void* reportCursor, int reportOptions, int channel) {
    if (!reportCursor)
        return 0;
    char* record = (char*)self + channel * 0x244;
    char* item;
    int count = ((gtl::CRvlClassicPadDriver*)record)->channels[0].sampleCount;
    struct CheckTable {
        virtual void unused0() DECOMP_LOCAL_VIRTUAL;
        virtual void unused1() DECOMP_LOCAL_VIRTUAL;
        virtual void unused2() DECOMP_LOCAL_VIRTUAL;
        virtual void unused3() DECOMP_LOCAL_VIRTUAL;
        virtual int check(int) DECOMP_LOCAL_VIRTUAL;
    };
    int channelStatus = ((CheckTable*)self)->check(channel);
    float scale = kControllerAnalogReportScale;
    item = record + 8;
    float negativeScale = kControllerStickReportYSign;
    int ready = (channelStatus == kDriverReady);
    for (int i = 0; i < count; item += 0x24, ++i) {
        int reportValues[20];
        volatile unsigned int* flags = (volatile unsigned int*)item;
        reportValues[kClassicReportLeftStickX] = (int)(scale * ((GtlClassicPadSample*)item)->leftStickX);
        reportValues[kClassicReportLeftStickY] =
            (int)(negativeScale * (scale * ((GtlClassicPadSample*)item)->leftStickY));
        reportValues[kClassicReportRightStickX] = (int)(scale * ((GtlClassicPadSample*)item)->rightStickX);
        reportValues[kClassicReportRightStickY] =
            (int)(negativeScale * (scale * ((GtlClassicPadSample*)item)->rightStickY));
        reportValues[kClassicReportLeftTrigger] = (int)(scale * ((GtlClassicPadSample*)item)->leftTrigger);
        reportValues[kClassicReportRightTrigger] = (int)(scale * ((GtlClassicPadSample*)item)->rightTrigger);
        reportValues[kClassicReportUp] = (*(unsigned int*)flags & 1) << kButtonPressedShift;
        reportValues[kClassicReportDown] = (*(unsigned int*)flags >> kClassicDown & 1) << kButtonPressedShift;
        reportValues[kClassicReportLeft] = (*(unsigned int*)flags >> kClassicLeft & 1) << kButtonPressedShift;
        reportValues[kClassicReportRight] = (*(unsigned int*)flags >> kClassicRight & 1) << kButtonPressedShift;
        reportValues[kClassicReportPlus] = (*(unsigned int*)flags >> kClassicPlus & 1) << kButtonPressedShift;
        reportValues[kClassicReportMinus] = (*(unsigned int*)flags >> kClassicMinus & 1) << kButtonPressedShift;
        reportValues[kClassicReportHome] = (*(unsigned int*)flags >> kClassicHome & 1) << kButtonPressedShift;
        reportValues[kClassicReportR] = (*(unsigned int*)flags >> kClassicR & 1) << kButtonPressedShift;
        reportValues[kClassicReportA] = (*(unsigned int*)flags >> kClassicA & 1) << kButtonPressedShift;
        reportValues[kClassicReportB] = (*(unsigned int*)flags >> kClassicB & 1) << kButtonPressedShift;
        reportValues[kClassicReportX] = (*(unsigned int*)flags >> kClassicX & 1) << kButtonPressedShift;
        reportValues[kClassicReportY] = (*(unsigned int*)flags >> kClassicY & 1) << kButtonPressedShift;
        reportValues[kClassicReportZL] = (*(unsigned int*)flags >> kClassicZL & 1) << kButtonPressedShift;
        reportValues[kClassicReportZR] = (*(unsigned int*)flags >> kClassicZR & 1) << kButtonPressedShift;
        if (!ready)
            Memory_Set(reportValues, sizeof(reportValues), 0);
        reportCursor = (void*)InputDriver_BuildStateReport(self, reportCursor, reportOptions, reportValues, channel);
    }
    return ready;
}

// Convert each history sample to named report slots; nonready devices zero source values before mapping.
// Same descriptor/output extent/lifetime/advancement contract as Classic above,
// with 22 source values and this pointing channel's sampleCount. reportOptions
// is a descriptor pointer here; zeroing source values still leaves unmatched
// output bytes unchanged. Null cursor returns 0 before any status/history access.
int CRvlPointingDeviceDriver::BuildStateReports(void* reportCursor, void* reportOptions, int channel) {
    if (!reportCursor)
        return 0;
    char* record = (char*)this + channel * 0x304;
    char* item;
    int count = ((GtlPointingDeviceDriver*)record)->channels[0].sampleCount;
    struct CheckTable {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void b() DECOMP_LOCAL_VIRTUAL;
        virtual void c() DECOMP_LOCAL_VIRTUAL;
        virtual void d() DECOMP_LOCAL_VIRTUAL;
        virtual int check(int) DECOMP_LOCAL_VIRTUAL;
    };
    int channelStatus = ((CheckTable*)this)->check(channel);
    float scale = kControllerAnalogReportScale;
    item = record + 8;
    int ready = (channelStatus == kDriverReady);
    for (int i = 0; i < count; item += 0x30, ++i) {
        int reportValues[22] = {};
        reportValues[kPointingReportPointerX] = (int)(scale * ((GtlPointingDeviceSample*)item)->pointerX);
        reportValues[kPointingReportPointerY] = (int)(scale * ((GtlPointingDeviceSample*)item)->pointerY);
        reportValues[kPointingReportHorizonX] = (int)(scale * ((GtlPointingDeviceSample*)item)->horizonX);
        reportValues[kPointingReportHorizonY] = (int)(scale * ((GtlPointingDeviceSample*)item)->horizonY);
        reportValues[kPointingReportAccelerationX] = (int)(scale * ((GtlPointingDeviceSample*)item)->accelerationX);
        reportValues[kPointingReportAccelerationY] = (int)(scale * ((GtlPointingDeviceSample*)item)->accelerationY);
        reportValues[kPointingReportAccelerationZ] = (int)(scale * ((GtlPointingDeviceSample*)item)->accelerationZ);
        reportValues[kPointingReportPointerDistance] = (int)(scale * ((GtlPointingDeviceSample*)item)->pointerDistance);
        reportValues[kPointingReportPointingValidity] = ((GtlPointingDeviceSample*)item)->pointingValidity;
        reportValues[kPointingReportUp] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteUp & 1)
                                          << kButtonPressedShift;
        reportValues[kPointingReportDown] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteDown & 1)
                                            << kButtonPressedShift;
        reportValues[kPointingReportLeft] = (((GtlPointingDeviceSample*)item)->buttons & 1) << kButtonPressedShift;
        reportValues[kPointingReportRight] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteRight & 1)
                                             << kButtonPressedShift;
        reportValues[kPointingReportPlus] = (((GtlPointingDeviceSample*)item)->buttons >> kRemotePlus & 1)
                                            << kButtonPressedShift;
        reportValues[kPointingReportMinus] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteMinus & 1)
                                             << kButtonPressedShift;
        reportValues[kPointingReportHome] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteHome & 1)
                                            << kButtonPressedShift;
        reportValues[kPointingReportA] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteA & 1)
                                         << kButtonPressedShift;
        reportValues[kPointingReportB] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteB & 1)
                                         << kButtonPressedShift;
        reportValues[kPointingReportOne] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteOne & 1)
                                           << kButtonPressedShift;
        reportValues[kPointingReportTwo] = (((GtlPointingDeviceSample*)item)->buttons >> kRemoteTwo & 1)
                                           << kButtonPressedShift;
        if (!ready)
            Memory_Set(reportValues, sizeof(reportValues), 0);
        reportCursor =
            (void*)InputDriver_BuildStateReport(this, reportCursor, (int)reportOptions, reportValues, channel);
    }
    return ready;
}

// Poll GameCube/Wii, then publish GameCube, FreeStyle, Classic and pointing histories in that order.
// Provider virtual slot zero copies borrowed histories into driver-owned
// storage. Return is always 1; provider statuses are queried separately.
int InputDriver_UpdateAll() {
    ObjTable* obj;
    InputDriver_PollGameCubePads();
    Engine_UpdateInput();
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderGameCube);
    obj->fn(s_controllerPollingState);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderFreeStyle);
    obj->fn(s_wiiRemoteChannelHistories);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderClassic);
    obj->fn(s_wiiRemoteChannelHistories);
    obj = (ObjTable*)InputDriver_GetByProviderId(kProviderPointing);
    obj->fn(s_wiiRemoteChannelHistories);
    return 1;
}

// Poll four Wii Remote channels, compact successful samples and update idle state; return 1.
// Classic input; this source retains the stock provider/sample pipeline.
// After polling each channel: samples[0..count) contain status==0 reports in
// read order; unused slots retain cleared payload with disconnected markers.
// All later inactivity checks intentionally inspect slot 0 even when count==0;
// channel 0's missing sample forces activity. The baseline is four XYZ vectors,
// refreshed when the frame counter is zero, including cleared missing channels.
int Engine_UpdateInput() {
    ControllerPollingState* polling = (ControllerPollingState*)s_controllerPollingState;
    KpadChannelHistory* readBuffer;
    KpadChannelHistory* channels;
    int extensionProbe[4];
    int channel;
    int allIdle, channelIndex, channelOffset;
    char* b;
    char* channelBase;
    float* idleAcceleration;
    // Phase 1: clear output histories, poll each channel and mark all spare slots invalid.
    Memory_Clear(&polling->channels[0], 0x2120);
    readBuffer = &polling->readBuffer;
    channels = polling->channels;
    for (channel = 0; channel < 4; channel++) {
        char* outputChannel = (char*)channels + channel * 0x848;
        readBuffer->probeResult = WPADProbe(channel, extensionProbe);
        polling->readBuffer.count = KPADRead(channel, polling->readBuffer.samples, 16);
        int sampleOffset = 0;
        int initializationPass = 2;
        do {
            for (int j = 0; j < 8; j++, sampleOffset += 0x84) {
                char* sampleSlot = outputChannel + sampleOffset;
                ((KpadChannelHistory*)sampleSlot)->samples[0].status = -1;
                ((KpadChannelHistory*)sampleSlot)->samples[0].extensionType = 0xfd;
            }
        } while (--initializationPass);
        int validSampleCount = 0;
        int destinationIndex = 0;
        int i;
        // Phase 2: compact successful samples. Word views keep floating-point
        // bit patterns unchanged; byte status fields are widened
        // explicitly. The full extension payload is copied, then cleared for type 0.
        for (i = 0; i < polling->readBuffer.count; i++) {
            char* destinationSample = outputChannel + destinationIndex * 0x84;
            char* sourceSample = (char*)readBuffer + i * 0x84;
            KpadSampleWords* __restrict dst = (KpadSampleWords*)(destinationSample + 8);
            KpadSampleWords* __restrict src = (KpadSampleWords*)(sourceSample + 8);
            int extensionType;
            int b;
            int a;
            float y, x;
            if (((KpadChannelHistory*)sourceSample)->samples[0].status != 0)
                continue;
            a = *(volatile int*)&src->heldButtons;
            extensionType = *(volatile unsigned char*)&src->extensionType;
            dst->heldButtons = a;
            b = src->pressedButtons;
            a = src->releasedButtons;
            dst->pressedButtons = b;
            b = src->accelerationXBits;
            dst->releasedButtons = a;
            a = src->accelerationYBits;
            dst->accelerationXBits = b;
            b = src->accelerationZBits;
            dst->accelerationYBits = a;
            x = src->accelerationMagnitude;
            dst->accelerationZBits = b;
            y = src->accelerationSpeed;
            dst->accelerationMagnitude = x;
            a = src->pointerXBits;
            dst->accelerationSpeed = y;
            b = src->pointerYBits;
            dst->pointerXBits = a;
            a = src->pointerDeltaXBits;
            dst->pointerYBits = b;
            b = src->pointerDeltaYBits;
            dst->pointerDeltaXBits = a;
            y = src->pointerSpeed;
            dst->pointerDeltaYBits = b;
            a = src->horizonXBits;
            dst->pointerSpeed = y;
            b = src->horizonYBits;
            dst->horizonXBits = a;
            a = src->horizonDeltaXBits;
            dst->horizonYBits = b;
            b = src->horizonDeltaYBits;
            dst->horizonDeltaXBits = a;
            x = src->horizonSpeed;
            dst->horizonDeltaYBits = b;
            y = src->pointerDistance;
            dst->horizonSpeed = x;
            x = src->pointerDistanceDelta;
            dst->pointerDistance = y;
            y = src->pointerDistanceSpeed;
            dst->pointerDistanceDelta = x;
            b = src->verticalAccelerationXBits;
            dst->pointerDistanceSpeed = y;
            a = src->verticalAccelerationYBits;
            dst->verticalAccelerationXBits = b;
            b = src->extensionType;
            dst->verticalAccelerationYBits = a;
            a = (unsigned char)src->status;
            dst->extensionType = b;
            b = (unsigned char)src->pointingValidity;
            dst->status = a;
            a = (unsigned char)src->dataFormat;
            dst->pointingValidity = b;
            b = src->extensionWords[0];
            dst->dataFormat = a;
            a = src->extensionWords[1];
            dst->extensionWords[0] = b;
            b = src->extensionWords[2];
            dst->extensionWords[1] = a;
            a = src->extensionWords[3];
            dst->extensionWords[2] = b;
            b = src->extensionWords[4];
            dst->extensionWords[3] = a;
            a = src->extensionWords[5];
            dst->extensionWords[4] = b;
            b = src->extensionWords[6];
            dst->extensionWords[5] = a;
            a = src->extensionWords[7];
            dst->extensionWords[6] = b;
            b = src->extensionWords[8];
            dst->extensionWords[7] = a;
            dst->extensionWords[8] = b;
            if (extensionType == kRemoteWithoutExtension)
                Memory_Clear(((KpadSampleView*)(destinationSample + 8))->extensionData,
                             sizeof(((KpadSampleView*)0)->extensionData));
            validSampleCount++;
            destinationIndex++;
        }
        ((KpadChannelHistory*)outputChannel)->count = validSampleCount;
        ((KpadChannelHistory*)outputChannel)->probeResult = readBuffer->probeResult;
    }
    // Phase 3: apply a pending refresh and capture the new per-channel baseline.
    if (s_inputIdleBaselineRefreshPending) {
        s_inputIdleFrameCount = 0;
        s_inputIdleBaselineRefreshPending = 0;
    }
    // Snapshot each channel's first acceleration sample when idle tracking restarts.
    if (s_inputIdleFrameCount == 0) {
        KpadChannelHistory* sourceChannels = polling->channels;
        volatile float* dst = (volatile float*)polling->idleAcceleration;
        float x, y, z;
        x = ((KpadSampleView*)&sourceChannels[0].samples[0])->acceleration[0];
        y = ((KpadSampleView*)&sourceChannels[0].samples[0])->acceleration[1];
        dst[0] = x;
        z = ((KpadSampleView*)&sourceChannels[0].samples[0])->acceleration[2];
        dst[1] = y;
        x = ((KpadSampleView*)&sourceChannels[1].samples[0])->acceleration[0];
        dst[2] = z;
        y = ((KpadSampleView*)&sourceChannels[1].samples[0])->acceleration[1];
        dst[3] = x;
        z = ((KpadSampleView*)&sourceChannels[1].samples[0])->acceleration[2];
        dst[4] = y;
        x = ((KpadSampleView*)&sourceChannels[2].samples[0])->acceleration[0];
        dst[5] = z;
        y = ((KpadSampleView*)&sourceChannels[2].samples[0])->acceleration[1];
        dst[6] = x;
        z = ((KpadSampleView*)&sourceChannels[2].samples[0])->acceleration[2];
        dst[7] = y;
        x = ((KpadSampleView*)&sourceChannels[3].samples[0])->acceleration[0];
        dst[8] = z;
        y = ((KpadSampleView*)&sourceChannels[3].samples[0])->acceleration[1];
        dst[9] = x;
        z = ((KpadSampleView*)&sourceChannels[3].samples[0])->acceleration[2];
        dst[10] = y;
        dst[11] = z;
    }
    // Phase 4: classify activity against slot 0 and the baseline, then count idle calls.
    allIdle = 1;
    idleAcceleration = polling->idleAcceleration;
    channelBase = (char*)polling->channels;
    int baselineOffset = 0;
    // Activity means buttons, acceleration speed, tracked-pointer motion, or baseline drift.
    for (channelIndex = 0, channelOffset = 0; channelIndex < 4;
         channelIndex++, channelOffset += 0x848, baselineOffset += 12) {
        b = channelBase + channelOffset;
        if ((channelIndex == 0 && ((KpadChannelHistory*)b)->count == 0) ||
            ((KpadChannelHistory*)b)->samples[0].heldButtons != 0 ||
            ((KpadChannelHistory*)b)->samples[0].accelerationSpeed > kIdleAccelerationSpeedThreshold ||
            (((KpadChannelHistory*)b)->samples[0].pointingValidity == kTrackedPointerValidity &&
             ((KpadChannelHistory*)b)->samples[0].pointerSpeed > kIdlePointerSpeedThreshold) ||
            InputActivityAbsolute(((KpadSampleView*)&((KpadChannelHistory*)b)->samples[0])->acceleration[0] -
                                  *(float*)((char*)idleAcceleration + baselineOffset)) >
                kIdleAccelerationDriftThreshold ||
            InputActivityAbsolute(((KpadSampleView*)&((KpadChannelHistory*)b)->samples[0])->acceleration[1] -
                                  *(float*)((char*)idleAcceleration + baselineOffset + 4)) >
                kIdleAccelerationDriftThreshold ||
            InputActivityAbsolute(((KpadSampleView*)&((KpadChannelHistory*)b)->samples[0])->acceleration[2] -
                                  *(float*)((char*)idleAcceleration + baselineOffset + 8)) >
                kIdleAccelerationDriftThreshold)
            allIdle = 0;
    }
    // More than 1800 idle updates requests a fresh acceleration baseline on the next update.
    int idleFrames;
    if (allIdle) {
        idleFrames = s_inputIdleFrameCount + 1;
        s_inputIdleFrameCount = idleFrames;
    } else
        s_inputIdleFrameCount = idleFrames = 0;
    if (idleFrames > kIdleResetUpdates) {
        s_inputIdleBaselineRefreshPending = 1;
        s_inputIdleFrameCount = 0;
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462720", _ctors_u80436FA0_0, InputDriver_InitializeItemIds);
#pragma pop
