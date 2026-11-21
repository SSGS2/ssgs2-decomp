// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBPPLAYER_H
#define CLASSES_CBPPLAYER_H

#include "game/types.h"

class CBpPlayer {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    void* messageTextContext; // 0x4: CBpPlayer_StartLoad destroys this resource
    void* loadEvent; // 0x8: CBpPlayer_StartLoad: CEventLoad_ctor result
    void* fixedFontRenderer; // 0xC: EventText_PollMessageResources: WFixedFont_CreateMsgEcbResource receiver
    int controlEntryId; // 0x10: CBpPlayer_LoadMessageResources: CBpCtrl_CreateEntry out parameter
    void* textureArrays[2]; // 0x14: CBpPlayer_LoadMessageResources: arrays of 0x28-byte CBpTex
    void* speedController; // 0x1C: CBpPlayer_PollResourceLoad: ns0::CSpdCtrl allocation
    void* textureResource; // 0x20: CBpPlayer_PollResourceLoad: CPackTextureResource_PollLoad
    void* speakerNameDraw; // 0x24: CBpPlayer_ReleaseResources: pair-list removal and virtual destruction
    int speakerSlotIndex; // 0x28: CBpPlayer_ReleaseResources: reset to -1
    int speakerTableIndex; // 0x2C: CBpPlayer_ReleaseResources: reset to -1
    int speakerIndex; // 0x30: CBpPlayer_ReleaseResources: reset to -1
    int messageAlpha; // 0x34: CBpPlayer_ReleaseResources: reset to zero
    void* messageDraw; // 0x38: CBpPlayer_ReleaseResources: pair-list removal and virtual destruction
    unsigned int advanceRequested; // 0x3C: CBpPlayer_ReleaseResources: reset to zero
    union {
        unsigned int messageRequestScriptFlags;                // 0x40: 1 use in CBpPlayer_Dtor
        unsigned char stringBytes_40[4]; // 0x40: CBpPlayer_ReleaseResources inline string flag/data bytes
    };
    unsigned int stringLength_44; // 0x44: CBpPlayer_ReleaseResources: heap-string reset
    void* messageRequestScriptData;                       // 0x48: 1 use in CBpPlayer_Dtor
    union {
        unsigned int windowOutScriptFlags;                // 0x4c: 1 use in CBpPlayer_Dtor
        unsigned char stringBytes_4C[4]; // 0x4C: CBpPlayer_ReleaseResources inline string flag/data bytes
    };
    unsigned int stringLength_50; // 0x50: CBpPlayer_ReleaseResources: heap-string reset
    void* windowOutScriptData;                       // 0x54: 1 use in CBpPlayer_Dtor
    union {
        unsigned int windowInScriptFlags;                // 0x58: 1 use in CBpPlayer_Dtor
        unsigned char stringBytes_58[4]; // 0x58: CBpPlayer_ReleaseResources inline string flag/data bytes
    };
    unsigned int stringLength_5C; // 0x5C: CBpPlayer_ReleaseResources: heap-string reset
    void* windowInScriptData;                       // 0x60: 1 use in CBpPlayer_Dtor
    union {
        unsigned int messageReadyScriptFlags;                // 0x64: 1 use in CBpPlayer_Dtor
        unsigned char stringBytes_64[4]; // 0x64: CBpPlayer_ReleaseResources inline string flag/data bytes
    };
    unsigned int stringLength_68; // 0x68: CBpPlayer_ReleaseResources: heap-string reset
    void* messageReadyScriptData;                       // 0x6c: 1 use in CBpPlayer_Dtor
    union {
        unsigned int messageAdvanceScriptFlags;                // 0x70: 1 use in CBpPlayer_Dtor
        unsigned char stringBytes_70[4]; // 0x70: CBpPlayer_ReleaseResources inline string flag/data bytes
    };
    unsigned int stringLength_74; // 0x74: CBpPlayer_ReleaseResources: heap-string reset
    void* messageAdvanceScriptData;                       // 0x78: 1 use in CBpPlayer_Dtor
    void* sharedFile; // 0x7C: CBpPlayer_StartLoad, SharedFile_Create result
    void* speedFile; // 0x80: CBpPlayer_LoadMessageResources, SharedFile_Create result
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBPPLAYER_OFF(f, o) typedef char CBpPlayer_off_##f[((int)&((CBpPlayer*)0)->f == (o)) ? 1 : -1]
CBPPLAYER_OFF(vtbl, 0x0);
CBPPLAYER_OFF(messageRequestScriptFlags, 0x40);
CBPPLAYER_OFF(messageRequestScriptData, 0x48);
CBPPLAYER_OFF(windowOutScriptFlags, 0x4c);
CBPPLAYER_OFF(windowOutScriptData, 0x54);
CBPPLAYER_OFF(windowInScriptFlags, 0x58);
CBPPLAYER_OFF(windowInScriptData, 0x60);
CBPPLAYER_OFF(messageReadyScriptFlags, 0x64);
CBPPLAYER_OFF(messageReadyScriptData, 0x6c);
CBPPLAYER_OFF(messageAdvanceScriptFlags, 0x70);
CBPPLAYER_OFF(messageAdvanceScriptData, 0x78);
CBPPLAYER_OFF(messageTextContext, 0x4);
CBPPLAYER_OFF(loadEvent, 0x8);
CBPPLAYER_OFF(fixedFontRenderer, 0xC);
CBPPLAYER_OFF(controlEntryId, 0x10);
CBPPLAYER_OFF(textureArrays, 0x14);
CBPPLAYER_OFF(speedController, 0x1C);
CBPPLAYER_OFF(textureResource, 0x20);
CBPPLAYER_OFF(speakerNameDraw, 0x24);
CBPPLAYER_OFF(speakerSlotIndex, 0x28);
CBPPLAYER_OFF(speakerTableIndex, 0x2C);
CBPPLAYER_OFF(speakerIndex, 0x30);
CBPPLAYER_OFF(messageAlpha, 0x34);
CBPPLAYER_OFF(messageDraw, 0x38);
CBPPLAYER_OFF(advanceRequested, 0x3C);
CBPPLAYER_OFF(stringLength_44, 0x44);
CBPPLAYER_OFF(stringLength_50, 0x50);
CBPPLAYER_OFF(stringLength_5C, 0x5C);
CBPPLAYER_OFF(stringLength_68, 0x68);
CBPPLAYER_OFF(stringLength_74, 0x74);
CBPPLAYER_OFF(sharedFile, 0x7C);
CBPPLAYER_OFF(speedFile, 0x80);
CBPPLAYER_OFF(stringBytes_40, 0x40);
CBPPLAYER_OFF(stringBytes_4C, 0x4C);
CBPPLAYER_OFF(stringBytes_58, 0x58);
CBPPLAYER_OFF(stringBytes_64, 0x64);
CBPPLAYER_OFF(stringBytes_70, 0x70);
#undef CBPPLAYER_OFF

#endif  // CLASSES_CBPPLAYER_H
