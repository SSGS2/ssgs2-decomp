#pragma push
// Translation unit 0x801BFDC4-0x801C06BC (15 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "classes/CSharedDoc.h"

struct CourseObjectView {
    char reservedPrefix[0x14];
    unsigned int rawWord14; // Bit word: the observed caller stores 0x01000000, not an address.
    char reservedBeforeResource[0x108 - 0x18];
    unsigned int triangleCount; // WPuppet::triangleCount (+0x108), used to size collision storage.
    char reservedBeforeBounds[0x124 - 0x10C];
    char embeddedData[0x164 - 0x124];
    unsigned int metadataWord;
    char reservedBeforeFlags[0x360 - 0x168];
    unsigned char statusByte;
};

struct CourseSelectionView {
    char reservedPrefix[0x9C6];
    unsigned char courseVariant;
    char reservedBeforeName[0x9E4 - 0x9C7];
    char modelBaseName[0x20];
    char reservedBeforeGolfDoc[0xB90 - 0xA04];
    void* golfDoc;
};

struct Node_801BFDC4 {
    void* value;
    char reservedBeforeLinks[0xC];
    void* next;
};

struct Iter_801BFDC4 {
    void* head;
    Node_801BFDC4* cur;
    char reservedBeforeLock[0x18];
    char lock[4];
};

extern "C" {

void OSLockMutex(void*);
void OSUnlockMutex(void*);

#define LockGuard_801BFDC4 WInstanceLock
struct LockGuard_801BFDC4 {
    void* p;
    // Acquire the recursive OS mutex for this scope.
    LockGuard_801BFDC4(void* q) : p(q) { OSLockMutex(q); }
    // Release the matching recursive acquisition.
    ~LockGuard_801BFDC4() { OSUnlockMutex(p); }
};

extern CSharedDoc* g_sharedDoc;

struct CourseRoundNameView { char prefix[0x3B7]; unsigned char variantIndex; };
struct CourseTriangleView {
    char prefix[0x38]; int firstCacheWord, secondCacheWord; char reserved[4]; int thirdCacheWord;
};
struct CoursePointerPairView { char prefix[0x20]; int first, second; };

// Borrow the course-selection fields of the shared document singleton.
inline CourseSelectionView* CourseSelection() { return (CourseSelectionView*)g_sharedDoc; }

// Store the raw +0x14 word without retaining/releasing anything; the caller passes 0x01000000.
void Object_SetWord14(CourseObjectView* self, void* value);

// Return the +0x108 word: course_model_scene_loader.cpp uses it to size the collision-triangle array.
// WPuppet_LoadModelFromArchive counts valid index triples into WPuppet::triangleCount
// at this same offset. The return signature retains the legacy ABI word view;
// the actual field is integer-valued.
void* WPuppet_GetTriangleCount(CourseObjectView* self);

// Return the root-bone address as a legacy integer ABI word; no reference is acquired.
unsigned int WPuppet_GetRootBone(CourseObjectView* self);

// Default option query: always false; the observed setup caller ignores the result.
int CourseObject_DefaultFalseQuery(CourseObjectView* self);

// Return CGolfDoc::isDaytime; zero selects the course night-name suffix.
unsigned char CGolfDoc_IsDaytime(CourseObjectView* self);

// Return the address of the embedded data block.
void* CourseObject_GetEmbeddedData(CourseObjectView* self);

// Clear the three cached words of the triangle record.
void CourseTriangle_Init(int* self);

// Return the selected course resource variant.
unsigned char CourseResource_GetSelectedVariant();

// Clear the two stored words of the resource pair.
void CourseResourcePair_Clear(int* self);

// Return the current object and advance the circular-list cursor under its mutex.
void* CourseModelObjectIterator_GetNext(Iter_801BFDC4* self);

// Borrow the cursor's value and advance once, mapping a wrap to head to end-of-iteration.
inline void* Next_801BFDC4(Iter_801BFDC4* self) {
    void* lk;
    void* v;
    v = 0;
    lk = self->lock;
    OSLockMutex(lk);
    Node_801BFDC4* n = self->cur;
    if (n) {
        void* next = n->next;
        v = n->value;
        self->cur = (Node_801BFDC4*)(next == self->head ? 0 : next);
        OSUnlockMutex(lk);
        return v;
    }
    OSUnlockMutex(lk);
    return 0;
}
// Rewind to the first active node; caller provides synchronization.
inline void Reset_801BFDC4(Iter_801BFDC4* self) {
    self->cur = (Node_801BFDC4*)self->head;
}

// Reset the cursor to the list head and return its first object under the recursive mutex.
void* CourseModelObjectIterator_GetFirst(Iter_801BFDC4* self);

struct String_801BFDC4 {
    unsigned int stringFlags;
    unsigned int length;
    // Decode inline length or the heap-string length without changing ownership.
    unsigned int size() const {
        unsigned int n;
        if ((stringFlags >> 31) == 0) n = *(const unsigned char*)this & 127;
        else n = length;
        return n;
    }
};
unsigned int strlen(const char*);
void String_ReplaceRange(String_801BFDC4*, unsigned int, unsigned int, const char*, const char*, void*);
char* String_FormatTemporary(const char*, ...);
extern char s_courseVariant1Suffix[8], s_courseVariant2Suffix[8], s_courseDefaultVariantSuffix[8], shadowResourceNameSuffix[8], s_courseRankResourceSuffix[8], s_courseRoundVariantSuffixFormat[8];
struct Alloc_801BFDC4 {
    volatile char c; char pad[3];
    // Leave the allocator token's byte untouched, matching the original copy sequence.
    Alloc_801BFDC4() {}
    // Copy the opaque allocator token through its required volatile byte read.
    Alloc_801BFDC4(const Alloc_801BFDC4& x): c(x.c) {}
};
// Append a borrowed C string; n receives the previous destination length.
inline void Append_801BFDC4(String_801BFDC4* self, const char* text, unsigned int& n) {
    n = self->size();
    Alloc_801BFDC4 b;
    Alloc_801BFDC4 a;
    b.c = a.c;
    unsigned int len = strlen(text);
    String_ReplaceRange(self, n, 0, text, text + len, &b);
}
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma use_lmw_stmw on
// Build the course base name with its variant, optional shadow suffix, and rank or round suffix.
void CourseResource_BuildVariantName(String_801BFDC4* self, int rank, int shadow);
#pragma use_lmw_stmw reset
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
struct VT_801BFDC4 { int a, b; void (*dtor)(void*, int); };
// The packet owns its copied 0x68-byte light description; no pointer ownership is transferred.
// Fixed-function light-description ABI (also produced by gtl_GenericLight_GetDescription).
// Color channels, position and direction are floats, but this copy deliberately uses
// their word bits to preserve volatile integer loads and the original store order.
union LightDescriptionComponent { int bits; float value; };
struct LightDescriptionStorageView {
    int type;
    LightDescriptionComponent diffuse[4], specular[4], ambient[4];
    LightDescriptionComponent position[3], direction[3];
    float range, falloff, constantAttenuation, linearAttenuation, quadraticAttenuation;
    float innerCone, outerCone;
};
typedef char LightDescriptionStorageSize[(sizeof(LightDescriptionStorageView) == 0x68) ? 1 : -1];
typedef char LightDescriptionDirectionOffset[((int)&((LightDescriptionStorageView*)0)->direction == 0x40) ? 1 : -1];
typedef char LightDescriptionRangeOffset[((int)&((LightDescriptionStorageView*)0)->range == 0x4C) ? 1 : -1];
typedef LightDescriptionStorageView Big_801C0080;
struct Dst_801C0080 { char reservedPrefix[0x4C]; Big_801C0080 b; };

// Copy the 19-word and seven-float property block while preserving the source read order.
void RenderPacket_CopyLightDescription(Dst_801C0080* self, volatile Big_801C0080* src);

struct V3_801C030C {
    float x, y, z;
    // Leave components uninitialized for callers that fill the vector explicitly.
    V3_801C030C() {}
    // Construct the three-component bounds value.
    V3_801C030C(float a, float b, float c) : x(a), y(b), z(c) {}
};

void __as__7WVectorFRC7WVector(V3_801C030C*, const V3_801C030C&);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void memset(void*, int, unsigned int);

struct Node_801C030C {
    void* value;             // 0x00
    char reservedBeforeLinks[8];           // 0x04
    Node_801C030C* volatile prev;     // 0x0c
    Node_801C030C* volatile next;     // 0x10
};

// Three lifetime roles: head/cur traverse the circular ACTIVE ring (no sentinel;
// wrap to head ends iteration), freeHead holds reusable nodes, allocationHead
// traverses owned allocations to free at final teardown. Do not infer that all
// three heads use the same link pair: only +0x0C/+0x10 are exposed in this view.
// DeleteValues frees active values; Clear recycles links and zeros the slot array;
// DestroyList frees allocations/array. OS mutexes are recursive: outer guards
// intentionally call helpers that acquire the same mutex again.
// Active payloads must be non-null: DeleteValues uses a null return as iteration end.
struct List_801C030C {
    Node_801C030C* head;     // 0x00
    Node_801C030C* cur;      // 0x04
    Node_801C030C* allocationHead;      // 0x08
    Node_801C030C* freeHead;      // 0x0c
    char reservedBeforeSlotCount[8];           // 0x10
    int count;               // 0x18
    void** array;            // 0x1c
    char lock[4];            // 0x20
};

struct Outer_801C030C {
    int objectCount;                 // 0x00
    List_801C030C list;      // 0x04
    char reservedBeforeBounds[0x3c - 0x28]; // 0x28
    V3_801C030C minimumBounds;         // 0x3c
    V3_801C030C maximumBounds;         // 0x48
    char cachedState[0x20];          // 0x54
};

// Return a borrowed active value and advance under the recursive list mutex.
inline void* Next_801C030C(List_801C030C* self) {
    void* lk = (*self).lock;
    OSLockMutex(lk);
    Node_801C030C* n = (*self).cur;
    if (n) {
        Node_801C030C* next = n->next;
        void* v = n->value;
        (*self).cur = (next == (*self).head) ? 0 : next;
        OSUnlockMutex(lk);
        return v;
    }
    OSUnlockMutex(lk);
    return 0;
}

// Detach n without freeing it; return its successor or null for a singleton ring.
inline Node_801C030C* Unlink_801C030C(void* lock, Node_801C030C* n) {
    Node_801C030C* nx;
    void* lk = lock;
    OSLockMutex(lk);
    n->next->prev = n->prev;
    n->prev->next = n->next;
    nx = n->next;
    if (nx == n) nx = 0;
    OSUnlockMutex(lk);
    return nx;
}
// Insert hh before pp, or make a singleton ring; return the resulting ring head.
inline Node_801C030C* Insert_801C030C(Node_801C030C* pp, void* lock, Node_801C030C* hh) {
    void* lk = lock;
    OSLockMutex(lk);
    if (!pp) {
        hh->prev = hh;
        hh->next = hh;
            OSUnlockMutex(lk);
            return hh;
    }
    hh->next = pp;
    hh->prev = pp->prev;
    pp->prev = hh;
    hh->prev->next = hh;
    OSUnlockMutex(lk);
    return pp;
}
// Free node allocations and the lookup array after values have been released.
inline void DestroyList_801C030C(List_801C030C* self) {
    if (self) {
        LockGuard_801BFDC4 g(self->lock);
        while (self->allocationHead) {
            Node_801C030C* n = self->allocationHead;
            Node_801C030C* nn = Unlink_801C030C(self->lock, n);
            self->allocationHead = nn;
            appFreeUni__FPv(n);
        }
        if (self->array) {
            appFreeUni__FPv(self->array);
            self->array = 0;
        }
    }
}

// Delete each active payload allocation; retain nodes for Clear/DestroyList.
inline void DeleteValues_801C030C(List_801C030C* self) {
        void* v;
        {
            LockGuard_801BFDC4 g((*self).lock);
            (*self).cur = (*self).head;
            v = Next_801C030C(&(*self));
        }
        for (; v != 0; v = Next_801C030C(&(*self))) {
            if (v) __dl__FPv(v);
        }
}
// Recycle the active ring into the free ring and clear every indexed object slot.
inline void Clear_801C030C(List_801C030C* self) {
        {
            LockGuard_801BFDC4 g((*self).lock);
            if ((*self).head && (*self).freeHead) {
                    // Splice whole rings when both exist; no per-node free is needed.
                    (*self).head->prev->next = (*self).freeHead->next;
                    (*self).freeHead->next->prev = (*self).head->prev;
                    (*self).freeHead->next = (*self).head;
                    (*self).head->prev = (*self).freeHead;
                    (*self).head = 0;
            } else {
                    while ((*self).head) {
                        Node_801C030C* n = (*self).head;
                        Node_801C030C* nn = Unlink_801C030C((*self).lock, n);
                        Node_801C030C* hh = (*self).head;
                        Node_801C030C* pp = (*self).freeHead;
                        (*self).freeHead = Insert_801C030C(pp, (*self).lock, hh);
                        (*self).head = nn;
                    }
            }
            int offset;
            // Indexed pointers no longer refer to live active values after recycling.
            int i = 0;
            offset = 0;
            while (i < (*self).count) {
                ++i;
                *(void**)((char*)(*self).array + offset) = 0;
                offset += 4;
            }
        }
}
extern float kFloat_neg6_3999996_CourseBounds, kFloat_6_3999996_CourseBounds;
// Reset bounds and cached state, delete list values, then release list storage and optionally the object.
void* CourseObjectList_Dtor(Outer_801C030C* self, int flag);

struct Refc_Count { int count; };
struct Refc_801BFDC4 : Refc_Count { 
virtual void dtor(int);
};

// Release one reference and invoke the deleting destructor when the count reaches zero.
int CourseObject_ReleaseReference(Refc_801BFDC4* self);

} // extern "C"
































#pragma pop

// Definitions follow the original text-address order.
extern "C" {

#pragma push
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma use_lmw_stmw on
// Build the selected course filename from base, variant and rank/round suffixes.
void CourseResource_BuildVariantName(String_801BFDC4* self, int rank, int shadow) {
    const char* text;
    const char* base = CourseSelection()->modelBaseName;
    unsigned int n;
    if ((self->stringFlags >> 31) == 0) n = *(const unsigned char*)self & 127;
    else n = self->length;
    Alloc_801BFDC4 a;
    Alloc_801BFDC4 b;
    b.c = a.c;
    unsigned int len = strlen(base);
    String_ReplaceRange(self, 0, n, base, base + len, &b);
    switch (CourseSelection()->courseVariant) {
    case 1: Append_801BFDC4(self, s_courseVariant1Suffix, n); break;
    case 2: Append_801BFDC4(self, s_courseVariant2Suffix, n); break;
    case 0:
    default: Append_801BFDC4(self, s_courseDefaultVariantSuffix, n); break;
    }
    if (shadow) Append_801BFDC4(self, shadowResourceNameSuffix, n);
    if (rank) Append_801BFDC4(self, s_courseRankResourceSuffix, n);
    else {
        text = String_FormatTemporary(s_courseRoundVariantSuffixFormat, ((CourseRoundNameView*)CourseSelection()->golfDoc)->variantIndex);
        Append_801BFDC4(self, text, n);
    }
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Copy one borrowed light description into the render packet without changing its bit patterns.
void RenderPacket_CopyLightDescription(Dst_801C0080* self, volatile Big_801C0080* src) {
    int directionZBits, directionYBits, directionXBits, positionZBits, positionYBits, positionXBits, ambientAlphaBits, ambientBlueBits, ambientGreenBits, ambientRedBits, specularAlphaBits, specularBlueBits, specularGreenBits, specularRedBits, diffuseAlphaBits, diffuseBlueBits, diffuseGreenBits, diffuseRedBits, typeBits;
    float outerCone, innerCone, quadraticAttenuation, linearAttenuation, constantAttenuation, falloff, range;
    // Phase 1: snapshot every volatile source component before touching the destination.
    typeBits = src->type;
    diffuseRedBits = src->diffuse[0].bits;
    diffuseGreenBits = src->diffuse[1].bits;
    diffuseBlueBits = src->diffuse[2].bits;
    diffuseAlphaBits = src->diffuse[3].bits;
    specularRedBits = src->specular[0].bits;
    specularGreenBits = src->specular[1].bits;
    specularBlueBits = src->specular[2].bits;
    specularAlphaBits = src->specular[3].bits;
    ambientRedBits = src->ambient[0].bits;
    ambientGreenBits = src->ambient[1].bits;
    ambientBlueBits = src->ambient[2].bits;
    ambientAlphaBits = src->ambient[3].bits;
    positionXBits = src->position[0].bits;
    positionYBits = src->position[1].bits;
    positionZBits = src->position[2].bits;
    directionXBits = src->direction[0].bits;
    directionYBits = src->direction[1].bits;
    directionZBits = src->direction[2].bits;
    range = src->range;
    falloff = src->falloff;
    constantAttenuation = src->constantAttenuation;
    linearAttenuation = src->linearAttenuation;
    quadraticAttenuation = src->quadraticAttenuation;
    innerCone = src->innerCone;
    outerCone = src->outerCone;
    // Phase 2: retain the original nonsequential first store (ambient blue).
    self->b.ambient[2].bits = ambientBlueBits;
    self->b.type = typeBits;
    self->b.diffuse[0].bits = diffuseRedBits;
    self->b.diffuse[1].bits = diffuseGreenBits;
    self->b.diffuse[2].bits = diffuseBlueBits;
    self->b.diffuse[3].bits = diffuseAlphaBits;
    self->b.specular[0].bits = specularRedBits;
    self->b.specular[1].bits = specularGreenBits;
    self->b.specular[2].bits = specularBlueBits;
    self->b.specular[3].bits = specularAlphaBits;
    self->b.ambient[0].bits = ambientRedBits;
    self->b.ambient[1].bits = ambientGreenBits;
    self->b.ambient[3].bits = ambientAlphaBits;
    self->b.position[0].bits = positionXBits;
    self->b.position[1].bits = positionYBits;
    self->b.position[2].bits = positionZBits;
    self->b.direction[0].bits = directionXBits;
    self->b.direction[1].bits = directionYBits;
    self->b.direction[2].bits = directionZBits;
    self->b.range = range;
    self->b.falloff = falloff;
    self->b.constantAttenuation = constantAttenuation;
    self->b.linearAttenuation = linearAttenuation;
    self->b.quadraticAttenuation = quadraticAttenuation;
    self->b.innerCone = innerCone;
    self->b.outerCone = outerCone;
}
#pragma pop

#pragma push
// Set the puppet word used by fallback mesh setup (observed value 0x01000000).
void Object_SetWord14(CourseObjectView* self, void* value) {
    self->rawWord14 = (unsigned int)value;
}
#pragma pop

#pragma push
// Return the integer triangle count as the legacy ABI word used to size collision storage.
void* WPuppet_GetTriangleCount(CourseObjectView* self) {
    return (void*)self->triangleCount;
}
#pragma pop

#pragma push
// Clear the triangle record caches before collision data is populated.
void CourseTriangle_Init(int* self) {
    ((CourseTriangleView*)self)->firstCacheWord = 0;
    ((CourseTriangleView*)self)->secondCacheWord = 0;
    ((CourseTriangleView*)self)->thirdCacheWord = 0;
}
#pragma pop

#pragma push
// Borrow the puppet root bone whose world matrix transforms fallback collision triangles.
unsigned int WPuppet_GetRootBone(CourseObjectView* self) {
    return self->metadataWord;
}
#pragma pop

#pragma push
// Return the option query default false; the observed caller discards this result.
int CourseObject_DefaultFalseQuery(CourseObjectView* self) {
    return 0;
}
#pragma pop

#pragma push
// Return the selected course resource variant byte.
unsigned char CourseResource_GetSelectedVariant() {
    return CourseSelection()->courseVariant;
}
#pragma pop

#pragma push
// Return the golf-document daytime flag; zero selects the night resource suffix.
unsigned char CGolfDoc_IsDaytime(CourseObjectView* self) {
    return self->statusByte;
}
#pragma pop

#pragma push
// Borrow the embedded block at +0x124; its consumer meaning is unresolved here.
void* CourseObject_GetEmbeddedData(CourseObjectView* self) {
    return self->embeddedData;
}
#pragma pop

#pragma push
// Reset and borrow the first active object under the recursive mutex.
void* CourseModelObjectIterator_GetFirst(Iter_801BFDC4* self) {
    LockGuard_801BFDC4 outer(self->lock);
    Reset_801BFDC4(self);
    return Next_801BFDC4(self);
}
#pragma pop

#pragma push
// Borrow the next active object, returning null on wrap to the ring head.
void* CourseModelObjectIterator_GetNext(Iter_801BFDC4* self) {
    LockGuard_801BFDC4 g(self->lock);
    Node_801BFDC4* n = self->cur;
    if (n) {
        void* v = n->value;
        self->cur = (Node_801BFDC4*)(n->next == self->head ? 0 : n->next);
        return v;
    }
    return 0;
}
#pragma pop

#pragma push
// Clear both cached words of the resource pair.
void CourseResourcePair_Clear(int* self) {
    ((CoursePointerPairView*)self)->first = 0;
    ((CoursePointerPairView*)self)->second = 0;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Release active values and owned list storage, then optionally delete the owner.
void* CourseObjectList_Dtor(Outer_801C030C* self, int flag) {
    if (self) {
        self->objectCount = 0;
        __as__7WVectorFRC7WVector(&self->minimumBounds, V3_801C030C(kFloat_neg6_3999996_CourseBounds, kFloat_neg6_3999996_CourseBounds, kFloat_neg6_3999996_CourseBounds));
        __as__7WVectorFRC7WVector(&self->maximumBounds, V3_801C030C(kFloat_6_3999996_CourseBounds, kFloat_6_3999996_CourseBounds, kFloat_6_3999996_CourseBounds));
        memset(self->cachedState, 0, 0x20);
        DeleteValues_801C030C(&self->list);
        Clear_801C030C(&self->list);
        DestroyList_801C030C(&self->list);
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma pop

#pragma push
#pragma opt_lifetimes reset
#pragma opt_propagation reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset
// Release one reference and delete through the virtual destructor at zero.
int CourseObject_ReleaseReference(Refc_801BFDC4* self) {
    int c = --self->count;
    if (c == 0) {
        if (self) {
            self->dtor(1);
        }
        return 0;
    }
    return c;
}
#pragma pop

}
