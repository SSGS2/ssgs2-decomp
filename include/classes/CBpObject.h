#ifndef CLASSES_CBPOBJECT_H
#define CLASSES_CBPOBJECT_H

#include "game/types.h"

class CBpObject {
public:
    void* vtbl;               // 0x0: original vtable pointer
    void* script;             // 0x4: BpObject_StartScriptByName passes it to script calls
    char pad_008[8];          // 0x8
    void* scriptBuffer;       // 0x10: BpObject_PollLoad allocates/clears this buffer
    int scriptState;          // 0x14: SetScriptResourcePath sets 2, BpObject_StartScriptByName sets 3
    unsigned int objectCount; // 0x18: ReplaceObjectById decrements after removing an object node
    void* objectRoot;         // 0x1c
    void* firstObject;        // 0x20: CBpObject_ClearResources traverses the object tree from here
    void* objectArray;        // 0x24
    int loadState;            // 0x28: SetScriptResourcePath sets 1, PollLoad advances it
    char pad_02C[0x8];        // 0x2C
    int callbackListEmpty;    // 0x34
    char pad_038[0x8];        // 0x38
    void* callbackInvoked;    // 0x40: CTourMain_UpdateItemMenuVisuals: cleared after BpObject_IsCurrentScriptComplete
    char pad_044[0xC];        // 0x44
    unsigned int namedGroupCount; // 0x50: named group tree container; CBpObject_ClearResources clears it
    void* namedGroupRoot;         // 0x54: CBpObject_ClearResources frees this tree
    void* firstNamedGroup;        // 0x58: CBpObject_ClearResources restores its sentinel here
    void* loader;                 // 0x5c
    char scriptPathStorage[0xC];  // 0x60: SetScriptResourcePath replaces/trims this string's path
    float callbackPosition
        [2]; // 0x6c: BpObject_TryInvokeCurrentCallback copies arg2 via gtl_Vec2_Copy, or zeros both components

    void ResetSpriteVertexCountsLocked();
    int ClearResources();

    // Object map and script resource path.
    void* FindTextureById(int key);
    int SetScriptResourcePath(const char* filename, int scriptHandle);
    int ReplaceObjectById(int key, int value);

    ~CBpObject();

    CBpObject();
};

#define CBPOBJECT_OFF(f, o) typedef char CBpObject_off_##f[((int)&((CBpObject*)0)->f == (o)) ? 1 : -1]
CBPOBJECT_OFF(objectRoot, 0x1c);
CBPOBJECT_OFF(objectArray, 0x24);
CBPOBJECT_OFF(loader, 0x5c);
CBPOBJECT_OFF(callbackInvoked, 0x40);
CBPOBJECT_OFF(scriptPathStorage, 0x60);
CBPOBJECT_OFF(callbackPosition, 0x6c);
CBPOBJECT_OFF(vtbl, 0x0);
CBPOBJECT_OFF(script, 0x4);
CBPOBJECT_OFF(scriptBuffer, 0x10);
CBPOBJECT_OFF(scriptState, 0x14);
CBPOBJECT_OFF(objectCount, 0x18);
CBPOBJECT_OFF(firstObject, 0x20);
CBPOBJECT_OFF(loadState, 0x28);
CBPOBJECT_OFF(callbackListEmpty, 0x34);
CBPOBJECT_OFF(namedGroupCount, 0x50);
CBPOBJECT_OFF(namedGroupRoot, 0x54);
CBPOBJECT_OFF(firstNamedGroup, 0x58);
#undef CBPOBJECT_OFF

#endif // CLASSES_CBPOBJECT_H
