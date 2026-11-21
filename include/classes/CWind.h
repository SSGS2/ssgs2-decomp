// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CWIND_H
#define CLASSES_CWIND_H

#include "game/types.h"

class CWind {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0xc]; // 0x4
    int additiveWindXBits; // 0x10: CPartyItemFujinUchiwa_UpdateOpponentWind: save/restore vector bits
    int additiveWindYBits; // 0x14: CPartyItemFujinUchiwa_UpdateOpponentWind: save/restore vector bits
    int additiveWindZBits; // 0x18: CPartyItemFujinUchiwa_UpdateOpponentWind: save/restore vector bits
    char pad_01C[0x70]; // 0x1c
    unsigned char enabled; // 0x8c: CWind_GetWindVector / CWind_GetWindSpeed early return
    unsigned char cannonEndSoundPlayed; // 0x8d: CWind_PlayCannonEndSoundOnce guards sound at courseEnvironmentStrings + 0x0d
    unsigned char cannonChargeSoundPlayed; // 0x8e: CWind_PlayCannonChargeSoundOnce guards sound at courseEnvironmentStrings + 0x1c
    char pad_08F[0x5]; // 0x8f
    int resourceCount;                         // 0x94: 2 uses in CWind_dtor
    char pad_098[4]; // 0x98
    float windVector[3]; // 0x9c: CWind_GetWindVector, fn_802135B8
    float overrideVector[3]; // 0xa8: CWind_GetWindVector when overrideActive
    float overrideSpeed; // 0xb4: CWind_InitOverrideFromCourse sets 6 + index; CWind_GetWindSpeed returns it
    char pad_0B8[4]; // 0xb8
    unsigned char overrideActive; // 0xbc: CWind_GetWindVector selects overrideVector
    char pad_0BD[3]; // 0xbd
    char effectMatrix[0x24]; // 0xc0: CWind_InitOverrideFromCourse initializes affine matrix; translation follows
    float effectPosition[3]; // 0xe4: CWind_InitOverrideFromCourse; Sound_PlayByName position
    float effectOffset[3]; // 0xf0: CWind_InitOverrideFromCourse zeros; CWind_SetEffectEnabled attached-effect argument
    void* attachedEffect; // 0xfc: CWind_SetEffectEnabled creates/releases the effect

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x90
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CWIND_OFF(f, o) typedef char CWind_off_##f[((int)&((CWind*)0)->f == (o)) ? 1 : -1]
CWIND_OFF(resourceCount, 0x94);
CWIND_OFF(additiveWindXBits, 0x10);
CWIND_OFF(additiveWindYBits, 0x14);
CWIND_OFF(additiveWindZBits, 0x18);
CWIND_OFF(enabled, 0x8c);
CWIND_OFF(cannonEndSoundPlayed, 0x8d);
CWIND_OFF(cannonChargeSoundPlayed, 0x8e);
CWIND_OFF(windVector, 0x9c);
CWIND_OFF(overrideVector, 0xa8);
CWIND_OFF(overrideSpeed, 0xb4);
CWIND_OFF(overrideActive, 0xbc);
CWIND_OFF(effectMatrix, 0xc0);
CWIND_OFF(effectPosition, 0xe4);
CWIND_OFF(effectOffset, 0xf0);
CWIND_OFF(attachedEffect, 0xfc);
#undef CWIND_OFF

#endif  // CLASSES_CWIND_H
