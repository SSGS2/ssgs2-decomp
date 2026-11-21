// GtlInputDevice: layout-only view owned by gtl_texture_projection_input_device.cpp (gtl_IInputDevice_Ctor through gtl_IInputDevice_ReadItemDataByKind).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_GTLINPUTDEVICE_H
#define CLASSES_GTLINPUTDEVICE_H

class GtlInputDevice {
public:
    int unk_0; // 0x0: gtl_IInputDevice_Ctor initializes to 1
    void* vtbl; // 0x4: gtl_IInputDevice_Ctor installs __vt__Q23gtl12IInputDevice
    int activeChannel; // 0x8: channel copied from state by RefreshDriverState; 5 = unavailable; ReadState passes it to the driver
    void* provider; // 0xc: gtl_IInputDevice_RefreshDriverState passes to InputDriver_GetByProviderId
    void* state; // 0x10: gtl_IInputDevice_RefreshDriverState passes to device vfunc_6
    void* descriptor; // 0x14: gtl_IInputDevice_ResolveItemHandle treats as descriptor; gtl_IInputDevice_SetDescriptor writes an integer
    unsigned int handles[256]; // 0x18: gtl_IInputDevice_Ctor initializes 256 entries; gtl_IInputDevice_ResolveItemHandle indexes them
    unsigned char motorEnabled; // 0x418: gtl_IInputDevice_RefreshDriverState passes to device vfunc_9
    char pad_419[0x3];
    void* deviceResult; // 0x41c: gtl_IInputDevice_RefreshDriverState stores result of device vfunc_6
};

#define OFF(f, o) typedef char GtlInputDevice_off_##f[((int)&((GtlInputDevice*)0)->f == (o)) ? 1 : -1]
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

#endif
