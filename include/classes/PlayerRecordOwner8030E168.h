#ifndef CLASSES_PLAYERRECORDOWNER8030E168_H
#define CLASSES_PLAYERRECORDOWNER8030E168_H

#include "game/types.h"

// The prize-game session object: the BP entry IDs, the state machine's state and
// counters, the borrowed UI resources, and the working copy of the player record
// that the save path commits.
class PlayerRecordOwner8030E168 {
public:
    // Advance the prize game one frame. Returns 1 when the session can close.
    int UpdateStateMachine();

    // BP registry entries for the layered title, controls, message, result and
    // effect objects.
    int bpEntryId0;                  // 0x0 title
    int bpEntryId1;                  // 0x4 arrow
    int bpEntryId2;                  // 0x8 spin controls
    int bpEntryId3;                  // 0xc instruction message
    int bpEntryId4;                  // 0x10 result
    int bpEntryId5;                  // 0x14 speed effect
    int bpEntryId6;                  // 0x18 balloon
    int bpEntryId7;                  // 0x1c intro flare
    unsigned int state;              // 0x20
    int speedLevel;                  // 0x24
    int peakSpeedLevel;              // 0x28
    int preloadAnimationIndex;       // 0x2c
    char pad_030[0x4];               // 0x30
    int spinInputArmed;              // 0x34
    void* roundDol3D;                // 0x38
    void* numberDraw;                // 0x3c
    int rewardTable;                 // 0x40
    int galleryImageTable;           // 0x44
    int prizeGalleryTable;           // 0x48
    int rewardKind;                  // 0x4c
    void* texture0;                  // 0x50 prize image
    void* texture1;                  // 0x54 gallery image
    char savedPlayerRecord[0x11c];   // 0x58 working copy of the player record
    short itemCounts[13];            // 0x174
    char pad_18E[0x38e];             // 0x18e
    unsigned char profileAttribute0; // 0x51c
    unsigned char profileAttribute1; // 0x51d
    unsigned char profileAttribute2; // 0x51e
    unsigned char profileAttribute3; // 0x51f
    unsigned char profileAttribute4; // 0x520
    unsigned char profileAttribute5; // 0x521
    char pad_522[0x35e];             // 0x522
    void* dialog;                    // 0x880
    void* layout;                    // 0x884
    int messageResource;             // 0x888
    void* textFactory;               // 0x88c
    int messageDraw;                 // 0x890
    void* saveMenu;                  // 0x894
    int resourcesLoaded;             // 0x898
};

#define PLAYERRECORDOWNER8030E168_OFF(f, o) \
    typedef char PlayerRecordOwner8030E168_off_##f[((int)&((PlayerRecordOwner8030E168*)0)->f == (o)) ? 1 : -1]
PLAYERRECORDOWNER8030E168_OFF(bpEntryId1, 0x4);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId2, 0x8);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId3, 0xc);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId4, 0x10);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId5, 0x14);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId6, 0x18);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId7, 0x1c);
PLAYERRECORDOWNER8030E168_OFF(state, 0x20);
PLAYERRECORDOWNER8030E168_OFF(speedLevel, 0x24);
PLAYERRECORDOWNER8030E168_OFF(peakSpeedLevel, 0x28);
PLAYERRECORDOWNER8030E168_OFF(preloadAnimationIndex, 0x2c);
PLAYERRECORDOWNER8030E168_OFF(spinInputArmed, 0x34);
PLAYERRECORDOWNER8030E168_OFF(roundDol3D, 0x38);
PLAYERRECORDOWNER8030E168_OFF(numberDraw, 0x3c);
PLAYERRECORDOWNER8030E168_OFF(rewardTable, 0x40);
PLAYERRECORDOWNER8030E168_OFF(galleryImageTable, 0x44);
PLAYERRECORDOWNER8030E168_OFF(prizeGalleryTable, 0x48);
PLAYERRECORDOWNER8030E168_OFF(rewardKind, 0x4c);
PLAYERRECORDOWNER8030E168_OFF(texture0, 0x50);
PLAYERRECORDOWNER8030E168_OFF(texture1, 0x54);
PLAYERRECORDOWNER8030E168_OFF(savedPlayerRecord, 0x58);
PLAYERRECORDOWNER8030E168_OFF(itemCounts, 0x174);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute0, 0x51c);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute1, 0x51d);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute2, 0x51e);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute3, 0x51f);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute4, 0x520);
PLAYERRECORDOWNER8030E168_OFF(profileAttribute5, 0x521);
PLAYERRECORDOWNER8030E168_OFF(dialog, 0x880);
PLAYERRECORDOWNER8030E168_OFF(layout, 0x884);
PLAYERRECORDOWNER8030E168_OFF(messageResource, 0x888);
PLAYERRECORDOWNER8030E168_OFF(textFactory, 0x88c);
PLAYERRECORDOWNER8030E168_OFF(messageDraw, 0x890);
PLAYERRECORDOWNER8030E168_OFF(saveMenu, 0x894);
PLAYERRECORDOWNER8030E168_OFF(resourcesLoaded, 0x898);
PLAYERRECORDOWNER8030E168_OFF(bpEntryId0, 0x0);
#undef PLAYERRECORDOWNER8030E168_OFF
typedef char PlayerRecordOwner8030E168_size[(sizeof(PlayerRecordOwner8030E168) == 0x89c) ? 1 : -1];

#endif // CLASSES_PLAYERRECORDOWNER8030E168_H
