#ifndef CLASSES_CBGMCOLLECTION_H
#define CLASSES_CBGMCOLLECTION_H

#include "game/types.h"

class CBgmCollection {
public:
    void* vtbl;                 // 0x0: original vtable pointer
    char pad_004[0x14];         // 0x4
    int bpEntryId;              // 0x18: CBgmCollection_Ctor (0x80317EF0), CBpCtrl_CreateEntry out parameter.
    void* layout;               // 0x1C: ctor allocates Controller and calls Layout_LoadExtended.
    void* trackTable;           // 0x20: ctor clears; dtor releases virtual object.
    void* messageResource;      // 0x24: ctor passes address to WFixedFont_CreateMsgEcbResource (CChatMsg resource).
    void* chatResource;         // 0x28: ctor copies CChatMsg::defaultFixedFont.
    void* messageDraw;          // 0x2C: ctor passes address to WFixedFont_CreateAndRegisterMsgDraw, mode 1.
    int selectedTrackIndex;     // 0x30: ctor clears.
    void* textureState;         // 0x34: ctor allocates State initialized by CBpTex::CBpTex.
    int finished;               // 0x38: ctor clears.
    int buttonDelayActive;      // 0x3C: ctor clears.
    int buttonDelayUpdateCount; // 0x40: ctor clears.
    int inputInitialized;       // 0x44: ctor clears.
    unsigned int stringFlags;   // 0x48: ctor calls String_ReplaceRange at this address.
    unsigned int stringLength;  // 0x4C: middle word of the 12-byte string.
    void* stringData;           // 0x50
    int exitRequested;          // 0x54: ctor clears.
    char playerRecord[0x828];   // 0x58: ctor calls SaveData_InitPlayerRecord and copies Data here.
    void* sharedFile;           // 0x880: ctor stores SharedFile_Create result; dtor calls CSharedFileHandle_Destroy.
    void* textureResource;      // 0x884: ctor stores CPackTextureResource_Init object.

    void Draw();
    void UpdateLayoutAndTexture();
    unsigned AreResourcesLoaded();
    int UpdateTrackInfo(int trackIndex);
    int UpdateAndHandleInput();
    void StartMenu();

    CBgmCollection();
    void* Destroy(int deleteStorage);
};

#define CBGMCOLLECTION_OFF(f, o) typedef char CBgmCollection_off_##f[((int)&((CBgmCollection*)0)->f == (o)) ? 1 : -1]
CBGMCOLLECTION_OFF(stringData, 0x50);
CBGMCOLLECTION_OFF(bpEntryId, 0x18);
CBGMCOLLECTION_OFF(layout, 0x1C);
CBGMCOLLECTION_OFF(trackTable, 0x20);
CBGMCOLLECTION_OFF(messageResource, 0x24);
CBGMCOLLECTION_OFF(chatResource, 0x28);
CBGMCOLLECTION_OFF(messageDraw, 0x2C);
CBGMCOLLECTION_OFF(selectedTrackIndex, 0x30);
CBGMCOLLECTION_OFF(textureState, 0x34);
CBGMCOLLECTION_OFF(finished, 0x38);
CBGMCOLLECTION_OFF(buttonDelayActive, 0x3C);
CBGMCOLLECTION_OFF(buttonDelayUpdateCount, 0x40);
CBGMCOLLECTION_OFF(inputInitialized, 0x44);
CBGMCOLLECTION_OFF(stringFlags, 0x48);
CBGMCOLLECTION_OFF(stringLength, 0x4C);
CBGMCOLLECTION_OFF(exitRequested, 0x54);
CBGMCOLLECTION_OFF(playerRecord, 0x58);
CBGMCOLLECTION_OFF(sharedFile, 0x880);
CBGMCOLLECTION_OFF(textureResource, 0x884);
#undef CBGMCOLLECTION_OFF

#endif // CLASSES_CBGMCOLLECTION_H
