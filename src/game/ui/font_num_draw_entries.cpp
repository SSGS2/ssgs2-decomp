// Numeric text entries for the on-screen numbers (CFontNumDraw): a keyed table of numbers with
// formatting flags (leading characters, thousands separators, percent suffix).
//
// Also holds the per-hole rival stroke and Pang lookups and the tree of font table nodes.

#include <decomp/static_init.h>
#include "classes/FontTreeLinks80250.h"
#include "classes/FontNumericEntry.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CFontNumDraw.h"
#include "classes/CComStrokeScore.h"

// Numeric entry headers differ from the manager's tree layout.
struct NumericEntryHeader {
    void* definition;
    void* group;
    void* vtable;
};
struct NumericEntryDefinition {
    const char* name;
    int key;
    unsigned int flags;
};

// Definition = {borrowed R_NUM_ group name, integer lookup key, formatting flags}.
// Entry = {definition +0, BP group +4, vtable +8, resource name +0xC,
//          small string +0x10, resource handle +0x1C, format state +0x20}.
// FontNumericEntry::resourceNameWord is the resource-name pointer encoded as a word here;
// the actual tree lookup key is NumericKeyNode::key. Do not interchange them.
// Small string: top bit at +0x10 selects heap storage (+0x14 length/+0x18 data);
// otherwise low seven bits of byte +0x10 give length, text starts at +0x11.
// Tree nodes have links at +0/+4, tagged parent at +8, key/value at +0xC/+0x10.
// The root-pointer address acts as end sentinel; the entry table owns payloads.
struct SharedRoundHoleOrder {
    char reserved[0x82A];
    unsigned char holes[18];
};
struct HoleRewardModes {
    char reserved[0x36C];
    unsigned int modes[18];
};
struct NumericEntryStorage {
    FontNumericEntry entry;
    unsigned int formatState;
};
// Construction view of the payload, deliberately separate from NumericKeyNode.
// textStorageWord is zeroed as a word because it also contains inline characters.
struct NumericEntryConstruction {
    void* definition;
    void* group;
    void* vtable;
    int resourceNameWord;
    unsigned int textStorageWord;
    unsigned int textLength;
    void* text;
    void* resource;
    unsigned int formatState;
};
typedef char NumericEntryConstruction_size[(sizeof(NumericEntryConstruction) == 0x24) ? 1 : -1];
typedef char NumericEntryStorage_size_check[(sizeof(NumericEntryStorage) == 0x24) ? 1 : -1];
typedef char NumericEntryDefinition_size_check[(sizeof(NumericEntryDefinition) == 0xC) ? 1 : -1];
// Bits of definition->flags (the entry definition's flags word) read by
// UITextTable_ApplyNumericTextFormatFlags. Per-flag map, which function step uses it:
//   0x004 kKeepLeadingCharacter  step 1: never strip the first character (all modes)
//   0x040 kKeepInModeOne         step 1: keep the first character when mode == 1
//   0x080 kKeepInModeZero        step 1: keep the first character when mode == 0
//   0x100 kGroupThousands        step 2: insert the pooled separator every 3 characters
//   0x200 kAppendPercent         step 3: append the pooled percent suffix (value not scaled)
//   0x400 kOwnedResource         read elsewhere via kOwnedResourceShift (resource-ownership bit)
// Byte offset of the inline-string length byte (low 7 bits; bit 7 of textFlags means heap-backed text);
// inline text bytes start at +0x11.
enum {
    kEntryInlineLengthByte = 0x10,
    kEntryInlineTextByte = 0x11
};
enum NumericEntryFlags {
    kKeepLeadingCharacter = 0x4,
    kKeepInModeOne = 0x40,
    kKeepInModeZero = 0x80,
    kGroupThousands = 0x100,
    kAppendPercent = 0x200,
    kOwnedResource = 0x400,
    kOwnedResourceShift = 10,
};
enum NumericLayout {
    kEntryResourceOffset = (int)&((FontNumericEntry*)0)->resource,
    kTreePayloadOffset = 0xC,
    kEntryTextOffset = 0x10,
};
enum NumericFormatMode {
    kFormatModeZero,
    kFormatModeOne,
    kStripUnlessExplicitlyKept,
};
enum NumericStringOffset {
    kThousandsSeparator = 7,
    kPercentSuffix = 9,
};
enum RivalScoreLayout {
    kRivalHoleStride = 0x28,
    kStrokeCellBias = 0x16,
    kPangCellBias = 0x14,
    kDoublePangMode = 2,
};
// Each selected four-byte score column stores signed strokes then unsigned Pang.
// APIs receive a biased base: subtract kStrokeCellBias before selecting this view.
struct RivalScoreCell {
    signed char strokes;
    unsigned char reserved;
    unsigned short pang;
};
typedef char RivalScoreCell_size[sizeof(RivalScoreCell) == 4 ? 1 : -1];

extern "C" {

extern void __dl__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern CSharedDoc* g_sharedDoc;
extern void* g_resourceManager;
extern float kFloat_6_2831855;
extern float s_numericFontTwoPiStorage;
extern void ResourceManager_DeleteResource(void*, void*);
extern void appFreeUni__FPv(void*);
extern void __dt__9CInfoBaseFv(void*, int);
extern void TableLock_ctor(void*, int, int);
extern void RBTree_FreeTrivialSubtree(void*, void*);
extern void TableLock_ctor(void*, int, int);
extern int strncmp(const char*, const char*, unsigned int);
extern unsigned int strlen(const char*);
extern void* BpObject_GetOrCreateNamedGroup(void*, const char*);
extern void* IntegerKeyTree_FindInsertionPosition(void*, int*, int*, unsigned char*, unsigned char*);
extern void* BpEntryTree_InsertNode(void*, void*, int, int, int*);
extern void __dt__9TableLockFv(void*, int);
extern unsigned int __vt__Q212CFontNumDraw18FONT_NUM_DRAW_INFO[];
extern unsigned int __vt__12CFontNumDraw[];
extern const char fontNumDrawStrings[];
extern int s_numericTreeDefaultValueStorage;
extern void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);

struct NumericKeyNode {
    NumericKeyNode* left;
    NumericKeyNode* right;
    unsigned int headerWord08;
    int key;
    void* value;
};
// The payload at node+0xC is a key/entry pair, not a second tree node.
struct NumericEntryBinding {
    int key;
    NumericEntryConstruction* entry;
};
struct NumericEntryTable {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void setValue(int resourceNameWord, void* resourceOut, int owned);
    unsigned char padField[0x14];
    void* subtree;
    NumericKeyNode* root;
};
struct NumericResourceEntry {
    void* obj;
    unsigned char pad4[8];
    int resourceNameWord;
    unsigned char pad10[0xc];
    void* resource;
};
struct TableLock80250 {
    unsigned char bytes[8];
    // Acquire table-lock category 3, mode 1; p is an unused allocator-style tag.
    TableLock80250(void* p) {
        TableLock_ctor(this, 3, 1);
    }
    // Release the table lock on all exit paths.
    ~TableLock80250() {
        __dt__9TableLockFv(this, -1);
    }
};

// Release the numeric entry resource and heap-backed text. The vtable is set first.
FontNumericEntry::~FontNumericEntry() {
    vtable = __vt__Q212CFontNumDraw18FONT_NUM_DRAW_INFO;
    if (resource) {
        ResourceManager_DeleteResource(g_resourceManager, resource);
        resource = 0;
    }
    // The address test on this is how the original compiles the heap-text check.
    if ((unsigned int)this + 0x10 != 0 && (textFlags >> 31) != 0)
        appFreeUni__FPv(text);
}

struct FontTableTreeNode {
    FontTableTreeNode* left;
    FontTableTreeNode* right;
    unsigned int parentTag;
};
struct FontTableTree {
    void* vtable;
    unsigned char baseStorage[0x14];
    unsigned int nodeCount;
    FontTableTreeNode* root;
    FontTableTreeNode* firstNode;
    FontTableTreeNode* end;
};
struct FontEntryVirtualBase {
    unsigned int u0;
    unsigned int u1;
};
struct FontEntryVirtual : FontEntryVirtualBase {
    virtual void unk();
    virtual void destroy(int);
};
struct TableLock {
    unsigned char bytes[8];
    // Acquire the numeric table's lock category; the object argument is unused.
    TableLock(void* p) {
        TableLock_ctor(this, 3, 1);
    }
    ~TableLock();
};

#pragma push
#pragma always_inline on
// Free both descendants through the shared recursive destructor, then this node.
// The caller has already destroyed entry payloads and checked leaf != null.
inline void FreeNodeAndSubtrees(void* owner, void* leaf) {
    void* descendant = *(void**)leaf;
    if (descendant)
        RBTree_FreeTrivialSubtree(owner, descendant);
    descendant = ((FontTreeLinks80250*)leaf)->right;
    if (descendant)
        RBTree_FreeTrivialSubtree(owner, descendant);
    appFreeUni__FPv(leaf);
}
#pragma pop

#pragma push
#pragma always_inline on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Preserve the base-container teardown after derived cleanup, including its emitted address guards.
// The four cursor references keep the caller's lifetime/register allocation. Payloads are already gone.
static inline void CleanupFontTableBase(FontTableTree* self, FontTableTreeNode*& baseRoot,
                                        FontTableTreeNode*& baseChild, FontTableTreeNode*& baseBranch,
                                        FontTableTreeNode*& baseFringe) {
    if (((char*)&self->nodeCount))
        if (((char*)&self->nodeCount))
            if (((char*)&self->nodeCount))
                if (((char*)&self->nodeCount))
                    if (((char*)&self->nodeCount))
                        if (((char*)&self->nodeCount)) {
                            baseRoot = self->root;
                            if (baseRoot) {
                                baseChild = baseRoot->left;
                                if (baseChild) {
                                    baseBranch = baseChild->left;
                                    if (baseBranch) {
                                        baseFringe = baseBranch->left;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        baseFringe = baseBranch->right;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        appFreeUni__FPv(baseBranch);
                                    }
                                    baseBranch = baseChild->right;
                                    if (baseBranch) {
                                        baseFringe = baseBranch->left;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        baseFringe = baseBranch->right;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        appFreeUni__FPv(baseBranch);
                                    }
                                    appFreeUni__FPv(baseChild);
                                }
                                baseChild = baseRoot->right;
                                if (baseChild) {
                                    baseBranch = baseChild->left;
                                    if (baseBranch) {
                                        baseFringe = baseBranch->left;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        baseFringe = baseBranch->right;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        appFreeUni__FPv(baseBranch);
                                    }
                                    baseBranch = baseChild->right;
                                    if (baseBranch) {
                                        baseFringe = baseBranch->left;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        baseFringe = baseBranch->right;
                                        if (baseFringe) {
                                            FreeNodeAndSubtrees(((char*)&self->nodeCount), baseFringe);
                                        }
                                        appFreeUni__FPv(baseBranch);
                                    }
                                    appFreeUni__FPv(baseChild);
                                }
                                appFreeUni__FPv(baseRoot);
                            }
                        }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
// Destroy numeric payloads, free both tree cleanup paths, and destroy the information-table base.
// A named member, not a destructor: it returns this and keeps the base container teardown.
void* CFontNumDraw::Destroy(int shouldDelete) {
    FontTableTree* self = (FontTableTree*)this;
    FontTableTreeNode* payloadCursor;
    // These three cursors are reused at different depths between left/right
    // halves; assignments define their current subtree, not their suffix.
    FontTableTreeNode* nodeCursorA;
    FontTableTreeNode* nodeCursorB;
    FontTableTreeNode* nodeCursorC;
    FontTableTreeNode* baseRoot;
    FontTableTreeNode* baseChild;
    FontTableTreeNode* baseBranch;
    FontTableTreeNode* baseFringe;
    // Ownership summary: (1) numeric-entry payloads are destroyed through their virtual destroy(1);
    // (2) the numeric table's own nodes are freed leaf-first, then root, and the tree is reset to
    // empty; (3) the same cleanup is executed again for the CInfoBase base-container part (the
    // tree is already empty, so it frees nothing); (4) the base destructor runs and the object
    // itself is freed only when shouldDelete > 0.
    // The nested if (nodeCursor...) blocks below are the inlined, depth-limited tree free: each
    // level frees left then right child's fringe (grandchildren via FreeNodeAndSubtrees), then
    // the child, then the node; deeper subtrees are handled inside FreeNodeAndSubtrees.
    if (self) {
        self->vtable = (void*)__vt__12CFontNumDraw;
        {
            // Phase 1: destroy every numeric-entry payload in sorted traversal order.
            TableLock lock1(self);
            TableLock lock2(self);
            {
                FontTableTreeNode* nil = (FontTableTreeNode*)(((char*)&self->root));
                payloadCursor = self->firstNode;
                while (payloadCursor != nil) {
                    void* value = ((FontTreeLinks80250*)payloadCursor)->value;
                    if (value)
                        ((FontEntryVirtual*)value)->destroy(1);
                    FontTableTreeNode* right = payloadCursor->right;
                    if (right) {
                        FontTableTreeNode* child = right;
                        FontTableTreeNode* left;
                        while ((left = child->left) != 0)
                            child = left;
                        payloadCursor = child;
                    } else {
                        FontTableTreeNode* parent;
                        while (payloadCursor !=
                               (parent = (FontTableTreeNode*)((unsigned int)payloadCursor->parentTag & ~1))->left) {
                            payloadCursor = parent;
                        }
                        payloadCursor = parent;
                    }
                }
            }
            payloadCursor = self->root;
            if (payloadCursor) {
                // Phase 2: free nodes, after their payloads have been destroyed.
                // Three levels are expanded here; deeper branches recurse through
                // RBTree_FreeTrivialSubtree. This is not a fixed-depth tree.
                nodeCursorA = payloadCursor->left;
                if (nodeCursorA) {
                    nodeCursorB = nodeCursorA->left;
                    if (nodeCursorB) {
                        nodeCursorC = nodeCursorB->left;
                        if (nodeCursorC) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorC);
                        }
                        nodeCursorC = nodeCursorB->right;
                        if (nodeCursorC) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorC);
                        }
                        appFreeUni__FPv(nodeCursorB);
                    }
                    nodeCursorC = nodeCursorA->right;
                    if (nodeCursorC) {
                        nodeCursorB = nodeCursorC->left;
                        if (nodeCursorB) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorB);
                        }
                        nodeCursorB = nodeCursorC->right;
                        if (nodeCursorB) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorB);
                        }
                        appFreeUni__FPv(nodeCursorC);
                    }
                    appFreeUni__FPv(nodeCursorA);
                }
                nodeCursorC = payloadCursor->right;
                if (nodeCursorC) {
                    nodeCursorB = nodeCursorC->left;
                    if (nodeCursorB) {
                        nodeCursorA = nodeCursorB->left;
                        if (nodeCursorA) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorA);
                        }
                        nodeCursorA = nodeCursorB->right;
                        if (nodeCursorA) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorA);
                        }
                        appFreeUni__FPv(nodeCursorB);
                    }
                    nodeCursorB = nodeCursorC->right;
                    if (nodeCursorB) {
                        nodeCursorA = nodeCursorB->left;
                        if (nodeCursorA) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorA);
                        }
                        nodeCursorA = nodeCursorB->right;
                        if (nodeCursorA) {
                            FreeNodeAndSubtrees(((char*)&self->nodeCount), nodeCursorA);
                        }
                        appFreeUni__FPv(nodeCursorB);
                    }
                    appFreeUni__FPv(nodeCursorC);
                }
                appFreeUni__FPv(payloadCursor);
                self->nodeCount = 0;
                self->root = 0;
                self->firstNode = (FontTableTreeNode*)&self->root;
            }
        }
        // Phase 3: retain the base-container cleanup path (root was cleared above, so this walks
        // an empty tree). The repeated identical conditions are an inlining artifact.
        CleanupFontTableBase(self, baseRoot, baseChild, baseBranch, baseFringe);
        __dt__9CInfoBaseFv(self, 0);
        if (shouldDelete > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma pop

// Find the numeric entry by key and clear its inline or heap-backed text.
// key must exist: a failed lower_bound reaches the sentinel payload unchecked.
// Keeps the resource and text capacity; returns 1 unconditionally.
int NumericTextTable_ClearEntryText(NumericEntryTable* self, int key) {
    TableLock80250 lock(self);
    NumericKeyNode* n = self->root;
    NumericKeyNode* parent = (NumericKeyNode*)&self->root;
    while (n) {
        if (n->key >= key) {
            parent = n;
            n = n->left;
        } else
            n = n->right;
    }
    if (parent != (NumericKeyNode*)&self->root && key < parent->key)
        parent = (NumericKeyNode*)&self->root;
    unsigned char* value = (unsigned char*)parent->value;
    if ((((FontNumericEntry*)value)->textFlags >> 31) == 0) {
        value[0x11] = 0;
        value[0x10] &= 0x80;
    } else {
        *(unsigned char*)((FontNumericEntry*)value)->text = 0;
        (*(void**)&((FontNumericEntry*)value)->textLength) = 0;
    }
    return 1;
}

// Replace an entry resource using its group definition ownership flag.
// key must exist. resourceName is borrowed, stored for later rebinding, and
// forwarded to virtual setValue(name, &resource, owned); this is not a new tree key.
int UITextTable_SetEntryResourceName(NumericEntryTable* self, int key, void* resourceName) {
    TableLock80250 lock(self);
    NumericKeyNode* n = self->root;
    NumericKeyNode* parent = (NumericKeyNode*)&self->root;
    while (n) {
        if (n->key >= key) {
            parent = n;
            n = n->left;
        } else
            n = n->right;
    }
    if (parent != (NumericKeyNode*)&self->root && key < parent->key)
        parent = (NumericKeyNode*)&self->root;
    unsigned char* value = (unsigned char*)parent->value;
    ((FontNumericEntry*)value)->resourceNameWord = (int)resourceName;
    int owned = (((NumericEntryDefinition*)*(void**)value)->flags >> kOwnedResourceShift) & 1;
    if (((FontNumericEntry*)value)->resource) {
        ResourceManager_DeleteResource(g_resourceManager, ((FontNumericEntry*)value)->resource);
        ((FontNumericEntry*)value)->resource = 0;
    }
    self->setValue(((FontNumericEntry*)value)->resourceNameWord, value + kEntryResourceOffset, owned != 0);
    return 1;
}

// Delete the wrapper only for a deleting-destructor call. Named member: the class already has a destructor.
void* CComStrokeScore::Destroy(int shouldDelete) {
    if (this != 0 && shouldDelete > 0) {
        __dl__FPv(this);
    }
    return this;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Free the font table tree nodes using the original unrolled postorder cleanup.
void* FontNumDrawEntries_DestroyTree(void* ptr, int shouldDelete) {
    // Phase 1: free tree-node storage in postorder; payload cleanup belongs to
    // CFontNumDraw_dtor. Expanded levels delegate deeper branches to the helper.
    void* nodeCursorC;
    void* nodeCursorB;
    void* nodeCursorA;
    void* rootNode;
    if (ptr) {
        if (ptr)
            if (ptr)
                if (ptr)
                    if (ptr)
                        if (ptr) {
                            rootNode = ((FontTreeLinks80250*)ptr)->right;
                            if (rootNode) {
                                nodeCursorA = *(void**)rootNode;
                                if (nodeCursorA) {
                                    nodeCursorB = *(void**)nodeCursorA;
                                    if (nodeCursorB) {
                                        nodeCursorC = *(void**)nodeCursorB;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        nodeCursorC = ((FontTreeLinks80250*)nodeCursorB)->right;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        appFreeUni__FPv(nodeCursorB);
                                    }
                                    nodeCursorB = ((FontTreeLinks80250*)nodeCursorA)->right;
                                    if (nodeCursorB) {
                                        nodeCursorC = *(void**)nodeCursorB;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        nodeCursorC = ((FontTreeLinks80250*)nodeCursorB)->right;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        appFreeUni__FPv(nodeCursorB);
                                    }
                                    appFreeUni__FPv(nodeCursorA);
                                }
                                nodeCursorA = ((FontTreeLinks80250*)rootNode)->right;
                                if (nodeCursorA) {
                                    nodeCursorB = *(void**)nodeCursorA;
                                    if (nodeCursorB) {
                                        nodeCursorC = *(void**)nodeCursorB;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        nodeCursorC = ((FontTreeLinks80250*)nodeCursorB)->right;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        appFreeUni__FPv(nodeCursorB);
                                    }
                                    nodeCursorB = ((FontTreeLinks80250*)nodeCursorA)->right;
                                    if (nodeCursorB) {
                                        nodeCursorC = *(void**)nodeCursorB;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        nodeCursorC = ((FontTreeLinks80250*)nodeCursorB)->right;
                                        if (nodeCursorC) {
                                            FreeNodeAndSubtrees(ptr, nodeCursorC);
                                        }
                                        appFreeUni__FPv(nodeCursorB);
                                    }
                                    appFreeUni__FPv(nodeCursorA);
                                }
                                appFreeUni__FPv(rootNode);
                            }
                        }
        // Phase 2: only a deleting-destructor call releases the wrapper itself.
        if (shouldDelete > 0)
            __dl__FPv(ptr);
    }
    return ptr;
}
#pragma pop

extern "C" int __cntlzw(unsigned int);
#pragma push
#pragma opt_propagation off
// Sum hole Pang rewards in round order, doubling rewards for mode 2.
// index selects a four-byte column; count is the round-prefix length (0..18).
// Example: for round r (0-based) with stored hole number h = holes[r], the cell read is the
// 16-bit value at (byte*)values + index*4 + h*0x28 - 0x14; rounds in mode 2 count double.
// holes[] supplies stored hole numbers; access is base + index*4 + hole*0x28 - 0x14.
// values is consequently a biased view, not the start of a flat Pang array.
int RivalStrokeUse_GetPangThroughHole(unsigned int* values, int index, int count) {
    int rewardAmount;
    int holeByteOffset;
    unsigned int mode;
    int i;
    unsigned int* entries;
    unsigned char* base;
    unsigned char* state;
    int total = 0;
    if (count > 0) {
        unsigned int off = (unsigned int)index << 2;
        base = (unsigned char*)values + off;
        state = (unsigned char*)g_sharedDoc;
        entries = (*(unsigned int**)&((CSharedDoc*)g_sharedDoc)->golfDoc);
        for (i = 0; i < count; ++i) {
            unsigned char slot;
            unsigned int m;
            unsigned char* p;
            unsigned int doubleReward;
            slot = ((SharedRoundHoleOrder*)state)->holes[0];
            mode = ((HoleRewardModes*)entries)->modes[0];
            holeByteOffset = (unsigned int)slot * kRivalHoleStride;
            m = mode - 2;
            p = base + holeByteOffset;
            doubleReward = mode == kDoublePangMode;
            rewardAmount = ((RivalScoreCell*)(p - kStrokeCellBias))->pang;
            if (rewardAmount > 0) {
                if (doubleReward != 0) {
                    rewardAmount = rewardAmount << 1;
                }
            }
            total += rewardAmount;
            state = state + 1;
            entries = (unsigned int*)((unsigned char*)entries + 4);
        }
    }
    return total;
}
#pragma pop

// Initialize the numeric-font angle constant at startup.
void CFontNumDraw_InitTwoPiConstant() {
    s_numericFontTwoPiStorage = kFloat_6_2831855;
}

// Sum signed stroke counts in the selected round hole order.
// Same column/round-prefix contract as Pang; signed byte cell is at hole*0x28 - 0x16.
int RivalStrokeUse_GetStrokesThroughHole(unsigned char* base, int index, int count) {
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += ((RivalScoreCell*)(base + index * 4 + ((SharedRoundHoleOrder*)g_sharedDoc)->holes[i] * kRivalHoleStride -
                                  kStrokeCellBias))
                   ->strokes;
    }
    return sum;
}

#pragma push
#pragma use_lmw_stmw on
// Create a numeric text entry for an R_NUM_ definition and insert it into the key tree.
// definition and data (the BP group provider) remain borrowed. Name must match
// R_NUM_ and allocation/group lookup must succeed: node is dereferenced unchecked.
// *out receives the table-owned entry; return value is the definition's lookup key.
int CFontNumDraw::CreateNumericEntry(NumericEntryDefinition* definition, void* data, NumericEntryConstruction** out) {
    NumericEntryTable* self = (NumericEntryTable*)this;
    TableLock80250 lock(self);
    int groupNameAddress = (int)definition->name;
    NumericEntryConstruction* node = 0;
    if (strncmp((const char*)groupNameAddress, fontNumDrawStrings, strlen(fontNumDrawStrings)) == 0) {
        NumericEntryConstruction* tmp = (NumericEntryConstruction*)__nw__FUl(0x24);
        if (tmp) {
            tmp->definition = 0;
            tmp->group = 0;
            tmp->vtable = __vt__Q212CFontNumDraw18FONT_NUM_DRAW_INFO;
            tmp->resourceNameWord = 0;
            tmp->textStorageWord = 0;
            *(int*)&tmp->textLength = 0;
            *(int*)&tmp->text = 0;
            *(int*)&tmp->resource = 0;
            tmp->formatState = 0;
        }
        node = tmp;
    }
    // Phase 1 complete: entry storage exists; bind borrowed definition/group references.
    node->group = BpObject_GetOrCreateNamedGroup(data, (const char*)groupNameAddress);
    node->definition = definition;
    *out = node;
    int value = definition->key;
    NumericKeyNode* parent;
    unsigned char b, a;
    // Phase 2: insert a missing tree key or replace its payload pointer at an existing key.
    void* tree = IntegerKeyTree_FindInsertionPosition(&self->subtree, &value, (int*)&parent, &a, &b);
    NumericEntryBinding* slot;
    if (!parent || *(int*)((char*)parent + kTreePayloadOffset) < value) {
        int old[2];
        old[0] = value;
        old[1] = s_numericTreeDefaultValueStorage;
        slot = (NumericEntryBinding*)((char*)BpEntryTree_InsertNode((char*)&self->subtree, tree, a, b, old) + 0xc);
    } else
        slot = (NumericEntryBinding*)((char*)parent + kTreePayloadOffset);
    slot->entry = node;
    return value;
}
#pragma pop

#pragma opt_common_subs off
struct NodeBase80250 {
    unsigned char pad[8];
};
struct NodeVirtual80250 : NodeBase80250 {
    virtual int test();
};
// Query the existing entry hook that gates resource binding; the predicate's internal meaning is unknown.
static inline int EntryRequestsResourceBinding(NumericEntryConstruction* entry) {
    return ((NodeVirtual80250*)entry)->test();
}
#pragma push
#pragma use_lmw_stmw on
// Create entries from the definition array and bind their resource values.
// resourceName is the font resource's name pointer, stored as a word. Returns 1;
// definitions and provider must outlive the entries, and keys should be unique.
int CFontNumDraw::RegisterEntries(const void* definitions, int count, void* groupProvider, const void* resourceName) {
    NumericEntryTable* self = (NumericEntryTable*)this;
    TableLock80250 outer(self);
    unsigned char* cursor = (unsigned char*)definitions;
    unsigned int owned;
    NumericResourceEntry* entry = 0;
    for (int i = 0; i < count; cursor += 0xc, ++i) {
        NumericEntryConstruction* node;
        int lookupKey = CreateNumericEntry((NumericEntryDefinition*)cursor, groupProvider, &node);
        // Slot zero of the entry's +8 interface gates binding. Its underlying
        // predicate meaning is unverified; false leaves the new entry unbound.
        if (EntryRequestsResourceBinding(node)) {
            TableLock80250 inner(self);
            NumericKeyNode* n = self->root;
            NumericKeyNode* parent = (NumericKeyNode*)&self->root;
            while (n) {
                if (n->key >= lookupKey) {
                    parent = n;
                    n = n->left;
                } else
                    n = n->right;
            }
            if (parent == (NumericKeyNode*)&self->root || lookupKey < parent->key)
                parent = (NumericKeyNode*)&self->root;
            entry = (NumericResourceEntry*)parent->value;
            entry->resourceNameWord = (int)resourceName;
            void* old = entry->resource;
            owned = (((NumericEntryDefinition*)entry->obj)->flags >> kOwnedResourceShift) & 1;
            if (old) {
                ResourceManager_DeleteResource(g_resourceManager, old);
                entry->resource = 0;
            }
            self->setValue(entry->resourceNameWord, &entry->resource, owned != 0);
        }
    }
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Visual formatting contract, in this order:
// - Leading character: preserve with bit 0x4 in every mode; otherwise mode 0
//   preserves with 0x80, mode 1 with 0x40, mode 2 always strips. Other modes
//   leave it alone. The stripped character need not be a sign.
// - 0x100 inserts the pooled separator every three positions left of the
//   decimal point (or string end); it counts characters, not parsed digits.
// - 0x200 appends the pooled percent suffix without scaling the value.
// Example: mode 2 turns a prefixed "X1234.5" into "1234.5" before grouping.
// Apply leading-character rules, thousands separators, and percent suffix from definition flags.
void UITextTable_ApplyNumericTextFormatFlags(void* self, unsigned char* entryBytes, int leadingCharacterMode) {
    // allocScratch*/allocTag* are never-written scratch bytes (volatile) copied into the by-value
    // allocator argument of String_ReplaceRange; they only keep the original stack layout and carry no data.
    NumericEntryDefinition* definition;
    int removeLeadingCharacter;
    volatile unsigned char allocScratchA;
    unsigned char allocTagA;
    int heapBackedText;
    unsigned int textLength;
    int idx;
    unsigned char* textBytes;
    unsigned int decimalPoint;
    unsigned int lastCharacter;
    unsigned char* scanCharacter;
    const char* insertedText;
    int insertionPosition;
    int digitsSinceSeparator;
    volatile unsigned char allocScratchSrcB;
    unsigned char allocScratchB;
    unsigned char allocTagB;
    unsigned int len;
    volatile unsigned char allocScratchC;
    unsigned char allocTagC;

    // Phase 1: optionally remove the first character; the flag names describe
    // preservation, and String_ReplaceRange receives an empty replacement range.
    definition = *(NumericEntryDefinition**)entryBytes;
    if ((definition->flags & kKeepLeadingCharacter) == 0) {
        removeLeadingCharacter = 0;
        switch (leadingCharacterMode) {
        case kFormatModeZero:
            if ((definition->flags & kKeepInModeZero) == 0)
                removeLeadingCharacter = 1;
            break;
        case kFormatModeOne:
            if ((definition->flags & kKeepInModeOne) == 0)
                removeLeadingCharacter = 1;
            break;
        case kStripUnlessExplicitlyKept:
            removeLeadingCharacter = 1;
            break;
        }
        if (removeLeadingCharacter) {
            allocTagA = allocScratchA;
            String_ReplaceRange(entryBytes + kEntryTextOffset, 0, 1, 0, 0, (const char*)&allocTagA);
        }
    }
    // Phase 2: locate the decimal point and insert separators to its left, right to left.
    // Existing text is assumed numeric; this does not validate signs or digit characters.
    if (definition->flags & kGroupThousands) {
        heapBackedText = ((FontNumericEntry*)entryBytes)->textFlags >> 31;
        idx = -1;
        if (!heapBackedText) {
            textLength = entryBytes[kEntryInlineLengthByte] & 0x7f;
            textBytes = entryBytes + kEntryInlineTextByte;
        } else {
            textBytes = (*(unsigned char**)&((FontNumericEntry*)entryBytes)->text);
            textLength = ((FontNumericEntry*)entryBytes)->textLength;
        }
        if (textLength != 0) {
            lastCharacter = textLength - 1;
            if (lastCharacter < 0xFFFFFFFFu)
                idx = (int)lastCharacter;
            scanCharacter = textBytes + idx;
            while (1) {
                if (*scanCharacter == '.') {
                    decimalPoint = (unsigned int)(scanCharacter - textBytes);
                    break;
                }
                if (scanCharacter > textBytes)
                    --scanCharacter;
                else {
                    decimalPoint = 0xFFFFFFFFu;
                    break;
                }
            }
        } else {
            decimalPoint = 0xFFFFFFFFu;
        }
        if (decimalPoint == 0xFFFFFFFFu) {
            if (!heapBackedText)
                insertionPosition = (int)(entryBytes[kEntryInlineLengthByte] & 0x7f) - 1;
            else
                insertionPosition = (int)((FontNumericEntry*)entryBytes)->textLength - 1;
        } else {
            insertionPosition = (int)decimalPoint - 1;
        }
        insertedText = fontNumDrawStrings + kThousandsSeparator;
        digitsSinceSeparator = 0;
        allocScratchB = allocScratchSrcB;
        for (; insertionPosition > 0; --insertionPosition) {
            if (digitsSinceSeparator >= 2) {
                allocTagB = allocScratchB;
                len = strlen(insertedText);
                String_ReplaceRange(entryBytes + kEntryTextOffset, insertionPosition, 0, insertedText,
                                    insertedText + len, (const char*)&allocTagB);
                digitsSinceSeparator = 0;
            } else
                ++digitsSinceSeparator;
        }
    }
    // Phase 3: append the suffix after grouping; repeated formatting can append it again.
    if (definition->flags & kAppendPercent) {
        insertedText = fontNumDrawStrings + kPercentSuffix;
        if (((((FontNumericEntry*)entryBytes)->textFlags) >> 31) == 0) {
            textLength = entryBytes[kEntryInlineLengthByte] & 0x7f;
        } else {
            textLength = ((FontNumericEntry*)entryBytes)->textLength;
        }
        allocTagC = allocScratchC;
        len = strlen(insertedText);
        String_ReplaceRange(entryBytes + kEntryTextOffset, textLength, 0, insertedText, insertedText + len,
                            (const char*)&allocTagC);
    }
}
#pragma pop

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804622E0", _ctors_u80250D60_0, CFontNumDraw_InitTwoPiConstant);
#pragma pop
