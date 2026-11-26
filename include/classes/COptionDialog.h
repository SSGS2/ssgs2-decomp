#ifndef CLASSES_COPTIONDIALOG_H
#define CLASSES_COPTIONDIALOG_H

#include "game/types.h"

class COptionDialog {
public:
    void* vtbl;                              // 0x0: original vtable pointer
    char pad_004[0x14];                      // 0x4
    int bpEntryId;                           // 0x18
    unsigned char controlModeOptionsVisible; // 0x1c
    char pad_01D[0x3];                       // 0x1d
    float inputDelay;                        // 0x20
    char savedRecordCopy[0x4e0];             // 0x24
    int streamVolumePercent;                 // 0x504
    int bufferVolumePercent;                 // 0x508
    unsigned int optionFlags;                // 0x50c
    char pad_510[0x33c];                     // 0x510
    void* layout;                            // 0x84c
    void* saveMenu;                          // 0x850
    void* textureResource;                   // 0x854
    int previousScriptIndex;                 // 0x858
    unsigned char closePending;              // 0x85c

    int UpdateAndProcessInput(float dt);
    void UpdateLayoutAndTexture();
    int IsLoaded();
    void Draw();

    void InitializeFromSaveData();
    void RefreshAudioOutputModeButtons(void* saveRecord);
    void RefreshControlModeSelection(void* saveRecord);

    COptionDialog(int arg);

    ~COptionDialog();
};

#define COPTIONDIALOG_OFF(f, o) typedef char COptionDialog_off_##f[((int)&((COptionDialog*)0)->f == (o)) ? 1 : -1]
COPTIONDIALOG_OFF(bpEntryId, 0x18);
COPTIONDIALOG_OFF(controlModeOptionsVisible, 0x1c);
COPTIONDIALOG_OFF(inputDelay, 0x20);
COPTIONDIALOG_OFF(streamVolumePercent, 0x504);
COPTIONDIALOG_OFF(bufferVolumePercent, 0x508);
COPTIONDIALOG_OFF(optionFlags, 0x50c);
COPTIONDIALOG_OFF(layout, 0x84c);
COPTIONDIALOG_OFF(saveMenu, 0x850);
COPTIONDIALOG_OFF(textureResource, 0x854);
COPTIONDIALOG_OFF(previousScriptIndex, 0x858);
COPTIONDIALOG_OFF(closePending, 0x85c);
#undef COPTIONDIALOG_OFF

#endif // CLASSES_COPTIONDIALOG_H
