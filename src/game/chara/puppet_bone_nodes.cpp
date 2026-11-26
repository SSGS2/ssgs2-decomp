// Puppet bone nodes: bone links and chains, names and hashes, matrices, opacity and render flags,
// and the instance tables and angle constants used by puppets.

#include <decomp/static_init.h>
#include "game/wvector.h"
#include "classes/CNode.h"
#include "classes/WGeneric3DR.h"

// Layout shared by the bone links and matrices used below; no runtime methods.
struct PuppetBoneLayout {
    void* vtable;
    char pad_04[4];
    char name[0x20];
    int nameHash;
    float initialMatrix[12];
    void* child;
    void* parent;
    void* next;
    void* attachment;
    unsigned char opacity;
    char pad_6D[3];
    float chainScale;
    unsigned int renderFlags;
    float currentMatrix[12];
    float bindMatrix[12];
};
typedef char PuppetBoneLayout_size[(sizeof(PuppetBoneLayout) == 0xD8) ? 1 : -1];
typedef char PuppetBoneLayout_matrixOffset[((int)&((PuppetBoneLayout*)0)->currentMatrix == 0x78) ? 1 : -1];
#define AsPuppetBone(p) ((PuppetBoneLayout*)(p))
enum {
    kBoneNameCapacity = 0x20,
    kNameHashSeed = 0x1505,
    kBoneRenderFlag40000 = 0x00040000,
    kOpaque = 0xFF,
    kCreateRenderer = 1,
    kCreateInputManager = 8,
    kInitialTableCapacity = 16,
    kPuppetInstanceSlotCount = 0x100,
    kPuppetInstanceSlotSize = 0x28
};

extern "C" {
void* dtor_80048DD4(void*, int);
void* dtor_80048EAC(void*, int);
void* dtor_80048F84(void*, int);
void* dtor_8004905C(void*, int);
void* dtor_80049134(void*, int);

extern float kFloat_6_2831855;
extern float s_twoPi_80047884;
extern float s_twoPi_80047884_805E2718;
extern float s_twoPi_80047884_805E2720;
extern int s_puppetInstanceSlots[4];
extern char kGenericRendererInstanceName[];

void* Matrix_CopyAffine(void*, const void*);
float WVectorLen__FRC7WVector(void* self);
void Matrix_ConcatAffine(void* dest, void* source, void* arg);
void strncpy(void* dest, void* source, int size);
int strstr(void* name, void* key);
void* Matrix_SetIdentityAffine(void* self);
void* appMallocUni__FUl(unsigned int size);
void* __nw__FUl(unsigned int size);
void WDirectInput_Ctor(void* self);
void OSInitMutex(void* self);
void OSLockMutex(void* self);
void* __dl__FPv(void* self);
void OSUnlockMutex(void* self);
void* memset(void* dest, int value, unsigned int size);
void __construct_array(void* resource, void (*callback)(), int arg, int size, int flags);
int CString_Compare(void* key, void* name);

#pragma inline_depth(3)
// Return the first bone in the next chain whose name equals the query.
void* WPuppetBone_FindBone(void* self, void* key, int arg) {
    if (CString_Compare(key, AsPuppetBone(self)->name) == 0)
        return self;
    void* nextBone = AsPuppetBone(self)->next;
    if (nextBone != 0) {
        void* result = WPuppetBone_FindBone(nextBone, key, arg);
        if (result != 0)
            return result;
    }
    return 0;
}
#pragma inline_depth reset

// Initialize the puppet-bone helper's full-turn constant.
void PuppetBone_InitializeAngleConstant() {
    s_twoPi_80047884 = kFloat_6_2831855;
}

// Clear bone links and flags, set full opacity, and initialize the current matrix.
void* WPuppetBone_Init(void* self) {
    AsPuppetBone(self)->child = 0;
    AsPuppetBone(self)->parent = 0;
    AsPuppetBone(self)->next = 0;
    AsPuppetBone(self)->opacity = kOpaque;
    AsPuppetBone(self)->renderFlags = 0;
    Matrix_SetIdentityAffine(AsPuppetBone(self)->currentMatrix);
    memset(AsPuppetBone(self)->name, 0, kBoneNameCapacity);
    return self;
}

// Optionally free bone-node storage.
void* WPuppetBone_dtor(void* self, int deleteFlag) {
    if (self != 0 && deleteFlag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// Initialize the bone-chain helper's full-turn constant.
void BoneChain_InitializeAngleConstant() {
    s_twoPi_80047884_805E2718 = kFloat_6_2831855;
}

// Set a child link and its reciprocal parent link.
void BoneNode_SetChildAndParent(void* self, void* next) {
    if (next != 0) {
        AsPuppetBone(self)->child = next;
        AsPuppetBone(next)->parent = self;
        return;
    }
    AsPuppetBone(self)->child = 0;
}

// Trivial instance-slot initializer used by the static array constructor.
void PuppetInstanceSlot_Init() {}

#pragma opt_common_subs off
// Copy bone name/hash, attachment, matrices, scale, and opacity while clearing links.
void* WPuppetBone_CopyState(void* self, void* other) {
    register void* s = self;
    register void* o = other;
    if (s != o) {
        AsPuppetBone(s)->child = 0;
        AsPuppetBone(s)->parent = 0;
        AsPuppetBone(s)->next = 0;
        strncpy(AsPuppetBone(s)->name, AsPuppetBone(o)->name, kBoneNameCapacity);
        register signed char* p = reinterpret_cast<signed char*>(AsPuppetBone(s)->name);
        register int hash = kNameHashSeed;
        int c;
        while ((c = *p++) != 0) {
            hash = (hash << 5) + (hash + c);
        }
        AsPuppetBone(s)->nameHash = hash;
        AsPuppetBone(s)->attachment = AsPuppetBone(o)->attachment;
        (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(s)->currentMatrix,
                                                                        AsPuppetBone(o)->currentMatrix);
        (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(s)->bindMatrix,
                                                                        AsPuppetBone(o)->bindMatrix);
        AsPuppetBone(s)->chainScale = AsPuppetBone(o)->chainScale;
        AsPuppetBone(s)->opacity = AsPuppetBone(o)->opacity;
    }
    return s;
}
#pragma opt_common_subs reset

} // extern C

#pragma opt_common_subs off
class WInstanceLock {
public:
    void* const instance;
    WInstanceLock(void* p) : instance(p) {
        OSLockMutex(p);
    }
    ~WInstanceLock() {
        OSUnlockMutex(instance);
    }
};

struct InitTable {
    int count, field4, field8, fieldC, size, mask, capacity;
    int* entries;
    char mutex[0x18];
    void initializeMutex() {
        entries = 0;
        OSInitMutex(mutex);
    }
    template <bool Scoped> void initialize() {
        if (Scoped) {
            WInstanceLock lock(mutex);
            setup();
        } else {
            register void* lock = mutex;
            OSLockMutex(lock);
            try {
                setup();
            } catch (...) {
                OSUnlockMutex(lock);
                throw;
            }
            OSUnlockMutex(lock);
        }
    }
    void setup() {
        register int zero = 0;
        count = zero;
        fieldC = zero;
        field8 = zero;
        size = kInitialTableCapacity;
        field4 = zero;
        capacity = kInitialTableCapacity;
        if (size > 0) {
            entries = static_cast<int*>(appMallocUni__FUl(size * 4));
            mask = capacity - 1;
            for (int i = 0; i < capacity; ++i)
                entries[i] = zero;
        } else
            entries = 0;
    }
};

struct InitG3DR {
    char storage[0x1AA54];
    InitG3DR() {
        ((WGeneric3DR*)this)->Init(kGenericRendererInstanceName, 0);
    }
};
struct InitManager {
    char storage[0x10];
    InitManager() {
        WDirectInput_Ctor(this);
    }
};

extern "C" {
// Initialize five locked instance tables and create the optional renderer/input manager.
void* WInstanceTable_Initialize(void* self, unsigned int unused, unsigned int flags) {
    char* object = static_cast<char*>(self);
    int completed = 0;
    try {
        reinterpret_cast<InitTable*>(object)->initializeMutex();
        reinterpret_cast<InitTable*>(object)->initialize<true>();
        completed = 1;
        reinterpret_cast<InitTable*>(object + 0x38)->initializeMutex();
        reinterpret_cast<InitTable*>(object + 0x38)->initialize<false>();
        completed = 2;
        reinterpret_cast<InitTable*>(object + 0x70)->initializeMutex();
        reinterpret_cast<InitTable*>(object + 0x70)->initialize<false>();
        completed = 3;
        reinterpret_cast<InitTable*>(object + 0xA8)->initializeMutex();
        reinterpret_cast<InitTable*>(object + 0xA8)->initialize<false>();
        completed = 4;
        reinterpret_cast<InitTable*>(object + 0xE0)->initializeMutex();
        reinterpret_cast<InitTable*>(object + 0xE0)->initialize<false>();
        completed = 5;
        *reinterpret_cast<void**>(object + 0x118) = 0;
        if (flags & kCreateRenderer) {
            void* p = new InitG3DR;
            *reinterpret_cast<void**>(object + 0x118) = p;
        }
        *reinterpret_cast<void**>(object + 0x11C) = 0;
        if (flags & kCreateInputManager) {
            void* p = new InitManager;
            *reinterpret_cast<void**>(object + 0x11C) = p;
        }
        return self;
    } catch (...) {
        if (completed >= 5)
            dtor_80049134(object + 0xE0, -1);
        if (completed >= 4)
            dtor_8004905C(object + 0xA8, -1);
        if (completed >= 3)
            dtor_80048F84(object + 0x70, -1);
        if (completed >= 2)
            dtor_80048EAC(object + 0x38, -1);
        if (completed >= 1)
            dtor_80048DD4(object, -1);
        throw;
    }
}

#pragma opt_common_subs reset
// Search the next chain for the first bone containing the requested name fragment.
void* WPuppetBone_FindBoneByTail(void* self, void* key) {
    register void* cur = self;
    register void* search = key;
    if (strstr(AsPuppetBone(cur)->name, search) != 0)
        return cur;
    cur = AsPuppetBone(cur)->next;
    if (cur != 0) {
        if (strstr(AsPuppetBone(cur)->name, search) == 0) {
            cur = AsPuppetBone(cur)->next;
            if (cur != 0) {
                if (strstr(AsPuppetBone(cur)->name, search) == 0) {
                    cur = AsPuppetBone(cur)->next;
                    if (cur != 0) {
                        if (strstr(AsPuppetBone(cur)->name, search) == 0) {
                            void* next = AsPuppetBone(cur)->next;
                            cur = (next != 0) ? WPuppetBone_FindBoneByTail(next, search) : 0;
                        } else {
                            cur = cur;
                        }
                    } else {
                        cur = 0;
                    }
                } else {
                    cur = cur;
                }
            } else {
                cur = 0;
            }
        } else {
            cur = cur;
        }
    } else {
        cur = 0;
    }
    if (cur == 0)
        return 0;
    return cur;
}

// Concatenate each bone's bind matrix with the supplied parent matrix.
void WPuppetBone_ConcatMatrixAlongChain(void* self, float scale, void* parentMatrix, void* arg3, void* arg2) {
    char concatenated0[0x30];
    char concatenated1[0x30];
    char concatenated2[0x30];
    char concatenated3[0x30];
    Matrix_ConcatAffine(concatenated0, AsPuppetBone(self)->bindMatrix, parentMatrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(self)->currentMatrix, concatenated0);
    void* nextBone = AsPuppetBone(self)->next;
    if (!nextBone)
        return;
    Matrix_ConcatAffine(concatenated1, AsPuppetBone(nextBone)->bindMatrix, parentMatrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(nextBone)->currentMatrix,
                                                                    concatenated1);
    nextBone = AsPuppetBone(nextBone)->next;
    if (!nextBone)
        return;
    Matrix_ConcatAffine(concatenated2, AsPuppetBone(nextBone)->bindMatrix, parentMatrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(nextBone)->currentMatrix,
                                                                    concatenated2);
    nextBone = AsPuppetBone(nextBone)->next;
    if (!nextBone)
        return;
    Matrix_ConcatAffine(concatenated3, AsPuppetBone(nextBone)->bindMatrix, parentMatrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(nextBone)->currentMatrix,
                                                                    concatenated3);
    nextBone = AsPuppetBone(nextBone)->next;
    if (nextBone)
        WPuppetBone_ConcatMatrixAlongChain(nextBone, scale, parentMatrix, arg3, arg2);
}

// Set or clear the bone render flag selected by the enable parameter.
void WPuppetBone_SetRenderFlag40000(void* self, void* arg1, int enabled) {
    unsigned int* field = &AsPuppetBone(self)->renderFlags;
    if (enabled != 0) {
        *field |= kBoneRenderFlag40000;
    } else {
        *field &= ~kBoneRenderFlag40000;
    }
}

// Initialize the full-turn constant and construct the puppet instance-slot array.
void PuppetInstanceSlots_Initialize() {
    s_twoPi_80047884_805E2720 = kFloat_6_2831855;
    __construct_array(&s_puppetInstanceSlots, PuppetInstanceSlot_Init, 0, kPuppetInstanceSlotSize,
                      kPuppetInstanceSlotCount);
}

// Splice a next chain, optionally preserving the old chain at its tail.
void BoneNode_SetNextChain(void* self, void* next, void* preserveOldChain) {
    if (next != 0 && preserveOldChain != 0) {
        void* tail = next;
        while (AsPuppetBone(tail)->next != 0) {
            tail = AsPuppetBone(tail)->next;
        }
        AsPuppetBone(tail)->next = AsPuppetBone(self)->next;
    }
    AsPuppetBone(self)->next = next;
}

// Copy an affine matrix into the bone's current matrix.
void* WPuppetBone_SetCurrentMatrix(void* self, const void* forwarded1) {
    return Matrix_CopyAffine(AsPuppetBone(self)->currentMatrix, forwarded1);
}

// Return the next bone-chain link.
void* CNode::GetNext() {
    return AsPuppetBone(this)->next;
}

// Copy the bounded bone name and compute its signed-byte DJB2-style hash.
void WPuppetBone_SetName(void* self, void* source) {
    register char* base = static_cast<char*>(self);
    // Byte addition retains the original register lifetimes across strncpy.
    enum {
        nameOffset = (int)&((PuppetBoneLayout*)0)->name
    };
    strncpy(base + nameOffset, source, kBoneNameCapacity);
    register char* p = base + nameOffset;
    register int hash = kNameHashSeed;
    signed char c;
    while ((c = *p++) != 0) {
        hash = (hash << 5) + (hash + c);
    }
    AsPuppetBone(base)->nameHash = hash;
}

// Initialize all bone matrices and derive scale from the bind matrix's third axis.
void WPuppetBone_SetBasicMatrix(void* self, void* matrix) {
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(self)->initialMatrix, matrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(self)->bindMatrix, matrix);
    (*reinterpret_cast<void* (*)(void*, void*)>(Matrix_CopyAffine))(AsPuppetBone(self)->currentMatrix, matrix);
    float f = WVectorLen(*(WVector*)(&AsPuppetBone(self)->bindMatrix[6]));
    AsPuppetBone(self)->chainScale = f;
}

#pragma inline_depth(0)
// Set bone opacity and optionally propagate it down the next chain.
void WPuppetBone_SetAlpha(void* self, unsigned char value, int flag) {
    AsPuppetBone(self)->opacity = value;
    if (flag != 0) {
        void* cur = AsPuppetBone(self)->next;
        if (cur != 0) {
            AsPuppetBone(cur)->opacity = value;
            cur = AsPuppetBone(cur)->next;
            if (cur != 0) {
                AsPuppetBone(cur)->opacity = value;
                cur = AsPuppetBone(cur)->next;
                if (cur != 0) {
                    AsPuppetBone(cur)->opacity = value;
                    void* next = AsPuppetBone(cur)->next;
                    if (next != 0) {
                        AsPuppetBone(next)->opacity = value;
                        if (((CNode*)next)->GetNext() != 0) {
                            WPuppetBone_SetAlpha(((CNode*)next)->GetNext(), value, 1);
                        }
                    }
                }
            }
        }
    }
}
#pragma inline_depth(255)

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80461FBC", _ctors_u80047884_0, PuppetBone_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80461FBC", _ctors_u80047884_1, BoneChain_InitializeAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU80461FBC", _ctors_u80047884_2, PuppetInstanceSlots_Initialize);
#pragma pop
