// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CLETTER_H
#define CLASSES_CLETTER_H

#include "game/types.h"

class CLetter {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x14];                 // 0x4
    void* currentMessage;                       // 0x18: 9 uses in CLetter_HandleMsg, CLetter_Ctor
    char pad_01C[0x8];                  // 0x1c
    void* nodeAllocationHead;                       // 0x24: 4 uses in CLetter_dtor, CLetter_Ctor
    char pad_028[0x8];                  // 0x28
    unsigned int nameHashMask;          // 0x30: fn_80194F9C_hash masks the bucket index.
    char pad_034[0x4];                  // 0x34
    void* nameHashBuckets;                       // 0x38: 7 uses in CLetter_Ctor, CLetter_dtor, CLetter_OnDestroy
    char nameHashMutex[0x18];           // 0x3c: fn_80194F9C_hash/find and CLetter_dtor lock this storage.
    float messageAlpha;                       // 0x54: 2 uses in CLetter_HandleMsg
    int messageState;                         // 0x58: 6 uses in CLetter_HandleMsg, CLetter_OnInit
    float messageFadeDurationSeconds; // 0x5c: overlay alpha integrates 255 / duration * dt seconds
    unsigned char messageDisplayEnabled;               // 0x60: 2 uses in CLetter_OnPreLoadInit, CLetter_HandleMsg
    char pad_061[0x7];                  // 0x61
    int scriptedMessageIndex;                         // 0x68: 7 uses in CLetter_HandleMsg, CLetter_OnInit
    int completedScriptHoldUpdates; // 0x6c: eligible process calls after script completion
    float knownMessageValue;                       // 0x70: 3 uses in CLetter_HandleMsg, CLetter_OnInit
    int textAlignment;                 // 0x74: CLetter_DrawText: 0 left, 1 half-width, 2 full-width subtraction.
    char pad_078[0x4];                  // 0x78
    void* fontImageNames;                       // 0x7c: 1 use in CLetter_OnLoad
    int fontImageCount;                         // 0x80: 1 use in CLetter_OnLoad
    int fontGlyphWidth;                         // 0x84: 1 use in CLetter_OnLoad
    int fontGlyphHeight;                         // 0x88: 1 use in CLetter_OnLoad
    int fontTextureWidth;                         // 0x8c: 1 use in CLetter_OnLoad
    int fontTextureHeight;                         // 0x90: 1 use in CLetter_OnLoad
    int unk_94;                         // 0x94: 1 use in CLetter_OnLoad
    void* fontCharacters;                       // 0x98: 1 use in CLetter_OnLoad
    void* titleFont;                    // 0x9c: ResourceManager_CreateWTitleFont result
    void* fastForwardIcon;                       // 0xa0: 4 uses in CLetter_HandleMsg, CLetter_Ctor
    void* replayIcon;                       // 0xa4: 5 uses in CLetter_HandleMsg, CLetter_Ctor
    float replayBlinkTimer;                       // 0xa8: 2 uses in CLetter_OnPreLoadInit, CLetter_HandleMsg

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x1c, 0x3c, 0x64, 0x7c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CLETTER_OFF(f, o) typedef char CLetter_off_##f[((int)&((CLetter*)0)->f == (o)) ? 1 : -1]
CLETTER_OFF(currentMessage, 0x18);
CLETTER_OFF(nodeAllocationHead, 0x24);
CLETTER_OFF(nameHashMask, 0x30);
CLETTER_OFF(nameHashMutex, 0x3c);
CLETTER_OFF(textAlignment, 0x74);
CLETTER_OFF(nameHashBuckets, 0x38);
CLETTER_OFF(messageAlpha, 0x54);
CLETTER_OFF(messageState, 0x58);
CLETTER_OFF(messageFadeDurationSeconds, 0x5c);
CLETTER_OFF(completedScriptHoldUpdates, 0x6c);
CLETTER_OFF(messageDisplayEnabled, 0x60);
CLETTER_OFF(scriptedMessageIndex, 0x68);
CLETTER_OFF(knownMessageValue, 0x70);
CLETTER_OFF(fontImageNames, 0x7c);
CLETTER_OFF(fontImageCount, 0x80);
CLETTER_OFF(fontGlyphWidth, 0x84);
CLETTER_OFF(fontGlyphHeight, 0x88);
CLETTER_OFF(fontTextureWidth, 0x8c);
CLETTER_OFF(fontTextureHeight, 0x90);
CLETTER_OFF(unk_94, 0x94);
CLETTER_OFF(fontCharacters, 0x98);
CLETTER_OFF(titleFont, 0x9c);
CLETTER_OFF(fastForwardIcon, 0xa0);
CLETTER_OFF(replayIcon, 0xa4);
CLETTER_OFF(replayBlinkTimer, 0xa8);
#undef CLETTER_OFF

#endif  // CLASSES_CLETTER_H
