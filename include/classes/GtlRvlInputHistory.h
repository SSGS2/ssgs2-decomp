// History records used by CRvlClassicPadDriver and CRvlPointingDeviceDriver in input_device_drivers.cpp.
// vfunc_6 returns sampleCount; vfunc_4 traverses samples backwards; vfunc_5 converts each sample.
// Layout views only; observed channel strides are 0x244 and 0x304.
#ifndef CLASSES_GTLRVLINPUTHISTORY_H
#define CLASSES_GTLRVLINPUTHISTORY_H
struct GtlClassicPadSample {
    unsigned int buttons; // 0x0: vfunc_5 extracts button bits into the output packet.
    float leftStickX, leftStickY, rightStickX, rightStickY, leftTrigger, rightTrigger;
    int status;                 // 0x1C: vfunc_4 maps 0/-1/-2/-3 to result codes.
    unsigned int extensionType; // 0x20: vfunc_4 tests against 0xFD.
};
struct GtlPointingDeviceSample {
    unsigned int buttons; // 0x0: vfunc_5 extracts button bits into the output packet.
    float accelerationX, accelerationY, accelerationZ, pointerX, pointerY, horizonX, horizonY, pointerDistance;
    int pointingValidity;
    int status;                 // 0x28: vfunc_4 maps 0/-1/-2/-3 to result codes.
    unsigned int extensionType; // 0x2C: vfunc_4 tests against 0xFD.
};
struct GtlClassicPadHistory {
    int sampleCount;
    GtlClassicPadSample samples[16];
};
struct GtlPointingDeviceHistory {
    int sampleCount;
    GtlPointingDeviceSample samples[16];
};
namespace gtl {
// Classic Controller pad driver: vtable word, then four channel histories.
class CRvlClassicPadDriver {
public:
    // Copy the buffered Classic Controller samples from the SDK history into the driver.
    int ExtractSamples(char* rawHistory);
    // Report success for the motor request without issuing hardware I/O.
    int vfunc_9();

    void* vtbl;                       // 0x00
    GtlClassicPadHistory channels[4]; // 0x04
};
} // namespace gtl
struct GtlPointingDeviceDriver {
    void* vtable;
    GtlPointingDeviceHistory channels[4];
};
#define OFF(T, f, o) typedef char T##_off_##f[((int)&((T*)0)->f == (o)) ? 1 : -1]
OFF(GtlClassicPadSample, buttons, 0x0);
OFF(GtlClassicPadSample, leftStickX, 0x4);
OFF(GtlClassicPadSample, leftStickY, 0x8);
OFF(GtlClassicPadSample, rightStickX, 0xC);
OFF(GtlClassicPadSample, rightStickY, 0x10);
OFF(GtlClassicPadSample, leftTrigger, 0x14);
OFF(GtlClassicPadSample, rightTrigger, 0x18);
OFF(GtlClassicPadSample, status, 0x1C);
OFF(GtlClassicPadSample, extensionType, 0x20);
OFF(GtlPointingDeviceSample, buttons, 0x0);
OFF(GtlPointingDeviceSample, accelerationX, 0x4);
OFF(GtlPointingDeviceSample, accelerationY, 0x8);
OFF(GtlPointingDeviceSample, accelerationZ, 0xC);
OFF(GtlPointingDeviceSample, pointerX, 0x10);
OFF(GtlPointingDeviceSample, pointerY, 0x14);
OFF(GtlPointingDeviceSample, horizonX, 0x18);
OFF(GtlPointingDeviceSample, horizonY, 0x1C);
OFF(GtlPointingDeviceSample, pointerDistance, 0x20);
OFF(GtlPointingDeviceSample, pointingValidity, 0x24);
OFF(GtlPointingDeviceSample, status, 0x28);
OFF(GtlPointingDeviceSample, extensionType, 0x2C);
OFF(GtlClassicPadHistory, sampleCount, 0x0);
OFF(GtlClassicPadHistory, samples, 0x4);
OFF(GtlPointingDeviceHistory, sampleCount, 0x0);
OFF(GtlPointingDeviceHistory, samples, 0x4);
OFF(GtlPointingDeviceDriver, vtable, 0x0);
OFF(GtlPointingDeviceDriver, channels, 0x4);
#undef OFF
typedef char CRvlClassicPadDriver_off_channels[((int)&((gtl::CRvlClassicPadDriver*)0)->channels == 0x4) ? 1 : -1];
typedef char GtlClassicPadHistory_size[(sizeof(GtlClassicPadHistory) == 0x244) ? 1 : -1];
typedef char GtlPointingDeviceHistory_size[(sizeof(GtlPointingDeviceHistory) == 0x304) ? 1 : -1];
#endif
