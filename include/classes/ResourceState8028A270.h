// ResourceState8028A270: layout view; methods in small_common_dialog.cpp (HomeButtonMenu_InitializeState, HomeButtonResources_dtor, HomeButton_InitializeSoundArchive; identity unknown).
// Inferred descriptive type name; no vtable, RTTI, constructors or base classes emitted.
#ifndef CLASSES_RESOURCESTATE8028A270_H
#define CLASSES_RESOURCESTATE8028A270_H

class ResourceState8028A270 {
public:
    char pad_000[0x10];
    void* resourceBuffer; // 0x10
    char pad_014[0x18];
    int resourceBufferSize; // 0x2C
    char pad_030[0xC];
    int unk_3C; // 0x3C
    char pad_040[0x4];
    float unk_44; // 0x44
    float unk_48; // 0x48
    int unk_4C;   // 0x4C
    char pad_050[0x4];
    float unk_54; // 0x54
    float unk_58; // 0x58
    int unk_5C;   // 0x5C
    char pad_060[0x4];
    float unk_64; // 0x64
    float unk_68; // 0x68
    int unk_6C;   // 0x6C
    char pad_070[0x4];
    float unk_74;     // 0x74
    float unk_78;     // 0x78
    int unk_7C;       // 0x7C
    void* sharedFile; // 0x80
    char pad_084[0x30];
    int homeMenuState;                   // 0xB4
    unsigned char unk_B8;                // 0xB8
    unsigned char homeMenuDisabled;      // 0xB9
    unsigned char savedHomeMenuDisabled; // 0xBA
    unsigned char unk_BB;                // 0xBB
    int unk_BC;                          // 0xBC
    unsigned char unk_C0;                // 0xC0
    char pad_0C1[0x3];
    int bpEntryId;                       // 0xC4
    float savedStreamVolume;             // 0xC8
    float savedBufferVolume;             // 0xCC
    float unk_D0;                        // 0xD0
    unsigned char homeMenuAdjustEnabled; // 0xD4
    char pad_0D5[0x3];
    int soundStopDelayFrames;                      // 0xD8
    unsigned char blockedNotificationOutroStarted; // 0xDC
    unsigned char blockedNotificationSuppressed;   // 0xDD
};

#define OFF(f, o) typedef char ResourceState8028A270_off_##f[((int)&((ResourceState8028A270*)0)->f == (o)) ? 1 : -1]
OFF(resourceBuffer, 0x10);
OFF(resourceBufferSize, 0x2C);
OFF(unk_3C, 0x3C);
OFF(unk_44, 0x44);
OFF(unk_48, 0x48);
OFF(unk_4C, 0x4C);
OFF(unk_54, 0x54);
OFF(unk_58, 0x58);
OFF(unk_5C, 0x5C);
OFF(unk_64, 0x64);
OFF(unk_68, 0x68);
OFF(unk_6C, 0x6C);
OFF(unk_74, 0x74);
OFF(unk_78, 0x78);
OFF(unk_7C, 0x7C);
OFF(sharedFile, 0x80);
OFF(homeMenuState, 0xB4);
OFF(unk_B8, 0xB8);
OFF(homeMenuDisabled, 0xB9);
OFF(savedHomeMenuDisabled, 0xBA);
OFF(unk_BB, 0xBB);
OFF(unk_BC, 0xBC);
OFF(unk_C0, 0xC0);
OFF(bpEntryId, 0xC4);
OFF(savedStreamVolume, 0xC8);
OFF(savedBufferVolume, 0xCC);
OFF(unk_D0, 0xD0);
OFF(homeMenuAdjustEnabled, 0xD4);
OFF(soundStopDelayFrames, 0xD8);
OFF(blockedNotificationOutroStarted, 0xDC);
OFF(blockedNotificationSuppressed, 0xDD);
#undef OFF

#endif
