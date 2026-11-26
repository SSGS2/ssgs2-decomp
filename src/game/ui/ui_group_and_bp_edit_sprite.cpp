// Property access for groups of sprites in the BP editor (CBpEditSprite): reading and writing position,
// scale, opacity, tint and enabled state, plus the resource index loader and its tree cleanup helpers.

#include <decomp/static_init.h>
#include <decomp/ct_member.h>
#include "MSL/throw.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthErrorStrings_800C2F30[];
}
}

#include "sdk_protos.h"
#include "classes/CBpObject.h"
#include "classes/MWStringStorage.h"
#define AsCBpObject(p) ((CBpObject*)(p))
#include "classes/CBpEditSprite.h"
#include "classes/CBpSpriteCtrl.h"

extern "C" {

extern float kFloat_6_2831855;
extern float s_uiGroupTwoPi;
extern float s_initializedTwoPi_800C3910;
extern const float kFloat_1024_UIGroup;
extern void* B2dSprite_GetParentGroupObject(void*, void*);
extern const float kFloat_0_0009765625_LayoutMetrics;
extern char __vt__13CBpEditSprite[0x10000];

enum UIGroupConstants {
    kPropertyDirty = 1,
    kItemDisabled = 0x10,
    kOpacityFractionBits = 7,
    kMaxPointerCount = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555,
    kHalfGrowthLimit = 0x2AAAAAAA,
    kLengthErrorTypeOffset = 0x14,
    kScriptPathPending = 2
};
// Group contract: items/count is a borrowed selection of sprite-property objects.
// Setters visit all selected items and return 1 even for an empty selection.
// Getters (except count/enabled) dereference item zero: selection must be nonempty.
// X/Y and source-image dimensions use integer layout/image units; float setters
// truncate toward zero. Scale is signed 1/1024 fixed point. Opacity is 7-bit fixed
// point (128 == full); tint/opacity setters retain word-sized values, not pointers
// to owned objects, despite the reconstructed void* signatures.
// Dirty bit 1 tells the property evaluator to refresh. Disabled bit 0x10 is an
// exclusion bit, so IsEnabled(empty) is true and SetEnabled clears it for nonzero.
// Returned property words and parent-group objects remain owned by the sprite.

// Neutral words in CBpObject, not the unrelated CBpLoad filename/mutex layout.
// Only their reset values are established here; none is named after a filename.
struct BpObjectInitWords {
    char prefix[0x0C];
    int unresolved0C;
    char gap10[0x18];
    int unresolved28, unresolved2C, unresolved30, unresolved34, unresolved38, unresolved3C;
};
typedef char BpInit0C_offset[((int)&((BpObjectInitWords*)0)->unresolved0C == 0x0C) ? 1 : -1];
typedef char BpInit28_offset[((int)&((BpObjectInitWords*)0)->unresolved28 == 0x28) ? 1 : -1];
typedef char BpInit3C_offset[((int)&((BpObjectInitWords*)0)->unresolved3C == 0x3C) ? 1 : -1];

struct BpTransformTreeStorage {
    char prefix[0x44];
    int count;
    void* root;
    void* first;
};
struct BpLoaderMutexView {
    char prefix[0x18];
    char mutex[0x18];
};
enum BpObjectMemberOffset {
    kObjectTree = (int)&((CBpObject*)0)->objectCount,
    kObjectRoot = (int)&((CBpObject*)0)->objectRoot,
    kTransformTree = (int)&((BpTransformTreeStorage*)0)->count,
    kTransformRoot = (int)&((BpTransformTreeStorage*)0)->root,
    kNamedGroupRoot = (int)&((CBpObject*)0)->namedGroupRoot
};

// Selection-view map: every group stores a borrowed item-pointer array at +4
// and (when iterating) count at +8. Getter views expose item zero only. Writable
// position/scale/opacity/tint views expose a dirty short at value+6. They alias the same
// sprite property layout, not separate owned allocations:
// source XY/WH +34/+4C/+64/+7C, position XY +AC/+C4, scale XY +DC/+F4,
// opacity +10C, tint +1B4. Narrow views retain the original load/store types.
// A void* opacity/tint argument is the numeric 32-bit property word itself;
// callers must not pass the address of a number and expect dereferencing.
struct UIGroupSourceMetrics {
    char padding00[0x34];
    int sourceX;
    char padding38[0x14];
    int sourceY;
    char padding50[0x14];
    int sourceWidth;
    char padding68[0x14];
    int sourceHeight;
};

struct UIGroupSourceItems {
    UIGroupSourceMetrics* first;
};

struct UIGroupSourceHandle {
    char padding00[4];
    UIGroupSourceItems* items;
};

struct UISourcePositionItem {
    char padding00[0x34];
    unsigned int sourceX;
    char padding38[0x14];
    unsigned int sourceY;
};

struct UISourcePositionGroup {
    char padding00[4];
    UISourcePositionItem** items;
    unsigned int count;
};

struct UIPositionItem {
    char padding00[0xAC];
    int positionX;
    char paddingB0[2];
    short positionXFlags;
    char paddingB4[0x10];
    int positionY;
    char paddingC8[2];
    short positionYFlags;
};

struct UIPositionGroup {
    char padding00[4];
    UIPositionItem** items;
    unsigned int count;
};

struct UIScaleItem {
    char padding00[0xDC];
    int scaleX;
    char paddingE0[0x14];
    int scaleY;
};

struct UIScaleGroup {
    char padding00[4];
    UIScaleItem** items;
};

struct UIWritableScaleItem {
    char padding00[0xDC];
    int scaleX;
    char paddingE0[2];
    short scaleXFlags;
    char paddingE4[0x10];
    int scaleY;
    char paddingF8[2];
    short scaleYFlags;
};
struct UIWritableScaleGroup {
    char padding00[4];
    UIWritableScaleItem** items;
    unsigned int count;
};

struct UIPositionReadItem {
    char padding00[0xAC];
    int positionX;
    char paddingB0[0x14];
    int positionY;
};
struct UIPositionReadGroup {
    char padding00[4];
    UIPositionReadItem** items;
    char padding08[8];
    struct UIParentGroupHandle* nested;
};
struct UIParentGroupHandle {
    char padding00[4];
    void* parentGroup;
};

struct UIGroupBuffer {
    char padding00[4];
    void* data;
    unsigned int count;
};
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

struct UIGroupVector {
    char padding00[4];
    void** begin;
    unsigned int size;
    unsigned int capacity;
};
extern void* appMallocDummy__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[],
    vectorLengthErrorStrings_800C2F30[];
extern char* strcpy(char*, const char*);
extern void* __nwa__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void __dt__Q23std12length_errorFv();
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct GrowthRef {
    void* table;
    int refs, owners;
    char* text;
    // Attach the exception message allocation to a shared-text control block.
    GrowthRef(char* s) {
        refs = 1;
        owners = 1;
        table = s_sharedCharDeleterVtable;
        text = s;
    }
};
struct GrowthGuard {
    char* text;
    char* flag;
    // Free exception text if shared-control-block construction fails.
    ~GrowthGuard() {
        if (text)
            __dla__FPv(text);
    }
};
struct GrowthBase {
    void* table;
    // Install the logic_error base vtable before message construction.
    GrowthBase() {
        table = __vt__Q23std11logic_error;
    }
    // Run the exception base cleanup without freeing this enclosing object.
    ~GrowthBase() {
        __dt__Q23std9exceptionFv(this, 0);
    }
};
} // extern C
namespace std {
struct length_error : GrowthBase {
    char* text;
    void* shared;
    ~length_error();
    // Allocate/copy the shared length-error message with unwind cleanup.
    length_error() {
        const char* message = vectorLengthErrorStrings_800C2F30;
        int flag;
        *(char*)&flag = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(n * 2 - n);
        char* allocated = text;
        {
            GrowthGuard guard;
            guard.text = allocated;
            guard.flag = (char*)&flag;
            shared = new GrowthRef(allocated);
            guard.text = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
// Build and throw length_error with cleanup for the exception's shared text buffer.
static inline void GrowthThrow() {
    {
        std::__throw_exception<std::length_error>(exception_runtime::vectorLengthErrorStrings_800C2F30 +
                                                      kLengthErrorTypeOffset,
                                                  (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
struct GrowthBuffer {
    void** data;
    unsigned int size, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Free whichever pointer allocation this swap/insert guard currently owns.
    ~GrowthBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Choose a growth bound by reference; neither argument may outlive this expression.
static inline const unsigned int& GrowthMax(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Append a sprite pointer, growing the pointer vector with overflow checks and exception cleanup.
int UIGroupVector_Append(UIGroupVector* self, void* value) {
    // The vector owns only the pointer allocation; value is borrowed. Reallocation
    // invalidates pointers into the array, not the selected sprite objects.
    unsigned int size = self->size;
    unsigned int cap = self->capacity;
    if (size < cap) {
        self->size = ++size;
        self->begin[size - 1] = value;
    } else {
        if ((unsigned int)kMaxPointerCount - self->capacity < 1)
            GrowthThrow();
        unsigned int needed = ((volatile UIGroupVector*)self)->size;
        unsigned int previous = ((volatile UIGroupVector*)self)->capacity;
        unsigned int three, half;
        unsigned int extra = needed + 1 - previous;
        GrowthBuffer buffer;
        buffer.data = 0;
        buffer.size = 0;
        buffer.capacity = 0;
        buffer.owner = &self->capacity;
        buffer.offset = 0;
        unsigned int capacity = self->capacity;
        if (extra > (unsigned int)kMaxPointerCount - capacity)
            GrowthThrow();
        if (capacity < (unsigned int)kThreeFifthsGrowthLimit) {
            three = (capacity + 1) * 3 / 5;
            capacity += GrowthMax(three, extra);
        } else if (capacity < (unsigned int)kHalfGrowthLimit) {
            half = (capacity + 1) / 2;
            capacity += GrowthMax(half, extra);
        } else
            capacity = (unsigned int)kMaxPointerCount;
        if (capacity > (unsigned int)kMaxPointerCount)
            GrowthThrow();
        // Phase 1: allocate and stage the new element before moving old pointer words.
        buffer.data = (void**)appMallocDummy__FUl(capacity * 4);
        buffer.offset = self->size;
        (buffer.data + buffer.offset)[buffer.size] = value;
        buffer.capacity = capacity;
        ++buffer.size;
        void** first = self->begin;
        void** last = self->begin + self->size;
        int count = ((char*)last - (char*)first) / 4;
        unsigned int bytes;
        buffer.offset -= count;
        bytes = count * 4;
        // Phase 2: move pointer words and zero old storage; no pointee is destroyed.
        memcpy(buffer.data + buffer.offset, first, bytes);
        memset(first, 0, bytes);
        buffer.size += count;
        self->size = 0;
        // Phase 3: transfer new storage to the vector; the guard now frees the old array.
        void** old = self->begin;
        void** fresh = buffer.data;
        self->begin = fresh;
        buffer.data = old;
        unsigned int oldCapacity = self->capacity;
        self->capacity = buffer.capacity;
        buffer.capacity = oldCapacity;
        self->size = buffer.size;
        buffer.size = 0;
    }
    return 1;
}
#pragma opt_lifetimes reset
struct BpIndexRecord {
    int id;
    int count;
};
struct BpIndexTable {
    char padding00[3];
    signed char done;
    int first;
    int second;
    int count;
};
union BpEndianWord {
    unsigned int w;
    unsigned char b[4];
};

// Reverse the four bytes of one index-table word without interpreting its sign.
static inline int ByteSwapWord(int value) {
    BpEndianWord dst, src;
    src.w = value;
    dst.w = value;
    dst.b[0] = src.b[3];
    dst.b[1] = src.b[2];
    dst.b[2] = src.b[1];
    dst.b[3] = src.b[0];
    return dst.w;
}

#pragma use_lmw_stmw on
// Convert the resource index header and variable-length index records once from opposite byte order.
int BpResource_ByteSwapIndexTable(BpIndexTable* root) {
    // Mutates a writable file image in place. first is a byte offset from root.
    // Each record is id/count followed by count words, with a minimum 12-byte
    // footprint even for count == 0. No file-size/offset validation is performed.
    // done == 1 prevents a second swap; null returns 0, all nonnull paths return 1.
    if (!root)
        return 0;
    if (root->done)
        return 1;
    // The index table is little-endian on disc.
    int count;
    int second;
    int first;
    first = root->first;
    second = root->second;
    count = root->count;
    root->first = ByteSwapWord(first);
    root->second = ByteSwapWord(second);
    root->count = ByteSwapWord(count);
    BpIndexRecord* node = (BpIndexRecord*)(root->first + (int)root);
    for (int i = 0; i < root->count; i++) {
        node->id = ByteSwapWord(node->id);
        node->count = ByteSwapWord(node->count);
        int* p = (int*)node;
        for (int j = 0; j < node->count; j++) {
            p[2] = ByteSwapWord(p[2]);
            p++;
        }
        node = (BpIndexRecord*)((char*)node + 12 + (node->count < 1 ? 0 : node->count - 1) * 4);
    }
    root->done = 1;
    return 1;
}

struct UITrivialTreeLinks {
    void* first;
    void* second;
};
extern void RBTree_FreeTrivialSubtree(void*, void*);
extern void MapTree_FreeSubtree(void*, void*);

// Free a node after delegating both subtrees; payloads are trivial, with no pointee deletion.
static void fn_800C3FE0_FreeNode(void* owner, UITrivialTreeLinks* node) {
    if (!node)
        return;
    if (node->first)
        RBTree_FreeTrivialSubtree(owner, node->first);
    if (node->second)
        RBTree_FreeTrivialSubtree(owner, node->second);
    appFreeUni__FPv(node);
}

// Expand one more postorder level before freeing this trivial-payload node.
static void FreeNodeAndChildren(void* owner, UITrivialTreeLinks* node) {
    if (!node)
        return;
    fn_800C3FE0_FreeNode(owner, (UITrivialTreeLinks*)node->first);
    fn_800C3FE0_FreeNode(owner, (UITrivialTreeLinks*)node->second);
    appFreeUni__FPv(node);
}

// Expand two postorder levels before freeing this trivial-payload node.
static void fn_800C3FE0_FreeTree(void* owner, UITrivialTreeLinks* node) {
    if (!node)
        return;
    FreeNodeAndChildren(owner, (UITrivialTreeLinks*)node->first);
    FreeNodeAndChildren(owner, (UITrivialTreeLinks*)node->second);
    appFreeUni__FPv(node);
}

#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Release all trivial tree nodes; preserve the expanded cleanup and deleting-destructor convention.
void* dtor_800C3FE0(UITrivialTreeLinks* self, int destroy) {
    // Header root is +4; first/second links below are child links, not payloads.
    // Phase 1: expanded postorder frees every node. Phase 2 at the end optionally
    // frees the header; it must not be used as an initialized empty tree afterward.
    if (self) {
        // CodeWarrior emits each repeated receiver branch; keep all five gates.
        if (self)
            if (self)
                if (self)
                    if (self)
                        if (self) {
                            UITrivialTreeLinks* root = (UITrivialTreeLinks*)self->second;
                            if (root) {
                                UITrivialTreeLinks* branch = (UITrivialTreeLinks*)root->first;
                                if (branch) {
                                    UITrivialTreeLinks* node = (UITrivialTreeLinks*)branch->first;
                                    if (node) {
                                        UITrivialTreeLinks* leaf = (UITrivialTreeLinks*)node->first;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        leaf = (UITrivialTreeLinks*)node->second;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    UITrivialTreeLinks* node2 = (UITrivialTreeLinks*)branch->second;
                                    if (node2) {
                                        UITrivialTreeLinks* leaf = (UITrivialTreeLinks*)node2->first;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        leaf = (UITrivialTreeLinks*)node2->second;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        appFreeUni__FPv(node2);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                UITrivialTreeLinks* other = (UITrivialTreeLinks*)root->second;
                                if (other) {
                                    UITrivialTreeLinks* node = (UITrivialTreeLinks*)other->first;
                                    if (node) {
                                        UITrivialTreeLinks* leaf = (UITrivialTreeLinks*)node->first;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        leaf = (UITrivialTreeLinks*)node->second;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    node = (UITrivialTreeLinks*)other->second;
                                    if (node) {
                                        UITrivialTreeLinks* leaf = (UITrivialTreeLinks*)node->first;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        leaf = (UITrivialTreeLinks*)node->second;
                                        fn_800C3FE0_FreeNode(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    appFreeUni__FPv(other);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_lifetimes reset

struct UIMapTreeLinks {
    void* first;
    void* second;
};
// Release deeper map descendants before this leaf; payloads require no destructor.
static inline void ReleaseUIMapLeaf(void* owner, UIMapTreeLinks* leaf) {
    if (leaf) {
        if (leaf->first)
            MapTree_FreeSubtree(owner, leaf->first);
        if (leaf->second)
            MapTree_FreeSubtree(owner, leaf->second);
        appFreeUni__FPv(leaf);
    }
}
#pragma opt_lifetimes off
// Release all map nodes; preserve the expanded cleanup and deleting-destructor convention.
void* dtor_800C42A4(void* self, int destroy) {
    // Phase 1: free child subtrees before parents. No payload destructor appears
    // in this specialization. Phase 2 optionally frees self when destroy > 0.
    if (self) {
        // CodeWarrior emits each repeated receiver branch; keep all five gates.
        if (self)
            if (self)
                if (self)
                    if (self)
                        if (self) {
                            UIMapTreeLinks* root = *(UIMapTreeLinks**)((char*)self + 4);
                            if (root) {
                                UIMapTreeLinks* branch = (UIMapTreeLinks*)root->first;
                                if (branch) {
                                    UIMapTreeLinks* node = (UIMapTreeLinks*)branch->first;
                                    if (node) {
                                        UIMapTreeLinks* leaf = (UIMapTreeLinks*)node->first;
                                        ReleaseUIMapLeaf(self, leaf);
                                        leaf = (UIMapTreeLinks*)node->second;
                                        ReleaseUIMapLeaf(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    UIMapTreeLinks* node2 = (UIMapTreeLinks*)branch->second;
                                    if (node2) {
                                        UIMapTreeLinks* leaf = (UIMapTreeLinks*)node2->first;
                                        ReleaseUIMapLeaf(self, leaf);
                                        leaf = (UIMapTreeLinks*)node2->second;
                                        ReleaseUIMapLeaf(self, leaf);
                                        appFreeUni__FPv(node2);
                                    }
                                    appFreeUni__FPv(branch);
                                }
                                UIMapTreeLinks* other = (UIMapTreeLinks*)root->second;
                                if (other) {
                                    UIMapTreeLinks* node = (UIMapTreeLinks*)other->first;
                                    if (node) {
                                        UIMapTreeLinks* leaf = (UIMapTreeLinks*)node->first;
                                        ReleaseUIMapLeaf(self, leaf);
                                        leaf = (UIMapTreeLinks*)node->second;
                                        ReleaseUIMapLeaf(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    node = (UIMapTreeLinks*)other->second;
                                    if (node) {
                                        UIMapTreeLinks* leaf = (UIMapTreeLinks*)node->first;
                                        ReleaseUIMapLeaf(self, leaf);
                                        leaf = (UIMapTreeLinks*)node->second;
                                        ReleaseUIMapLeaf(self, leaf);
                                        appFreeUni__FPv(node);
                                    }
                                    appFreeUni__FPv(other);
                                }
                                appFreeUni__FPv(root);
                            }
                        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_lifetimes reset

// Set the integer source-image origin on every item in the group.
int UIGroup_SetSourcePosition(UISourcePositionGroup* group, float* position) {
    UISourcePositionItem** item = group->items;
    // Original 0x800C3124: visit each item, testing the end before the body.
    while (item != group->items + group->count) {
        int sourceY;
        int sourceX;
        UISourcePositionItem* entry;
        sourceX = (int)position[0];
        entry = *item;
        sourceY = (int)position[1];
        ++item;
        entry->sourceX = sourceX;
        entry->sourceY = sourceY;
    }
    return 1;
}

// Read the first item's scale, converting 1/1024 units to floating point.
int UIGroup_GetScale(UIScaleGroup* group, float* out) {
    UIScaleItem* data = *group->items;
    out[0] = (float)data->scaleX * kFloat_0_0009765625_LayoutMetrics;
    out[1] = (float)data->scaleY * kFloat_0_0009765625_LayoutMetrics;
    return 1;
}

// Set both scale channels in 1/1024 units and mark their properties dirty.
int UIGroup_SetScale(UIWritableScaleGroup* group, float* value) {
    float scale = kFloat_1024_UIGroup;
    UIWritableScaleItem** item = group->items;

    // Original 0x800C31DC: the array end is reloaded for each iteration.
    while (item != group->items + group->count) {
        UIWritableScaleItem* entry = *item++;

        float x = scale * (value[0]);
        short xFlags = entry->scaleXFlags;
        xFlags = (short)((xFlags | kPropertyDirty));
        float y = scale * (value[1]);

        entry->scaleXFlags = xFlags;
        short yFlags = entry->scaleYFlags;

        yFlags = (short)(yFlags | kPropertyDirty);
        entry->scaleYFlags = yFlags;
        entry->scaleX = ((int)x);
        entry->scaleY = ((int)y);
    }
    return 1;
}

// Read the first item's position and add its parent-group item's position when present.
int UIGroup_GetPosition(UIPositionReadGroup* group, float* position) {
    // Adds just the parent object's position; no scale/rotation or recursive
    // ancestor transform is applied, so this is not a general world transform.
    UIPositionReadItem* first = *group->items;
    position[0] = (float)first->positionX;
    position[1] = (float)first->positionY;
    UIPositionReadItem* parentItem =
        (UIPositionReadItem*)B2dSprite_GetParentGroupObject(group->nested->parentGroup, first);
    if (parentItem) {
        position[0] += (float)parentItem->positionX;
        position[1] += (float)parentItem->positionY;
    }
    return 1;
}

#pragma opt_common_subs off
// Release the selected-item pointer vector and optionally delete the edit-sprite object.
CBpEditSprite::~CBpEditSprite() {
    if ((char*)this + 4) {
        if ((char*)this + 4) {
            if ((char*)this + 4) {
                if ((char*)this + 4) {
                    if ((char*)this + 4) {
                        void* data = ((void**)this)[1];
                        if (data) {
                            int oldCount = (*(int*)&selectedItemCount);
                            unsigned int removedCount = selectedItemCount;
                            selectedItemCount = oldCount - removedCount;
                            appFreeUni__FPv(data);
                        }
                    }
                }
            }
        }
    }
}
#pragma opt_common_subs reset

// Set both integer position channels and mark their properties dirty.
int UIGroup_SetPosition(UIPositionGroup* group, const float* value) {
    UIPositionItem** item = group->items;
    // Original 0x800C2FDC.
    while (item != group->items + group->count) {
        float x = value[0];
        UIPositionItem* entry = *item++;

        short xFlags = entry->positionXFlags;
        float y = value[1];

        xFlags = (short)((xFlags | kPropertyDirty));
        entry->positionXFlags = xFlags;

        short yFlags = entry->positionYFlags;
        yFlags = (short)((kPropertyDirty | yFlags));
        entry->positionYFlags = yFlags;
        entry->positionX = ((int)x);
        entry->positionY = ((int)y);
    }
    return 1;
}

struct UIOpacityParentHandle {
    char padding00[4];
    void* parentGroup;
    char padding08[4];
    void* owner;
};

struct UIOpacityGroup {
    char padding00[4];
    void** items;
    char padding08[8];
    UIOpacityParentHandle* parent;
};

struct UIOpacityReadItem {
    char padding00[0x10C];
    int opacity;
};

extern void* B2dSprite_GetParentGroupObject(void*, void*);

// Read the first item's source-image origin.
int UIGroup_GetSourcePosition(UIGroupSourceHandle* group, float* out) {
    UIGroupSourceMetrics* data = group->items->first;
    out[0] = (float)data->sourceX;
    out[1] = (float)data->sourceY;
    return 1;
}

// Multiply local and parent opacity in seven-bit fixed point when a parent exists.
int UIGroup_GetCombinedOpacity(UIOpacityGroup* group) {
    // parent/local values are multiplied before >>7; this does not clamp the result.
    int opacity = ((UIOpacityReadItem*)*group->items)->opacity;
    void* obj = group->parent->parentGroup;
    UIOpacityReadItem* result = (UIOpacityReadItem*)B2dSprite_GetParentGroupObject(obj, *group->items);
    if (result != 0)
        opacity = (opacity * result->opacity) >> kOpacityFractionBits;
    return opacity;
}

// Read the first item's source-image width and height.
int UIGroup_GetSourceSize(UIGroupSourceHandle* group, float* out) {
    UIGroupSourceMetrics* data = group->items->first;
    out[0] = (float)data->sourceWidth;
    out[1] = (float)data->sourceHeight;
    return 1;
}

struct UITintReadGroup {
    char padding00[4];
    struct UITintReadItems* items;
};

struct UITintReadItems {
    void* first;
};

struct UITintReadItem {
    char padding00[0x1B4];
    void* tint;
};

struct UIEditSpriteStorage {
    void* vtable;
    unsigned int items;
    unsigned int count;
    unsigned int capacity;
    void* owner;
};

struct UIOpacityItem {
    char padding00[0x10C];
    void* opacity;
    char padding110[2];
    short opacityFlags;
};

struct UIOpacityItems {
    char padding00[4];
    UIOpacityItem** items;
    unsigned int count;
};

// Return the first item's tint-color property.
void* UIGroup_GetTintColor(UITintReadGroup* group) {
    return ((UITintReadItem*)group->items->first)->tint;
}

// Initialize an empty selected-item vector and remember its owning sprite group.
CBpEditSprite::CBpEditSprite(void* value) {
    UIEditSpriteStorage* arg = (UIEditSpriteStorage*)this;
    char* address = __vt__13CBpEditSprite;
    arg->items = 0;
    arg->vtable = address;
    arg->count = 0;
    arg->capacity = 0;
    arg->owner = value;
}

// Set every selected item's opacity and mark the property dirty.
// opacityWord is immediate 7-bit-fixed-point opacity (128 == full), never dereferenced.
int UIGroup_SetOpacity(UIOpacityItems* group, void* opacityWord) {
    UIOpacityItem** item = group->items;
    // Original 0x800C32C4.
    while (item != group->items + group->count) {
        UIOpacityItem* entry = *item++;
        entry->opacityFlags = (short)(entry->opacityFlags | kPropertyDirty);
        entry->opacity = opacityWord;
    }
    return 1;
}

struct UIEnabledItem {
    char padding00[2];
    short flags;
};

struct UIEnabledGroup {
    char padding00[4];
    UIEnabledItem** items;
    unsigned int count;
};

// Clear or set the disabled bit on every selected item.
int UIGroup_SetEnabled(UIEnabledGroup* group, int value) {
    UIEnabledItem** item = group->items;
    // Original 0x800C2F44.
    while (item != group->items + group->count) {
        UIEnabledItem* entry = *item;
        if (value != 0)
            entry->flags = (short)(entry->flags & ~kItemDisabled);
        else
            entry->flags = (short)(entry->flags | kItemDisabled);
        ++item;
    }
    return 1;
}

// Return true only when no selected item carries the disabled bit.
int UIGroup_IsEnabled(UIEnabledGroup* group) {
    UIEnabledItem* entry;
    UIEnabledItem** item;
    unsigned int flags;
    UIEnabledItem** end;
    end = group->items + group->count;
    item = group->items;
    flags = 0;
    // Original 0x800C2FA4: accumulate the disabled flags up to the saved end.
    while (item != end) {
        entry = *item++;
        flags |= entry->flags & kItemDisabled;
    }
    return __rlwnm(1, __cntlzw(flags), 31, 31);
}

struct UITintItem {
    char padding00[0x1B4];
    void* tint;
    char padding1B8[2];
    short flags;
};

struct UITintGroup {
    char padding00[4];
    UITintItem** items;
    unsigned int count;
};

// Set every selected item's tint and mark the property dirty.
// tintWord is copied verbatim as a packed property; its channel order is unresolved here.
int UIGroup_SetTintColor(UITintGroup* group, void* tintWord) {
    UITintItem** item = group->items;
    // Original 0x800C33B0.
    while (item != group->items + group->count) {
        UITintItem* entry = *item++;
        entry->flags = (short)(entry->flags | kPropertyDirty);
        entry->tint = tintWord;
    }
    return 1;
}

struct UIGroupCountView {
    char padding00[8];
    unsigned int count;
};

// Return the number of selected sprite items.
unsigned int UIGroup_GetItemCount(UIGroupCountView* group) {
    return group->count;
}

// Initialize this UI-group unit's cached two-pi constant.
void UIGroup_InitializeTwoPi() {
    s_uiGroupTwoPi = kFloat_6_2831855;
}

// Initialize this resource-index unit's cached two-pi constant.
void BpResource_InitializeTwoPi() {
    s_initializedTwoPi_800C3910 = kFloat_6_2831855;
}

struct BpObjectTreeLinks {
    void* left;
    void* right;
};
// Release the two deeper object-tree subtrees before freeing their parent node.
static inline void ReleaseBpObjectLeaf(void* owner, BpObjectTreeLinks* leaf) {
    if (leaf) {
        if (leaf->left)
            RBTree_FreeTrivialSubtree(owner, leaf->left);
        if (leaf->right)
            RBTree_FreeTrivialSubtree(owner, leaf->right);
        appFreeUni__FPv(leaf);
    }
}
struct TableLock {
    int active;
    int index;
    ~TableLock();
};
typedef TableLock fn_800C3AFC_Lock;
struct BpTransformWords {
    unsigned int a, b, c;
};
struct BpTransformPair {
    int key;
    BpTransformWords value;
};
struct BpTransformNode {
    char padding00[0xc];
    BpTransformPair pair;
};
extern void TableLock_ctor(fn_800C3AFC_Lock*, int, int);
extern void* __nw__FUl(unsigned int);
struct CBpSpriteCtrlStorage {
    char data[0x28];
    // Construct the owned sprite-pool controller in the wrapper's existing storage.
    CBpSpriteCtrlStorage() {
        ((CBpSpriteCtrl*)this)->__ct();
    }
};
extern void* BpIntObjectTree_FindInsertionPosition(void*, const void*, void**, unsigned char*, unsigned char*);
extern void* BpStateCallbackTree_InsertNode(void*, void*, unsigned char, unsigned char, void*);
extern unsigned int s_bpResourceSetCallbackState;
extern unsigned int s_bpRunningCallbackState;
extern BpTransformWords zeroInitializationWords;
extern BpTransformWords s_bpRunningStateCallback;
extern char __vt__7CBpLoad[];
extern char __vt__9CBpObject[];
extern const float kZeroCallbackPosition;
extern void __dt__9TableLockFv(fn_800C3AFC_Lock*, int);

#pragma opt_lifetimes off
#pragma opt_propagation off
// Initialize the BP object, empty its registries, create the loader and sprite controller, and seed transforms.
CBpObject::CBpObject() {
    // Empty tree iterators point at their embedded root fields. The neutral
    // initialization view covers CBpObject words whose meanings are unresolved;
    // the owned CBpLoad is allocated separately in phase 2.
    char* p = (char*)this;
    *(void**)p = __vt__9CBpObject;
    (*(int*)&AsCBpObject(p)->script) = -1;
    ((BpObjectInitWords*)p)->unresolved0C = 0;
    (*(int*)&AsCBpObject(p)->scriptBuffer) = 0;
    AsCBpObject(p)->scriptState = kScriptPathPending;
    // Object registry: count, root and sentinel iterator.
    AsCBpObject(p)->objectCount = 0;
    AsCBpObject(p)->objectRoot = 0;
    AsCBpObject(p)->firstObject = p + kObjectRoot;
    ((BpObjectInitWords*)p)->unresolved28 = 0;
    ((BpObjectInitWords*)p)->unresolved2C = 0;
    ((BpObjectInitWords*)p)->unresolved30 = 1;
    ((BpObjectInitWords*)p)->unresolved34 = 0;
    ((BpObjectInitWords*)p)->unresolved38 = 0;
    ((BpObjectInitWords*)p)->unresolved3C = 0;
    // Transform and auxiliary registries: empty trees with sentinel iterators.
    ((BpTransformTreeStorage*)p)->count = 0;
    ((BpTransformTreeStorage*)p)->root = 0;
    ((BpTransformTreeStorage*)p)->first = p + kTransformRoot;
    AsCBpObject(p)->namedGroupCount = 0;
    AsCBpObject(p)->namedGroupRoot = 0;
    AsCBpObject(p)->firstNamedGroup = p + kNamedGroupRoot;
    AsMWStringStorage(AsCBpObject(p)->scriptPathStorage)->flags = 0;
    AsMWStringStorage(AsCBpObject(p)->scriptPathStorage)->length = 0;
    AsMWStringStorage(AsCBpObject(p)->scriptPathStorage)->data = 0;
    // Phase 1: initialize registries and clear any object-tree nodes under TableLock.
    fn_800C3AFC_Lock lock;
    TableLock_ctor(&lock, 1, 1);
    BpObjectTreeLinks* root = (BpObjectTreeLinks*)*(void**)(p + kObjectRoot);
    if (root) {
        BpObjectTreeLinks* branchA = (BpObjectTreeLinks*)root->left;
        if (branchA) {
            BpObjectTreeLinks* nodeA = (BpObjectTreeLinks*)branchA->left;
            if (nodeA) {
                BpObjectTreeLinks* leafA = (BpObjectTreeLinks*)nodeA->left;
                ReleaseBpObjectLeaf(p + kObjectTree, leafA);
                leafA = (BpObjectTreeLinks*)nodeA->right;
                ReleaseBpObjectLeaf(p + kObjectTree, leafA);
                appFreeUni__FPv(nodeA);
            }
            BpObjectTreeLinks* nodeB = (BpObjectTreeLinks*)branchA->right;
            if (nodeB) {
                BpObjectTreeLinks* leafB = (BpObjectTreeLinks*)nodeB->left;
                ReleaseBpObjectLeaf(p + kObjectTree, leafB);
                leafB = (BpObjectTreeLinks*)nodeB->right;
                ReleaseBpObjectLeaf(p + kObjectTree, leafB);
                appFreeUni__FPv(nodeB);
            }
            appFreeUni__FPv(branchA);
        }
        BpObjectTreeLinks* branchC = (BpObjectTreeLinks*)root->right;
        if (branchC) {
            BpObjectTreeLinks* nodeC = (BpObjectTreeLinks*)branchC->left;
            if (nodeC) {
                BpObjectTreeLinks* leafC = (BpObjectTreeLinks*)nodeC->left;
                ReleaseBpObjectLeaf(p + kObjectTree, leafC);
                leafC = (BpObjectTreeLinks*)nodeC->right;
                ReleaseBpObjectLeaf(p + kObjectTree, leafC);
                appFreeUni__FPv(nodeC);
            }
            BpObjectTreeLinks* nodeD = (BpObjectTreeLinks*)branchC->right;
            if (nodeD) {
                BpObjectTreeLinks* leafD = (BpObjectTreeLinks*)nodeD->left;
                ReleaseBpObjectLeaf(p + kObjectTree, leafD);
                leafD = (BpObjectTreeLinks*)nodeD->right;
                ReleaseBpObjectLeaf(p + kObjectTree, leafD);
                appFreeUni__FPv(nodeD);
            }
            appFreeUni__FPv(branchC);
        }
        appFreeUni__FPv(root);
        AsCBpObject(p)->objectCount = 0;
        *(void**)(p + kObjectRoot) = 0;
        AsCBpObject(p)->firstObject = p + kObjectRoot;
    }
    {
        // Phase 2: allocate the owned loader and sprite-pool controller.
        void* sprite;
        void* obj;
        obj = (void*)__nw__FUl(0x30);
        if (obj) {
            ((void**)obj)[0] = (void*)__vt__7CBpLoad;
            ((int*)obj)[1] = 0;
            ((int*)obj)[2] = 0;
            ((int*)obj)[3] = 0;
            ((int*)obj)[4] = 0;
            ((int*)obj)[5] = 0;
            OSInitMutex((OSMutex*)((BpLoaderMutexView*)obj)->mutex);
        }
        AsCBpObject(p)->loader = obj;
        sprite = new CBpSpriteCtrlStorage;
        AsCBpObject(p)->objectArray = sprite;
    }
    // Phase 3: find-or-insert two transform keys and assign their three raw words.
    // Key meanings are unresolved; preserve both shared initial-value records.
    unsigned int second1;
    unsigned int first1;
    BpTransformWords* vec1;
    BpTransformPair* pair;
    BpTransformNode* found;
    unsigned char flagB, flagA;
    void* pos = BpIntObjectTree_FindInsertionPosition(p + kTransformTree, &s_bpResourceSetCallbackState, (void**)&found,
                                                      &flagA, &flagB);
    if (found == 0 || found->pair.key < (int)s_bpResourceSetCallbackState) {
        BpTransformPair value;
        BpTransformWords* vec = &zeroInitializationWords;
        int key;
        key = (int)s_bpResourceSetCallbackState;
        value.key = key;
        value.value = *vec;
        pair = &((BpTransformNode*)BpStateCallbackTree_InsertNode(p + kTransformTree, pos, flagA, flagB, &value))->pair;
    } else {
        pair = &found->pair;
    }
    vec1 = &zeroInitializationWords;
    first1 = vec1->a;
    second1 = vec1->b;
    pair->value.a = first1;
    pair->value.b = second1;
    pair->value.c = vec1->c;
    unsigned int second3;
    unsigned int first3;
    BpTransformWords* vec3;
    BpTransformPair* pair2;
    BpTransformNode* found2;
    unsigned char flag2B, flag2A;
    void* pos2 = BpIntObjectTree_FindInsertionPosition(p + kTransformTree, &s_bpRunningCallbackState, (void**)&found2,
                                                       &flag2A, &flag2B);
    if (found2 == 0 || found2->pair.key < (int)s_bpRunningCallbackState) {
        BpTransformPair value2;
        BpTransformWords* vec2 = &zeroInitializationWords;
        int key2;
        key2 = (int)s_bpRunningCallbackState;
        value2.key = key2;
        value2.value = *vec2;
        pair2 = &((BpTransformNode*)BpStateCallbackTree_InsertNode(p + kTransformTree, pos2, flag2A, flag2B, &value2))
                     ->pair;
    } else {
        pair2 = &found2->pair;
    }
    vec3 = &s_bpRunningStateCallback;
    first3 = vec3->a;
    second3 = vec3->b;
    pair2->value.a = first3;
    pair2->value.b = second3;
    pair2->value.c = vec3->c;
    AsCBpObject(p)->callbackPosition[0] = kZeroCallbackPosition;
    AsCBpObject(p)->callbackPosition[1] = kZeroCallbackPosition;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620A8", _ctors_u800C2F30_0, UIGroup_InitializeTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU804620A8", _ctors_u800C2F30_1, BpResource_InitializeTwoPi);
#pragma pop
