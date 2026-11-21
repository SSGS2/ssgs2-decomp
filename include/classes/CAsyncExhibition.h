// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CASYNCEXHIBITION_H
#define CLASSES_CASYNCEXHIBITION_H

#include "game/types.h"

class CAsyncExhibition {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    int viewportLeft;                          // 0x4: 1 use in CAsyncExhibition_ReplaceExhibition
    int viewportTop;                          // 0x8: 1 use in CAsyncExhibition_ReplaceExhibition
    int viewportRight;                          // 0xc: 1 use in CAsyncExhibition_ReplaceExhibition
    int viewportBottom;                         // 0x10: 1 use in CAsyncExhibition_ReplaceExhibition
    float cameraDistance;                       // 0x14: 3 uses in CAsyncExhibition_ReplaceExhibition, CAsyncExhibition_Ctor
    unsigned char previewMode;               // 0x18: 3 uses in CAsyncExhibition_ReplaceExhibition, CAsyncExhibition_Ctor
    char pad_019[0x3];                  // 0x19
    void* preloadObject;                       // 0x1c: 6 uses in CAsyncExhibition_UpdateLoadingStateAndIndicator, CAsyncExhibition_dtor, CAsyncExhibition_Draw...
    unsigned char preloaderExternallyOwned;               // 0x20: 2 uses in CAsyncExhibition_dtor, CAsyncExhibition_Ctor
    char pad_021[0x3];                  // 0x21
    char displayDescriptor[0xc8];       // 0x24: CAsyncExhibition_Ctor calls ResolvedItemSelection_Clear; CAsyncExhibition_InstallResolvedSelectionCallback supplies it to CExhibition_CopyResolvedAvatarSelection
    void* attachments[5];              // 0xec: CAsyncExhibition_ToggleAttachment checks/updates one of five slots after CExhibition_SetAttachment; ctor clears 0x14 bytes
    float previousPointerX;                      // 0x100: 1 use in CAsyncExhibition_ReplaceExhibition
    float previousPointerY;                      // 0x104: 1 use in CAsyncExhibition_ReplaceExhibition
    float dragStartDepth;                      // 0x108: 1 use in CAsyncExhibition_Ctor
    unsigned char pointerDragActive;              // 0x10c: 1 use in CAsyncExhibition_Ctor
    char pad_10D[0x3];                  // 0x10d
    void* exhibition;                      // 0x110: 10 uses in CAsyncExhibition_dtor, CAsyncExhibition_ReplaceExhibition, CAsyncExhibition_Draw...
    float loadingIndicatorX;                      // 0x114: 1 use in CAsyncExhibition_Ctor
    float loadingIndicatorY;                      // 0x118: 1 use in CAsyncExhibition_Ctor
    int bpEntryId;                        // 0x11c: 5 uses in CAsyncExhibition_dtor, CAsyncExhibition_Draw, CAsyncExhibition_Ctor
    unsigned char loadingIndicatorShown;              // 0x120: 1 use in CAsyncExhibition_Ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x4, 0x24, 0xec, 0x114, 0x11c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CASYNCEXHIBITION_OFF(f, o) typedef char CAsyncExhibition_off_##f[((int)&((CAsyncExhibition*)0)->f == (o)) ? 1 : -1]
CASYNCEXHIBITION_OFF(viewportLeft, 0x4);
CASYNCEXHIBITION_OFF(viewportTop, 0x8);
CASYNCEXHIBITION_OFF(viewportRight, 0xc);
CASYNCEXHIBITION_OFF(viewportBottom, 0x10);
CASYNCEXHIBITION_OFF(cameraDistance, 0x14);
CASYNCEXHIBITION_OFF(previewMode, 0x18);
CASYNCEXHIBITION_OFF(preloadObject, 0x1c);
CASYNCEXHIBITION_OFF(preloaderExternallyOwned, 0x20);
CASYNCEXHIBITION_OFF(displayDescriptor, 0x24);
CASYNCEXHIBITION_OFF(attachments, 0xec);
CASYNCEXHIBITION_OFF(previousPointerX, 0x100);
CASYNCEXHIBITION_OFF(previousPointerY, 0x104);
CASYNCEXHIBITION_OFF(dragStartDepth, 0x108);
CASYNCEXHIBITION_OFF(pointerDragActive, 0x10c);
CASYNCEXHIBITION_OFF(exhibition, 0x110);
CASYNCEXHIBITION_OFF(loadingIndicatorX, 0x114);
CASYNCEXHIBITION_OFF(loadingIndicatorY, 0x118);
CASYNCEXHIBITION_OFF(bpEntryId, 0x11c);
CASYNCEXHIBITION_OFF(loadingIndicatorShown, 0x120);
#undef CASYNCEXHIBITION_OFF

#endif  // CLASSES_CASYNCEXHIBITION_H
