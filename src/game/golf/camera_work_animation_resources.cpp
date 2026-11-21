#include "game/string.h"
#include "classes/ResourceStringNode.h"
#define AsResourceStringNode(p) ((ResourceStringNode*)(p))

// Translation unit 0x800E3DE8-0x800E4CD4 (6 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Red-black tree node view shared by the byte-keyed name map and the numeric group map.
// +0x00 left, +0x04 right, +0x08 parent/color word, +0x0c key (byte) / tagged string, +0x18 payload.
struct KeyedTreeNode {
    KeyedTreeNode* left;
    KeyedTreeNode* right;
    char parentAndColor[4];
    unsigned char key;
    unsigned char keyPadding[3];
    unsigned int flags;
    char reserved14[4];
    unsigned int value;
};

void* CameraResourceGroups_FindCharacterModelMapping(void*, void*); /* (self, key): key goes on to ItemInventory_FindCharacterData in r4 */
void* MWString_ConstructCString(void*, void*);
void MWString_AppendCString(void*, const char*);
// CameraStringValueTree_InitEmpty: public symbol spelling is retained for coordinator renaming.
void CameraStringValueTree_InitEmpty(void*);
void Storage_ClearThreeWords(void*); // Clears three storage words; initializes empty strings here.
// CameraStringMapIterator_GetFirst: public symbol spelling is retained for coordinator renaming.
void CameraStringMapIterator_GetFirst(void*, void*);
void* ScriptString_GetCharacterData(void*);
void MWString_AssignCString(void*, void*);
// CameraStringMapIterator_GetEntry: public symbol spelling is retained for coordinator renaming.
void* CameraStringMapIterator_GetEntry(void*);
// CameraResourceString_AppendString: public symbol spelling is retained for coordinator renaming.
void CameraResourceString_AppendString(void*, void*);
void* CameraScript_CreatePlayerForMode(void*, void*, int);
void* StringValueTree_GetOrInsertValue(void*, void*);
// CameraStringMapIterator_PostIncrement: public symbol spelling is retained for coordinator renaming.
void CameraStringMapIterator_PostIncrement(void*, void*, int);
// CameraStringMapIterator_GetEnd: public symbol spelling is retained for coordinator renaming.
void CameraStringMapIterator_GetEnd(void*, void*);
// CameraStringMapIterator_NotEqual: public symbol spelling is retained for coordinator renaming.
int CameraStringMapIterator_NotEqual(void*, void*);
// CameraNumericGroupMap_GetOrCreateStringTree: public symbol spelling is retained for coordinator renaming.
void* CameraNumericGroupMap_GetOrCreateStringTree(void*, void*);
void CameraResourceLookupTree_Assign(void*, void*);
void* CameraWorkTable_GetOrCreateAnimationMap(void*, void*);
unsigned int strlen(const char*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*, const char*, unsigned char*);
int memcmp(const char*, const char*, unsigned int);
void appFreeUni__FPv(void*);
extern char cameraWorkStrings[];

void CameraStringValueTree_dtor(void*, int);

// CameraOwnedGroupIterator_GetFirst: public symbol spelling is retained for coordinator renaming.
void CameraOwnedGroupIterator_GetFirst(void*, void*);
// CameraOwnedGroupIterator_GetEntry: public symbol spelling is retained for coordinator renaming.
void* CameraOwnedGroupIterator_GetEntry(void*);
// CameraOwnedAnimationIterator_GetFirst: public symbol spelling is retained for coordinator renaming.
void CameraOwnedAnimationIterator_GetFirst(void*, void*);
// CameraOwnedAnimationIterator_GetEntry: public symbol spelling is retained for coordinator renaming.
void* CameraOwnedAnimationIterator_GetEntry(void*);
// CameraOwnedAnimationIterator_PostIncrement: public symbol spelling is retained for coordinator renaming.
void CameraOwnedAnimationIterator_PostIncrement(void*, void*, int);
// CameraOwnedAnimationIterator_GetEnd: public symbol spelling is retained for coordinator renaming.
void CameraOwnedAnimationIterator_GetEnd(void*, void*);
// CameraOwnedAnimationIterator_NotEqual: public symbol spelling is retained for coordinator renaming.
int CameraOwnedAnimationIterator_NotEqual(void*, void*);
void CameraResourceLookupTree_Clear(void*);
// CameraOwnedGroupIterator_PostIncrement: public symbol spelling is retained for coordinator renaming.
void CameraOwnedGroupIterator_PostIncrement(void*, void*, int);
// CameraOwnedGroupIterator_GetEnd: public symbol spelling is retained for coordinator renaming.
void CameraOwnedGroupIterator_GetEnd(void*, void*);
// CameraOwnedGroupIterator_NotEqual: public symbol spelling is retained for coordinator renaming.
int CameraOwnedGroupIterator_NotEqual(void*, void*);
struct CleanupSelf;
void CameraNestedStringMap_Clear(CleanupSelf*);
void CameraNestedStringMap_DestroySubtree(void*, void*);
void CameraNestedStringMap_DestroyEntry(void*, void*);
void StringValueTree_DestroySubtree(void*, void*);

// Nested numeric-key map; its payload is a string-key tree, not a string object.
struct AnimationGroupNode {
    void* left; void* right; unsigned parentAndColor; unsigned key;
    unsigned animationCount; void* animationRoot; void* firstAnimation;
};
struct CameraGroupStorage {
    char reserved00[0x298]; char playerAnimationNames[0x40];
    unsigned loadedGroupCount; AnimationGroupNode* loadedGroupRoot;
};
struct CameraAnimationEntry { char key[12]; void* player; };
struct CameraNamedGroup { unsigned key; char animations[12]; };
struct CameraStringEntry { char key[12]; char suffix[12]; };
struct CameraAnimationNamePool { char reserved00[0x282]; char separator[2], rankPrefix[6]; };
enum CameraResourceConstants {
    kAnimationNameSeparatorOffset = (int)&((CameraAnimationNamePool*)0)->separator,
    kAnimationSetPrefixOffset = (int)&((CameraAnimationNamePool*)0)->rankPrefix,
    kPlayerAnimationMapOffset = (int)&((CameraGroupStorage*)0)->playerAnimationNames,
    kLoadedGroupMapOffset = (int)&((CameraGroupStorage*)0)->loadedGroupCount
};

struct CameraResourceNameMapView { char pad0[0x28c]; KeyedTreeNode* head; };

// Find a byte-keyed resource name and return its inline or heap-backed text.
void* ResourceNameMap_FindById(void* resourceGroups, unsigned int key) {
    CameraResourceNameMapView* self = (CameraResourceNameMapView*)resourceGroups;
    KeyedTreeNode* node = self->head;
    KeyedTreeNode* result = (KeyedTreeNode*)&self->head;
    while (node) {
        if (node->key >= key) {
            result = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (result != (KeyedTreeNode*)&self->head && key < result->key)
        result = (KeyedTreeNode*)&self->head;
    if (result != (KeyedTreeNode*)&self->head) {
        if ((result->flags >> 31) == 0)
            return (char*)&result->flags + 1;
        return (void*)result->value;
    }
    return 0;
}

struct CameraOwnedGroupsView { char pad0[0x2d8]; char map[4]; };

struct CameraOwnedPlayer { 
virtual void v2(int);
};

// Delete every owned animation player, then clear the nested group maps.
// map owns group nodes; each group owns animation nodes and their player pointers.
// Iterator buffers are opaque ABI storage, not strings: outer needs 12 bytes,
// inner/end/postincrement results need 4. PostIncrement updates its second argument.
void CameraResourceGroups_ClearOwnedAnimationPlayers(CameraOwnedGroupsView* self) {
    int i = 0;
    char groupIterator[0xc];
    char animationIterator[4];
    char previousAnimationIterator[4];
    char animationEnd[4];
    char previousGroupIterator[4];
    unsigned char groupEnd[4];
    // Phase 1: visit groups and destroy their players before releasing any map nodes.
    CameraOwnedGroupIterator_GetFirst((void*)groupIterator, self->map);
    // Refresh each end iterator before the condition, including on the first pass.
    while ((CameraOwnedGroupIterator_GetEnd(groupEnd, self->map), CameraOwnedGroupIterator_NotEqual((void*)groupIterator, groupEnd))) {
        void* groupEntry = CameraOwnedGroupIterator_GetEntry((void*)groupIterator);
        CameraOwnedAnimationIterator_GetFirst(animationIterator, ((CameraNamedGroup*)groupEntry)->animations);
        while ((CameraOwnedAnimationIterator_GetEnd(animationEnd, ((CameraNamedGroup*)CameraOwnedGroupIterator_GetEntry((void*)groupIterator))->animations), CameraOwnedAnimationIterator_NotEqual(animationIterator, animationEnd))) {
            void* animationEntry = CameraOwnedAnimationIterator_GetEntry(animationIterator);
            if (((CameraAnimationEntry*)animationEntry)->player) {
                animationEntry = CameraOwnedAnimationIterator_GetEntry(animationIterator);
                void* player = ((CameraAnimationEntry*)animationEntry)->player;
                if (player) {
                    ((CameraOwnedPlayer*)player)->v2(1);
                }
                animationEntry = CameraOwnedAnimationIterator_GetEntry(animationIterator);
                ((CameraAnimationEntry*)animationEntry)->player = 0;
            }
            CameraOwnedAnimationIterator_PostIncrement(previousAnimationIterator, animationIterator, 0);
        }
        // No live player remains in this group; release its animation nodes next.
        CameraResourceLookupTree_Clear(((CameraNamedGroup*)CameraOwnedGroupIterator_GetEntry((void*)groupIterator))->animations);
        CameraOwnedGroupIterator_PostIncrement(previousGroupIterator, (void*)groupIterator, 0);
    }
    // Phase 2: all inner maps are empty; release the containing group nodes.
    CameraNestedStringMap_Clear((CleanupSelf*)self->map);
}

// Build a character-specific animation name for each suffix and cache its player by group.
void CameraResourceGroups_LoadPlayerAnimationGroup(void* self, void* groupKey) {
    unsigned char animationPrefix[0x20];
    char loadedPlayers[0x0c];
    char animationName[0x0c];
    char suffixIterator[4];
    char previousIterator[4];
    char suffixEnd[4];
    void* result;

    MWString_ConstructCString(animationPrefix, CameraResourceGroups_FindCharacterModelMapping(self, groupKey)); /* 0x800E45BC: r4 = groupKey */
    MWString_AppendCString(animationPrefix, cameraWorkStrings + kAnimationNameSeparatorOffset);
    CameraStringValueTree_InitEmpty(loadedPlayers);
    Storage_ClearThreeWords(animationName);
    CameraStringMapIterator_GetFirst(suffixIterator, (char*)self + kPlayerAnimationMapOffset);
    /* 0x800E45FC: b 0x800E4670 - the end() compare (CameraStringMapIterator_GetEnd/CameraStringMapIterator_NotEqual) runs before the
       first pass, so an empty map runs no body (ours ran it once on the end node) */
    while ((CameraStringMapIterator_GetEnd(suffixEnd, (char*)self + kPlayerAnimationMapOffset), CameraStringMapIterator_NotEqual(suffixIterator, suffixEnd))) {
        MWString_AssignCString(animationName, ScriptString_GetCharacterData(animationPrefix));
        CameraResourceString_AppendString(animationName, ((CameraStringEntry*)CameraStringMapIterator_GetEntry(suffixIterator))->suffix);
        result = CameraScript_CreatePlayerForMode(self, ScriptString_GetCharacterData(animationName), 1);
        *(void**)StringValueTree_GetOrInsertValue(loadedPlayers, CameraStringMapIterator_GetEntry(suffixIterator)) = result;
        CameraStringMapIterator_PostIncrement(previousIterator, suffixIterator, 0);
    }
    CameraResourceLookupTree_Assign(CameraNumericGroupMap_GetOrCreateStringTree((char*)self + kLoadedGroupMapOffset, &groupKey), loadedPlayers);
    ((NString*)(animationName))->~NString();
    CameraStringValueTree_dtor(loadedPlayers, -1);
    ((NString*)(animationPrefix))->~NString();
}

struct CameraAnimationSetView { char pad0[0x2b8]; char map[4]; char pad1[0x2c]; char table[4]; };

// Load the rank animation set for a byte key and replace its cached player map.
void CameraWorkTable_LoadAnimationSet(CameraAnimationSetView* self, unsigned char key) {
    char animationPrefix[0x18];
    char loadedPlayers[0x0c];
    char animationName[0x0c];
    char suffixIterator[4];
    char previousIterator[4];
    char suffixEnd[4];
    void* result;
    MWString_ConstructCString(animationPrefix, cameraWorkStrings + kAnimationSetPrefixOffset);
    MWString_AppendCString(animationPrefix, (const char*)ResourceNameMap_FindById(self, key));
    MWString_AppendCString(animationPrefix, cameraWorkStrings + kAnimationNameSeparatorOffset);
    CameraStringValueTree_InitEmpty(loadedPlayers);
    Storage_ClearThreeWords(animationName);
    CameraStringMapIterator_GetFirst(suffixIterator, self->map);
    while ((CameraStringMapIterator_GetEnd(suffixEnd, self->map), CameraStringMapIterator_NotEqual(suffixIterator, suffixEnd))) {
        MWString_AssignCString(animationName, ScriptString_GetCharacterData(animationPrefix));
        CameraResourceString_AppendString(animationName, ((CameraStringEntry*)CameraStringMapIterator_GetEntry(suffixIterator))->suffix);
        result = CameraScript_CreatePlayerForMode(self, ScriptString_GetCharacterData(animationName), 0);
        *(void**)StringValueTree_GetOrInsertValue(loadedPlayers, CameraStringMapIterator_GetEntry(suffixIterator)) = result;
        CameraStringMapIterator_PostIncrement(previousIterator, suffixIterator, 0);
    }
    CameraResourceLookupTree_Assign(CameraWorkTable_GetOrCreateAnimationMap(self->table, &key), loadedPlayers);
    ((NString*)(animationName))->~NString();
    CameraStringValueTree_dtor(loadedPlayers, -1);
    ((NString*)(animationPrefix))->~NString();
}

struct CleanupNode { void* child[2]; unsigned int pad; unsigned int flags; unsigned int size; void* data; };

struct CleanupSelf { CleanupNode* first; CleanupNode* root; char pad[4]; CleanupNode** map; };

// CleanupNode is a shared storage view, not one universal payload type.
// Outer node: +0xc numeric key, +0x10 inner-map count, +0x14 inner root.
// Inner node: +0xc tagged string, +0x10 heap length, +0x14 heap address.
// All cleanup below is left subtree -> right subtree -> owned payload -> node.
// Tests of member addresses come from inlined destructors; they are not ownership tests.

// Release only the heap-backed form of a tagged small string.
static inline void destroyCleanupString(unsigned int* rep) {
    if (rep) {
        if (rep) {
            if (*rep >> 31) appFreeUni__FPv((void*)rep[2]);
        }
    }
}

// Destroy a visible inner node after recursively releasing both deeper subtrees.
static inline void destroyStringLeaf(CleanupNode* root, CleanupNode* node) {
    if (node->child[0]) StringValueTree_DestroySubtree(&root->size, node->child[0]);
    if (node->child[1]) StringValueTree_DestroySubtree(&root->size, node->child[1]);
    destroyCleanupString(&node->flags);
    appFreeUni__FPv(node);
}

// Destroy two child leaves before this node; scratch aliases preserve caller lifetimes.
static inline void destroyStringPair(CleanupNode* root, CleanupNode* node, CleanupNode*& child) {
    child = (CleanupNode*)node->child[0];
    if (child) { destroyStringLeaf(root, child); }
    child = (CleanupNode*)node->child[1];
    if (child) { destroyStringLeaf(root, child); }
    destroyCleanupString(&node->flags);
    appFreeUni__FPv(node);
}

// Destroy one inner-tree branch in postorder, retaining the three visible unrolled levels.
static inline void destroyStringBranch(CleanupNode* root, CleanupNode* node, CleanupNode*& branch, CleanupNode*& child) {
    branch = (CleanupNode*)node->child[0];
    if (branch) { destroyStringPair(root, branch, child); }
    branch = (CleanupNode*)node->child[1];
    if (branch) { destroyStringPair(root, branch, child); }
    destroyCleanupString(&node->flags);
    appFreeUni__FPv(node);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_lifetimes off
// Plain C: free every node of the two-level map (group nodes, each holding a string tree) in postorder,
// then reset the map header to empty. The six-deep unrolled blocks below only mirror the compiler's inlining.
// level1/level2/level3 are successive depths below the root; scratch is a leftover temp for the helpers.
void CameraNestedStringMap_Clear(CleanupSelf* self) {
    CleanupNode* root;
    CleanupNode* level3;
    CleanupNode* level1;
    CleanupNode* level2;
    CleanupNode* scratch;
    root = self->root;
    if (root) {
        // Phase 1: outer left branch; deepest visible levels recurse into the same specialization.
        level1 = (CleanupNode*)root->child[0];
        if (level1) {
            level2 = (CleanupNode*)level1->child[0];
            if (level2) {
                level3 = (CleanupNode*)level2->child[0];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                level3 = (CleanupNode*)level2->child[1];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                CameraNestedStringMap_DestroyEntry(self, &level2->flags);
                appFreeUni__FPv(level2);
            }
            level2 = (CleanupNode*)level1->child[1];
            if (level2) {
                level3 = (CleanupNode*)level2->child[0];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                level3 = (CleanupNode*)level2->child[1];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                CameraNestedStringMap_DestroyEntry(self, &level2->flags);
                appFreeUni__FPv(level2);
            }
            CameraNestedStringMap_DestroyEntry(self, &level1->flags);
            appFreeUni__FPv(level1);
        }
        // Phase 2: outer right branch, using the same postorder destruction.
        level1 = (CleanupNode*)root->child[1];
        if (level1) {
            level2 = (CleanupNode*)level1->child[0];
            if (level2) {
                level3 = (CleanupNode*)level2->child[0];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                level3 = (CleanupNode*)level2->child[1];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                CameraNestedStringMap_DestroyEntry(self, &level2->flags);
                appFreeUni__FPv(level2);
            }
            level2 = (CleanupNode*)level1->child[1];
            if (level2) {
                level3 = (CleanupNode*)level2->child[0];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                level3 = (CleanupNode*)level2->child[1];
                if (level3) {
                    if (level3->child[0]) CameraNestedStringMap_DestroySubtree(self, level3->child[0]);
                    if (level3->child[1]) CameraNestedStringMap_DestroySubtree(self, level3->child[1]);
                    CameraNestedStringMap_DestroyEntry(self, &level3->flags);
                    appFreeUni__FPv(level3);
                }
                CameraNestedStringMap_DestroyEntry(self, &level2->flags);
                appFreeUni__FPv(level2);
            }
            CameraNestedStringMap_DestroyEntry(self, &level1->flags);
            appFreeUni__FPv(level1);
        }
        // Phase 3: root's inner string tree. Only heap-backed keys own separate allocations.
        // The stacked address-of tests below are always true (inlined destructor guards); removing them changes codegen.
        // Net effect: free the root's inner string tree, then its nodes' heap strings.
        if (&root->flags) {
            if (&root->size) {
                if (&root->size) {
                    if (&root->size) {
                        if (&root->size) {
                            if (&root->size) {
                                if (&root->size) {
                                    level2 = (CleanupNode*)root->data;
                                    if (level2) {
                                        level1 = (CleanupNode*)level2->child[0];
                                        if (level1) { destroyStringBranch(root, level1, level3, scratch); }
                                        level1 = (CleanupNode*)level2->child[1];
                                        if (level1) { destroyStringBranch(root, level1, level3, scratch); }
                                        destroyCleanupString(&level2->flags);
                                        appFreeUni__FPv(level2);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // Phase 4: release the outer root and restore the empty header/sentinel links.
        appFreeUni__FPv(root);
        self->first = 0;
        self->root = 0;
        self->map = &self->root;
    }
}

#pragma pop

struct TemporaryStringStorage { unsigned flags; unsigned len; const char* ptr; };

// Return the shorter byte length by reference, preserving the original comparison temporaries.
static inline const unsigned& smallerLength(const unsigned& a, const unsigned& b) { return (b < a) ? b : a; }

// Decode a tree key's length without taking ownership of its inline or heap storage.
static inline unsigned nodeStringLength(const char* n) {
    // String-tree node: tagged string +0xc, heap length +0x10, heap data +0x14.
    unsigned f = AsResourceStringNode(n)->stringFlags;
    return ((f >> 31) == 0) ? *(const unsigned char*)&AsResourceStringNode(n)->stringFlags & 0x7f : AsResourceStringNode(n)->heapLength;
}

// Compare a stored node name with query text, using length to break equal-prefix ties.
// qs/qlong/tmp are unused ABI remnants; ql is the actual query length in bytes.
static inline int compareNodeToQuery(const char* n, const char* queryData, unsigned queryLength, unsigned qs, bool qlong, unsigned* tmp) {
    unsigned queryLengthCopy = queryLength;
    unsigned f = AsResourceStringNode(n)->stringFlags;
    unsigned storedLength = ((f >> 31) == 0) ? *(const unsigned char*)&AsResourceStringNode(n)->stringFlags & 0x7f : AsResourceStringNode(n)->heapLength;
    unsigned storedLengthCopy = storedLength;
    const char* storedData;
    unsigned decodedLength;
    f = AsResourceStringNode(n)->stringFlags;
    if ((f >> 31) == 0) { storedData = (const char*)&AsResourceStringNode(n)->stringFlags + 1; decodedLength = *(const unsigned char*)&AsResourceStringNode(n)->stringFlags & 0x7f; }
    else { storedData = AsResourceStringNode(n)->heapData; decodedLength = AsResourceStringNode(n)->heapLength; }
    unsigned decodedLengthCopy = decodedLength;
    unsigned storedMinimum = smallerLength(decodedLengthCopy, storedLengthCopy);
    unsigned prefixLength = smallerLength(storedMinimum, queryLengthCopy);
    int r = memcmp(storedData, queryData, prefixLength);
    if (r == 0) {
        if (storedMinimum < queryLength) return -1;
        if (storedMinimum == queryLength) return 0;
        return 1;
    }
    return r;
}

// Compare the query string with a stored name for the final lower-bound check.
static inline int compareQueryToNode(const char* n, TemporaryStringStorage* temp, bool isLong, unsigned qsz) {
    const char* storedData;
    unsigned storedLength;
    unsigned f = AsResourceStringNode(n)->stringFlags;
    if ((f >> 31) == 0) { storedData = (const char*)&AsResourceStringNode(n)->stringFlags + 1; storedLength = *(const unsigned char*)&AsResourceStringNode(n)->stringFlags & 0x7f; }
    else { storedData = AsResourceStringNode(n)->heapData; storedLength = AsResourceStringNode(n)->heapLength; }
    unsigned storedLengthCopy = storedLength;
    unsigned queryLength = isLong ? temp->len : qsz;
    const char* queryData;
    unsigned decodedQueryLength;
    if (!isLong) { queryData = (const char*)temp + 1; decodedQueryLength = qsz; }
    else { queryData = temp->ptr; qsz = temp->len; decodedQueryLength = qsz; }
    unsigned queryMinimum = smallerLength(decodedQueryLength, queryLength);
    unsigned prefixLength = smallerLength(queryMinimum, storedLengthCopy);
    int comparison = memcmp(queryData, storedData, prefixLength);
    if (comparison == 0) {
        if (queryMinimum < storedLength) return -1;
        if (queryMinimum == storedLength) return 0;
        return 1;
    }
    return comparison;
}

// Form the inner map's end sentinel; it aliases its root-pointer slot, not a real node.
static inline char* animationGroupEnd(KeyedTreeNode* group) { return (char*)&((AnimationGroupNode*)group)->animationRoot; }

// Find a numeric animation group, then its named animation or first entry when query is null.
// Returns a borrowed player or null. A null query requires a nonempty group's inner map.
void* CameraWorkTable_FindAnimation(void* self, unsigned int key, const char* query) {
    // Phase 1: lower_bound(key), followed by the reverse comparison to require equality.
    KeyedTreeNode* head = (KeyedTreeNode*)&((CameraGroupStorage*)self)->loadedGroupRoot;
    KeyedTreeNode* node = *(KeyedTreeNode**)head;
    KeyedTreeNode* found = head;
    while (node) {
        if (((AnimationGroupNode*)node)->key >= key) {
            found = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (found != head && key < ((AnimationGroupNode*)found)->key)
        found = head;
    if (found == head)
        return 0;
    if (!query)
        return AsResourceStringNode(((AnimationGroupNode*)found)->firstAnimation)->value;

    // Phase 2: own a temporary query string; the returned player never aliases this storage.
    TemporaryStringStorage temp = {0, 0, 0};
    unsigned qlen = strlen(query);
    String_ReserveCapacity(&temp, qlen);
    unsigned char local = *(unsigned char*)&temp;
    String_ReplaceRange(&temp, 0, 0, query, query + qlen, 0, 0, &local);

    char* cur = *(char**)(animationGroupEnd(found));
    char* parent = animationGroupEnd(found);
    unsigned top = temp.flags >> 31;
    unsigned qsz = *(unsigned char*)&temp.flags & 0x7f; // short length is byte 0 & 0x7F: lbz 0x30(r1) at 0x800E47BC, clrlwi at 0x800E47DC (ours masked the word's low byte)
    const char* tptr = temp.ptr;
    unsigned tlen = temp.len;
    bool isLong = !(top == 0);
    // Phase 3: lower_bound(query); compare prefixes, then lengths, then verify the candidate.
    while (cur) {
        unsigned ql;
        const char* qd;
        if (!isLong) { ql = qsz; qd = (const char*)&temp + 1; }
        else { qd = tptr; ql = tlen; }
        int cmp = compareNodeToQuery(cur, qd, ql, qsz, isLong, 0);
        if (cmp >= 0) { parent = cur; cur = (char*)AsResourceStringNode(cur)->left; }
        else cur = (char*)AsResourceStringNode(cur)->right;
    }
    if (parent != animationGroupEnd(found)) {
        int cmp = compareQueryToNode(parent, &temp, isLong, qsz);
        parent = (cmp >= 0) ? parent : animationGroupEnd(found);
    }
    if (top) appFreeUni__FPv((void*)temp.ptr);
    if (parent == animationGroupEnd(found)) return 0;
    return AsResourceStringNode(parent)->value; // string-tree payload at +0x18
}

} // extern "C"
