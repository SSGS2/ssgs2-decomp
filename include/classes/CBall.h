// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBALL_H
#define CLASSES_CBALL_H

#include "game/types.h"

class CBall {
public:
    CBall(); // Defined inline in ball_actor_and_ambient_sound.cpp.
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    int unk_8;                          // 0x8: CBall constructor initializes to 1
    char pad_00C[0xC];                  // 0xc
    void* secondaryVtable;              // 0x18: CBall secondary vtable
    int puppetEntryCount;               // 0x1c: 1 use in CBall_HandleMsg
    void* puppetListSentinel;           // 0x20: CBall constructor list sentinel link
    void* firstPuppetEntry;             // 0x24: 4 uses in CBall_HandleMsg, CBall_dtor
    void* puppet;                       // 0x28: 4 uses in CBall_HandleMsg
    void* bitmap;                       // 0x2c: CBall_OnPreLoadInit ResourceManager_LoadBitmap
    float matrixBasis[9];              // 0x30: CBall_OnInit affine matrix starts here
    float position[3];                 // 0x54: CBall_OnProcess copies physics position, lowers y
    float rotationAxis[3];             // 0x60: CBall_OnProcess cross product, matrix axis
    unsigned char lightUpdatePending;               // 0x6c: 1 use in CBall_HandleMsg
    char pad_06D[0x3];                  // 0x6d
    float groundSinkFactor;                       // 0x70: 4 uses in CBall_HandleMsg
    int sinkStartFrame;                         // 0x74: 2 uses in CBall_HandleMsg
    int lightingMode;                         // 0x78: CBall_OnPreLoadInit sets to 2; OnDisplay passes address
    float lightDirection[3];                   // 0x7c: CBall_OnInit copies vector, ctor zeros it
    void* lightColor;                       // 0x88: 1 use in CBall_HandleMsg
    void* ambientLightColor;                       // 0x8c: 1 use in CBall_HandleMsg
    unsigned int unk_90;                // 0x90: constructor initializes to 0x00FFFFFF
    unsigned char visible;              // 0x94: 2 uses in CBall_HandleMsg
    char pad_095[0x3];                  // 0x95
    int previousPlaybackFrame;                         // 0x98: 1 use in CBall_HandleMsg
    void* vertexStreams[4];             // 0x9c: CBall_OnPreLoadInit assigns offsets B0/E8/158/120
    void* unk_AC;                       // 0xac: CBall_OnPreLoadInit initializes to zero
    char vertexData[0xE0];              // 0xb0: four 0x38-byte vertices, CBall_OnPreLoadInit
    int effectMessageValue;                        // 0x190: 1 use in CBall_HandleMsg

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x20, 0x78
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBALL_OFF(f, o) typedef char CBall_off_##f[((int)&((CBall*)0)->f == (o)) ? 1 : -1]
CBALL_OFF(puppetEntryCount, 0x1c);
CBALL_OFF(firstPuppetEntry, 0x24);
CBALL_OFF(puppet, 0x28);
CBALL_OFF(lightUpdatePending, 0x6c);
CBALL_OFF(groundSinkFactor, 0x70);
CBALL_OFF(sinkStartFrame, 0x74);
CBALL_OFF(lightColor, 0x88);
CBALL_OFF(ambientLightColor, 0x8c);
CBALL_OFF(visible, 0x94);
CBALL_OFF(previousPlaybackFrame, 0x98);
CBALL_OFF(effectMessageValue, 0x190);
CBALL_OFF(unk_8, 0x8);
CBALL_OFF(secondaryVtable, 0x18);
CBALL_OFF(puppetListSentinel, 0x20);
CBALL_OFF(bitmap, 0x2c);
CBALL_OFF(matrixBasis, 0x30);
CBALL_OFF(position, 0x54);
CBALL_OFF(rotationAxis, 0x60);
CBALL_OFF(lightingMode, 0x78);
CBALL_OFF(lightDirection, 0x7c);
CBALL_OFF(unk_90, 0x90);
CBALL_OFF(vertexStreams, 0x9c);
CBALL_OFF(unk_AC, 0xac);
CBALL_OFF(vertexData, 0xb0);
#undef CBALL_OFF

#endif  // CLASSES_CBALL_H
