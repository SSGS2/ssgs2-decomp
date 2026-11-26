#ifndef CLASSES_CINGAMEFADEDIALOG_H
#define CLASSES_CINGAMEFADEDIALOG_H

#include "game/types.h"

class CInGameFadeDialog {
public:
    void* vtbl;                             // 0x0: original vtable pointer
    char pad_004[0x20];                     // 0x4
    float fadeOpacity;                      // 0x24
    char pad_028[0x24];                     // 0x28
    void* bpEntryId;                        // 0x4c
    unsigned int scriptNameStorageFlags;    // 0x50
    char pad_054[0x4];                      // 0x54
    void* scriptNameHeap;                   // 0x58
    void* groupVisibilityOverrides;         // 0x5c
    int groupVisibilityOverrideCount;       // 0x60
    char pad_064[0x4];                      // 0x64
    unsigned int textGroupNameStorageFlags; // 0x68
    char pad_06C[0x4];                      // 0x6c
    void* textGroupNameHeap;                // 0x70
    int scriptStarted;                      // 0x74

    void StartScriptAndSetGroups();
    void Draw();
    void CompleteClose();
    int vfunc_8();
    void* GetScriptStarted();
    int ResetScriptStarted();
    static void vfunc_4(void* arg);

    int Create(void** out);

    ~CInGameFadeDialog();
};

#define CINGAMEFADEDIALOG_OFF(f, o) \
    typedef char CInGameFadeDialog_off_##f[((int)&((CInGameFadeDialog*)0)->f == (o)) ? 1 : -1]
CINGAMEFADEDIALOG_OFF(fadeOpacity, 0x24);
CINGAMEFADEDIALOG_OFF(bpEntryId, 0x4c);
CINGAMEFADEDIALOG_OFF(scriptNameStorageFlags, 0x50);
CINGAMEFADEDIALOG_OFF(scriptNameHeap, 0x58);
CINGAMEFADEDIALOG_OFF(groupVisibilityOverrides, 0x5c);
CINGAMEFADEDIALOG_OFF(groupVisibilityOverrideCount, 0x60);
CINGAMEFADEDIALOG_OFF(textGroupNameStorageFlags, 0x68);
CINGAMEFADEDIALOG_OFF(textGroupNameHeap, 0x70);
CINGAMEFADEDIALOG_OFF(scriptStarted, 0x74);
#undef CINGAMEFADEDIALOG_OFF

#endif // CLASSES_CINGAMEFADEDIALOG_H
