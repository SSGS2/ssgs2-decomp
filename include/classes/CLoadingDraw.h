#ifndef CLASSES_CLOADINGDRAW_H
#define CLASSES_CLOADINGDRAW_H

#include "game/types.h"

class CLoadingDraw {
public:
    void* vtbl;               // 0x0: original vtable pointer
    char pad_004[0x14];       // 0x4
    void* messageFile;        // 0x18
    void* messageEcbResource; // 0x1c
    int bpEntryId;            // 0x20
    void* backgroundPane;     // 0x24
    void* messageDraw;        // 0x28
    int messageLineIndex;     // 0x2c
    int promptAlphaPhase;     // 0x30
    int readyToContinue;      // 0x34
    int unk_38;               // 0x38
    void* texturePack;        // 0x3c

    CLoadingDraw();

    int UpdateMessagePagingAndContinue();
    void BeginLoadingScreen();
    void MarkReadyAndShowContinuePrompt();
    void ResetMessage();
    int PollResourcesLoaded();
    void SelectAndShowMessage();
    void DrawBackgroundBpAndMessage();

    ~CLoadingDraw();
};

#define CLOADINGDRAW_OFF(f, o) typedef char CLoadingDraw_off_##f[((int)&((CLoadingDraw*)0)->f == (o)) ? 1 : -1]
CLOADINGDRAW_OFF(messageFile, 0x18);
CLOADINGDRAW_OFF(messageEcbResource, 0x1c);
CLOADINGDRAW_OFF(bpEntryId, 0x20);
CLOADINGDRAW_OFF(backgroundPane, 0x24);
CLOADINGDRAW_OFF(messageDraw, 0x28);
CLOADINGDRAW_OFF(messageLineIndex, 0x2c);
CLOADINGDRAW_OFF(promptAlphaPhase, 0x30);
CLOADINGDRAW_OFF(readyToContinue, 0x34);
CLOADINGDRAW_OFF(unk_38, 0x38);
CLOADINGDRAW_OFF(texturePack, 0x3c);
#undef CLOADINGDRAW_OFF

#endif // CLASSES_CLOADINGDRAW_H
