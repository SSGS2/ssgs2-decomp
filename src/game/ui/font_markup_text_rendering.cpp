#include <decomp/static_init.h>
// String-pool view; byte lengths include terminators and original padding.
struct FontMarkupStringPool {
    char sourceTag[5]; // +0x0
    char stringFormat[3]; // +0x5
    char sizeTag[6]; // +0x8
    char pairFormat[6]; // +0xE
    char rectangleTag[6]; // +0x14
    char rectangleFormat[12]; // +0x1A
    char scaleTag[7]; // +0x26
    char floatFormat[3]; // +0x2D
    char offsetTag[8]; // +0x30
};
#define FontMarkup (*((FontMarkupStringPool*)fontMarkupStrings))

#include "classes/W3dAniSpr.h"
#include "classes/WFixedFont.h"
#define AsW3dAniSpr(p) ((W3dAniSpr*)(p))
// Translation unit 0x80219964-0x8021B01C (20 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "classes/WFont.h"
#define AsWFont(p) ((WFont*)(p))

#define FIELD_OFFSET(type, field) ((int)&((type*)0)->field)

enum MarkupStorageConstants {
    kMarkupCameraWidthWord = 0x62, kMarkupCameraHeightWord = 0x63,
    kNameHashFirstByteBits = 0x2a, kNameHashLastByteBits = 0x15,
    kNameHashLengthShift = 6, kInitialFontCacheNodeCount = 4,
    kMarkupTextRunBytes = 0x400, kMarkupImageNameBytes = 0x20,
    kTitleFontInitialFlags = 0x2200, kInlineImageDrawFlags = 0x1200,
    kFontRedMask = 0x00ff0000, kFontGreenMask = 0x0000ff00, kFontBlueMask = 0xff
};
enum { kSpriteBitmapIndexMask = 0x7ff, kFontRequiredFlag = 0x400, kFontScaleOffsets = 0x100, kFontNormalizeWidth = 0x1000 };

extern "C" {

extern "C" float kFloat_6_2831855;
extern "C" float s_fontFullTurnAngle;
extern "C" const float kFloat_0_Font;
extern "C" const float kFloat_1_FixedFont;
extern "C" const float kFloat_1024_SpriteDrawScale;
extern "C" void appFreeUni__FPv(void*);
extern "C" void __dl__FPv(void*);
extern "C" void __ct__8WOverlayFv(void*);
extern "C" void* appMallocUni__FUl(unsigned int);
extern "C" void WOverlay_dtor(void*, int);
extern "C" void Object_DeleteVirtual(void*, void*);
extern "C" char __vt__5WFont[];
extern "C" char __vt__10WTitleFont[];

struct LinkNode { char pad[0xC]; LinkNode* prev; LinkNode* next; };
struct WInstanceLock {
    void* lock;
    // Lock this wrapper address using the existing inline ABI.
    WInstanceLock() { lock = this; OSLockMutex((OSMutex*)this); }
    // Acquire the supplied font/frame mutex for the enclosing scope.
    WInstanceLock(void* l) { lock = l; OSLockMutex((OSMutex*)l); }
    // Release the mutex acquired by this wrapper.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};
// Cache layout +68: activeHead/activeIterator traverse populated entries;
// allocationHead owns blocks, recycledHead holds reusable nodes. Only allocation-ring
// nodes own whole blocks; active/free nodes borrow storage inside those blocks.
struct DtorOwner { char pad0[0x68]; LinkNode* activeHead; LinkNode* activeIterator; LinkNode* allocationHead; LinkNode* recycledHead; char pad78[8]; unsigned int count; unsigned int* array; char mutex[0x18]; };
// Same cache layout viewed from its +68 member, without the font prefix.
struct DtorList { LinkNode* activeHead; LinkNode* activeIterator; LinkNode* allocationHead; LinkNode* recycledHead; char pad10[8]; unsigned int count; unsigned int* array; char mutex[0x18]; };
// Advance the locked resource iterator, stopping when the circular list wraps.
static inline LinkNode* advanceDtorNode(DtorOwner* self, void* lock) {
    OSLockMutex((OSMutex*)lock);
    LinkNode* node = self->activeIterator;
    if (node) {
        LinkNode* item = *(LinkNode**)node;
        LinkNode* next = node->next;
        if (next == self->activeHead) next = 0;
        self->activeIterator = next;
        OSUnlockMutex((OSMutex*)lock);
        return item;
    }
    OSUnlockMutex((OSMutex*)lock);
    return 0;
}
#pragma push
#pragma use_lmw_stmw on
// Delete cached font resources, recycle active nodes and release node/hash allocations.
void* WFont_dtor(DtorOwner* self, int doDelete) {
    if (self) {
        LinkNode* item;
        LinkNode* next;
        *(void**)self = __vt__5WFont;
        {
            void* lock = self->mutex;
            WInstanceLock first(lock);
            self->activeIterator = self->activeHead;
            next = (LinkNode*)((char*)AsWFixedFont(self)->mutex);
            item = advanceDtorNode(self, next);
        }
        if (item) {
            next = (LinkNode*)self->mutex;
            while (item) {
                Object_DeleteVirtual(AsWFont(self)->resourceOwner, item); // original +0x10
                item = advanceDtorNode(self, next);
            }
        }
        {
            WInstanceLock iter(self->mutex);
            if (self->activeHead && self->recycledHead) {
                self->activeHead->prev->next = self->recycledHead->next;
                self->recycledHead->next->prev = self->activeHead->prev;
                self->recycledHead->next = self->activeHead;
                self->activeHead->prev = self->recycledHead;
                self->activeHead = 0;
            } else if (self->activeHead) {
                void* lock = (char*)&self->allocationHead + (FIELD_OFFSET(DtorOwner, mutex) - FIELD_OFFSET(DtorOwner, allocationHead));
                while ((item = self->activeHead) != 0) {
                    OSLockMutex((OSMutex*)lock);
                    item->next->prev = item->prev;
                    item->prev->next = item->next;
                    __typeof__(item->next) savedNext = item->next; next = savedNext;
                    if (next == item) next = 0;
                    OSUnlockMutex((OSMutex*)lock);
                    item = self->activeHead;
                    LinkNode* tail = self->recycledHead;
                    OSLockMutex((OSMutex*)lock);
                    if (0 == tail) {
                        item->prev = item;
                        item->next = item;
                        OSUnlockMutex((OSMutex*)lock);
                    } else {
                        item->next = tail;
                        item->prev = tail->prev;
                        tail->prev = item;
                        item->prev->next = item;
                        OSUnlockMutex((OSMutex*)lock);
                        item = tail;
                    }
                    self->recycledHead = item;
                    self->activeHead = next;
                }
            }
            int i = 0;
            unsigned int offset = 0;
            while (i < (int)self->count) {
                *(unsigned int*)((char*)self->array + offset) = 0;
                i++;
                offset += 4;
            }
        }
        if ((char*)&AsWFixedFont(self)->activeHead) {
            WInstanceLock guard(self->mutex);
            if (self->allocationHead) {
                void* lock = ((DtorList*)&self->activeHead)->mutex;
                while ((item = self->allocationHead) != 0) {
                    OSLockMutex((OSMutex*)lock);
                    item->next->prev = item->prev;
                    item->prev->next = item->next;
                    next = item->next;
                    if (next == item) next = 0;
                    OSUnlockMutex((OSMutex*)lock);
                    self->allocationHead = next;
                    appFreeUni__FPv(item);
                }
            }
            if (self->array) {
                appFreeUni__FPv(self->array);
                self->array = 0;
            }
        }
        WOverlay_dtor(self, 0);
        if (0 < doDelete) __dl__FPv(self);
    }
    return self;
}
#pragma pop

struct CtorNode { char pad[0xC]; CtorNode* prev; CtorNode* next; };
struct CtorOwner { char pad0[0x68]; CtorNode* first; CtorNode* current; CtorNode* extra; CtorNode* end; char pad78[8]; unsigned int count; CtorNode** array; char mutex[0x18]; };
extern "C" void WTitleFont_ReleaseGlyphResources(void*);
#pragma push
#pragma use_lmw_stmw on
// Release title glyph resources before destroying the base font cache.
// Ownership: glyph resources are released by WTitleFont_ReleaseGlyphResources; the node/array
// allocations below are freed under the instance mutex; the base destructor runs last and the
// object itself is freed only when doDelete > 0.
extern "C" void* WTitleFont_dtor(CtorOwner* self, int doDelete) {
    if (self) {
        *(void**)self = __vt__10WTitleFont;
        WTitleFont_ReleaseGlyphResources(self);
        if (self) {
            *(void**)self = __vt__5WFont;
            CtorNode* node;
            CtorNode* item;
            {
                WInstanceLock first(self->mutex);
                self->current = self->first;
                char* lock = (char*)self + FIELD_OFFSET(WFixedFont, mutex);
                OSLockMutex((OSMutex*)lock);
                node = self->current;
                if (node) {
                    item = *(CtorNode**)node;
                    CtorNode* next = node->next;
                    if (next == self->first) next = 0;
                    self->current = next;
                    OSUnlockMutex((OSMutex*)lock);
                } else {
                    OSUnlockMutex((OSMutex*)lock);
                    item = 0;
                }
            }
            if (item) {
                char* lock = (char*)&self->mutex[0];
                while (item) {
                    Object_DeleteVirtual(AsWFont(self)->resourceOwner, item); // original +0x10
                    OSLockMutex((OSMutex*)lock);
                    node = self->current;
                    if (node) {
                        item = *(CtorNode**)node;
                        CtorNode* next = node->next;
                        if (next == self->first) next = 0;
                        self->current = next;
                        OSUnlockMutex((OSMutex*)lock);
                    } else {
                        OSUnlockMutex((OSMutex*)lock);
                        item = 0;
                    }
                }
            }
            {
                WInstanceLock listLock((void*)(self->mutex));
                if (self->first && self->end) {
                    self->first->prev->next = self->end->next;
                    self->end->next->prev = self->first->prev;
                    self->end->next = self->first;
                    self->first->prev = self->end;
                    self->first = 0;
                } else if (self->first) {
                    char* lock2 = (char*)self + FIELD_OFFSET(WFixedFont, mutex);
                    while (self->first) {
                        CtorNode* cur = self->first;
                        CtorNode* next;
                        OSLockMutex((OSMutex*)lock2);
                        cur->next->prev = cur->prev;
                        cur->prev->next = cur->next;
                        next = cur->next;
                        if (next == cur) next = 0;
                        OSUnlockMutex((OSMutex*)lock2);
                        CtorNode* old = self->first;
                        CtorNode* tail = self->end;
                        OSLockMutex((OSMutex*)lock2);
                        if (tail == 0) {
                            old->prev = old;
                            old->next = old;
                            OSUnlockMutex((OSMutex*)lock2);
                        } else {
                            old->next = tail;
                            old->prev = tail->prev;
                            tail->prev = old;
                            old->prev->next = old;
                            OSUnlockMutex((OSMutex*)lock2);
                            old = tail;
                        }
                        self->end = old;
                        self->first = next;
                    }
                }
                int i = 0;
                unsigned int offset = 0;
                while (i < (int)self->count) {
                    i++;
                    *(unsigned int*)((char*)self->array + offset) = 0;
                    offset += 4;
                }
            }
            if ((char*)self + FIELD_OFFSET(WFixedFont, activeHead)) {
                WInstanceLock guard(self->mutex);
                if (self->extra) {
                    char* lock3 = (char*)&self->mutex[0];
                    while (self->extra) {
                        CtorNode* p = self->extra;
                        CtorNode* next;
                        OSLockMutex((OSMutex*)lock3);
                        p->next->prev = p->prev;
                        p->prev->next = p->next;
                        next = p->next;
                        if (next == p) next = 0;
                        OSUnlockMutex((OSMutex*)lock3);
                        self->extra = next;
                        appFreeUni__FPv(p);
                    }
                }
                if (self->array) { appFreeUni__FPv(self->array); self->array = 0; }
            }
            WOverlay_dtor(self, 0);
        }
        if (doDelete > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Destroy a locked node pool and release its bucket array.
void* dtor_80219F08(void* obj, int doDelete) {
    if (obj) {
        char* self = (char*)obj;
        OSLockMutex((OSMutex*)(self + 0x20));
        while (*(LinkNode**)(self + 8)) {
            LinkNode* node = *(LinkNode**)(self + 8);
            LinkNode* next;
            {
                WInstanceLock g(self + 0x20);
                node->prev->next = node->next;
                node->next->prev = node->prev;
                next = node == node->next ? 0 : node->next;
            }
            *(LinkNode**)(self + 8) = next;
            appFreeUni__FPv(node);
        }
        if (*(void**)(self + 0x1c)) {
            appFreeUni__FPv(*(void**)(self + 0x1c));
            *(void**)(self + 0x1c) = 0;
        }
        OSUnlockMutex((OSMutex*)(self + 0x20));
        if (doDelete > 0) __dl__FPv(obj);
    }
    return obj;
}

struct IterOwner { char pad[0x4c]; LinkNode* current; LinkNode* end; char pad54[0x18]; char mutex[0x18]; };
// Return the indexed sprite frame payload, or null when the frame list ends.
void* W3dAniSpr_GetFrameDescription(IterOwner* self, int count) {
    void* result;
    char* lock2;
    {
        char* l0 = (char*)AsW3dAniSpr(self)->frameMutex;
        WInstanceLock outer(l0);
        self->end = self->current;
        lock2 = (char*)AsW3dAniSpr(self)->frameMutex;
        char* lock = lock2;
        OSLockMutex((OSMutex*)lock);
        LinkNode* node = self->end;
        if (node) {
            result = *(void**)node;
            LinkNode* next = node->next;
            if (next == self->current) next = 0;
            self->end = (LinkNode*)next;
            OSUnlockMutex((OSMutex*)lock);
        } else {
            OSUnlockMutex((OSMutex*)lock);
            result = 0;
        }
    }
    lock2 = (char*)AsW3dAniSpr(self)->frameMutex;
    for (int i = 0; i < count && result; ++i) {
        OSLockMutex((OSMutex*)lock2);
        LinkNode* node = self->end;
        if (node) {
            result = *(void**)node;
            LinkNode* next = node->next;
            if (next == self->current) next = 0;
            self->end = (LinkNode*)next;
            OSUnlockMutex((OSMutex*)lock2);
        } else {
            OSUnlockMutex((OSMutex*)lock2);
            result = 0;
        }
    }
    return result;
}

struct Vec4 { float x, y, z, w; };
// Immediate sprite packet: frame data, fixed-point scale, pivot, world position and opacity.
struct SpriteDrawPacket {
    float frameData[4]; unsigned int reserved10; unsigned int scaleFixed10;
    float left, top, right, bottom; float position[3]; unsigned char alpha;
};
// Matrix draw packet mapping (transform is 12 input floats, not inferred axes):
// corner0.frameData[0..2] <- input[0..2]; corner1.v0 <- input[3..5];
// corner1.v28 <- input[6..8]; corner1.v18 <- input[9..11].
// corner2.v18 <- [leftU, topV, bottomV, rightU] from the frame. These three
// borrowed packet buffers are mutated before draw(&corner0,...); do not free them.
// The geometry roles of the three triplets remain unresolved, so vNN field names
// are retained and the exact data mapping is stated instead of assigning axes.
struct RenderObjB { float matrixInput3To5[3]; char pad0c[0xc]; float matrixInput9To11[3]; char pad24[4]; float matrixInput6To8[3]; };
struct RenderObjC { char pad0[0x18]; float frameUvLeftTopBottomRight[4]; };
struct RenderOwner { char pad0[0x4c]; LinkNode* current; LinkNode* end; char pad54[0x18]; char mutex[0x18]; int bitmapCount, frameCount; float left; float right; float top; float bottom; float positionX; float positionY; float positionZ; char padA8[0xe0]; SpriteDrawPacket* corner0; RenderObjB* corner1; RenderObjC* corner2; };
struct NodeData { unsigned int id; float topV, leftU, rightU, bottomV; };
struct DrawVtable { char pad0[0x2c]; void (*draw)(void*, void*, int, int, unsigned int); };
struct DrawObj { DrawVtable* vtable; };
struct DrawInterface {
    virtual void v00(); virtual void v04(); virtual void v08();
    virtual void v0c(); virtual void v10(); virtual void v14();
    virtual void v18(); virtual void v1c(); virtual void v20();
    virtual void draw(void*, int, int, unsigned int);
};
// Return the next locked sprite-frame payload without wrapping the circular list.
static inline NodeData* advanceRenderNode(RenderOwner* self, void* lockptr) {
    OSLockMutex((OSMutex*)lockptr);
    LinkNode* p = self->end;
    if (p) {
        NodeData* result = *(NodeData**)p;
        LinkNode* n = p->next;
        if (n == self->current) n = 0;
        self->end = n;
        OSUnlockMutex((OSMutex*)lockptr);
        return result;
    }
    OSUnlockMutex((OSMutex*)lockptr);
    return 0;
}
// Reset the frame iterator and report whether its first payload exists.
static NodeData* nextNode(RenderOwner* self, int& ok) {
    WInstanceLock outer((void*)((char*)AsW3dAniSpr(self)->frameMutex));
    self->end = self->current;
    NodeData* result = 0;
    {
        WInstanceLock inner((void*)((char*)AsW3dAniSpr(self)->frameMutex));
        LinkNode* p = self->end;
        if (p) {
            result = *(NodeData**)p;
            LinkNode* n = p->next;
            if (n == self->current) n = 0;
            self->end = n;
            ok = 1;
        } else ok = 0;
    }
    return result;
}

// Copy the chosen frame UV rectangle and sprite placement into a draw packet.
void W3dAniSpr_DrawIndexedFrame(RenderOwner* self, DrawObj* draw, int drawFlags, unsigned char opacity, int frameIndex, float drawScale) {
    NodeData* node;
    {
        char* outerptr = self->mutex;
        WInstanceLock outer(outerptr);
        self->end = self->current;
        node = advanceRenderNode(self, (char*)AsW3dAniSpr(self)->frameMutex);
    }
    int i;
    char* looplock = self->mutex;
    for (i = 0; i < frameIndex && node; ++i) {
        node = advanceRenderNode(self, looplock);
    }
    if (!node) return;
    self->corner0->position[0] = self->positionX;
    self->corner0->position[1] = self->positionY;
    self->corner0->position[2] = self->positionZ;
    self->corner0->left = self->left;
    self->corner0->top = self->top;
    self->corner0->right = self->right;
    self->corner0->bottom = self->bottom;
    self->corner0->alpha = opacity;
    unsigned int scaleFixed10 = Runtime_ConvertDoubleToUnsigned(kFloat_1024_SpriteDrawScale * drawScale);
    self->corner0->scaleFixed10 = scaleFixed10;
    self->corner0->frameData[0] = node->leftU; // original +0x00..+0x08
    self->corner0->frameData[1] = node->topV;
    self->corner0->frameData[2] = node->bottomV;
    self->corner0->frameData[3] = node->rightU;
    ((DrawInterface*)draw)->draw(&self->corner0, drawFlags, -1, node->id & kSpriteBitmapIndexMask);
}

// Submit the indexed frame using the supplied twelve transform/corner values.
void W3dAniSpr_DrawIndexedFrameWithMatrix(RenderOwner* self, DrawObj* draw, int drawFlags, const Vec4* transform, int frameIndex) {
    NodeData* node;
    {
        char* outerptr = self->mutex;
        WInstanceLock outer(outerptr);
        self->end = self->current;
        node = advanceRenderNode(self, (char*)AsW3dAniSpr(self)->frameMutex);
    }
    int i;
    char* looplock = self->mutex;
    for (i = 0; i < frameIndex && node; ++i) {
        node = advanceRenderNode(self, looplock);
    }
    if (!node) return;
    float _f11 = transform->x;
    float _f10 = transform->y;
    float _f9 = transform->z;
    float _f8 = transform->w;
    float _f7 = transform[1].x;
    float _f6 = transform[1].y;
    float _f5 = transform[1].z;
    float _f4 = transform[1].w;
    float _f3 = transform[2].x;
    float _f2 = transform[2].y;
    float _f1 = transform[2].z;
    float _f0v = transform[2].w;
    self->corner0->position[0]=self->positionX;
    self->corner0->position[1]=self->positionY;
    self->corner0->position[2]=self->positionZ;
    self->corner0->left=self->left;
    self->corner0->top=self->top;
    self->corner0->right=self->right;
    self->corner0->bottom=self->bottom;
    self->corner0->alpha=0xff;
    self->corner0->frameData[0]=_f11; // original +0x00..+0x08
    self->corner0->frameData[1]=_f10;
    self->corner0->frameData[2]=_f9;
    self->corner1->matrixInput3To5[0]=_f8; // original +0x00..+0x08
    self->corner1->matrixInput3To5[1]=_f7;
    self->corner1->matrixInput3To5[2]=_f6;
    self->corner1->matrixInput6To8[0]=_f5; // original +0x28..+0x30
    self->corner1->matrixInput6To8[1]=_f4;
    self->corner1->matrixInput6To8[2]=_f3;
    self->corner1->matrixInput9To11[0]=_f2; // original +0x18..+0x20
    self->corner1->matrixInput9To11[1]=_f1;
    self->corner1->matrixInput9To11[2]=_f0v;
    self->corner2->frameUvLeftTopBottomRight[0]=node->leftU; // original +0x18..+0x24
    self->corner2->frameUvLeftTopBottomRight[1]=node->topV;
    self->corner2->frameUvLeftTopBottomRight[2]=node->bottomV;
    self->corner2->frameUvLeftTopBottomRight[3]=node->rightU;
    ((DrawInterface*)draw)->draw(&self->corner0, drawFlags, -1, node->id & kSpriteBitmapIndexMask);
}

// Font construction view, matching WFixedFont fields where established:
// a30/a34: unresolved mode words (0/1); field38: glyph scale (1.0);
// color40/44/48: red/green/blue channel masks; flag64: unresolved byte (0).
// a68/a6c: active-list head/iterator; a70: allocation-block ring; a74: free ring;
// a78: node block size (4); a7c: hash mask (3); a80: bucket count (4); a84: buckets.
// aC0/aC4/aC8/aD0/aE0 are cleared title-font extension words, roles unresolved.
// These field names are layout views; semantic names are still provisional.
struct InitObj { char pad0[0x30]; unsigned int a30; unsigned int a34; float glyphScale; unsigned int field3c; unsigned int redChannelMask, greenChannelMask, blueChannelMask; char pad4c[0x18]; unsigned char flag64; char pad65[3]; unsigned int activeHead,activeIterator,allocationHead,recycledHead,nodeBlockCount,bucketMask,bucketCount; void* buckets; char pad88[0x38]; unsigned int aC0,aC4,aC8; char padCC[4]; unsigned int aD0; char padD4[0xC]; unsigned int aE0; };
// Initialize the font cache, RGB channel masks and empty title-glyph resources.
void WTitleFont_Ctor(InitObj* self) {
    // Phase 1: construct the base overlay and empty cache under its own mutex.
    __ct__8WOverlayFv(self);
    self->a30 = 0;
    *(void**)self = __vt__5WFont;
    self->buckets = 0;
    OSInitMutex((OSMutex*)((char*)AsWFixedFont(self)->mutex));
    {
        WInstanceLock guard((char*)AsWFixedFont(self)->mutex);
        self->activeHead = 0; self->recycledHead = 0; self->allocationHead = 0; self->nodeBlockCount = kInitialFontCacheNodeCount; self->activeIterator = 0; self->bucketCount = kInitialFontCacheNodeCount;
        if (self->nodeBlockCount > 0) {
            self->buckets = appMallocUni__FUl(0x10);
            self->bucketMask = self->bucketCount - 1;
            int i = 0;
            int offset = 0;
            while (i < (int)self->bucketCount) {
                *(unsigned int*)((char*)self->buckets + offset) = 0;
                ++i;
                offset += 4;
            }
        } else {
            self->buckets = 0;
        }
    }
    // Phase 2: configure inherited flags/scale/channel masks, then clear title-only extension state.
    // a30/a34/field3c/flag64/aC0..aE0 have no semantic readers in this constructor.
    typedef void (*InitFn)(void*, unsigned int);
    InitFn* vtable = (InitFn*)*(void**)self;
    vtable[3](self, kTitleFontInitialFlags);
    self->glyphScale = kFloat_1_FixedFont;
    self->flag64 = 0;
    self->redChannelMask = kFontRedMask; self->greenChannelMask = kFontGreenMask; self->blueChannelMask = kFontBlueMask;
    /* 0x8021ACB8: the original stores 1 to +0x34 (stw r0, 0x34(r31)), not +0x30 */
    self->a34 = 1;
    *(void**)self = __vt__10WTitleFont;
    self->aC0 = self->aC4 = self->aC8 = self->aD0 = self->aE0 = 0;
}

// Initialize the font module angle constant to one full turn.
void Font_InitAngleConstant() { s_fontFullTurnAngle = kFloat_6_2831855; }
// Store requested font flags with the mandatory base-font flag enabled.
void WFont_SetCoordMode(void* self, int value) { AsWFont(self)->coordMode = value | kFontRequiredFlag; } // original +0x28

// This base-font virtual hook intentionally performs no work.
void WFont_SetFontWidth() {}
// This base-font virtual hook intentionally performs no work.
void WFont_vfunc_7() {}
// The fixed-font implementation returns no title message resource.
int WFixedFont_vfunc_6() { return 0; }
struct FixedFontModeView {
    char pad00[0xA1];
    unsigned char glyphMode;
};
// Set the fixed-font glyph mode byte.
void WFixedFont_SetFixedWidth(FixedFontModeView* self, unsigned char value) { self->glyphMode = value; }
// This base-font virtual hook intentionally performs no work.
void WFont_vfunc_8() {}
// This base-font virtual hook intentionally performs no work.
void WFont_SetFixedWidth() {}
struct TitleFontResourceView {
    char pad00[0xAC];
    unsigned int messageResource;
};
// Return the title font message-resource handle.
unsigned int WTitleFont_vfunc_6(TitleFontResourceView* self) { return self->messageResource; }
// The base-font implementation returns no message resource.
int WFont_vfunc_6() { return 0; }
extern "C" const float kFloat_256_FontMarkup;
extern "C" const float kFloat_640_font;
extern "C" const float kFloat_480_FontMarkupViewport;
extern "C" char fontMarkupStrings[];
extern "C" void* WOverlay_Create(void*, const char*, int);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void WOverlay_DrawTransformedRect(void*, void*, void*, float, float*, void*, void*, void*);
struct TextVtable { char pad[0x2C]; float (*measure)(void*, void*, const char*, float, float, int, unsigned int); float (*measure2)(void*, void*, const char*); };
struct TextNode { void* data; char* name; unsigned char active; char pad09[3]; TextNode* next; TextNode* prev; TextNode* hashNext; };
struct TextOwner {
    TextVtable* vtable;
    char pad04[0xC];
    void* alloc;
    char pad14[0x14];
    unsigned int flags;
    char pad2C[0x14];
    unsigned int palette[9];
    unsigned char state;
    char pad65[3];
    TextNode* activeHead; // active overlays; node owns name, font owns overlay
    char pad6C[4];
    TextNode* allocationHead; // allocation-block ring; node zero of each owned block
    TextNode* freeHead; // unused nodes borrowed from those blocks
    unsigned int nodesPerAllocation; // nodes per allocation, including the block anchor
    unsigned int mask;
    unsigned int bucketCount;
    TextNode** buckets;
    char mutex[0x18];
};
#pragma push
#pragma opt_propagation off
// Link an unlinked node immediately after a non-null ring head; caller holds the mutex.
// These links also serve the allocation/free rings, so payload ownership is unchanged.
static inline void LinkMarkupRingNodeAfter(TextNode* node, TextNode* head) {
    node->prev = head;
    node->next = head->next;
    head->next = node;
    node->next->prev = node;
}

// Detach a node from its ring; caller holds the mutex and updates the owning head.
static inline void UnlinkMarkupRingNode(TextNode* node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

// Initialize the links of a one-node ring; payload/name ownership is unchanged.
static inline void InitializeMarkupRing(TextNode* node) {
    node->next = node;
    node->prev = node;
}

// Hash a name's endpoint bytes and length; caller masks this to a bucket index.
static inline unsigned int MarkupNameHash(char* firstByte, char* name, int length) {
    return (((unsigned char)firstByte[0] & kNameHashFirstByteBits) |
        ((unsigned char)name[length > 0 ? length - 1 : 0] & kNameHashLastByteBits)) |
        (length << kNameHashLengthShift);
}

// Parse palette escapes and inline image tags; draw or measure text with cached overlays.
// text is borrowed NUL-terminated SJIS. Caller must provide well-formed escapes:
// image tags need a closing >, tag/text runs must fit textRun[0x400], names must fit
// imageName[0x20], and image width/height used as UV divisors must be nonzero.
// x/y and offsets use drawing coordinates. Camera float words 0x62/0x63 supply the
// horizontal/vertical extent relative to 640x480 (schema beyond those fields unknown).
// Returns horizontal text advance; images overlay the current pen without advancing.
// Measurement still parses tags and populates the owning font cache. color is ARGB;
// escape digits 1..9 use palette[digit-1], multiplying RGB by tint with /256 truncation
// and preserving input alpha. Digit 0 restores the input color. Unknown escapes skip
// the slash and following byte. A high-bit SJIS byte consumes its trailing byte.
// Cache owns copied names, node blocks and created overlays, released by font teardown.
// Markup grammar (tag keywords live in the pooled strings, see FontMarkupStringPool):
//   \<digit>   digit 0 restores the input colour; 1..9 select palette[digit-1] as a tint.
//   \<...>     inline image tag; the text between "\<" and ">" holds tagged fields, each
//              found by strstr and read with sscanf (value begins right after the keyword):
//                source (value at match+4)    -> imageName (string)
//                size   (value at match+5)    -> w, h (float pair; default 256 x 256)
//                rect   (value at match+5)    -> imageRect[0..3] (default 0,0,w,h)
//                scale  (value at match+6)    -> imageScale (float; default 1.0)
//                offset (value at match+7)    -> imageOffsetX/Y (float pair; default 0,0)
//   Any other character run is passed to the font's text drawing/measuring vfuncs.
// Function structure: per escape token, parse the tag, look the image name up in the hash cache,
// create and register it on a miss, then draw it; plain runs add to the returned advance.
float WFont_PrintOut(TextOwner* self, void* camera, const char* text, int drawFlags, unsigned int color, int draw, float x, float y) {
    float originX = x;
    float originY = y;
    float advance = kFloat_0_Font;
    unsigned int tint;
    if (draw) tint = color;
    unsigned int cGreen = (color >> 8) & 0xff;
    unsigned int cRed = (color >> 16) & 0xff;
    unsigned int cBlue = color & 0xff;
    unsigned int cKeep = color & 0xff000000;
    char textRun[kMarkupTextRunBytes];
    char imageName[kMarkupImageNameBytes];
    char* name = imageName;
    const char* cursor = text;
    // Phase 1 per token: handle palette changes or parse an inline-image tag.
    while (cursor && *cursor) {
        if (*cursor == '\\') {
            if (cursor[1] >= '0' && cursor[1] <= '9') {
                if (cursor[1] == '0') {
                    tint = color;
                } else {
                    unsigned int v = *(unsigned int*)((char*)self + cursor[1] * 4 - ('1' * 4 - FIELD_OFFSET(TextOwner, palette)));
                    unsigned int tb = (v & 0xff) * cBlue;
                    unsigned int ta = ((v >> 16) & 0xff) * cRed;
                    unsigned int tg = ((v >> 8) & 0xff) * cGreen;
                    tint = cKeep | (tb >> 8) | (((ta >> 8) << 16) | (tg & 0xffffff00));
                }
                cursor++;
            } else if (cursor[1] == '<') {
                float imageRect[4];
                float imageWidth = kFloat_256_FontMarkup;
                float imageHeight = kFloat_256_FontMarkup;
                float imageScale = kFloat_1_FixedFont;
                float imageOffsetX = kFloat_0_Font;
                float imageOffsetY = kFloat_0_Font;
                const char* close = strchr(cursor, '>');
                int tagByteCount = close - (cursor + 2);
                memcpy(textRun, cursor + 2, tagByteCount);
                textRun[tagByteCount] = 0;
                cursor = cursor + tagByteCount + 2;
                char* tagField = strstr(textRun, fontMarkupStrings);
                if (tagField) sscanf(tagField + 4, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, stringFormat)), imageName);
                tagField = strstr(textRun, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, sizeTag)));
                if (tagField) sscanf(tagField + 5, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, pairFormat)), &imageWidth, &imageHeight);
                tagField = strstr(textRun, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, rectangleTag)));
                if (tagField) sscanf(tagField + 5, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, rectangleFormat)), &imageRect[0], &imageRect[1], &imageRect[2], &imageRect[3]);
                else { imageRect[1] = kFloat_0_Font; imageRect[0] = kFloat_0_Font; imageRect[2] = imageWidth; imageRect[3] = imageHeight; }
                tagField = strstr(textRun, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, scaleTag)));
                if (tagField) sscanf(tagField + 6, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, floatFormat)), &imageScale);
                tagField = strstr(textRun, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, offsetTag)));
                if (tagField) {
                    sscanf(tagField + 7, (fontMarkupStrings + FIELD_OFFSET(FontMarkupStringPool, pairFormat)), &imageOffsetX, &imageOffsetY);
                    if ((self->flags & kFontScaleOffsets) && camera) {
                        imageOffsetX *= ((float*)camera)[kMarkupCameraWidthWord] / kFloat_640_font;
                        imageOffsetY *= ((float*)camera)[kMarkupCameraHeightWord] / kFloat_480_FontMarkupViewport;
                    }
                }
                // Phase 2: look up the exact image name under the font mutex.
                // Hash = selected alternating bits of first/last bytes | length<<6;
                // mask restricts it to the power-of-two bucket table. strcmp resolves collisions.
                self->state = 1;
                void* found = 0;
                {
                    WInstanceLock guard(self->mutex);
                    if (name && self->buckets) {
                        void* mutex = self->mutex;
                        OSLockMutex((OSMutex*)mutex);
                        int len = (int)strlen(name);
                        unsigned int nameHash = MarkupNameHash(imageName, name, len);
                        unsigned int lookupBucket = nameHash & self->mask;
                        OSUnlockMutex((OSMutex*)mutex);
                        TextNode* node = (TextNode*)self->buckets[lookupBucket];
                        while (node) {
                            if (strcmp(node->name, imageName) == 0) { found = node->data; break; }
                            node = node->hashNext;
                        }
                    }
                }
                // Phase 3 on a miss: create overlay, reserve a node, own a name copy,
                // then link the same node into its hash chain and active circular list.
                // The first node of each allocated block anchors the allocation ring;
                // remaining nodes enter the free ring before one is removed for use.
                // 3a: if no free node exists, allocate nodesPerAllocation nodes (24 bytes each);
                //     node 0 joins the allocation ring, nodes 1.. go on the free ring.
                // 3b: pop one node from the free ring.
                // 3c: fill it (data, active flag, name copy), link it into its hash bucket and
                //     into the active ring (activeHead).
                if (!found) {
                    found = WOverlay_Create(self->alloc, imageName, 0);
                    TextNode* node;
                    void* mutex = self->mutex;
                    {
                        WInstanceLock outer(self->mutex);
                        {
                            WInstanceLock inner(self->mutex);
                            if (self->freeHead == 0) {
                                TextNode* block = (TextNode*)appMallocUni__FUl(self->nodesPerAllocation * sizeof(TextNode));
                                TextNode* allocationRingHead = self->allocationHead;
                                OSLockMutex((OSMutex*)mutex);
                                if (allocationRingHead == 0) {
                                    InitializeMarkupRing(block);
                                    OSUnlockMutex((OSMutex*)mutex);
                                    allocationRingHead = block;
                                } else {
                                    LinkMarkupRingNodeAfter(block, allocationRingHead);
                                    OSUnlockMutex((OSMutex*)mutex);
                                }
                                self->allocationHead = allocationRingHead;
                                int i = 1;
                                if ((int)self->nodesPerAllocation > 1) {
                                    TextNode* recycledNode = block + 1;
                                    while (i < (int)self->nodesPerAllocation) {
                                        TextNode* freeHead = self->freeHead;
                                        OSLockMutex((OSMutex*)mutex);
                                        if (freeHead == 0) {
                                            InitializeMarkupRing(recycledNode);
                                            OSUnlockMutex((OSMutex*)mutex);
                                            freeHead = recycledNode;
                                        } else {
                                            LinkMarkupRingNodeAfter(recycledNode, freeHead);
                                            OSUnlockMutex((OSMutex*)mutex);
                                        }
                                        self->freeHead = freeHead;
                                        recycledNode++;
                                        i++;
                                    }
                                }
                            }
                            // Pop a reusable node; prev selects the next free head by convention.
                            node = self->freeHead;
                            OSLockMutex((OSMutex*)mutex);
                            UnlinkMarkupRingNode(node);
                            TextNode* nextFree = node->prev;
                            if (nextFree == node) nextFree = 0;
                            OSUnlockMutex((OSMutex*)mutex);
                            self->freeHead = nextFree;
                        }
                        node->data = found;
                        node->active = 1;
                        node->name = (char*)appMallocUni__FUl(strlen(imageName) + 1);
                        strcpy(node->name, imageName);
                        OSLockMutex((OSMutex*)mutex);
                        int insertNameLength = (int)strlen(imageName);
                        unsigned int insertNameHash = MarkupNameHash(imageName, name, insertNameLength);
                        unsigned int insertBucket = insertNameHash & self->mask;
                        OSUnlockMutex((OSMutex*)mutex);
                        OSLockMutex((OSMutex*)mutex);
                        node->hashNext = self->buckets[insertBucket];
                        self->buckets[insertBucket] = node;
                        OSUnlockMutex((OSMutex*)mutex);
                        TextNode* activeRingHead = self->activeHead;
                        OSLockMutex((OSMutex*)mutex);
                        if (activeRingHead == 0) {
                            InitializeMarkupRing(node);
                            OSUnlockMutex((OSMutex*)mutex);
                        } else {
                            LinkMarkupRingNodeAfter(node, activeRingHead);
                            OSUnlockMutex((OSMutex*)mutex);
                            node = activeRingHead;
                        }
                        self->activeHead = node;
                    }
                }
                // Phase 4: normalize source rectangle to UVs and draw at pen+offset.
                // Tag rectangles retain their original four-value convention.
                if (draw && found) {
                    ((void(*)(void*, int))(*(void***)found)[3])(found, kInlineImageDrawFlags);
                    float uv[4];
                    uv[3] = imageRect[3] / imageHeight;
                    uv[2] = imageRect[2] / imageWidth;
                    uv[1] = imageRect[1] / imageHeight;
                    uv[0] = imageRect[0] / imageWidth;
                    float pos[4];
                    pos[3] = imageRect[3] * imageScale;
                    pos[2] = imageRect[2] * imageScale;
                    pos[1] = originY + imageOffsetY;
                    pos[0] = originX + imageOffsetX + advance;
                    WOverlay_DrawTransformedRect(found, camera, uv, kFloat_0_Font, pos, (void*)drawFlags, (void*)tint, 0);
                }
            }
            cursor++;
            continue;
        }
        // Phase 5: collect a plain SJIS run up to the next escape and add its advance.
        int len = strlen(cursor);
        const char* stop = 0;
        int i;
        for (i = 0; i < len; i++) {
            if (cursor[i] == '\\') { stop = cursor + i; break; }
            if ((unsigned char)cursor[i] & 0x80) i++;
        }
        if (stop) {
            int tagByteCount = stop - cursor;
            memcpy(textRun, cursor, tagByteCount);
            textRun[tagByteCount] = 0;
            float adv;
            if (draw) adv = self->vtable->measure(self, camera, textRun, originX + advance, originY, drawFlags, tint);
            else adv = self->vtable->measure2(self, camera, textRun);
            advance += adv;
        } else {
            float adv;
            if (draw) adv = self->vtable->measure(self, camera, cursor, originX + advance, originY, drawFlags, tint);
            else adv = self->vtable->measure2(self, camera, cursor);
            advance += adv;
        }
        cursor = stop;
    }
    // Final conversion: optionally return width in the 640-wide reference space.
    int scale = ((self->flags & kFontNormalizeWidth) != 0) && (camera != 0);
    float mul;
    if (scale) {
        float fov = ((float*)camera)[kMarkupCameraWidthWord];
        mul = kFloat_640_font / fov;
    } else mul = kFloat_1_FixedFont;
    return advance * mul;
}
#pragma pop
// Draw markup text at the requested position using the supplied tint.
void WFont_Print(int font, int camera, int text, int drawFlags, int tint, float x, float y) {
    WFont_PrintOut((TextOwner*)font, (void*)camera, (const char*)text, (unsigned)drawFlags, (unsigned)tint, 1, x, y);
}
// Measure markup text without drawing, starting at the origin.
void WFont_GetTextWidth(int font, int camera, int text) {
    WFont_PrintOut((TextOwner*)font, (void*)camera, (const char*)text, 0, 0, 0, kFloat_0_Font, kFloat_0_Font);
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462200", _ctors_u80219964_0, Font_InitAngleConstant);
#pragma pop
