// Effect box registries: the sequences and sprays an effect box owns, and removing, clearing and
// registering them. Also the cache of animated sprite grids by bitmap name and the string keys
// those registry trees use.

#include <decomp/local_class.h>
#include "game/wvector.h"
#include "classes/CFxSpray.h"
#include "classes/CFxBox.h"
#include "classes/CFxSequence.h"
#include "classes/CFxRegistryTree.h"
// Borrowed layout views. Forced-inline replacements preserve instructions but
// change anonymous exception/constant relocation symbols in this compiler.
#define AsCFxBox(p) ((CFxBox*)(p))

// Registry keys are effect addresses; parent links reserve bit zero for colour.
static const unsigned int kTreeParentAddressMask = ~1u;

// Narrow strings keep their length in the low byte while bit 31 is clear, and
// in the second word otherwise; inline text starts at +1.
enum {
    kSmallStringLengthMask = 0x7F,
    kHeapStringTagShift = 31
};

// The box's virtual extension hooks. The base implementations do nothing; the
// scene effect box overrides them to keep its attachment caches in step.
struct FxBoxHooks {
    virtual ~FxBoxHooks() DECOMP_LOCAL_VIRTUAL;
    virtual void Slot1() DECOMP_LOCAL_VIRTUAL; // the first virtual function after the destructor
    virtual void Slot2() DECOMP_LOCAL_VIRTUAL; // TestHeightCrossing
    virtual void SequenceRegistered(void* sequence) DECOMP_LOCAL_VIRTUAL; // +0x14
    virtual void SequenceRemoved(void* sequence) DECOMP_LOCAL_VIRTUAL;    // +0x18
    virtual void SprayRegistered(void* spray) DECOMP_LOCAL_VIRTUAL;       // +0x1C
    virtual void SprayRemoved(void* spray) DECOMP_LOCAL_VIRTUAL;          // +0x20
};
// An effect, of which only the deleting destructor is called through its vtable.
struct FxEffect {
    virtual ~FxEffect() DECOMP_LOCAL_VIRTUAL;
};
enum SequenceChildPolicy {
    kDetachChildSprays = 0,
    kDestroyChildSprays = 1
};
// The prefix of a registry node that holds its key.
struct EffectRegistryNodePrefix {
    void* left;
    void* right;
    void* parentTag;
    unsigned int key;
};
struct SpriteResourceView {
    char pad_00[0x10];
    void* resourceManager;
};
// The parts of a spray that the CFxSpray header does not place.
struct DetachedSprayView {
    char pad_00[8];
    void* attachment;
    char pad_0C[0xAC];
    WVector worldPosition;
};

extern "C" {

// Default effect-box extension hook; no additional work is required.
void CFxBox::vfunc_6() {}
// Default effect-box extension hook; no additional work is required.
void CFxBox::vfunc_4() {}
// Default effect-box extension hook; no additional work is required.
void CFxBox::vfunc_3() {}

// Default effect-box extension hook; no additional work is required.
void CFxBox::vfunc_5() {}

extern unsigned int strlen(const char*);
extern double atof(const char*);
extern float kFloat_0_FxBox;
extern float kDegreesToRadians_FxValueParser;
struct EffectParameterScale {
    char pad_00[4];
    float scale;
};
// Parse a numeric effect parameter; d converts degrees and f applies the configured scale.
float CFxSpray::ParseScaledFloat(const char* value) {
    EffectParameterScale* self = (EffectParameterScale*)this;
    unsigned int len = strlen(value);
    if (len == 0)
        return kFloat_0_FxBox;
    char suffix = value[len - 1];
    if (suffix == 'd')
        return kDegreesToRadians_FxValueParser * (float)atof(value);
    if (suffix == 'f')
        return self->scale * (float)atof(value);
    return (float)atof(value);
}

extern void appFreeUni__FPv(void*);
extern void RBTree_RebalanceAfterErase(void*, void*);
extern unsigned int s_fxRegistryMapDefaultValue;
extern FxRegistryNode* FxBoxTree_InsertNode(CFxRegistryTree*, FxRegistryNode**, unsigned char, unsigned char, void*);

// Find a sequence by its address and erase it with the requested child policy.
void CFxBox::RemoveSequence(CFxSequence* sequence, int mode) {
    FxRegistryNode* node = (FxRegistryNode*)sequenceTreeRoot;
    FxRegistryNode* where = (FxRegistryNode*)&sequenceTreeRoot;
    while (node != 0) {
        if (node->key >= (unsigned int)sequence) {
            where = node;
            node = node->left;
        } else
            node = node->right;
    }
    if (where == (FxRegistryNode*)&sequenceTreeRoot || (unsigned int)sequence < where->key)
        where = (FxRegistryNode*)&sequenceTreeRoot;
    FxRegistryNode* iter = where;
    if (where != (FxRegistryNode*)&sequenceTreeRoot)
        EraseSequenceAtIterator(&iter, mode);
}

// Views used only by the erase below, which keeps the box's registry offsets
// spelled out. The box's spray registry sits at 0x154 and is seen as CFxBoxView.
struct FxRegistryNodeView {
    FxRegistryNodeView* left;
    FxRegistryNodeView* right;
    FxRegistryNodeView* parentTag;
    unsigned int key;
    void* value;
};
struct CFxBoxView {
    char pad_00[0x154];
    unsigned int size;
    FxRegistryNodeView* root;
    FxRegistryNodeView* firstNode;
    unsigned int count0;
    unsigned int count1;
    unsigned int count2;
    void* object;
};
struct Interface_8010A28C {
    virtual ~Interface_8010A28C();
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void removeGroup(void*);
    virtual void insert(void*);
    virtual void erase(void*);
};
// Advance a tagged-parent tree iterator to its in-order successor.
static inline void advance_8010A28C(FxRegistryNodeView*& node) {
    FxRegistryNodeView* next = node->right;
    if (next != 0) {
        while (next->left != 0)
            next = next->left;
        node = next;
    } else {
        while (node != (next = (FxRegistryNodeView*)((unsigned int)node->parentTag & kTreeParentAddressMask))->left) {
            node = next;
        }
        node = next;
    }
}
struct FxRegistryIterator {
    FxRegistryNodeView* node;
    // Borrow a registry node as an iterator; no ownership changes.
    FxRegistryIterator(FxRegistryNodeView* p) : node(p) {}
    // Copy the borrowed node cursor.
    FxRegistryIterator(const FxRegistryIterator& p) : node(p.node) {}
    // Advance to the successor and return this cursor.
    FxRegistryIterator& operator++() {
        advance_8010A28C(node);
        return *this;
    }
    // Advance to the successor, returning the previous cursor.
    FxRegistryIterator operator++(int) {
        FxRegistryIterator old = *this;
        ++*this;
        return old;
    }
};
#pragma push
#pragma use_lmw_stmw on
// Erase a sequence, deleting or detaching its sprays according to the caller's policy.
void CFxBox::EraseSequenceAtIterator(FxRegistryNode** iter, int mode) {
    // *iter is a live sequence node and advances before removal. mode == 0 detaches
    // the sprays into world space; any other mode removes them. The sequence itself
    // is deleted only when its own flag says so.
    enum {
        kBoxOffset_sprayCount = (int)&((CFxBox*)0)->sprayCount
    };
    enum {
        kBoxOffset_sequenceCount = (int)&((CFxBox*)0)->sequenceCount
    };
    enum {
        kBoxOffset_sequenceTreeRoot = (int)&((CFxBox*)0)->sequenceTreeRoot
    };
    enum {
        kBoxOffset_sequenceTreeFirst = (int)&((CFxBox*)0)->sequenceTreeFirst
    };
    enum {
        kSequenceOffset_firstSprayNode = (int)&((CFxSequence*)0)->firstSprayNode
    };
    enum {
        kSequenceOffset_sprayTreeRoot = (int)&((CFxSequence*)0)->sprayTreeRoot
    };
    enum {
        kSequenceOffset_sprayCount = (int)&((CFxSequence*)0)->sprayCount
    };
    enum {
        kSequenceOffset_worldPosition = (int)&((CFxSequence*)0)->worldPosition
    };
    // Phase 1: unlink/free the box's sequence node; keep its object for child cleanup.
    char* sequenceBytes = (char*)(*iter)->value;
    char* boxBytes = (char*)this;
    CFxBoxView* self = (CFxBoxView*)this;
    FxRegistryIterator removedSequenceIterator = (*(FxRegistryIterator*)iter)++;
    FxRegistryIterator erasedIterator = removedSequenceIterator;
    if (removedSequenceIterator.node == *(FxRegistryNodeView**)(boxBytes + kBoxOffset_sequenceTreeFirst)) {
        removedSequenceIterator++;
        *(FxRegistryIterator*)(boxBytes + kBoxOffset_sequenceTreeFirst) = removedSequenceIterator;
    }
    RBTree_RebalanceAfterErase(erasedIterator.node, *(void**)(boxBytes + kBoxOffset_sequenceTreeRoot));
    appFreeUni__FPv(erasedIterator.node);
    --*(unsigned int*)(boxBytes + kBoxOffset_sequenceCount);
    FxRegistryIterator sprayIterator = *(FxRegistryIterator*)(sequenceBytes + kSequenceOffset_firstSprayNode);
    FxRegistryNodeView* end = (FxRegistryNodeView*)(sequenceBytes + kSequenceOffset_sprayTreeRoot);
    // Phase 2: discard null nodes, destroy selected children, or detach survivors.
    while (end != sprayIterator.node) {
        char* sprayBytes = (char*)sprayIterator.node->value;
        if (sprayBytes == 0) {
            // The node holding the null entry is erased, not its successor.
            FxRegistryIterator item = sprayIterator++;
            erasedIterator = item;
            if (item.node == *(FxRegistryNodeView**)(sequenceBytes + kSequenceOffset_firstSprayNode)) {
                ++item;
                *(FxRegistryIterator*)(sequenceBytes + kSequenceOffset_firstSprayNode) = item;
            }
            RBTree_RebalanceAfterErase(erasedIterator.node, *(void**)(sequenceBytes + kSequenceOffset_sprayTreeRoot));
            appFreeUni__FPv(erasedIterator.node);
            --*(unsigned int*)(sequenceBytes + kSequenceOffset_sprayCount);
        } else if (0 != sprayBytes && mode != 0) {
            ++sprayIterator;
            FxRegistryNodeView* where = (FxRegistryNodeView*)&self->root;
            FxRegistryNodeView* search = self->root;
            while (search != 0) {
                if (search->key >= (unsigned int)sprayBytes) {
                    where = search;
                    search = search->left;
                } else
                    search = search->right;
            }
            if (where == (FxRegistryNodeView*)&self->root || (unsigned int)sprayBytes < where->key)
                where = (FxRegistryNodeView*)&self->root;
            FxRegistryNodeView* unused = where;
            if (where == (FxRegistryNodeView*)&self->root)
                continue;
            FxRegistryNodeView* unusedNext = unused->right;
            sprayBytes = (char*)where->value;
            if (0 != unusedNext) {
                do {
                    unusedNext = unusedNext->left;
                } while (unusedNext);
            } else {
                while (unused !=
                       (unusedNext = (FxRegistryNodeView*)((unsigned int)unused->parentTag & kTreeParentAddressMask))
                           ->left)
                    unused = unusedNext;
            }
            FxRegistryNodeView* erased = where;
            if (where == self->firstNode) {
                advance_8010A28C(where);
                self->firstNode = where;
            }
            RBTree_RebalanceAfterErase(erased, self->root);
            appFreeUni__FPv(erased);
            self->size -= 1;
            ((Interface_8010A28C*)self)->erase(sprayBytes);
            if (*(void**)&((CFxSpray*)sprayBytes)->parentSequence != 0)
                ((CFxSequence*)(*(void**)&((CFxSpray*)sprayBytes)->parentSequence))
                    ->UnregisterSpray((unsigned int)sprayBytes);
            delete (Interface_8010A28C*)sprayBytes;
        } else {
            ((DetachedSprayView*)sprayBytes)->worldPosition +=
                *(WVector*)(sequenceBytes + kSequenceOffset_worldPosition);
            *(void**)&((CFxSpray*)sprayBytes)->parentSequence = 0;
            if (((CFxSpray*)sprayBytes)->particlesFollowEmitter != 0) {
                if (*(void**)&((CFxSequence*)sequenceBytes)->attachment != 0) {
                    unsigned char descendLeft;
                    ((DetachedSprayView*)sprayBytes)->attachment = *(void**)&((CFxSequence*)sequenceBytes)->attachment;
                    unsigned int key = (unsigned int)sprayBytes;
                    FxRegistryNode* parent;

                    unsigned int* slot;
                    unsigned char noPredecessor;
                    FxRegistryNode** link = ((CFxRegistryTree*)(boxBytes + kBoxOffset_sprayCount))
                                                ->FindInsertionPosition(&key, &parent, &descendLeft, &noPredecessor);
                    // Insert unless the greatest key <= query is an exact match.
                    if (0 == parent || (slot = &parent->key, *slot < key)) {
                        struct Pair {
                            unsigned int key;
                            int global;
                        } pair = {key, s_fxRegistryMapDefaultValue};
                        slot = &FxBoxTree_InsertNode((CFxRegistryTree*)(boxBytes + kBoxOffset_sprayCount), link,
                                                     descendLeft, noPredecessor, &pair)
                                    ->key;
                    }
                    slot[1] = key;
                    ((Interface_8010A28C*)self)->insert((void*)key);
                }
            }
            sprayIterator++;
        }
    }
    // Phase 3: notify the box and honor the sequence's own deletion policy.
    ((Interface_8010A28C*)self)->removeGroup(sequenceBytes);
    if (*(unsigned char*)&((CFxSequence*)sequenceBytes)->deleteWhenRemovedFromBox != 0)
        delete (Interface_8010A28C*)sequenceBytes;
}
#pragma pop

extern void* __nw__FUl(unsigned int);
extern void sprintf(char*, const char*, ...);
extern void String_ReserveCapacity(void*, unsigned int);
extern void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
extern int memcmp(const void*, const void*, unsigned int);
extern void W3dAniSpr_Ctor(void*);
extern void W3dAniSpr_LoadTexture(void*, void*, float, float);
extern void* ResourceManager_LoadBitmap(void*, const char*, int, int, int);
extern void* AnimatedSpriteCache_InsertNode(void*, void*, unsigned char, unsigned char, void**);
extern void appFreeUni__FPv(void*);
extern char hudBitmapVariantStrings[];
struct SpriteCacheString {
    unsigned int storageHeader;
    unsigned int lengthOrInlineBytes;
    char* data;
};
struct SpriteCacheEntry {
    SpriteCacheString str;
    void* extra0;
    void* extra1;
};
struct Alloc_AB20 {
    char c __attribute__((aligned(4)));
};
struct SpriteCacheStringView;
struct SpriteCacheNodeView;
extern SpriteCacheNodeView* FxAnimatedSpriteCache_FindInsertionPosition(SpriteCacheNodeView*, SpriteCacheStringView*,
                                                                        SpriteCacheNodeView**, unsigned char*,
                                                                        unsigned char*);
struct AnimatedSpriteStorage {
    char storage[0x19c];
    // Construct the animated-sprite base in this newly allocated wrapper.
    AnimatedSpriteStorage() {
        W3dAniSpr_Ctor(this);
    }
};
struct SpriteCacheNode {
    SpriteCacheNode* left;
    SpriteCacheNode* right;
    char pad_08[4];
    SpriteCacheString key;
    void* made;
    void* obj;
};
struct SpriteCacheOwner {
    char pad_00[0x140];
    void* fallback;
    char pad_144[4];
    void* root;
};
struct StringNode_AB20 {
    StringNode_AB20* left;
    StringNode_AB20* right;
    char pad_08[4];
    unsigned char flag;
};
struct StringTree_AB20 {
    void* header;
    StringNode_AB20* root;
};

// Read a narrow small-string length from its inline or heap representation.
static inline unsigned int sslen_AB20(const SpriteCacheString* s) {
    if ((s->storageHeader >> kHeapStringTagShift) == 0)
        return *(const unsigned char*)s & kSmallStringLengthMask;
    return s->lengthOrInlineBytes;
}
// Return inline or heap text without allocating.
static inline const char* ssdata_AB20(const SpriteCacheString* s) {
    if ((s->storageHeader >> kHeapStringTagShift) == 0)
        return (const char*)s + 1;
    return s->data;
}
// Return the smaller string-comparison bound by reference.
static inline const unsigned int& min_AB20(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Compare bounded byte strings, breaking a matching prefix by length.
static inline int cmpstr_AB20(const char* ad, const char* bd, unsigned int alen, unsigned int alen2,
                              unsigned int blen) {
    unsigned int t = min_AB20(alen, alen2);
    int c = memcmp(ad, bd, min_AB20(blen, t));
    if (c == 0) {
        if (t < blen)
            c = -1;
        else if (t == blen)
            c = 0;
        else
            c = 1;
    }
    return c;
}
// Compare insertion keys with the original nested length bounds.
static inline int cmplt_AB20(const char* ad, const char* bd, unsigned int alen, unsigned int alen2, unsigned int blen) {
    unsigned int t = min_AB20(alen, alen2);
    unsigned int n = min_AB20(blen, t);
    int c = memcmp(ad, bd, n);
    if (c == 0) {
        if (n < blen)
            c = -1;
        else if (n == blen)
            c = 0;
        else
            c = 1;
    }
    return c;
}
// Decode a node key for bounded comparison without treating inline words as a heap pointer.
static inline void ReadSpriteComparisonSpan(SpriteCacheString* key, unsigned int& firstLength,
                                            unsigned int& secondLength, const char*& characters) {
    if ((key->storageHeader >> kHeapStringTagShift) == 0)
        firstLength = *(unsigned char*)&key->storageHeader & kSmallStringLengthMask;
    else
        firstLength = key->lengthOrInlineBytes;
    if ((key->storageHeader >> kHeapStringTagShift) == 0) {
        characters = (char*)key + 1;
        secondLength = *(unsigned char*)&key->storageHeader & kSmallStringLengthMask;
    } else {
        characters = key->data;
        secondLength = key->lengthOrInlineBytes;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Cache a bitmap grid by name and dimensions, loading and creating it on a miss.
void* CFxBox::GetOrLoadAnimatedSprite(const char* bitmapName, unsigned int cellWidth, unsigned int cellHeight) {
    SpriteCacheOwner* self = (SpriteCacheOwner*)this;
    // Dimensions are grid-cell pixels, not a frame count. Return a cached sprite
    // pointer (borrowed from this box), or null if manager/bitmap lookup fails.
    // The formatted name+dimensions key must fit buffer; formatting is unbounded.
    char spriteCacheKeyText[0x114] __attribute__((aligned(4)));
    SpriteCacheEntry bound;
    SpriteCacheString key;
    SpriteCacheString name;
    Alloc_AB20 keySource;
    Alloc_AB20 keyTerm;
    Alloc_AB20 nameSource;
    Alloc_AB20 nameTerm;
    Alloc_AB20 boundSource;
    Alloc_AB20 boundTerm;
    unsigned char insert;
    unsigned char found;
    SpriteCacheNodeView* insertionPredecessor;
    unsigned int len;
    SpriteCacheNode* node;
    void* bitmap;
    void* sprite;
    char* cacheEntryBytes;

    // Phase 1: format the composite key and search its lexicographic lower bound.
    sprintf(spriteCacheKeyText, hudBitmapVariantStrings, bitmapName, cellWidth, cellHeight);
    key.storageHeader = 0;
    key.lengthOrInlineBytes = 0;
    key.data = 0;
    len = strlen(spriteCacheKeyText);
    String_ReserveCapacity(&key, len);
    keyTerm.c = *(volatile char*)&keySource.c;
    String_ReplaceRange(&key, 0, 0, spriteCacheKeyText, spriteCacheKeyText + len, &keyTerm.c);

    unsigned int keyLong = key.storageHeader >> kHeapStringTagShift;
    SpriteCacheNode* parent = (SpriteCacheNode*)(&AsCFxBox(self)->spriteCacheRoot);
    node = (SpriteCacheNode*)self->root;
    int keyIsLong;
    {
        bool keyIsZero = (keyLong == 0);
        keyIsLong = !keyIsZero;
    }
    char* keyData = key.data;
    unsigned int keyHeapLength = key.lengthOrInlineBytes;
    unsigned int keySmallLen = *(unsigned char*)&key.storageHeader & kSmallStringLengthMask;

    unsigned int keyLen;
    const char* keyPtr;
    unsigned int nodeLen1;
    unsigned int nodeLen2;
    const char* nodePtr;
    while (node != 0) {
        if (!keyIsLong) {
            keyLen = keySmallLen;
            keyPtr = (char*)&key + 1;
        } else {
            keyPtr = keyData;
            keyLen = keyHeapLength;
        }
        ReadSpriteComparisonSpan(&node->key, nodeLen1, nodeLen2, nodePtr);
        // Compare bounded byte strings, breaking a matching prefix by length.
        if (cmpstr_AB20(nodePtr, keyPtr, nodeLen1, nodeLen2, keyLen) >= 0) {
            parent = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if ((char*)parent != (char*)&AsCFxBox(self)->spriteCacheRoot) {
        unsigned int parentLen;
        const char* parentPtr;
        if ((parent->key.storageHeader >> kHeapStringTagShift) == 0) {
            parentLen = *(unsigned char*)&parent->key.storageHeader & kSmallStringLengthMask;
            parentPtr = (char*)&parent->key + 1;
        } else {
            parentPtr = parent->key.data;
            parentLen = parent->key.lengthOrInlineBytes;
        }
        parent = (cmplt_AB20(!keyIsLong ? (const char*)&key + 1 : key.data, parentPtr,
                             !keyIsLong ? keySmallLen : key.lengthOrInlineBytes,
                             !keyIsLong ? keySmallLen : key.lengthOrInlineBytes, parentLen) >= 0)
                     ? parent
                     : (SpriteCacheNode*)(&AsCFxBox(self)->spriteCacheRoot);
    }
    if (keyLong != 0)
        appFreeUni__FPv(key.data);
    if ((char*)parent != (char*)&AsCFxBox(self)->spriteCacheRoot)
        return parent->made;
    if (self->fallback == 0)
        return 0;
    // Phase 2: load the bitmap by its original name and construct grid frames.
    bitmap = ResourceManager_LoadBitmap(self->fallback, bitmapName, 0, 0, 0);
    if (bitmap == 0)
        return 0;
    sprite = new AnimatedSpriteStorage();
    ((SpriteResourceView*)sprite)->resourceManager = self->fallback;
    W3dAniSpr_LoadTexture(sprite, bitmap, (float)(int)cellWidth, (float)(int)cellHeight);
    // Phase 3: retain the composite key and publish sprite/bitmap in the cache.
    name.storageHeader = 0;
    name.lengthOrInlineBytes = 0;
    name.data = 0;
    len = strlen(spriteCacheKeyText);
    String_ReserveCapacity(&name, len);
    nameTerm.c = *(volatile char*)&nameSource.c;
    String_ReplaceRange(&name, 0, 0, spriteCacheKeyText, spriteCacheKeyText + len, &nameTerm.c);
    try {
        SpriteCacheNodeView* result = FxAnimatedSpriteCache_FindInsertionPosition(
            (SpriteCacheNodeView*)(&AsCFxBox(self)->spriteCacheCount), (SpriteCacheStringView*)&name,
            &insertionPredecessor, &found, &insert);
        // 0x8010AB20: Keep an existing map slot or insert a new sprite-name entry.
        do {
            if (insertionPredecessor != 0) {
                SpriteCacheString* stored;
                unsigned int nameLen;
                const char* namePtr;
                cacheEntryBytes = (char*)&((EffectRegistryNodePrefix*)insertionPredecessor)->key;
                stored = (SpriteCacheString*)cacheEntryBytes;
                if ((name.storageHeader >> kHeapStringTagShift) == 0) {
                    nameLen = *(unsigned char*)&name.storageHeader & kSmallStringLengthMask;
                    namePtr = (char*)&name + 1;
                } else {
                    namePtr = name.data;
                    nameLen = name.lengthOrInlineBytes;
                }
                unsigned int storedLen1;
                unsigned int storedLen2;
                const char* storedPtr;
                ReadSpriteComparisonSpan(stored, storedLen1, storedLen2, storedPtr);
                // Compare bounded byte strings, breaking a matching prefix by length.
                if (cmpstr_AB20(storedPtr, namePtr, storedLen1, storedLen2, nameLen) >= 0) {
                    break;
                }
            }
            if ((name.storageHeader >> kHeapStringTagShift) == 0) {
                bound.str = name;
            } else {
                bound.str.storageHeader = 0;
                bound.str.lengthOrInlineBytes = 0;
                bound.str.data = 0;
                String_ReserveCapacity(&bound.str, name.lengthOrInlineBytes);
                boundTerm.c = *(volatile char*)&boundSource.c;
                String_ReplaceRange(&bound.str, 0, 0, name.data, name.data + name.lengthOrInlineBytes, &boundTerm.c);
            }
            {
                bound.extra0 = 0;
                bound.extra1 = 0;
                void* ins;
                try {
                    ins = AnimatedSpriteCache_InsertNode(&AsCFxBox(self)->spriteCacheCount, result, found, insert,
                                                         (void**)&bound.str);
                } catch (...) {
                    if (bound.str.storageHeader >> kHeapStringTagShift)
                        appFreeUni__FPv(bound.str.data);
                    throw;
                }
                cacheEntryBytes = (char*)&((EffectRegistryNodePrefix*)ins)->key;
            }
            if ((bound.str.storageHeader >> kHeapStringTagShift) != 0)
                appFreeUni__FPv(bound.str.data);
        } while (0);

        ((SpriteCacheEntry*)cacheEntryBytes)->extra0 = sprite;
        ((SpriteCacheEntry*)cacheEntryBytes)->extra1 = bitmap;
        if ((name.storageHeader >> kHeapStringTagShift) != 0)
            appFreeUni__FPv(name.data);
        return sprite;
    } catch (...) {
        if (name.storageHeader >> kHeapStringTagShift)
            appFreeUni__FPv(name.data);
        throw;
    }
}
#pragma pop
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

struct SpriteCacheStringView {
    unsigned char flag;
};
struct SpriteCacheNodeView {
    SpriteCacheNodeView* left;
    SpriteCacheNodeView* right;
    char pad_08[4];
    unsigned char flag;
    unsigned char inlineKey[0];
    unsigned int heapKeyLength; // 0x10: used when the flag word has its high bit set.
    void* heapKeyData;          // 0x14
};
extern int memcmp(const void*, const void*, unsigned int);
// Return the smaller insertion-key comparison bound.
static inline const unsigned int& min_8010B088(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}
// Decode the query string's inline/heap data pointer and length.
static inline const void* data_8010B088(SpriteCacheStringView* query, unsigned int& len) {
    unsigned int n;
    const void* p;
    if ((*(unsigned int*)query >> kHeapStringTagShift) == 0) {
        p = &query->flag + 1;
        n = query->flag & kSmallStringLengthMask;
    } else {
        p = *(void**)((char*)query + 8);
        n = *(unsigned int*)((char*)query + 4);
    }
    len = n;
    return p;
}
#pragma push
#pragma use_lmw_stmw on
// Find the tree link and predecessor flags for an animated-sprite name.
SpriteCacheNodeView* FxAnimatedSpriteCache_FindInsertionPosition(SpriteCacheNodeView* tree,
                                                                 SpriteCacheStringView* query,
                                                                 SpriteCacheNodeView** outNode, unsigned char* found,
                                                                 unsigned char* end) {
    *outNode = 0;
    unsigned int nodeKey;
    SpriteCacheNodeView* parent = (SpriteCacheNodeView*)&tree->right;
    SpriteCacheNodeView* node = tree->right;
    *found = 1;
    *end = 1;
    while (node != 0) {
        parent = node;
        const void* nodeData;
        unsigned int queryKey;
        if ((*(unsigned int*)&node->flag >> kHeapStringTagShift) == 0) {
            nodeData = &node->flag + 1;
            nodeKey = node->flag & kSmallStringLengthMask;
        } else {
            nodeData = node->heapKeyData;
            nodeKey = node->heapKeyLength;
        }
        unsigned int nodeSize = nodeKey;
        if ((*(unsigned int*)query >> kHeapStringTagShift) == 0)
            queryKey = query->flag & kSmallStringLengthMask;
        else
            queryKey = *(unsigned int*)((char*)query + 4);
        unsigned int qend;
        const void* queryData = data_8010B088(query, qend);
        unsigned int lo = min_8010B088(queryKey, qend);
        int cmp = memcmp(queryData, nodeData, min_8010B088(nodeSize, lo));
        if (cmp == 0) {
            if (lo < nodeKey)
                cmp = -1;
            else if (lo != nodeKey)
                cmp = 1;
        }
        if (cmp < 0) {
            node = node->left;
            *found = 1;
        } else {
            *outNode = parent;
            node = node->right;
            *found = 0;
            *end = 0;
        }
    }
    return parent;
}
#pragma pop

// Erase a registered spray, notify the box and parent sequence, then destroy it.
void CFxBox::RemoveSpray(CFxSpray* spray) {
    void* data;
    FxRegistryNode* result = (FxRegistryNode*)&sprayTreeRoot;
    FxRegistryNode* node = (FxRegistryNode*)sprayTreeRoot;
    FxRegistryNode* value;
    while (node != 0) {
        if (node->key >= (unsigned int)spray) {
            result = node;
            node = node->left;
        } else
            node = node->right;
    }
    if (result != (FxRegistryNode*)&sprayTreeRoot && (unsigned int)spray < result->key)
        result = (FxRegistryNode*)&sprayTreeRoot;
    if (result == (FxRegistryNode*)&sprayTreeRoot)
        return;
    data = result->value;
    value = result->right;
    if (value != 0) {
        while (value->left != 0)
            value = value->left;
    } else {
        value = result;
        for (;;) {
            FxRegistryNode* parent = (FxRegistryNode*)((unsigned int)value->parentTag & kTreeParentAddressMask);
            if (value != parent->left) {
                value = parent;
                continue;
            }
            value = parent;
            break;
        }
    }
    if (result == (FxRegistryNode*)sprayTreeFirst) {
        if (result->right != 0) {
            value = result->right;
            while (value->left != 0)
                value = value->left;
            sprayTreeFirst = value;
        } else {
            value = result;
            for (;;) {
                FxRegistryNode* parent = (FxRegistryNode*)((unsigned int)value->parentTag & kTreeParentAddressMask);
                if (value != parent->left) {
                    value = parent;
                    continue;
                }
                value = parent;
                break;
            }
            sprayTreeFirst = value;
        }
    }
    RBTree_RebalanceAfterErase(result, (void*)sprayTreeRoot);
    appFreeUni__FPv(result);
    --sprayCount;
    ((FxBoxHooks*)this)->SprayRemoved(data);
    if (data != 0 && ((CFxSpray*)data)->parentSequence != 0)
        ((CFxSequence*)(((CFxSpray*)data)->parentSequence))->UnregisterSpray((unsigned int)data);
    if (data != 0)
        delete (FxEffect*)data;
}

#pragma push
#pragma use_lmw_stmw on
// Clear sequences and sprays, optionally preserving entries whose cleanup byte is zero.
void CFxBox::ClearEffects(int mode) {
    // mode == 0 removes every effect. Otherwise the entries whose +4 byte is
    // zero are skipped. The sequence children are detached first, then the
    // spray registry is swept; each sweep advances before deleting its node.
    FxRegistryNode* cur = (FxRegistryNode*)sequenceTreeFirst;
    FxRegistryNode* sequenceEnd = (FxRegistryNode*)&sequenceTreeRoot;
    while (cur != sequenceEnd) {
        if (mode != 0 && ((CFxSequence*)cur->value)->clearOnPartialClear == 0) {
            FxRegistryNode* s = cur->right;
            if (s != 0) {
                while (s->left != 0)
                    s = s->left;
            } else {
                FxRegistryNode* node = cur;
                for (;;) {
                    s = (FxRegistryNode*)((unsigned int)node->parentTag & kTreeParentAddressMask);
                    if (node == s->left)
                        break;
                    node = s;
                }
            }
            cur = s;
        } else {
            EraseSequenceAtIterator(&cur, kDetachChildSprays);
        }
    }
    FxRegistryNode* node = (FxRegistryNode*)sprayTreeFirst;
    FxRegistryNode* end = (FxRegistryNode*)&sprayTreeRoot;
    while (node != end) {
        CFxSpray* spray = (CFxSpray*)node->value;
        int preserveEntry = (mode != 0) && (spray->clearOnPartialClear == 0);
        FxRegistryNode* next;
        if (preserveEntry) {
            next = node->right;
            if (next != 0) {
                while (next->left != 0)
                    next = next->left;
            } else {
                FxRegistryNode* n = node;
                for (;;) {
                    next = (FxRegistryNode*)((unsigned int)n->parentTag & kTreeParentAddressMask);
                    if (n == next->left)
                        break;
                    n = next;
                }
            }
        } else {
            next = node->right;
            if (next != 0) {
                while (next->left != 0)
                    next = next->left;
            } else {
                FxRegistryNode* n = node;
                for (;;) {
                    next = (FxRegistryNode*)((unsigned int)n->parentTag & kTreeParentAddressMask);
                    if (n == next->left)
                        break;
                    n = next;
                }
            }
            FxRegistryNode* saved = node;
            FxRegistryNode* sentinel = (FxRegistryNode*)sprayTreeFirst;
            if (saved == sentinel) {
                FxRegistryNode* succ;
                if (saved->right != 0) {
                    succ = saved->right;
                    while (succ->left != 0)
                        succ = succ->left;
                } else {
                    succ = saved;
                    for (;;) {
                        FxRegistryNode* parent =
                            (FxRegistryNode*)((unsigned int)succ->parentTag & kTreeParentAddressMask);
                        if (succ == parent->left) {
                            succ = parent;
                            break;
                        }
                        succ = parent;
                    }
                }
                sprayTreeFirst = succ;
            }
            RBTree_RebalanceAfterErase(saved, (void*)sprayTreeRoot);
            appFreeUni__FPv(saved);
            --sprayCount;
            ((FxBoxHooks*)this)->SprayRemoved(spray);
            if (spray->parentSequence != 0)
                ((CFxSequence*)(spray->parentSequence))->UnregisterSpray((unsigned int)spray);
            if (spray != 0)
                delete (FxEffect*)spray;
        }
        node = next;
    }
}
#pragma pop
#pragma use_simple_stdpnew on
inline void* operator new(unsigned long, void* storage) {
    return storage;
}
struct CFxSequenceStorage {
    char storage[0x1D8];
    // Construct a sequence in the wrapper's allocated storage.
    CFxSequenceStorage(void* owner, void* cachedTemplateAbiWord) {
        new (this) CFxSequence(owner, (unsigned int)cachedTemplateAbiWord);
    }
};
struct CFxSequenceView {
    virtual ~CFxSequenceView();
};
#pragma push
#pragma use_lmw_stmw on
// Construct and load a sequence, deleting it if script loading fails.
void* CFxSequence_Create(void* sequenceOwner, void* scriptName, void* cachedTemplateAbiWord, void* unusedContext,
                         void* bypassCacheAbiWord, void*, void*) {
    // sequenceOwner is borrowed by construction; scriptName is borrowed for loading.
    // cachedTemplateAbiWord/bypassCacheAbiWord carry low-byte boolean flags in
    // pointer-sized ABI slots (callee definitions: fx_sequence_scripts_and_attachments.cpp). unusedContext and
    // the last two words are unused. Success transfers the allocated sequence;
    // failure deletes it. Preserve these ABI slot types in this matching wrapper.
    CFxSequenceStorage* sequence = new CFxSequenceStorage(sequenceOwner, cachedTemplateAbiWord);
    if (((CFxSequence*)sequence)->LoadByName((const char*)scriptName, (unsigned int)bypassCacheAbiWord))
        return sequence;
    delete (CFxSequenceView*)sequence;
    return 0;
}
#pragma pop

struct CFxSprayView {
    virtual ~CFxSprayView();
};
struct CFxSprayStorage {
    char storage[0x378];
    // Construct a spray in the wrapper's allocated storage.
    CFxSprayStorage(void* effectOwner) {
        ((CFxSpray*)this)->__ct(effectOwner);
    }
};
#pragma push
#pragma use_lmw_stmw on
// Construct a spray and load its script, deleting it if loading fails.
CFxSpray* CFxSpray::CreateFromScript(CFxBox* effectOwner, const char* scriptName, int bypassCachedConfig,
                                     int replaceCachedConfig) {
    CFxSprayStorage* spray = new CFxSprayStorage(effectOwner);
    if (((CFxSpray*)spray)->LoadByName(scriptName, bypassCachedConfig == 0, replaceCachedConfig))
        return (CFxSpray*)spray;
    delete (CFxSprayView*)spray;
    return 0;
}
#pragma pop

// Find the insertion link and predecessor for an object-address registry key.
FxRegistryNode** CFxRegistryTree::FindInsertionPosition(unsigned int* key, FxRegistryNode** predecessorOut,
                                                        unsigned char* descendLeftOut,
                                                        unsigned char* noPredecessorOut) {
    *predecessorOut = 0;
    FxRegistryNode** link = &root;
    FxRegistryNode* node = root;
    *descendLeftOut = 1;
    *noPredecessorOut = 1;
    while (node != 0) {
        // The node's address is the link to its left child, which sits at offset 0;
        // the caller's side flag chooses which child field to fill.
        link = (FxRegistryNode**)node;
        if (*key < node->key) {
            node = node->left;
            *descendLeftOut = 1;
        } else {
            *predecessorOut = node;
            node = node->right;
            *descendLeftOut = 0;
            *noPredecessorOut = 0;
        }
    }
    return link;
}

// Prepare sequence command times, register its address, and notify the box.
void CFxBox::RegisterSequence(CFxSequence* sequence) {
    sequence->RandomizeAndSortCommandTimes();
    FxRegistryNode* parent;
    unsigned char noPredecessor;
    unsigned char descendLeft;
    unsigned int* slot;
    FxRegistryNode** link =
        ((CFxRegistryTree*)&sequenceCount)
            ->FindInsertionPosition((unsigned int*)&sequence, &parent, &descendLeft, &noPredecessor);
    // Insert unless the greatest key not above the query is an exact match.
    if (parent == 0 || (slot = &parent->key, *slot < (unsigned int)sequence)) {
        struct Pair {
            unsigned int key;
            unsigned int global;
        } pair = {(unsigned int)sequence, s_fxRegistryMapDefaultValue};
        slot = &((EffectRegistryNodePrefix*)FxBoxTree_InsertNode((CFxRegistryTree*)&sequenceCount, link, descendLeft,
                                                                 noPredecessor, &pair))
                    ->key;
    }
    slot[1] = (unsigned int)sequence;
    ((FxBoxHooks*)this)->SequenceRegistered(sequence);
}

// Register the spray address and notify the effect box.
void CFxBox::RegisterSpray(CFxSpray* spray) {
    unsigned int* slot;
    FxRegistryNode* parent;
    unsigned char noPredecessor;
    unsigned char descendLeft;
    FxRegistryNode** link;
    link = ((CFxRegistryTree*)&sprayCount)
               ->FindInsertionPosition((unsigned int*)&spray, &parent, &descendLeft, &noPredecessor);
    // Insert unless the greatest key not above the query is an exact match.
    if (parent == 0 || (slot = &parent->key, *slot < (unsigned int)spray)) {
        struct Pair {
            unsigned int key;
            unsigned int global;
        } pair = {(unsigned int)spray, s_fxRegistryMapDefaultValue};
        slot = &((EffectRegistryNodePrefix*)FxBoxTree_InsertNode((CFxRegistryTree*)&sprayCount, link, descendLeft,
                                                                 noPredecessor, &pair))
                    ->key;
    }
    slot[1] = (unsigned int)spray;
    ((FxBoxHooks*)this)->SprayRegistered(spray);
}

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

// Release heap-backed narrow-string data and optionally delete its storage.
void* dtor_8010B1DC(void* ptr, int deleteFlag) {
    if (ptr != 0) {
        if (ptr != 0) {
            unsigned int flags = *(unsigned int*)ptr;
            if ((flags >> kHeapStringTagShift) != 0)
                appFreeUni__FPv(*(void**)((char*)ptr + 8));
        }
        if (deleteFlag > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}

} // extern "C"
