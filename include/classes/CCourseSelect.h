// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CCOURSESELECT_H
#define CLASSES_CCOURSESELECT_H

#include "game/types.h"

class CCourseSelect {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* unk_004;                        // 0x4: 1 use in CHoleNoSelect_IsLoaded
    char pad_008[0x10];                 // 0x8
    unsigned char selectedCourseIndex;               // 0x18: 1 use in CCourseSelect_Ctor
    unsigned char holeOrder;               // 0x19: 1 use in CCourseSelect_Ctor
    unsigned char weatherMode;               // 0x1a: 1 use in CCourseSelect_Ctor
    unsigned char holeCount;               // 0x1b: 1 use in CCourseSelect_Ctor
    void* holeNumbers;                       // 0x1c: 1 use in CCourseSelect_Ctor
    void* holeNumberCount;                       // 0x20: 1 use in CCourseSelect_Ctor
    void* holeNumberCapacity;                       // 0x24: 1 use in CCourseSelect_Ctor
    void* holeCourseIds;                       // 0x28: 1 use in CCourseSelect_Ctor
    void* holeCourseCount;                       // 0x2c: 1 use in CCourseSelect_Ctor
    void* holeCourseCapacity;                       // 0x30: 1 use in CCourseSelect_Ctor
    unsigned int selectionMode;                // 0x34: 3 uses in CCourseSelect_Ctor, CCourseSelect_StartMenuFadeOut, CCourseSelect_AreResourcesLoaded
    void* layout;                       // 0x38: 2 uses in CCourseSelect_Ctor
    void* scriptObject;                       // 0x3c: 2 uses in CCourseSelect_Ctor
    int bpEntryId;                         // 0x40: 2 uses in CCourseSelect_Ctor
    void* courseTextures;                       // 0x44: 4 uses in CCourseSelect_Ctor
    void* auxiliaryTexture;                       // 0x48: 3 uses in CCourseSelect_Ctor
    void* coursePage;                       // 0x4c: 1 use in CCourseSelect_Ctor
    int selectionDelayFrames;                         // 0x50: 1 use in CCourseSelect_Ctor
    unsigned char selectionComplete;               // 0x54: 1 use in CCourseSelect_Ctor
    char pad_055[0x3];                  // 0x55
    void* sharedFile;                       // 0x58: 1 use in CCourseSelect_Ctor
    void* fixedFont;                       // 0x5c: 2 uses in CCourseSelect_Ctor
    void* text;                       // 0x60: 1 use in CCourseSelect_Ctor
    void* messageRenderer;                       // 0x64: 1 use in CCourseSelect_Ctor
    void* textTable;                       // 0x68: 2 uses in CCourseSelect_Ctor
    void* holeNoSelect;                       // 0x6c: 1 use in CCourseSelect_Ctor
    unsigned char holeSelectorActive;               // 0x70: 1 use in CCourseSelect_Ctor
    char pad_071[0x3];                  // 0x71
    void* miniGameHole;                       // 0x74: 2 uses in CCourseSelect_Ctor
    void* texturePack;                       // 0x78: 2 uses in CCourseSelect_Ctor
    unsigned char weatherOptionEnabled;               // 0x7c: 1 use in CCourseSelect_Ctor
    unsigned char exitRequested;               // 0x7d: 2 uses in CCourseSelect_Ctor, CCourseSelect_UpdateSelection

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x18, 0x1c, 0x28, 0x40, 0x60, 0x64
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CCOURSESELECT_OFF(f, o) typedef char CCourseSelect_off_##f[((int)&((CCourseSelect*)0)->f == (o)) ? 1 : -1]
CCOURSESELECT_OFF(unk_004, 0x4);
CCOURSESELECT_OFF(selectedCourseIndex, 0x18);
CCOURSESELECT_OFF(holeOrder, 0x19);
CCOURSESELECT_OFF(weatherMode, 0x1a);
CCOURSESELECT_OFF(holeCount, 0x1b);
CCOURSESELECT_OFF(holeNumbers, 0x1c);
CCOURSESELECT_OFF(holeNumberCount, 0x20);
CCOURSESELECT_OFF(holeNumberCapacity, 0x24);
CCOURSESELECT_OFF(holeCourseIds, 0x28);
CCOURSESELECT_OFF(holeCourseCount, 0x2c);
CCOURSESELECT_OFF(holeCourseCapacity, 0x30);
CCOURSESELECT_OFF(selectionMode, 0x34);
CCOURSESELECT_OFF(layout, 0x38);
CCOURSESELECT_OFF(scriptObject, 0x3c);
CCOURSESELECT_OFF(bpEntryId, 0x40);
CCOURSESELECT_OFF(courseTextures, 0x44);
CCOURSESELECT_OFF(auxiliaryTexture, 0x48);
CCOURSESELECT_OFF(coursePage, 0x4c);
CCOURSESELECT_OFF(selectionDelayFrames, 0x50);
CCOURSESELECT_OFF(selectionComplete, 0x54);
CCOURSESELECT_OFF(sharedFile, 0x58);
CCOURSESELECT_OFF(fixedFont, 0x5c);
CCOURSESELECT_OFF(text, 0x60);
CCOURSESELECT_OFF(messageRenderer, 0x64);
CCOURSESELECT_OFF(textTable, 0x68);
CCOURSESELECT_OFF(holeNoSelect, 0x6c);
CCOURSESELECT_OFF(holeSelectorActive, 0x70);
CCOURSESELECT_OFF(miniGameHole, 0x74);
CCOURSESELECT_OFF(texturePack, 0x78);
CCOURSESELECT_OFF(weatherOptionEnabled, 0x7c);
CCOURSESELECT_OFF(exitRequested, 0x7d);
#undef CCOURSESELECT_OFF

#endif  // CLASSES_CCOURSESELECT_H
