#include "classes/ResourceGroupNode.h"
#include "classes/ResourceLookupTree.h"
#include "classes/ResourceStringNode.h"
#include "classes/ResourceTreeLinks.h"
#include "classes/CCameraWork.h"
// Translation unit 0x800E2A58-0x800E3DE8 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// The outer map has one extra header word beyond the nested string trees.
struct CameraGroupMap {
    unsigned int header;
    void* root;
    void* last;
    void* first;
};
struct CameraGroupValue { unsigned int key; ResourceLookupTree strings; };
struct CleanupObject;
struct CameraAnimationValue {
    unsigned int stringFlags;
    unsigned int heapLength;
    const char* heapData;
    CleanupObject* resource;
};
// Node links occupy +0/+4/+8; the low parent-pointer bit is a tree tag.
// Payload starts at +0xC: group nodes hold {integer key, nested string tree},
// string nodes hold {12-byte small string, resource pointer}. Inline bytes
// overlap the string's three words; bit 31 selects heap ownership instead.
// End iterators point INTO the owning header (at root), never to a node.
// Outer GetFirst reads +0xC; nested GetFirst reads +8. The views are not
// interchangeable merely because both GetEnd implementations use +4.
// Tree cleanup frees keys/nodes only. CCameraWork_ClearResourceTrees explicitly
// deletes resource payloads first; ordinary Clear/Assign do not delete them.
static const unsigned int kParentTagMask = 1u;
static const unsigned int kInlineStringLengthMask = 0x7Fu;

// Return the group-tree sentinel stored in the map header.
void CameraOwnedGroupIterator_GetEnd(void** dst, void* value) {
    *dst = &((ResourceLookupTree*)value)->root;
}

// Expose the integer key and nested string tree of a group node.
// Borrowed payload; the ResourceStringNode cast supplies only the +0xC offset.
void* CameraOwnedGroupIterator_GetEntry(void** value) {
    return &((ResourceStringNode*)*value)->stringFlags;
}

// Begin iteration at the nested string tree's first node.
void CameraOwnedAnimationIterator_GetFirst(void** dst, void** src) {
    *dst = ((ResourceLookupTree*)src)->first;
}

// Expose a string node's key and owned resource pointer.
void* CameraOwnedAnimationIterator_GetEntry(void** value) {
    return &((ResourceStringNode*)*value)->stringFlags;
}

// Return the nested string tree's sentinel.
void CameraOwnedAnimationIterator_GetEnd(void** dst, void* value) {
    *dst = &((ResourceLookupTree*)value)->root;
}

// Begin iteration at the outer map's first node.
void CameraOwnedGroupIterator_GetFirst(void** dst, void** src) {
    *dst = ((CameraGroupMap*)src)->first;
}

// Test whether two nested-tree iterators designate different nodes.
int CameraOwnedAnimationIterator_NotEqual(void** a, void** b) {
    return *a != *b;
}

// Test whether two outer-map iterators designate different nodes.
int CameraOwnedGroupIterator_NotEqual(void** a, void** b) {
    return *a != *b;
}

#pragma opt_common_subs off
// Search by unsigned group key and report the insertion parent and side.
// key points to an ABI word containing the integer key (not an object pointer).
// Return the final visited node, or the header sentinel for an empty tree.
// *predecessor is the last node with key <= requested key, not necessarily the insertion parent returned.
// *insertLeft is the final edge direction; *noPredecessor stays 1 only if no predecessor
// was encountered. It is not a complementary 'right child' flag.
void* CameraResourceGroupTree_FindInsertionPosition(void* tree, void** key, void** predecessor, unsigned char* insertLeft, unsigned char* noPredecessor) {
    unsigned char zero = 0;
    unsigned char one = 1;
    *predecessor = 0;
    void* last = &((ResourceLookupTree*)tree)->root;
    void* node = ((ResourceLookupTree*)tree)->root;
    *insertLeft = one;
    *noPredecessor = one;
    while (node) {
        last = node;
        if ((unsigned int)*key < ((ResourceGroupNode*)node)->key) {
            node = ((ResourceTreeLinks*)node)->left;
            *insertLeft = one;
        } else {
            *predecessor = node;
            node = ((ResourceGroupNode*)node)->right;
            *insertLeft = zero;
            *noPredecessor = zero;
        }
    }
    return last;
}
#pragma opt_common_subs reset

// Save the current group iterator and advance to its in-order successor.
// a receives the old iterator, b is updated; sentinel argument is unused.
// Requires a dereferenceable node, not end(); header links terminate the climb.
void CameraOwnedGroupIterator_PostIncrement(void** a, void** b, void* sentinel) {
    *a = *b;
    void* node = ((ResourceTreeLinks*)*b)->right;
    if (node != 0) {
        while (((ResourceTreeLinks*)node)->left != 0)
            node = ((ResourceTreeLinks*)node)->left;
        *b = node;
        return;
    }
    void* cur;
    void* next;
    // Climb until the current node is a left child, then take that parent.
    while ((cur = *b, next = (void*)((unsigned int)((ResourceTreeLinks*)cur)->taggedParent & ~kParentTagMask),
        cur != ((ResourceTreeLinks*)next)->left)) {
        *b = next;
    }
    *b = next;
    return;
}

// Save the current animation iterator and advance to its in-order successor.
// dst receives the old iterator, src advances; sentinel argument is unused.
void CameraOwnedAnimationIterator_PostIncrement(void** dst, void** src, void* sentinel) {
    *dst = *src;
    void* node = ((ResourceTreeLinks*)*src)->right;
    if (node != 0) {
        while (((ResourceTreeLinks*)node)->left != 0)
            node = ((ResourceTreeLinks*)node)->left;
        *src = node;
        return;
    }
    void* cur;
    void* parent;
    // Original 0x800E3874: the same parent-climbing iterator step.
    while ((cur = *src, parent = (void*)((unsigned int)((ResourceTreeLinks*)cur)->taggedParent & ~kParentTagMask),
        cur != ((ResourceTreeLinks*)parent)->left)) {
        *src = parent;
    }
    *src = parent;
}

void StringValueTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
void CameraNestedStringMap_Clear(void*);
void __dl__FPv(void*);

// Recursively dispose child subtrees, caching each link before its cleanup call.
inline void DestroyStringNodeCached(void* node, void* owner) {
    void* leftChild = *(void**)node;
    if (leftChild) StringValueTree_DestroySubtree(owner, leftChild);
    void* rightChild = ((ResourceStringNode*)node)->right;
    if (rightChild) StringValueTree_DestroySubtree(owner, rightChild);
    if (&((ResourceStringNode*)node)->stringFlags) {
        if (&((ResourceStringNode*)node)->stringFlags) {
            if (((ResourceStringNode*)node)->stringFlags >> 31)
                appFreeUni__FPv((*(void**)&((ResourceStringNode*)node)->heapData));
        }
    }
    appFreeUni__FPv(node);
}

// Release a node's heap-backed string, then the node itself.
// Repeated subobject guards emit original branches and must retain their scopes.
inline void FreeStringNode(void* node) {
    if (&((ResourceStringNode*)node)->stringFlags) {
        if (&((ResourceStringNode*)node)->stringFlags) {
            if (((ResourceStringNode*)node)->stringFlags >> 31)
                appFreeUni__FPv((*(void**)&((ResourceStringNode*)node)->heapData));
        }
    }
    appFreeUni__FPv(node);
}


#pragma opt_common_subs off
// Destroy the owned string tree and optionally free its containing object.
// should_free > 0 deletes self; the returned address may then be dangling.
void* ResourceLookupTree_dtor(void* self, int should_free) {
    if (self) {
      // These six legacy subobject guards each correspond to an emitted branch.
      // This destructor view starts at +8 (first), unlike Clear/Assign at +4.
      // Do not replace one entry point with another based only on the tree type.
      if (&((ResourceLookupTree*)self)->root && &((ResourceLookupTree*)self)->root &&
          &((ResourceLookupTree*)self)->root && &((ResourceLookupTree*)self)->root &&
          &((ResourceLookupTree*)self)->root && &((ResourceLookupTree*)self)->root) {
        void* tree = ((ResourceLookupTree*)self)->first;
        if (tree) {
            if (*(void**)tree) do {
                // Unrolled postorder: left descendants, right descendants, then this node.
                void* subtreeRoot = *(void**)tree;
                void* grandchild;
                void* child = *(void**)subtreeRoot;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    FreeStringNode(child);
                }
                child = ((ResourceTreeLinks*)subtreeRoot)->right;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    FreeStringNode(child);
                }
                FreeStringNode(subtreeRoot);
            } while (0);
            if (((ResourceTreeLinks*)tree)->right) do {
                // Unrolled postorder: left descendants, right descendants, then this node.
                void* subtreeRoot = ((ResourceTreeLinks*)tree)->right;
                void* grandchild;
                void* child = *(void**)subtreeRoot;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    FreeStringNode(child);
                }
                child = ((ResourceTreeLinks*)subtreeRoot)->right;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeCached(grandchild, &((ResourceLookupTree*)self)->root);
                    FreeStringNode(child);
                }
                FreeStringNode(subtreeRoot);
            } while (0);
            FreeStringNode(tree);
        }
      }
        // All descendant keys/nodes are gone; only the containing object remains.
        if (should_free > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset


// Dispose child subtrees while reloading each link for the recursive call.
inline void DestroyStringNodeReloaded(void* node, void* owner) {
    if (*(void**)node) StringValueTree_DestroySubtree(owner, *(void**)node);
    if (((ResourceStringNode*)node)->right) StringValueTree_DestroySubtree(owner, ((ResourceStringNode*)node)->right);
    if (&((ResourceStringNode*)node)->stringFlags) {
        if (&((ResourceStringNode*)node)->stringFlags) {
            if (((ResourceStringNode*)node)->stringFlags >> 31)
                appFreeUni__FPv((*(void**)&((ResourceStringNode*)node)->heapData));
        }
    }
    appFreeUni__FPv(node);
}

#pragma opt_common_subs off
// Release all string nodes and reset the empty-tree header.
void CameraResourceLookupTree_Clear(void* self) {
    void* top = ((ResourceLookupTree*)self)->root;
    if (top) {
        if (*(void**)top) do {
            // Unrolled postorder: left descendants, right descendants, then this node.
            void* subtreeRoot = *(void**)top;
            void* grandchild;
            void* child = *(void**)subtreeRoot;
            if (child) {
                grandchild = *(void**)child;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                grandchild = ((ResourceTreeLinks*)child)->right;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                FreeStringNode(child);
            }
            child = ((ResourceTreeLinks*)subtreeRoot)->right;
            if (child) {
                grandchild = *(void**)child;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                grandchild = ((ResourceTreeLinks*)child)->right;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                FreeStringNode(child);
            }
            FreeStringNode(subtreeRoot);
        } while (0);
        if (((ResourceTreeLinks*)top)->right) do {
            // Unrolled postorder: left descendants, right descendants, then this node.
            void* subtreeRoot = ((ResourceTreeLinks*)top)->right;
            void* grandchild;
            void* child = *(void**)subtreeRoot;
            if (child) {
                grandchild = *(void**)child;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                grandchild = ((ResourceTreeLinks*)child)->right;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                FreeStringNode(child);
            }
            child = ((ResourceTreeLinks*)subtreeRoot)->right;
            if (child) {
                grandchild = *(void**)child;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                grandchild = ((ResourceTreeLinks*)child)->right;
                if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                FreeStringNode(child);
            }
            FreeStringNode(subtreeRoot);
        } while (0);
        FreeStringNode(top);
        *(void**)self = 0;
        ((ResourceLookupTree*)self)->root = 0;
        ((ResourceLookupTree*)self)->first = &((ResourceLookupTree*)self)->root;
    }
}
#pragma opt_common_subs reset

void CameraStringValueTree_Copy(void*, void*);

#pragma opt_common_subs off
// Clear existing nodes and copy the other tree unless assigning to itself.
void* CameraResourceLookupTree_Assign(void* self, void* other) {
    if (self != other) {
        // Phase 1: destroy destination keys/nodes; payload pointers are not deleted.
        void* tree = ((ResourceLookupTree*)self)->root;
        if (tree) {
            if (*(void**)tree) do {
                // Unrolled postorder: left descendants, right descendants, then this node.
                void* subtreeRoot = *(void**)tree;
                void* grandchild;
                void* child = *(void**)subtreeRoot;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    FreeStringNode(child);
                }
                child = ((ResourceTreeLinks*)subtreeRoot)->right;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    FreeStringNode(child);
                }
                FreeStringNode(subtreeRoot);
            } while (0);
            if (((ResourceTreeLinks*)tree)->right) do {
                // Unrolled postorder: left descendants, right descendants, then this node.
                void* subtreeRoot = ((ResourceTreeLinks*)tree)->right;
                void* grandchild;
                void* child = *(void**)subtreeRoot;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    FreeStringNode(child);
                }
                child = ((ResourceTreeLinks*)subtreeRoot)->right;
                if (child) {
                    grandchild = *(void**)child;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    grandchild = ((ResourceTreeLinks*)child)->right;
                    if (grandchild) DestroyStringNodeReloaded(grandchild, self);
                    FreeStringNode(child);
                }
                FreeStringNode(subtreeRoot);
            } while (0);
            FreeStringNode(tree);
            *(void**)self = 0;
            ((ResourceLookupTree*)self)->root = 0;
            ((ResourceLookupTree*)self)->first = &((ResourceLookupTree*)self)->root;
        }
        // Phase 2: copy into the now-empty destination. Self-assignment does nothing.
        CameraStringValueTree_Copy(self, other);
    }
    return self;
}
#pragma opt_common_subs reset

#pragma inline_depth(0)
struct CleanupObject {
    virtual ~CleanupObject();
};
#pragma opt_loop_invariants off
#pragma opt_common_subs off
// Delete resources in every group, then clear the nested and outer maps.
// self and its group-map storage are required; each non-null resource is owned
// exclusively here. Null each resource after deletion before erasing its node.
void CCameraWork_ClearResourceTrees(void* self) {
    register void* camera = self;
    void* groupIterator;
    CameraOwnedGroupIterator_GetFirst((void**)&groupIterator, (void**)(((CCameraWork*)camera)->loadedCharacterResourceGroups));
    int zero = 0;
    void* animationIterator;
    void* previousAnimation;
    void* animationEnd;
    void* previousGroup;
    void* groupEnd;
    // Phase 1: groupIterator belongs to the OUTER integer map; animationIterator belongs
    // to that group's INNER string map. Entry results borrow node payloads.
    // Re-fetch payloads after virtual deletion; do not cache them across callbacks.
    // Delete resources, then erase the group's string nodes before advancing groups.
    while ((CameraOwnedGroupIterator_GetEnd(&groupEnd, ((CCameraWork*)camera)->loadedCharacterResourceGroups), CameraOwnedGroupIterator_NotEqual(&groupIterator, &groupEnd))) {
        CameraOwnedAnimationIterator_GetFirst(&animationIterator, (void**)(&((CameraGroupValue*)CameraOwnedGroupIterator_GetEntry(&groupIterator))->strings));
        while ((CameraOwnedAnimationIterator_GetEnd(&animationEnd, &((CameraGroupValue*)CameraOwnedGroupIterator_GetEntry(&groupIterator))->strings), CameraOwnedAnimationIterator_NotEqual(&animationIterator, &animationEnd))) {
            if (((CameraAnimationValue*)CameraOwnedAnimationIterator_GetEntry(&animationIterator))->resource) {
                delete ((CameraAnimationValue*)CameraOwnedAnimationIterator_GetEntry(&animationIterator))->resource;
                (void*&) ((CameraAnimationValue*)CameraOwnedAnimationIterator_GetEntry(&animationIterator))->resource = (void*)zero;
            }
            CameraOwnedAnimationIterator_PostIncrement(&previousAnimation, &animationIterator, 0);
        }
        CameraResourceLookupTree_Clear(&((CameraGroupValue*)CameraOwnedGroupIterator_GetEntry(&groupIterator))->strings);
        CameraOwnedGroupIterator_PostIncrement(&previousGroup, &groupIterator, 0);
    }
    // Phase 2: no nested resources/nodes remain; erase the outer group nodes.
    CameraNestedStringMap_Clear((void*)(((CCameraWork*)camera)->loadedCharacterResourceGroups));
}
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset
#pragma inline_depth(255)

#include "sdk_protos.h"
unsigned int String_ReserveCapacity(unsigned int*, unsigned int);
void String_ReplaceRange(unsigned int*, unsigned int, unsigned int, const char*, const char*, const char*);

// Build the temporary search string using the library's range replacement routine.
// The final byte supplies a stateless allocator token, not a string terminator.
static inline void AssignQueryString(unsigned int* q, const char* b, const char* e) {
    unsigned char term;
    volatile unsigned int fillw;
    term = *(volatile unsigned char*)&fillw;
    String_ReplaceRange(q, 0, 0, b, e, (const char*)&term);
}

// Test a decoded heap-storage flag (0/1), not a raw string representation word.
static inline bool IsInlineString(unsigned int heapStorageFlag) {
    return heapStorageFlag == 0;
}

// Return a reference to the shorter length, preserving the caller's storage.
static inline const unsigned int& MinimumLengthRef(const unsigned int& a, const unsigned int& b) {
    return b < a ? b : a;
}

// Read logical byte length from a borrowed MW string node, selecting inline tag or heap length.
inline unsigned int StringNodeLength(void* node) {
    if ((((ResourceStringNode*)node)->stringFlags >> 31) == 0)
        return (*(unsigned char*)&((ResourceStringNode*)node)->stringFlags) & kInlineStringLengthMask;
    return ((ResourceStringNode*)node)->heapLength;
}
// Borrow the key's byte span; no ownership transfer, allocation or NUL scan occurs.
inline void DecodeStringNode(void* node, const char*& data, unsigned int& length) {
    if ((((ResourceStringNode*)node)->stringFlags >> 31) == 0) {
        data = (const char*)&((ResourceStringNode*)node)->stringFlags + 1;
        length = (*(unsigned char*)&((ResourceStringNode*)node)->stringFlags) & kInlineStringLengthMask;
    } else {
        data = ((ResourceStringNode*)node)->heapData;
        length = ((ResourceStringNode*)node)->heapLength;
    }
}

#pragma opt_common_subs off
#pragma opt_loop_invariants off
#pragma opt_propagation off
#pragma use_lmw_stmw on
// Find a group by integer key, then find its resource by exact string key.
// Returns a borrowed resource pointer or null; self and its outer map are required.
// text is borrowed NUL-terminated bytes, compared case-sensitively by memcmp.
// Null text bypasses name lookup and returns firstString's value: the caller
// must ensure that the selected group has a dereferenceable first string node.
void* ResourceGroups_FindNamedValue(void* self, unsigned int key, const char* text) {
    void* stringNode;
    const char* heapData;
    unsigned int heapLength;
    unsigned int queryOwnsHeap;
    unsigned int inlineLength;
    void* groupPosition;
    void* candidateNode;
    unsigned int query[3];
    unsigned int comparableQueryLength;
    unsigned int queryLengthFromData;
    unsigned int candidateLength;
    unsigned int queryLengthFromFlags;
    unsigned int keyReportedLength;
    // The repeated lengths are snapshots for two comparisons, not different units:
    // lower_bound uses key-vs-query; equality verification uses query-vs-candidate.
    // All lengths count bytes, excluding NUL. Heap length words are invalid for inline keys.
    unsigned int queryLengthForCompare;
    unsigned int keyLengthFromData;
    unsigned int comparableKeyLength;
    // Phase 1: lower_bound on the unsigned group key, then reject a non-equal key.
    void* node = *(void**)(&((CameraGroupMap*)((CCameraWork*)self)->loadedCharacterResourceGroups)->root);
    groupPosition = &((CameraGroupMap*)((CCameraWork*)self)->loadedCharacterResourceGroups)->root;
    while (node) {
        if (!(((ResourceGroupNode*)node)->key < key)) {
            groupPosition = node;
            node = *(void**)node;
        } else {
            node = ((ResourceGroupNode*)node)->right;
        }
    }
    if (groupPosition == &((CameraGroupMap*)((CCameraWork*)self)->loadedCharacterResourceGroups)->root || key < ((ResourceGroupNode*)groupPosition)->key) {
        groupPosition = &((CameraGroupMap*)((CCameraWork*)self)->loadedCharacterResourceGroups)->root;
    }
    if (groupPosition == &((CameraGroupMap*)((CCameraWork*)self)->loadedCharacterResourceGroups)->root) {
        return 0;
    }
    if (text == 0) {
        return ((ResourceStringNode*)((ResourceGroupNode*)groupPosition)->firstString)->value;
    }

    // Phase 2: own a temporary query string; the tree's stored keys stay borrowed.
    query[0] = 0;
    query[1] = 0;
    query[2] = 0;
    unsigned int length = strlen(text);
    String_ReserveCapacity(query, length);
    AssignQueryString(query, text, text + length);
    queryOwnsHeap = query[0] >> 31;
    candidateNode = &((ResourceGroupNode*)groupPosition)->stringRoot;
    stringNode = ((ResourceGroupNode*)groupPosition)->stringRoot;
    heapData = (const char*)query[2];
    heapLength = query[1];
    inlineLength = *(unsigned char*)query & kInlineStringLengthMask;
    unsigned int queryIsInline = queryOwnsHeap == 0;
    unsigned int queryIsHeap = !queryIsInline;
    // Phase 3: lower_bound on byte strings: compare common prefix, then length.
    // Length helpers expose the inline/heap ABI; duplicate reads/minima preserve code shape.
    while (stringNode) {
        unsigned int requestedLength;
        const char* requestedData;
        if (!queryIsHeap) {
            requestedLength = inlineLength;
            requestedData = (const char*)query + 1;
        } else {
            requestedData = heapData;
            requestedLength = heapLength;
        }
        queryLengthForCompare = requestedLength;
        unsigned int decodedKeyLength;
        const char* valueData;
        keyReportedLength = StringNodeLength(stringNode);
        DecodeStringNode(stringNode, valueData, decodedKeyLength);
        keyLengthFromData = decodedKeyLength;
        comparableKeyLength = MinimumLengthRef(keyLengthFromData, keyReportedLength);
        unsigned int commonPrefixLength = MinimumLengthRef(comparableKeyLength, queryLengthForCompare);
        int keyVsQuery = memcmp(valueData, requestedData, commonPrefixLength);
        if (keyVsQuery == 0) {
            if (comparableKeyLength < requestedLength) {
                keyVsQuery = -1;
            } else if (comparableKeyLength != requestedLength) {
                keyVsQuery = 1;
            }
        }
        if (!(keyVsQuery < 0)) {
            candidateNode = stringNode;
            stringNode = *(void**)stringNode;
        } else {
            stringNode = ((ResourceStringNode*)stringNode)->right;
        }
    }
    // Phase 4: reject a lower_bound candidate greater than the requested string.
    if (candidateNode != &((ResourceGroupNode*)groupPosition)->stringRoot) {
        // Decode the candidate from one saved tag, then compare query-vs-candidate for equality.
        unsigned int bestMarker = ((ResourceStringNode*)candidateNode)->stringFlags;
        unsigned int bestLength;
        const char* bestData;
        if ((bestMarker >> 31) == 0) {
            bestLength = (*(unsigned char*)&((ResourceStringNode*)candidateNode)->stringFlags) & kInlineStringLengthMask;
            bestData = (const char*)&((ResourceStringNode*)candidateNode)->stringFlags + 1;
        } else {
            bestLength = ((ResourceStringNode*)candidateNode)->heapLength;
            bestData = ((ResourceStringNode*)candidateNode)->heapData;
        }
        candidateLength = bestLength;
        queryLengthFromFlags = !queryIsHeap ? inlineLength : query[1];
        const char* queryData;
        if (!queryIsHeap) {
            queryData = (const char*)query + 1;
            queryLengthFromData = inlineLength;
        } else {
            queryData = (const char*)query[2];
            queryLengthFromData = query[1];
        }
        comparableQueryLength = MinimumLengthRef(queryLengthFromData, queryLengthFromFlags);
        unsigned int bestLen = MinimumLengthRef(comparableQueryLength, candidateLength);
        int queryVsCandidate = memcmp(queryData, bestData, bestLen);
        if (queryVsCandidate == 0) {
            if (comparableQueryLength < bestLength) {
                queryVsCandidate = -1;
            } else if (comparableQueryLength != bestLength) {
                queryVsCandidate = 1;
            }
        }
        if (queryVsCandidate < 0) {
            candidateNode = &((ResourceGroupNode*)groupPosition)->stringRoot;
        }
    }
    // Phase 5: release only the temporary query; return the tree's borrowed value.
    if (queryOwnsHeap) {
        appFreeUni__FPv((void*)query[2]);
    }
    return candidateNode == &((ResourceGroupNode*)groupPosition)->stringRoot ? 0 : ((ResourceStringNode*)candidateNode)->value;
}
#pragma use_lmw_stmw reset
#pragma opt_propagation reset
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset

} // extern "C"
