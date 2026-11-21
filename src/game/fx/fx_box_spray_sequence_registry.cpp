#include "game/wvector.h"
#include "classes/CFxSpray.h"
#include "classes/CFxBox.h"
#include "classes/CFxSequence.h"
// Borrowed layout views. Forced-inline replacements preserve instructions but
// change anonymous exception/constant relocation symbols in this compiler.
#define AsCFxBox(p) ((CFxBox*)(p))
#define AsCFxSequence(p) ((CFxSequence*)(p))

// Registry keys are object addresses; parent pointers reserve bit zero for color.
enum { kSmallStringLengthMask = 0x7F, kHeapStringTagShift = 31 };
static const unsigned int kTreeParentAddressMask = ~1u;
// Each registry owns its tree nodes; key and value both hold the effect address.
// End is the address of the root WORD, not null; firstNode caches the minimum.
// Erasure frees a node separately from the virtual effect destructor/hook.
// Cache strings own heap text only when bit 31 is set; inline text starts at +1.
enum SequenceChildPolicy { kDetachChildSprays = 0, kDestroyChildSprays = 1 };
struct EffectRegistryNodePrefix { void* left; void* right; void* parentTag; unsigned int key; };
struct SpriteResourceView { char pad_00[0x10]; void* resourceManager; };
// These fields are outside the currently placed CFxSpray header members.
struct DetachedSprayView { char pad_00[8]; void* attachment; char pad_0C[0xAC]; WVector worldPosition; };

#define AsCFxSpray(p) ((CFxSpray*)(p))
// Translation unit 0x80109E1C-0x8010B244 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Default effect-box extension hook; no additional work is required.
void CFxBox_vfunc_6() {}
// Default effect-box extension hook; no additional work is required.
void CFxBox_vfunc_4() {}
// Default effect-box extension hook; no additional work is required.
void CFxBox_vfunc_3() {}

// Default effect-box extension hook; no additional work is required.
void CFxBox_vfunc_5() {}

extern unsigned int strlen(const char*);
extern double atof(const char*);
extern float kFloat_0_FxBox;
extern float kDegreesToRadians_FxValueParser;
struct EffectParameterScale { char pad_00[4]; float scale; };
// Parse a numeric effect parameter; d converts degrees and f applies the configured scale.
float CFxSpray_ParseScaledFloat(EffectParameterScale* self, const char* value)
{
    unsigned int len = strlen(value);
    if (len == 0) return kFloat_0_FxBox;
    char suffix = value[len - 1];
    if (suffix == 'd') return kDegreesToRadians_FxValueParser * (float)atof(value);
    if (suffix == 'f') return self->scale * (float)atof(value);
    return (float)atof(value);
}

struct TreeNode_8010A20C {
    TreeNode_8010A20C* left;
    TreeNode_8010A20C* right;
    char pad_08[4];
    unsigned int key;
};
struct Tree_8010A20C { char pad_00[0x164]; TreeNode_8010A20C* root; };
struct IterNode_8010A28C;
struct Owner_8010A28C;
void CFxBox_EraseSequenceAtIterator(Owner_8010A28C*, IterNode_8010A28C**, int);
// Find a sequence by its address key and erase it with the requested child policy.
void CFxBox_RemoveSequence(Tree_8010A20C* tree, unsigned int key, int mode)
{
    TreeNode_8010A20C* node = tree->root;
    TreeNode_8010A20C* where = (TreeNode_8010A20C*)&tree->root;
    while (node != 0) {
        if (node->key >= key) {
            where = node;
            node = node->left;
        } else node = node->right;
    }
    if (where == (TreeNode_8010A20C*)&tree->root || key < where->key)
        where = (TreeNode_8010A20C*)&tree->root;
    TreeNode_8010A20C* iter = where;
    if (where != (TreeNode_8010A20C*)&tree->root)
        CFxBox_EraseSequenceAtIterator((Owner_8010A28C*)tree, (IterNode_8010A28C**)&iter, mode);
}

struct IterNode_8010A28C {
    IterNode_8010A28C* left;
    IterNode_8010A28C* right;
    IterNode_8010A28C* parentTag;
    unsigned int key;
    void* value;
};
struct IterTree_8010A28C { char pad_00[0x154]; unsigned int size; IterNode_8010A28C* root; IterNode_8010A28C* firstNode; };
struct Owner_8010A28C {
    char pad_00[0x154]; unsigned int size;
    IterNode_8010A28C* root;
    IterNode_8010A28C* firstNode;
    unsigned int count0;
    unsigned int count1;
    unsigned int count2;
    void* object;
};
extern void appFreeUni__FPv(void*);
extern void RBTree_RebalanceAfterErase(void*, void*);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern void CFxSequence_UnregisterSpray(void*, void*);
extern unsigned int s_fxRegistryMapDefaultValue;
struct Tree_8010A8CC;
struct TreeNode_8010A8CC;
extern TreeNode_8010A8CC** CFxRegistryTree_FindInsertionPosition(Tree_8010A8CC*, unsigned int*, TreeNode_8010A8CC**, unsigned char*, unsigned char*);
extern TreeNode_8010A8CC* FxBoxTree_InsertNode(Tree_8010A8CC*, TreeNode_8010A8CC**, unsigned char, unsigned char, void*);
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
static inline void advance_8010A28C(IterNode_8010A28C*& node)
{
    IterNode_8010A28C* next = node->right;
    if (next != 0) {
        while (next->left != 0) next = next->left;
        node = next;
    } else {
        while (node != (next = (IterNode_8010A28C*)((unsigned int)node->parentTag & kTreeParentAddressMask))->left) {
            node = next;
        }
        node = next;
    }
}
struct Iterator_8010A28C {
    IterNode_8010A28C* node;
    // Borrow a registry node as an iterator; no ownership changes.
    Iterator_8010A28C(IterNode_8010A28C* p) : node(p) {}
    // Copy the borrowed node cursor.
    Iterator_8010A28C(const Iterator_8010A28C& p) : node(p.node) {}
    // Advance to the successor and return this cursor.
    Iterator_8010A28C& operator++() { advance_8010A28C(node); return *this; }
    // Advance to the successor, returning the previous cursor.
    Iterator_8010A28C operator++(int) {
        Iterator_8010A28C old = *this;
        ++*this;
        return old;
    }
};
#pragma push
#pragma use_lmw_stmw on
// Erase a sequence, deleting or detaching its sprays according to the caller's policy.
void CFxBox_EraseSequenceAtIterator(Owner_8010A28C* self, IterNode_8010A28C** iter, int mode)
{
    // *iter must be a live sequence node; it advances before removal. mode == 0
    // detaches children into world space; any nonzero mode removes/deletes them.
    // The sequence object is deleted only if deleteWhenRemovedFromBox is set.
    // Offsets are derived from the shared classes; retain these byte-based
    // accesses because direct member casts change tree repair code generation.
    enum { kBoxOffset_sprayCount = (int)&((CFxBox*)0)->sprayCount };
    enum { kBoxOffset_sequenceCount = (int)&((CFxBox*)0)->sequenceCount };
    enum { kBoxOffset_sequenceTreeRoot = (int)&((CFxBox*)0)->sequenceTreeRoot };
    enum { kBoxOffset_sequenceTreeFirst = (int)&((CFxBox*)0)->sequenceTreeFirst };
    enum { kSequenceOffset_firstSprayNode = (int)&((CFxSequence*)0)->firstSprayNode };
    enum { kSequenceOffset_sprayTreeRoot = (int)&((CFxSequence*)0)->sprayTreeRoot };
    enum { kSequenceOffset_sprayCount = (int)&((CFxSequence*)0)->sprayCount };
    enum { kSequenceOffset_worldPosition = (int)&((CFxSequence*)0)->worldPosition };
    // Phase 1: unlink/free the box's sequence node; keep its object for child cleanup.
    char* sequenceBytes = (char*)(*iter)->value;
    char* boxBytes = (char*)self;
    Iterator_8010A28C removedSequenceIterator = (*(Iterator_8010A28C*)iter)++;
    // 0x8010A2A4-0x8010A368: the erased node is the original *iter (r25 saved before the
    // begin-pointer update); ours erased begin's successor when *iter was begin.
    Iterator_8010A28C erasedIterator = removedSequenceIterator;
    if (removedSequenceIterator.node == *(IterNode_8010A28C**)(boxBytes + kBoxOffset_sequenceTreeFirst)) {
        removedSequenceIterator++;
        *(Iterator_8010A28C*)(boxBytes + kBoxOffset_sequenceTreeFirst) = removedSequenceIterator;
    }
    RBTree_RebalanceAfterErase(erasedIterator.node, *(void**)(boxBytes + kBoxOffset_sequenceTreeRoot));
    appFreeUni__FPv(erasedIterator.node);
    --*(unsigned int*)(boxBytes + kBoxOffset_sequenceCount);
    Iterator_8010A28C sprayIterator = *(Iterator_8010A28C*)(sequenceBytes + kSequenceOffset_firstSprayNode);
    IterNode_8010A28C* end = (IterNode_8010A28C*)(sequenceBytes + kSequenceOffset_sprayTreeRoot);
    // Phase 2: discard null nodes, destroy selected children, or detach survivors.
    while (end != sprayIterator.node) {
        char* sprayBytes = (char*)sprayIterator.node->value;
        if (sprayBytes == 0) {
            // 0x8010A388-0x8010A454 (r4 set at 0x8010A714): erase the node holding the null entry (r4 = cur before the
            // advance), not its successor; the begin check is on that node too.
            Iterator_8010A28C item = sprayIterator++; erasedIterator = item;
            if (item.node == *(IterNode_8010A28C**)(sequenceBytes + kSequenceOffset_firstSprayNode)) {
                ++item;
                *(Iterator_8010A28C*)(sequenceBytes + kSequenceOffset_firstSprayNode) = item;
            }
            RBTree_RebalanceAfterErase(erasedIterator.node, *(void**)(sequenceBytes + kSequenceOffset_sprayTreeRoot));
            appFreeUni__FPv(erasedIterator.node);
            --*(unsigned int*)(sequenceBytes + kSequenceOffset_sprayCount);
        } else if (0 != sprayBytes && mode != 0) {
            ++sprayIterator;
            IterNode_8010A28C* where = (IterNode_8010A28C*)&self->root;
            IterNode_8010A28C* search = self->root;
            while (search != 0) {
                if (search->key >= (unsigned int)sprayBytes) { where = search; search = search->left; }
                else search = search->right;
            }
            if (where == (IterNode_8010A28C*)&self->root || (unsigned int)sprayBytes < where->key)
                where = (IterNode_8010A28C*)&self->root;
            IterNode_8010A28C* unused = where;
            if (where == (IterNode_8010A28C*)&self->root) continue;
            IterNode_8010A28C* unusedNext = unused->right;
            sprayBytes = (char*)where->value;
            if (0 != unusedNext) {
                do { unusedNext = unusedNext->left; } while (unusedNext);
            } else {
                while (unused != (unusedNext = (IterNode_8010A28C*)((unsigned int)unused->parentTag & kTreeParentAddressMask))->left)
                    unused = unusedNext;
            }
            IterNode_8010A28C* erased = where;
            if (where == self->firstNode) {
                advance_8010A28C(where);
                self->firstNode = where;
            }
            RBTree_RebalanceAfterErase(erased, self->root);
            appFreeUni__FPv(erased);
            self->size -= 1;
            ((Interface_8010A28C*)self)->erase(sprayBytes);
            if (*(void**)&AsCFxSpray(sprayBytes)->parentSequence != 0) CFxSequence_UnregisterSpray(*(void**)&AsCFxSpray(sprayBytes)->parentSequence, sprayBytes); // 0x8010A28C: +0x88
            delete (Interface_8010A28C*)sprayBytes;
        } else {
            ((DetachedSprayView*)sprayBytes)->worldPosition += *(WVector*)(sequenceBytes + kSequenceOffset_worldPosition);
            *(void**)&AsCFxSpray(sprayBytes)->parentSequence = 0; // 0x8010A28C: +0x88
            if (AsCFxSpray(sprayBytes)->particlesFollowEmitter != 0) { if (*(void**)&AsCFxSequence(sequenceBytes)->attachment != 0) { unsigned char descendLeft; // +0x170
                    ((DetachedSprayView*)sprayBytes)->attachment = *(void**)&AsCFxSequence(sequenceBytes)->attachment;
                    unsigned int key = (unsigned int)sprayBytes;
                    TreeNode_8010A8CC* parent;

                    unsigned int* slot;
                    unsigned char noPredecessor;
                    TreeNode_8010A8CC** link = CFxRegistryTree_FindInsertionPosition((Tree_8010A8CC*)(boxBytes + kBoxOffset_sprayCount), &key, &parent, &descendLeft, &noPredecessor);
                    // Insert unless the greatest key <= query is an exact match.
                    if (0 == parent || (slot = (unsigned int*)((char*)parent + (int)&((EffectRegistryNodePrefix*)0)->key), *slot < key))
                    {
                        struct Pair { unsigned int key; int global; } pair = { key, s_fxRegistryMapDefaultValue };
                        slot = &((EffectRegistryNodePrefix*)FxBoxTree_InsertNode((Tree_8010A8CC*)(boxBytes + kBoxOffset_sprayCount), link, descendLeft, noPredecessor, &pair))->key;
                    }
                    slot[1] = key;
                    ((Interface_8010A28C*)self)->insert((void*)key);
                } }
            sprayIterator++;
        }
    }
    // Phase 3: notify the box and honor the sequence's own deletion policy.
    ((Interface_8010A28C*)self)->removeGroup(sequenceBytes);
    if (*(unsigned char*)&AsCFxSequence(sequenceBytes)->deleteWhenRemovedFromBox != 0) delete (Interface_8010A28C*)sequenceBytes;
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
struct SpriteCacheString { unsigned int storageHeader; unsigned int lengthOrInlineBytes; char* data; };
struct SpriteCacheEntry { SpriteCacheString str; void* extra0; void* extra1; };
struct Alloc_AB20 { char c __attribute__((aligned(4))); };
struct Bound_8010B088;
struct CompareNode_8010B088;
extern CompareNode_8010B088* FxAnimatedSpriteCache_FindInsertionPosition(CompareNode_8010B088*, Bound_8010B088*, CompareNode_8010B088**, unsigned char*, unsigned char*);
struct AnimatedSpriteStorage {
    char storage[0x19c];
    // Construct the animated-sprite base in this newly allocated wrapper.
    AnimatedSpriteStorage() { W3dAniSpr_Ctor(this); }
};
struct SpriteCacheNode { SpriteCacheNode* left; SpriteCacheNode* right; char pad_08[4]; SpriteCacheString key; void* made; void* obj; };
struct SpriteCacheOwner { char pad_00[0x140]; void* fallback; char pad_144[4]; void* root; };
struct StringNode_AB20 { StringNode_AB20* left; StringNode_AB20* right; char pad_08[4]; unsigned char flag; };
struct StringTree_AB20 { void* header; StringNode_AB20* root; };

// Read a narrow small-string length from its inline or heap representation.
static inline unsigned int sslen_AB20(const SpriteCacheString* s)
{
    if ((s->storageHeader >> kHeapStringTagShift) == 0) return *(const unsigned char*)s & kSmallStringLengthMask;
    return s->lengthOrInlineBytes;
}
// Return inline or heap text without allocating.
static inline const char* ssdata_AB20(const SpriteCacheString* s)
{
    if ((s->storageHeader >> kHeapStringTagShift) == 0) return (const char*)s + 1;
    return s->data;
}
// Return the smaller string-comparison bound by reference.
static inline const unsigned int& min_AB20(const unsigned int& a, const unsigned int& b)
{
    return a < b ? a : b;
}
// Compare bounded byte strings, breaking a matching prefix by length.
static inline int cmpstr_AB20(const char* ad, const char* bd, unsigned int alen,
                               unsigned int alen2, unsigned int blen)
{
    unsigned int t = min_AB20(alen, alen2);
    int c = memcmp(ad, bd, min_AB20(blen, t));
    if (c == 0) {
        if (t < blen) c = -1;
        else if (t == blen) c = 0;
        else c = 1;
    }
    return c;
}
// Compare insertion keys with the original nested length bounds.
static inline int cmplt_AB20(const char* ad, const char* bd, unsigned int alen,
                              unsigned int alen2, unsigned int blen)
{
    unsigned int t = min_AB20(alen, alen2);
    unsigned int n = min_AB20(blen, t);
    int c = memcmp(ad, bd, n);
    if (c == 0) {
        if (n < blen) c = -1;
        else if (n == blen) c = 0;
        else c = 1;
    }
    return c;
}
// Decode a node key for bounded comparison without treating inline words as a heap pointer.
static inline void ReadSpriteComparisonSpan(SpriteCacheString* key, unsigned int& firstLength,
    unsigned int& secondLength, const char*& characters) {
    if ((key->storageHeader >> kHeapStringTagShift) == 0)
        firstLength = *(unsigned char*)&key->storageHeader & kSmallStringLengthMask;
    else firstLength = key->lengthOrInlineBytes;
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
void* CFxBox_GetOrLoadAnimatedSprite(SpriteCacheOwner* self, const char* bitmapName, unsigned int cellWidth, unsigned int cellHeight)
{
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
    CompareNode_8010B088* insertionPredecessor;
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
            !keyIsLong ? keySmallLen : key.lengthOrInlineBytes,
            parentLen) >= 0) ? parent : (SpriteCacheNode*)(&AsCFxBox(self)->spriteCacheRoot);
    }
    if (keyLong != 0) appFreeUni__FPv(key.data);
    if ((char*)parent != (char*)&AsCFxBox(self)->spriteCacheRoot) return parent->made;
    if (self->fallback == 0) return 0;
    // Phase 2: load the bitmap by its original name and construct grid frames.
    bitmap = ResourceManager_LoadBitmap(self->fallback, bitmapName, 0, 0, 0);
    if (bitmap == 0) return 0;
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
        CompareNode_8010B088* result = FxAnimatedSpriteCache_FindInsertionPosition((CompareNode_8010B088*)(&AsCFxBox(self)->spriteCacheCount), (Bound_8010B088*)&name, &insertionPredecessor, &found, &insert);
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
                    ins = AnimatedSpriteCache_InsertNode(&AsCFxBox(self)->spriteCacheCount, result, found, insert, (void**)&bound.str);
                } catch (...) {
                    if (bound.str.storageHeader >> kHeapStringTagShift) appFreeUni__FPv(bound.str.data);
                    throw;
                }
                cacheEntryBytes = (char*)&((EffectRegistryNodePrefix*)ins)->key;
            }
            if ((bound.str.storageHeader >> kHeapStringTagShift) != 0) appFreeUni__FPv(bound.str.data);
        } while (0);

        ((SpriteCacheEntry*)cacheEntryBytes)->extra0 = sprite;
        ((SpriteCacheEntry*)cacheEntryBytes)->extra1 = bitmap;
        if ((name.storageHeader >> kHeapStringTagShift) != 0) appFreeUni__FPv(name.data);
        return sprite;
    } catch (...) {
        if (name.storageHeader >> kHeapStringTagShift) appFreeUni__FPv(name.data);
        throw;
    }
}
#pragma pop
extern void CFxSequence_ctor(void*, void*, void*);
extern int CFxSequence_LoadByName(void*, void*, void*);
extern void CFxSpray_Ctor(void*, void*);
extern int CFxSpray_LoadByName(void*, void*, int, void*);
extern void __dl__FPv(void*);
extern void appFreeUni__FPv(void*);

struct Bound_8010B088 { unsigned char flag; };
struct CompareNode_8010B088 {
    CompareNode_8010B088* left;
    CompareNode_8010B088* right;
    char pad_08[4];
    unsigned char flag;
    unsigned char inlineKey[0];
    unsigned int heapKeyLength; // 0x10: used when the flag word has its high bit set.
    void* heapKeyData; // 0x14
};
extern int memcmp(const void*, const void*, unsigned int);
// Return the smaller insertion-key comparison bound.
static inline const unsigned int& min_8010B088(const unsigned int& a, const unsigned int& b)
{
    return a < b ? a : b;
}
// Decode the query string's inline/heap data pointer and length.
static inline const void* data_8010B088(Bound_8010B088* query, unsigned int& len)
{
    unsigned int n;
    const void* p;
    if ((*(unsigned int*)query >> kHeapStringTagShift) == 0) { p = &query->flag + 1; n = query->flag & kSmallStringLengthMask; }
    else { p = *(void**)((char*)query + 8); n = *(unsigned int*)((char*)query + 4); }
    len = n;
    return p;
}
#pragma push
#pragma use_lmw_stmw on
// Find the tree link and predecessor flags for an animated-sprite name.
CompareNode_8010B088* FxAnimatedSpriteCache_FindInsertionPosition(CompareNode_8010B088* tree, Bound_8010B088* query,
    CompareNode_8010B088** outNode, unsigned char* found, unsigned char* end)
{
    *outNode = 0;
    unsigned int nodeKey;
    CompareNode_8010B088* parent = (CompareNode_8010B088*)&tree->right;
    CompareNode_8010B088* node = tree->right;
    *found = 1;
    *end = 1;
    while (node != 0) {
        parent = node;
        const void* nodeData;
        unsigned int queryKey;
        if ((*(unsigned int*)&node->flag >> kHeapStringTagShift) == 0) { nodeData = &node->flag + 1; nodeKey = node->flag & kSmallStringLengthMask; }
        else { nodeData = node->heapKeyData; nodeKey = node->heapKeyLength; }
        unsigned int nodeSize = nodeKey;
        if ((*(unsigned int*)query >> kHeapStringTagShift) == 0) queryKey = query->flag & kSmallStringLengthMask;
        else queryKey = *(unsigned int*)((char*)query + 4);
        unsigned int qend;
        const void* queryData = data_8010B088(query, qend);
        unsigned int lo = min_8010B088(queryKey, qend);
        int cmp = memcmp(queryData, nodeData, min_8010B088(nodeSize, lo));
        if (cmp == 0) {
            if (lo < nodeKey) cmp = -1;
            else if (lo != nodeKey) cmp = 1;
        }
        if (cmp < 0) { node = node->left; *found = 1; }
        else { *outNode = parent; node = node->right; *found = 0; *end = 0; }
    }
    return parent;
}
#pragma pop

struct EraseNode_8010A070 { EraseNode_8010A070* left; EraseNode_8010A070* right; EraseNode_8010A070* parentTag; unsigned int key; void* value; };
struct EraseTree_8010A070 { char pad_00[0x154]; unsigned int size; EraseNode_8010A070* root; EraseNode_8010A070* firstNode; };
extern void RBTree_RebalanceAfterErase(void*, void*);
extern void CFxSequence_UnregisterSpray(void*, void*);
// Erase a registered spray, notify the box and parent sequence, then destroy it.
void CFxBox_RemoveSpray(EraseTree_8010A070* self, unsigned int key)
{
    void* data;
    EraseNode_8010A070* result = (EraseNode_8010A070*)&self->root;
    EraseNode_8010A070* node = self->root;
    EraseNode_8010A070* value;
    while (node != 0) {
        if (node->key >= key) { result = node; node = node->left; }
        else node = node->right;
    }
    if (result != (EraseNode_8010A070*)&self->root && key < result->key) result = (EraseNode_8010A070*)&self->root;
    if (result == (EraseNode_8010A070*)&self->root) return;
    data = result->value;
    value = result->right;
    if (value != 0) { while (value->left != 0) value = value->left; }
    else {
        value = result;
        for (;;) {
            EraseNode_8010A070* parent = (EraseNode_8010A070*)((unsigned int)value->parentTag & kTreeParentAddressMask);
            if (value != parent->left) { value = parent; continue; }
            value = parent; break;
        }
    }
    if (result == self->firstNode) {
        if (result->right != 0) { value = result->right; while (value->left != 0) value = value->left; self->firstNode = value; }
        else {
            value = result;
            for (;;) {
                EraseNode_8010A070* parent = (EraseNode_8010A070*)((unsigned int)value->parentTag & kTreeParentAddressMask);
                if (value != parent->left) { value = parent; continue; }
                value = parent; break;
            }
            self->firstNode = value;
        }
    }
    RBTree_RebalanceAfterErase(result, self->root);
    appFreeUni__FPv(result);
    --self->size;
    ((void (*)(void*, void*))((void**)*(void**)self)[8])(self, data);
    if (data != 0 && *(void**)&AsCFxSpray(data)->parentSequence != 0) CFxSequence_UnregisterSpray(*(void**)&AsCFxSpray(data)->parentSequence, data); // +0x88
    if (data != 0) ((void (*)(void*, int))(((void**) *(void**)data)[2]))(data, 1);
}

struct CleanupNode_80109E1C {
    CleanupNode_80109E1C* left;
    CleanupNode_80109E1C* right;
    CleanupNode_80109E1C* parentTag;
    char pad_0C[4];
    void* value;
};
struct CleanupTree_80109E1C { char pad_00[0x154]; unsigned int size; CleanupNode_80109E1C* root; CleanupNode_80109E1C* firstNode; };
extern void RBTree_RebalanceAfterErase(void*, void*);
#pragma push
#pragma use_lmw_stmw on
// Clear sequences and sprays, optionally preserving entries whose cleanup byte is zero.
void CFxBox_ClearEffects(CleanupTree_80109E1C* self, int mode)
{
    // mode == 0 removes all effects. Otherwise skip entries whose +4 byte is
    // zero (the byte's wider meaning is unresolved). First detach sequence
    // children, then sweep sprays; each sweep advances before deleting nodes.
    CleanupNode_80109E1C* cur = *(CleanupNode_80109E1C**)(&AsCFxBox(self)->sequenceTreeFirst);
    CleanupNode_80109E1C* header1 = (CleanupNode_80109E1C*)(&AsCFxBox(self)->sequenceTreeRoot);
    while (cur != header1) {
        if (mode != 0 && *(unsigned char*)&AsCFxSequence(cur->value)->unk_4 == 0) {
            CleanupNode_80109E1C* s = cur->right;
            if (s != 0) { while (s->left != 0) s = s->left; }
            else {
                // 0x80109E8C-0x80109EA8: climb while the node is not its parent's left child
                // (ours compared the start node with each ancestor's right link)
                CleanupNode_80109E1C* node = cur;
                for (;;) {
                    s = (CleanupNode_80109E1C*)((unsigned int)node->parentTag & kTreeParentAddressMask);
                    if (node == s->left) break;
                    node = s;
                }
            }
            cur = s;
        } else {
            CFxBox_EraseSequenceAtIterator((Owner_8010A28C*)self, (IterNode_8010A28C**)&cur, kDetachChildSprays);
        }
    }
    CleanupNode_80109E1C* node = *(CleanupNode_80109E1C**)(&AsCFxBox(self)->sprayTreeFirst);
    CleanupNode_80109E1C* end = (CleanupNode_80109E1C*)(&AsCFxBox(self)->sprayTreeRoot);
    while (node != end) {
        void* value = node->value;
        int preserveEntry = (mode != 0) && (AsCFxSpray(value)->unk_4 == 0);
        CleanupNode_80109E1C* next;
        if (preserveEntry) {
            next = node->right;
            if (next != 0) { while (next->left != 0) next = next->left; }
            else {
                // 0x80109F1C-0x80109F38: climb while not the parent's left child
                CleanupNode_80109E1C* n = node;
                for (;;) {
                    next = (CleanupNode_80109E1C*)((unsigned int)n->parentTag & kTreeParentAddressMask);
                    if (n == next->left) break;
                    n = next;
                }
            }
        } else {
            next = node->right;
            if (next != 0) { while (next->left != 0) next = next->left; }
            else {
                // 0x80109F70-0x80109F88: climb while not the parent's left child
                CleanupNode_80109E1C* n = node;
                for (;;) {
                    next = (CleanupNode_80109E1C*)((unsigned int)n->parentTag & kTreeParentAddressMask);
                    if (n == next->left) break;
                    n = next;
                }
            }
            CleanupNode_80109E1C* saved = node;
            CleanupNode_80109E1C* sentinel = *(CleanupNode_80109E1C**)(&AsCFxBox(self)->sprayTreeFirst);
            if (saved == sentinel) {
                CleanupNode_80109E1C* succ;
                if (saved->right != 0) { succ = saved->right; while (succ->left != 0) succ = succ->left; }
                else {
                    succ = saved;
                    for (;;) {
                        CleanupNode_80109E1C* parent = (CleanupNode_80109E1C*)((unsigned int)succ->parentTag & kTreeParentAddressMask);
                        if (succ == parent->left) { succ = parent; break; }
                        succ = parent;
                    }
                }
                *(CleanupNode_80109E1C**)(&AsCFxBox(self)->sprayTreeFirst) = succ;
            }
            RBTree_RebalanceAfterErase(saved, *(void**)(&AsCFxBox(self)->sprayTreeRoot));
            appFreeUni__FPv(saved);
            --self->size;
            ((void (*)(void*, void*))((void**)*(void**)self)[8])(self, value);
            if (*(void**)&AsCFxSpray(value)->parentSequence != 0) CFxSequence_UnregisterSpray(*(void**)&AsCFxSpray(value)->parentSequence, value); // +0x88
            // 0x8010A034-0x8010A050: if (value) deleting destructor at vtable +0x8 (ours called vtable word 0)
            if (value != 0) ((void (*)(void*, int))((void**)*(void**)value)[2])(value, 1);
        }
        node = next;
    }
}
#pragma pop
struct AllocObj_8010A778 {
    char storage[0x1D8];
    // Borrow the owner and forward the cached-template flag in its original ABI slot.
    AllocObj_8010A778(void* owner, void* cachedTemplateAbiWord) { CFxSequence_ctor(this, owner, cachedTemplateAbiWord); }
};
struct AllocIface_8010A778 { 
virtual ~AllocIface_8010A778();
};
#pragma push
#pragma use_lmw_stmw on
// Construct and load a sequence, deleting it if script loading fails.
void* CFxSequence_Create(void* sequenceOwner, void* scriptName, void* cachedTemplateAbiWord, void* unusedContext, void* bypassCacheAbiWord, void*, void*)
{
    // sequenceOwner is borrowed by construction; scriptName is borrowed for loading.
    // cachedTemplateAbiWord/bypassCacheAbiWord carry low-byte boolean flags in
    // pointer-sized ABI slots (callee definitions: fx_sequence_scripts_and_attachments.cpp). unusedContext and
    // the last two words are unused. Success transfers the allocated sequence;
    // failure deletes it. Preserve these ABI slot types in this matching wrapper.
    AllocObj_8010A778* sequence = new AllocObj_8010A778(sequenceOwner, cachedTemplateAbiWord);
    if (CFxSequence_LoadByName(sequence, scriptName, bypassCacheAbiWord)) return sequence;
    delete (AllocIface_8010A778*)sequence;
    return 0;
}
#pragma pop

struct AllocObj_8010A92C {
    virtual ~AllocObj_8010A92C();
};
struct AllocStorage_8010A92C {
    char storage[0x378];
    // Construct a spray in the wrapper's allocated storage.
    AllocStorage_8010A92C(void* effectOwner) { CFxSpray_Ctor(this, effectOwner); }
};
#pragma push
#pragma use_lmw_stmw on
// Construct a spray and load its script, deleting it if loading fails.
void* CFxSpray_CreateFromScript(void* effectOwner, void* scriptName, int bypassCachedConfig, void* replaceConfigAbiWord, void* unusedArgument5, void* unusedArgument6)
{
    // effectOwner is borrowed by construction; scriptName is borrowed for loading.
    // bypassCachedConfig==0 enables cached configuration; replaceConfigAbiWord
    // forwards the loader's integer replace flag in its original ABI slot.
    // The last two words are unused. Success transfers the new spray; failure
    // calls its deleting destructor (callee definitions: fx_particle_and_spray.cpp/fx_spray_list_and_loading.cpp).
    AllocStorage_8010A92C* spray = new AllocStorage_8010A92C(effectOwner);
    if (CFxSpray_LoadByName(spray, scriptName, bypassCachedConfig == 0, replaceConfigAbiWord)) return spray;
    delete (AllocObj_8010A92C*)spray;
    return 0;
}
#pragma pop

struct TreeNode_8010A8CC {
    TreeNode_8010A8CC* left;
    TreeNode_8010A8CC* right;
    char pad_08[4];
    unsigned int key;
};

struct Tree_8010A8CC {
    char pad_00[4];
    TreeNode_8010A8CC* root;
};

// Find the insertion link and predecessor for an object-address registry key.
TreeNode_8010A8CC** CFxRegistryTree_FindInsertionPosition(Tree_8010A8CC* tree, unsigned int* key,
    TreeNode_8010A8CC** predecessorOut, unsigned char* descendLeftOut, unsigned char* noPredecessorOut)
{
    // predecessorOut is the greatest key <= query (or null).
    // descendLeftOut is the last descent direction (1 = left); noPredecessorOut is 1 only
    // when no predecessor exists. The returned link/parent are for InsertNode.
    *predecessorOut = 0;
    TreeNode_8010A8CC** link = &tree->root;
    TreeNode_8010A8CC* node = tree->root;
    *descendLeftOut = 1;
    *noPredecessorOut = 1;
    while (node != 0) {
        link = (TreeNode_8010A8CC**)node;
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

extern void* CFxSequence_RandomizeAndSortCommandTimes(void*);
extern unsigned int s_fxRegistryMapDefaultValue;
extern TreeNode_8010A8CC* FxBoxTree_InsertNode(Tree_8010A8CC*, TreeNode_8010A8CC**, unsigned char,
    unsigned char, void*);
struct Owner_8010A810 {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void insert(unsigned int);
    char pad_04[0x15C];
    Tree_8010A8CC tree;
};
// Prepare sequence command times, register its address, and notify the box.
void CFxBox_RegisterSequence(Owner_8010A810* self, unsigned int key)
{
    CFxSequence_RandomizeAndSortCommandTimes((void*)key);
    TreeNode_8010A8CC* parent;
    unsigned char noPredecessor;
    unsigned char descendLeft;
    unsigned int* slot;
    TreeNode_8010A8CC** link = CFxRegistryTree_FindInsertionPosition(&self->tree, &key, &parent, &descendLeft, &noPredecessor);
    // Insert unless the greatest key <= query is an exact match.
    if (parent == 0 || (slot = &parent->key, *slot < key))
    {
        struct Pair { unsigned int key; unsigned int global; } pair = { key, s_fxRegistryMapDefaultValue };
        slot = &((EffectRegistryNodePrefix*)FxBoxTree_InsertNode(&self->tree, link, descendLeft, noPredecessor, &pair))->key;
    }
    slot[1] = key;
    self->insert(key);
}

extern TreeNode_8010A8CC** CFxRegistryTree_FindInsertionPosition(Tree_8010A8CC*, unsigned int*, TreeNode_8010A8CC**, unsigned char*, unsigned char*);
struct Owner_8010A9C8 {
    virtual ~Owner_8010A9C8();
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void insert(unsigned int);
    char pad_04[0x150];
    Tree_8010A8CC tree;
};
// Register the spray address and notify the effect box.
void CFxBox_RegisterSpray(Owner_8010A9C8* self, unsigned int key)
{
    unsigned int* slot;
    TreeNode_8010A8CC* parent;
    unsigned char noPredecessor;
    unsigned char descendLeft;
    TreeNode_8010A8CC** link;
    link = CFxRegistryTree_FindInsertionPosition(&self->tree, &key, &parent, &descendLeft, &noPredecessor);
    // Insert unless the greatest key <= query is an exact match.
    if (parent == 0 || (slot = &parent->key, *slot < key))
    {
        struct Pair { unsigned int key; unsigned int global; } pair = { key, s_fxRegistryMapDefaultValue };
        slot = &((EffectRegistryNodePrefix*)FxBoxTree_InsertNode(&self->tree, link, descendLeft, noPredecessor, &pair))->key;
    }
    slot[1] = key;
    self->insert(key);
}

extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);

// Release heap-backed narrow-string data and optionally delete its storage.
void* dtor_8010B1DC(void* ptr, int deleteFlag)
{
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

