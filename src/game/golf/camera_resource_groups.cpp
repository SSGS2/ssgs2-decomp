// Camera resource groups: the named camera players and the power-shot and hole-end camera
// paths loaded for each character, stored in string-keyed trees.
// The file also holds the iterators and teardown code for those trees.

#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/string.h"
#include "classes/CCameraWork.h"
#include "classes/ResourceStringNode.h"
#include "classes/ResourceGroupNode.h"

extern "C" {

// Logical string storage on big-endian PPC (12 bytes overlaid by these ABI views):
//   inline: byte 0 = length & 0x7F; bytes 1..11 = characters, including NUL.
//   heap:   word 0 bit31 set; word 1 = length; word 2 = owned char pointer.
// SSO, TreeString, InlineCameraString and PathSSO are overlapping ABI views,
// not different encodings. SSO.unk_0C at +12 is NOT the inline character start.
// Tree nodes: +0 left, +4 right, +8 packed parent/color; payload begins at +0xC.
// Map header: +0 count, +4 root/end-sentinel address, +8 cached end, +0xC first.
// The three-word string-value specialization stops after the cached link.
struct SSO {
    unsigned int flag;
    unsigned int size;
    unsigned int heapDataWord;
    char unk_0C[1];
};
struct TreeString {
    unsigned int flag;
    unsigned int size;
    char* data;
};
struct Node {
    Node* left;
    Node* right;
    unsigned int parent;
};
struct CameraModelMappingNode {
    Node links;
    TreeString key, value;
};
struct CameraTreeStorage {
    unsigned int count;
    Node* root;
    Node* end;
    Node* first;
};
struct CameraStringPayload {
    TreeString key;
    void* value;
};
struct InlineCameraString {
    unsigned char flags;
    char data[11];
};
struct CameraPathPool {
    char prefix[0x248];
    char emptyPath[1];
    char powerCameraDirectory[28];
    char holeEndCameraDirectory[24];
    char extension[5];
    char groupSeparator[2];
};
enum CameraNodeLayout {
    kCameraNodePayloadOffset = sizeof(Node)
};
enum CameraStringLimits {
    kInlineCameraStringLengthMask = 0x7F,
    kStringHeapBit = 0x80000000
};
struct CameraResourceStateView {
    char prefix[(int)&((CCameraWork*)0)->savedFov + sizeof(float)];
    CameraTreeStorage modelNames;
    char beforeGroupNames[0x20];
    CameraTreeStorage groupNames;
    char beforeLoadedGroups[0x10];
    CameraTreeStorage loadedGroups;
};
enum CameraMapOffsets {
    kModelMapSentinel = (int)&((CameraResourceStateView*)0)->modelNames + (int)&((CameraTreeStorage*)0)->root,
    kResourceGroupNames = (int)&((CameraResourceStateView*)0)->groupNames,
    kLoadedResourceGroups = (int)&((CameraResourceStateView*)0)->loadedGroups
};

int String_ReplaceRange(void*, void*, int, char*, char*, char*);
char* ItemInventory_FindCharacterData(void*, void*); /* (list, key) */
char* CameraResourceGroups_FindCharacterModelMapping(void*, void*);
void MWString_ConstructCString(void*, char*);
void MWString_AppendCString(void*, char*);
void* Storage_ClearThreeWords(void*);
void MWString_AssignCString(void*, char*);
char* ScriptString_GetCharacterData(void*);
void* CameraScript_CreatePlayerForMode(void*, char*, int);
void* StringTree_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
int memcmp(void*, void*, unsigned int);
void String_ReserveCapacity(void*, unsigned int);
void* NamedObjectMap_InsertNode(void*, int, int, int, void*);
void appFreeUni__FPv(void*);
void StringValueTree_DestroySubtree(void*, void*);
void __dl__FPv(void*);
unsigned int strlen(const char*);
void* __nw__FUl(unsigned int);
void __ct__10CVCGPlayerFv(void*);
void CameraMotionGroupVector_Clear(void*);
int CameraScript_LoadFileByPath(void*, const char*);

void* CameraStringValueTree_dtor(void*, int);
void CameraStringValueTree_InitEmpty(void**);
void CameraStringMapIterator_GetEnd(void**, char*);
int CameraStringMapIterator_NotEqual(unsigned int*, unsigned int*);
char* CameraStringMapIterator_GetEntry(char**);
void CameraStringMapIterator_GetFirst(void**, void*);
int CameraResourceString_AppendString(SSO*, SSO*);
void CameraStringMapIterator_PostIncrement(Node**, Node**, Node*);
void* CameraNumericGroupMap_GetOrCreateStringTree(void*, unsigned int*);
void* StringValueTree_GetOrInsertValue(void*, void*);
void CameraResourceLookupTree_Assign(void*, void*);
void* CameraResourceGroupTree_FindInsertionPosition(void*, void*, void**, unsigned char*, unsigned char*);
void* CameraStringValueTree_Copy(void*, void*);
void* CameraWordKeyTree_InsertValueAtLink(void*, void*, unsigned char, unsigned char, void*);
extern unsigned int s_namedObjectMapDefaultValue;
extern char* g_sharedDoc;
extern char cameraWorkStrings[];

struct PathSSO {
    unsigned int flag;
    union {
        char local[4];
        char* data;
    };
    unsigned int heapDataWord;
    // Release heap storage owned by the temporary path string.
    ~PathSSO() {
        ((NString*)(this))->~NString();
    }
};

struct DtorNode {
    DtorNode* left;
    DtorNode* right;
    unsigned int pad;
    unsigned int flag;
    unsigned int size;
    char* data;
};

struct TreeContainer {
    void* root;
    void* mid;
    void* end;
    // Destroy the temporary tree contents while retaining its stack header.
    ~TreeContainer() {
        CameraStringValueTree_dtor(this, -1);
    }
};

#pragma dont_inline on
// Append a packed camera string using its inline or heap character range.
int CameraResourceString_AppendString(SSO* destination, SSO* source) {
    unsigned int sourceLength;
    char* sourceData;
    unsigned int destinationLength;
    if ((destination->flag >> 31) == 0)
        destinationLength = *(unsigned char*)destination & kInlineCameraStringLengthMask;
    else
        destinationLength = destination->size;
    if ((source->flag >> 31) == 0) {
        sourceData = (char*)source + 1;
        sourceLength = *(unsigned char*)source & kInlineCameraStringLengthMask;
    } else {
        sourceData = (char*)source->heapDataWord;
        sourceLength = source->size;
    }
    struct {
        volatile char unused;
        char pad[3];
        char c;
    } local;
    local.c = local.unused;
    return String_ReplaceRange(destination, (char*)(unsigned long)destinationLength, 0, sourceData,
                               sourceData + sourceLength, &local.c);
}
#pragma dont_inline reset

struct DelimSlot {
    char c;
    char pad[3];
    volatile char unused;
};

// Return the unsigned value unchanged at the recovered inline boundary.
static inline unsigned int id_u(unsigned int x) {
    return x;
}
// Borrow the smaller operand without making a new temporary.
static inline const unsigned int& min_ref(const unsigned int& a, const unsigned int& b) {
    return a < b ? a : b;
}

// Replace a string with the borrowed half-open range [b,e).
static inline void assign_range(SSO* s, char* b, char* e) {
    volatile char unused;
    char c = unused;
    String_ReplaceRange(s, 0, 0, b, e, &c);
}
// Return 1 for zero, otherwise 0.
static inline unsigned int is_zero(unsigned int x) {
    return x == 0;
}
// Compare two unsigned lengths and return -1, 0 or 1.
static inline int cmp3(unsigned int a, unsigned int b) {
    if (a < b)
        return -1;
    if (a == b)
        return 0;
    return 1;
}
// Return the path length and expose its borrowed inline or heap characters.
static inline unsigned int path_len_data(unsigned int isHeap, unsigned int local, unsigned int size, char* heapData,
                                         char* localData, char*& data) {
    if (isHeap == 0) {
        data = localData;
        return local;
    }
    data = heapData;
    return size;
}
// Decode the string length without changing ownership.
static inline unsigned int len_of(TreeString* s) {
    if ((s->flag >> 31) == 0)
        return *(unsigned char*)s & kInlineCameraStringLengthMask;
    return s->size;
}
// Decode the length and expose borrowed string characters.
static inline unsigned int len_data(TreeString* s, char*& data) {
    if ((s->flag >> 31) == 0) {
        data = (char*)s + 1;
        return *(unsigned char*)s & kInlineCameraStringLengthMask;
    }
    data = s->data;
    return s->size;
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
#pragma opt_common_subs off
#pragma opt_propagation off
// Resolve the character model name through the camera model-name map.
char* CameraResourceGroups_FindCharacterModelMapping(void* self, void* key) {
    // Build a temporary owned key from the character model name; the result
    // borrows mapped text from the tree and never points into this temporary.
    SSO path;
    unsigned int pathCompareLength;
    unsigned int pathStoredLength;
    unsigned int modelKeyLength;
    unsigned int pathLength;
    char* sharedDoc = g_sharedDoc;
    char* characterRecord =
        (char*)ItemInventory_FindCharacterData((char*)&AsCSharedDoc(sharedDoc)->itemInventory,
                                               key); /* 0x800E158C-0x800E1594: r4 = the incoming key, passed through */
    path.flag = 0;
    path.size = 0;
    path.heapDataWord = 0;
    unsigned int modelNameLength = strlen(characterRecord + IFF_OFFSET(IFF_STRUCT::sChar, model));
    String_ReserveCapacity(&path, modelNameLength);
    assign_range(&path, characterRecord + IFF_OFFSET(IFF_STRUCT::sChar, model),
                 characterRecord + modelNameLength + IFF_OFFSET(IFF_STRUCT::sChar, model));

    unsigned int pathHeap = path.flag >> 31;
    unsigned int pathLocal = *(unsigned char*)&path & kInlineCameraStringLengthMask;
    unsigned int pathSize = path.size;
    char* pathHeapData = (char*)path.heapDataWord;
    unsigned int isHeap = is_zero(is_zero(pathHeap));
#define SENTINEL ((Node*)((char*)self + kModelMapSentinel))
    // Lower bound: remember the smallest key >= the requested model name.
    Node* node = *(Node**)SENTINEL;
    Node* candidate = SENTINEL;
    while (node) {
        char* pathData;
        unsigned int storedKeyLength;
        const unsigned int pathLen =
            path_len_data(isHeap, pathLocal, pathSize, pathHeapData, (char*)&path + 1, pathData);
        TreeString* key = &((CameraModelMappingNode*)node)->key;
        char* keyData;
        storedKeyLength = len_of(key);
        unsigned int keyDataLength = len_data(key, keyData);
        unsigned int keyCompareLength = min_ref(storedKeyLength, keyDataLength);
        unsigned int compareBytes = min_ref(pathLen, keyCompareLength);
        int comparison = memcmp(keyData, pathData, compareBytes);
        if (comparison == 0)
            comparison = cmp3(keyCompareLength, pathLen);
        if (comparison >= 0) {
            candidate = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    // Reject a strictly greater candidate before exposing the mapped name.
    if (candidate != SENTINEL) {
        TreeString* key = &((CameraModelMappingNode*)candidate)->key;
        char* keyData;
        const unsigned int kLen = len_data(key, keyData);
        modelKeyLength = kLen;
        pathLength = isHeap == 0 ? pathLocal : path.size;
        char* pathData;
        if (isHeap == 0) {
            pathData = (char*)&path + 1;
        } else {
            pathData = (char*)path.heapDataWord;
            pathLocal = path.size;
        }
        pathStoredLength = pathLocal;
        pathCompareLength = min_ref(pathLength, pathStoredLength);
        unsigned int compareBytes = min_ref(modelKeyLength, pathCompareLength);
        int comparison = memcmp(pathData, keyData, compareBytes);
        if (comparison == 0)
            comparison = cmp3(pathCompareLength, kLen);
        if (comparison < 0)
            candidate = SENTINEL;
    }
    if (pathHeap)
        appFreeUni__FPv((void*)path.heapDataWord);
    if (candidate != SENTINEL) {
        TreeString* value = &((CameraModelMappingNode*)candidate)->value;
        if ((value->flag >> 31) == 0)
            return (char*)value + 1;
        return value->data;
    }
    return 0;
}
#pragma pop
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset

struct PlayerObj {
    char unk[0x30];
    // Initialize the camera-script player base in the new allocation.
    PlayerObj() {
        __ct__10CVCGPlayerFv(this);
    }
};

struct PlayerVt {
    virtual void del(int);
};

#pragma push
#pragma use_lmw_stmw on
// Build a power-shot or hole-end camera path, create its player, and release it if loading fails.
void* CameraScript_CreatePlayerForMode(void* unused, char* value, int mode) {
    SSO path;
    path.flag = 0;
    path.size = 0;
    path.heapDataWord = 0;
    char* pathPart = ((CameraPathPool*)cameraWorkStrings)->emptyPath;
    unsigned int pathLength = strlen(pathPart);
    String_ReserveCapacity(&path, pathLength);
    DelimSlot d0;
    d0.c = d0.unused;
    String_ReplaceRange(&path, (void*)0, 0, pathPart, pathPart + pathLength, &d0.c);
    struct StringCleanup {
        SSO* object;
        // Release the guarded temporary string when leaving this scope.
        ~StringCleanup() {
            if (object->flag >> 31)
                appFreeUni__FPv((void*)object->heapDataWord);
        }
    } cleanup = {&path};
    if (mode != 0) {
        pathPart = ((CameraPathPool*)cameraWorkStrings)->powerCameraDirectory;
        pathLength = (path.flag >> 31) == 0 ? *(unsigned char*)&path & kInlineCameraStringLengthMask : path.size;
        DelimSlot d1;
        d1.c = d1.unused;
        unsigned int partLength = strlen(pathPart);
        String_ReplaceRange(&path, (void*)(unsigned long)pathLength, 0, pathPart, pathPart + partLength, &d1.c);
    } else {
        pathPart = ((CameraPathPool*)cameraWorkStrings)->holeEndCameraDirectory;
        pathLength = (path.flag >> 31) == 0 ? *(unsigned char*)&path & kInlineCameraStringLengthMask : path.size;
        DelimSlot d2;
        d2.c = d2.unused;
        unsigned int partLength = strlen(pathPart);
        String_ReplaceRange(&path, (void*)(unsigned long)pathLength, 0, pathPart, pathPart + partLength, &d2.c);
    }
    pathLength = (path.flag >> 31) == 0 ? *(unsigned char*)&path & kInlineCameraStringLengthMask : path.size;
    DelimSlot d3;
    d3.c = d3.unused;
    unsigned int valueLen = strlen(value);
    String_ReplaceRange(&path, (void*)(unsigned long)pathLength, 0, value, value + valueLen, &d3.c);
    pathPart = ((CameraPathPool*)cameraWorkStrings)->extension;
    pathLength = (path.flag >> 31) == 0 ? *(unsigned char*)&path & kInlineCameraStringLengthMask : path.size;
    DelimSlot d4;
    d4.c = d4.unused;
    unsigned int partLength = strlen(pathPart);
    String_ReplaceRange(&path, (void*)(unsigned long)pathLength, 0, pathPart, pathPart + partLength, &d4.c);
    PlayerObj* player = new PlayerObj();
    void* result = player;
    CameraMotionGroupVector_Clear(player);
    char* assembledPath = (path.flag >> 31) == 0 ? (char*)&path + 1 : (char*)path.heapDataWord;
    int loaded = CameraScript_LoadFileByPath(player, assembledPath);
    if (loaded == 0) {
        if (player != 0) {
            if (player != 0) {
                ((PlayerVt*)player)->del(1);
            }
            result = 0;
        }
    }
    return result;
}
#pragma pop

#pragma opt_loop_invariants off
#pragma opt_common_subs off
// Load named camera players for a character and assign the completed group to its numeric key.
void CameraResourceGroups_LoadPlayerResourceGroup(void* self, void* characterKey) {
    PathSSO modelPrefix;
    char* base = CameraResourceGroups_FindCharacterModelMapping(self, characterKey); /* 0x800E1870: r4 = characterKey */
    MWString_ConstructCString(&modelPrefix, base);
    MWString_AppendCString(&modelPrefix, ((CameraPathPool*)cameraWorkStrings)->groupSeparator);

    TreeContainer loadedPlayers;
    CameraStringValueTree_InitEmpty((void**)&loadedPlayers);

    PathSSO resourceName;
    Storage_ClearThreeWords(&resourceName);

    void* nameIterator;
    CameraStringMapIterator_GetFirst(&nameIterator, (char*)self + kResourceGroupNames);

    void* previousIterator;
    void* endIterator;

    for (; (CameraStringMapIterator_GetEnd((void**)&endIterator, (char*)self + kResourceGroupNames),
            CameraStringMapIterator_NotEqual((unsigned int*)&nameIterator, (unsigned int*)&endIterator)) != 0;
         CameraStringMapIterator_PostIncrement((Node**)&previousIterator, (Node**)&nameIterator, (Node*)0)) {
        char* prefixText = ScriptString_GetCharacterData(&modelPrefix);
        MWString_AssignCString(&resourceName, prefixText);
        char* groupName = CameraStringMapIterator_GetEntry((char**)&nameIterator);
        CameraResourceString_AppendString((SSO*)&resourceName, (SSO*)(groupName + 0xC));
        char* resourcePath = ScriptString_GetCharacterData(&resourceName);
        void* player = CameraScript_CreatePlayerForMode(self, resourcePath, 0);
        void* loadedPlayer = player;
        char* groupKey = CameraStringMapIterator_GetEntry((char**)&nameIterator);
        void* valueSlot = StringValueTree_GetOrInsertValue(&loadedPlayers, groupKey);
        *(void**)valueSlot = loadedPlayer;
    }

    void* destinationTree =
        CameraNumericGroupMap_GetOrCreateStringTree((char*)self + kLoadedResourceGroups, (unsigned int*)&characterKey);
    CameraResourceLookupTree_Assign(destinationTree, &loadedPlayers);
}
#pragma opt_loop_invariants reset
#pragma opt_common_subs reset

// Return the tree sentinel used as the end iterator.
void CameraStringMapIterator_GetEnd(void** out, char* value) {
    *out = value + (int)&((CameraTreeStorage*)0)->root;
}

// Return the key/value payload following an iterator node header.
char* CameraStringMapIterator_GetEntry(char** value) {
    return *value + sizeof(Node);
}

// Return the first node of the string map.
void CameraStringMapIterator_GetFirst(void** out, void* value) {
    *out = ((CameraTreeStorage*)value)->first;
}

// Initialize an empty string-value tree with a self-referencing sentinel.
void CameraStringValueTree_InitEmpty(void** value) {
    // count=0, root=null, cached link=&root. The sentinel is embedded, not allocated.
    value[0] = 0;
    value[1] = 0;
    value[2] = value + 1;
}

// Cleanup expansion, in postorder: left descendants, right descendants, the
// node's owned heap string (inline strings need no free), then the node itself.
// LEAF delegates deeper descendants; BRANCH expands one more level. Repeated
// self/payload checks mirror nested container/string destructor boundaries.
// Release the owned heap text while retaining the original nested null checks.
static inline void ReleaseCameraNodeText(DtorNode* node) {
    if ((char*)node + kCameraNodePayloadOffset) {
        if ((char*)node + kCameraNodePayloadOffset) {
            if (node->flag >> 31)
                appFreeUni__FPv(node->data);
        }
    }
}
// Free child subtrees, then owned text, then this node; self is borrowed tree storage.
static inline void ReleaseCameraLeaf(void* self, DtorNode* node) {
    if (node) {
        if (node->left)
            StringValueTree_DestroySubtree(self, node->left);
        if (node->right)
            StringValueTree_DestroySubtree(self, node->right);
        ReleaseCameraNodeText(node);
        appFreeUni__FPv(node);
    }
}
// Keep this single scope-expansion macro: extracting the branch changes register lifetimes.
#define DTOR_BRANCH(n)                      \
    if (n) {                                \
        DtorNode* leftLeaf = (n)->left;     \
        ReleaseCameraLeaf(self, leftLeaf);  \
        DtorNode* rightLeaf = (n)->right;   \
        ReleaseCameraLeaf(self, rightLeaf); \
        ReleaseCameraNodeText(n);           \
        appFreeUni__FPv(n);                 \
    }
#pragma push
#pragma use_lmw_stmw on
// Destroy string keys and free all tree nodes, then optionally delete the tree storage.
void* CameraStringValueTree_dtor(void* self, int deleting) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            // Phase 1: destroy the left and right subtrees before releasing the root.
                            DtorNode* root = (DtorNode*)((TreeContainer*)self)->mid;
                            if (root) {
                                DtorNode* leftSubtree = root->left;
                                if (leftSubtree) {
                                    DtorNode* leftBranch = leftSubtree->left;
                                    DTOR_BRANCH(leftBranch)
                                    DtorNode* rightBranch = leftSubtree->right;
                                    DTOR_BRANCH(rightBranch)
                                    ReleaseCameraNodeText(leftSubtree);
                                    appFreeUni__FPv(leftSubtree);
                                }
                                DtorNode* rightSubtree = root->right;
                                if (rightSubtree) {
                                    DtorNode* leftBranch = rightSubtree->left;
                                    DTOR_BRANCH(leftBranch)
                                    DtorNode* rightBranch = rightSubtree->right;
                                    DTOR_BRANCH(rightBranch)
                                    ReleaseCameraNodeText(rightSubtree);
                                    appFreeUni__FPv(rightSubtree);
                                }
                                ReleaseCameraNodeText(root);
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        // Phase 2: only a positive deleting flag frees the header allocation.
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop
#undef DTOR_BRANCH

// Compare iterator node addresses and return a normalized nonzero result when different.
int CameraStringMapIterator_NotEqual(unsigned int* a, unsigned int* b) {
    unsigned int y;
    unsigned int x;
    x = *a;
    y = *b;
    return ((y - x) | (x - y)) >> 31;
}

// Save the old iterator and advance to the in-order successor, masking the parent colour bit.
void CameraStringMapIterator_PostIncrement(Node** out, Node** slot, Node* fallback) {
    *out = *slot;
    Node* right = (*slot)->right;
    if (right != 0) {
        Node* n = right;
        while (n->left != 0)
            n = n->left;
        *slot = n;
        return;
    }
    Node* node = *slot;
    Node* parent;
    while ((parent = (Node*)(node->parent & ~1u)), node != parent->left) {
        *slot = parent;
        node = parent;
    }
    *slot = parent;
    return;
    (void)fallback;
}

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
    unsigned int pad;
    unsigned int flag;
    unsigned int size;
    void* data;
};

// Retain the unused recursive cleanup expansion, including repeated subtree calls.
static void clear_tree(TreeNode* n, void* owner) {
    if (!n)
        return;
    clear_tree(n->left, owner);
    clear_tree(n->right, owner);
    if (n->left)
        StringValueTree_DestroySubtree(owner, n->left);
    if (n->right)
        StringValueTree_DestroySubtree(owner, n->right);
    if (n->flag & kStringHeapBit)
        appFreeUni__FPv(n->data);
    appFreeUni__FPv(n);
}

// Destroy both child subtrees, then free this node and its heap string.
static void del_tree_node(TreeNode* n, void* owner) {
    if (!n)
        return;
    TreeNode* a = n->left;
    if (a) {
        if (a->left)
            StringValueTree_DestroySubtree(owner, a->left);
        if (a->right)
            StringValueTree_DestroySubtree(owner, a->right);
        if (a->flag & kStringHeapBit)
            appFreeUni__FPv(a->data);
        appFreeUni__FPv(a);
    }
    TreeNode* b = n->right;
    if (b) {
        if (b->left)
            StringValueTree_DestroySubtree(owner, b->left);
        if (b->right)
            StringValueTree_DestroySubtree(owner, b->right);
        if (b->flag & kStringHeapBit)
            appFreeUni__FPv(b->data);
        appFreeUni__FPv(b);
    }
    if (n->flag & kStringHeapBit)
        appFreeUni__FPv(n->data);
    appFreeUni__FPv(n);
}

// Cleanup expansion, in postorder: left descendants, right descendants, the
// node's owned heap string (inline strings need no free), then the node itself.
// LEAF delegates deeper descendants; BRANCH expands one more level. Repeated
// self/payload checks mirror nested container/string destructor boundaries.
#define ReleaseCameraNodeText(n)            \
    ;                                       \
    if ((char*)(n) + 12) {                  \
        if ((char*)(n) + 12) {              \
            if ((n)->flag >> 31)            \
                appFreeUni__FPv((n)->data); \
        }                                   \
    }
#define ReleaseCameraLeaf(self, n)                            \
    ;                                                         \
    if (n) {                                                  \
        if ((n)->left)                                        \
            StringValueTree_DestroySubtree(self, (n)->left);  \
        if ((n)->right)                                       \
            StringValueTree_DestroySubtree(self, (n)->right); \
        ReleaseCameraNodeText(n);                             \
        appFreeUni__FPv(n);                                   \
    }
// Keep this single scope-expansion macro: extracting the branch changes register lifetimes.
#define DTOR_BRANCH(n)                      \
    if (n) {                                \
        DtorNode* leftLeaf = (n)->left;     \
        ReleaseCameraLeaf(self, leftLeaf);  \
        DtorNode* rightLeaf = (n)->right;   \
        ReleaseCameraLeaf(self, rightLeaf); \
        ReleaseCameraNodeText(n);           \
        appFreeUni__FPv(n);                 \
    }

struct InlTree {
    void* first;
    DtorNode* root;
    void* end;
    // Initialize an empty nested tree with its cached link pointing at root.
    InlTree() {
        first = 0;
        root = 0;
        end = &root;
    }
    // Release nested string-tree nodes in left/right/node postorder.
    ~InlTree() {
        InlTree* self = this;
        DtorNode* r = root;
        if (r) {
            DtorNode* a = r->left;
            if (a) {
                DtorNode* b = a->left;
                DTOR_BRANCH(b)
                DtorNode* right = a->right;
                DTOR_BRANCH(right)
                ReleaseCameraNodeText(a);
                appFreeUni__FPv(a);
            }
            DtorNode* other = r->right;
            if (other) {
                DtorNode* b = other->left;
                DTOR_BRANCH(b)
                DtorNode* right = other->right;
                DTOR_BRANCH(right)
                ReleaseCameraNodeText(other);
                appFreeUni__FPv(other);
            }
            ReleaseCameraNodeText(r);
            appFreeUni__FPv(r);
        }
    }
};

struct InlPair {
    unsigned int groupKey;
    InlTree tree;
};

#pragma push
#pragma use_lmw_stmw on
// Find the numeric camera group, inserting an empty nested string tree when absent.
void* CameraNumericGroupMap_GetOrCreateStringTree(void* self, unsigned int* groupKey) {
    unsigned char insertLeft, insertRight;
    void* candidate;
    TreeNode* insertionParent =
        (TreeNode*)CameraResourceGroupTree_FindInsertionPosition(self, groupKey, &candidate, &insertLeft, &insertRight);
    TreeNode* groupPayload = 0;
    if (candidate != 0 && *groupKey <= ((ResourceGroupNode*)candidate)->key) {
        groupPayload = (TreeNode*)&((ResourceGroupNode*)candidate)->key;
    }
    if (!groupPayload) {
        InlTree emptyTree;
        InlPair newGroup;
        newGroup.groupKey = *groupKey;
        CameraStringValueTree_Copy(&newGroup.tree, &emptyTree);
        TreeNode* insertedNode =
            (TreeNode*)CameraWordKeyTree_InsertValueAtLink(self, insertionParent, insertLeft, insertRight, &newGroup);
        groupPayload = (TreeNode*)&((ResourceGroupNode*)insertedNode)->key;
    }
    return (char*)groupPayload + 4;
}
#pragma pop

struct InsertString {
    unsigned int flag, size;
    char* data;
    // Copy an inline string or independently allocate its heap representation.
    InsertString(const InsertString& other, char* allocatorTag) {
        init(other, allocatorTag);
    }
    // Copy the source string representation, deep-copying heap characters.
    void init(const InsertString& other, char* allocatorTag) {
        if ((other.flag >> 31) == 0) {
            unsigned int a = other.flag, b = other.size;
            char* c = other.data;
            flag = a;
            size = b;
            data = c;
        } else {
            flag = 0;
            unsigned int n = other.size;
            size = 0;
            data = 0;
            String_ReserveCapacity(this, n);
            unsigned char c = *(volatile unsigned char*)allocatorTag;
            char* begin = other.data;
            unsigned int len = other.size;
            allocatorTag[4] = c;
            String_ReplaceRange(this, 0, 0, begin, begin + len, allocatorTag + 4);
        }
    }
    // Free owned heap characters; inline characters need no cleanup.
    ~InsertString() {
        if (flag >> 31)
            appFreeUni__FPv(data);
    }
};

struct InsertFrame {
    char allocatorTagBytes[8];
    unsigned char insertRight, insertLeft;
    char pad[2];
    unsigned int storedCompareLength, storedDataLength, requestedLength, storedLength;
    void* candidate;
    unsigned int key[3];
    unsigned int value;
};

#pragma push
#pragma peephole on
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Find a string key or copy and insert it with the default value, returning its value slot.
void* StringValueTree_GetOrInsertValue(void* self, void* requestedKey) {
    // Keep local declaration IDs stable for the weak unwind destructor below.
    // These unused scalar slots emit no code and carry no lookup state.
    unsigned abiSlot00, abiSlot01, abiSlot02, abiSlot03, abiSlot04, abiSlot05, abiSlot06, abiSlot07, abiSlot08,
        abiSlot09, abiSlot10, abiSlot11, abiSlot12, abiSlot13, abiSlot14, abiSlot15, abiSlot16, abiSlot17, abiSlot18,
        abiSlot19, abiSlot20, abiSlot21, abiSlot22;
    InsertFrame frame;

    void* insertionParent =
        StringTree_FindInsertionPosition(self, requestedKey, &frame.candidate, &frame.insertLeft, &frame.insertRight);
    void* candidateOrKeyData = frame.candidate;
    void* storedKey = 0;
    unsigned int requestedLength = 0;
    // An equal lower-bound candidate exits the lookup; missing/greater keys insert.
    // candidateOrKeyData holds a node until key decoding, then borrowed character bytes.
    // This structured join also preserves the compiler-local cleanup symbol identity.
    do {
        if (candidateOrKeyData != 0) {
            unsigned int stringFlagsOrInlineLength = *(unsigned int*)requestedKey;
            storedKey = &((ResourceStringNode*)candidateOrKeyData)->stringFlags;
            int heap = (stringFlagsOrInlineLength >> 31) & 1;
            if (!heap) {
                stringFlagsOrInlineLength = *(unsigned char*)requestedKey;
                candidateOrKeyData = (char*)requestedKey + 1;
                requestedLength = stringFlagsOrInlineLength & kInlineCameraStringLengthMask;
            } else {
                candidateOrKeyData = ((TreeString*)requestedKey)->data;
                requestedLength = ((TreeString*)requestedKey)->size;
            }
            frame.requestedLength = requestedLength;
            stringFlagsOrInlineLength = *(unsigned int*)storedKey;
            unsigned int storedLength;
            if ((stringFlagsOrInlineLength >> 31) == 0) {
                stringFlagsOrInlineLength = *(unsigned char*)storedKey;
                storedLength = stringFlagsOrInlineLength & kInlineCameraStringLengthMask;
            } else {
                storedLength = ((TreeString*)storedKey)->size;
            }
            frame.storedLength = storedLength;
            stringFlagsOrInlineLength = *(unsigned int*)storedKey;
            heap = (stringFlagsOrInlineLength >> 31) & 1;
            void* storedData;
            unsigned int storedDataLength;
            if (!heap) {
                storedData = (char*)storedKey + 1;
                storedDataLength = *(unsigned char*)storedKey & kInlineCameraStringLengthMask;
            } else {
                storedData = ((TreeString*)storedKey)->data;
                storedDataLength = ((TreeString*)storedKey)->size;
            }
            unsigned int* storedLengthPointer = &frame.storedDataLength;
            frame.storedDataLength = storedDataLength;
            if (storedLength < storedDataLength)
                storedLengthPointer = &frame.storedLength;
            frame.storedCompareLength = *storedLengthPointer;
            unsigned int* comparisonLengthPointer = &frame.storedCompareLength;
            if (requestedLength < frame.storedCompareLength)
                comparisonLengthPointer = &frame.requestedLength;
            unsigned int comparisonBytes = *comparisonLengthPointer;
            int comparison = memcmp(storedData, candidateOrKeyData, comparisonBytes);
            if (comparison == 0) {
                if (frame.storedCompareLength < requestedLength)
                    comparison = -1;
                else if (frame.storedCompareLength == requestedLength)
                    comparison = 0;
                else
                    comparison = 1;
            }
            if (comparison >= 0) {
                break;
            }
        }
        // Insert only after the candidate comparison failed; the key copy is owned
        // by the cleanup guard until InsertNode has copied it into its own node.
        InsertString* copiedKey = (InsertString*)frame.key;
        copiedKey->init(*(InsertString*)requestedKey, frame.allocatorTagBytes);
        struct StringCleanup {
            unsigned int* words;
            // Release the guarded temporary string when leaving this scope.
            ~StringCleanup() {
                if (words[0] >> 31)
                    appFreeUni__FPv((void*)words[2]);
            }
        } cleanup = {frame.key};
        frame.value = s_namedObjectMapDefaultValue;
        void* insertedNode = NamedObjectMap_InsertNode(self, (int)insertionParent, (int)frame.insertLeft,
                                                       (int)frame.insertRight, &frame.key);
        storedKey = &((ResourceStringNode*)insertedNode)->stringFlags;
        break;
    } while (0);
    // Borrow the node value slot; erase/tree destruction invalidates the result.
    return &((CameraStringPayload*)storedKey)->value;
}
#pragma pop

} // extern "C"
