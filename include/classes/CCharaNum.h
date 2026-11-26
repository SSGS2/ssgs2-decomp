#ifndef CLASSES_CCHARANUM_H
#define CLASSES_CCHARANUM_H

#include "game/types.h"

class CCharaNum {
public:
    void* vtbl;                // 0x0: original vtable pointer
    char pad_004[0x14];        // 0x4
    int selectedPlayerCount;   // 0x18
    int selectionComplete;     // 0x1c
    void* layout;              // 0x20
    int bpEntryId;             // 0x24
    int selectionMade;         // 0x28
    int selectionFrames;       // 0x2c
    int state;                 // 0x30
    void* packTextureResource; // 0x34
    void* bpTexture;           // 0x38

    int UpdatePlayerCountSelection();
    void Draw();
    int GetSelectedPlayerCount(int* out);
    void UpdateWhileWaiting();
    void StartCharaCountMenu();
    int PollLoadComplete();

    CCharaNum();
    ~CCharaNum();
};

#define CCHARANUM_OFF(f, o) typedef char CCharaNum_off_##f[((int)&((CCharaNum*)0)->f == (o)) ? 1 : -1]
CCHARANUM_OFF(selectedPlayerCount, 0x18);
CCHARANUM_OFF(selectionComplete, 0x1c);
CCHARANUM_OFF(layout, 0x20);
CCHARANUM_OFF(bpEntryId, 0x24);
CCHARANUM_OFF(selectionMade, 0x28);
CCHARANUM_OFF(selectionFrames, 0x2c);
CCHARANUM_OFF(state, 0x30);
CCHARANUM_OFF(packTextureResource, 0x34);
CCHARANUM_OFF(bpTexture, 0x38);
#undef CCHARANUM_OFF

#endif // CLASSES_CCHARANUM_H
