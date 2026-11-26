#ifndef CLASSES_CSHADOWMANAGER_H
#define CLASSES_CSHADOWMANAGER_H

#include "game/types.h"

// Render settings copied into a shadow entry at +0x8C (0x68 bytes). The anchor is
// the caster position; the light direction is stored as float bits and negated on copy.
// Other words and floats keep their offset names until their consumers are established.
struct Src801EF {
    unsigned int anchorXBits, anchorYBits, anchorZBits, unresolvedWord0C, unresolvedWord10, unresolvedWord14,
        unresolvedWord18, unresolvedWord1C, unresolvedWord20, unresolvedWord24, unresolvedWord28, unresolvedWord2C,
        unresolvedWord30, unresolvedWord34, unresolvedWord38, unresolvedWord3C, lightDirectionXBits,
        lightDirectionYBits, lightDirectionZBits;
    float unresolvedFloat4C, unresolvedFloat50, unresolvedFloat54, unresolvedFloat58, unresolvedFloat5C,
        unresolvedFloat60, unresolvedFloat64;
};

class CShadowManager {
public:
    void* vtbl;                   // 0x0: original vtable pointer
    char pad_004[0xc];            // 0x4
    unsigned char selectionFlags; // 0x10
    char pad_011[0x3];            // 0x11
    void* captureQueue;           // 0x14
    float minimumSelectionValue;  // 0x18
    unsigned int entryCount;      // 0x1c
    void* listSentinelNext;       // 0x20
    void* listSentinelPrevious;   // 0x24
    void* selection;              // 0x28
    int projectionDrawArgument;   // 0x2c

    CShadowManager(void*, unsigned char selectionFlags);
    ~CShadowManager();

    // Entries and selection.
    void SetSelectionFlags(unsigned char value);
    static void CopyNamedRecordSettings(CShadowManager* manager, const char* key, const Src801EF* source);
    void ClearEntriesAndResizeCaptureQueue(int width, int height);
    void* ReplaceSelection(const char* key);
    void ReplaceNamedTextureQueueFront(const char* key, void* arg);
    void* GetOrCreateEntry(const char* key, float value, unsigned int flags, const char* copyKey,
                           const Src801EF* source);
};

#define CSHADOWMANAGER_OFF(f, o) typedef char CShadowManager_off_##f[((int)&((CShadowManager*)0)->f == (o)) ? 1 : -1]
CSHADOWMANAGER_OFF(selectionFlags, 0x10);
CSHADOWMANAGER_OFF(captureQueue, 0x14);
CSHADOWMANAGER_OFF(minimumSelectionValue, 0x18);
CSHADOWMANAGER_OFF(entryCount, 0x1c);
CSHADOWMANAGER_OFF(listSentinelNext, 0x20);
CSHADOWMANAGER_OFF(listSentinelPrevious, 0x24);
CSHADOWMANAGER_OFF(selection, 0x28);
CSHADOWMANAGER_OFF(projectionDrawArgument, 0x2c);
#undef CSHADOWMANAGER_OFF

#endif // CLASSES_CSHADOWMANAGER_H
