// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CBUNKERPARTICLE_H
#define CLASSES_CBUNKERPARTICLE_H

#include "game/types.h"

class CBunkerParticle {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x10];                 // 0x4
    unsigned int bitmapId;                // 0x14: 2 uses in CBunkerParticle_Ctor, CBunkerParticle_RestartEffect
    unsigned char active;               // 0x18: 2 uses in CBunkerParticle_Ctor, CBunkerParticle_RestartEffect

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x8, 0x44, 0x7c, 0xb4, 0xec
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CBUNKERPARTICLE_OFF(f, o) typedef char CBunkerParticle_off_##f[((int)&((CBunkerParticle*)0)->f == (o)) ? 1 : -1]
CBUNKERPARTICLE_OFF(bitmapId, 0x14);
CBUNKERPARTICLE_OFF(active, 0x18);
#undef CBUNKERPARTICLE_OFF

#endif  // CLASSES_CBUNKERPARTICLE_H
