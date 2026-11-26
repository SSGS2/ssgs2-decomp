// Context8028A588: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_CONTEXT8028A588_H
#define CLASSES_CONTEXT8028A588_H

class Context8028A588 {
public:
    char pad_000[0x10];
    void* resourceBuffer; // 0x10: HomeButton_ShutdownResources
    char pad_014[0x2C];
    void* padSamples; // 0x40: HomeButtonMenu_ProcessState
    char pad_044[0x8];
    unsigned int padExtensionMode; // 0x4C: HomeButtonMenu_ProcessState
    void* nextPadSamples;          // 0x50: HomeButtonMenu_ProcessState
    char pad_054[0x8];
    unsigned int nextPadExtensionMode; // 0x5C: HomeButtonMenu_ProcessState
    char pad_060[0x20];
    void* sharedFile; // 0x80: HomeMenu_LoadResourceBuffers
    char pad_084[0x30];
    int state; // 0xB4: HomeButtonMenu_ProcessState
    char pad_0B8[0x1];
    char homeMenuBlocked;      // 0xB9: HomeButtonMenu_ProcessState
    char savedHomeMenuBlocked; // 0xBA: HomeMenu_SetBlockedState
    char pad_0BB[0x9];
    int scriptEntryId;       // 0xC4: HomeButtonMenu_ProcessState
    float savedStreamVolume; // 0xC8: HomeButtonMenu_ProcessState
    float savedBufferVolume; // 0xCC: HomeButtonMenu_ProcessState
    char pad_0D0[0x4];
    char screenAdjustmentEnabled; // 0xD4: HomeButtonMenu_ProcessState
    char pad_0D5[0x3];
    int soundStopDelayFrames;        // 0xD8: HomeButtonMenu_ProcessState
    unsigned char scriptExitStarted; // 0xDC: HomeButtonMenu_UpdateScript
    char blockedMessageSuppressed;   // 0xDD: HomeButtonMenu_ProcessState
    char pad_0DE[0x2];
    int padProbeStatus;     // 0xE0: HomeButtonMenu_ProcessState
    int nextPadProbeStatus; // 0xE4: HomeButtonMenu_ProcessState
    char pad_0E8[0x38];
    int inputWord120; // 0x120: HomeButtonMenu_ProcessState
    int inputWord124; // 0x124: HomeButtonMenu_ProcessState
};

#define OFF(f, o) typedef char Context8028A588_off_##f[((int)&((Context8028A588*)0)->f == (o)) ? 1 : -1]
OFF(resourceBuffer, 0x10);
OFF(padSamples, 0x40);
OFF(padExtensionMode, 0x4C);
OFF(nextPadSamples, 0x50);
OFF(nextPadExtensionMode, 0x5C);
OFF(sharedFile, 0x80);
OFF(state, 0xB4);
OFF(homeMenuBlocked, 0xB9);
OFF(savedHomeMenuBlocked, 0xBA);
OFF(scriptEntryId, 0xC4);
OFF(savedStreamVolume, 0xC8);
OFF(savedBufferVolume, 0xCC);
OFF(screenAdjustmentEnabled, 0xD4);
OFF(soundStopDelayFrames, 0xD8);
OFF(scriptExitStarted, 0xDC);
OFF(blockedMessageSuppressed, 0xDD);
OFF(padProbeStatus, 0xE0);
OFF(nextPadProbeStatus, 0xE4);
OFF(inputWord120, 0x120);
OFF(inputWord124, 0x124);
#undef OFF

#endif
