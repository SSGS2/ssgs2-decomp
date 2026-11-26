// gtl::IInputDevice: one input device bound to a driver provider and a controller channel.
// It has no virtual methods; its vtable pointer is installed by Init.
#ifndef CLASSES_GTLINPUTDEVICE_H
#define CLASSES_GTLINPUTDEVICE_H

struct DriverEntry;

namespace gtl {

class IInputDevice {
public:
    // Creation and destruction
    static int Create(const DriverEntry* entry, void** destination);
    void Init(const unsigned int* providerAndChannel);
    void* Destroy(int deleting);

    // Driver queries
    int RefreshDriverState();
    int SetDescriptor(int value);
    void* EnumerateItems(void* a, void* b);
    int ReadState(unsigned int stateBytes, unsigned int outputAddress);
    int ResolveItemHandle(void** handleOut, void* itemQuery);
    int ReadItemDataByKind(int valueKind, void* queryAndValue);
    int GetDeviceResult(int* resultOut);
    int GetProviderId(int* providerOut);

    int unk_0;                  // 0x0: set to 1 by Init
    void* vtbl;                 // 0x4: installed by Init
    int activeChannel;          // 0x8: channel copied from state by RefreshDriverState; 5 = unavailable
    void* provider;             // 0xc: passed to InputDriver_GetByProviderId
    void* state;                // 0x10: controller channel handed to the driver by RefreshDriverState
    void* descriptor;           // 0x14: installed by SetDescriptor; ResolveItemHandle reads it as a descriptor
    unsigned int handles[256];  // 0x18: provider handle per descriptor item
    unsigned char motorEnabled; // 0x418: motor request, re-sent by RefreshDriverState
    char pad_419[0x3];
    void* deviceResult; // 0x41c: result of the last driver sample count
};

} // namespace gtl

#define OFF(f, o) typedef char GtlInputDevice_off_##f[((int)&((gtl::IInputDevice*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(vtbl, 0x4);
OFF(activeChannel, 0x8);
OFF(provider, 0xc);
OFF(state, 0x10);
OFF(descriptor, 0x14);
OFF(handles, 0x18);
OFF(motorEnabled, 0x418);
OFF(deviceResult, 0x41c);
#undef OFF
typedef char GtlInputDevice_size[(sizeof(gtl::IInputDevice) == 0x420) ? 1 : -1];

#endif
