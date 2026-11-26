#ifndef CLASSES_CPROJECTG_H
#define CLASSES_CPROJECTG_H

#include "game/types.h"

class CProjectG {
public:
    void* vtbl;                     // 0x0: original vtable pointer
    char pad_004[0xc];              // 0x4
    void* interfaceVtable;          // 0x10
    char pad_014[0xc];              // 0x14
    void* secondaryInterfaceVtable; // 0x20
    int frameUpdateFlags;           // 0x24
    char unk_28;                    // 0x28
    char pad_029[0x3];              // 0x29
    unsigned int projectNameTag;    // 0x2c
    int projectNameLongLength;      // 0x30
    int projectNameHeap;            // 0x34
    char unk_38;                    // 0x38
    char unk_39;                    // 0x39
    char pad_03A[0x2];              // 0x3a
    int string1Tag;                 // 0x3c
    int string1LongLength;          // 0x40
    int string1Heap;                // 0x44
    int string2Tag;                 // 0x48
    int string2LongLength;          // 0x4c
    int string2Heap;                // 0x50
    int string3Tag;                 // 0x54
    int string3LongLength;          // 0x58
    int string3Heap;                // 0x5c
    char pad_060[0x4];              // 0x60
    float timeScale;                // 0x64
    char pad_068[0x8];              // 0x68
    void* allocatedDeviceBlocks;    // 0x70
    char pad_074[0x10];             // 0x74
    int deviceBuckets;              // 0x84
    char pad_088[0x18];             // 0x88
    int projectFlags;               // 0xa0
    char restoreInputPending;       // 0xa4
    char pad_0A5[0x3];              // 0xa5
    int clockOriginMilliseconds;    // 0xa8
    int unk_AC;                     // 0xac
    int deviceFactory;              // 0xb0
    int deviceRegistrations;        // 0xb4
    char pad_0B8[0x4];              // 0xb8
    int extensionProperty;          // 0xbc
    char pad_0C0[0x1fbc];           // 0xc0
    unsigned int unk_207C;          // 0x207c
    char pad_2080[0xc8];            // 0x2080
    void* unk_2148;                 // 0x2148
    char pad_214C[0x64];            // 0x214c
    void* unk_21B0;                 // 0x21b0
    unsigned int unk_21B4;          // 0x21b4
    char pad_21B8[0x34];            // 0x21b8
    unsigned int unk_21EC;          // 0x21ec
    char pad_21F0[0x14];            // 0x21f0
    void* unk_2204;                 // 0x2204
    void* unk_2208;                 // 0x2208

    ~CProjectG();
    void UpdateVirtualHooks();
    static void InitializeGameObjects();
    int GetMillisecondsSinceClockOrigin();

    void Update(float delta);
    int CreateWindowProvider();
    int InitializeLegacyDevices();
    int DispatchRegisteredObjectCallback(void* unused, void* arg1, void* arg2, void* arg3);
    void UpdateDisplayStatus();
    void UpdateRegisteredDevices();
    static void Draw();

    // The extension property at 0xBC, read and written as one word.
    unsigned int GetExtensionProperty();
    void SetExtensionProperty(unsigned int value);
};

#define CPROJECTG_OFF(f, o) typedef char CProjectG_off_##f[((int)&((CProjectG*)0)->f == (o)) ? 1 : -1]
CPROJECTG_OFF(interfaceVtable, 0x10);
CPROJECTG_OFF(secondaryInterfaceVtable, 0x20);
CPROJECTG_OFF(frameUpdateFlags, 0x24);
CPROJECTG_OFF(unk_28, 0x28);
CPROJECTG_OFF(projectNameTag, 0x2c);
CPROJECTG_OFF(projectNameLongLength, 0x30);
CPROJECTG_OFF(projectNameHeap, 0x34);
CPROJECTG_OFF(unk_38, 0x38);
CPROJECTG_OFF(unk_39, 0x39);
CPROJECTG_OFF(string1Tag, 0x3c);
CPROJECTG_OFF(string1LongLength, 0x40);
CPROJECTG_OFF(string1Heap, 0x44);
CPROJECTG_OFF(string2Tag, 0x48);
CPROJECTG_OFF(string2LongLength, 0x4c);
CPROJECTG_OFF(string2Heap, 0x50);
CPROJECTG_OFF(string3Tag, 0x54);
CPROJECTG_OFF(string3LongLength, 0x58);
CPROJECTG_OFF(string3Heap, 0x5c);
CPROJECTG_OFF(timeScale, 0x64);
CPROJECTG_OFF(allocatedDeviceBlocks, 0x70);
CPROJECTG_OFF(deviceBuckets, 0x84);
CPROJECTG_OFF(projectFlags, 0xa0);
CPROJECTG_OFF(restoreInputPending, 0xa4);
CPROJECTG_OFF(clockOriginMilliseconds, 0xa8);
CPROJECTG_OFF(unk_AC, 0xac);
CPROJECTG_OFF(deviceFactory, 0xb0);
CPROJECTG_OFF(deviceRegistrations, 0xb4);
CPROJECTG_OFF(extensionProperty, 0xbc);
CPROJECTG_OFF(unk_207C, 0x207c);
CPROJECTG_OFF(unk_2148, 0x2148);
CPROJECTG_OFF(unk_21B0, 0x21b0);
CPROJECTG_OFF(unk_21B4, 0x21b4);
CPROJECTG_OFF(unk_21EC, 0x21ec);
CPROJECTG_OFF(unk_2204, 0x2204);
CPROJECTG_OFF(unk_2208, 0x2208);
#undef CPROJECTG_OFF

#endif // CLASSES_CPROJECTG_H
