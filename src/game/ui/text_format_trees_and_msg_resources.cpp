// Formatted text: the tree and string storage behind messages with sound tags and font markup,
// plus the stream, locale and message-resource classes those messages are built on.

#include <decomp/static_init.h>
#include "classes/CSharedDoc.h"
#include "classes/CMsgEcbResource.h"
#include "classes/CMsgDrawSoundTagMap.h"

extern "C" {

// Small-string payload and locale table views used by the formatting helper.
struct FormatStringView {
    unsigned int flags;
    unsigned int length;
    void* data;
};
struct LocaleFacetTable {
    void** entries;
    unsigned int count;
};
struct IOSAdjustment {
    char prefix[0x3c];
    int virtualBaseOffset;
};
struct IOStreamBases {
    void* outputAdjustment;
    char outputState[8];
    void* inputAdjustment;
    char inputState[4];
    char sharedIos[1];
};
struct InputStreamBase {
    char inputState[0x0c];
    char ios[1];
};
struct StringBufferMembers {
    char streamState[0x1c];
    unsigned int localeFlags;
    void* localeReference;
    char bufferState[8];
    FormatStringView string;
};
typedef char IOStreamBases_ios_offset[((int)&((IOStreamBases*)0)->sharedIos == 0x14) ? 1 : -1];
typedef char StringBuffer_string_offset[((int)&((StringBufferMembers*)0)->string == 0x2c) ? 1 : -1];
typedef char StringBuffer_locale_offset[((int)&((StringBufferMembers*)0)->localeReference == 0x20) ? 1 : -1];

void __vt__15CMsgEcbResource();
extern float kFloat_6_2831855;
extern float s_textFormattingTwoPi;
extern CSharedDoc* g_sharedDoc;
extern int g_currentSharedContext;
void Count_SelectedInventoryItemOccurrences(void*, int);
void std_ios_base_dtor(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
void appFreeUni__FPv(void*);
void MWSharedOwner_ReleaseReference(void*);
void __unexpected() __attribute__((noreturn));
void* appMallocDummy__FUl(unsigned int);
void RBTree_RebalanceAfterInsert(void*, int, int);
void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
void* __nwa__FUl(unsigned int);
void* __nw__FUl(unsigned int);
void __throw(char*, void*, void*);
#pragma does_not_return __throw
void __dt__Q23std12length_errorFv(void*, int);
unsigned int strlen(const char*);
char* strcpy(char*, const char*);
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
struct ScriptAliasStringPool {
    char vectorLengthError[20];   // 0
    char lengthErrorTypeName[55]; // 20
    char endCommand[4];           // 75
    char testAliasCommand[11];    // 79
    char treeLengthError[26];     // 90
    char outOfRange[27];          // 116
    char outOfRangeTypeName[55];  // 143
    char stringLengthError[27];   // 198
    char reserveLengthError[35];  // 225
};
extern ScriptAliasStringPool scriptAliasExceptionStrings;
void __dl__FPv(void*);
void __dt__Q33std6detail16char_array_guardFv(void*);

struct AE594Node {
    AE594Node* left;
    AE594Node* right;
    int parentLink;
    int key;
};

struct TreeHead {
    int count;
    void* root;
    void* firstNode;
    int pad;
};
struct TreeLink {
    TreeLink* left;
    TreeLink* right;
    TreeLink* parent;
    int color;
    int key;
};
struct Pair {
    void* first;
    void* second;
};

struct TextFormatTreeNode {
    TextFormatTreeNode* left;
    TextFormatTreeNode* right;
    int parentLink;
    int key;
    unsigned int flags;
    int stringLength;
    void* owned;
};
struct TextFormatTreeValue;
void TextFormatTree_DestroyStringValue(void*, TextFormatTreeValue*);

void TextFormatTree_FreeStringSubtree(void*, TextFormatTreeNode*);

// Finish a non-null leaf of the expanded traversal; deeper descendants use the recursive entry point.
static inline void FreeTextFormatLeaf(void* allocator, TextFormatTreeNode* leaf) {
    if (leaf->left)
        TextFormatTree_FreeStringSubtree(allocator, leaf->left);
    if (leaf->right)
        TextFormatTree_FreeStringSubtree(allocator, leaf->right);
    TextFormatTree_DestroyStringValue(allocator, (TextFormatTreeValue*)((char*)&leaf->key));
    appFreeUni__FPv(leaf);
}

// Destroy string-owning tree nodes in postorder; only heap-backed string values release memory.
// The top traversal levels remain expanded; the shared leaf helper states their ownership rule.
void TextFormatTree_FreeStringSubtree(void* allocator, TextFormatTreeNode* root) {
    // Postorder ownership invariant: finish left and right descendants before freeing
    // the node's heap-tagged string and the node itself. Deeper branches recurse;
    // the first levels are expanded. allocator is forwarded as the owning tree context.
    TextFormatTreeNode* firstLeft = root->left;
    TextFormatTreeNode* node = firstLeft;
    if (node) {
        TextFormatTreeNode* child = node->left;
        if (0 != child) {
            TextFormatTreeNode* leftChild = child->left;
            TextFormatTreeNode* leaf = leftChild;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            TextFormatTreeNode* rightChild = child->right;
            leaf = rightChild;
            if (leaf) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            if (((char*)&child->key) != 0 && ((char*)&child->flags) != 0 && (child->flags >> 31) != 0)
                appFreeUni__FPv(child->owned);
            appFreeUni__FPv(child);
        }
        TextFormatTreeNode* rightBranch = node->right;
        child = rightBranch;
        if (child != 0) {
            TextFormatTreeNode* leftChild = child->left;
            TextFormatTreeNode* leaf = leftChild;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            TextFormatTreeNode* rightChild = child->right;
            leaf = rightChild;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            if (((char*)&child->key) != 0 && ((char*)&child->flags) != 0 && (child->flags >> 31) != 0)
                appFreeUni__FPv(child->owned);
            appFreeUni__FPv(child);
        }
        if (((char*)&node->key) != 0 && ((char*)&node->flags) != 0 && (node->flags >> 31) != 0)
            appFreeUni__FPv(node->owned);
        appFreeUni__FPv(node);
    }
    TextFormatTreeNode* firstRight = root->right;
    node = firstRight;
    if (0 != node) {
        TextFormatTreeNode* leftBranch = node->left;
        TextFormatTreeNode* child = leftBranch;
        if (child != 0) {
            TextFormatTreeNode* leaf = child->left;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            TextFormatTreeNode* rightChild = child->right;
            leaf = rightChild;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            if (((char*)&child->key) != 0 && ((char*)&child->flags) != 0 && (child->flags >> 31) != 0)
                appFreeUni__FPv(child->owned);
            appFreeUni__FPv(child);
        }
        child = node->right;
        if (0 != child) {
            TextFormatTreeNode* leaf = child->left;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            leaf = child->right;
            if (leaf != 0) {
                FreeTextFormatLeaf(allocator, leaf);
            }
            if (((char*)&child->key) != 0 && ((char*)&child->flags) != 0 && (child->flags >> 31) != 0)
                appFreeUni__FPv(child->owned);
            appFreeUni__FPv(child);
        }
        if (((char*)&node->key) != 0 && ((char*)&node->flags) != 0 && (node->flags >> 31) != 0)
            appFreeUni__FPv(node->owned);
        appFreeUni__FPv(node);
    }
    if (((char*)&root->key) != 0 && ((char*)&root->flags) != 0 && (root->flags >> 31) != 0)
        appFreeUni__FPv(root->owned);
    appFreeUni__FPv(root);
}

// Free a trivial-value tree in postorder, with the original three-level traversal unrolling.
void RBTree_FreeTrivialSubtree(void* allocator, Pair* pair) {
    // Trivial payload specialization: destroy child allocations before their parent;
    // no string or pointed-to object is owned by the payload here.
    Pair* a;
    Pair* b;
    Pair* c;
    a = (Pair*)pair->first;
    if (a) {
        b = (Pair*)a->first;
        if (b) {
            c = (Pair*)b->first;
            if (c) {
                if (c->first)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                if (c->second)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                appFreeUni__FPv(c);
            }
            c = (Pair*)b->second;
            if (c) {
                if (c->first)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                if (c->second)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                appFreeUni__FPv(c);
            }
            appFreeUni__FPv(b);
        }
        b = (Pair*)a->second;
        if (b) {
            c = (Pair*)b->first;
            if (c) {
                if (c->first)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                if (c->second)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                appFreeUni__FPv(c);
            }
            c = (Pair*)b->second;
            if (c) {
                if (c->first)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                if (c->second)
                    RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                appFreeUni__FPv(c);
            }
            appFreeUni__FPv(b);
        }
        appFreeUni__FPv(a);
    }
    {
        Pair* a = (Pair*)pair->second;
        if (a) {
            Pair* b = (Pair*)a->first;
            if (b) {
                Pair* c = (Pair*)b->first;
                if (c) {
                    if (c->first)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                    if (c->second)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                    appFreeUni__FPv(c);
                }
                c = (Pair*)b->second;
                if (c) {
                    if (c->first)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                    if (c->second)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            b = (Pair*)a->second;
            if (b) {
                Pair* c = (Pair*)b->first;
                if (c) {
                    if (c->first)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                    if (c->second)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                    appFreeUni__FPv(c);
                }
                c = (Pair*)b->second;
                if (c) {
                    if (c->first)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->first);
                    if (c->second)
                        RBTree_FreeTrivialSubtree(allocator, (Pair*)c->second);
                    appFreeUni__FPv(c);
                }
                appFreeUni__FPv(b);
            }
            appFreeUni__FPv(a);
        }
    }
    appFreeUni__FPv(pair);
}

struct SmallNode {
    unsigned char b[0x14];
};
struct LargeNode {
    unsigned char b[0x1c];
};
struct LocalString {
    void* data;
    unsigned char flags[4];
};
struct LocalIter {
    void* first;
    void* last;
};

struct PairTreeNode {
    void* left;
    void* right;
    PairTreeNode* parent;
    void* first;
    void* second;
};
// Free an uncommitted two-pointer-tree node on exceptional exit.
struct PairTreeNodeGuard {
    void* a;
    PairTreeNode* volatile p;
    ~PairTreeNodeGuard() {
        void* q = p;
        if (q)
            appFreeUni__FPv(q);
    }
};
struct AEC14Msg {
    void* vtable;
    int refs;
    int count;
    char* buf;
};
struct AEC14Exc {
    void* vtbl;
    char* volatile buf;
    AEC14Msg* ref;
    int pad;
};
void __dla__FPv(void*);
// Free diagnostic character storage until its shared owner takes responsibility.
struct AEC14Str {
    void* buf;
    void* ref;
    ~AEC14Str() {
        if (buf)
            __dla__FPv(buf);
    }
};

// Copy a two-pointer tree value while preserving its exception and volatile-store boundaries.
inline void AEC14SetPair(Pair* slot, Pair* value) {
    void* first = value->first;
    void* second = value->second;
    *(void* volatile*)&slot->first = first;
    try {
        slot->second = second;
    } catch (...) {
        throw;
    }
}

// Assign the parent link through the original exception boundary.
inline void AEC14SetParent(PairTreeNode** p, Pair* w) {
    try {
        *p = (PairTreeNode*)w;
    } catch (...) {
        throw;
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Allocate a BP-entry node, attach it on the chosen side, and rebalance the tree.
void* BpEntryTree_InsertNode(TreeHead* self, Pair* where, int left, int add, Pair* value) {
    volatile unsigned char tagC __attribute__((aligned(4)));
    PairTreeNodeGuard guard;
    // Size check: throws std::length_error("tree::insert length error") when the map is full (library code; the
    // multi-step exception construction below is the inlined std::length_error constructor).
    if ((unsigned int)(self->count + 0x10000) == 0xffffu) {
        const char* msg = (const char*)scriptAliasExceptionStrings.treeLengthError;
        AEC14Exc exc;
        AEC14Exc* ep = &exc;
        exc.vtbl = __vt__Q23std11logic_error;
        volatile unsigned char tagB __attribute__((aligned(4))) = 0;
        volatile unsigned char tagA __attribute__((aligned(4))) = 0;
        unsigned int len = strlen(msg);
        char* buf = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
        exc.buf = buf;
        {
            AEC14Str tmp;
            tmp.buf = buf;
            tmp.ref = (void*)&tagA;
            AEC14Msg* _m = (AEC14Msg*)__nw__FUl(0x10);
            if (_m) {
                _m->refs = 1;
                _m->count = 1;
                _m->vtable = s_sharedCharDeleterVtable;
                _m->buf = buf;
            }
            exc.ref = _m;
            tmp.buf = 0;
        }
        strcpy(exc.buf, msg);
        ep->vtbl = __vt__Q23std12length_error;
        __throw((char*)(scriptAliasExceptionStrings.lengthErrorTypeName), ep, (void*)__dt__Q23std12length_errorFv);
    }
    tagC = 0;
    PairTreeNode* node = (PairTreeNode*)appMallocDummy__FUl(0x14);
    PairTreeNode* volatile allocA;
    PairTreeNode* volatile allocB;
    allocB = node;
    PairTreeNode* saved = node;
    allocA = node;
    guard.a = (char*)&self->root;
    guard.p = node;
    Pair* slot = (Pair*)&node->first;
    if (slot != 0) {
        AEC14SetPair(slot, value);
    }
    node->right = 0;
    guard.p = 0;
    node->left = 0;
    PairTreeNode** pslot = &node->parent;
    if (pslot != 0) {
        try {
            *pslot = (PairTreeNode*)where;
        } catch (...) {
            throw;
        }
    }
    if (left)
        where->first = saved;
    else
        where->second = saved;
    int old = self->count;
    self->count = old + 1;
    RBTree_RebalanceAfterInsert(saved, (int)self->root, old);
    if (add)
        self->firstNode = saved;
    return saved;
}
#pragma pop

// Formatted-text map storage (the AE5F4Guard/AE5F4Str types keep their address names because their destructor symbols embed them):
// In plain C: this is std::map<int glyphIndex, small-string soundTag>::insert for a node whose position was already
// found. Over the size limit it throws std::length_error("tree::insert length error"), built inline.
// header = count, root, opaque tree word, first/end iterator; nodes start with
// left/right/parent links and the payload begins at +0x0c. Payload = integer glyph
// index followed by a 12-byte narrow small string (sound tag). Cross-unit evidence:
// CMsgDraw_GetOrInsertFormattedTextEntry in msgdraw_text_rendering.cpp passes an int key; its callers
// assign defaultRevealSoundStorage or a parsed soundNameToken at the current glyph.
// The retained void* first field transports key bits; it does not own an object.
// flags bit 31 selects heap ownership: length/data are meaningful only in that mode;
// otherwise bytes at &flags+1 (including the length/data words) are INLINE characters.
// AE5F4Guard owns only the unlinked node allocation; string-copy calls keep their existing unwind boundaries.
struct SoundTagMapValue {
    void* glyphIndexWord;
    unsigned int flags;
    unsigned int length;
    char* data;
};
struct SoundTagMapNode {
    SoundTagMapNode* left;
    SoundTagMapNode* right;
    SoundTagMapNode* parent;
    void* glyphIndexWord;
    unsigned int flags;
    unsigned int length;
    char* data;
};
// Free an uncommitted formatted-text-map node on exceptional exit.
struct AE5F4Guard {
    void* a;
    SoundTagMapNode* volatile p;
    ~AE5F4Guard() {
        void* q = p;
        if (q)
            appFreeUni__FPv(q);
    }
};
// Initialize shared diagnostic ownership with one reference in each counter.
struct SharedMessageBlock {
    void* vtable;
    int refs;
    int count;
    char* buf;
    SharedMessageBlock(char* b) throw() {
        refs = 1;
        count = 1;
        vtable = s_sharedCharDeleterVtable;
        buf = b;
    }
};
struct SoundTagMapAllocs {
    SoundTagMapNode* volatile a;
    SoundTagMapNode* volatile b;
};
struct LengthErrorStorage {
    void* vtbl;
    char* volatile buf;
    SharedMessageBlock* ref;
    int pad;
};
struct AE5F4Str {
    void* volatile buf;
    void* volatile ref;
    // Free uncommitted diagnostic character storage.
    ~AE5F4Str() {
        if (buf)
            __dla__FPv(buf);
    }
};

// Assign a formatted-text node parent through the original exception boundary.
inline void SoundTagNode_SetParent(SoundTagMapNode** parentSlot, SoundTagMapNode* parentNode) {
    try {
        *parentSlot = parentNode;
    } catch (...) {
        throw;
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Insert a formatted-text value, copying inline strings or allocating heap strings as required.
void* CMsgDrawSoundTagMap::InsertNode(SoundTagMapNode* where, int left, int add, SoundTagMapValue* value) {
    volatile unsigned char tagC __attribute__((aligned(4)));
    if ((unsigned int)(count + 0x10000) == 0xffffu) {
        const char* msg = (const char*)scriptAliasExceptionStrings.treeLengthError;
        LengthErrorStorage exc;
        LengthErrorStorage* ep = &exc;
        exc.vtbl = __vt__Q23std11logic_error;
        volatile unsigned char tagB __attribute__((aligned(4))) = 0;
        volatile unsigned char tagA __attribute__((aligned(4))) = 0;
        unsigned int len = strlen(msg);
        char* buf = (char*)__nwa__FUl((len + 1) * 2 - (len + 1));
        AE5F4Str tmp;
        exc.buf = buf;
        tmp.buf = buf;
        tmp.ref = (void*)&tagA;
        SharedMessageBlock* _m = (SharedMessageBlock*)__nw__FUl(0x10);
        if (_m) {
            _m->refs = 1;
            _m->count = 1;
            _m->vtable = s_sharedCharDeleterVtable;
            _m->buf = buf;
        }
        exc.ref = _m;
        tmp.buf = 0;
        strcpy(exc.buf, msg);
        ep->vtbl = __vt__Q23std12length_error;
        __throw((char*)(scriptAliasExceptionStrings.lengthErrorTypeName), ep, (void*)__dt__Q23std12length_errorFv);
    }
    tagC = 0;
    // 0x1c = sizeof(SoundTagMapNode): left/right/parent links + {glyph index, 12-byte string}.
    // Phase 1: allocate an unlinked node under the raw-allocation guard.
    SoundTagMapNode* alloc = (SoundTagMapNode*)appMallocDummy__FUl(0x1c);
    AE5F4Guard guard;
    SoundTagMapNode* volatile allocA;
    SoundTagMapNode* volatile allocB;
    allocB = alloc;
    allocA = alloc;
    guard.a = (char*)&root;
    guard.p = alloc;
    // Phase 2: copy the key and inline bytes, or deep-copy an owned heap string.
    SoundTagMapValue* slot = (SoundTagMapValue*)&alloc->glyphIndexWord;
    if (slot != 0) {
        unsigned int flags = value->flags;
        void* first = value->glyphIndexWord;
        try {
            slot->glyphIndexWord = first;
            if ((flags >> 31) == 0) {
                slot->flags = flags;
                char* data;
                unsigned int length = value->length;
                data = value->data;
                slot->length = length;
                slot->data = data;
            } else {
                slot->flags = 0;
                unsigned int length = value->length;
                slot->length = 0;
                slot->data = 0;
                String_ReserveCapacity(&slot->flags, (int)length);
                volatile unsigned char raw __attribute__((aligned(4)));
                unsigned char flag = raw;
                String_ReplaceRange(&slot->flags, 0, 0, value->data, (char*)value->data + value->length, &flag);
            }
        } catch (...) {
            throw;
        }
    }
    // Phase 3: release the guard, link under where, update count, then rebalance.
    // left selects the parent link; add requests an update of the first-node cache.
    SoundTagMapNode* node;
    node = guard.p;
    node->right = 0;
    guard.p = 0;
    node->left = 0;
    SoundTagMapNode** pslot = &node->parent;
    if (pslot != 0) {
        SoundTagNode_SetParent(pslot, where);
    }
    if (left)
        where->left = node;
    else
        where->right = node;
    int old = count;
    count = old + 1;
    RBTree_RebalanceAfterInsert(node, (int)root, old);
    if (add)
        firstNode = node;
    return node;
}
#pragma opt_propagation reset
#pragma pop

// Find an insertion parent and predecessor while returning side and uniqueness hints.
void* IntegerKeyTree_FindInsertionPosition(AE594Node* self, int* key, AE594Node** out, unsigned char* flagA,
                                           unsigned char* flagB) {
    *out = 0;
    AE594Node* parent = (AE594Node*)&self->right;
    AE594Node* node = self->right;
    *flagA = 1;
    *flagB = 1;
    while (node != 0) {
        parent = node;
        if (*key < node->key) {
            node = node->left;
            *flagA = 1;
        } else {
            *out = node;
            node = node->right;
            *flagA = 0;
            *flagB = 0;
        }
    }
    return parent;
}

struct OwnedPointerPair {
    void* first;
    void* owned;
};

// Free the owned second pointer, then optionally delete the wrapper.
void* dtor_800AF068(void* self, int deleting) {
    if (self != 0) {
        void* p = ((OwnedPointerPair*)self)->owned;
        if (p != 0)
            appFreeUni__FPv(p);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Restore the stream virtual-base adjustment, destroy ios when requested, and optionally delete.
void* OStream_dtor(void* self, int deleting) {
    if (self != 0) {
        void* p = *(void**)self;
        void* q = (char*)self + 8;
        ((IOSAdjustment*)p)->virtualBaseOffset = (char*)q - (char*)p;
        if (deleting != 0 && q != 0)
            std_ios_base_dtor(q, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma opt_common_subs off
// Restore both stream base adjustments before destroying the shared ios virtual base.
void* IOStream_dtor(void* self, int deleting) {
    if (self != 0) {
        void* p = *(void**)self;
        ((IOSAdjustment*)p)->virtualBaseOffset = ((IOStreamBases*)self)->sharedIos - (char*)p;
        if (&((volatile IOStreamBases*)self)->inputAdjustment != 0) {
            void* t = ((volatile IOStreamBases*)self)->inputAdjustment;
            ((IOSAdjustment*)t)->virtualBaseOffset = ((IOStreamBases*)self)->sharedIos - (char*)t;
        }
        if (self != 0) {
            p = *(void**)self;
            ((IOSAdjustment*)p)->virtualBaseOffset = (char*)&((IOStreamBases*)self)->inputAdjustment - (char*)p;
        }
        if (deleting != 0 && ((IOStreamBases*)self)->sharedIos != 0)
            std_ios_base_dtor(((IOStreamBases*)self)->sharedIos, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset

// Restore the input-stream virtual-base adjustment before ios cleanup.
void* IStream_DtorThunk(void* self, int deleting) {
    if (self != 0) {
        void* first = *(void**)self;
        void* end = ((InputStreamBase*)self)->ios;
        ((IOSAdjustment*)first)->virtualBaseOffset = (char*)end - (char*)first;
        if (deleting != 0 && end != 0)
            std_ios_base_dtor(end, 0);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Preserve the original empty exception specification and its runtime catch record.
static inline void ReleaseStringBufferLocale(void* locale) throw() {
    MWSharedOwner_ReleaseReference(locale);
}

// Release a heap-backed string and locale reference, then optionally delete the stream buffer.
void* MWStringBuffer_DestroyMembers(void* self, short deleting) {
    if (self != 0) {
        if (&((StringBufferMembers*)self)->string && (((volatile StringBufferMembers*)self)->string.flags >> 31) != 0)
            appFreeUni__FPv(((StringBufferMembers*)self)->string.data);
        if (self != 0 && &((StringBufferMembers*)self)->localeFlags != 0 &&
            &((StringBufferMembers*)self)->localeFlags != 0) {
            void* ptr = ((StringBufferMembers*)self)->localeReference;
            if (ptr != 0) {
                ReleaseStringBufferLocale(ptr);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct CMsgEcbResourceView {
    void* vtable;
    int initialStateWords[3]; // Only zero initialization is established for this unit.
};

// Install the message ECB resource vtable and clear its three initial state words.
CMsgEcbResource::CMsgEcbResource() {
    CMsgEcbResourceView* self = (CMsgEcbResourceView*)this;
    int zero = 0;
    self->initialStateWords[0] = zero;
    self->vtable = (void*)&__vt__15CMsgEcbResource;
    self->initialStateWords[1] = zero;
    self->initialStateWords[2] = zero;
}

struct TextFormatTreeValue {
    int key;
    unsigned int flags;
    int stringLength;
    void* ptr;
};

// Free a formatted-text value only when its small-string tag denotes heap storage.
void TextFormatTree_DestroyStringValue(void* unused, TextFormatTreeValue* self) {
    (void)unused;
    if (self != 0 && (unsigned int)((unsigned long)self + 4) != 0 && (self->flags >> 31) != 0) {
        appFreeUni__FPv(self->ptr);
    }
}

// Destroy the ios base and optionally free its allocation.
void* dtor_800AF548(void* self, int deleting) {
    if (self != 0) {
        std_ios_base_dtor(self, 0);
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize this unit's stored full-turn angle constant.
void TextFormatting_InitTwoPi() {
    s_textFormattingTwoPi = kFloat_6_2831855;
}

struct AF0FCString {
    unsigned char data[0x20];
};
struct AF0FCTree {
    unsigned char data[0x4c];
};
void MWCtypeFacet_Construct(void*, int, int, int);
struct AF0FCObj {
    void* vtable;
    unsigned char data[0x14];
    // Construct the runtime character-classification facet used by numeric formatting.
    AF0FCObj(int a, int b, int c) {
        MWCtypeFacet_Construct(this, a, b, c);
    }
};
extern void* s_localeFacetIdCounter;
extern void* s_ctypeFacetId;
extern void* g_formattedByteToWideGlyphTable;
extern void MWIOS_InitializeStreamBuffer(void*, void*); /* (ios, streambuf): reads r4 */
extern void MWCtypeFacet_Construct(void*, int, int, int);
extern void MWLocale_InstallFacet(void*, void*, unsigned int);
extern void* MWLocale_GetDefault();
extern void MWStream_CopyLocale(void*, void*);
extern void MWOStream_InsertSigned32(void*, void*);
extern void MWStringStream_CopyBufferString(void*, void*);
extern int WString_ReplaceRange(void*, int, int, void*, void*, void*);
extern void LocaleRef_dtor(void*);
extern void StringStream_dtor(void*, short);

extern void* __nw__FUl(unsigned int);
extern char iosBaseVtable[];
extern char iStreamVtableGroup[];
extern char s_basicOstreamVtable[];
extern char outputStringStreamVtables[];
extern char s_appStringstreamVTables[];
extern char stringStreamBufferVtable[];
extern char s_appStringbufVTable[];

// Write the offset locating the shared ios virtual base from the current stream base.
inline void AF0FCRelocate(char* base, char* target) {
    ((IOSAdjustment*)base)->virtualBaseOffset = target - base;
}
// Release a locale reference through the runtime's non-throwing cleanup path.
inline void AF0FCRelease(void* ptr) throw() {
    if (ptr)
        MWSharedOwner_ReleaseReference(ptr);
}
// Copy the default locale and retain its shared reference owner.
inline void AF0FCCopyLocale(Pair* dst) throw() {
    Pair* src = (Pair*)MWLocale_GetDefault();
    dst->first = src->first;
    dst->second = src->second;
    if (dst->second)
        ++((int*)dst->second)[1];
}

// Six cursor words belong to streambuf, not to the adjacent owned small string.
// +0x18..+0x20 are input/get cursors; +0x24..+0x2C are output/put cursors.
// Their internal begin/current/end order is not established by this unit, which
// only clears all six or sets a whole triplet to the same empty-buffer address.
struct FormatBufferCursorWords {
    void* input[3];
    void* output[3];
};
enum FormatStreamBits {
    kFormatInputCursors = 0x08,
    kFormatOutputCursors = 0x10,
    kFormatInputAndOutput = kFormatInputCursors | kFormatOutputCursors,
    kFormatIosFlagBit2 = 4,
    kFormatInlineLengthMask = 0x7F,
    kFormatGlyphTableByteBias = 0x60
};
enum FormatIosOffsets {
    kFormatIosStateWord = 0x28,
    kFormatIosFlags = 0x30,
    kFormatIosWord34 = 0x34,
    kFormatIosFillCharacter = 0x38
};

// Borrow the streambuf cursor block beginning at the existing p18 ABI field.
static inline FormatBufferCursorWords* FormatCursors(void* firstCursorWord) {
    return (FormatBufferCursorWords*)firstCursorWord;
}

#pragma use_lmw_stmw on
// Format the signed-32 insertion argument context, map result bytes to glyphs, and append to text.
// text is a caller-owned wide string; the temporary stream/locale/narrow string own their storage.
// Mapping is a font-specific table lookup, not a general Unicode/encoding conversion.
void Text_AppendMappedFormattedValue(void* text, void* context) {
    struct Stream {
        char* volatile base;
        int zero;
        void* vtable;
        char* volatile base2;
        void* vtable2;
        void* bufVtable;
        void* p18;
        void* p1c;
        void* p20;
        void* p24;
        void* p28;
        void* p2c;
        void* locale;
        void* ref;
        unsigned char mode;
        char pad39[3];
        char* data;
        unsigned int flags;
        int length;
        char* string;
        char ios[0x40];
    } stream;
    struct StreamCleanup {
        Stream* stream;
        bool complete;
        // Unwind a complete stream, or only ios if later bases were never constructed.
        ~StreamCleanup() {
            if (stream) {
                if (complete)
                    StringStream_dtor(stream, -1);
                else
                    std_ios_base_dtor(stream->ios, 0);
            }
        }
    };
    struct LocaleCleanup {
        Pair* locale;
        // Release the temporary retained locale unless normal exit disarmed this owner.
        ~LocaleCleanup() {
            if (locale)
                AF0FCRelease(locale->second);
        }
    };
    Pair name;
    struct String {
        unsigned int flags;
        unsigned int length;
        char* data;
    } item;
    volatile unsigned char unused[9];
    unsigned short mapped;
    // Phase 1: construct shared ios/input bases and install a temporary locale owner.
    // Stream members model the runtime virtual-base offsets and stream-buffer cursors.
    stream.base = stream.ios;
    *(void* volatile*)stream.ios = &iosBaseVtable;
    stream.base2 = stream.ios;
    stream.vtable = &iStreamVtableGroup;
    *(void**)stream.ios = (char*)&iStreamVtableGroup + 12;
    AF0FCRelocate(stream.base, (char*)&stream.base2);
    char* ios = stream.base;
    stream.zero = 0;
    StreamCleanup streamCleanup = {&stream, false};
    MWIOS_InitializeStreamBuffer(
        ios,
        &stream
             .bufVtable); /* 0x800AF140-0x800AF178: r4 = r31+0x8c, the stream buffer; ours left the incoming context in r4 */
    *(int*)(ios + kFormatIosWord34) = 0;
    MWStream_CopyLocale(&name, ios);
    LocaleCleanup nameCleanup = {&name};
    unused[8] = 0;
    if ((int)(long)s_ctypeFacetId == 0) {
        int n = (int)(long)s_localeFacetIdCounter + 1;
        s_localeFacetIdCounter = (void*)(long)n;
        s_ctypeFacetId = (void*)(long)n;
    }
    unsigned int index = (unsigned int)(long)s_ctypeFacetId;
    void* entries = name.first;
    void* selected;
    if (index >= ((LocaleFacetTable*)entries)->count || (selected = ((void**)*(void**)entries)[index]) == 0) {
        selected = new AF0FCObj(0, 0, 0);
        void* entries = name.first;
        if ((int)(long)s_ctypeFacetId == 0) {
            int n = (int)(long)s_localeFacetIdCounter + 1;
            s_localeFacetIdCounter = (void*)(long)n;
            s_ctypeFacetId = (void*)(long)n;
        }
        MWLocale_InstallFacet(entries, selected, (unsigned int)(long)s_ctypeFacetId);
        selected = ((void**)*(void**)name.first)[index];
    }
    // Facet vtable slot 7 widens the space byte 0x20 for ios fill storage at +0x38.
    // Its result is a fill character, separate from the stringbuf openmode bits below.
    int fillCharacter = ((int (**)(void*, signed char))(*(void***)selected))[7](selected, 0x20);
    nameCleanup.locale = 0;
    AF0FCRelease(name.second);
    ios[kFormatIosFillCharacter] = fillCharacter;
    // Phase 2: finish output/string-stream bases and attach its initially empty stringbuf.
    stream.vtable2 = &s_basicOstreamVtable;
    *(void**)stream.base2 = (char*)&s_basicOstreamVtable + 12;
    AF0FCRelocate(stream.base2, (char*)&stream.bufVtable);
    stream.vtable = &outputStringStreamVtables;
    *(void**)stream.base = (char*)&outputStringStreamVtables + 24;
    stream.vtable2 = (char*)&outputStringStreamVtables + 12;
    AF0FCRelocate(stream.base, (char*)&stream.bufVtable);
    stream.vtable = &s_appStringstreamVTables;
    *(void**)stream.base = (char*)&s_appStringstreamVTables + 24;
    stream.vtable2 = (char*)&s_appStringstreamVTables + 12;
    AF0FCRelocate(stream.base, stream.ios);
    stream.bufVtable = &stringStreamBufferVtable;
    FormatCursors(&stream.p18)->input[0] = 0;
    FormatCursors(&stream.p18)->input[1] = 0;
    FormatCursors(&stream.p18)->input[2] = 0;
    FormatCursors(&stream.p18)->output[0] = 0;
    FormatCursors(&stream.p18)->output[1] = 0;
    FormatCursors(&stream.p18)->output[2] = 0;
    AF0FCCopyLocale((Pair*)&stream.locale);
    stream.flags = 0;
    stream.bufVtable = &s_appStringbufVTable;
    // CodeWarrior openmode: 0x10 enables put/output cursors, 8 get/input cursors.
    // 0x18 enables both. The small-string tag chooses the initial cursor storage.
    stream.mode = kFormatInputAndOutput;
    stream.length = 0;
    stream.string = 0;
    char* data = stream.string;
    if (!(stream.flags >> 31))
        data = (char*)&stream.flags + 1;
    stream.data = data;
    if (stream.mode & kFormatOutputCursors) {
        FormatCursors(&stream.p18)->output[1] = data;
        FormatCursors(&stream.p18)->output[0] = data;
        FormatCursors(&stream.p18)->output[2] = data;
    }
    if (stream.mode & kFormatInputCursors) {
        FormatCursors(&stream.p18)->input[0] = data;
        FormatCursors(&stream.p18)->input[1] = data;
        FormatCursors(&stream.p18)->input[2] = data;
    }
    // Retain the raw ios flag/state writes; the meaning of bit 2 is unresolved here.
    *(unsigned short*)(stream.base + kFormatIosFlags) |= kFormatIosFlagBit2;
    *(int*)(stream.base + kFormatIosStateWord) = 0;
    streamCleanup.complete = true;
    // Phase 3: format, copy out the narrow buffer, then append one mapped wide glyph per byte.
    MWOStream_InsertSigned32((void*)&stream.base2, context);
    MWStringStream_CopyBufferString(&item, &stream);
    struct ItemCleanup {
        String* string;
        // Free copied narrow output only when its small-string tag selects heap mode.
        ~ItemCleanup() {
            if (string && (string->flags >> 31))
                appFreeUni__FPv(string->data);
        }
    } itemCleanup = {&item};
    unsigned int i = 0;
    while (i < (!(item.flags >> 31) ? (*(unsigned char*)&item & kFormatInlineLengthMask) : item.length)) {
        char* bytes;
        if (!(item.flags >> 31))
            bytes = (char*)&item + 1;
        else
            bytes = item.data;
        unsigned char ch = (unsigned char)bytes[i];
        // Signed byte index with a -0x60 BYTE bias: table element index = signed(ch)-0x30.
        // Thus ASCII digit 0 starts at element 0; punctuation (e.g. minus) indexes before it.
        // Caller/table must cover every byte produced by the installed numeric locale.
        mapped = *(unsigned short*)((char*)g_formattedByteToWideGlyphTable +
                                    (int)(signed char)ch * sizeof(unsigned short) - kFormatGlyphTableByteBias);
        int textLen = !(*(unsigned int*)text >> 31) ? (*(unsigned char*)text & kFormatInlineLengthMask)
                                                    : (int)((FormatStringView*)text)->length;
        volatile unsigned char flag;
        unsigned char copy = flag;
        WString_ReplaceRange(text, textLen, 0, &mapped, (char*)&mapped + 2, &copy);
        ++i;
    }
    // Phase 4: release copied text, stringbuf storage and locale, then restore base
    // adjustments in reverse order. Guards cover exceptions; explicit teardown is normal exit.
    itemCleanup.string = 0;
    if (item.flags >> 31)
        appFreeUni__FPv(item.data);
    streamCleanup.stream = 0;
    AF0FCRelocate(stream.base, stream.ios);
    if (stream.flags >> 31)
        appFreeUni__FPv(stream.string);
    AF0FCRelease(stream.ref);
    AF0FCRelocate(stream.base, (char*)&stream.bufVtable);
    AF0FCRelocate(stream.base2, (char*)&stream.bufVtable);
    AF0FCRelocate(stream.base, (char*)&stream.base2);
    std_ios_base_dtor(stream.ios, (short)0);
}
#pragma use_lmw_stmw reset

// Count occurrences of the selected inventory item in category six.
void SharedDoc_CountSelectedInventoryCategory6() {
    Count_SelectedInventoryItemOccurrences(g_sharedDoc, 6);
}

// Return the current shared-context index plus one.
int SharedContext_GetOneBasedIndex() {
    return g_currentSharedContext + 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046207C", _ctors_u800AE594_0, TextFormatting_InitTwoPi);
#pragma pop
