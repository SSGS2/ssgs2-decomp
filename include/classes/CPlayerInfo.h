#ifndef CLASSES_CPLAYERINFO_H
#define CLASSES_CPLAYERINFO_H

#include "game/types.h"

class CPlayerInfo {
public:
    void* vtbl;                      // 0x0: original vtable pointer
    char pad_004[0x14];              // 0x4
    void* bpObject;                  // 0x18
    int bpEntryId;                   // 0x1c
    void* layout;                    // 0x20
    void* numberDraw;                // 0x24
    int textFactory;                 // 0x28
    void* text;                      // 0x2c
    int textStateFlags;              // 0x30
    int unk_34;                      // 0x34
    int textStateStorage;            // 0x38
    int unk_3C;                      // 0x3c
    int unk_40;                      // 0x40
    int unk_44;                      // 0x44
    int unk_48;                      // 0x48
    char pad_04C[0xc];               // 0x4c
    int levelIconIndex;              // 0x58
    int page;                        // 0x5c
    char pad_060[0x4];               // 0x60
    int pageInitialized;             // 0x64
    char pad_068[0xc];               // 0x68
    void* courseTextures;            // 0x74
    void* unavailableCourseTextures; // 0x78
    void* characterTextures;         // 0x7c
    unsigned char nameStringFlags;   // 0x80
    char pad_081[0x3];               // 0x81
    unsigned int nameStringLength;   // 0x84
    char pad_088[0xc];               // 0x88
    void* padSwingCtrl;              // 0x94
    int playerRecord;                // 0x98
    void* texturePack;               // 0x9c
    int previousScriptIndex;         // 0xa0
    int pageEntryId;                 // 0xa4

    void UpdateWhileWaiting();
    int PollLoadComplete();
    void InitializePage();
    int UpdatePageInputAndDispatch();
    void Draw();
    static int SetModeButtonStates(void* obj, int pageIndex);
    static int PopulateStatisticsPage(void* obj, int pageIndex);
    int DrawRecordIcons(int index);
    // Releases the owned resources; deletes this object when freeFlag > 0.
    void* Destroy(int freeFlag);

    CPlayerInfo(int recordPointer);
};

#define CPLAYERINFO_OFF(f, o) typedef char CPlayerInfo_off_##f[((int)&((CPlayerInfo*)0)->f == (o)) ? 1 : -1]
CPLAYERINFO_OFF(bpObject, 0x18);
CPLAYERINFO_OFF(bpEntryId, 0x1c);
CPLAYERINFO_OFF(layout, 0x20);
CPLAYERINFO_OFF(numberDraw, 0x24);
CPLAYERINFO_OFF(textFactory, 0x28);
CPLAYERINFO_OFF(text, 0x2c);
CPLAYERINFO_OFF(textStateFlags, 0x30);
CPLAYERINFO_OFF(unk_34, 0x34);
CPLAYERINFO_OFF(textStateStorage, 0x38);
CPLAYERINFO_OFF(unk_3C, 0x3c);
CPLAYERINFO_OFF(unk_40, 0x40);
CPLAYERINFO_OFF(unk_44, 0x44);
CPLAYERINFO_OFF(unk_48, 0x48);
CPLAYERINFO_OFF(levelIconIndex, 0x58);
CPLAYERINFO_OFF(page, 0x5c);
CPLAYERINFO_OFF(pageInitialized, 0x64);
CPLAYERINFO_OFF(courseTextures, 0x74);
CPLAYERINFO_OFF(unavailableCourseTextures, 0x78);
CPLAYERINFO_OFF(characterTextures, 0x7c);
CPLAYERINFO_OFF(nameStringFlags, 0x80);
CPLAYERINFO_OFF(nameStringLength, 0x84);
CPLAYERINFO_OFF(padSwingCtrl, 0x94);
CPLAYERINFO_OFF(playerRecord, 0x98);
CPLAYERINFO_OFF(texturePack, 0x9c);
CPLAYERINFO_OFF(previousScriptIndex, 0xa0);
CPLAYERINFO_OFF(pageEntryId, 0xa4);
#undef CPLAYERINFO_OFF

#endif // CLASSES_CPLAYERINFO_H
