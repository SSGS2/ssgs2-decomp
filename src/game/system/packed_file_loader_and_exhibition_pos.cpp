// Packed file archives (CPackedFile): begins and polls archive loading and reads its resources.
// Also the exhibition model position records (rotation limits and Y rotation), the story movie
// display settings, and the disk-error dialog's initialization.

#include <decomp/static_init.h>
#include <decomp/local_class.h>

#include "sdk_protos.h"
#include "classes/CPackedFile.h"
#include "classes/ExhibitionModelPositionRecord.h"
#include "classes/CDiskErrorDialog_PAL.h"
#include "classes/CExhibition.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CSharedFilePtr.h"
#include "classes/CHashPath.h"

#define AsCDiskErrorDialog_PAL(p) ((CDiskErrorDialog_PAL*)(p))

#define AsCPackedFile(p) ((CPackedFile*)(p))

// The existing exhibition header describes the common fields through rotation X.
struct ExhibitionRotationRecord : ExhibitionModelPositionRecord {
    float rotationLimit1, rotationLimit2, rotationY;
};
// Display-mode configuration owns a vertex-input block and a frame-based settle timer.
struct StoryDisplayState {
    char configuration[0x34];
    unsigned int displayMode;
    char vertexInput[0x14];
    int settleFrames;
};
enum PackedFileLoadState {
    kWaitingForArchive = 0,
    kReadingArchive = 1,
    kLoadFinished = 2
};
enum StoryDisplayTiming {
    kDisplaySettleFrames = 90
};
// Movie output occupies twenty 32-bit words. Dimensions, viewport copies,
// mode/timer overlays and the TV-dependent word are established here. Unknown fields keep their
// byte offsets in the name: the literal defaults below are not GX format enums.
struct MovieOutputConfiguration {
    int width, height;
    int unresolved08, unresolved0C, unresolved10, unresolved14;
    int unresolved18, unresolved1C, unresolved20, unresolved24;
    int unresolved28, tvDependentWord, unresolved30;
    unsigned int displayMode;
    int viewportWord0, viewportWord1, viewportWidth, viewportHeight;
    int unresolved48, settleFrames;
};
typedef char MovieOutputConfiguration_size[(sizeof(MovieOutputConfiguration) == 80) ? 1 : -1];
// These two routines' receiver views overlap the same mode and timer words.
typedef char Movie_mode_overlay
    [((int)&((MovieOutputConfiguration*)0)->displayMode == (int)&((StoryDisplayState*)0)->displayMode) ? 1 : -1];
typedef char Movie_timer_overlay
    [((int)&((MovieOutputConfiguration*)0)->settleFrames == (int)&((StoryDisplayState*)0)->settleFrames) ? 1 : -1];

enum MovieConfigDefaults {
    kMovieDefaultWidth = 640,
    kMovieDefaultHeight = 480
};

extern "C" {

extern const float kFloat_6_2831855;
extern float s_twoPi_8029BFC0;
extern float s_twoPi_8029C264;
extern float s_twoPi_8029D05C;
extern const float kFloat_0_0_Exhibition;
extern const float kFloat_0_DiskErrorDialog;
extern void* g_resourceManager;
extern char __vt__11CPackedFile[];
extern char __vt__16IDiskErrorDialog[];
extern char __vt__20CDiskErrorDialog_PAL[];
extern const volatile int defaultVideoViewport[];
void* Table_GetEntry(void*, int); // Locate an indexed table record.
void Engine_SetOutputRectangle(void*);
void Engine_ChangeDisplayMode(void*);
int CSharedFilePtr_GetLoadedBuffer(void*);
void* __nw__FUl(unsigned int);
void FloatTween_Init(void*);
void* SharedFile_Create(void*, int);
void* byteCount(void*, void*, int);
void* Archive_Open(void*, void*, int);
void Archive_GetEntryInfo(void*, int, void*);
void Archive_ReadEntry(void*, int, void*, int, int);
void Archive_Close(void*);
void MSLString_LowercaseInPlace(void*);
int CreateSharedTextureResource__FPCci(void*, int);
int Get__15ISharedResourceFv(void*);
void CreateOnMemoryResource__22CSharedTextureResourceFPvUl(void*, void*, int);
void* __nwa__FUl(unsigned int);
void* appMallocDummy__FUl(unsigned int);
void* memcpy(void*, const void*, unsigned int);
void* memset(void*, int, unsigned int);
void __dla__FPv(void*);
extern char s_ownedByteBufferVtable[], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[],
    __vt__Q23std12length_error[];
extern const char resourceArchiveExtensionAndExceptionStrings[];
extern void __dt__Q23std12length_errorFv();
int Video_GetDtvOutputCode(void*);
void VideoConfig_InitFromTvFormat(void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
// Return the constant success value for unresolved exhibition virtual slot 1.
int CExhibition::model_pos_vfunc_1() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 2.
int CExhibition::model_pos_vfunc_2() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 3.
int CExhibition::model_pos_vfunc_3() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 4.
int CExhibition::model_pos_vfunc_4() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 5.
int CExhibition::model_pos_vfunc_5() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 6.
int CExhibition::model_pos_vfunc_6() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 7.
int CExhibition::model_pos_vfunc_7() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 8.
int CExhibition::model_pos_vfunc_8() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 9.
int CExhibition::model_pos_vfunc_9() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 10.
int CExhibition::model_pos_vfunc_10() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 11.
int CExhibition::model_pos_vfunc_11() {
    return 1;
}
// Return the constant success value for unresolved exhibition virtual slot 12.
int CExhibition::model_pos_vfunc_12() {
    return 1;
}

// Return the constant success value for unresolved exhibition virtual slot 13.
int CExhibition::model_pos_vfunc_13() {
    return 1;
}
// Initialize 640x480 movie output and adapt its dimensions to the current TV format.
void* StoryMovieConfig_InitDefaults(void* p) {
    MovieOutputConfiguration* config = (MovieOutputConfiguration*)p;
    int g0 = defaultVideoViewport[0];
    int g1 = defaultVideoViewport[1];
    int g2 = defaultVideoViewport[2];
    int g3 = defaultVideoViewport[3];
    config->width = kMovieDefaultWidth;
    config->height = kMovieDefaultHeight;
    config->unresolved08 = 6;
    config->unresolved0C = 0;
    config->unresolved10 = 0;
    config->unresolved14 = 0;
    config->unresolved18 = 1;
    config->unresolved1C = 0;
    config->unresolved20 = 0;
    config->unresolved24 = 1;
    config->unresolved28 = 0x16;
    config->tvDependentWord = 0x20;
    config->unresolved30 = 0;
    config->displayMode = 2;
    config->viewportWord0 = g0;
    config->viewportWord1 = g1;
    config->viewportWidth = g2;
    config->viewportHeight = g3;
    config->unresolved48 = 2;
    // Override both output and viewport dimensions together for this TV-format branch.
    if ((unsigned int)Video_GetDtvOutputCode(p) == 1) {
        int values[12];
        VideoConfig_InitFromTvFormat(values);
        if (values[2] == 0) {
            config->tvDependentWord = 0x40;
            config->width = values[0];
            config->height = values[1];
            config->viewportWidth = values[0];
            config->viewportHeight = values[1];
        }
    }
    config->settleFrames = 0;
    return p;
}
// Initialize the exhibition-position unit's writable two-pi cache.
void ExhibitionPosition_InitTwoPiCache() {
    s_twoPi_8029BFC0 = kFloat_6_2831855;
}
// Release loaded resources and shared-file handles, then optionally free the archive object.
CPackedFile::~CPackedFile() {
    *(void**)this = __vt__11CPackedFile;
    void** slot = (*(void***)&resources);
    while (slot != (void**)((char*)resources + resourceCount * 4)) {
        if (*slot != 0)
            (*(void (**)(void*))((char*)*(void**)*slot + 0x0c))(*slot);
        *slot = 0;
        ++slot;
    }
    resourceCount = 0;
    slot = (*(void***)&sharedFileHandles);
    while (slot != (void**)((char*)sharedFileHandles + sharedFileHandleCount * 4)) {
        if (*slot != 0) {
            CSharedFileHandle::Destroy(*slot, 1);
            *slot = 0;
        }
        ++slot;
    }
    sharedFileHandleCount = 0;
    void* handle = sharedFileHandle;
    if (handle != 0) {
        CSharedFileHandle::Destroy(handle, 1);
        sharedFileHandle = 0;
    }
    if (sharedFileHandles != 0) {
        sharedFileHandleCount -= sharedFileHandleCount;
        appFreeUni__FPv(sharedFileHandles);
    }
    if (resources != 0) {
        resourceCount -= resourceCount;
        appFreeUni__FPv(resources);
    }
}
// Initialize the packed-file unit's writable two-pi cache.
void PackedFile_InitTwoPiCache() {
    s_twoPi_8029C264 = kFloat_6_2831855;
}
// Initialize the disk-error-dialog unit's writable two-pi cache.
void DiskErrorDialog_InitTwoPiCache() {
    s_twoPi_8029D05C = kFloat_6_2831855;
}

// Store a changed display mode and report whether it differs from the previous mode.
int StoryMovieConfig_SetDisplayModeIfChanged(void* displayState, unsigned int newMode) {
    if (((StoryDisplayState*)displayState)->displayMode == newMode)
        return 0;
    ((StoryDisplayState*)displayState)->displayMode = newMode;
    return 1;
}

// Report whether asynchronous archive loading has reached its finished state.
int CPackedFile::IsLoadFinished() {
    unsigned int state = (*(unsigned int*)&loadState);
    return (state == kLoadFinished) ? 1 : 0;
}

// Count down display-settle frames and report completion after the counter reaches zero.
int StoryMovieConfig_PollDisplaySettled(void* displayState) {
    int remainingFrames = ((StoryDisplayState*)displayState)->settleFrames;
    if (remainingFrames != 0) {
        ((StoryDisplayState*)displayState)->settleFrames = remainingFrames - 1;
        return 0;
    }
    return 1;
}

// Advance archive acquisition and resource loading by one state-machine step.
int CPackedFile::PollLoad() {
    int state = loadState;
    switch (state) {
    case kWaitingForArchive: {
        void* handle = sharedFileHandle;
        if (handle == 0) {
            loadState = kLoadFinished;
        } else if (CSharedFilePtr_GetLoadedBuffer(handle)) {
            loadState = kReadingArchive;
        }
        break;
    }
    case kReadingArchive: {
        CPackedFile::LoadArchiveResources(this);
        void* handle = sharedFileHandle;
        if (handle != 0) {
            CSharedFileHandle::Destroy(handle, 1);
            sharedFileHandle = 0;
        }
        loadState = kLoadFinished;
        break;
    }
    default:
        break;
    }
    return loadState == kLoadFinished;
}

// Minimal virtual interface needed by the allocation/initialization failure path.
class PackedFileInitializer {
public:
    virtual void destroy(int);
    virtual int initialize(void*);
};
// Allocate an empty archive object and discard it if its virtual initializer fails.
void* CPackedFile::Create(void* arg) {
    CPackedFile* obj = (CPackedFile*)__nw__FUl(0x24);
    if (obj != 0) {
        obj->vtbl = __vt__11CPackedFile;
        obj->loadState = kLoadFinished;
        obj->sharedFileHandle = 0;
        obj->resources = 0;
        obj->resourceCount = 0;
        obj->resourceCapacity = 0;
        obj->sharedFileHandles = 0;
        obj->sharedFileHandleCount = 0;
        obj->sharedFileHandleCapacity = 0;
    }
    if (obj == 0)
        return 0;
    if (!((PackedFileInitializer*)obj)->initialize(arg)) {
        if (obj != 0)
            ((PackedFileInitializer*)obj)->destroy(1);
        return 0;
    }
    return obj;
}

// These reset slots occur in both base and PAL vtables. Their distinct
// effects remain unresolved; preserve order and reload the vtable each time.
struct DiskDialogResetSlots {
    char reserved00[0x1c];
    void (*reset1C)(void* receiver);
    char reserved20[4];
    void (*reset24)(void* receiver);
};
// Invoke initialization slot +0x1c on the object's current vtable.
static inline void CallDiskDialogResetSlot1C(void* object) {
    (*(DiskDialogResetSlots**)object)->reset1C(object);
}
// Invoke initialization slot +0x24 on the object's current vtable.
static inline void CallDiskDialogResetSlot24(void* object) {
    (*(DiskDialogResetSlots**)object)->reset24(object);
}

// Initialize the base dialog tween, install the PAL vtable, and reset image resources.
// FloatTween_Init is declared generically above; this call supplies receiver in r3
// and zero in f1. Each reset loads the function pointer stored in that vtable slot.
// arg carries an integer archive-entry index in a pointer-sized word, stored verbatim.
// CDiskErrorDialog_PAL_LoadImageTexture consumes it as an index, remapping zero to entry 5.
void* CDiskErrorDialog_PAL::Init(void* arg) {
    vtbl = __vt__16IDiskErrorDialog;
    ((void (*)(void*, float))FloatTween_Init)(opacityTweenPrefix, kFloat_0_DiskErrorDialog);
    CallDiskDialogResetSlot1C(this);
    CallDiskDialogResetSlot24(this);
    vtbl = __vt__20CDiskErrorDialog_PAL;
    archiveEntryIndex = arg;
    imageIndex = -1;
    texture = 0;
    CallDiskDialogResetSlot1C(this);
    CallDiskDialogResetSlot24(this);
    return this;
}

/* 0x8029BFC0-0x8029BFD0: r4 (index) is never set, so the caller's second argument goes to Table_GetEntry */
// Read the first rotation limit, returning zero for a missing record.
float CExhibition::model_pos_GetRotationLimit1(const void* p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0)
        return ((ExhibitionRotationRecord*)q)->rotationLimit1;
    return kFloat_0_0_Exhibition;
}

// Read the second rotation limit, returning zero for a missing record.
float CExhibition::model_pos_GetRotationLimit2(const void* p, int index) {
    void* q = Table_GetEntry(*(void**)p, index);
    if (q != 0)
        return ((ExhibitionRotationRecord*)q)->rotationLimit2;
    return kFloat_0_0_Exhibition;
}

// Read the Y rotation, returning zero for a missing record.
float CExhibition::model_pos_GetRotationY(const void* p, int index) {
    void* q = *(void**)p;
    q = Table_GetEntry(q, index);
    if (q != 0)
        return ((ExhibitionRotationRecord*)q)->rotationY;
    return kFloat_0_0_Exhibition;
}

// Apply vertex-input and display settings, then allow 90 frames for settling.
void StoryMovieConfig_ApplyDisplayMode(void* displayState) {
    Engine_SetOutputRectangle(((StoryDisplayState*)displayState)->vertexInput);
    Engine_ChangeDisplayMode(displayState);
    ((StoryDisplayState*)displayState)->settleFrames = kDisplaySettleFrames;
}

// Replace the previous archive handle and begin loading an existing resource path.
int CPackedFile::BeginArchiveLoad(void* path) {
    if (path == 0)
        return 0;
    void* previousHandle = sharedFileHandle;
    if (previousHandle != 0) {
        CSharedFileHandle::Destroy(previousHandle, 1);
        sharedFileHandle = 0;
    }
    void* r = (void*)(long)((CHashPath*)g_resourceManager)->Exists((const char*)path);
    if (r == 0)
        return 0;
    void* handle = SharedFile_Create(path, 0);
    sharedFileHandle = handle;
    if (handle == 0)
        return 0;
    loadState = kWaitingForArchive;
    return 1;
}

// Pointer counts use a 32-bit address space with four-byte entries.
enum PackedFilePointerLimits {
    kMaxPackedFilePointers = 0x3fffffffU,
    kThreeFifthsGrowthLimit = 0x15555555U,
    kHalfGrowthLimit = 0x2aaaaaaaU
};

// Choose pointer-vector growth (3/5, then 1/2, then saturation).
// Reference scratch slots retain the compiler's min-reference temporaries.
// oldCapacity/extra count pointers, not bytes; the caller checks the hard limit.
static inline void PackedFileGrowthCapacity(unsigned int& oldCapacity, unsigned int& extra,
                                            unsigned int& incrementThree, unsigned int& incrementHalf,
                                            unsigned int& newCapacity) {
    if (oldCapacity < kThreeFifthsGrowthLimit) {
        incrementThree = ((oldCapacity + 1) * 3) / 5;
        const unsigned int& bigger = incrementThree < extra ? extra : incrementThree;
        newCapacity = oldCapacity + bigger;
    } else if (oldCapacity < kHalfGrowthLimit) {
        incrementHalf = (oldCapacity + 1) / 2;
        const unsigned int& bigger = incrementHalf < extra ? extra : incrementHalf;
        newCapacity = oldCapacity + bigger;
    } else
        newCapacity = kMaxPackedFilePointers;
}

// Exception construction is kept at the call site to preserve EH object lifetimes.
// INDEX selects one of three independent error/allocator slots in append.
// This envelope remains a macro because its local CharGuard scopes define
// the original exception cleanup table. Arguments must be side-effect free.
#define PACKED_FILE_LENGTH_ERROR(INDEX)                                                                           \
    {                                                                                                             \
        const char* message = resourceArchiveExtensionAndExceptionStrings + 5;                                    \
        Error& error = errors[INDEX];                                                                             \
        error.vtable = __vt__Q23std11logic_error;                                                                 \
        char* allocator = allocators[INDEX];                                                                      \
        allocator[0] = 0;                                                                                         \
        unsigned int length = strlen(message) + 1;                                                                \
        error.text = (char*)__nwa__FUl((length * 2) - length);                                                    \
        {                                                                                                         \
            char* text = error.text;                                                                              \
            CharGuard guard;                                                                                      \
            guard.data = error.text;                                                                              \
            guard.allocator = allocator;                                                                          \
            Ref* ref = (Ref*)__nw__FUl(0x10);                                                                     \
            if (ref) {                                                                                            \
                ref->strong = 1;                                                                                  \
                ref->weak = 1;                                                                                    \
                ref->vtable = s_sharedCharDeleterVtable;                                                          \
                ref->data = text;                                                                                 \
            }                                                                                                     \
            error.ref = ref;                                                                                      \
            guard.data = 0;                                                                                       \
        }                                                                                                         \
        strcpy(error.text, message);                                                                              \
        error.vtable = __vt__Q23std12length_error;                                                                \
        __throw(resourceArchiveExtensionAndExceptionStrings + 0x19, &error, (void*)__dt__Q23std12length_errorFv); \
    }
// Pointer-vector append: reuse capacity, or allocate -> move pointers -> clear old
// slots -> swap storage. Growth owns the replaced allocation after commit; pointed-to
// resources are transferred, never released here. 0x3fffffff is the 32-bit pointer limit.
#define PACKED_FILE_APPEND_RETAINED_POINTER(DATA, SIZE, CAPACITY, VALUE)                          \
    if (CAPACITY > SIZE) {                                                                        \
        ++SIZE;                                                                                   \
        DATA[SIZE - 1] = VALUE;                                                                   \
    } else {                                                                                      \
        Error errors[3];                                                                          \
        char allocators[3][4];                                                                    \
        if (kMaxPackedFilePointers - CAPACITY < 1U)                                               \
            PACKED_FILE_LENGTH_ERROR(2);                                                          \
        unsigned int incrementThree, incrementHalf, extra;                                        \
        extra = SIZE + 1 - CAPACITY;                                                              \
        Growth growth;                                                                            \
        growth.data = 0;                                                                          \
        unsigned int oldCapacity = CAPACITY;                                                      \
        growth.size = 0;                                                                          \
        growth.capacity = 0;                                                                      \
        growth.originalCapacity = &CAPACITY;                                                      \
        growth.position = 0;                                                                      \
        if (extra > kMaxPackedFilePointers - oldCapacity)                                         \
            PACKED_FILE_LENGTH_ERROR(0);                                                          \
        unsigned int newCapacity;                                                                 \
        PackedFileGrowthCapacity(oldCapacity, extra, incrementThree, incrementHalf, newCapacity); \
        if (newCapacity > kMaxPackedFilePointers)                                                 \
            PACKED_FILE_LENGTH_ERROR(1);                                                          \
        void** allocation = (void**)appMallocDummy__FUl(newCapacity * 4);                         \
        growth.position = SIZE;                                                                   \
        (allocation + SIZE)[growth.size] = VALUE;                                                 \
        growth.capacity = newCapacity;                                                            \
        growth.data = allocation;                                                                 \
        ++growth.size;                                                                            \
        void** oldData = (void**)DATA;                                                            \
        int moved = ((char*)(oldData + SIZE) - (char*)oldData) / 4;                               \
        growth.position -= moved;                                                                 \
        memcpy(allocation + growth.position, oldData, moved * 4);                                 \
        memset(oldData, 0, moved * 4);                                                            \
        growth.size += moved;                                                                     \
        void** newData = growth.data;                                                             \
        unsigned int newSize = growth.size;                                                       \
        unsigned int oldCap = CAPACITY;                                                           \
        SIZE = 0;                                                                                 \
        growth.data = (void**)DATA;                                                               \
        CAPACITY = growth.capacity;                                                               \
        growth.capacity = oldCap;                                                                 \
        DATA = (TYPEOF_DATA)newData;                                                              \
        SIZE = newSize;                                                                           \
        growth.size = 0;                                                                          \
    }

// Read archive entries, create their texture resources, and retain their backing file handles.
// Return 0 for missing/unready/unopenable archive; 1 once all entries were visited
// (individual missing paths/resources can be skipped). resources owns retained
// texture references; sharedFileHandles owns file handles. The archive handle is borrowed.
// Per-entry Buffer owns only temporary bytes and frees them even on continue.
int CPackedFile::LoadArchiveResources(void* p) {
    struct Object {
        virtual void unused() DECOMP_LOCAL_VIRTUAL;
        virtual void release() DECOMP_LOCAL_VIRTUAL;
    };
    struct State {
        void* vtable;
        int state;
        void* handle;
        Object** resources;
        unsigned int resourceCount, resourceCapacity;
        void** sharedFileHandles;
        unsigned int sharedFileHandleCount, sharedFileHandleCapacity;
    };
    struct Record {
        int index;
        char name[0x124];
        int size;
    };
    struct Buffer {
        void* vtable;
        int size;
        void* data;
        // Release the temporary entry bytes after a resource has consumed them.
        ~Buffer() {
            vtable = s_ownedByteBufferVtable;
            if (data)
                __dla__FPv(data);
            data = 0;
        }
    };
    struct Growth {
        void** data;
        unsigned int size, capacity;
        unsigned int* originalCapacity;
        unsigned int position;
        // Free old pointer-array storage after commit, or new storage on rollback.
        ~Growth() {
            if (data) {
                size = 0;
                appFreeUni__FPv(data);
            }
        }
    };
    struct Ref {
        void* vtable;
        int strong, weak;
        char* data;
    };
    struct Error {
        void* vtable;
        char* text;
        Ref* ref;
    };
    struct CharGuard {
        char* data;
        char* allocator;
        // Release exception text if ownership has not transferred to its shared ref block.
        ~CharGuard() {
            if (data)
                __dla__FPv(data);
        }
    };
    void* file;
    int i;
    State* self;
    void* data;
    volatile int count;
    self = (State*)p;
    if (!self->handle)
        return 0;
    // Phase 1: drop previously retained textures, then handles; keep vector capacity.
    Object** object = self->resources;
    unsigned int objectCount;
    while (object != self->resources + (objectCount = self->resourceCount)) {
        if (*object) {
            (*object)->release();
            *object = 0;
        }
        ++object;
    }
    self->resourceCount -= objectCount;
    void** sharedFileHandle = self->sharedFileHandles;
    unsigned int sharedFileHandleCount;
    while (sharedFileHandle != self->sharedFileHandles + (sharedFileHandleCount = self->sharedFileHandleCount)) {
        if (*sharedFileHandle) {
            CSharedFileHandle::Destroy(*sharedFileHandle, 1);
            *sharedFileHandle = 0;
        }
        ++sharedFileHandle;
    }
    self->sharedFileHandleCount -= sharedFileHandleCount;
    // Phase 2: open the loaded archive buffer and obtain the entry count (index -1).
    data = (void*)CSharedFilePtr_GetLoadedBuffer(self->handle);
    if (!data)
        return 0;
    file = Archive_Open(data, (void*)CSharedFilePtr::GetSize(self->handle), 3);
    if (!file)
        return 0;
    Record header;
    Archive_GetEntryInfo(file, -1, &header);
    count = header.index;
    // Phase 3: normalize name -> require a known path -> read bytes -> route by extension.
    for (i = 0; i < count; ++i) {
        Record entry;
        Archive_GetEntryInfo(file, i, &entry);
        MSLString_LowercaseInPlace(entry.name);
        if (!((CHashPath*)g_resourceManager)->Exists((const char*)entry.name) || (unsigned int)entry.name == 0)
            continue;
        Buffer buffer;
        buffer.vtable = s_ownedByteBufferVtable;
        buffer.size = entry.size;
        buffer.data = 0;
        if (entry.size > 0)
            buffer.data = __nwa__FUl(entry.size);
        Archive_ReadEntry(file, i, buffer.data, buffer.size, 3);
        char* extension = strrchr(entry.name, '.');
        if (!extension)
            continue;
        // Matching extension creates/retains a texture; other extensions retain a file handle.
        if (strcmp(extension, resourceArchiveExtensionAndExceptionStrings) == 0) {
            Object* loaded = (Object*)CreateSharedTextureResource__FPCci(entry.name, 1);
            if (!loaded)
                continue;
            if (!Get__15ISharedResourceFv(loaded))
                CreateOnMemoryResource__22CSharedTextureResourceFPvUl(loaded, buffer.data, buffer.size);
#define TYPEOF_DATA Object**
            PACKED_FILE_APPEND_RETAINED_POINTER(self->resources, self->resourceCount, self->resourceCapacity, loaded);
#undef TYPEOF_DATA
        } else {
            // Event scripts (.obj) and BinaryTable files (.ecb) are file-order bytes (big-endian on disc).
            void* loaded = byteCount(entry.name, buffer.data, buffer.size);
            if (!loaded)
                continue;
#define TYPEOF_DATA void**
            PACKED_FILE_APPEND_RETAINED_POINTER(self->sharedFileHandles, self->sharedFileHandleCount,
                                                self->sharedFileHandleCapacity, loaded);
#undef TYPEOF_DATA
        }
    }
    // Phase 4: entry buffers are gone; the two collections now own successful results.
    Archive_Close(file);
    return 1;
#undef PACKED_FILE_APPEND_RETAINED_POINTER
#undef PACKED_FILE_LENGTH_ERROR
}
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462484", _ctors_u8029BFC0_0, ExhibitionPosition_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462484", _ctors_u8029BFC0_1, PackedFile_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU80462484", _ctors_u8029BFC0_2, DiskErrorDialog_InitTwoPiCache);
#pragma pop
