#include "classes/CParticleEffect.h"
#include "classes/Object801AAED8.h"
#define AsObject801AAED8(p) ((Object801AAED8*)(p))
#define AsCParticleEffect(p) ((CParticleEffect*)(p))
// Translation unit 0x801A8C20-0x801A97D0 (7 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"

#define OSLockMutex(p) OSLockMutex((OSMutex*)(p))
#define OSUnlockMutex(p) OSUnlockMutex((OSMutex*)(p))
#define OSInitMutex(p) OSInitMutex((OSMutex*)(p))

extern "C" {
void* __nw__FUl(unsigned long);
void __dl__FPv(void*);
void ParticleBitmapRegistry_Ctor(void*);
void CLightColumn_Ctor(void*);
void ParticleEffectPool_AddAndStart(void*, void*, int, int);
char* ResourcePreloadQueue_AddRequest(void*, int, int, float, char*, float, char*, int, int, int);
void appFreeUni__FPv(void*);
void __ct__18NativeBase80201D18Fv(void*);
void* appMallocUni__FUl(unsigned long);
void ParticleEmitter_UpdateAndEmit(void*, float);
void ParticleEffectPool_Destroy(void*, int);
void ParticleEffectPool_UpdateAndReap(void*, float);
void W3dAniSpr_SetSizeAndPivot(void*, float, float, float, float);
void W3dAniSpr_SetPosition(void*, void*);
void W3dAniSpr_SetColor(void*, unsigned int);
void W3dAniSpr_DrawFrame(void*, unsigned int, float, unsigned int, unsigned int);
void Object_DeleteVirtual(void*, void*);
void __dla__FPv(void*);

void* g_sceneManager;
extern char particleEffectStrings[16];
extern float kFloat_1_particles;
extern float kFloat_0_805DF3CC;
extern float kParticleSpriteCenterPivotFraction;
unsigned int g_view;
void* g_resourceManager;
int g_particleCourseEffects;
}

// Emitter-list ownership and traversal:
// A list node stores its emitter pointer at +0; Elem describes its links at
// +0xC/+0x10. Active/free rings are circular with null for an empty ring; the
// head is a real node, not an extra sentinel. activeCursor becomes null when
// traversal wraps back to activeHead. The allocation ring owns node storage.
// OnInit/OnDestroy delete emitter payloads and recycle active nodes; the list
// destructor releases allocation nodes and buckets, not emitter payloads.
// Only list/cursor changes hold the recursive OS mutex. Updates/draws run after
// unlocking; this is not a guarantee of safe concurrent emitter destruction.
struct Elem {
    char reservedPrefix[0xC];
    Elem* prev;
    Elem* next;
};

struct C801A8DA8 {
    char reservedPrefix[0x8];
    Elem* head;
    char reservedBeforeBuckets[0x10];
    void* slotBuckets;
    void* lock20;
};

struct WInstanceLock {
    void* lock;
    // Acquire the list's recursive mutex for this lexical scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex(l); }
    // Release the scope's mutex acquisition, including during exception unwind.
    ~WInstanceLock() { OSUnlockMutex(lock); }
};
extern "C" void __dt__13WInstanceLockFv(WInstanceLock*);

struct ParticleDrawFlagsView { char prefix[0x4C]; unsigned int flags; };
struct ParticleBitmapRegistryView { void* slots[20]; int lastSlot; };
// Particle records have stride 0x3C. Position is forwarded as the sprite's world
// position; this unit applies no additional world transform. opacity is
// multiplied by the emitter's opacity multiplier before packing into alpha.
// rotationRadians (+0x20) is the rotation passed to the sprite, angularVelocity
// (+0x24) is its velocity per caller time unit, and frameIndex (+0x2C) selects
// the sprite frame. The update code in particle_effect_emitters.cpp integrates
// rotation += dt * angularVelocity. tail holds the frame countdown (+0x30),
// initial lifetime (+0x34) and remaining lifetime (+0x38); dt units are unresolved.
struct ParticleVertexView {
    float position[3], velocity[3], opacity;
    unsigned char active; char padding[3];
    float rotationRadians, angularVelocity, size; unsigned int frameIndex;
    char tail[0xC];
};
// Bit 22 requests primitive queuing in the renderer; sprite geometry itself
// supplies camera-facing orientation in W3dAniSpr_DrawFrame.
enum ParticleRenderConstants { kParticleStride = 0x3C, kQueuedPrimitiveFlag = 0x00400000,
    kAlphaPackingUnit = 0x01000000, kInitialEmitterSlots = 16, kTaskMainVtableOffset = 0x3C };

struct ParticleListNode { void* emitter; char reserved[0xC]; void* next; };

struct Object801A8C20 { char data[0xA8]; };

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
// dt is forwarded unchanged to emitter/pool updates; its unit is defined by the task caller.
// Update active emitters under synchronized list traversal, then update and reap pooled effects.
extern "C" void CParticleEffect_OnProcess(char* self, float dt) {
    void* val;
    void* node;
    void* item;
    void* cur;
    {
        WInstanceLock local(self + 0x3C);
        AsCParticleEffect(self)->activeCursor = AsCParticleEffect(self)->activeHead;
        char* list = self + 0x3C;
        OSLockMutex(list);
        node = AsCParticleEffect(self)->activeCursor;
        if (node) {
            val = ((ParticleListNode*)node)->next;
            void* nxt = ((ParticleListNode*)node)->emitter;
            if (val == AsCParticleEffect(self)->activeHead) val = 0;
            AsCParticleEffect(self)->activeCursor = val;
            cur = nxt;
            OSUnlockMutex(list);
        } else {
            OSUnlockMutex(list);
            cur = 0;
        }
    }
    if (cur) {
    char* list = self + 0x3C;
    while (cur) {
        if (AsObject801AAED8(cur)->active) ParticleEmitter_UpdateAndEmit(cur, dt); // original +0x50
        OSLockMutex(list);
        item = AsCParticleEffect(self)->activeCursor;
        if (item) {
            val = ((ParticleListNode*)item)->next;
            void* nxt = ((ParticleListNode*)item)->emitter;
            if (val == AsCParticleEffect(self)->activeHead) val = 0;
            AsCParticleEffect(self)->activeCursor = val;
            OSUnlockMutex(list);
            cur = nxt;
        } else {
            OSUnlockMutex(list);
            cur = 0;
        }
    }
    }
    ParticleEffectPool_UpdateAndReap(AsCParticleEffect(self)->effectManager, dt);
}
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
struct Cursor801A95B4 { void* item; char pad[0xC]; Cursor801A95B4* next; };
struct Draw801A95B4 {
    virtual void slot0();
    virtual void slot1();
    virtual void draw();
};
#pragma opt_lifetimes off
#pragma use_lmw_stmw off
#pragma opt_propagation on
// Draw active particles of one enabled emitter, centering each square on its position.
inline void DrawParticle801A95B4(Elem* cur, float pivotFraction) {
    int particleByteOffset;
    int packedColor;
    int particleIndex;
    if (AsObject801AAED8(cur)->active) { // original +0x50
        if (AsObject801AAED8(cur)->drawingEnabled) { // original +0x110
            particleIndex = 0;
            particleByteOffset = 0;
            for (; particleIndex < AsObject801AAED8(cur)->particleCapacity; particleByteOffset += kParticleStride, ++particleIndex) { // original +0xF0
                unsigned char* particleBytes = (unsigned char*)AsObject801AAED8(cur)->particleStorage + particleByteOffset; // original +0x54
                if (((ParticleVertexView*)particleBytes)->active) {
                    // ((alpha << 24) + 0x01000000) - 1 gives alpha:FF:FF:FF.
                    packedColor = (((int)(AsObject801AAED8(cur)->surfaceOpacityMultiplier * ((ParticleVertexView*)particleBytes)->opacity)) << 24) + kAlphaPackingUnit - 1; // original +0x118
                    float size = ((ParticleVertexView*)particleBytes)->size;
                    W3dAniSpr_SetSizeAndPivot(AsObject801AAED8(cur)->sprite, size, size, pivotFraction * size, pivotFraction * size); // original +0x5C
                    W3dAniSpr_SetPosition(AsObject801AAED8(cur)->sprite, (unsigned char*)AsObject801AAED8(cur)->particleStorage + particleByteOffset); // original +0x5C/+0x54
                    W3dAniSpr_SetColor(AsObject801AAED8(cur)->sprite, packedColor); // original +0x5C
                    unsigned char* frameParticleBytes = (unsigned char*)AsObject801AAED8(cur)->particleStorage + particleByteOffset; // original +0x54
                    float rotationRadians = ((ParticleVertexView*)frameParticleBytes)->rotationRadians;
                    W3dAniSpr_DrawFrame(AsObject801AAED8(cur)->sprite, g_view, rotationRadians, (((ParticleDrawFlagsView*)cur)->flags | kQueuedPrimitiveFlag), ((ParticleVertexView*)frameParticleBytes)->frameIndex); // original +0x5C
                }
            }
        }
    }
}
#pragma opt_lifetimes off
#pragma opt_common_subs off
// Draw active emitter sprites and each registered bitmap effect.
extern "C" void CParticleEffect_Draw(char* arg) {
    char* list;
    Elem* cur;
    char* self;
    char* p;
    void* obj;
    int i;
    self = arg;
    {
        WInstanceLock local(self + 0x3C);
        AsCParticleEffect(self)->activeCursor = AsCParticleEffect(self)->activeHead; // original +0x20/+0x1C
        char* initialList = self + 0x3C;
        OSLockMutex(initialList);
        Cursor801A95B4* next;
        Cursor801A95B4* c = (Cursor801A95B4*)AsCParticleEffect(self)->activeCursor; // original +0x20
        if (c) {
            next = c->next;
            cur = (Elem*)c->item;
            if (next == AsCParticleEffect(self)->activeHead) next = 0; // original +0x1C
            AsCParticleEffect(self)->activeCursor = next; // original +0x20
            OSUnlockMutex(initialList);
        } else {
            OSUnlockMutex(initialList);
            cur = 0;
        }
    }
    if (cur) {
    float pivotFraction = kParticleSpriteCenterPivotFraction;
    list = self + 0x3C;
    while (cur) {
        if (AsObject801AAED8(cur)->active) { // original +0x50
            DrawParticle801A95B4(cur, pivotFraction);
        }
        OSLockMutex(list);
        Cursor801A95B4* next;
        Cursor801A95B4* item = (Cursor801A95B4*)AsCParticleEffect(self)->activeCursor; // original +0x20
        if (item) {
            next = item->next;
            cur = (Elem*)item->item;
            if (next == AsCParticleEffect(self)->activeHead) next = 0; // original +0x1C
            AsCParticleEffect(self)->activeCursor = next; // original +0x20
            OSUnlockMutex(list);
        } else {
            OSUnlockMutex(list);
            cur = 0;
        }
    }
    }
    // The bitmap registry owns effects; draw nonnull slots through its inclusive lastSlot.
    obj = AsCParticleEffect(self)->effectManager;
    p = (char*)obj;
    for (i = 0; i <= ((ParticleBitmapRegistryView*)obj)->lastSlot; p += 4, ++i) {
        if (*(void**)p) (*(Draw801A95B4**)p)->draw();
    }
}
#pragma use_lmw_stmw off
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset

#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma use_lmw_stmw on
// Delete active emitters, recycle their list nodes and clear the emitter lookup buckets.
extern "C" void CParticleEffect_OnInit(char* self) {
    int zero = 0;
    g_particleCourseEffects = zero;
    Elem* cur;
    struct Cursor { void* item; char pad[0xC]; Cursor* next; };
    {
        WInstanceLock local(self + 0x3C);
        AsCParticleEffect(self)->activeCursor = AsCParticleEffect(self)->activeHead;
        char* list = self + 0x3C;
        OSLockMutex(list);
        Cursor* next;
        Cursor* cursor = (Cursor*)AsCParticleEffect(self)->activeCursor; // original +0x20
        if (cursor) {
            next = cursor->next;
            cur = (Elem*)cursor->item;
            zero = (next == AsCParticleEffect(self)->activeHead) ? zero : (int)next; // original +0x1C
            AsCParticleEffect(self)->activeCursor = (Cursor*)zero; // original +0x20
            OSUnlockMutex(list);
        } else {
            OSUnlockMutex(list);
            cur = 0;
        }
    }
    if (cur) {
        char* list3 = self + 0x3C;
        int zero2 = 0;
        while (cur) {
            if (cur) {
                if (cur) {
                    void* p = AsObject801AAED8(cur)->sprite; // original +0x5C
                    if (p) {
                        Object_DeleteVirtual(g_resourceManager, p);
                        AsObject801AAED8(cur)->sprite = 0; // original +0x5C
                    }
                    void* q = AsObject801AAED8(cur)->particleStorage; // original +0x54
                    if (q) {
                        if (q) {
                            __dla__FPv((char*)q - 0x10);
                        }
                        AsObject801AAED8(cur)->particleStorage = 0; // original +0x54
                    }
                    __dl__FPv(cur);
                }
            }
            OSLockMutex(list3);
            Cursor* next;
            Cursor* cursor = (Cursor*)AsCParticleEffect(self)->activeCursor; // original +0x20
            if (cursor) {
                next = cursor->next;
                cur = (Elem*)cursor->item;
                if (next == AsCParticleEffect(self)->activeHead) next = 0; // original +0x1C
                AsCParticleEffect(self)->activeCursor = next; // original +0x20
                OSUnlockMutex(list3);
            } else {
                OSUnlockMutex(list3);
                cur = 0;
            }
        }
    }
    {
        WInstanceLock local2(self + 0x3C);
        // Payloads have been destroyed. Splice the active ring onto the free
        // ring, then clear lookup buckets; keep allocation ownership intact.
        Elem* head = (Elem*)AsCParticleEffect(self)->activeHead; // original +0x1C
        // Original circular-list heads at +0x1C (active) and +0x28 (free).
        if (head && AsCParticleEffect(self)->freeHead) {
            head->prev->next = ((Elem*)AsCParticleEffect(self)->freeHead)->next;
            ((Elem*)AsCParticleEffect(self)->freeHead)->next->prev = ((Elem*)AsCParticleEffect(self)->activeHead)->prev;
            ((Elem*)AsCParticleEffect(self)->freeHead)->next = (Elem*)AsCParticleEffect(self)->activeHead;
            ((Elem*)AsCParticleEffect(self)->activeHead)->prev = (Elem*)AsCParticleEffect(self)->freeHead;
            AsCParticleEffect(self)->activeHead = 0;
        } else if (head) {
            Elem* nxt;
            Elem* cur;
            Elem* anchor2;
            char* list3 = self + 0x3C;
            while ((cur = (Elem*)AsCParticleEffect(self)->activeHead)) { // original +0x1C
                OSLockMutex(list3);
                cur->next->prev = cur->prev;
                cur->prev->next = cur->next;
                nxt = cur->next;
                if (nxt == cur) nxt = 0;
                OSUnlockMutex(list3);
                cur = (Elem*)AsCParticleEffect(self)->activeHead; // original +0x1C
                anchor2 = (Elem*)AsCParticleEffect(self)->freeHead; // original +0x28
                OSLockMutex(list3);
                if (!anchor2) {
                    cur->prev = cur;
                    cur->next = cur;
                    OSUnlockMutex(list3);
                } else {
                    cur->next = anchor2;
                    cur->prev = anchor2->prev;
                    anchor2->prev = cur;
                    cur->prev->next = cur;
                    OSUnlockMutex(list3);
                    cur = anchor2;
                }
                AsCParticleEffect(self)->freeHead = cur; // original +0x28
                AsCParticleEffect(self)->activeHead = nxt; // original +0x1C
            }
        }
        int i = 0;
        int off = 0;
        while (i < AsCParticleEffect(self)->slotCount) {
            ++i;
            *(void**)(((char*)AsCParticleEffect(self)->slotBuckets) + off) = 0;
            off += 4;
        }
    }
}
#pragma use_lmw_stmw off
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_strength_reduction reset
#pragma opt_propagation reset

extern "C" char __vt__15CParticleEffect[256];
extern "C" void __ct__18NativeBase80201D18Fv(void*);
#pragma use_lmw_stmw on
// Allocate the particle task and initialize its two synchronized lists and bucket arrays.
extern "C" void* CParticleEffect_Create() {
    char* self = (char*)__nw__FUl(0xA8);
    if (self) {
        __ct__18NativeBase80201D18Fv(self);
        *(void**)self = &__vt__15CParticleEffect;
        AsCParticleEffect(self)->taskMainVtable = (char*)&__vt__15CParticleEffect + kTaskMainVtableOffset;
        AsCParticleEffect(self)->slotBuckets = 0;
        OSInitMutex(self + 0x3C);
        {
            WInstanceLock local(self + 0x3C);
            AsCParticleEffect(self)->activeHead = 0;
            AsCParticleEffect(self)->freeHead = 0;
            AsCParticleEffect(self)->allocationHead = 0;
            AsCParticleEffect(self)->nodeAllocationBatchSize = kInitialEmitterSlots;
            AsCParticleEffect(self)->activeCursor = 0;
            AsCParticleEffect(self)->slotCount = kInitialEmitterSlots;
            if (AsCParticleEffect(self)->slotCount > 0) {
                AsCParticleEffect(self)->slotBuckets = appMallocUni__FUl(kInitialEmitterSlots * sizeof(void*));
                AsCParticleEffect(self)->slotMask = AsCParticleEffect(self)->slotCount - 1;
                for (int i = 0, off = 0; i < AsCParticleEffect(self)->slotCount; ++i, off += 4) {
                    *(void**)(((char*)AsCParticleEffect(self)->slotBuckets) + off) = 0;
                }
            } else {
                AsCParticleEffect(self)->slotBuckets = 0;
            }
        }
        // The second synchronized list is initialized independently of the emitter list.
        AsCParticleEffect(self)->effectManager = 0;
        AsCParticleEffect(self)->secondSlotBuckets = 0;
        OSInitMutex(self + 0x8C);
        {
            WInstanceLock local2(self + 0x8C);
            AsCParticleEffect(self)->secondListHead = 0;
            AsCParticleEffect(self)->secondFreeHead = 0;
            AsCParticleEffect(self)->secondAllocationHead = 0;
            AsCParticleEffect(self)->secondNodeAllocationBatchSize = kInitialEmitterSlots;
            AsCParticleEffect(self)->secondListCursor = 0;
            AsCParticleEffect(self)->secondSlotCount = kInitialEmitterSlots;
            if (AsCParticleEffect(self)->secondSlotCount > 0) {
                AsCParticleEffect(self)->secondSlotBuckets = appMallocUni__FUl(kInitialEmitterSlots * sizeof(void*));
                AsCParticleEffect(self)->secondSlotMask = AsCParticleEffect(self)->secondSlotCount - 1;
                for (int i = 0, off = 0; i < AsCParticleEffect(self)->secondSlotCount; ++i, off += 4) {
                    *(void**)(((char*)AsCParticleEffect(self)->secondSlotBuckets) + off) = 0;
                }
            } else {
                AsCParticleEffect(self)->secondSlotBuckets = 0;
            }
        }
        AsCParticleEffect(self)->lightColumn = 0;
    }
    return self;
}
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
// Free all allocated list nodes and bucket storage, then optionally delete the list owner.
extern "C" void* dtor_801A8DA8(C801A8DA8* self, int flag) {
    if (self) {
        {
            WInstanceLock local(&self->lock20);
            if (self->head) {
                Elem* nxt = 0;
                void** p = &self->lock20;
                Elem* start;
                while ((start = self->head)) {
                    OSLockMutex(p);
                    start->next->prev = start->prev;
                    start->prev->next = start->next;
                    nxt = start->next;
                    if (nxt == start) {
                        nxt = 0;
                    }
                    OSUnlockMutex(p);
                    self->head = nxt;
                    appFreeUni__FPv(start);
                }
            }
            if (self->slotBuckets) {
                appFreeUni__FPv(self->slotBuckets);
                self->slotBuckets = 0;
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma use_lmw_stmw off

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma opt_propagation off
// Destroy pooled effects and active emitters, clear the light column and recycle list nodes.
extern "C" void CParticleEffect_OnDestroy(char* self) {
    if (AsCParticleEffect(self)->effectManager) {
        ParticleEffectPool_Destroy(AsCParticleEffect(self)->effectManager, 1);
        // Pool destruction also releases its light-column effect.
        AsCParticleEffect(self)->effectManager = 0;
    }
    int zero = 0;
    AsCParticleEffect(self)->lightColumn = (void*)zero;
    g_particleCourseEffects = zero;
    Elem* node;
    char* list;
    struct Cursor { void* item; char pad[0xC]; Cursor* next; };
    {
        WInstanceLock local(self + 0x3C);
        AsCParticleEffect(self)->activeCursor = AsCParticleEffect(self)->activeHead;
        list = self + 0x3C;
        OSLockMutex(list);
        Cursor* next;
        Cursor* cursor = *(Cursor**)(self + 0x20);
        if (cursor) {
            next = cursor->next;
            node = (Elem*)cursor->item;
            zero = next == *(Cursor**)(self + 0x1C) ? zero : (int)next;
            *(Cursor**)(self + 0x20) = (Cursor*)zero;
            OSUnlockMutex(list);
        } else {
            OSUnlockMutex(list);
            node = 0;
        }
    }
    if (node) {
        list = self + 0x3C;
        int zero2 = 0;
        while (node) {
            if (node) {
              if (node) {
                void* p = AsObject801AAED8(node)->sprite;
                if (p) { Object_DeleteVirtual(g_resourceManager, p); AsObject801AAED8(node)->sprite = (void*)zero2; }
                void* q = AsObject801AAED8(node)->particleStorage;
                if (q) { if (q) __dla__FPv((char*)q - 0x10); AsObject801AAED8(node)->particleStorage = (void*)zero2; }
                __dl__FPv(node);
              }
            }
            OSLockMutex(list);
            Cursor* next;
            Cursor* cursor = *(Cursor**)(self + 0x20);
            if (cursor) {
                node = (Elem*)cursor->item;
                next = cursor->next;
                if (next == *(Cursor**)(self + 0x1C)) next = 0;
                *(Cursor**)(self + 0x20) = next;
                OSUnlockMutex(list);
            } else {
                OSUnlockMutex(list);
                node = 0;
            }
        }
    }
    {{
        WInstanceLock local2(self + 0x3C);
        // Recycle nodes after payload destruction, then invalidate all emitter lookup buckets.
        Elem* endNode = *(Elem**)(self + 0x1C);
        if (endNode && *(Elem**)(self + 0x28)) {
            (*(Elem**)(self + 0x1C))->prev->next = (*(Elem**)(self + 0x28))->next;
            (*(Elem**)(self + 0x28))->next->prev = (*(Elem**)(self + 0x1C))->prev;
            (*(Elem**)(self + 0x28))->next = *(Elem**)(self + 0x1C);
            (*(Elem**)(self + 0x1C))->prev = *(Elem**)(self + 0x28);
            *(Elem**)(self + 0x1C) = 0;
        } else if (endNode) {
            Elem* nxt;
            Elem* cur;
            Elem* anchor2;
            char* list3 = self + 0x3C;
            while ((cur = *(Elem**)(self + 0x1C))) {
                OSLockMutex(list3);
                cur->next->prev = cur->prev;
                cur->prev->next = cur->next;
                nxt = cur->next;
                if (nxt == cur) nxt = 0;
                OSUnlockMutex(list3);
                cur = *(Elem**)(self + 0x1C);
                anchor2 = *(Elem**)(self + 0x28);
                OSLockMutex(list3);
                if (!anchor2) {
                    cur->prev = cur;
                    cur->next = cur;
                    OSUnlockMutex(list3);
                } else {
                    cur->next = anchor2;
                    cur->prev = anchor2->prev;
                    anchor2->prev = cur;
                    cur->prev->next = cur;
                    OSUnlockMutex(list3);
                    cur = anchor2;
                }
                *(Elem**)(self + 0x28) = cur;
                *(Elem**)(self + 0x1C) = nxt;
            }
        }
        int i = 0;
        int off = 0;
        while (i < AsCParticleEffect(self)->slotCount) {
            ++i;
            *(void**)(((char*)AsCParticleEffect(self)->slotBuckets) + off) = 0;
            off += 4;
        }
    }}
}
#pragma use_lmw_stmw off

#pragma opt_lifetimes reset
#pragma opt_propagation reset
// Construct the owned bitmap-effect registry in its original storage extent.
struct S94 { char d[0x94]; S94() { ParticleBitmapRegistry_Ctor(this); } };
// Construct the light-column effect in its original storage extent.
struct S150 { char d[0x150]; S150() { CLightColumn_Ctor(this); } };
// Create the effect registry and light column, then queue this task for scene preloading.
extern "C" void CParticleEffect_OnPreLoadInit(char* self) {
    char* a = (char*)new S94;
    AsCParticleEffect(self)->effectManager = a;
    char* c = (char*)new S150;
    AsCParticleEffect(self)->lightColumn = c;
    // Pool takes ownership of the light column; the task keeps a non-owning alias.
    ParticleEffectPool_AddAndStart(AsCParticleEffect(self)->effectManager, c, 0, 1);
    // Preload queue receives the secondary task interface at +0x18, not the object base.
    char* r = self;
    if (r) {
        r += 0x18;
    }
    char* g = particleEffectStrings;
    ResourcePreloadQueue_AddRequest(g_sceneManager, 0x6, 0, kFloat_1_particles, r, kFloat_0_805DF3CC, g, 0, 0, 0);
}
