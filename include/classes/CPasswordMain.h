#ifndef CLASSES_CPASSWORDMAIN_H
#define CLASSES_CPASSWORDMAIN_H

#include "game/types.h"

class CPasswordMain {
public:
    void* vtbl;                        // 0x0: original vtable pointer
    char pad_004[0x14];                // 0x4
    void* taskMainVtable;              // 0x18
    int passwordPanelEntryId;          // 0x1c
    int backgroundEntryId;             // 0x20
    int titleCaptionEntryId;           // 0x24
    void* textRenderer;                // 0x28
    unsigned char transitionRequested; // 0x2c

    static void OnPreLoadInit();
    static void OnInit();
    static void* GetRTTI();
    void OnLoad();
    void OnProcess();
    static void GetPasswordText(void* selfUnused, void* outputText, unsigned int index);
    void OnDisplay();
    void OnDestroy();
    void BuildTextLabels();

    // Adjust the task-main interface pointer back to the owning actor for destruction.
    void DtorThunk18(int deleteFlag);

    CPasswordMain();
    ~CPasswordMain();
};

#define CPASSWORDMAIN_OFF(f, o) typedef char CPasswordMain_off_##f[((int)&((CPasswordMain*)0)->f == (o)) ? 1 : -1]
CPASSWORDMAIN_OFF(taskMainVtable, 0x18);
CPASSWORDMAIN_OFF(passwordPanelEntryId, 0x1c);
CPASSWORDMAIN_OFF(backgroundEntryId, 0x20);
CPASSWORDMAIN_OFF(titleCaptionEntryId, 0x24);
CPASSWORDMAIN_OFF(textRenderer, 0x28);
CPASSWORDMAIN_OFF(transitionRequested, 0x2c);
#undef CPASSWORDMAIN_OFF

#endif // CLASSES_CPASSWORDMAIN_H
