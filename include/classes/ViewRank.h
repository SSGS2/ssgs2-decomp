// ViewRank: explicit layout view; no inheritance, virtual methods, or RTTI.
// Field comments cite the original function addresses that establish their use.
#ifndef CLASSES_VIEWRANK_H
#define CLASSES_VIEWRANK_H

#include "game/types.h"

class ViewRank {
public:
    char pad_000[0x18];
    void* layout; // 0x18: ViewRank_dtor/vfunc_7: Layout_Destroy/Update
    void* numberDraw; // 0x1C: ViewRank_dtor: UITextTable_ClearEntries and PointerArray_DeleteValue
    void* messageDraw; // 0x20: ViewRank_dtor: raw +0x20; meaning unknown
    void* taggedText; // 0x24: ViewRank_dtor: raw +0x24; meaning unknown
    int entryId; // 0x28: ViewRank_dtor: CBpCtrl_DeleteEntryById
    int secondaryEntryId; // 0x2C: ViewRank_dtor: CBpCtrl_DeleteEntryById
    int activationDelay; // 0x30: ViewRank_UpdateConfirmOrCancel decrements; sets 4 after activation
    int numberEntriesDirty; // 0x34: ViewRank_Activate: raw +0x34; meaning unknown
    int selectionResult; // 0x38: ViewRank_Activate: raw +0x38; meaning unknown
    int rankRowByCharacter[8]; // 0x3C: ViewRank_DrawEntryText/ViewRank_RefreshPlayerScores: eight indexed UI rank slots
    void* sharedFile; // 0x5C: ViewRank_dtor/vfunc_2: shared-file destroy/load
    void* textureResource; // 0x60: ViewRank_UpdateLayoutAndPollTexture: CPackTextureResource_PollLoad
};

#define VIEWRANK_OFF(f, o) typedef char ViewRank_off_##f[((int)&((ViewRank*)0)->f == (o)) ? 1 : -1]
VIEWRANK_OFF(layout, 0x18);
VIEWRANK_OFF(numberDraw, 0x1C);
VIEWRANK_OFF(messageDraw, 0x20);
VIEWRANK_OFF(taggedText, 0x24);
VIEWRANK_OFF(entryId, 0x28);
VIEWRANK_OFF(secondaryEntryId, 0x2C);
VIEWRANK_OFF(numberEntriesDirty, 0x34);
VIEWRANK_OFF(selectionResult, 0x38);
VIEWRANK_OFF(sharedFile, 0x5C);
VIEWRANK_OFF(textureResource, 0x60);
VIEWRANK_OFF(activationDelay, 0x30);
VIEWRANK_OFF(rankRowByCharacter, 0x3C);
#undef VIEWRANK_OFF

#endif
