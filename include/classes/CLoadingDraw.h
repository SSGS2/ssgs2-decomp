// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CLOADINGDRAW_H
#define CLASSES_CLOADINGDRAW_H

#include "game/types.h"

class CLoadingDraw {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* messageFile;                       // 0x18: 5 uses in CLoadingDraw_ResetMessage, CLoadingDraw_Ctor, CLoadingDraw_BeginLoadingScreen...
    void* messageEcbResource;                       // 0x1c: 2 uses in CLoadingDraw_dtor
    int bpEntryId;                         // 0x20: 18 uses in CLoadingDraw_BeginLoadingScreen, CLoadingDraw_UpdateMessagePagingAndContinue, CLoadingDraw_dtor...
    void* backgroundPane;                       // 0x24: 6 uses in CLoadingDraw_dtor, CLoadingDraw_DrawBackgroundBpAndMessage, CLoadingDraw_Ctor...
    void* messageDraw;                       // 0x28: 17 uses in CLoadingDraw_UpdateMessagePagingAndContinue, CLoadingDraw_BeginLoadingScreen, CLoadingDraw_DrawBackgroundBpAndMessage...
    int messageLineIndex;                         // 0x2c: 6 uses in CLoadingDraw_UpdateMessagePagingAndContinue, CLoadingDraw_BeginLoadingScreen
    int promptAlphaPhase;                         // 0x30: 5 uses in CLoadingDraw_UpdateMessagePagingAndContinue, CLoadingDraw_BeginLoadingScreen
    int readyToContinue;                         // 0x34: 4 uses in CLoadingDraw_MarkReadyAndShowContinuePrompt, CLoadingDraw_UpdateMessagePagingAndContinue, CLoadingDraw_BeginLoadingScreen
    int unk_38;                         // 0x38: 1 use in CLoadingDraw_BeginLoadingScreen
    void* texturePack;                       // 0x3c: 8 uses in CLoadingDraw_PollResourcesLoaded, CLoadingDraw_BeginLoadingScreen, CLoadingDraw_dtor...

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1c, 0x20
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
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

#endif  // CLASSES_CLOADINGDRAW_H
