// Input drivers for GameCube, Wii Classic Controller and Nunchuk pads: report item
// descriptors and ranges, buffered samples, motor requests, and channel status.
//
// The functions that map these items onto the engine's device report are here too.

#include "sdk_protos.h"
#include "classes/GtlInputDevice.h"
#include "classes/GtlPadDriverState.h"
#include "classes/GtlRvlInputHistory.h"
// Driver packet/report field evidence: original 0x804360D0-0x80436FA0.

// Input boundary: channels are 0..3, item indices are below the driver item count.
// Report/descriptor/range storage is borrowed. Counts and offsets are trusted; no
// bounds checks are added here. Button reports use 0 or 0x80, axes use driver units.
// Wii Remote samples arrive in SDK packets; only these extension fields are read here.
struct RvlRawSample {
    unsigned int buttons;
    char remoteData[0x58];
    unsigned char extensionType; // 0x5C: 1 = Nunchuk, 2 = Classic Controller.
    signed char extensionStatus;
    char alignment[2];
    union {
        struct {
            float stick[2];
            float acceleration[3];
            char reserved[0x10];
        } nunchuk;
        struct {
            unsigned int buttons;
            char reserved[8];
            float leftStick[2], rightStick[2], leftTrigger, rightTrigger;
        } classic;
    } extension; // 0x60
};
struct RvlRawHistory {
    int sampleCount, enabled;
    RvlRawSample samples[16];
};
typedef char RvlRawSample_size[(sizeof(RvlRawSample) == 0x84) ? 1 : -1];
typedef char RvlRawHistory_size[(sizeof(RvlRawHistory) == 0x848) ? 1 : -1];

// Report item order is part of the driver ABI.
enum GameCubeReportItem {
    kGcStickX,
    kGcStickY,
    kGcSubstickX,
    kGcSubstickY,
    kGcTriggerLeft,
    kGcTriggerRight,
    kGcUp,
    kGcDown,
    kGcLeft,
    kGcRight,
    kGcA,
    kGcB,
    kGcX,
    kGcY,
    kGcZ,
    kGcStart,
    kGcReportItemCount
};
enum NunchukReportItem {
    kNunchukStickX,
    kNunchukStickY,
    kNunchukAccelerationX,
    kNunchukAccelerationY,
    kNunchukAccelerationZ,
    kNunchukZ,
    kNunchukC,
    kNunchukReportItemCount
};

enum {
    kControllerChannels = 4,
    kHistorySamples = 16,
    kNunchukExtension = 1,
    kClassicExtension = 2,
    kExtensionDisconnected = 0xFD,
    kNunchukButtons = 0x6000,
    kReportButtonDown = 0x80,
    kReportItemLimit = 256,
    kAxisThresholdScale = 10000,
    kPadReportKind = 0x15,
    kInputItemDescriptorBytes = 0x1C,
    kEnumerationRecordBytes = 0x14,
    kDeviceReady = 0,
    kDeviceDisconnected = 1,
    kDeviceNotReady = 2,
    kDeviceTransferError = 3,
    kDeviceUnknownError = 4
};

#pragma use_lmw_stmw on

extern "C" {} // extern "C"

// Copy the registered provider identifier when an output pointer is supplied.
int gtl::IInputDevice::GetProviderId(int* providerOut) {
    if (providerOut == 0)
        return 0;
    *providerOut = (int)provider;
    return 1;
}

extern "C" {} // extern "C"

// Copy the four GameCube channel records into the driver snapshot.
int gtl::CGCPadDriver::CopyChannelRecords(const void* src) {
    Memory_Copy(channels, src, 0x40);
    return 1;
}

extern "C" {
extern void PADControlMotor(void*, int);
} // extern "C"

// Start or stop the GameCube controller motor for the supplied channel.
int gtl::CGCPadDriver::SetMotorEnabled(int enabled, void* channel) {
    if (enabled != 0) {
        PADControlMotor(channel, 1);
    } else {
        PADControlMotor(channel, 0);
    }
    return 1;
}

// Report success for the Classic Controller motor request without issuing hardware I/O.
int gtl::CRvlClassicPadDriver::vfunc_9() {
    return 1;
}

// Return the number of GameCube axes and button report items.
int gtl::CGCPadDriver::GetReportItemCount() {
    return kGcReportItemCount;
}

// Return the input report kind used by this driver.
int gtl::CGCPadDriver::GetReportKind() {
    return kPadReportKind;
}

// Return the Nunchuk report item count: stick XY, acceleration XYZ, and Z/C.
int gtl::CRvlFreeStylePadDriver::GetReportItemCount() {
    return kNunchukReportItemCount;
}

// Return the sample count for a GameCube channel.
int gtl::CGCPadDriver::GetSampleCount(int channel) {
    return channels[channel].sampleCount;
}

extern "C" {

// kGcDriverCapabilityCode = 14
extern int kGcDriverCapabilityCode;

} // extern "C"

// Copy the GameCube driver capability value to the caller.
int gtl::CGCPadDriver::GetCapability(int* capabilityOut) {
    *capabilityOut = kGcDriverCapabilityCode;
    return 1;
}

// Return the buffered Nunchuk sample count for a Wii Remote channel.
int gtl::CRvlFreeStylePadDriver::GetSampleCount(int channel) {
    return channels[channel].sampleCount;
}

// Return the input report kind used by the Nunchuk driver.
int gtl::CRvlFreeStylePadDriver::GetReportKind() {
    return kPadReportKind;
}

extern "C" {

// kNunchukDriverCapabilityCode = 12
extern int kNunchukDriverCapabilityCode;

} // extern "C"

// Copy the Nunchuk driver capability value to the caller.
int gtl::CRvlFreeStylePadDriver::GetCapability(int* capabilityOut) {
    *capabilityOut = kNunchukDriverCapabilityCode;
    return 1;
}

extern "C" {

// kInputDriverZero = 0
extern float kInputDriverZero;
extern int gtl_Vec2_Copy(void*, const void*);
extern int InputDriver_RemapAxisValue(unsigned int*, unsigned int*, int, int);
extern void Memory_Set(void*, int, int);
extern int InputDriver_BuildStateReport(void*, void*, void*, int*, int);
// kControllerAnalogReportScale = 100000
extern float kControllerAnalogReportScale;
// kControllerStickReportYSign = -1
extern float kControllerStickReportYSign;
// kInputAxisThresholdScale = 10000
extern float kInputAxisThresholdScale;
extern int kInputAxisType0, kInputAxisId1, kInputAxisId2, kInputAxisCode3;
// kInputAxisId5 = 5; kInputAxisId6 = 6
extern int kInt_4_inputDriver, kInputAxisId5, kInputAxisId6;
int InputDriver_RemapAxisValue(unsigned int* a, unsigned int* b, int c, int d);
#pragma opt_lifetimes off
// Wire layout stays unsigned; endpoint words are interpreted as signed values.
struct AxisRangeView {
    unsigned int deadZoneThreshold, saturationThreshold;
    int minimum, maximum;
};
typedef char AxisRangeView_size[(sizeof(AxisRangeView) == 0x10) ? 1 : -1];
// Range words: [dead-zone threshold, saturation threshold, signed minimum, signed maximum].
// Thresholds are fractions of 10000. Axis 6 maps [min,max] linearly; IDs 0..5
// normalize each sign separately about zero. The identity fast path returns the input
// before testing endpoints. Non-identity ranges require nonzero input spans and
// distinct thresholds; the caller owns validation. Conversion truncates toward zero.
// Apply dead zone and saturation thresholds, then map an axis into its requested output range.
int InputDriver_RemapAxisValue(unsigned int* outputRangeWords, unsigned int* deviceRangeWords, int axisId,
                               int inputValue) {
    const AxisRangeView* outputRange = (const AxisRangeView*)outputRangeWords;
    const AxisRangeView* deviceRange = (const AxisRangeView*)deviceRangeWords;
    float inputFloat, outputMinimum, outputMaximum, deviceMinimum, deviceMaximum, deadZone, saturation, span;
    if (outputRange->deadZoneThreshold == 0 && outputRange->saturationThreshold == kAxisThresholdScale &&
        outputRange->deadZoneThreshold == deviceRange->deadZoneThreshold &&
        outputRange->saturationThreshold == deviceRange->saturationThreshold)
        return inputValue;
    inputFloat = (float)inputValue;
    outputMinimum = (float)(int)outputRange->minimum;
    outputMaximum = (float)(int)outputRange->maximum;
    deviceMinimum = (float)(int)deviceRange->minimum;
    deviceMaximum = (float)(int)deviceRange->maximum;
    deadZone = (float)outputRange->deadZoneThreshold / (float)kAxisThresholdScale;
    saturation = (float)outputRange->saturationThreshold / (float)kAxisThresholdScale;
    span = outputMaximum - outputMinimum;
    if (outputRange->minimum == 0 && outputRange->maximum == 0)
        return 0;
    // Normalize in device units, apply thresholds, then expand into output units.
    float value;
    if (axisId == kInputAxisId6) {
        float normalized = (inputFloat - deviceMinimum) / (deviceMaximum - deviceMinimum);
        if (normalized <= deadZone)
            value = outputMinimum;
        else if (normalized >= saturation)
            value = outputMaximum;
        else {
            float ratio = (normalized - deadZone) / (saturation - deadZone);
            value = outputMinimum + ratio * span;
        }
    } else if (axisId == kInputAxisType0 || axisId == kInputAxisId1 || axisId == kInputAxisId2 ||
               axisId == kInputAxisCode3 || axisId == kInt_4_inputDriver || axisId == kInputAxisId5) {
        float normalized = (inputFloat >= 0.0f) ? inputFloat / deviceMaximum : inputFloat / deviceMinimum;
        int positive = inputFloat >= 0.0f;
        if (normalized <= deadZone)
            value = 0.0f;
        else if (normalized >= saturation)
            value = positive ? outputMaximum : outputMinimum;
        else {
            float ratio = (normalized - deadZone) / (saturation - deadZone);
            value = positive ? ratio * outputMaximum : ratio * outputMinimum;
        }
    } else {
        return 0;
    }
    return (int)value;
}
#pragma opt_lifetimes reset
} // extern "C"

// Vtable slots called through the driver's interface. Only the slot order matters here.
class RvlFreeStyleSlotInterface {
public:
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual int v4(int index);
};

// Returns 0 for a null report or unavailable channel, 1 for ready. Even unavailable
// channels emit sampleCount zero-valued reports. Reserve count * descriptor.reportSize
// bytes; the report cursor advances once per sample. Y is inverted; analog scale is 100000.
// Convert each Nunchuk sample into seven report values; clear values when the channel is unavailable.
int gtl::CRvlFreeStylePadDriver::BuildSampleReports(void* report, void* descriptor, int channel) {
    if (report == 0)
        return 0;
    GtlFreeStyleHistory* history = &channels[channel];
    int count = history->sampleCount;
    int deviceResult = ((RvlFreeStyleSlotInterface*)this)->v4(channel);
    int ready = !deviceResult;
    float invertY;
    float scale = kControllerAnalogReportScale;
    invertY = kControllerStickReportYSign;
    GtlFreeStyleSample* sample = history->samples;
    for (int sampleIndex = 0; sampleIndex < count; ++sample, ++sampleIndex) {
        int values[kNunchukReportItemCount];
        values[kNunchukStickX] = (int)(scale * sample->stickX);
        values[kNunchukStickY] = (int)(invertY * (scale * sample->stickY));
        values[kNunchukAccelerationX] = (int)(scale * sample->accelerationX);
        values[kNunchukAccelerationY] = (int)(scale * sample->accelerationY);
        values[kNunchukAccelerationZ] = (int)(scale * sample->accelerationZ);
        values[kNunchukZ] = ((sample->buttons >> 13) & 1) * kReportButtonDown;
        values[kNunchukC] = ((sample->buttons >> 14) & 1) * kReportButtonDown;
        if (!ready)
            Memory_Set(values, sizeof(values), 0);
        report = (void*)InputDriver_BuildStateReport(this, report, descriptor, values, channel);
    }
    return ready;
}

extern "C" {

// s_descriptorMappingUsedItemFill
extern unsigned char s_descriptorMappingUsedItemFill;
// s_stateReportUsedItemFill
extern unsigned char s_stateReportUsedItemFill;
// s_descriptorMappingDefaultIndex
extern int s_descriptorMappingDefaultIndex;
extern void* memset(void*, int, unsigned int);
extern int kInputAxisType0, kInputAxisId1, kInputAxisId2, kInputAxisCode3;
extern int kInt_4_inputDriver, kInputAxisId5, kInputAxisId6, kInt_7_inputDriver;
class InputDriverReportInterface {
public:
    virtual void v0();
    virtual void* getItem(unsigned int index);
    virtual void* getOutput(unsigned int index, int layer);
    virtual unsigned int getCount();
    virtual int getState(unsigned int index);
};

// Preconditions: itemCount and entryCount <= 256; values has itemCount integers;
// output has reportSize bytes and each mapped field fits. Unmatched fields stay untouched.
// Each physical item can satisfy only one descriptor entry. Returns the next report
// address as an integer, not a success flag. All pointers remain caller-owned.
// Match descriptor entries to unused device items, remap axes, and write the caller-defined report.
int InputDriver_BuildStateReport(void* self, void* output, void* descriptor, int* values, int channel) {
    // Phase 1: establish the device item set and one-use matching flags.
    // Descriptor offsets are byte offsets in caller-defined reports, so byte addressing is intentional.
    int itemCount = ((InputDriverReportInterface*)self)->getCount();
    int entryCount = (int)((GtlInputReportDescriptor*)descriptor)->entryCount;
    char* entries = ((GtlInputReportDescriptor*)descriptor)->entries;
    // Reserve one match per physical item; duplicate IDs consume successive items.
    unsigned char itemUsed[kReportItemLimit];
    memset(itemUsed, s_stateReportUsedItemFill, sizeof(itemUsed));
    int axisIds[3];
    axisIds[2] = kInputAxisType0;
    axisIds[1] = kInputAxisId1;
    axisIds[0] = kInputAxisId2;
    int axisId3 = kInputAxisCode3;
    int axisId4 = kInt_4_inputDriver;
    int axisId5 = kInputAxisId5;
    int linearAxisId = kInputAxisId6;
    int buttonAxisId = kInt_7_inputDriver;
    GtlInputMappingEntry* entry = (GtlInputMappingEntry*)entries;
    for (unsigned int entryIndex = 0; entryIndex < (unsigned int)entryCount; ++entryIndex, ++entry) {
        // Phase 2: match this descriptor to the first unused physical item.
        int reportOffset = entry->outputOffset;
        for (int itemIndex = 0; (unsigned)itemIndex < (unsigned)itemCount; ++itemIndex) {
            void* item = ((InputDriverReportInterface*)self)->getItem(itemIndex);
            int id = ((GtlInputItem*)item)->id;
            unsigned int* deviceRange = ((GtlInputItem*)item)->range;
            void* outputRange = ((InputDriverReportInterface*)self)->getOutput(itemIndex, channel);
            if (itemUsed[itemIndex] != 0)
                continue;
            if (id != *entry->itemId)
                continue;
            // Phase 3: serialize axes as signed 32-bit words and buttons as bytes.
            // Unknown IDs still consume a match but leave output bytes untouched.
            if (id == axisIds[2] || id == axisIds[1] || id == axisIds[0] || id == axisId3 || id == axisId4 ||
                id == axisId5) {
                *(int*)((char*)output + reportOffset) =
                    InputDriver_RemapAxisValue((unsigned int*)outputRange, deviceRange, id, values[itemIndex]);
            } else if (id == linearAxisId) {
                *(int*)((char*)output + reportOffset) =
                    InputDriver_RemapAxisValue((unsigned int*)outputRange, deviceRange, id, values[itemIndex]);
            } else if (id == buttonAxisId) {
                *(unsigned char*)((char*)output + reportOffset) = (unsigned char)values[itemIndex];
            }
            itemUsed[itemIndex] = 1;
            break;
        }
    }
    return (int)((char*)output + ((GtlInputReportDescriptor*)descriptor)->reportSize);
}
} // extern "C"

// rawHistory contains four SDK histories. A channel with neither enabled nor samples
// is left unchanged; otherwise all 16 sample slots are copied, including unused slots.
// Extract Classic Controller extension samples; preserve the extension type and mark other types disconnected.
int gtl::CRvlClassicPadDriver::ExtractSamples(char* rawHistory) {
    int disconnectedStatus;
    int releasedButtons;
    char* channelHistory;
    char* sourceHistory;
    char* sample;
    char* sourceSample;
    int channel;
    int sampleIndex;
    float zero;
    int buttons;
    unsigned char extensionType;
    unsigned char rawStatus;
    int status;
    float rightTrigger;
    float leftTrigger;

    zero = kInputDriverZero;
    channel = 0;
    releasedButtons = 0;
    disconnectedStatus = -1;
    channelHistory = (char*)channels;
    sourceHistory = rawHistory;
    while (channel < kControllerChannels) {
        if (((RvlRawHistory*)sourceHistory)->enabled != 0 || ((RvlRawHistory*)sourceHistory)->sampleCount != 0) {
            ((GtlClassicPadHistory*)channelHistory)->sampleCount = ((RvlRawHistory*)sourceHistory)->sampleCount;
            sample = channelHistory + 4;
            sourceSample = (char*)((RvlRawHistory*)sourceHistory)->samples;
            sampleIndex = 0;
            while (sampleIndex < kHistorySamples) {
                extensionType = ((RvlRawSample*)sourceSample)->extensionType;
                if (extensionType == kClassicExtension) {
                    buttons = (*(volatile unsigned int*)&((RvlRawSample*)sourceSample)->extension.classic.buttons);
                    // Shared sample fields: left XY, right XY, triggers, and extension type.
                    char* leftStick = (char*)&((GtlClassicPadSample*)sample)->leftStickX;
                    rawStatus = (*(volatile unsigned char*)&((RvlRawSample*)sourceSample)->extensionStatus);
                    float* sourceLeftStick = ((RvlRawSample*)sourceSample)->extension.classic.leftStick;
                    ((GtlClassicPadSample*)sample)->buttons = buttons;
                    leftTrigger = (*(volatile float*)&((RvlRawSample*)sourceSample)->extension.classic.leftTrigger);
                    status = (signed char)rawStatus;
                    rightTrigger = (*(volatile float*)&((RvlRawSample*)sourceSample)->extension.classic.rightTrigger);
                    ((GtlClassicPadSample*)sample)->leftTrigger = leftTrigger;
                    ((GtlClassicPadSample*)sample)->rightTrigger = rightTrigger;
                    ((GtlClassicPadSample*)sample)->extensionType = extensionType;
                    ((GtlClassicPadSample*)sample)->status = status;
                    gtl_Vec2_Copy(leftStick, sourceLeftStick);
                    gtl_Vec2_Copy((char*)&((GtlClassicPadSample*)sample)->rightStickX,
                                  ((RvlRawSample*)sourceSample)->extension.classic.rightStick);
                } else {
                    ((GtlClassicPadSample*)sample)->buttons = releasedButtons;
                    ((GtlClassicPadSample*)sample)->leftTrigger = zero;
                    ((GtlClassicPadSample*)sample)->rightTrigger = zero;
                    ((GtlClassicPadSample*)sample)->extensionType = extensionType;
                    ((GtlClassicPadSample*)sample)->status = disconnectedStatus;
                    ((GtlClassicPadSample*)sample)->leftStickX = zero;
                    ((GtlClassicPadSample*)sample)->leftStickY = zero;
                    ((GtlClassicPadSample*)sample)->rightStickX = zero;
                    ((GtlClassicPadSample*)sample)->rightStickY = zero;
                }
                ++sampleIndex;
                sample += 0x24;
                sourceSample += sizeof(RvlRawSample);
            }
        }
        ++channel;
        channelHistory += 0x244;
        sourceHistory += sizeof(RvlRawHistory);
    }
    return 1;
}

extern "C" {} // extern "C"

// Build six analog and ten button values from a GameCube channel, clearing unavailable input.
int gtl::CGCPadDriver::FillReportFromChannel(int* report, int* descriptor, unsigned int channelIndex) {
    GtlGCPadChannel* channel = &channels[channelIndex];
    int deviceResult = ((InputDriverReportInterface*)this)->getState(channelIndex);
    int ready = !deviceResult;
    int values[kGcReportItemCount];
    values[kGcStickX] = channel->stickX;
    values[kGcStickY] = -channel->stickY;
    values[kGcSubstickX] = channel->substickX;
    values[kGcSubstickY] = -channel->substickY;
    values[kGcTriggerLeft] = channel->triggerLeft;
    values[kGcTriggerRight] = channel->triggerRight;
    values[kGcUp] = (channel->buttons << 4) & kReportButtonDown;
    values[kGcDown] = (channel->buttons << 5) & kReportButtonDown;
    values[kGcLeft] = (channel->buttons << 7) & kReportButtonDown;
    values[kGcRight] = (channel->buttons << 6) & kReportButtonDown;
    values[kGcA] = (channel->buttons >> 1) & kReportButtonDown;
    values[kGcB] = (channel->buttons >> 2) & kReportButtonDown;
    values[kGcX] = (channel->buttons >> 3) & kReportButtonDown;
    values[kGcY] = (channel->buttons >> 4) & kReportButtonDown;
    values[kGcZ] = (channel->buttons << 3) & kReportButtonDown;
    values[kGcStart] = (channel->buttons >> 5) & kReportButtonDown;
    if (!ready)
        Memory_Set(values, sizeof(values), 0);
    InputDriver_BuildStateReport(this, report, descriptor, values, channelIndex);
    return ready;
}

extern "C" {

// mapping needs 256 integers even for a short descriptor: the entire array is
// initialized first. Each output slot receives an item index or the default index.
// Map each descriptor entry to the first unused device item with the same identifier.
int InputDriver_BuildDescriptorMapping(void* self, int* mapping, char* descriptor) {
    unsigned int itemCount = ((InputDriverReportInterface*)self)->getCount();
    unsigned int entryCount = ((GtlInputReportDescriptor*)descriptor)->entryCount;
    GtlInputMappingEntry* entry = (GtlInputMappingEntry*)((GtlInputReportDescriptor*)descriptor)->entries;
    unsigned char itemUsed[kReportItemLimit];
    memset(itemUsed, s_descriptorMappingUsedItemFill, sizeof(itemUsed));
    for (int itemIndex = 0; itemIndex < kReportItemLimit; ++itemIndex)
        mapping[itemIndex] = s_descriptorMappingDefaultIndex;
    for (unsigned int entryIndex = 0; entryIndex < entryCount; ++mapping, ++entryIndex, ++entry) {
        for (unsigned int itemIndex = 0; itemIndex < itemCount; ++itemIndex) {
            void* item = ((InputDriverReportInterface*)self)->getItem(itemIndex);
            int id = ((GtlInputItem*)item)->id;
            if (itemUsed[itemIndex] == 0 && id == *entry->itemId) {
                *mapping = itemIndex;
                itemUsed[itemIndex] = 1;
                break;
            }
        }
    }
    return 1;
}

// gcReportItems
extern char gcReportItems[0x1000];
// nunchukReportItems
extern char nunchukReportItems[0x1000];

} // extern "C"

// Return the GameCube report item descriptor at the requested index.
char* gtl::CGCPadDriver::GetItemDescriptor(int itemIndex) {
    return gcReportItems + itemIndex * kInputItemDescriptorBytes;
}

// Return the Nunchuk report item descriptor at the requested index.
char* gtl::CRvlFreeStylePadDriver::GetItemDescriptor(int itemIndex) {
    return nunchukReportItems + itemIndex * kInputItemDescriptorBytes;
}

extern "C" {

// s_gcReportRanges
extern char s_gcReportRanges[0x4000];
// s_nunchukReportRanges
extern char s_nunchukReportRanges[0x4000];

// Range storage is item-major: itemIndex * (4 channels * 16 bytes) + channel * 16.
// BuildStateReport and ResetItemRanges call vtable slot 2 with (itemIndex, channel).
// Returned ranges are borrowed, mutable, and contain four words.
} // extern "C"

// Return a GameCube item range for the requested channel and item.
char* gtl::CGCPadDriver::GetItemRange(int itemIndex, int channel) {
    return (char*)s_gcReportRanges + (itemIndex << 6) + (channel << 4);
}

// Return a Nunchuk item range for the requested channel and item.
char* gtl::CRvlFreeStylePadDriver::GetItemRange(int itemIndex, int channel) {
    return (char*)s_nunchukReportRanges + (itemIndex << 6) + (channel << 4);
}

// Copy the last device result when an output pointer is supplied.
int gtl::IInputDevice::GetDeviceResult(int* resultOut) {
    if (resultOut == 0)
        return 0;
    *resultOut = (int)deviceResult;
    return 1;
}

extern "C" {} // extern "C"

// Translate the SDK channel status into the engine device result.
int gtl::CGCPadDriver::TranslateChannelStatus(int channel) {
    signed char status = channels[channel].status;
    switch (status) {
    case 0:
        return kDeviceReady;
    case -1:
        return kDeviceDisconnected;
    case -2:
        return kDeviceNotReady;
    case -3:
        return kDeviceTransferError;
    default:
        return kDeviceUnknownError;
    }
}

extern "C" {

extern void* InputDriver_GetByProviderId(void*);

// Slot order mirrors the provider vtable; the last slot requests a motor state.
class W4 {
public:
    virtual void v08();
    virtual void v0C();
    virtual void v10();
    virtual void v14();
    virtual void v18();
    virtual void v1C();
    virtual void v20();
    virtual void v24();
    virtual void v28();
    virtual int v2C(unsigned char, void*);
};
// Request a motor state from the provider and remember it only if the provider accepts.
int PadManager_SetMotorEnabled(char* device, unsigned char motorEnabled) {
    void* driver = InputDriver_GetByProviderId(((gtl::IInputDevice*)device)->provider);
    if (((W4*)driver)->v2C(motorEnabled, (void*)((gtl::IInputDevice*)device)->activeChannel) != 0) {
        ((gtl::IInputDevice*)device)->motorEnabled = motorEnabled;
        return 1;
    }
    return 0;
}

class InputDriverItemInterface {
public:
    virtual void v0();
    virtual void* getItem(unsigned int index);
    virtual void* getOutput(unsigned int index, int layer);
    virtual unsigned int getCount();
};

// The five-word callback record is stack storage: [byte size, ID, attribute0,
// attribute1, zero]. A callback must copy any data it retains beyond the call.
// Call the optional callback with each item identifier and its two descriptor attributes.
int InputDriver_EnumerateItems(void* self, void (*callback)(void*)) {
    unsigned int count = ((InputDriverItemInterface*)self)->getCount();
    for (unsigned int i = 0; i < count; ++i) {
        void* item = ((InputDriverItemInterface*)self)->getItem(i);
        int entry[5];
        entry[0] = kEnumerationRecordBytes;
        entry[1] = ((GtlInputItem*)item)->id;
        entry[2] = ((GtlInputItem*)item)->descriptorAttribute0;
        entry[3] = ((GtlInputItem*)item)->descriptorAttribute1;
        entry[4] = 0;
        if (callback)
            callback(entry);
    }
    return 1;
}

extern int Vector_CopyComponents(void*, const void*);
extern int gtl_Vec2_Copy(void*, const void*);
extern float kInputDriverZero;

} // extern "C"

// Copies all 16 slots only for active/nonempty raw channels. Other channels retain
// their old histories. The sampleCount word, not the copied capacity, controls reporting.
// Extract Nunchuk Z/C, stick, and acceleration data; clear analog data for other extension types.
int gtl::CRvlFreeStylePadDriver::ExtractSamples(char* rawHistory) {
    int buttons;
    unsigned char extensionType;
    float zero = kInputDriverZero;
    char* channelHistory = (char*)channels;
    char* sourceHistory = rawHistory;
    char* sample;
    char* sourceSample;
    int channel = 0;
    for (; channel < kControllerChannels; ++channel) {
        int enabled = ((RvlRawHistory*)sourceHistory)->enabled;
        if (enabled != 0 || ((RvlRawHistory*)sourceHistory)->sampleCount != 0) {
            ((GtlFreeStyleHistory*)channelHistory)->sampleCount = ((RvlRawHistory*)sourceHistory)->sampleCount;
            sample = channelHistory + 4;
            sourceSample = (char*)((RvlRawHistory*)sourceHistory)->samples;
            for (int sampleIndex = 0; sampleIndex < kHistorySamples; ++sampleIndex) {
                buttons = ((RvlRawSample*)sourceSample)->buttons;
                extensionType = ((RvlRawSample*)sourceSample)->extensionType;
                ((GtlFreeStyleSample*)sample)->buttons = buttons & kNunchukButtons;
                ((GtlFreeStyleSample*)sample)->extensionType = extensionType;
                if (extensionType == kNunchukExtension) {
                    ((GtlFreeStyleSample*)sample)->status = ((RvlRawSample*)sourceSample)->extensionStatus;
                    Vector_CopyComponents(&((GtlFreeStyleSample*)sample)->accelerationX,
                                          ((RvlRawSample*)sourceSample)->extension.nunchuk.acceleration);
                    gtl_Vec2_Copy(&((GtlFreeStyleSample*)sample)->stickX,
                                  ((RvlRawSample*)sourceSample)->extension.nunchuk.stick);
                } else {
                    ((GtlFreeStyleSample*)sample)->status = -1;
                    ((GtlFreeStyleSample*)sample)->accelerationX = zero;
                    ((GtlFreeStyleSample*)sample)->accelerationY = zero;
                    ((GtlFreeStyleSample*)sample)->accelerationZ = zero;
                    ((GtlFreeStyleSample*)sample)->stickX = zero;
                    ((GtlFreeStyleSample*)sample)->stickY = zero;
                }
                sample += sizeof(GtlFreeStyleSample);
                sourceSample += sizeof(RvlRawSample);
            }
        }
        channelHistory += sizeof(GtlFreeStyleHistory);
        sourceHistory += sizeof(RvlRawHistory);
    }
    return 1;
}

// A ready sample wins immediately. If none is ready, return the oldest examined
// sample's mapped result; an empty history returns unknown. sampleCount must be <= 16.
// Search buffered samples newest first for a usable Nunchuk status, recognizing disconnected extensions.
int gtl::CRvlFreeStylePadDriver::GetChannelStatus(int channel) {
    // Cursors precede the typed object by the driver vtable word (+4).
    // Keep that bias when walking backwards; stride comes from the sample layout.
    char* historyCursor = (char*)this + channel * sizeof(GtlFreeStyleHistory);
    int sampleCount = ((GtlFreeStyleHistory*)(historyCursor + 4))->sampleCount;
    if (sampleCount < 1)
        return kDeviceUnknownError;
    int result = kDeviceUnknownError;
    char* sampleCursor = historyCursor + 4 + (sampleCount - 1) * sizeof(GtlFreeStyleSample);
    for (int sampleIndex = sampleCount - 1; sampleIndex >= 0; --sampleIndex) {
        int status = ((GtlFreeStyleSample*)(sampleCursor + 4))->status;
        unsigned int extensionType = ((GtlFreeStyleSample*)(sampleCursor + 4))->extensionType;
        switch (status) {
        case 0:
            result = kDeviceReady;
            break;
        case -1:
            result = kDeviceDisconnected;
            break;
        case -2:
            result = kDeviceNotReady;
            break;
        case -3:
            result = kDeviceTransferError;
            break;
        case -4:
        default:
            result = kDeviceUnknownError;
            break;
        }
        if (extensionType == kExtensionDisconnected)
            result = kDeviceDisconnected;
        if (result == 0)
            return result;
        sampleCursor -= sizeof(GtlFreeStyleSample);
    }
    return result;
}

extern "C" {

// Restore each item range for all four controller channels.
int InputDriver_ResetItemRanges(void* self) {
    unsigned int count = ((InputDriverItemInterface*)self)->getCount();
    for (int layer = 0; layer < kControllerChannels; ++layer) {
        for (unsigned int i = 0; i < count; ++i) {
            void* item = ((InputDriverItemInterface*)self)->getItem(i);
            int* output = (int*)((InputDriverItemInterface*)self)->getOutput(i, layer);
            output[0] = (int)((GtlInputItem*)item)->range[0];
            output[1] = (int)((GtlInputItem*)item)->range[1];
            output[2] = (int)((GtlInputItem*)item)->range[2];
            output[3] = (int)((GtlInputItem*)item)->range[3];
        }
    }
    return 1;
}

} // extern "C"
