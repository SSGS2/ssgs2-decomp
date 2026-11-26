// String-keyed maps used by CCameraWork: the camera name and resource maps, their insertion
// and lookup, and the teardown code that frees their nodes and strings.

typedef unsigned long new_size_t;
#define CAMERA_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
#include "classes/StringPair800D750C.h"
#include "classes/CCameraWork.h"
// View the existing storage as StringPair800D750C without transferring ownership.
static inline StringPair800D750C* AsStringPair800D750C(const volatile void* p) {
    return (StringPair800D750C*)p;
}

extern "C" {

// Node links: left/right at +0/+4, color word at +8, payload at +0xC.
// The 16-byte owner stores count, root, an unresolved link word, and a tail link.
// Empty trees use the address of their root slot as the insertion-parent/tail sentinel.
struct Node {
    Node* leftChild;
    Node* right;
    unsigned char treeColorStorage[4];
};

// Only byte-key maps have this payload. String-key maps share Node links, not its key layout.
struct CameraByteKeyNodeView {
    Node links;
    unsigned char key;
};

struct OrderedTreeOwner {
    int count;
    Node* head;
    unsigned char reservedTreeLink[4];
    Node* sentinel;
};

void appFreeUni__FPv(void*);
extern char __vt__11CCameraWork[];
void __dl__FPv(void*);
Node* CameraByteStringMap_FindInsertPosition(OrderedTreeOwner*, unsigned char*, Node**, unsigned char*, unsigned char*);
void ResourceGroups_ClearThirdTable(void*);
void CameraResourceGroups_ClearOwnedAnimationPlayers(void*);
void CameraWorkStringMap_Clear(void*);
void CameraWorkStringValueMap_Clear(void*);
void* CameraWorkTree_dtor(void*, int);
void CameraNestedMap_dtor(void*, int);
void StringMap_dtor(void*, int);
void CameraIndexedGroupMap_dtor(void*, int);
void CameraActionVector_dtor(void*, int);
void CameraWork_DestroyBase(void*, int);
void* CameraByteStringMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
struct CameraStringAllocatorTag {
    unsigned char value;
    // Preserve the allocator tag and its original volatile-copy behavior.
    CameraStringAllocatorTag() {}
    // Preserve the allocator tag and its original volatile-copy behavior.
    CameraStringAllocatorTag(const CameraStringAllocatorTag& other) {
        value = *(volatile const unsigned char*)&other;
    }
};
void* String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, void*, void*, void*);
// Copy the half-open byte range into string storage using the supplied allocator tag.
// The tag is an ABI byte, not an ownership flag; the resulting string owns its copy.
static inline void range7594(CameraStringAllocatorTag allocator, void* p, unsigned char* begin, unsigned char* end) {
    String_ReplaceRange(p, 0, 0, begin, end, &allocator);
}
// Test whether the node value precedes the requested unsigned key.
static inline bool NodePrecedesKey(unsigned int node, unsigned int key) {
    return node < key;
} // currently unused
int memcmp(const unsigned char*, const unsigned char*, unsigned int);

enum CameraStringEncoding {
    kCameraHeapStringFlagShift = 31,
    kCameraInlineLengthMask = 0x7F
};

struct StringRep {
    unsigned int flag;
    unsigned int length;
    unsigned char* data;
};

// Ordered map node header followed by two engine strings (inline or heap).
struct CameraStringNodeView {
    void* left;
    void* right;
    unsigned int color;
    StringRep key;
    StringRep value;
};
struct CameraMapRootView {
    unsigned int count;
    void* root;
};

// Heap strings store length/data separately; inline strings store length in their first byte.
static unsigned int strLength(StringRep* s) {
    return (s->flag >> kCameraHeapStringFlagShift) ? s->length : (s->flag & kCameraInlineLengthMask);
}
// Return borrowed bytes from the active inline or heap string representation.
static unsigned char* strData(StringRep* s) {
    return (s->flag >> kCameraHeapStringFlagShift) ? s->data : (unsigned char*)s + 1;
}
// Compare unsigned values, returning -1, 0, or 1.
static inline int cmp3(unsigned int lhs, unsigned int rhs) {
    return lhs < rhs ? -1 : (lhs == rhs ? 0 : 1);
}

#pragma use_lmw_stmw on
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_propagation off
#pragma opt_loop_invariants off
// Find the insertion parent and greatest visited key <= the requested string.
// found is a predecessor candidate, not proof of equality; the caller compares it again.
// insertBefore selects the parent's left link; updateTail stays 1 only on an all-left search.
// self/key and all outputs must be valid; returned nodes remain owned by the tree.
void* CameraStringMap_FindInsertPosition(void* self, StringRep* key, Node** found, unsigned char* insertBefore,
                                         unsigned char* updateTail) {
    unsigned int nlen;
    Node* previous = (Node*)((char*)self + CAMERA_MEMBER_OFFSET(OrderedTreeOwner, head));
    *found = 0;
    Node* node = ((OrderedTreeOwner*)self)->head;
    *insertBefore = 1;
    *updateTail = 1;
    // Descend by memcmp over the common prefix, then by length when prefixes match.
    while (node) {
        previous = node;
        unsigned int nflag = ((CameraStringNodeView*)node)->key.flag;
        unsigned char* ndata;
        if (!(nflag >> kCameraHeapStringFlagShift)) {
            ndata = (unsigned char*)&((CameraStringNodeView*)node)->key + 1;
            nlen = *(unsigned char*)&((CameraStringNodeView*)node)->key & kCameraInlineLengthMask;
        } else {
            ndata = ((CameraStringNodeView*)node)->key.data;
            nlen = ((CameraStringNodeView*)node)->key.length;
        }
        unsigned int keyLengthBeforeData;
        (void)*(volatile unsigned int*)key;
        unsigned int nodeLengthCopy = nlen;
        unsigned int kl;
        if (!(*(unsigned int*)key >> kCameraHeapStringFlagShift)) {
            kl = *(unsigned char*)key & kCameraInlineLengthMask;
        } else {
            kl = key->length; // original +4
        }
        keyLengthBeforeData = kl;
        unsigned char* kdata;
        unsigned int len2;
        if (!(*(unsigned int*)key >> kCameraHeapStringFlagShift)) {
            kdata = (unsigned char*)key + 1;
            len2 = *(unsigned char*)key & kCameraInlineLengthMask;
        } else {
            kdata = key->data;
            len2 = key->length; // original +4
        }
        unsigned int keyLengthAfterData = len2;
        // Repeated length reads belong to the original string accessor expansion.
        // First stabilize the key length, then compare min(key length, node length).
        unsigned int* shorterKeyLength = &keyLengthAfterData;
        if (keyLengthBeforeData < keyLengthAfterData)
            shorterKeyLength = &keyLengthBeforeData;
        unsigned int stableKeyLength = *shorterKeyLength;
        unsigned int* shorterPrefixLength = &stableKeyLength;
        if (nlen < stableKeyLength)
            shorterPrefixLength = &nodeLengthCopy;
        unsigned int commonPrefixLength = *shorterPrefixLength;
        int cmp = memcmp(kdata, ndata, commonPrefixLength);
        if (!cmp) {
            unsigned int kk = stableKeyLength;
            if (kk < nlen)
                cmp = -1;
            else if (kk == nlen)
                cmp = 0;
            else
                cmp = 1;
        }
        // Moving right records a possible equal key or predecessor; it may still be too small.
        if (cmp < 0) {
            node = node->leftChild;
            *insertBefore = 1;
        } else {
            *found = node;
            node = node->right;
            *insertBefore = 0;
            *updateTail = 0;
        }
    }
    return previous;
}
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

struct CameraMapKeyString;
// Return caller-provided placement storage; no allocation or ownership transfer occurs.
inline void* operator new(new_size_t, void* p) throw() {
    return p;
}
static inline void copy7158(CameraMapKeyString* self, const CameraMapKeyString& other, CameraStringAllocatorTag& state,
                            CameraStringAllocatorTag& arg);
struct CameraMapKeyString {
    unsigned int flags;
    unsigned int length;
    unsigned char* data;
    // Initialize engine-string storage for the string-map insertion payload.
    CameraMapKeyString() : flags(0), length(0), data(0) {}
    // Initialize engine-string storage for the string-map insertion payload.
    CameraMapKeyString(const CameraMapKeyString& other, CameraStringAllocatorTag& st, CameraStringAllocatorTag& ar) {
        copy7158(this, other, st, ar);
    }
    // Free the string buffer only when the representation owns heap storage.
    ~CameraMapKeyString() {
        if (flags >> kCameraHeapStringFlagShift)
            appFreeUni__FPv(data);
    }
};
struct CameraMapValueString {
    unsigned int flags;
    unsigned int length;
    unsigned char* data;
    // Copy the mapped string into the insertion payload.
    CameraMapValueString(const CameraMapKeyString& other, CameraStringAllocatorTag& st, CameraStringAllocatorTag& ar) {
        copy7158((CameraMapKeyString*)this, other, st, ar);
    }
    // Free the string buffer only when the representation owns heap storage.
    ~CameraMapValueString() {
        if (flags >> kCameraHeapStringFlagShift)
            appFreeUni__FPv(data);
    }
};
// Copy inline string words directly, or reserve and copy an independently owned heap buffer.
static inline void copy7158(CameraMapKeyString* self, const CameraMapKeyString& other, CameraStringAllocatorTag& state,
                            CameraStringAllocatorTag& arg) {
    if (!(other.flags >> kCameraHeapStringFlagShift)) {
        unsigned int ln = other.length;
        unsigned char* dp = other.data;
        self->flags = other.flags;
        self->length = ln;
        self->data = dp;
    } else {
        self->flags = 0;
        self->length = 0;
        self->data = 0;
        String_ReserveCapacity(self, other.length);
        arg.value = *(volatile const unsigned char*)&state;
        unsigned char* begin = other.data;
        unsigned char* end = begin + other.length;
        String_ReplaceRange(self, 0, 0, begin, end, &arg);
    }
}
struct CameraMapInsertionPair {
    CameraMapKeyString first;
    CameraMapValueString second;
    // Construct the key/value pair with independent string copies.
    CameraMapInsertionPair(const CameraMapKeyString& key, const CameraMapKeyString& value, CameraStringAllocatorTag& s1,
                           CameraStringAllocatorTag& a1, CameraStringAllocatorTag& s2, CameraStringAllocatorTag& a2)
        : first(key, s1, a1), second(value, s2, a2) {}
    // Destroy the contained strings through their member destructors.
    ~CameraMapInsertionPair() {}
};

void* CameraStringMap_InsertNode(void*, void*, unsigned char, unsigned char, void*);
// Return a reference to the smaller unsigned value, choosing rhs on equality.
static inline unsigned int& min7158(unsigned int& lhs, unsigned int& rhs) {
    return lhs < rhs ? lhs : rhs;
}

// Decode the string length from the active inline or heap representation.
static inline unsigned int size7158(StringRep* s) {
    if (!(s->flag >> kCameraHeapStringFlagShift))
        return *(unsigned char*)s & kCameraInlineLengthMask;
    return s->length;
}
// Compare unsigned lengths, returning -1, 0, or 1.
static inline int cmp7158(unsigned int lhs, unsigned int rhs) {
    return lhs < rhs ? -1 : (lhs == rhs ? 0 : 1);
}
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Return the mapped string, inserting an empty owned string when the key is absent.
// The returned mutable string is borrowed from the map, valid until its node is erased.
// key is copied on insertion; a failed equality test inserts using the saved parent/side.
void* CameraStringMap_GetOrInsertString(void* self, StringRep* key) {
    Node* found;
    unsigned char insertBefore;
    unsigned char updateTail;
    void* insertionParent = CameraStringMap_FindInsertPosition(self, key, &found, &insertBefore, &updateTail);
    void* keyStorage;
    // Original 0x800D7158: an existing key skips insertion.
    do {
        if (found) {
            StringRep* candidateKey = &((CameraStringNodeView*)found)->key;
            keyStorage = candidateKey;
            unsigned int lenA;
            unsigned int klen;
            unsigned int klenCopy;
            unsigned int lenB;
            unsigned int m1;
            unsigned char* kdata;
            unsigned char* ndata;
            if (!(key->flag >> kCameraHeapStringFlagShift)) {
                kdata = (unsigned char*)key + 1;
                klen = *(unsigned char*)key & kCameraInlineLengthMask;
            } else {
                kdata = key->data;
                klen = key->length;
            }
            klenCopy = klen;
            lenA = size7158(candidateKey);
            unsigned int lenBTmp;
            if (!(candidateKey->flag >> kCameraHeapStringFlagShift)) {
                ndata = (unsigned char*)candidateKey + 1;
                lenBTmp = *(unsigned char*)candidateKey & kCameraInlineLengthMask;
            } else {
                ndata = candidateKey->data;
                lenBTmp = candidateKey->length;
            }
            lenB = lenBTmp;
            m1 = min7158(lenA, lenB);
            int cmp = memcmp(ndata, kdata, min7158(klenCopy, m1));
            if (!cmp) {
                if (m1 < klen)
                    cmp = -1;
                else if (m1 == klen)
                    cmp = 0;
                else
                    cmp = 1;
            }
            if (cmp >= 0)
                break;
        }
        // No equal key: construct an owned key/empty-value pair and insert its copy.
        {
            CameraMapKeyString empty;
            CameraStringAllocatorTag a2;
            CameraStringAllocatorTag s2;
            CameraStringAllocatorTag a1;
            CameraStringAllocatorTag s1;
            CameraMapInsertionPair pair(*(CameraMapKeyString*)key, empty, s1, a1, s2, a2);
            keyStorage =
                (char*)CameraStringMap_InsertNode(self, insertionParent, insertBefore, updateTail, &pair) + 0x0c;
        }
    } while (0);
    return (char*)keyStorage + sizeof(StringRep);
}
#pragma opt_common_subs reset
#pragma use_lmw_stmw reset

struct StringPair800D750CView {
    unsigned int flags;
    unsigned char firstLengthStorage[4];
    void* ptr08;
    unsigned int secondFlags;
    unsigned char secondLengthStorage[4];
    void* ptr14;
};

// Release the second and first heap-backed strings, then optionally free their pair.
void* StringPair800D750C_dtor(StringPair800D750CView* self, int deleting) {
    if (self) {
        if ((volatile char*)self + 0x0C &&
            ((*(volatile unsigned int*)&self->secondFlags >> kCameraHeapStringFlagShift) != 0))
            appFreeUni__FPv(AsStringPair800D750C(self)->secondStringData);
        if (self && ((*(volatile unsigned int*)self >> kCameraHeapStringFlagShift) != 0))
            appFreeUni__FPv(AsStringPair800D750C(self)->firstStringData);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

struct CameraByteMapInsertionString {
    unsigned int flags;
    unsigned int length;
    unsigned char* data;
    // Initialize the byte-map temporary string using this constructor variant.
    CameraByteMapInsertionString() : flags(0), length(0), data(0) {}
    // Initialize the byte-map temporary string using this constructor variant.
    // This recovered variant copies heap input only; inline input leaves an empty string.
    CameraByteMapInsertionString(const CameraByteMapInsertionString& other) {
        flags = 0;
        length = 0;
        data = 0;
        if (other.flags >> kCameraHeapStringFlagShift) {
            flags = 0;
            length = 0;
            data = 0;
            // Unrecovered ABI call: this code supplies no C++ arguments and uses
            // the returned r3 word as the replacement destination. Adding this/
            // length would change existing code; a typed repair needs a separate audit.
            void* reservedStringStorage = ((void* (*)())String_ReserveCapacity)();
            CameraStringAllocatorTag state;
            unsigned char* begin = other.data;
            unsigned char* end = begin + other.length;
            CameraStringAllocatorTag allocator(state);
            String_ReplaceRange(reservedStringStorage, 0, 0, begin, end, &allocator);
        }
    }
    // Free the string buffer only when the representation owns heap storage.
    ~CameraByteMapInsertionString() {
        if (flags >> kCameraHeapStringFlagShift)
            appFreeUni__FPv(data);
    }
};
struct CameraByteMapInsertionPair {
    unsigned char key;
    CameraByteMapInsertionString value;
    // Copy the byte key and temporary string into the insertion payload.
    CameraByteMapInsertionPair(unsigned char k, const CameraByteMapInsertionString& v) : key(k), value(v) {}
    // Destroy the contained strings through their member destructors.
    ~CameraByteMapInsertionPair() {}
};

// Return the string for a byte key, inserting an empty value when absent.
// The returned mutable value is borrowed from the map; insertion copies the one-byte key.
void* CameraByteStringMap_GetOrInsert(OrderedTreeOwner* self, unsigned char* key) {
    Node* found;
    unsigned char insertBefore, updateTail;
    Node* insertionParent = CameraByteStringMap_FindInsertPosition(self, key, &found, &insertBefore, &updateTail);
    void* keyStorage;
    // Original 0x800D7594: share the value return after optional insertion.
    do {
        if (found) {
            unsigned char keyValue = *(volatile unsigned char*)key;
            keyStorage = (char*)found + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key);
            if (keyValue <= ((CameraByteKeyNodeView*)found)->key)
                break;
        }
        {
            CameraByteMapInsertionString empty;
            CameraByteMapInsertionPair pair(*key, empty);
            keyStorage =
                (char*)CameraByteStringMap_InsertNode(self, insertionParent, insertBefore, updateTail, &pair) + 0xc;
        }
    } while (0);
    return (char*)keyStorage + 4;
}

// Release the camera's resource trees, its name maps, and the action history.
CCameraWork::~CCameraWork() {
    vtbl = __vt__11CCameraWork;
    ResourceGroups_ClearThirdTable(this);
    ClearResourceTrees();
    CameraResourceGroups_ClearOwnedAnimationPlayers(this);
    CameraWorkStringMap_Clear(awardMotionAliases);
    CameraWorkStringValueMap_Clear(indexedCameraGroups);
    CameraWorkStringMap_Clear(resourceGroupNames);
    CameraWorkStringMap_Clear(characterModelMappings);
    CameraWorkStringMap_Clear(powerShotMotionAliases);
    // The clear calls above release payloads; the map destructors here dispose of the tree storage.
    // A negative deleting flag destroys an embedded map without freeing its address.
    CameraWorkTree_dtor(cameraWorkTree, -1);
    CameraNestedMap_dtor(nestedCameraMap, -1);
    CameraNestedMap_dtor(loadedCharacterResourceGroups, -1);
    StringMap_dtor(awardMotionAliases, -1);
    StringMap_dtor(resourceGroupNames, -1);
    StringMap_dtor(powerShotMotionAliases, -1);
    CameraIndexedGroupMap_dtor(indexedCameraGroups, -1);
    StringMap_dtor(characterModelMappings, -1);
    CameraActionVector_dtor(&actionHistory, -1);
    CameraWork_DestroyBase(this, 0);
}

// Initialize an empty tree whose tail points at the root slot.
void CameraWorkTree_Initialize(OrderedTreeOwner* self) {
    // Phase 3: restore the owner header to its empty sentinel representation.
    self->count = 0;
    self->head = 0;
    self->sentinel = (Node*)((char*)self + CAMERA_MEMBER_OFFSET(OrderedTreeOwner, head));
}

// Find the insertion parent and greatest visited byte key <= *key.
// Outputs follow CameraStringMap_FindInsertPosition; equality is checked by the caller.
Node* CameraByteStringMap_FindInsertPosition(OrderedTreeOwner* self, unsigned char* key, Node** found,
                                             unsigned char* insertBefore, unsigned char* updateTail) {
    *found = 0;
    Node* previous = (Node*)((char*)self + CAMERA_MEMBER_OFFSET(OrderedTreeOwner, head));
    Node* node = self->head;
    *insertBefore = 1;
    *updateTail = 1;
    while (node) {
        unsigned int keyValue = *key;
        previous = node;
        if (keyValue < (unsigned int)((CameraByteKeyNodeView*)node)->key) {
            node = node->leftChild;
            *insertBefore = 1;
        } else {
            *found = node;
            node = node->right;
            *insertBefore = 0;
            *updateTail = 0;
        }
    }
    return previous;
}

// This allocation contains an unknown leading word and a tagged string, not tree links.
struct TaggedStringOwnerView {
    unsigned int prefix;
    unsigned int flags;
    unsigned int length;
    void* data;
};

// Free the heap representation when its ownership bit is set, then optionally delete its owner.
void* TaggedString_dtor(TaggedStringOwnerView* self, int deleting) {
    if (self) {
        volatile char* base = (char*)self;
        if (base + 4) {
            unsigned int stringFlags = self->flags;
            if (stringFlags >> kCameraHeapStringFlagShift)
                appFreeUni__FPv(self->data);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

void* MapTree_FreeNodesRecursive(void*, void*);
void CameraResourceMap_DestroyNodePayload(void*, void*);
void StringValueTree_DestroySubtree(void*, void*);

struct D6980Tree {
    unsigned int count;
    void* root;
};
struct D6980Root {
    char pad[0x10];
    D6980Tree tree;
};
struct D6980Node {
    void* left;
    void* right;
    unsigned int treeColor;
    unsigned int flags;
    unsigned int length;
    void* data;
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Release only the heap-owned representation of a nested node's tagged string.
// The repeated subobject-address guards preserve destructor expansion; ownership is flags>>31.
static inline void DestroyNestedStringPayload(void* node) {
    if ((char*)node + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
        if ((char*)node + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
            if (((D6980Node*)node)->flags >> kCameraHeapStringFlagShift)
                appFreeUni__FPv(((D6980Node*)node)->data);
}
// Free one outer-tree node after recursively freeing its children and payload.
static inline void DestroyOuterNodePostorder(void* self, void* node) {
    void* child1;
    void* child0;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        MapTree_FreeNodesRecursive(self, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        MapTree_FreeNodesRecursive(self, child1);
    CameraResourceMap_DestroyNodePayload(self, (char*)node + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
    appFreeUni__FPv(node);
}
// Expand one additional level of outer-tree postorder destruction.
static inline void DestroyOuterBranchTwoLevels(void* self, void* node) {
    void* child1;
    void* child0;
    void* n = node;
    child0 = ((CameraStringNodeView*)n)->left;
    if (child0)
        DestroyOuterNodePostorder(self, child0);
    child1 = ((CameraStringNodeView*)n)->right;
    if (child1)
        DestroyOuterNodePostorder(self, child1);
    CameraResourceMap_DestroyNodePayload(self, (char*)n + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
    appFreeUni__FPv(n);
}
// Expand two additional levels of outer-tree postorder destruction.
static inline void DestroyOuterBranchThreeLevels(void* self, void* node) {
    void* child1;
    void* child0;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        DestroyOuterBranchTwoLevels(self, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        DestroyOuterBranchTwoLevels(self, child1);
    CameraResourceMap_DestroyNodePayload(self, (char*)node + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
    appFreeUni__FPv(node);
}
// Free a nested-map node and its heap string after freeing children.
static inline void DestroyNestedStringNodePostorder(void* root, void* node) {
    void* child0;
    void* child1;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        StringValueTree_DestroySubtree(&((D6980Root*)root)->tree, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        StringValueTree_DestroySubtree(&((D6980Root*)root)->tree, child1);
    DestroyNestedStringPayload(node);
    appFreeUni__FPv(node);
}
// Expand one additional level of nested-map postorder destruction.
static inline void DestroyNestedStringBranchTwoLevels(void* root, void* node) {
    void* child0;
    void* child1;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        DestroyNestedStringNodePostorder(root, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        DestroyNestedStringNodePostorder(root, child1);
    DestroyNestedStringPayload(node);
    appFreeUni__FPv(node);
}
// Expand two additional levels of nested-map postorder destruction.
static inline void DestroyNestedStringBranchThreeLevels(void* root, void* node) {
    void* child0;
    void* child1;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        DestroyNestedStringBranchTwoLevels(root, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        DestroyNestedStringBranchTwoLevels(root, child1);
    DestroyNestedStringPayload(node);
    appFreeUni__FPv(node);
}
// Expand three additional levels of nested-map postorder destruction.
static inline void DestroyNestedStringBranchFourLevels(void* root, void* node) {
    void* child0;
    void* child1;
    child0 = ((CameraStringNodeView*)node)->left;
    if (child0)
        DestroyNestedStringBranchThreeLevels(root, child0);
    child1 = ((CameraStringNodeView*)node)->right;
    if (child1)
        DestroyNestedStringBranchThreeLevels(root, child1);
    DestroyNestedStringPayload(node);
    appFreeUni__FPv(node);
}
// Ownership map for camera resource trees:
// outer owner -> outer nodes -> payload at +0x0C -> nested owner -> string nodes.
// The outer root's embedded tree begins at +0x10; descendants are released through
// CameraResourceMap_DestroyNodePayload. Nested strings own heap bytes, never cameras.
// All expanded traversal blocks follow left child -> right child -> payload -> node.
// Destroy the outer tree and the nested tree attached to its root.
// This is teardown, not a reusable clear: it frees nodes without resetting owner links.
// The expanded recursion levels and repeated guards preserve CodeWarrior cleanup shape.
// Outer descendants own nested maps in their +0x0C payload. Finish those
// maps before freeing the outer node; the outer root embeds its map at +0x10.
static inline void DestroyCameraTreeStorage(void* self) {
    if (self)
        if (self)
            if (self)
                if (self)
                    if (self) {
                        void* rightLeft;
                        void* nextBranch;
                        void* nextLeftChild;
                        void* nextRightChild;
                        void* leftLeft;
                        void* branch;
                        void* right;
                        void* nestedRoot;
                        void* root;
                        root = ((CameraMapRootView*)self)->root;
                        if (root) {
                            branch = ((CameraStringNodeView*)root)->left;
                            if (branch) {
                                leftLeft = ((CameraStringNodeView*)branch)->left;
                                if (leftLeft) {
                                    rightLeft = ((CameraStringNodeView*)leftLeft)->left;
                                    if (rightLeft)
                                        DestroyOuterNodePostorder(self, rightLeft);
                                    nextBranch = ((CameraStringNodeView*)leftLeft)->right;
                                    if (nextBranch)
                                        DestroyOuterNodePostorder(self, nextBranch);
                                    CameraResourceMap_DestroyNodePayload(
                                        self, (char*)leftLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                                    appFreeUni__FPv(leftLeft);
                                }
                                nextBranch = ((CameraStringNodeView*)branch)->right;
                                if (nextBranch) {
                                    nextLeftChild = ((CameraStringNodeView*)nextBranch)->left;
                                    if (nextLeftChild)
                                        DestroyOuterNodePostorder(self, nextLeftChild);
                                    nextRightChild = ((CameraStringNodeView*)nextBranch)->right;
                                    if (nextRightChild)
                                        DestroyOuterNodePostorder(self, nextRightChild);
                                    CameraResourceMap_DestroyNodePayload(
                                        self, (char*)nextBranch + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                                    appFreeUni__FPv(nextBranch);
                                }
                                CameraResourceMap_DestroyNodePayload(
                                    self, (char*)branch + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                                appFreeUni__FPv(branch);
                            }
                            nextBranch = ((CameraStringNodeView*)root)->right;
                            if (nextBranch)
                                DestroyOuterBranchThreeLevels(self, nextBranch);
                            if ((char*)root + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
                                if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                                    if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                                        if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                                            if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                                                if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                                                    if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree)) {
                                                        nestedRoot = ((D6980Root*)root)->tree.root;
                                                        if (nestedRoot)
                                                            DestroyNestedStringBranchFourLevels(root, nestedRoot);
                                                    }
                            appFreeUni__FPv(root);
                        }
                    }
}
// Destroy the outer camera tree and its nested owned resource maps.
void* CameraWorkTree_dtor(void* self, int deleting) {
    if (self) {
        DestroyCameraTreeStorage(self);
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

void StringMap_DestroyStringPair(void*, void*);
void StringMap_FreeSubtree(void*, void*);

struct T7880 {
    void* left;
    void* right;
    unsigned int treeColor;
    unsigned int flags0C;
    unsigned int length10;
    void* ptr14;
    unsigned int flags18;
    unsigned int length1C;
    void* ptr20;
};
struct U7880 {
    unsigned int count;
    void* root;
    void* reservedTreeLink;
    void* sentinel;
};

// Free both subtrees and both strings before freeing the node.
static inline void destroyStringPairNode(void* n, void* owner) {
    if (*(void**)(n))
        StringMap_FreeSubtree((owner), *(void**)(n));
    if (*(void**)((char*)(n) + 4))
        StringMap_FreeSubtree((owner), *(void**)((char*)(n) + 4));
    StringMap_DestroyStringPair((owner), (char*)(n) + 0x0C);
    appFreeUni__FPv((n));
}

#pragma use_lmw_stmw on
// Free string-key/string-value nodes and reset a previously nonempty tree.
void CameraWorkStringMap_Clear(void* owner) {
    // Phase 1: free descendant pairs in postorder; root-specific volatile cleanup follows.
    U7880* self = (U7880*)owner;
    T7880* root = (T7880*)self->root;
    void* rightLeft;
    void* leftLeft;
    void* branch;
    if (root) {
        branch = root->left;
        if (branch) {
            leftLeft = ((CameraStringNodeView*)branch)->left;
            if (leftLeft) {
                rightLeft = ((CameraStringNodeView*)leftLeft)->left;
                if (rightLeft)
                    destroyStringPairNode(rightLeft, self);
                rightLeft = ((CameraStringNodeView*)leftLeft)->right;
                if (rightLeft)
                    destroyStringPairNode(rightLeft, self);
                StringMap_DestroyStringPair(self, (char*)leftLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                appFreeUni__FPv(leftLeft);
            }
            rightLeft = ((CameraStringNodeView*)branch)->right;
            if (rightLeft) {
                leftLeft = ((CameraStringNodeView*)rightLeft)->left;
                if (leftLeft)
                    destroyStringPairNode(leftLeft, self);
                leftLeft = ((CameraStringNodeView*)rightLeft)->right;
                if (leftLeft)
                    destroyStringPairNode(leftLeft, self);
                StringMap_DestroyStringPair(self, (char*)rightLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                appFreeUni__FPv(rightLeft);
            }
            StringMap_DestroyStringPair(self, (char*)branch + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
            appFreeUni__FPv(branch);
        }
        rightLeft = ((CameraStringNodeView*)root)->right;
        if (rightLeft) {
            leftLeft = ((CameraStringNodeView*)rightLeft)->left;
            if (leftLeft) {
                branch = ((CameraStringNodeView*)leftLeft)->left;
                if (branch)
                    destroyStringPairNode(branch, self);
                branch = ((CameraStringNodeView*)leftLeft)->right;
                if (branch)
                    destroyStringPairNode(branch, self);
                StringMap_DestroyStringPair(self, (char*)leftLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                appFreeUni__FPv(leftLeft);
            }
            leftLeft = ((CameraStringNodeView*)rightLeft)->right;
            if (leftLeft) {
                branch = ((CameraStringNodeView*)leftLeft)->left;
                if (branch)
                    destroyStringPairNode(branch, self);
                branch = ((CameraStringNodeView*)leftLeft)->right;
                if (branch)
                    destroyStringPairNode(branch, self);
                StringMap_DestroyStringPair(self, (char*)leftLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
                appFreeUni__FPv(leftLeft);
            }
            StringMap_DestroyStringPair(self, (char*)rightLeft + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key));
            appFreeUni__FPv(rightLeft);
        }
        // Phase 2: destroy the root strings with their original guarded volatile reads.
        if ((unsigned int)root + 0xc) {
            if ((volatile char*)root + 0x18) {
                if ((*(volatile unsigned int*)((char*)root + CAMERA_MEMBER_OFFSET(CameraStringNodeView, value))) >>
                    kCameraHeapStringFlagShift)
                    appFreeUni__FPv(((T7880*)root)->ptr20);
            }
            if ((volatile char*)root + 0xc) {
                if ((*(volatile unsigned int*)&((T7880*)root)->flags0C) >> kCameraHeapStringFlagShift)
                    appFreeUni__FPv(((T7880*)root)->ptr14);
            }
        }
        appFreeUni__FPv(root);
        self->count = 0;
        self->root = 0;
        self->sentinel = (char*)self + CAMERA_MEMBER_OFFSET(OrderedTreeOwner, head);
    }
}
#pragma use_lmw_stmw reset

void MapTree_FreeSubtreeWithRelease(void*, void*);

// This specialization has a four-byte payload prefix at +0x0C, then its
// tagged string at +0x10 (data at +0x18). CameraStringNodeView is used only
// for its common left/right links; its key field must not decode this payload.
struct T7C0C {
    char pad00[0x10];
    unsigned int flags;
    char pad14[4];
    void* ptr18;
};

// Release a named-resource node's owned heap string; link/node storage stays with its caller.
static inline void ReleaseCameraResourceNodeName(void* node) {
    if ((char*)node + 0x0C)
        if ((char*)node + 0x10)
            if (((T7C0C*)node)->flags >> kCameraHeapStringFlagShift)
                appFreeUni__FPv(((T7C0C*)node)->ptr18);
}

// Free both subtrees and the heap string before freeing the node.
static inline void destroyNamedResourceNode(void* n, void* self) {
    if (*(void**)(n))
        MapTree_FreeSubtreeWithRelease(self, *(void**)(n));
    if (*(void**)((char*)(n) + 4))
        MapTree_FreeSubtreeWithRelease(self, *(void**)((char*)(n) + 4));
    if ((char*)(n) + 0x0C)
        if ((char*)(n) + 0x10)
            if (((T7C0C*)(n))->flags >> kCameraHeapStringFlagShift)
                appFreeUni__FPv(((T7C0C*)(n))->ptr18);
    appFreeUni__FPv(n);
}

struct _U7C0C {
    unsigned int count;
    void* root;
    void* reservedTreeLink;
    void* sentinel;
};
/* destroy the tree hanging off offset 4 */
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Free string-bearing resource nodes and reset a previously nonempty tree.
// Branch cleanup is expanded below: a helper call changes CodeWarrior register allocation.
// Each branch frees descendants first, then its owned heap string and node allocation.
void CameraWorkStringValueMap_Clear(void* self) {
    _U7C0C* s = (_U7C0C*)self;
    void* root = s->root;
    void* child;
    if (root) {
        void* branch = ((CameraStringNodeView*)root)->left;
        if (branch) {
            void* leftLeft = ((CameraStringNodeView*)branch)->left;
            if (leftLeft)
                do {
                    void* descendant = *(void**)leftLeft;
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    descendant = *(void**)((char*)leftLeft + 4);
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    ReleaseCameraResourceNodeName(leftLeft);
                    appFreeUni__FPv(leftLeft);
                } while (0);
            void* leftRight = ((CameraStringNodeView*)branch)->right;
            if (leftRight)
                do {
                    void* descendant = *(void**)leftRight;
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    descendant = *(void**)((char*)leftRight + 4);
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    ReleaseCameraResourceNodeName(leftRight);
                    appFreeUni__FPv(leftRight);
                } while (0);
            if ((char*)branch + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
                if ((char*)branch + CAMERA_MEMBER_OFFSET(T7C0C, flags))
                    if (((T7C0C*)branch)->flags >> kCameraHeapStringFlagShift)
                        appFreeUni__FPv(((T7C0C*)branch)->ptr18);
            appFreeUni__FPv(branch);
        }
        branch = ((CameraStringNodeView*)root)->right;
        if (branch) {
            void* rightLeft = ((CameraStringNodeView*)branch)->left;
            if (rightLeft)
                do {
                    void* descendant = *(void**)rightLeft;
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    descendant = *(void**)((char*)rightLeft + 4);
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    ReleaseCameraResourceNodeName(rightLeft);
                    appFreeUni__FPv(rightLeft);
                } while (0);
            void* rightRight = ((CameraStringNodeView*)branch)->right;
            if (rightRight)
                do {
                    void* descendant = *(void**)rightRight;
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    descendant = *(void**)((char*)rightRight + 4);
                    if (descendant)
                        destroyNamedResourceNode(descendant, self);
                    ReleaseCameraResourceNodeName(rightRight);
                    appFreeUni__FPv(rightRight);
                } while (0);
            if ((char*)branch + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
                if ((char*)branch + CAMERA_MEMBER_OFFSET(T7C0C, flags))
                    if (((T7C0C*)branch)->flags >> kCameraHeapStringFlagShift)
                        appFreeUni__FPv(((T7C0C*)branch)->ptr18);
            appFreeUni__FPv(branch);
        }
        if ((char*)root + CAMERA_MEMBER_OFFSET(CameraStringNodeView, key))
            if ((char*)root + CAMERA_MEMBER_OFFSET(D6980Root, tree))
                if (((T7C0C*)root)->flags >> kCameraHeapStringFlagShift)
                    appFreeUni__FPv(((T7C0C*)root)->ptr18);
        appFreeUni__FPv(root);
        s->count = 0;
        s->root = 0;
        s->sentinel = (void*)((char*)s + CAMERA_MEMBER_OFFSET(_U7C0C, root));
    }
}
#pragma opt_lifetimes reset
#pragma use_lmw_stmw reset

} // extern "C"
