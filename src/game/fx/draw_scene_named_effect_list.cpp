#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
// Translation unit 0x801E7790-0x801E8048 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "sdk_protos.h"

// View the borrowed shared-document pointer at its established ABI layout.
inline CSharedDoc* AsCSharedDoc(void* document) { return (CSharedDoc*)document; }
// View the borrowed golf-document pointer at its established ABI layout.
inline CGolfDoc* AsCGolfDoc(void* document) { return (CGolfDoc*)document; }

extern "C" {

#pragma use_lmw_stmw on

int CString_Compare(const void*, const void*);
enum { kMissingRenderCategory = 7, kOrderingNeedsRefresh = 1 };

// Circular doubly linked named-entry list: the embedded end marker at self+0x108
// contains links only, not a payload. Entries carry their object at +0x14, a name
// beginning at +0x1C, and squared distance at +0x4C. Searches visit the first match.
// These accessors never allocate, destroy or retain entries/puppets; callers must
// keep the scene, links and strings alive and serialize list mutation externally.
struct NamedSceneEntry {
    NamedSceneEntry* prev;
    NamedSceneEntry* next;
    union { void* renderKindWord; int renderKind; }; // 0x08: same 32-bit render category, two ABI views
    char reserved0C[8];
    void* puppet; // 0x14
    char reserved18[4];
    char name[1]; // 0x1C: inline name begins here; [1] is a layout view, not its capacity
    char reserved1D[0x24];
    unsigned char enabled; // 0x41: stored verbatim, not normalized to 0/1
    char reserved42[0xA];
    float maximumDistanceSquared; // 0x4C
};

struct DrawSceneNamedEntries {
    char reserved00[0xF0];
    void* textureMesh;
    char reservedF4[4];
    void* primaryUpdater;
    void* secondaryUpdater;
    char reserved100[8];
    NamedSceneEntry* sentinelPrevious; // sentinel begins at 0x108
    NamedSceneEntry* first;
    char reserved110[0x24];
    unsigned char renderKindSortPending;
    char reserved135[0x17];
    void* updateGateWord; // 0x14C: legacy pointer view of an integer gate initialized to 1
};

// Test a nonempty stored name against a borrowed search string; return integer 0/1.
inline int NodeNameMatches(NamedSceneEntry* node, const char* name) {
    int matches;
    if ((signed char)node->name[0] == 0 || name == 0)
        matches = 0;
    else
        matches = (CString_Compare(node->name, name) == 0);
    return matches;
}

// Empty scene hook retained for the virtual interface.
void DrawScene_NoOpHook() {
}

// Return the borrowed puppet for the first matching nonempty stored name, or null.
// Null name, no match and a matching entry with a null puppet all return null.
void* NamedEffectList_FindByName(DrawSceneNamedEntries* self, const char* name)
{
    if (name == 0)
        return 0;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // The end marker has no name; short-circuit before testing its payload.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious)
        return node->puppet;
    return 0;
}

struct ScenePrimaryHookInterface {
    virtual void vf00();
    virtual void vf04();
    virtual void vf08();
    virtual void vf14();
};

// Invoke the primary scene object's fourth virtual callback when present.
void DrawScene_InvokePrimaryHook(void* self)
{
    ScenePrimaryHookInterface* object = (ScenePrimaryHookInterface*)((DrawSceneNamedEntries*)self)->primaryUpdater;
    if (object != 0) {
        object->vf14();
    }
}

// Set the named render entry's enable byte, leaving missing names untouched.
// Null name is ignored; value is a raw byte, so values other than 0/1 are retained.
void DrawScene_SetNamedObjectEnabled(DrawSceneNamedEntries* self, const char* name, unsigned char value)
{
    if (name == 0)
        return;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // Find the first match without reading the embedded end marker as an entry.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious)
        node->enabled = value;
}

// Replace the puppet pointer in an existing named render entry.
// Null name/no match does nothing. The old puppet is not released and the new
// pointer is only assigned; this function performs no ownership bookkeeping.
void DrawScene_SetNamedObjectPuppet(DrawSceneNamedEntries* self, const char* name, void* value)
{
    if (name == 0)
        return;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // Find the first match without reading the embedded end marker as an entry.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious)
        node->puppet = value;
}

// Set an existing entry's render category and request an ordering refresh.
// value carries a 32-bit category, despite this view's pointer type; trajectory
// callers pass air/ground category integers. Null name/no match leaves dirty unchanged.
void NamedEffectList_SetRenderKind(DrawSceneNamedEntries* self, const char* name, void* value)
{
    if (name == 0)
        return;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // Find the entry before publishing both its category and the refresh flag.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious) {
        node->renderKindWord = value;
        vself->renderKindSortPending = kOrderingNeedsRefresh;
    }
}

// Return the first matching entry's render category, or 7 for null/missing name.
// The fallback is a raw category code; this accessor does not report a separate error.
int NamedEffectList_GetRenderKind(DrawSceneNamedEntries* self, const char* name)
{
    if (name == 0)
        return kMissingRenderCategory;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // Find the first match without reading the embedded end marker as an entry.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious)
        return node->renderKind;
    return kMissingRenderCategory;
}

// Square an unsquared scene-space distance and store it in the named entry.
// Null name/no match does nothing; negative input is squared without validation.
void NamedEffectList_SetDistance(DrawSceneNamedEntries* self, const char* name, float distance)
{
    if (name == 0)
        return;
    volatile DrawSceneNamedEntries* vself = self;
    NamedSceneEntry* end = (NamedSceneEntry*)&self->sentinelPrevious;
    NamedSceneEntry* node = self->first;
    // Find the first match without reading the embedded end marker as an entry.
    while (node != end && !NodeNameMatches(node, name)) {
        node = node->next;
    }
    if (node != (NamedSceneEntry*)&vself->sentinelPrevious)
        node->maximumDistanceSquared = distance * distance;
}

struct ScenePrimaryUpdateInterface { 
virtual void v0(); virtual void v3(float);
};
struct SceneSecondaryUpdateInterface { 
virtual void v0(); virtual void v1(); virtual void v4(float);
};

// Scope guard for cursor initialization, including exception cleanup.
struct InstanceLock_u801E7790 {
    OSMutex* lock;
    // Acquire the borrowed registry mutex; OSMutex permits the nested Next lock.
    InstanceLock_u801E7790(OSMutex* registryMutex) { lock = registryMutex; OSLockMutex(registryMutex); }
    // Release exactly this guard's lock acquisition on scope exit.
    ~InstanceLock_u801E7790() { OSUnlockMutex(lock); }
};

extern void* g_sharedDoc;
extern signed char s_renderPacketInitialized;
extern char s_renderPacket[];
extern char s_sceneRenderPacketDestructorRegistration[];
extern void CPvsFxBox_SetPendingDelta(void*, float);
extern void TextureMesh_ScrollTexcoords(void*, float);
extern void GridSurface_UpdateRipples(void*, float);
extern void RenderPacket_Init(void*);
extern void RenderPacket_NoOpHook(void*);
extern void RenderPacket_Destroy(void*);

} // extern C

struct SceneUpdateNode {
    void* data; // Borrowed object payload, not freed by iteration.
    char pad[12];
    SceneUpdateNode* next;
};

// Registry layout shared by all four update rings. head points to the first node
// (not a dummy payload); next wrapping to head ends a pass. cursor is shared
// traversal state, so overlapping passes are not independent. Empty head is null.
// Mutex at +0x20 protects cursor advancement, not object callback execution/lifetime.
struct SceneUpdateRegistry {
    void* head;
    SceneUpdateNode* cursor;
    char reserved08[0x18];
    OSMutex mutex;
};
struct SceneRegistries {
    char reserved00[0x10];
    SceneUpdateRegistry animated;
    SceneUpdateRegistry secondary;
    SceneUpdateRegistry tertiary;
    SceneUpdateRegistry waterSurfaces;
};
enum SceneRegistryOffset {
    kAnimatedRegistry = 0x10, kSecondaryRegistry = 0x48,
    kTertiaryRegistry = 0x80, kWaterRegistry = 0xB8
};
// These byte views preserve aliasing, redundant-test elimination and EH scheduling.
// SCENE_OFFSET must remain an integer constant expression for the template/layout checks.
#define SCENE_OFFSET(type, field) ((int)&((type*)0)->field)
template <int MutexOffset, int LockDisplacement> struct Walk801E7BC0Mutex { char pad[MutexOffset]; OSMutex mutex; };

template <int HeadOffset, int CursorOffset, int MutexOffset, int LockDisplacement>
// Advance one locked registry cursor and return its borrowed payload, or null.
// A null payload also stops the caller's pass, even if more ring nodes follow.
// LockDisplacement changes the byte-view base, not the effective mutex address.
inline void* SceneRegistryNext(void* self)
{
    OSLockMutex(&((Walk801E7BC0Mutex<MutexOffset - LockDisplacement, LockDisplacement>*)((char*)self + LockDisplacement))->mutex);
    SceneUpdateNode* node = *(SceneUpdateNode**)((char*)self + CursorOffset);
    if (node != 0) {
        SceneUpdateNode* next = node->next;
        void* head = *(void**)((char*)self + HeadOffset);
        void* data = node->data;
        if (next == head) next = 0;
        *(SceneUpdateNode**)((char*)self + CursorOffset) = next;
        OSUnlockMutex(&((Walk801E7BC0Mutex<MutexOffset - LockDisplacement, LockDisplacement>*)((char*)self + LockDisplacement))->mutex);
        return data;
    }
    OSUnlockMutex(&((Walk801E7BC0Mutex<MutexOffset - LockDisplacement, LockDisplacement>*)((char*)self + LockDisplacement))->mutex);
    return 0;
}

extern "C" {
// Advance scene effects/updaters and four registries, then invoke the render-packet hook.
// delta is forwarded unchanged in the caller's update timebase; no conversion here.
// A zero +0x14C gate disables the whole pass (CSceneManager retains unk_14C).
// With the gate set, shared/golf docs must exist. Objects remain borrowed and must
// survive callbacks after mutex unlock; this word is not dereferenced as a context.
void DrawScene_UpdateRegisteredObjects(void* self, float delta)
{
    float updateDelta = delta;
    if (((DrawSceneNamedEntries*)self)->updateGateWord == 0)
        return;
    // Phase 1: publish the effect-box delta, then update the two direct objects
    // and texture scrolling. Both direct objects use the same virtual slot.
    {
        void* sharedDocument = g_sharedDoc;
        void* golfDocument = ((void*)AsCSharedDoc(sharedDocument)->golfDoc);
        void* courseEffects = AsCGolfDoc(golfDocument)->courseEffectBox;
        if (courseEffects != 0)
            CPvsFxBox_SetPendingDelta(courseEffects, delta);
    }
    {
        void* updater = ((DrawSceneNamedEntries*)self)->primaryUpdater;
        if (updater != 0) {
            ScenePrimaryUpdateInterface* dispatch = (ScenePrimaryUpdateInterface*)updater;
            dispatch->v3(updateDelta);
        }
    }
    {
        void* updater = ((DrawSceneNamedEntries*)self)->secondaryUpdater;
        if (updater != 0) {
            ScenePrimaryUpdateInterface* dispatch = (ScenePrimaryUpdateInterface*)updater;
            dispatch->v3(updateDelta);
        }
    }
    {
        void* mesh = ((DrawSceneNamedEntries*)self)->textureMesh;
        if (mesh != 0)
            TextureMesh_ScrollTexcoords(mesh, updateDelta);
    }
    // Phase 2: process water, animated, secondary, tertiary rings in that order.
    // Each reset/first fetch holds the recursive mutex twice; later fetches lock
    // once. Each callback runs outside these scopes; no node/object is freed here.
    // Water-surface ripple registry.
    {
        void* registeredObject = 0;
        {
            InstanceLock_u801E7790 cursorLock((OSMutex*)((char*)self + SCENE_OFFSET(SceneRegistries, waterSurfaces.mutex)));
            *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, waterSurfaces.cursor)) = *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, waterSurfaces.head));
            registeredObject = SceneRegistryNext<kWaterRegistry, SCENE_OFFSET(SceneRegistries, waterSurfaces.cursor), SCENE_OFFSET(SceneRegistries, waterSurfaces.mutex), 0>(self);
        }
        // Cursor already names the next node; invoke this payload without the lock.
        if (registeredObject != 0) {
            while (registeredObject != 0) {
                GridSurface_UpdateRipples(registeredObject, updateDelta);
                registeredObject = SceneRegistryNext<kWaterRegistry, SCENE_OFFSET(SceneRegistries, waterSurfaces.cursor), SCENE_OFFSET(SceneRegistries, waterSurfaces.mutex), 4>(self);
            }
        }
    }
    // Primary animated-object registry
    {
        void* registeredObject = 0;
        {
            InstanceLock_u801E7790 cursorLock((OSMutex*)((char*)self + SCENE_OFFSET(SceneRegistries, animated.mutex)));
            *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, animated.cursor)) = *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, animated.head));
            registeredObject = SceneRegistryNext<kAnimatedRegistry, SCENE_OFFSET(SceneRegistries, animated.cursor), SCENE_OFFSET(SceneRegistries, animated.mutex), 0>(self);
        }
        // Animated and tertiary payloads dispatch v3; the secondary ring uses v4.
        if (registeredObject != 0) {
            while (registeredObject != 0) {
                {
                    ScenePrimaryUpdateInterface* dispatch = (ScenePrimaryUpdateInterface*)registeredObject;
                    dispatch->v3(updateDelta);
                }
                registeredObject = SceneRegistryNext<kAnimatedRegistry, SCENE_OFFSET(SceneRegistries, animated.cursor), SCENE_OFFSET(SceneRegistries, animated.mutex), 4>(self);
            }
        }
    }
    // Secondary object registry
    {
        void* registeredObject = 0;
        {
            InstanceLock_u801E7790 cursorLock((OSMutex*)((char*)self + SCENE_OFFSET(SceneRegistries, secondary.mutex)));
            *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, secondary.cursor)) = *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, secondary.head));
            registeredObject = SceneRegistryNext<kSecondaryRegistry, SCENE_OFFSET(SceneRegistries, secondary.cursor), SCENE_OFFSET(SceneRegistries, secondary.mutex), 0>(self);
        }
        // This ring has a different interface/virtual-slot position.
        if (registeredObject != 0) {
            while (registeredObject != 0) {
                {
                    SceneSecondaryUpdateInterface* dispatch = (SceneSecondaryUpdateInterface*)registeredObject;
                    dispatch->v4(updateDelta);
                }
                registeredObject = SceneRegistryNext<kSecondaryRegistry, SCENE_OFFSET(SceneRegistries, secondary.cursor), SCENE_OFFSET(SceneRegistries, secondary.mutex), 4>(self);
            }
        }
    }
    // Tertiary object registry
    {
        void* registeredObject = 0;
        {
            InstanceLock_u801E7790 cursorLock((OSMutex*)((char*)self + SCENE_OFFSET(SceneRegistries, tertiary.mutex)));
            *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, tertiary.cursor)) = *(void**)((char*)self + SCENE_OFFSET(SceneRegistries, tertiary.head));
            registeredObject = SceneRegistryNext<kTertiaryRegistry, SCENE_OFFSET(SceneRegistries, tertiary.cursor), SCENE_OFFSET(SceneRegistries, tertiary.mutex), 0>(self);
        }
        // Finish this ring before lazily acquiring the render-packet singleton.
        if (registeredObject != 0) {
            while (registeredObject != 0) {
                {
                    ScenePrimaryUpdateInterface* dispatch = (ScenePrimaryUpdateInterface*)registeredObject;
                    dispatch->v3(updateDelta);
                }
                registeredObject = SceneRegistryNext<kTertiaryRegistry, SCENE_OFFSET(SceneRegistries, tertiary.cursor), SCENE_OFFSET(SceneRegistries, tertiary.mutex), 4>(self);
            }
        }
    }
    // Phase 3: construct/register the static packet at most once, publish it to
    // the singleton slot when needed, then invoke its hook on every enabled pass.
    if (Sing_805DC7B0() == 0) {
        if (!(signed char)s_renderPacketInitialized) {
            RenderPacket_Init(s_renderPacket);
            __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy, (void*)s_sceneRenderPacketDestructorRegistration);
            s_renderPacketInitialized = 1;
        }
        Sing_805DC7B0() = (void*)s_renderPacket;
    }
    RenderPacket_NoOpHook(Sing_805DC7B0());
    return;
}

} // extern "C"

// Private views must retain the original 32-bit ABI offsets.
typedef char u801E7790_layout_check_0[(SCENE_OFFSET(NamedSceneEntry, puppet) == 0x14) ? 1 : -1];
typedef char u801E7790_layout_check_1[(SCENE_OFFSET(NamedSceneEntry, enabled) == 0x41) ? 1 : -1];
typedef char u801E7790_layout_check_2[(SCENE_OFFSET(NamedSceneEntry, maximumDistanceSquared) == 0x4C) ? 1 : -1];
typedef char u801E7790_layout_check_3[(SCENE_OFFSET(DrawSceneNamedEntries, sentinelPrevious) == 0x108) ? 1 : -1];
typedef char u801E7790_layout_check_4[(SCENE_OFFSET(DrawSceneNamedEntries, renderKindSortPending) == 0x134) ? 1 : -1];
typedef char u801E7790_layout_check_5[(SCENE_OFFSET(DrawSceneNamedEntries, updateGateWord) == 0x14C) ? 1 : -1];
typedef char u801E7790_layout_check_6[(SCENE_OFFSET(SceneRegistries, secondary) == 0x48) ? 1 : -1];
typedef char u801E7790_layout_check_7[(SCENE_OFFSET(SceneRegistries, waterSurfaces) == 0xB8) ? 1 : -1];
