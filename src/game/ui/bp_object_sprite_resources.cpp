// BP/B2D sprite resources: loading, script playback, texture/group caches and draw admission.
// Modding lookup (textures / fonts / prompt sheets): BpObject_Draw is the common
// admission gate; it does not define sprite rectangles or atlas resolution.
// Rectangles/UVs reach sprite submission through script entries in b2d_sprite_and_bp_texture_draw.cpp;
// B2D/PHF bytes and named GSR textures are registered by BpObject_PollLoad here.
// Keep sheet texels, script source rectangles, group names and texture IDs in
// agreement when replacing prompts/fonts. Resizing only a sheet changes sampling,
// not layout. Camera aspect / GX viewport/scissor are selected by other units.
// Entry points: CBpLoad::SetFilenameAndOpenSharedFile and
// CBpObject::SetScriptResourcePath retains paths; BpObject_PollLoad
// owns command work but borrows shared resource bytes; GetOrCreateNamedGroup /
// UIGroup_GetOrCreateById return cache-owned groups invalidated by ClearResources.
// Shared layouts: CBpObject.h, CBpLoad.h, BpTreeLinks.h, BpIntObjectNode.h.
// Functions taking self require initialized objects; keys/paths must be valid
// NUL-terminated strings unless explicitly nullable. All external calls retain
// their original opaque ABI. TableLock(1,1) means GLOBAL mutex table index 1,
// active=true, not two modes or a per-object mutex (wview_camera_projection.cpp TableLock_ctor).
// Its OSMutex is recursive: Draw->PollLoad and ClearResources' nested scopes
// reacquire the same mutex. Each guard unlocks on return or exception; lazy
// first initialization itself must be serialized. Coverage in this unit:
// - whole calls: ResetSpriteVertexCountsLocked, IsCurrentScriptComplete,
//   FindTextureById, Draw, StartScriptByName, SetScriptResourcePath,
//   ReplaceObjectById, TryInvokeCurrentCallback, both GetOrCreate group APIs,
//   ClearResources and PollLoad (including their allocation/callback work).
// - SetEnabled and UpdateAndDrawNodeList take NO table guard themselves;
//   integer insertion-position search is also unguarded. Loader polling uses
//   the distinct loader-instance mutex only around the shared-file access.
// A returned group/texture is borrowed AFTER unlock: table-locking a lookup
// does not protect its later use against reset. Serialize that lifetime too.
// No gameplay clock lives here.
#include "classes/CBpLoad.h"
#include "classes/BpTreeLinks.h"
#include "classes/BpIntObjectNode.h"
#include "classes/CBpObject.h"
// Borrow the loader layout; its mutex protects access to the shared file handle.
static inline CBpLoad* AsCBpLoad(void* object) {
    return (CBpLoad*)object;
}
// Borrow the sprite/script owner layout without adjusting or retaining its pointer.
static inline CBpObject* AsCBpObject(void* object) {
    return (CBpObject*)object;
}

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBpCtrl.h"
#include "classes/CBpEditSprite.h"

// Construct a tree payload in caller-provided storage.
inline void* operator new(size_t, void* p) throw() {
    return p;
}
// Placement-delete cleanup leaves that storage with the caller.
inline void operator delete(void*, void*) throw() {}

extern "C" {

enum BpLoadState {
    kLoadWaitingForFile = 1,
    kLoadWaitingForObjects = 2,
    kLoadReady = 3
};
// Load state is separate from script playback state. PollLoad advances at most
// one state per call: 1 waits for shared bytes and prepares work, 2 polls each
// mapped object's readiness, 3 returns ready. Transitions themselves return 0.
enum BpScriptState {
    kScriptResourceSet = 2,
    kScriptRunning = 3
};
enum BpStringConstants {
    kInlineStringLengthMask = 0x7F,
    kStringHeapTagShift = 31,
    kTreeRootLinkOffset = 4,
    kTreePayloadOffset = 0xC
};
enum BpStringOffset {
    kB2dExtension = 0,
    kWorkSizeDiagnostic = 5,
    kRegisterSpritesScript = 0x3B,
    kPathSeparator = 0x4A,
    kEmptyPath = 0x4C,
    kMissingPlayEndDiagnostic = 0x4D,
    kGroupIdFormat = 0x72,
    kSpriteIdPrefix = 0x77,
    kTreeLengthError = 0x82,
    kLengthErrorType = 0x9C
};
struct BpScriptEntryView {
    int entryType;
    int id;
    const char* groupName;
    char entryState[8];
    void* next;
};
struct BpControllerStateView {
    char baseStorage[0x14];
    int updateState;
};

void* B2d_SelectHighestCommandTotalItem(void*);
int B2dGroup_IsActive(void*);
void UIRender_SetDefaultState(void*);
int BpObject_PollLoad(void*);
void* BpSpritePool_DrawActiveSprites(void*);
extern char bpSpriteControlStrings[];

struct C6E60Object {
    char padding00[0x24];
    void* objectArray;
    char padding28[0x8];
    int enabled;
};

void ObjectArray_DeleteAllAndClear(void*);

// Set drawing enablement; disabling also deletes and clears all pooled sprite objects.
void BpObject_SetEnabled(C6E60Object* self, int value) {
    self->enabled = value;
    if (value == 0)
        ObjectArray_DeleteAllAndClear(self->objectArray);
}

struct C6288Object {
    char padding00[0x24];
    void* objectArray;
};

void TableLock_ctor(void*, int, int);
void BpSpritePool_MarkAllInactive(void*);

struct TableLock {
    char data[8];

    // Acquire the shared BP table lock with the original lock-mode arguments.
    TableLock(int a, int b) {
        TableLock_ctor(this, a, b);
    }
    ~TableLock();
};

// Reset pooled sprite vertex counts while holding the global BP table lock.
void CBpObject::ResetSpriteVertexCountsLocked() {
    C6288Object* self = (C6288Object*)this;
    TableLock temp(1, 1);
    BpSpritePool_MarkAllInactive(self->objectArray);
}

struct C6234Object {
    char padding00[4];
    void* script;
    char padding08[0x1c];
    void* objectArray;
    char padding28[0xc];
    int callbackListEmpty;
};

void BpSpritePool_MarkAllInactive(void*);
int BpCtrl_UpdateAndDrawNodeList(void*, int, void*);

// Reset sprite vertices and update/draw the script node list, recording when no nodes remain.
int BpObject_UpdateAndDrawNodeList(C6234Object* self) {
    BpSpritePool_MarkAllInactive(self->objectArray);
    if (BpCtrl_UpdateAndDrawNodeList(self->script, 0, ((char*)&AsCBpObject(self)->callbackPosition[0])) == 0)
        self->callbackListEmpty = 1;
    return 1;
}

struct BpIntegerTreeHeader_u800C4C08 {
    char padding00[4];
    void* root;
};

// Find insertion parent/side and the greatest existing key <= the requested integer key.
// Output flags: insertOnLeft is the final search edge; allVisitedKeysGreater
// stays one only when no visited key was <= the request (including an empty tree).
void* BpIntObjectTree_FindInsertionPosition(void* tree, const int* key, void** greatestNotAboveKey,
                                            unsigned char* insertOnLeft, unsigned char* allVisitedKeysGreater) {
    *greatestNotAboveKey = 0;
    void* insertionParent = (char*)tree + kTreeRootLinkOffset;
    void* node = ((BpIntegerTreeHeader_u800C4C08*)tree)->root;
    *insertOnLeft = 1;
    *allVisitedKeysGreater = 1;
    while (node != 0) {
        insertionParent = node;
        int nodeKey = AsBpIntObjectNode(node)->key; // original node +0xc
        if (*key < nodeKey) {
            node = *(void**)node;
            *insertOnLeft = 1;
        } else {
            *greatestNotAboveKey = node;
            node = AsBpTreeLinks(node)->right;
            *insertOnLeft = 0;
            *allVisitedKeysGreater = 0;
        }
    }
    return insertionParent;
}

struct C5C00Object {
    char padding00[4];
    int loaded;
    char padding08[0xc];
    void* sharedFileHandle;
};

void* CSharedFilePtr_GetLoadedBuffer(void*);

struct WInstanceLock {
    void* lock;
    // Borrow and acquire a loader's instance mutex.
    WInstanceLock(void* l) : lock(l) {
        OSLockMutex((OSMutex*)l);
    }
    // Unlock the borrowed mutex at scope exit.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
};

// Poll a shared file under the loader mutex and latch completion once its buffer exists.
int CBpLoad::PollLoad() {
    C5C00Object* self = (C5C00Object*)this;
    if (self->loaded != 0)
        return 1;
    void* result;
    {
        WInstanceLock guard((char*)&AsCBpLoad(self)->mutexStorage);
        result = CSharedFilePtr_GetLoadedBuffer(self->sharedFileHandle);
    }
    if (result == 0)
        return 0;
    self->loaded = 1;
    return 1;
}

// Retail BP diagnostic hook; formatted messages are discarded.
void BpObject_DiagnosticStub(int, ...) {}

struct C5F18Object {
    char padding00[4];
    void* script;
};

// Report completion when no script is selected or the current script is no longer playing.
int BpObject_IsCurrentScriptComplete(C5F18Object* self) {
    TableLock temp(1, 1);
    void* currentScript = B2d_SelectHighestCommandTotalItem(self->script);
    if (currentScript == 0) {
        BpObject_DiagnosticStub((int)(bpSpriteControlStrings + kMissingPlayEndDiagnostic));
        return 1;
    }
    int isPlaying = B2dGroup_IsActive(currentScript);
    return (isPlaying == 0) ? 1 : 0;
}

// Find an exact integer-keyed object value, returning null if the lower bound has a different key.
// Returns a borrowed map value. Texture interpretation is supported by both
// callers in b2d_sprite_and_bp_texture_draw.cpp (descriptor lookup/quad submission), not checked by this tree.
void* CBpObject::FindTextureById(int key) {
    TableLock temp(1, 1);
    void* node = objectRoot;
    void* parent = &objectRoot;
    while (node != 0) {
        int nodeKey = AsBpIntObjectNode(node)->key; // original node +0xc
        if (nodeKey >= key) {
            parent = node;
            node = *(void**)node;
        } else {
            node = AsBpTreeLinks(node)->right;
        }
    }
    if (parent != &objectRoot) {
        int pKey = AsBpIntObjectNode(parent)->key;
        if (key < pKey) {
            parent = &objectRoot;
        }
    }
    void* result;
    if (parent == &objectRoot) {
        result = 0;
    } else {
        result = AsBpIntObjectNode(parent)->value; // original node +0x10
    }
    return result;
}

struct C5508Object {
    char padding00[0x24];
    void* objectArray;
    char padding28[0x8];
    int enabled;
    char padding34[0x8];
    int drawnThisFrame;
};

// Submit enabled sprites once per externally reset draw latch, after load readiness.
// self is required; disabled returns 1 without loading/drawing, already submitted
// or not ready returns 0, otherwise returns the sprite-pool draw result as an ABI
// word. drawnThisFrame is set BEFORE submission, even if the pool returns zero.
// This method never clears that latch: preserve its reset protocol in the caller
// when changing frame cadence. Holds TableLock across polling and submission.
// Does not own a sheet or select resolution/UVs; those belong to script entries.
int BpObject_Draw(C5508Object* self) {
    TableLock temp(1, 1);
    if (self->enabled == 0) {
        return 1;
    }
    if (self->drawnThisFrame != 0) {
        return 0;
    }
    int loadReady = BpObject_PollLoad(self);
    if (loadReady == 0) {
        return 0;
    }
    self->drawnThisFrame = 1;
    UIRender_SetDefaultState((void*)loadReady);
    void* drawResult = BpSpritePool_DrawActiveSprites(self->objectArray);
    return (int)drawResult;
}

int B2dFile_ExecuteNamedScriptWithControlFlags(void*, int);
void BpSlot_ResetAnimationChannelValues(void*);
void ValueAnimation_SetUpdateMode4();
int MWString_ReverseFindSubstring(void*, const char*, int, int);
void String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, void*);
void* SharedFile_Create(void*, int);

struct C57D0Object {
    char padding00[4];
    void* script;
    char padding08[0xc];
    int scriptState;
};

// Start a named script after loading; optionally reset animation channels and mark the script running.
int BpObject_StartScriptByName(C57D0Object* self, int scriptName, int resetAnimationChannels) {
    TableLock temp(1, 1);
    if (BpObject_PollLoad(self) == 0) {
        return 0;
    }
    int scriptIndex = B2dFile_ExecuteNamedScriptWithControlFlags(self->script, scriptName);
    if (scriptIndex < 0) {
        return 0;
    }
    if (resetAnimationChannels != 0) {
        BpSlot_ResetAnimationChannelValues(self->script);
    }
    ValueAnimation_SetUpdateMode4();
    self->scriptState = kScriptRunning;
    return 1;
}

struct Frame5E50 {
    unsigned char argument;
    char padding0[3];
    unsigned char source;
    char padding1[3];
    WInstanceLock guard;
    // Preserve allocator scratch while holding the loader filename mutex.
    Frame5E50(void* lock) : guard(lock) {}
};

struct C5E50Object {
    char padding00[8];
    unsigned int filenameTag;
    int filenameLongLength;
    void* filenameHeapData;
    void* sharedFileHandle;
};

// Replace the loader filename and create its shared-file handle under the instance mutex.
int CBpLoad::SetFilenameAndOpenSharedFile(const char* filename) {
    C5E50Object* self = (C5E50Object*)this;
    {
        Frame5E50 local((char*)&AsCBpLoad(self)->mutexStorage);
        unsigned int previousLength;
        if ((self->filenameTag >> kStringHeapTagShift) == 0) {
            previousLength = *(unsigned char*)&self->filenameTag & kInlineStringLengthMask;
        } else {
            previousLength = (unsigned int)self->filenameLongLength;
        }
        local.argument = local.source;
        unsigned int filenameLength = strlen(filename);
        String_ReplaceRange((char*)&AsCBpLoad(self)->filenameStorage, 0, previousLength, filename,
                            filename + filenameLength, &local.argument);
        void* filenameData;
        if ((self->filenameTag >> kStringHeapTagShift) == 0) {
            filenameData = (char*)&self->filenameTag + 1;
        } else {
            filenameData = self->filenameHeapData;
        }
        self->sharedFileHandle = SharedFile_Create(filenameData, 0);
    }
    return 1;
}

// Set a script resource, retain its directory prefix and pass the full filename to the loader.
// The path lives in scriptPathStorage: a long string has bit 31 set in its first word,
// with its length at +4; otherwise the first byte holds the inline length.
int CBpObject::SetScriptResourcePath(const char* filename, int scriptHandle) {
    TableLock temp(1, 1);
    if (filename == 0) {
        return 0;
    }
    script = (void*)scriptHandle;
    loadState = kLoadWaitingForFile;
    callbackListEmpty = 0;

    unsigned int previousLength;
    if ((*(unsigned int*)scriptPathStorage >> kStringHeapTagShift) == 0) {
        previousLength = *(unsigned char*)scriptPathStorage & kInlineStringLengthMask;
    } else {
        previousLength = (unsigned int)*(int*)(scriptPathStorage + 4);
    }
    struct {
        unsigned char argument;
        char padding[3];
        unsigned char source;
    } local1;
    local1.argument = local1.source;
    unsigned int filenameLength = strlen(filename);
    String_ReplaceRange(scriptPathStorage, 0, previousLength, filename, filename + filenameLength, &local1.argument);

    char* base = bpSpriteControlStrings;
    char* separatorText = base + kPathSeparator;
    unsigned int lastSeparator =
        (unsigned int)MWString_ReverseFindSubstring(scriptPathStorage, separatorText, -1, (int)strlen(separatorText));
    if (lastSeparator == 0xffffffffu) {
        if ((*(unsigned int*)scriptPathStorage >> kStringHeapTagShift) == 0) {
            previousLength = *(unsigned char*)scriptPathStorage & kInlineStringLengthMask;
        } else {
            previousLength = (unsigned int)*(int*)(scriptPathStorage + 4);
        }
        char* emptyText = base + kEmptyPath;
        struct {
            unsigned char argument;
            char padding[3];
            unsigned char source;
        } local2;
        local2.argument = local2.source;
        unsigned int len2 = strlen(emptyText);
        String_ReplaceRange(scriptPathStorage, 0, previousLength, emptyText, emptyText + len2, &local2.argument);
    } else {
        if ((*(unsigned int*)scriptPathStorage >> kStringHeapTagShift) == 0) {
            previousLength = *(unsigned char*)scriptPathStorage & kInlineStringLengthMask;
        } else {
            previousLength = (unsigned int)*(int*)(scriptPathStorage + 4);
        }
        struct {
            unsigned char argument;
            char padding[3];
            unsigned char source;
        } local3;
        local3.argument = local3.source;
        unsigned int eraseStart = (unsigned int)(lastSeparator + 1);
        unsigned int eraseLength = previousLength - (unsigned int)lastSeparator;
        String_ReplaceRange(scriptPathStorage, eraseStart, eraseLength, 0, 0, &local3.argument);
    }

    void* loaderObject = loader;
    void** vtable = *(void***)loaderObject;
    void (*setFilename)(void*, const char*) = (void (*)(void*, const char*))vtable[4];
    setFilename(loaderObject, filename);

    scriptState = kScriptResourceSet;
    return 1;
}

struct Node {
    Node* left;
    Node* right;
    unsigned parent;
    int key;
    void* value;
};
Node* IntegerKeyTree_FindInsertionPosition(void*, int*, Node**, unsigned char*, unsigned char*);
Node* BpEntryTree_InsertNode(void*, Node*, unsigned char, unsigned char, void*);
void RBTree_RebalanceAfterErase(void*, void*);
void appFreeUni__FPv(void*);
extern int g_bpEntryMapDefaultValue;

// Destroy and erase an existing object for the ID, then store the replacement in the integer map.
// Takes ownership of value for later deleting-destructor cleanup. Replacing a
// key destroys its OLD value first; passing that same pointer would retain a
// dangling value. Tree keys are plain integers; payloads start at node +0x0c.
int CBpObject::ReplaceObjectById(int key, int value) {
    TableLock temp(1, 1);
    if (objectCount != 0) {
        // Find the old value, destroy it, update firstObject, and erase its node.
        Node* node = (Node*)objectRoot;
        Node* parent = (Node*)(((char*)&objectRoot));
        while (node != 0) {
            if (node->key >= key) {
                parent = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
        if (parent != (Node*)(((char*)&objectRoot))) {
            parent = (key >= parent->key) ? parent : (Node*)(((char*)&objectRoot));
        }
        if (parent != (Node*)(((char*)&objectRoot))) {
            Node* toErase = parent;
            if (toErase->value != 0) {
                void** vt = *(void***)toErase->value;
                void (*dtor)(void*, int) = (void (*)(void*, int))vt[2];
                dtor(toErase->value, 1);
            }
            if (toErase == (Node*)firstObject) {
                Node* r = toErase->right;
                if (r) {
                    while (r->left)
                        r = r->left;
                    parent = r;
                } else {
                    while (1) {
                        Node* p = (Node*)(parent->parent & ~1u);
                        if (parent != p->left)
                            parent = p;
                        else {
                            parent = p;
                            break;
                        }
                    }
                }
                firstObject = parent;
            }
            RBTree_RebalanceAfterErase(toErase, objectRoot);
            appFreeUni__FPv(toErase);
            objectCount--;
        }
    }
    // Recompute insertion position after erasure; commit replacement ownership.
    Node* found;
    unsigned char a, b;
    Node* pos = IntegerKeyTree_FindInsertionPosition(((char*)&objectCount), &key, &found, &a, &b);
    if (found == 0 || found->key < key) {
        int item[2];
        item[0] = key;
        item[1] = g_bpEntryMapDefaultValue;
        Node* inserted = BpEntryTree_InsertNode(((char*)&objectCount), pos, a, b, item);
        inserted->value = (void*)value;
    } else {
        found->value = (void*)value;
    }
    // Actually original always stores value at inserted/found node+0x10 via r5
    return 1;
}

// declarations for BpStateCallbackTree_InsertNode
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern void* __nwa__FUl(unsigned);
extern void* __nw__FUl(unsigned long);
extern void appFreeUni__FPv(void*);
extern void* appMallocDummy__FUl(unsigned);
extern void __dt__Q23std12length_errorFv(void*);
extern void __dla__FPv(void*);

extern void RBTree_RebalanceAfterInsert(void*, void*);

// Release the message array if construction of its shared ownership control block throws.
struct Holder6ED8 {
    char* p;
    char* flag;
    // Guard an exception-message array during shared control-block construction.
    Holder6ED8(char* buffer, char* f) : p(buffer), flag(f) {}
    // Release the array if the guard has not been disarmed.
    ~Holder6ED8() {
        if (p)
            __dla__FPv(p);
    }
};
struct Flag6ED8 {
    char value;
    // Initialize exception-construction cleanup scratch.
    Flag6ED8() : value(0) {}
};

// Reproduce the original length_error message ownership and exception-construction cleanup.
struct ErrBase6ED8 {
    void* vt;
    char* buf;
    void* ref;
    // Copy the diagnostic and attach its shared ownership control block.
    ErrBase6ED8(const char* msg0) {
        vt = __vt__Q23std11logic_error;
        const char* msg = msg0;
        char done __attribute__((aligned(4)));
        Flag6ED8 extra;
        done = 0;
        unsigned n = strlen(msg) + 1;
        char* buffer = (char*)__nwa__FUl(n * 2 - n);
        buf = buffer;
        {
            Holder6ED8 h(buffer, &done);
            void* r = __nw__FUl(16);
            if (r) {
                int table = (int)s_sharedCharDeleterVtable;
                *((int*)r + 1) = 1;
                *((int*)r + 2) = 1;
                *((int*)r) = table;
                *((char**)r + 3) = buffer;
            }
            ref = r;
            h.p = 0;
        }
        strcpy(buf, msg);
    }
    // Preserve the original empty local exception-base destructor body.
    ~ErrBase6ED8() {}
};

struct Err2_6ED8 : ErrBase6ED8 {
    // Construct and throw the map's length_error diagnostic.
    Err2_6ED8() : ErrBase6ED8(bpSpriteControlStrings + kTreeLengthError) {
        vt = __vt__Q23std12length_error;
        __throw(bpSpriteControlStrings + kLengthErrorType, this, (void (*)())__dt__Q23std12length_errorFv);
    }
    // Preserve the original empty derived cleanup body.
    ~Err2_6ED8() {}
};

struct C6ED8Object {
    int count;
    void* root;
    void* firstNode;
};

// State-callback tree payload, NOT the named-group string-tree payload below.
// The ABI name BpStateCallbackTree_InsertNode is historical: TryInvokeCurrentCallback
// reads this SAME tree at object+44/+48 and interprets its values as member calls.
// CBpObject::CBpObject (ui_group_and_bp_edit_sprite.cpp) passes BpTransformPair {int key; uint32 value[3]}.
// Keys are scriptState: 2 = resource set, 3 = running (s_bpResourceSetCallbackState /
// s_bpRunningCallbackState, the data section). Values come from zeroInitializationWords
// / s_bpRunningStateCallback and are a CodeWarrior member-function descriptor
// {this adjustment, virtual selector, function/slot word}, tested by __ptmf_test.
// They borrow code identity; none is an owned allocation or MSL string. Keep all
// three descriptor words together and do not reinterpret them as vector floats.
// Node = BpTreeLinks +0..B, payload +C..1B, sizeof 0x1C, alignment 4.
// Prefix grouping retains the original three-word copy followed by the last word.
struct BpTransformCopyPrefix {
    int key;
    unsigned int value0, value1;
};
struct Value6ED8 {
    BpTransformCopyPrefix prefix;
    unsigned int value2;
    // Copy a script-state/member-descriptor record without acquiring a resource.
    Value6ED8(const Value6ED8& v) : prefix(v.prefix), value2(v.value2) {}
};
typedef char BpTransformPayload_size[sizeof(Value6ED8) == 16 ? 1 : -1];
struct Alloc6ED8 {
    unsigned char flag;
    void* allocation;
    void* copy;
    // Allocate a 28-byte node (12-byte links plus 16-byte key/value payload).
    Alloc6ED8() : flag(0) {
        allocation = appMallocDummy__FUl(0x1c);
        copy = allocation;
    }
};
struct NodeGuard6ED8 {
    void* allocator;
    void* node;
    // Borrow allocator state and guard the unlinked node.
    NodeGuard6ED8(void* a, void* n) : allocator(a), node(n) {}
    // Free a node only if insertion did not transfer ownership to the tree.
    ~NodeGuard6ED8() {
        if (node)
            appFreeUni__FPv(node);
    }
};
// Allocate one integer-keyed callback node using the matching allocator wrapper.
inline void* Allocate6ED8() {
    Alloc6ED8 a;
    return a.allocation;
}
// Copy-construct the key/value payload after the node links.
inline void* ConstructValue6ED8(void* p, const Value6ED8& v) {
    new ((char*)p + kTreePayloadOffset) Value6ED8(v);
    return p;
}
// Construct the parent link in the node header.
inline void ConstructParent6ED8(void* p, void* v) {
    new (p) void*(v);
}

#pragma push
#pragma use_lmw_stmw on
// Allocate, copy and link one script-state callback record; no key comparison
// occurs here. Caller supplies a valid insertion parent/side from
// BpIntObjectTree_FindInsertionPosition; parent may be the embedded root sentinel.
// self/keyValue required and nonaliasing with the new allocation. Tree owns node
// storage, released by ClearResources' trivial callback-tree teardown using
// appFreeUni__FPv; value words have no separate release. Count-limit throws before
// mutation. The node guard frees an unlinked allocation on unwinding; allocator
// null is unchecked. Once linked, count/root/first may change during rebalancing.
void* BpStateCallbackTree_InsertNode(C6ED8Object* self, void* parent, int insertLeft, int becomesFirst,
                                     const Value6ED8* keyValue) {
    if ((unsigned)self->count + 0x10000u == 0xffffu) {
        Err2_6ED8 err;
    }
    void* nodePtr = Allocate6ED8();
    NodeGuard6ED8 guard(&self->root, nodePtr);
    nodePtr = ConstructValue6ED8(nodePtr, *keyValue);
    AsBpTreeLinks(nodePtr)->right = 0;
    guard.node = 0;
    *(int*)nodePtr = 0;
    ConstructParent6ED8(&AsBpTreeLinks(nodePtr)->parentAndColor, parent);
    if (insertLeft != 0) {
        *(void**)parent = nodePtr;
    } else {
        *((void**)parent + 1) = nodePtr;
    }
    int oldCount = self->count;
    self->count = oldCount + 1;
    RBTree_RebalanceAfterInsert(nodePtr, self->root);
    if (becomesFirst != 0) {
        self->firstNode = nodePtr;
    }
    return nodePtr;
}
#pragma pop

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_BpObject[];
extern void* g_currentBpCallbackObject;
extern float kZeroCallbackPosition;
void CBpCtrl_dtor(void*);
void* gtl_Vec2_Copy(void*, void*);
int __ptmf_test(void*);

struct C55Object {
    char padding00[0x14];
    int key;
    char padding18[0x30];
    Node* root;
};
struct C55CallbackObject {
    int invoke();
};
// Invoke the current script's member callback on THIS CBpObject, not on the
// controller or callbackPosition. The three-word CodeWarrior member-function
// descriptor is copied from callback tree {links +0, script key +C, descriptor +10}.
// callbackPosition is null or a borrowed float[2], copied synchronously to +6C;
// null supplies {0,0}. It is a layout translation, in the same layout XY units as
// BpCtrl_UpdateAndDrawNodeList (bp_layout_animation_and_sprite_draw.cpp); CTourMain passes mapTranslation and
// CScreen passes a HUD position. It is not a receiver, mouse event or retained ptr.
// Return table: controller paused -> 1, missing script-key entry -> 0, null member
// descriptor -> 1, already invoked -> 0. Otherwise latch callbackInvoked BEFORE
// invoking int member(void). Callback 0 clears g_currentBpCallbackObject and
// returns 0; nonzero returns 1 and LEAVES the global context set for sprite work.
// The latch remains set even on callback 0; script restart/reset clears it.
// Descriptor and self must remain alive for this locked synchronous call. No
// exception cleanup restores the global/latch; callbacks must obey that contract.
int BpObject_TryInvokeCurrentCallback(C55Object* self, void* callbackPosition) {
    TableLock temp(1, 1);
    if (Sing_805DC5F0() == 0) {
        if (s_bpCtrlInitialized == 0) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_BpObject);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* controller = Sing_805DC5F0();
    if (((BpControllerStateView*)controller)->updateState == 2) {
        return 1;
    }
    Node* node = self->root;
    Node* parent = (Node*)&self->root;
    while (node != 0) {
        int nodeKey = node->key;
        if (nodeKey >= self->key) {
            parent = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }
    if (parent == (Node*)&self->root || self->key < parent->key) {
        parent = (Node*)&self->root;
    }
    if (parent == (Node*)&self->root) {
        return 0;
    }
    struct Callback55 {
        int words[3];
    };
    struct CallbackTreeNode {
        BpTreeLinks links;
        int key;
        Callback55 callback;
    };
    Callback55 local = ((CallbackTreeNode*)parent)->callback;
    if (__ptmf_test(&local) == 0) {
        return 1;
    }
    if ((*(int*)&AsCBpObject(self)->callbackInvoked) != 0) {
        return 0;
    }
    (*(int*)&AsCBpObject(self)->callbackInvoked) = 1;
    g_currentBpCallbackObject = self;
    if (callbackPosition != 0) {
        gtl_Vec2_Copy(((char*)&AsCBpObject(self)->callbackPosition[0]), callbackPosition);
    } else {
        float zero = kZeroCallbackPosition;
        AsCBpObject(self)->callbackPosition[0] = zero;
        AsCBpObject(self)->callbackPosition[1] = zero;
    }
    // The descriptor is a CodeWarrior member-function record {this adjustment, slot, code token}.
    typedef int (C55CallbackObject::*Callback)();
    if ((((C55CallbackObject*)self)->*(*(Callback*)&local))() == 0) {
        g_currentBpCallbackObject = 0;
        return 0;
    }
    return 1;
}

#pragma push
#pragma use_lmw_stmw on
// Free both child subtrees and the node; the original expands several traversal levels before recursing.
// Node storage only: this helper never deletes values or releases string keys.
// Call it after payload cleanup, or only for a tree whose payload is trivial.
void MapTree_FreeSubtree(void* self, void* node) {
    void *branch2, *branch1, *branch0, *child;
    branch0 = AsBpTreeLinks(node)->left;
    if (branch0 != 0) {
        branch1 = AsBpTreeLinks(branch0)->left;
        if (branch1 != 0) {
            branch2 = AsBpTreeLinks(branch1)->left;
            if (branch2 != 0) {
                child = AsBpTreeLinks(branch2)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch2)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch2);
            }
            branch2 = AsBpTreeLinks(branch1)->right;
            if (branch2 != 0) {
                child = AsBpTreeLinks(branch2)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch2)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch2);
            }
            appFreeUni__FPv(branch1);
        }
        branch2 = AsBpTreeLinks(branch0)->right;
        if (branch2 != 0) {
            branch1 = AsBpTreeLinks(branch2)->left;
            if (branch1 != 0) {
                child = AsBpTreeLinks(branch1)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch1)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch1);
            }
            branch1 = AsBpTreeLinks(branch2)->right;
            if (branch1 != 0) {
                child = AsBpTreeLinks(branch1)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch1)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch1);
            }
            appFreeUni__FPv(branch2);
        }
        appFreeUni__FPv(branch0);
    }
    branch2 = AsBpTreeLinks(node)->right;
    if (branch2 != 0) {
        branch1 = AsBpTreeLinks(branch2)->left;
        if (branch1 != 0) {
            branch0 = AsBpTreeLinks(branch1)->left;
            if (branch0 != 0) {
                child = AsBpTreeLinks(branch0)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch0)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch0);
            }
            branch0 = AsBpTreeLinks(branch1)->right;
            if (branch0 != 0) {
                child = AsBpTreeLinks(branch0)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch0)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch0);
            }
            appFreeUni__FPv(branch1);
        }
        branch1 = AsBpTreeLinks(branch2)->right;
        if (branch1 != 0) {
            branch0 = AsBpTreeLinks(branch1)->left;
            if (branch0 != 0) {
                child = AsBpTreeLinks(branch0)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch0)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch0);
            }
            branch0 = AsBpTreeLinks(branch1)->right;
            if (branch0 != 0) {
                child = AsBpTreeLinks(branch0)->left;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                child = AsBpTreeLinks(branch0)->right;
                if (child != 0)
                    MapTree_FreeSubtree(self, child);
                appFreeUni__FPv(branch0);
            }
            appFreeUni__FPv(branch1);
        }
        appFreeUni__FPv(branch2);
    }
    appFreeUni__FPv(node);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
struct C733CObject {
    int count;
    void* root;
    void* sentinel;
};
// Free every node in a map and restore its empty root/sentinel state.
// This is the same node-only cleanup despite the exported NamedGroup spelling;
// it does not inspect a key or mapped value. Payload teardown is the caller's job.
void BpNamedGroupTree_ClearNodes(C733CObject* self) {
    void* root = self->root;
    if (root != 0) {
        void* node0 = AsBpTreeLinks(root)->left;
        if (node0 != 0) {
            void* node1 = AsBpTreeLinks(node0)->left;
            if (node1 != 0) {
                void* node2 = AsBpTreeLinks(node1)->left;
                if (node2 != 0) {
                    void* node3 = AsBpTreeLinks(node2)->left;
                    if (node3 != 0) {
                        MapTree_FreeSubtree(self, node3);
                    }
                    void* node4 = AsBpTreeLinks(node2)->right;
                    if (node4 != 0) {
                        MapTree_FreeSubtree(self, node4);
                    }
                    appFreeUni__FPv(node2);
                }
                void* node5 = AsBpTreeLinks(node1)->right;
                if (node5 != 0) {
                    void* node6 = AsBpTreeLinks(node5)->left;
                    if (node6 != 0) {
                        MapTree_FreeSubtree(self, node6);
                    }
                    void* node7 = AsBpTreeLinks(node5)->right;
                    if (node7 != 0) {
                        MapTree_FreeSubtree(self, node7);
                    }
                    appFreeUni__FPv(node5);
                }
                appFreeUni__FPv(node1);
            }
            void* node8 = AsBpTreeLinks(node0)->right;
            if (node8 != 0) {
                void* node9 = AsBpTreeLinks(node8)->left;
                if (node9 != 0) {
                    void* node10 = AsBpTreeLinks(node9)->left;
                    if (node10 != 0) {
                        MapTree_FreeSubtree(self, node10);
                    }
                    void* node11 = AsBpTreeLinks(node9)->right;
                    if (node11 != 0) {
                        MapTree_FreeSubtree(self, node11);
                    }
                    appFreeUni__FPv(node9);
                }
                void* node12 = AsBpTreeLinks(node8)->right;
                if (node12 != 0) {
                    void* node13 = AsBpTreeLinks(node12)->left;
                    if (node13 != 0) {
                        MapTree_FreeSubtree(self, node13);
                    }
                    void* node14 = AsBpTreeLinks(node12)->right;
                    if (node14 != 0) {
                        MapTree_FreeSubtree(self, node14);
                    }
                    appFreeUni__FPv(node12);
                }
                appFreeUni__FPv(node8);
            }
            appFreeUni__FPv(node0);
        }
        void* node15 = AsBpTreeLinks(root)->right;
        if (node15 != 0) {
            void* node16 = AsBpTreeLinks(node15)->left;
            if (node16 != 0) {
                void* node17 = AsBpTreeLinks(node16)->left;
                if (node17 != 0) {
                    void* node18 = AsBpTreeLinks(node17)->left;
                    if (node18 != 0) {
                        MapTree_FreeSubtree(self, node18);
                    }
                    void* node19 = AsBpTreeLinks(node17)->right;
                    if (node19 != 0) {
                        MapTree_FreeSubtree(self, node19);
                    }
                    appFreeUni__FPv(node17);
                }
                void* node20 = AsBpTreeLinks(node16)->right;
                if (node20 != 0) {
                    void* node21 = AsBpTreeLinks(node20)->left;
                    if (node21 != 0) {
                        MapTree_FreeSubtree(self, node21);
                    }
                    void* node22 = AsBpTreeLinks(node20)->right;
                    if (node22 != 0) {
                        MapTree_FreeSubtree(self, node22);
                    }
                    appFreeUni__FPv(node20);
                }
                appFreeUni__FPv(node16);
            }
            void* node23 = AsBpTreeLinks(node15)->right;
            if (node23 != 0) {
                void* node24 = AsBpTreeLinks(node23)->left;
                if (node24 != 0) {
                    void* node25 = AsBpTreeLinks(node24)->left;
                    if (node25 != 0) {
                        MapTree_FreeSubtree(self, node25);
                    }
                    void* node26 = AsBpTreeLinks(node24)->right;
                    if (node26 != 0) {
                        MapTree_FreeSubtree(self, node26);
                    }
                    appFreeUni__FPv(node24);
                }
                void* node27 = AsBpTreeLinks(node23)->right;
                if (node27 != 0) {
                    void* node28 = AsBpTreeLinks(node27)->left;
                    if (node28 != 0) {
                        MapTree_FreeSubtree(self, node28);
                    }
                    void* node29 = AsBpTreeLinks(node27)->right;
                    if (node29 != 0) {
                        MapTree_FreeSubtree(self, node29);
                    }
                    appFreeUni__FPv(node27);
                }
                appFreeUni__FPv(node23);
            }
            appFreeUni__FPv(node15);
        }
        appFreeUni__FPv(root);
        self->count = 0;
        self->root = 0;
        self->sentinel = &self->root;
    }
}
#pragma pop
int BpObject_HasPhfSignature(void*);
int PhfFile_GetEntryCountWord(void*);
void* PhfFile_GetEntry(void*, int);
void* PhfFile_GetEntryData(void*, int);
int strcmp(const char*, const char*);
void BpResource_ByteSwapIndexTable(void*);
int B2dFile_CalculateCommandWorkSize(void*, void*, void*, void*);
void* appMallocUni__FUl(unsigned);
void BpSlot_RegisterWork(void*, void*, int, int);
void BpSpritePool_MarkAllInactive(void*);
void B2dFile_ExecuteNamedScript(void*, void*);
int B2dFile_Register(void*, void*);
void B2d_SetTextureSubcommandCallback(void*);
void BpScript_CreateNamedGsrTexture(void*);
void CBpObject_CreateAndReplaceNamedTexture(void*);
extern "C" void* memset(void*, int, unsigned long);
void String_ReserveCapacity(void*, unsigned int);
void* StringTree_FindInsertionPosition(void*, void*, void**, char*, char*);
void* NamedObjectMap_InsertNode(void*, void*, int, int, void*);
void StringObj_dtor(void*, int);
void appFreeUni__FPv(void*);
void* BpSlot_GetSpriteListHead(void*);
void UIGroupVector_Append(void*, void*);
int UIGroup_GetItemCount(void*);
extern void* g_bpNamedGroupKeyAllocator;

struct C5898Object {
    char padding00[4];
    void* script;
    char padding08[4];
    void* resourceBuffer;
    void* scriptBuffer;
    int scriptState;
    char padding18[4];
    void* objectRoot;
    void* firstObject;
    void* objectArray;
    int loadState;
    char padding2c[0x30];
    void* loader;
};

#pragma push
#pragma use_lmw_stmw on

struct C62D4Object {
    void* vtable;
    char padding04[0x10];
    // Construct an editor group attached to its owning BP object.
    C62D4Object(void* owner) {
        ((CBpEditSprite*)this)->__ct(owner);
    }
};

struct C62D4Node {
    C62D4Node* left;
    C62D4Node* right;
    C62D4Node* parentAndColor;
    unsigned int keyFlags;
    unsigned int keyLength;
    void* keyData;
    void* value;
};

struct C62D4Pair {
    char key[0xC];
    void* value;
};
// Local cache model: objectCount/objectRoot/firstObject at +18/+1c/+20 own
// integer-keyed resources; namedGroupCount/root/first at +50/+54/+58 own editor
// groups. End iterators are ADDRESSES of root slots, not null and not allocated
// nodes. Every node starts with left/right/parent+color (low bit). Named nodes
// then own a 12-byte narrow string and a group pointer at +18. Cached groups
// borrow script-entry pointers, so clearing resources invalidates every result.

// CodeWarrior string storage: the high tag bit selects heap data; short text begins one byte after flags.
struct C62D4String {
    unsigned int flags;
    unsigned int length;
    char* data;
};

struct C62D4OwnedString {
    C62D4String string;
    void* allocator;
};

struct C62D4CharArgument {
    volatile unsigned char argument;
    char padding[3];
    volatile unsigned char source;
    char padding2[3];
};

struct C62D4Container {
    char padding00[4];
    void* script;
    char padding08[0x48];
    void* treeAnchor;
    C62D4Node* root;
};

// Return a reference to the smaller string length.
static inline const unsigned int& Min62D4(const unsigned int& length1, const unsigned int& length2) {
    return length1 < length2 ? length1 : length2;
}

#pragma push
#pragma use_lmw_stmw on
// Find a cached group by name, or collect matching script entries and cache the new group.
// key is a borrowed terminated name; return a cache-owned group or null if no
// script entry matches. The named path collects ALL matches. The ID path below
// instead formats a cache key and collects only the FIRST matching numeric ID.
// Neither result may outlive the owner's script/resource reset.
// Failure/publication contract (also see the ID path's different cleanup below):
// 1 lookup builds a plain search string, manually freed before group allocation.
//   Allocation/string exceptions before that free have no local string RAII.
// 2 build allocates a group and appends borrowed entries. It is NOT yet cached.
//   An append/temporary-key exception can leave this group and its pointer array
//   unreachable: the raw object pointer has no scope guard. No rollback is added.
// 3 NamedObjectMap_InsertNode (wpuppet_model_bone_animation.cpp) copies key/value before linking the node,
//   increments count/rebalances, then returns. Its allocation/key-copy failures
//   precede linking; its guard releases an unlinked node. The staged value is
//   localKey.allocator, NOT object, until pair->value=object after insertion.
//   Thus the new group becomes cache-owned only at that final store. The mutex
//   covers both steps; no other serialized lookup sees the intermediate value.
//   No atomic transaction is promised for reentrant allocator callbacks.
// 4 on normal return, temporary heap keys are manually freed. These plain
//   records have no destructor on throw; the table guard still unlocks. Do not
//   assume a failed call cleaned up its uncached group or temporary key bytes.
// Existing cache entries are not removed by any of these allocation failures.
void* BpObject_GetOrCreateNamedGroup(C62D4Container* self, const char* key) {
    TableLock guard(1, 1);
    // Lookup phase: copy the key, find a lexical lower bound, then verify equality.
    C62D4String search;
    search.flags = 0;
    search.length = 0;
    search.data = 0;
    unsigned int length = strlen(key);
    String_ReserveCapacity(&search, length);
    C62D4CharArgument initialLocal;
    initialLocal.argument = initialLocal.source;
    String_ReplaceRange(&search, 0, 0, key, key + length, (char*)&initialLocal.argument);

    C62D4Node** header = &self->root;
    C62D4Node* node = *header;
    C62D4Node* found = (C62D4Node*)header;
    unsigned int searchFlag = search.flags >> kStringHeapTagShift;
    bool searchHeap = !!searchFlag;
    unsigned int searchSmallLength = *((unsigned char*)&search) & kInlineStringLengthMask;
    unsigned int searchLength = search.length;
    const char* searchHeapData = search.data;
    // Lookup A: lower_bound compares existing key to requested name.
    while (node != 0) {
        unsigned int currentSearchLength;
        const char* currentSearchData;
        if (searchHeap == 0) {
            currentSearchLength = searchSmallLength;
            currentSearchData = (char*)&search + 1;
        } else {
            currentSearchLength = searchLength;
            currentSearchData = searchHeapData;
        }
        unsigned int nodeLength1;
        if ((node->keyFlags >> kStringHeapTagShift) == 0)
            nodeLength1 = *(unsigned char*)&node->keyFlags & kInlineStringLengthMask;
        else
            nodeLength1 = node->keyLength;
        unsigned int nodeLength2;
        const char* nodeData;
        if ((node->keyFlags >> kStringHeapTagShift) == 0) {
            nodeLength2 = *(unsigned char*)&node->keyFlags & kInlineStringLengthMask;
            nodeData = (const char*)&node->keyFlags + 1;
        } else {
            nodeLength2 = node->keyLength;
            nodeData = (const char*)node->keyData;
        }
        unsigned int nodeMin = Min62D4(nodeLength1, nodeLength2);
        unsigned int minLength = Min62D4(currentSearchLength, nodeMin);
        int cmp = memcmp(nodeData, currentSearchData, minLength);
        if (cmp == 0) {
            if (nodeMin < currentSearchLength)
                cmp = -1;
            else if (nodeMin > currentSearchLength)
                cmp = 1;
            else
                cmp = 0;
        }
        if (cmp >= 0) {
            found = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }

    // Lookup B: reverse comparison distinguishes exact equality from a greater key.
    if (found != (C62D4Node*)header) {
        unsigned int foundLength;
        const char* foundData;
        if ((found->keyFlags >> kStringHeapTagShift) == 0) {
            foundLength = *(unsigned char*)&found->keyFlags & kInlineStringLengthMask;
            foundData = (const char*)&found->keyFlags + 1;
        } else {
            foundLength = found->keyLength;
            foundData = (const char*)found->keyData;
        }
        unsigned int searchLen1;
        if (searchHeap == 0)
            searchLen1 = searchSmallLength;
        else
            searchLen1 = searchLength;
        const char* currentSearchData;
        unsigned int searchLen2 = searchSmallLength;
        if (searchHeap == 0) {
            currentSearchData = (char*)&search + 1;
        } else {
            currentSearchData = searchHeapData;
            searchLen2 = searchLength;
        }
        unsigned int searchMin = Min62D4(searchLen1, searchLen2);
        unsigned int minLength = Min62D4(foundLength, searchMin);
        int cmp = memcmp(currentSearchData, foundData, minLength);
        if (cmp == 0) {
            if (searchMin < foundLength) {
                cmp = -1;
            } else {
                if (searchMin == foundLength)
                    cmp = 0;
                else
                    cmp = 1;
            }
        }
        if (cmp < 0)
            found = (C62D4Node*)header;
    }

    // Lookup C: the temporary key owns its heap bytes; the cache result is borrowed.
    if (searchFlag != 0)
        appFreeUni__FPv(search.data);

    if (found != (C62D4Node*)header)
        return found->value;

    C62D4Object* object = new C62D4Object(self);
    // Build phase: group borrows every matching script entry; empty groups are deleted.
    void* item = BpSlot_GetSpriteListHead(self->script);
    while (item != 0) {
        void* itemValue = (void*)((BpScriptEntryView*)item)->groupName;
        if (itemValue != 0 && strcmp((const char*)itemValue, key) == 0)
            UIGroupVector_Append(object, item);
        item = ((BpScriptEntryView*)item)->next;
    }

    if (UIGroup_GetItemCount(object) == 0) {
        if (object != 0) {
            void** vtable = *(void***)object;
            ((void (*)(void*, int))vtable[2])(object, 1);
        }
        return 0;
    }

    // Cache phase: insertion copies its key. localKey is a temporary owning copy,
    // insertKey owns the request copy, and neither may transfer raw heap pointers.
    C62D4String insertKey;
    insertKey.flags = 0;
    insertKey.length = 0;
    insertKey.data = 0;
    length = strlen(key);
    String_ReserveCapacity(&insertKey, length);
    C62D4CharArgument inputLocal;
    inputLocal.argument = inputLocal.source;
    String_ReplaceRange(&insertKey, 0, 0, key, key + length, (char*)&inputLocal.argument);

    C62D4Node* position;
    unsigned char insertLeft;
    unsigned char insertRight;
    void* insertion = StringTree_FindInsertionPosition(((char*)&AsCBpObject(self)->namedGroupCount), &insertKey,
                                                       (void**)&position, (char*)&insertLeft, (char*)&insertRight);

    C62D4Pair* pair;
    // Original 0x800C62D4 / 0x800C6834: reuse the key or insert it before storing.
    do {
        if (position != 0) {
            pair = (C62D4Pair*)((char*)position + kTreePayloadOffset);
            unsigned int insertLength;
            const char* insertData;
            if ((insertKey.flags >> kStringHeapTagShift) == 0) {
                insertLength = *((unsigned char*)&insertKey) & kInlineStringLengthMask;
                insertData = (char*)&insertKey + 1;
            } else {
                insertLength = insertKey.length;
                insertData = insertKey.data;
            }
            unsigned int positionLength1;
            if ((position->keyFlags >> kStringHeapTagShift) == 0)
                positionLength1 = *((unsigned char*)position + kTreePayloadOffset) & kInlineStringLengthMask;
            else
                positionLength1 = position->keyLength;
            unsigned int positionLength2;
            const char* positionData;
            if ((position->keyFlags >> kStringHeapTagShift) == 0) {
                positionLength2 = *((unsigned char*)position + kTreePayloadOffset) & kInlineStringLengthMask;
                positionData = (const char*)&position->keyFlags + 1;
            } else {
                positionLength2 = position->keyLength;
                positionData = (const char*)position->keyData;
            }
            unsigned int positionMin = Min62D4(positionLength1, positionLength2);
            unsigned int minLength = Min62D4(insertLength, positionMin);
            int cmp = memcmp(positionData, insertData, minLength);
            if (cmp == 0) {
                if (positionMin < insertLength)
                    cmp = -1;
                else if (positionMin > insertLength)
                    cmp = 1;
                else
                    cmp = 0;
            }
            if (cmp >= 0)
                break;
        }
        {
            C62D4OwnedString localKey;
            if ((insertKey.flags >> kStringHeapTagShift) == 0) {
                localKey.string.flags = insertKey.flags;
                localKey.string.length = insertKey.length;
                localKey.string.data = insertKey.data;
            } else {
                localKey.string.flags = 0;
                localKey.string.length = 0;
                localKey.string.data = 0;
                String_ReserveCapacity(&localKey, insertKey.length);
                C62D4CharArgument copyLocal;
                copyLocal.argument = copyLocal.source;
                String_ReplaceRange(&localKey, 0, 0, insertKey.data, insertKey.data + insertKey.length,
                                    (char*)&copyLocal.argument);
            }
            localKey.allocator = g_bpNamedGroupKeyAllocator;
            C62D4Node* inserted = (C62D4Node*)NamedObjectMap_InsertNode(((char*)&AsCBpObject(self)->namedGroupCount),
                                                                        insertion, insertLeft, insertRight, &localKey);
            pair = (C62D4Pair*)((char*)inserted + kTreePayloadOffset);
            if (localKey.string.flags >> kStringHeapTagShift)
                appFreeUni__FPv(localKey.string.data);
        }
    } while (0);
    pair->value = object;
    if (insertKey.flags >> kStringHeapTagShift)
        appFreeUni__FPv(insertKey.data);
    return object;
}
#pragma pop

void StringObj_dtor(void*, int);

struct C6834Container {
    char padding00[4];
    void* script;
    char padding08[0x48];
    void* treeAnchor;
    C62D4Node* root;
};

struct C6834OwnedString {
    C62D4String string;
    // Release a temporary ID-path string's heap storage.
    ~C6834OwnedString() {
        if (string.flags >> kStringHeapTagShift)
            appFreeUni__FPv(string.data);
    }
};

struct C6834LocalKey {
    C62D4String string;
    void* allocator;
    // Release a temporary node-key copy after insertion copies its characters.
    ~C6834LocalKey() {
        if (string.flags >> kStringHeapTagShift)
            appFreeUni__FPv(string.data);
    }
};

#pragma push
#pragma use_lmw_stmw on
// Find a cached sprite-ID group, or create one containing the first script entry with that ID.
// Return is borrowed from the same named-group cache as the name lookup above.
// Existing cached values return immediately; a missing ID returns null and is
// not cached. Formatting requires the existing prefix/ID format to fit 0x400 bytes.
// Same build/node/group publication points as GetOrCreateNamedGroup above:
// object remains an UNGUARDED raw pointer until pair->value=object; an append
// or key-allocation exception does not delete it. Node insertion copies the
// temporary allocator-valued pair before linking; only the final store publishes
// a valid group. Allocation/key-copy failures inside InsertNode precede linking.
// Cleanup differs: path and insertKey are C6834OwnedString RAII and release
// completed heap strings on unwind. search is plain and manually freed after
// lookup. localKey is also plain C62D4OwnedString: only an InsertNode exception
// reaches the explicit catch/free/rethrow; failure while COPYING that key does
// not. The unused C6834LocalKey destructor below is not its cleanup mechanism.
// TableLock unlocks on all unwinds. This is not a strong exception guarantee;
// preserve the distinct cleanup boundaries when factoring either transaction.
void* UIGroup_GetOrCreateById(C6834Container* self, int key) {
    TableLock guard(1, 1);
    // Lookup phase: format the synthetic name and search the shared lexical cache.
    char pathBuffer[0x400];
    sprintf(pathBuffer, bpSpriteControlStrings + kGroupIdFormat, bpSpriteControlStrings + kSpriteIdPrefix, key);

    C6834OwnedString path;
    path.string.flags = 0;
    path.string.length = 0;
    path.string.data = 0;
    unsigned int pathLength = strlen(pathBuffer);
    String_ReserveCapacity(&path, pathLength);
    C62D4CharArgument pathLocal;
    pathLocal.argument = pathLocal.source;
    String_ReplaceRange(&path, 0, 0, pathBuffer, pathBuffer + pathLength, (char*)&pathLocal.argument);

    const char* pathData;
    if ((path.string.flags >> kStringHeapTagShift) == 0)
        pathData = (char*)&path + 1;
    else
        pathData = path.string.data;
    C62D4String search;
    search.flags = 0;
    search.length = 0;
    search.data = 0;
    unsigned int searchLength = strlen(pathData);
    String_ReserveCapacity(&search, searchLength);
    C62D4CharArgument searchLocal;
    searchLocal.argument = searchLocal.source;
    String_ReplaceRange(&search, 0, 0, pathData, pathData + searchLength, (char*)&searchLocal.argument);

    C62D4Node** header = &self->root;
    C62D4Node* node = *header;
    C62D4Node* found = (C62D4Node*)header;
    unsigned int searchFlag = search.flags >> kStringHeapTagShift;
    bool searchHeap = (searchFlag != 0);
    unsigned int searchSmallLength = *((unsigned char*)&search) & kInlineStringLengthMask;
    unsigned int searchHeapLength = search.length;
    const char* searchHeapData = search.data;
    while (node != 0) {
        unsigned int currentLength;
        const char* currentData;
        if (searchHeap == 0) {
            currentLength = searchSmallLength;
            currentData = (char*)&search + 1;
        } else {
            currentLength = searchHeapLength;
            currentData = searchHeapData;
        }
        unsigned int nodeLength1;
        if ((node->keyFlags >> kStringHeapTagShift) == 0)
            nodeLength1 = *(unsigned char*)&node->keyFlags & kInlineStringLengthMask;
        else
            nodeLength1 = node->keyLength;
        unsigned int nodeLength2;
        const char* nodeData;
        if ((node->keyFlags >> kStringHeapTagShift) == 0) {
            nodeLength2 = *(unsigned char*)&node->keyFlags & kInlineStringLengthMask;
            nodeData = (char*)&node->keyFlags + 1;
        } else {
            nodeLength2 = node->keyLength;
            nodeData = (const char*)node->keyData;
        }
        unsigned int nodeMin = Min62D4(nodeLength1, nodeLength2);
        unsigned int minLength = Min62D4(currentLength, nodeMin);
        int cmp = memcmp(nodeData, currentData, minLength);
        if (cmp == 0) {
            if (nodeMin < currentLength)
                cmp = -1;
            else if (nodeMin == currentLength)
                cmp = 0;
            else
                cmp = 1;
        }
        if (cmp >= 0) {
            found = node;
            node = node->left;
        } else {
            node = node->right;
        }
    }

    if (found != (C62D4Node*)header) {
        unsigned int foundLength;
        const char* foundData;
        if ((found->keyFlags >> kStringHeapTagShift) == 0) {
            foundLength = *(unsigned char*)&found->keyFlags & kInlineStringLengthMask;
            foundData = (char*)&found->keyFlags + 1;
        } else {
            foundLength = found->keyLength;
            foundData = (const char*)found->keyData;
        }
        unsigned int searchLen1;
        if (searchHeap == 0)
            searchLen1 = searchSmallLength;
        else
            searchLen1 = searchHeapLength;
        const char* currentData;
        unsigned int searchLen2 = searchSmallLength;
        if (searchHeap == 0) {
            currentData = (char*)&search + 1;
        } else {
            currentData = searchHeapData;
            searchLen2 = searchHeapLength;
        }
        unsigned int searchMin = Min62D4(searchLen1, searchLen2);
        unsigned int minLength = Min62D4(foundLength, searchMin);
        int cmp = memcmp(currentData, foundData, minLength);
        if (cmp == 0) {
            if (searchMin < foundLength)
                cmp = -1;
            else if (searchMin != foundLength)
                cmp = 1;
            else
                cmp = 0;
        }
        if (cmp < 0)
            found = (C62D4Node*)header;
    }

    if (searchFlag != 0)
        appFreeUni__FPv(search.data);

    if (found != (C62D4Node*)header) {
        void* result = found->value;
        return result;
    }

    // Build phase: preserve the first-ID-match break (different from name grouping).
    C62D4Object* object = new C62D4Object(self);
    void* item = BpSlot_GetSpriteListHead(self->script);
    while (item != 0) {
        // Original 0x800C6B8C-0x800C6BA0 (cmpw/bne, bl UIGroupVector_Append, b 0x800C6BB0):
        // the loop stops after the first entry whose id matches.
        if (((BpScriptEntryView*)item)->id == key) {
            UIGroupVector_Append(object, item);
            break;
        }
        item = ((BpScriptEntryView*)item)->next;
    }
    if (UIGroup_GetItemCount(object) == 0) {
        if (object != 0) {
            void** vtable = *(void***)object;
            ((void (*)(void*, int))vtable[2])(object, 1);
        }
        return 0;
    }

    // Cache phase: build/copy a node-owned key, then publish the owned group pointer.
    C6834OwnedString insertKey;
    const char* insertSource;
    if ((path.string.flags >> kStringHeapTagShift) == 0)
        insertSource = (char*)&path + 1;
    else
        insertSource = path.string.data;
    insertKey.string.flags = 0;
    insertKey.string.length = 0;
    insertKey.string.data = 0;
    unsigned int insertLength = strlen(insertSource);
    String_ReserveCapacity(&insertKey, insertLength);
    C62D4CharArgument insertLocal;
    insertLocal.argument = insertLocal.source;
    String_ReplaceRange(&insertKey, 0, 0, insertSource, insertSource + insertLength, (char*)&insertLocal.argument);

    C62D4Node* position;
    unsigned char insertLeft;
    unsigned char insertRight;
    void* insertion = StringTree_FindInsertionPosition(((char*)&AsCBpObject(self)->namedGroupCount), &insertKey,
                                                       (void**)&position, (char*)&insertLeft, (char*)&insertRight);

    C62D4Pair* pair;
    // Original 0x800C62D4 / 0x800C6834: reuse the key or insert it before storing.
    do {
        if (position != 0) {
            pair = (C62D4Pair*)((char*)position + kTreePayloadOffset);
            unsigned int newLength;
            const char* newData;
            if ((insertKey.string.flags >> kStringHeapTagShift) == 0) {
                newLength = *((unsigned char*)&insertKey) & kInlineStringLengthMask;
                newData = (char*)&insertKey + 1;
            } else {
                newLength = insertKey.string.length;
                newData = (const char*)insertKey.string.data;
            }
            unsigned int positionLength1;
            if ((position->keyFlags >> kStringHeapTagShift) == 0)
                positionLength1 = *((unsigned char*)position + kTreePayloadOffset) & kInlineStringLengthMask;
            else
                positionLength1 = position->keyLength;
            unsigned int positionLength2;
            const char* positionData;
            if ((position->keyFlags >> kStringHeapTagShift) == 0) {
                positionLength2 = *((unsigned char*)position + kTreePayloadOffset) & kInlineStringLengthMask;
                positionData = (const char*)&position->keyFlags + 1;
            } else {
                positionLength2 = position->keyLength;
                positionData = (const char*)position->keyData;
            }
            unsigned int positionMin = Min62D4(positionLength1, positionLength2);
            unsigned int minLength = Min62D4(newLength, positionMin);
            int cmp = memcmp(positionData, newData, minLength);
            if (cmp == 0) {
                if (positionMin < newLength)
                    cmp = -1;
                else if (positionMin != newLength)
                    cmp = 1;
                else
                    cmp = 0;
            }
            if (cmp >= 0)
                break;
        }
        {
            C62D4OwnedString localKey;
            if ((insertKey.string.flags >> kStringHeapTagShift) == 0) {
                localKey.string.flags = insertKey.string.flags;
                localKey.string.length = insertKey.string.length;
                localKey.string.data = insertKey.string.data;
            } else {
                localKey.string.flags = 0;
                localKey.string.length = 0;
                localKey.string.data = 0;
                String_ReserveCapacity(&localKey, insertKey.string.length);
                C62D4CharArgument copyLocal;
                copyLocal.argument = copyLocal.source;
                String_ReplaceRange(&localKey, 0, 0, insertKey.string.data,
                                    insertKey.string.data + insertKey.string.length, (char*)&copyLocal.argument);
            }
            localKey.allocator = g_bpNamedGroupKeyAllocator;
            C62D4Node* inserted;
            try {
                inserted = (C62D4Node*)NamedObjectMap_InsertNode(((char*)&AsCBpObject(self)->namedGroupCount),
                                                                 insertion, insertLeft, insertRight, &localKey);
            } catch (...) {
                if (localKey.string.flags >> kStringHeapTagShift)
                    appFreeUni__FPv(localKey.string.data);
                throw;
            }
            pair = (C62D4Pair*)((char*)inserted + kTreePayloadOffset);
            if (localKey.string.flags >> kStringHeapTagShift)
                appFreeUni__FPv(localKey.string.data);
        }
    } while (0);
    pair->value = object;
    return object;
}
#pragma pop

extern "C" void BpObject_ClearWorkBuffers(void*);
extern "C" void B2dFile_Reset(void*);
extern "C" void RBTree_FreeTrivialSubtree(void*, void*);
extern "C" void StringValueTree_DestroySubtree(void*, void*);

// Release a group node's copied heap key; inline bytes and group values stay untouched.
// The key-address guard is retained because its wrapping address test emits code.
static inline void ReleaseBpGroupHeapKey(void* node) {
    void* keyStorage = (char*)&((C62D4Node*)node)->keyFlags;
    if (keyStorage != 0) {
        unsigned keyFlags = *(unsigned*)keyStorage;
        if (keyFlags >> kStringHeapTagShift) {
            void* keyHeap = ((C62D4Node*)node)->keyData;
            appFreeUni__FPv(keyHeap);
        }
    }
}

// Free the deeper integer-key child subtrees; the caller releases this node afterward.
static inline void ReleaseBpIntegerChildSubtrees(void* owner, void* grandchildNode) {
    void* subtree = AsBpTreeLinks(grandchildNode)->left;
    if (subtree != 0)
        RBTree_FreeTrivialSubtree((char*)&AsCBpObject(owner)->objectCount, subtree);
    subtree = AsBpTreeLinks(grandchildNode)->right;
    if (subtree != 0)
        RBTree_FreeTrivialSubtree((char*)&AsCBpObject(owner)->objectCount, subtree);
}

// Free the deeper string-key child subtrees; the caller releases this node afterward.
static inline void ReleaseBpStringChildSubtrees(void* owner, void* grandchildNode) {
    void* subtree = AsBpTreeLinks(grandchildNode)->left;
    if (subtree != 0)
        StringValueTree_DestroySubtree((char*)&AsCBpObject(owner)->namedGroupCount, subtree);
    subtree = AsBpTreeLinks(grandchildNode)->right;
    if (subtree != 0)
        StringValueTree_DestroySubtree((char*)&AsCBpObject(owner)->namedGroupCount, subtree);
}

#pragma push
#pragma use_lmw_stmw on
// Reset the script, destroy mapped objects/groups and their keys, clear the loader and delete pooled sprites.
// Invalidates all borrowed resource/group/script pointers. Requires an initialized
// script, loader and sprite pool. The expanded postorder traversals deliberately
// separate deleting VALUES from freeing NODES; do not merge those ownership steps.
int CBpObject::ClearResources() {
    TableLock t1(1, 1);
    void* owner = this;
    // Script phase: release script work while the shared resource bytes still live.
    BpObject_ClearWorkBuffers(AsCBpObject(owner)->script);
    B2dFile_Reset(AsCBpObject(owner)->script);
    {
        TableLock t2(1, 1);
        void* scriptWork = AsCBpObject(owner)->scriptBuffer;
        if (scriptWork != 0) {
            appFreeUni__FPv(scriptWork);
            AsCBpObject(owner)->scriptBuffer = 0;
        }
    }
    // Object values: in-order deleting-destructor pass; nodes remain linked.
    {
        void* node = AsCBpObject(owner)->firstObject;
        void* sentinel = (char*)&AsCBpObject(owner)->objectRoot;
        while (node != sentinel) {
            void* val = AsBpIntObjectNode(node)->value;
            if (val != 0) {
                void (*dtor)(void*, int) = (void (*)(void*, int))(*(void***)val)[2];
                dtor(val, 1);
            }
            void* successor = AsBpTreeLinks(node)->right;
            if (successor != 0) {
                while (*(void**)successor != 0)
                    successor = *(void**)successor;
                node = successor;
            } else {
                while (true) {
                    unsigned u = AsBpTreeLinks(node)->parentAndColor;
                    void* parent = (void*)(u & ~1u);
                    if (node != *(void**)parent) {
                        node = parent;
                        continue;
                    }
                    node = parent;
                    break;
                }
            }
        }
    }
    // Object nodes: postorder node-only free, then restore the empty header.
    // Local branches expand the first levels; deeper children use the same
    // node-only recursion. No key cleanup is needed for these integer keys.
    {
        void* rootNode = AsCBpObject(owner)->objectRoot;
        if (rootNode != 0) {
            void* leftBranch = AsBpTreeLinks(rootNode)->left;
            if (leftBranch != 0) {
                void* childNode = AsBpTreeLinks(leftBranch)->left;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    appFreeUni__FPv(childNode);
                }
                childNode = AsBpTreeLinks(leftBranch)->right;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    appFreeUni__FPv(childNode);
                }
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = AsBpTreeLinks(rootNode)->right;
            if (rightBranch != 0) {
                void* childNode = AsBpTreeLinks(rightBranch)->left;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    appFreeUni__FPv(childNode);
                }
                childNode = AsBpTreeLinks(rightBranch)->right;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpIntegerChildSubtrees(owner, grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    appFreeUni__FPv(childNode);
                }
                appFreeUni__FPv(rightBranch);
            }
            appFreeUni__FPv(rootNode);
            AsCBpObject(owner)->objectCount = 0;
            AsCBpObject(owner)->objectRoot = 0;
            AsCBpObject(owner)->firstObject = (char*)&AsCBpObject(owner)->objectRoot;
        }
    }
    // Group values: delete editor groups after resources, before freeing copied keys.
    {
        void* node = AsCBpObject(owner)->firstNamedGroup;
        void* sentinel = (char*)&AsCBpObject(owner)->namedGroupRoot;
        while (node != sentinel) {
            void* val = ((C62D4Node*)node)->value;
            if (val != 0) {
                void (*dtor)(void*, int) = (void (*)(void*, int))(*(void***)val)[2];
                dtor(val, 1);
            }
            void* successor = AsBpTreeLinks(node)->right;
            if (successor != 0) {
                while (*(void**)successor != 0)
                    successor = *(void**)successor;
                node = successor;
            } else {
                while (true) {
                    unsigned u = AsBpTreeLinks(node)->parentAndColor;
                    void* parent = (void*)(u & ~1u);
                    if (node != *(void**)parent) {
                        node = parent;
                        continue;
                    }
                    node = parent;
                    break;
                }
            }
        }
    }
    // Group nodes: postorder string-key release plus node free, then empty header.
    // Each key's heap bit decides whether keyData is an allocation or inline bytes.
    {
        void* rootNode = AsCBpObject(owner)->namedGroupRoot;
        if (rootNode != 0) {
            void* leftBranch = AsBpTreeLinks(rootNode)->left;
            if (leftBranch != 0) {
                void* childNode = AsBpTreeLinks(leftBranch)->left;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        // Release this grandchild's owned heap key before its node.
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    // Release this parent's key only after both child subtrees.
                    ReleaseBpGroupHeapKey(childNode);
                    appFreeUni__FPv(childNode);
                }
                childNode = AsBpTreeLinks(leftBranch)->right;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    ReleaseBpGroupHeapKey(childNode);
                    appFreeUni__FPv(childNode);
                }
                ReleaseBpGroupHeapKey(leftBranch);
                appFreeUni__FPv(leftBranch);
            }
            void* rightBranch = AsBpTreeLinks(rootNode)->right;
            if (rightBranch != 0) {
                void* childNode = AsBpTreeLinks(rightBranch)->left;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    ReleaseBpGroupHeapKey(childNode);
                    appFreeUni__FPv(childNode);
                }
                childNode = AsBpTreeLinks(rightBranch)->right;
                if (childNode != 0) {
                    void* grandchildNode = AsBpTreeLinks(childNode)->left;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    grandchildNode = AsBpTreeLinks(childNode)->right;
                    if (grandchildNode != 0) {
                        ReleaseBpStringChildSubtrees(owner, grandchildNode);
                        ReleaseBpGroupHeapKey(grandchildNode);
                        appFreeUni__FPv(grandchildNode);
                    }
                    ReleaseBpGroupHeapKey(childNode);
                    appFreeUni__FPv(childNode);
                }
                ReleaseBpGroupHeapKey(rightBranch);
                appFreeUni__FPv(rightBranch);
            }
            ReleaseBpGroupHeapKey(rootNode);
            appFreeUni__FPv(rootNode);
            AsCBpObject(owner)->namedGroupCount = 0;
            AsCBpObject(owner)->namedGroupRoot = 0;
            AsCBpObject(owner)->firstNamedGroup = (char*)&AsCBpObject(owner)->namedGroupRoot;
        }
    }
    // Loader phase: under its mutex, release shared bytes only for a nonempty
    // filename and clear the filename without freeing its retained string capacity.
    // Script state resets here; loadState is not changed by this function.
    {
        void* loader = AsCBpObject(owner)->loader;
        AsCBpObject(owner)->scriptState = kScriptResourceSet;
        void* volatile lockPtr = &AsCBpLoad(loader)->mutexStorage;
        OSLockMutex((OSMutex*)lockPtr);
        unsigned filenameLength;
        unsigned filenameFlags = AsCBpLoad(loader)->filenameStorage;
        filenameLength = (!(filenameFlags >> kStringHeapTagShift))
                             ? (*(unsigned char*)&AsCBpLoad(loader)->filenameStorage) & kInlineStringLengthMask
                             : (*(unsigned*)&AsCBpLoad(loader)->filenameLongLength);
        if (filenameLength != 0) {
            void* sharedFile = AsCBpLoad(loader)->sharedFileHandle;
            if (sharedFile != 0) {
                CSharedFileHandle::Destroy(sharedFile, 1);
                AsCBpLoad(loader)->sharedFileHandle = 0;
            }
            unsigned u = AsCBpLoad(loader)->filenameStorage;
            if (!(u >> kStringHeapTagShift)) {
                ((unsigned char*)&AsCBpLoad(loader)->filenameStorage)[1] = 0;
                unsigned char tagByte = (*(unsigned char*)&AsCBpLoad(loader)->filenameStorage);
                (*(unsigned char*)&AsCBpLoad(loader)->filenameStorage) = (unsigned char)(tagByte & 0x80);
            } else {
                void* filenameBuffer = AsCBpLoad(loader)->filenameHeapData;
                *(unsigned char*)filenameBuffer = 0;
                (*(unsigned*)&AsCBpLoad(loader)->filenameLongLength) = 0;
            }
        }
        OSUnlockMutex((OSMutex*)lockPtr);
    }
    // Sprite-pool phase: delete pooled render objects after resource/cache cleanup.
    ObjectArray_DeleteAllAndClear(AsCBpObject(owner)->objectArray);
    return 1;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
// Replacement-resource boundary (R2PE9B consumer layout, not a version negotiator):
// B2D: +0 bytes[3]/+3 already-converted flag, +4 commandsOffset, +8 stringsOffset,
// +C commandCount. The last three uint32 words and each command's words are stored
// little-endian until BpResource_ByteSwapIndexTable converts them IN PLACE for Wii.
// Commands: {int32 opcode, int32 argumentWords, int32 firstArgument}; byte stride
// 12 + max(argumentWords-1,0)*4. Label opcode 1 resolves firstArgument relative to
// stringsOffset; opcode 2 ends a line; 0x13..0x24 are call labels. Handler table
// s_b2dCommandHandlers (b2d_file_scripts_and_input.cpp) has 38 entries and supplies draw counts plus
// 16-byte scratch units. Work bytes = drawCount*0x1C8 + maxLineScratchUnits*16;
// label count is diagnostic. Preserve line boundaries and script limit 128.
// PHF: party_main_task_and_phf_helpers.cpp's PhfHeader {char signature[8], int32 count, uint32 dataOffset},
// then count entries {char name[24], uint32 relativeDataOffset, uint32 dataSize}.
// PHF integer words are native big-endian on Wii; no PHF swap happens here.
// Entry bytes = file + dataOffset + relativeDataOffset. First '.b2d' suffix wins.
// Signature is compared against kPhfSignature as a C string; no numeric version
// field is checked in either path. These offsets describe the supported shipped
// layout, not a promise that a different asset version will be rejected safely.
// All offsets/counts/terminators and command opcodes must be validated by an asset
// producer: no length, overflow or opcode bounds gate protects these consumers.
// In particular work-size -1 is cast to unsigned allocation bytes, not handled as
// an error. File bytes must be writable, aligned for words and live until reset;
// don't pre-set byte +3 unless all command/index words are already native-endian.
// Advance file-load and object-readiness stages, registering B2D work or locating a B2D entry inside a PHF.
// Returns 1 only when loadState is already kLoadReady at entry; a transition to
// ready still returns 0. resourceBuffer borrows the loader's shared file bytes;
// scriptBuffer owns separately allocated/zeroed command work. PHF entry names
// must be terminated and at least four bytes long for the suffix comparison.
// Missing PHF B2D entries and negative registration results still advance to
// object polling: no failure state or error return is introduced here.
int BpObject_PollLoad(void* object) {
    C5898Object* self = (C5898Object*)object;
    TableLock temp(1, 1);
    int state = self->loadState;
    switch (state) {
    case kLoadWaitingForFile: {
        // Wait for loader vtable +0x0c, then borrow shared bytes under its mutex.
        void* loaderInterface = self->loader;
        void** vt = *(void***)loaderInterface;
        int (*pollLoader)(void*) = (int (*)(void*))vt[3];
        if (pollLoader(loaderInterface) == 0) {
            return 0;
        }
        void* loader = self->loader;
        void* loadedResource;
        {
            WInstanceLock guard(&AsCBpLoad(loader)->mutexStorage);
            loadedResource = CSharedFilePtr_GetLoadedBuffer(AsCBpLoad(loader)->sharedFileHandle);
        }
        if (BpObject_HasPhfSignature(loadedResource) == 0) {
            // Direct B2D: relocate indices, allocate work, register, and execute the
            // texture-registration script with this object as the callback context.
            // On negative registration the context remains set, as in the original.
            self->resourceBuffer = loadedResource;
            BpResource_ByteSwapIndexTable(loadedResource);
            int spriteWorkCount = 0, animationWorkCount = 0, labelWorkCount = 0;
            {
                TableLock t(1, 1);
                int size = B2dFile_CalculateCommandWorkSize(self->resourceBuffer, &spriteWorkCount, &animationWorkCount,
                                                            &labelWorkCount);
                self->scriptBuffer = appMallocUni__FUl((unsigned)size);
                memset(self->scriptBuffer, 0, (unsigned)size);
                BpSlot_RegisterWork(self->script, self->scriptBuffer, spriteWorkCount, animationWorkCount);
                char* base = bpSpriteControlStrings;
                BpObject_DiagnosticStub((int)(base + kWorkSizeDiagnostic), size, spriteWorkCount, animationWorkCount,
                                        labelWorkCount);
            }
            void* fnptr = (void*)BpScript_CreateNamedGsrTexture;
            B2d_SetTextureSubcommandCallback(fnptr);
            {
                TableLock t(1, 1);
                g_currentBpCallbackObject = self;
                int r = B2dFile_Register(self->script, self->resourceBuffer);
                if (r < 0) {
                } else {
                    BpSpritePool_MarkAllInactive(self->objectArray);
                    char* base = bpSpriteControlStrings;
                    B2dFile_ExecuteNamedScript(self->script, (void*)(base + kRegisterSpritesScript));
                    g_currentBpCallbackObject = 0;
                }
            }
            B2d_SetTextureSubcommandCallback((void*)CBpObject_CreateAndReplaceNamedTexture);
            self->loadState = kLoadWaitingForObjects;
            return 0;
        } else {
            // PHF: choose the first .b2d suffix and prepare its command work.
            // This branch does not execute the direct-B2D registration path.
            int count = PhfFile_GetEntryCountWord(loadedResource);
            int idx = 0;
            char* base = bpSpriteControlStrings;
            while (idx < count) {
                char* s = (char*)PhfFile_GetEntry(loadedResource, idx);
                int len = strlen(s);
                int cmp = strcmp(s + len - 4, base);
                if (cmp == 0) {
                    self->resourceBuffer = PhfFile_GetEntryData(loadedResource, idx);
                    BpResource_ByteSwapIndexTable(self->resourceBuffer);
                    int spriteWorkCount = 0, animationWorkCount = 0, labelWorkCount = 0;
                    {
                        TableLock t(1, 1);
                        int size = B2dFile_CalculateCommandWorkSize(self->resourceBuffer, &spriteWorkCount,
                                                                    &animationWorkCount, &labelWorkCount);
                        self->scriptBuffer = appMallocUni__FUl((unsigned)size);
                        memset(self->scriptBuffer, 0, (unsigned)size);
                        BpSlot_RegisterWork(self->script, self->scriptBuffer, spriteWorkCount, animationWorkCount);
                        BpObject_DiagnosticStub((int)(base + kWorkSizeDiagnostic), size, spriteWorkCount,
                                                animationWorkCount, labelWorkCount);
                    }
                    self->loadState = kLoadWaitingForObjects;
                    return 0;
                }
                idx++;
            }
            self->loadState = kLoadWaitingForObjects;
            return 0;
        }
    }
    case kLoadWaitingForObjects: {
        // Poll object vtable +0x18 in key order; any pending object stops the pass.
        void* node = self->firstObject;
        void* sentinel = ((char*)&AsCBpObject(self)->objectRoot);
        while (node != sentinel) {
            void* obj = AsBpIntObjectNode(node)->value;
            void** vt = *(void***)obj;
            int (*m)(void*) = (int (*)(void*))vt[6];
            if (m(obj) == 0) {
                return 0;
            }
            void* p = AsBpTreeLinks(node)->right;
            if (p != 0) {
                while (*(void**)p != 0) {
                    p = *(void**)p;
                }
                node = p;
            } else {
                while (1) {
                    unsigned u = AsBpTreeLinks(node)->parentAndColor;
                    void* r3 = (void*)(u & ~1u);
                    void* r0 = *(void**)r3;
                    if (node == r0) {
                        node = r3;
                        break;
                    }
                    node = r3;
                }
            }
        }
        self->loadState = kLoadReady;
        return 0;
    }
    case kLoadReady:
        return 1;
    default:
        return 0;
    }
}
#pragma pop

} // extern "C"
