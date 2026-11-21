// Non-virtual declarations only: no virtual keyword, no base class, so the compiler emits no vtable or RTTI.
// Names are unk_<offset> until behaviour justifies a real name; a comment says where an offset is used.
#ifndef CLASSES_CSTORYCONTE_H
#define CLASSES_CSTORYCONTE_H

#include "game/types.h"

class CStoryConte {
public:
    void* vtbl;                         // 0x0: original vtable pointer
    char pad_004[0x4];                  // 0x4
    unsigned int unk_8;                 // 0x8: 1 use in CStoryConte_ctor
    char pad_00C[0xc];                  // 0xc
    void* taskMainVtable;                       // 0x18: 2 uses in CStoryConte_ctor, CStoryConte_dtor
    int state;                         // 0x1c: 16 uses in CStoryConte_OnProcess, CStoryConte_HandleMsg, CStoryConte_OnLoad...
    unsigned int eventIndex;                // 0x20: 4 uses in CStoryConte_HandleMsg, CStoryConte_OnProcess, CStoryConte_ctor
    signed char eventPathFlags;                 // 0x24: 2 uses in CStoryConte_ctor, CStoryConte_OnDestroy
    signed char inlineEventPath;                 // 0x25: 2 uses in CStoryConte_ctor, CStoryConte_OnDestroy
    char pad_026[0x6];                  // 0x26
    void* eventPathData;                       // 0x2c: 5 uses in CStoryConte_OnLoad, CStoryConte_OnPreLoadInit, CStoryConte_HandleMsg
    void* eventLoader;                       // 0x30: 3 uses in CStoryConte_OnProcess, CStoryConte_ctor
    void* eventScript;                       // 0x34: 9 uses in CStoryConte_OnProcess, CStoryConte_ctor
    void* playerResourceLoader;                       // 0x38: 3 uses in CStoryConte_OnProcess, CStoryConte_ctor
    signed char playerResourceNameFlags;                 // 0x3c: 2 uses in CStoryConte_ctor, CStoryConte_HandleMsg (CONFLICT sizes [1, 4])
    unsigned char inlinePlayerResourceName;               // 0x3d: 2 uses in CStoryConte_ctor, CStoryConte_dtor
    char pad_03E[0x6];                  // 0x3e
    void* playerResourceNameData;                       // 0x44: 2 uses in CStoryConte_OnPreLoadInit, CStoryConte_HandleMsg
    unsigned char skipFadeUpdateCount;               // 0x48: 6 uses in CStoryConte_OnProcess, CStoryConte_ctor
    char pad_049[0x3];                  // 0x49
    char displayConfiguration;                        // 0x4c: 1 use in CStoryConte_HandleMsg
    char pad_04D[0x4f];                 // 0x4d
    unsigned int skipCaptionResource;                // 0x9c: 2 uses in CStoryConte_OnProcess, CStoryConte_ctor

    // offsets whose address is passed on (sub-objects or arrays, not placed): 0x25, 0x3c, 0x3d, 0x4c
};

// Layout checks: a wrong offset or size makes one of these array typedefs negative and the build fail.
#define CSTORYCONTE_OFF(f, o) typedef char CStoryConte_off_##f[((int)&((CStoryConte*)0)->f == (o)) ? 1 : -1]
CSTORYCONTE_OFF(unk_8, 0x8);
CSTORYCONTE_OFF(taskMainVtable, 0x18);
CSTORYCONTE_OFF(state, 0x1c);
CSTORYCONTE_OFF(eventIndex, 0x20);
CSTORYCONTE_OFF(eventPathFlags, 0x24);
CSTORYCONTE_OFF(inlineEventPath, 0x25);
CSTORYCONTE_OFF(eventPathData, 0x2c);
CSTORYCONTE_OFF(eventLoader, 0x30);
CSTORYCONTE_OFF(eventScript, 0x34);
CSTORYCONTE_OFF(playerResourceLoader, 0x38);
CSTORYCONTE_OFF(playerResourceNameFlags, 0x3c);
CSTORYCONTE_OFF(inlinePlayerResourceName, 0x3d);
CSTORYCONTE_OFF(playerResourceNameData, 0x44);
CSTORYCONTE_OFF(skipFadeUpdateCount, 0x48);
CSTORYCONTE_OFF(displayConfiguration, 0x4c);
CSTORYCONTE_OFF(skipCaptionResource, 0x9c);
#undef CSTORYCONTE_OFF

#endif  // CLASSES_CSTORYCONTE_H
