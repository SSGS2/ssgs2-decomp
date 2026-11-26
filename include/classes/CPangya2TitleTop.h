#ifndef CLASSES_CPANGYA2TITLETOP_H
#define CLASSES_CPANGYA2TITLETOP_H

#include "game/types.h"

class CPangya2TitleTop {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    int selectionComplete;              // 0x18
    int introStarted;                   // 0x1c
    int bpEntryId;                      // 0x20
    char pad_024[0x4];                  // 0x24
    void* bpObject;                     // 0x28
    int idleUpdateCount;                // 0x2c
    unsigned char singlePlayerSelected; // 0x30
    char pad_031[0x3];                  // 0x31
    void* layout;                       // 0x34
    int texturePack;                    // 0x38
    int texture;                        // 0x3c

    int UpdateTitleSequence();
    void PollLoadAndUpdateLayout();
    void StartMenuFadeOut();
    int PollLoadComplete();
    int HandlePairedSelection();
    void DrawBackgroundAndScene();
    bool TryGetPlayMode(int* out);

    ~CPangya2TitleTop();
    CPangya2TitleTop();
};

#define CPANGYA2TITLETOP_OFF(f, o) \
    typedef char CPangya2TitleTop_off_##f[((int)&((CPangya2TitleTop*)0)->f == (o)) ? 1 : -1]
CPANGYA2TITLETOP_OFF(selectionComplete, 0x18);
CPANGYA2TITLETOP_OFF(introStarted, 0x1c);
CPANGYA2TITLETOP_OFF(bpEntryId, 0x20);
CPANGYA2TITLETOP_OFF(bpObject, 0x28);
CPANGYA2TITLETOP_OFF(idleUpdateCount, 0x2c);
CPANGYA2TITLETOP_OFF(singlePlayerSelected, 0x30);
CPANGYA2TITLETOP_OFF(layout, 0x34);
CPANGYA2TITLETOP_OFF(texturePack, 0x38);
CPANGYA2TITLETOP_OFF(texture, 0x3c);
#undef CPANGYA2TITLETOP_OFF

#endif // CLASSES_CPANGYA2TITLETOP_H
