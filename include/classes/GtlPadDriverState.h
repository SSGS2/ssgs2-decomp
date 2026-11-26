// Layout views owned by gtl_pad_drivers_gc_classic_freestyle.cpp; no virtual functions or generated RTTI.
// Original addresses: 0x804360D0-0x80436FA0; function symbols in
// gtl_pad_drivers_gc_classic_freestyle.cpp identify each use below.
#ifndef CLASSES_GTLPADDRIVERSTATE_H
#define CLASSES_GTLPADDRIVERSTATE_H

struct GtlGCPadChannel {
    int sampleCount;                         // vfunc_6 returns this word; vfunc_0 copies four 0x10-byte records.
    unsigned short buttons;                  // vfunc_5 extracts the sixteen report button values.
    signed char stickX, stickY;              // vfunc_5: horizontal, negated vertical axis.
    signed char substickX, substickY;        // vfunc_5: second horizontal/negated vertical pair.
    unsigned char triggerLeft, triggerRight; // vfunc_5: remaining two unsigned analog axes.
    char pad_C[2];
    signed char status; // vfunc_4 maps 0/-1/-2/-3 to device result codes.
    char pad_F;
};
namespace gtl {
// GameCube pad driver: vtable word, then four channel records.
class CGCPadDriver {
public:
    // Copy the four channel records from src into the driver.
    int CopyChannelRecords(const void* src);
    // Start or stop the motor on the channel.
    int SetMotorEnabled(int enabled, void* channel);
    int GetReportItemCount();
    int GetReportKind();
    int GetSampleCount(int channel);
    int GetCapability(int* capabilityOut);
    char* GetItemDescriptor(int itemIndex);
    char* GetItemRange(int itemIndex, int channel);
    int FillReportFromChannel(int* report, int* descriptor, unsigned int channelIndex);
    int TranslateChannelStatus(int channel);

    void* vtbl;                  // 0x00
    GtlGCPadChannel channels[4]; // 0x04
};
} // namespace gtl

struct GtlFreeStyleSample {
    unsigned int buttons; // vfunc_0 retains 0x6000 (extension Z/C); vfunc_5 extracts them.
    float stickX, stickY; // vfunc_0 copies extension Vec2 at source +0x60; vfunc_5 negates Y.
    float accelerationX, accelerationY, accelerationZ; // vfunc_0 copies extension vector at +0x68.
    int status;                 // vfunc_4 maps status to device result; vfunc_0 sets -1 without extension type 1.
    unsigned int extensionType; // vfunc_0 copies source byte +0x5C; vfunc_4 also recognizes 0xFD.
};
struct GtlFreeStyleHistory {
    int sampleCount;
    GtlFreeStyleSample samples[16];
};
namespace gtl {
// Nunchuk (FreeStyle) pad driver: vtable word, then four channel histories.
class CRvlFreeStylePadDriver {
public:
    // Copy the buffered Nunchuk samples from the SDK history into the driver.
    int ExtractSamples(char* rawHistory);
    // Fill the report for one channel, one value per Nunchuk report item.
    int BuildSampleReports(void* report, void* descriptor, int channel);
    int GetChannelStatus(int channel);
    int GetReportItemCount();
    int GetReportKind();
    int GetSampleCount(int channel);
    int GetCapability(int* capabilityOut);
    char* GetItemDescriptor(int itemIndex);
    char* GetItemRange(int itemIndex, int channel);

    void* vtbl;                      // 0x00
    GtlFreeStyleHistory channels[4]; // 0x04
};
} // namespace gtl

struct GtlInputItem {
    int id; // BuildStateReport/BuildDescriptorMapping match this against a descriptor entry.
    int descriptorAttribute0, descriptorAttribute1; // EnumerateItems copies these into the callback record.
    unsigned int range[4]; // ResetItemRanges copies these to each layer; RemapAxisValue consumes them.
};
struct GtlInputMappingEntry {
    int* itemId;
    int outputOffset; // BuildStateReport adds this byte offset to the caller's report.
    char pad_8[8];
};
struct GtlInputReportDescriptor {
    char pad_0[0xC];
    int reportSize;          // BuildStateReport returns output + reportSize.
    unsigned int entryCount; // Both mapping functions traverse entryCount 0x10-byte records.
    char* entries;           // Keep the original byte cursor and stride in the mapping loops.
};

#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(GtlGCPadChannel, sampleCount, 0x0);
OFF(GtlGCPadChannel, buttons, 0x4);
OFF(GtlGCPadChannel, stickX, 0x6);
OFF(GtlGCPadChannel, stickY, 0x7);
OFF(GtlGCPadChannel, substickX, 0x8);
OFF(GtlGCPadChannel, substickY, 0x9);
OFF(GtlGCPadChannel, triggerLeft, 0xA);
OFF(GtlGCPadChannel, triggerRight, 0xB);
OFF(GtlGCPadChannel, status, 0xE);
typedef char CGCPadDriver_off_vtbl[((int)&((gtl::CGCPadDriver*)0)->vtbl == 0x0) ? 1 : -1];
typedef char CGCPadDriver_off_channels[((int)&((gtl::CGCPadDriver*)0)->channels == 0x4) ? 1 : -1];
OFF(GtlFreeStyleSample, buttons, 0x0);
OFF(GtlFreeStyleSample, stickX, 0x4);
OFF(GtlFreeStyleSample, stickY, 0x8);
OFF(GtlFreeStyleSample, accelerationX, 0xC);
OFF(GtlFreeStyleSample, accelerationY, 0x10);
OFF(GtlFreeStyleSample, accelerationZ, 0x14);
OFF(GtlFreeStyleSample, status, 0x18);
OFF(GtlFreeStyleSample, extensionType, 0x1C);
OFF(GtlFreeStyleHistory, sampleCount, 0x0);
OFF(GtlFreeStyleHistory, samples, 0x4);
typedef char CRvlFreeStylePadDriver_off_vtbl[((int)&((gtl::CRvlFreeStylePadDriver*)0)->vtbl == 0x0) ? 1 : -1];
typedef char CRvlFreeStylePadDriver_off_channels[((int)&((gtl::CRvlFreeStylePadDriver*)0)->channels == 0x4) ? 1 : -1];
OFF(GtlInputItem, id, 0x0);
OFF(GtlInputItem, descriptorAttribute0, 0x4);
OFF(GtlInputItem, descriptorAttribute1, 0x8);
OFF(GtlInputItem, range, 0xC);
OFF(GtlInputMappingEntry, itemId, 0x0);
OFF(GtlInputMappingEntry, outputOffset, 0x4);
OFF(GtlInputReportDescriptor, reportSize, 0xC);
OFF(GtlInputReportDescriptor, entryCount, 0x10);
OFF(GtlInputReportDescriptor, entries, 0x14);
#undef OFF
typedef char GtlGCPadChannel_size[(sizeof(GtlGCPadChannel) == 0x10) ? 1 : -1];
typedef char GtlFreeStyleSample_size[(sizeof(GtlFreeStyleSample) == 0x20) ? 1 : -1];
typedef char GtlFreeStyleHistory_size[(sizeof(GtlFreeStyleHistory) == 0x204) ? 1 : -1];
typedef char CRvlFreeStylePadDriver_size[(sizeof(gtl::CRvlFreeStylePadDriver) == 0x814) ? 1 : -1];
typedef char GtlInputMappingEntry_size[(sizeof(GtlInputMappingEntry) == 0x10) ? 1 : -1];
#endif
