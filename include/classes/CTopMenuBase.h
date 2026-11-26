#ifndef CLASSES_CTOPMENUBASE_H
#define CLASSES_CTOPMENUBASE_H

#include "game/types.h"

class CTopMenuBase {
public:
    void* vtbl;                          // 0x0: original vtable pointer
    void* layout;                        // 0x4
    void* bpObject;                      // 0x8
    int result;                          // 0xc
    unsigned char resultReady;           // 0x10
    unsigned char backRequested;         // 0x11
    char pad_012[0x2];                   // 0x12
    int menuItemCount;                   // 0x14
    unsigned char scriptRunning;         // 0x18
    char pad_019[0x3];                   // 0x19
    void* message;                       // 0x1c
    int optionDialog;                    // 0x20
    unsigned char optionDialogRequested; // 0x24
    char pad_025[0x3];                   // 0x25
    int bpEntryId;                       // 0x28
    int elementIds;                      // 0x2c
    unsigned char borrowsResources;      // 0x30
    char pad_031[0x3];                   // 0x31
    void* textureResource;               // 0x34

    int RequestOptionsDialog();
    int HitTestPlayer(int index);
    int UpdateHoverElementPairs();
    int PollLoad();
    int HideItemGroup(int index);
    int SyncItemPosition(int index);
    int ResetItemPlayerStates();
    int GetFirstActivatedElement();
    void ReleaseTextureResource();
    static int EnablePreviousElement(void* menu, int index);
    int ProcessInput();
    CTopMenuBase* InitMembers(int layoutAddress, int bpObjectAddress, int itemCount, void* elementIdTable);
    int UpdateHoverMessage(int* indices);

    CTopMenuBase(void* layoutName, void* unused, void* bpResourceName, void* textureName, int itemCount,
                 void* elementIdTable);
};

#define CTOPMENUBASE_OFF(f, o) typedef char CTopMenuBase_off_##f[((int)&((CTopMenuBase*)0)->f == (o)) ? 1 : -1]
CTOPMENUBASE_OFF(layout, 0x4);
CTOPMENUBASE_OFF(bpObject, 0x8);
CTOPMENUBASE_OFF(result, 0xc);
CTOPMENUBASE_OFF(resultReady, 0x10);
CTOPMENUBASE_OFF(backRequested, 0x11);
CTOPMENUBASE_OFF(menuItemCount, 0x14);
CTOPMENUBASE_OFF(scriptRunning, 0x18);
CTOPMENUBASE_OFF(message, 0x1c);
CTOPMENUBASE_OFF(optionDialog, 0x20);
CTOPMENUBASE_OFF(optionDialogRequested, 0x24);
CTOPMENUBASE_OFF(bpEntryId, 0x28);
CTOPMENUBASE_OFF(elementIds, 0x2c);
CTOPMENUBASE_OFF(borrowsResources, 0x30);
CTOPMENUBASE_OFF(textureResource, 0x34);
#undef CTOPMENUBASE_OFF

#endif // CLASSES_CTOPMENUBASE_H
