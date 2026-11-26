// Movie playback and the banner: the movie controller wraps the native movie player (state,
// readiness and sample drawing). CBanner reads the banner file from shared storage and parses it.

#include <decomp/static_init.h>
// Compare borrowed resource-array iterator positions.
template <class T> inline bool same_pos(const T& lhs, const T& rhs) {
    return lhs == rhs;
}

#include "game/singletons.h"
#include "classes/CSharedFileHandle.h"
#include "classes/CBanner.h"
#include "classes/CSharedFileStore.h"
#include "classes/CHashPath.h"
struct BannerLifecycleDispatch {
    virtual void destroy(int);
    virtual int initialize();
};

extern "C" {

// requestedName is a BORROWED pointer, not a copied string: caller must retain
// its text unchanged until reset/replacement. playback is owned by the controller.
struct MovieControllerView {
    void* playback;
    const char* requestedName;
    int readyLatched;
};
struct MoviePlaybackStringPool {
    char advertise[0xA];
    char alternateSuffix[8]; // "_pl.thp"
    char standardSuffix[8];  // "_st.thp"
};
struct MovieSaveOptions {
    char reserved[0x4E8];
    unsigned int flags;
};
enum {
    kMovieVariantShift = 28,
    kMovieVariantMask = 3,
    kAlternateMovieVariant = 2,
    kMovieInactive = 0,
    kMovieDecoderState3 = 1,
    kMovieDecoderState4 = 2,
    kBannerPending = 0,
    kBannerInitializationAttemptFinished = 1
};

void appFreeUni__FPv(void*);

// Narrow and wide strings share the heap flag and pointer words.
struct StringUnwindCleanup_u80293A54 {
    const unsigned int* words;
    bool active;
    // Arm exceptional cleanup for a borrowed local string representation.
    explicit StringUnwindCleanup_u80293A54(const void* string) : words((const unsigned int*)string), active(true) {}
    // Disarm before the explicit normal-path free.
    void release() {
        active = false;
    }
    // Release owned heap text only when normal cleanup has not run.
    ~StringUnwindCleanup_u80293A54() {
        if (active && (words[0] & 0x80000000u)) {
            appFreeUni__FPv((void*)words[2]);
        }
    }
};

void Movie_ReleaseSampleBuffers(void*);
void Movie_ReturnZero(void*); // No-op: original body only returns zero.
void MovieRenderer_DrawCurrentSamples(void*);
void Movie_AcquirePlaybackReference(void*);
void Movie_ReleasePlaybackReference(void*);
void MovieAudio_ReleaseSampleBuffersAndManager(void*);
void MoviePlaybackState_Init(void*);
void MoviePlayback_DeleteIfFlagged(void*, int);
void* Movie_OpenResource(void*, const char*, int, int);
int strcmp(const char*, const char*);
void SaveData_Ctor(void*);
void* SaveData_GetCurrentRecord(void*);
unsigned int strlen(const char*);
extern const char moviePlaybackStrings[];
extern signed char s_saveDataInitialized;
extern void* g_resourceManager;
extern char s_saveData[];
extern float kAdvertiseMovieVolume; // 1.0: advertise movie volume
extern float kStandardMovieVolume;  // 0.7: other movie volume
void appFreeUni__FPv(void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const void*, const void*, const void*);
void MovieAudio_SetMasterVolume(float);
void* __nw__FUl(unsigned int);
void __dl__FPv(void*);
int Movie_IsDecoderReadyOrState4(void*); // readiness predicate for native playback
int Movie_IsDecoderState3(void*);
int Movie_IsDecoderState4(void*);
int Movie_ConfigureAndStartDecoder(void*, void*); // native playback start request
int CSharedFilePtr_GetLoadedBuffer(void*);
int SaveBanner_ParseLocalizedTextSections(void*);
void SaveBanner_BindLoadedImages(void*);
void SaveBanner_DestroyLocalizedTextSubtree(void*, void*);
extern float kFloat_6_2831855;
extern float s_movieControllerTwoPi; // movie unit copy of two pi
extern float s_bannerTwoPi;          // banner unit copy of two pi
extern char __vt__16CSharedFileStore[];
extern char __vt__7CBanner[];

// Release native playback sample buffers without deleting the controller.
void MovieController_StopAndFreeSamples(void* self) {
    Movie_ReleaseSampleBuffers(((MovieControllerView*)self)->playback);
}
// Latch native readiness once true and keep returning the latched value.
int MovieController_PollReadyLatch(void* self) {
    if (((MovieControllerView*)self)->readyLatched == 0 &&
        Movie_IsDecoderReadyOrState4(((MovieControllerView*)self)->playback))
        ((MovieControllerView*)self)->readyLatched = 1;
    return ((MovieControllerView*)self)->readyLatched;
}
// Reject a start request while native playback is in decoder state 3.
int MovieController_StartPlayback(void* self, void* arg) {
    if (Movie_IsDecoderState3(((MovieControllerView*)self)->playback))
        return 0;
    return Movie_ConfigureAndStartDecoder(((MovieControllerView*)self)->playback, arg);
}
// Delete the native playback object and optionally the controller.
void* MovieController_Dtor(void* self, int deleting) {
    if (self) {
        if (((MovieControllerView*)self)->playback) {
            MoviePlayback_DeleteIfFlagged(((MovieControllerView*)self)->playback, 1);
            ((MovieControllerView*)self)->playback = 0;
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
// Map native decoder states 3 and 4 to wrapper states 1 and 2; otherwise return zero.
int Movie_GetPlaybackState(void* self) {
    void* object = ((MovieControllerView*)self)->playback;
    if (Movie_IsDecoderState3(object))
        return kMovieDecoderState3;
    object = ((MovieControllerView*)self)->playback;
    if (Movie_IsDecoderState4(object) && !Movie_IsDecoderState3(((MovieControllerView*)self)->playback))
        return kMovieDecoderState4;
    return kMovieInactive;
}
struct SharedFileStoreStringView {
    void* vtable;
    unsigned int flags;
    int stringLength;
    void* heapText;
};
// Free the stored filename only when its small-string heap flag is set.
CSharedFileStore::~CSharedFileStore() {
    SharedFileStoreStringView* self = (SharedFileStoreStringView*)this;
    if (&self->flags) {
        if (self->flags >> 31)
            appFreeUni__FPv(self->heapText);
    }
}
// Copy inline filename bytes or deep-copy heap text, then copy the metadata word.
CSharedFileStore::CSharedFileStore(const CSharedFileStore& src) {
    unsigned int flags = src.nameFlags;
    vtbl = &__vt__16CSharedFileStore;
    if ((flags >> 31) == 0) {
        nameFlags = flags;
        nameLength = src.nameLength;
        nameText = src.nameText;
    } else {
        unsigned int n = src.nameLength;
        nameFlags = 0;
        nameLength = 0;
        String_ReserveCapacity(&nameFlags, n);
        int spC;
        unsigned char c = *(unsigned char*)&spC;
        String_ReplaceRange(&nameFlags, 0, 0, src.nameText, src.nameText + src.nameLength, &c);
    }
    metadata = src.metadata;
}
// Construct native playback state in its exact 16-byte storage allocation.
struct NativeMoviePlaybackStorage {
    char data[0x10];
    NativeMoviePlaybackStorage() {
        MoviePlaybackState_Init(this);
    }
};
// Allocate the native playback state and clear the name cache and readiness latch.
void* MovieController_Ctor(void* self) {
    ((MovieControllerView*)self)->playback = 0;
    ((MovieControllerView*)self)->requestedName = 0;
    void* p = new NativeMoviePlaybackStorage;
    ((MovieControllerView*)self)->playback = p;
    ((MovieControllerView*)self)->readyLatched = 0;
    return self;
}
// Invoke the native no-op hook; its constant zero return value is ignored.
void MovieController_InvokeNoOpHook(void* self) {
    Movie_ReturnZero(((MovieControllerView*)self)->playback);
}
// Draw the current samples from the controller-owned native playback object.
void MoviePlayer_DrawCurrentSamples(void* self) {
    MovieRenderer_DrawCurrentSamples(((MovieControllerView*)self)->playback);
}
// Acquire one playback reference on the controller-owned movie.
void MovieController_AcquirePlaybackReference(void* self) {
    Movie_AcquirePlaybackReference(((MovieControllerView*)self)->playback);
}
// Release one playback reference without deleting the controller wrapper.
void MovieController_ReleasePlaybackReference(void* self) {
    Movie_ReleasePlaybackReference(((MovieControllerView*)self)->playback);
}
// Initialize the movie controller unit copy of two pi at static startup.
void MovieController_InitStaticConstants(void) {
    s_movieControllerTwoPi = kFloat_6_2831855;
}
// Initialize the banner unit copy of two pi at static startup.
void CBanner_InitStaticConstants(void) {
    s_bannerTwoPi = kFloat_6_2831855;
}
// Forget the cached name/readiness and release movie audio buffers and manager.
void MovieController_ReleaseAudioAndResetLatches(void* self) {
    ((MovieControllerView*)self)->requestedName = 0;
    ((MovieControllerView*)self)->readyLatched = 0;
    MovieAudio_ReleaseSampleBuffersAndManager(((MovieControllerView*)self)->playback);
}

struct TempString {
    union {
        struct {
            unsigned int flags;
            unsigned int length;
            char* heap;
        } lng;
        struct {
            unsigned char size;
            char local[11];
        } sht;
    };
};
struct Term {
    char c;
    char pad[7];
};
// Set movie volume, reuse a cached name, and select an available alternate or standard THP path.
void* MovieController_LoadNamedMovie(const void* self, const char* name, int arg2, int arg3) {
    // name is expected to be non-null (cache comparisons and suffix construction
    // require it). arg2/arg3 pass unchanged to Movie_OpenResource; meaning unknown.
    // Mixed return contract: cache hit returns (void*)1; otherwise forward the
    // native open result. Do not interpret every nonzero result as a resource ptr.
    // The requested-name pointer is cached BEFORE open, even when opening fails;
    // retrying the same text takes the cache path. This is not a success cache.
    // Phase 1: choose global volume, then handle request-name reuse/replacement.
    TempString text = {0};
    StringUnwindCleanup_u80293A54 stringCleanup0(&text);
    Term t1, t2, t3, t4, t5, t6;
    if (name) {
        if (!strcmp(name, moviePlaybackStrings))
            MovieAudio_SetMasterVolume(kAdvertiseMovieVolume);
        else
            MovieAudio_SetMasterVolume(kStandardMovieVolume);
    }
    void* old = (void*)((const MovieControllerView*)self)->requestedName;
    if (old && !strcmp((const char*)old, name)) {
        if (text.lng.flags >> 31) {
            stringCleanup0.release();
            appFreeUni__FPv(text.lng.heap);
        }
        return (void*)1;
    }
    if (old)
        MovieAudio_ReleaseSampleBuffersAndManager(((MovieControllerView*)self)->playback);
    ((MovieControllerView*)self)->requestedName = name;
    // Phase 2: consult save-option bits 28..29. Variant 2 tries _pl.thp;
    // unchanged hash-path lookup means fallback to _st.thp. Other variants
    // directly use _st.thp. The locally assembled path owns its heap bytes.
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = s_saveData;
    }
    void* mode = SaveData_GetCurrentRecord(GetSaveDataSlot());
    bool special =
        ((((MovieSaveOptions*)mode)->flags >> kMovieVariantShift) & kMovieVariantMask) == kAlternateMovieVariant;
    if (special) {
        unsigned int len1 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
        t1.c = t1.pad[3];
        String_ReplaceRange(&text, 0, len1, name, name + strlen(name), &t1.c);
        const char* suffix2 = ((const MoviePlaybackStringPool*)moviePlaybackStrings)->alternateSuffix;
        unsigned int len2 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
        t2.c = t2.pad[3];
        String_ReplaceRange(&text, len2, 0, suffix2, suffix2 + strlen(suffix2), &t2.c);
        bool is_short = (text.lng.flags >> 31) == 0;
        const char* converted = is_short ? text.sht.local : text.lng.heap;
        if (!strcmp((const char*)((CHashPath*)g_resourceManager)
                        ->FindPathForwarder(is_short ? text.sht.local : text.lng.heap),
                    converted)) {
            unsigned int len3 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
            t3.c = t3.pad[3];
            String_ReplaceRange(&text, 0, len3, name, name + strlen(name), &t3.c);
            const char* suffix4 = ((const MoviePlaybackStringPool*)moviePlaybackStrings)->standardSuffix;
            unsigned int len4 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
            t4.c = t4.pad[3];
            String_ReplaceRange(&text, len4, 0, suffix4, suffix4 + strlen(suffix4), &t4.c);
        }
    } else {
        unsigned int len5 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
        t5.c = t5.pad[3];
        String_ReplaceRange(&text, 0, len5, name, name + strlen(name), &t5.c);
        const char* suffix6 = ((const MoviePlaybackStringPool*)moviePlaybackStrings)->standardSuffix;
        unsigned int len6 = (text.lng.flags >> 31) == 0 ? (text.sht.size & 0x7f) : text.lng.length;
        t6.c = t6.pad[3];
        String_ReplaceRange(&text, len6, 0, suffix6, suffix6 + strlen(suffix6), &t6.c);
    }
    // Phase 3: open using temporary path bytes, then release them. The native
    // opener must consume/copy the path before returning; only name stays cached.
    const char* path = (text.lng.flags >> 31) == 0 ? text.sht.local : text.lng.heap;
    void* result = Movie_OpenResource(((MovieControllerView*)self)->playback, path, arg2, arg3);
    if (text.lng.flags >> 31) {
        stringCleanup0.release();
        appFreeUni__FPv(text.lng.heap);
    }
    return result;
}
// Report the banner initialization-attempt latch; it does not imply parse success.
int SaveBanner_HasFinishedInitializationAttempt(void* self) {
    return ((unsigned int*)self)[4] == kBannerInitializationAttemptFinished;
}

// Original 0x80294C7C: stop at the first resource whose buffer is not ready.
inline int AllResourceBuffersLoaded(void* self, unsigned int*& p) {
    while (!same_pos((unsigned int*)((void**)self)[1] + ((unsigned int*)self)[2], p)) {
        {
            void* obj = (void*)*p;
            if (obj) {
                if (!CSharedFilePtr_GetLoadedBuffer(obj))
                    return 0;
            }
        }
        ++p;
    }
    return 1;
}

// Once every file buffer is ready, attempt parsing and latch completion once.
int SaveBanner_PollInitializationAttempt(void* self) {
    // Return is always 1, including while waiting. Query SaveBanner_HasFinishedInitializationAttempt for the
    // completion latch. A failed parse skips image binding but STILL sets the
    // latch; later polls never retry. Null entries count as already ready.
    if (((int*)self)[4] == kBannerPending) {
        unsigned int* p = (unsigned int*)((void**)self)[1];
        int valid = AllResourceBuffersLoaded(self, p);
        if (valid) {
            if (SaveBanner_ParseLocalizedTextSections(self))
                SaveBanner_BindLoadedImages(self);
            ((int*)self)[4] = kBannerInitializationAttemptFinished;
        }
    }
    return 1;
}

struct BannerStorage {
    void** vtable;
    int resourceHandles, resourceCount, resourceCapacity, initializationAttemptFinished, localizedTextCount,
        localizedTextRoot;
    void* end;
    unsigned char loadRequested;
};
// Allocate an empty banner, invoke its initialization hook, and destroy it if initialization fails.
void* CBanner::Create() {
    BannerStorage* obj = (BannerStorage*)__nw__FUl(0x24);
    if (obj != 0) {
        obj->vtable = (void**)&__vt__7CBanner;
        obj->resourceHandles = 0;
        obj->resourceCount = 0;
        obj->resourceCapacity = 0;
        obj->initializationAttemptFinished = 0;
        obj->localizedTextCount = 0;
        obj->localizedTextRoot = 0;
        obj->end = &obj->localizedTextRoot;
        obj->loadRequested = 0;
    }
    if (obj == 0)
        return 0;
    if (!((BannerLifecycleDispatch*)obj)->initialize()) {
        if (obj != 0)
            ((BannerLifecycleDispatch*)obj)->destroy(1);
        return 0;
    }
    return obj;
}

// A line of localized text: a string, heap-allocated when bit 31 of flags is set.
struct BannerTextLine {
    unsigned int flags;
    unsigned int length;
    void* heapText;
};
// The value of a localized-text entry: an owned vector of lines.
struct BannerTextValue {
    BannerTextLine* lines;
    int count;
    int capacity;
};
struct BannerNode {
    BannerNode* left;
    BannerNode* right;
    unsigned int parent;
    unsigned int flags;
    unsigned int length;
    void* string;
    BannerTextValue* value;
};
// Banner ownership: resource vector owns shared-file handles. Localized-text
// tree nodes own key strings (heap iff bit 31) and value objects, each an owned
// vector of 12-byte strings. Tree parent word packs its color in bit 0.
// The tree's end marker is the localizedTextRoot field itself; it has no key or value.

// Free a node's heap key only; mapped values were released in the first walk.
inline void DestroyBannerNodeText(BannerNode* node) {
    if ((char*)node + 12) {
        if ((char*)node + 12) {
            if (node->flags >> 31)
                appFreeUni__FPv(node->string);
        }
    }
}
// The four postorder levels remain expanded here: converting all levels to
// inline calls changes register lifetimes in the destructor. Each level frees
// left/right descendants, then this node's heap key and node allocation.
// BANNER_SUBTREE delegates deeper children to the out-of-line tree eraser.
// The macros are expanded only inside the destructor and pass the tree that
// starts at localizedTextCount to the eraser.
#define BANNER_SUBTREE(n)                                                        \
    if ((n)->left)                                                               \
        SaveBanner_DestroyLocalizedTextSubtree(&localizedTextCount, (n)->left);  \
    if ((n)->right)                                                              \
        SaveBanner_DestroyLocalizedTextSubtree(&localizedTextCount, (n)->right); \
    DestroyBannerNodeText(n);                                                    \
    appFreeUni__FPv(n)
#define BANNER_LEVEL2(n)                \
    {                                   \
        BannerNode* child = (n)->left;  \
        if (child) {                    \
            BANNER_SUBTREE(child);      \
        }                               \
    }                                   \
    {                                   \
        BannerNode* child = (n)->right; \
        if (child) {                    \
            BANNER_SUBTREE(child);      \
        }                               \
    }                                   \
    DestroyBannerNodeText(n);           \
    appFreeUni__FPv(n)
#define BANNER_LEVEL3(n)                 \
    {                                    \
        BannerNode* branch = (n)->left;  \
        if (branch) {                    \
            BANNER_LEVEL2(branch);       \
        }                                \
    }                                    \
    {                                    \
        BannerNode* branch = (n)->right; \
        if (branch) {                    \
            BANNER_LEVEL2(branch);       \
        }                                \
    }                                    \
    DestroyBannerNodeText(n);            \
    appFreeUni__FPv(n)
#define BANNER_LEVEL4(n)                \
    {                                   \
        BannerNode* child = (n)->left;  \
        if (child) {                    \
            BANNER_LEVEL3(child);       \
        }                               \
    }                                   \
    {                                   \
        BannerNode* child = (n)->right; \
        if (child) {                    \
            BANNER_LEVEL3(child);       \
        }                               \
    }                                   \
    DestroyBannerNodeText(n);           \
    appFreeUni__FPv(n)

#pragma push
#pragma use_lmw_stmw on
// Destroy localized string vectors and tree nodes, then release the file handles.
// One difference from the original remains: it holds the first walk's end marker
// and the zero it stores in each other's registers.
CBanner::~CBanner() {
    BannerNode* node;
    BannerNode* end;
    BannerTextLine* line;
    int n;
    BannerTextValue* value;
    vtbl = &__vt__7CBanner;
    node = localizedTextFirst;
    end = (BannerNode*)&localizedTextRoot;
    // Phase 1: in-order walk destroys mapped string vectors and clears each
    // value pointer. Strings -> vector buffer -> value object, in that order.
    // Advance through tagged parent links without deleting key nodes yet.
    while (end != node) {
        value = node->value;
        if (value) {
            if (value) {
                if (value) {
                    if (value) {
                        if (value) {
                            BannerTextLine* lines = value->lines;
                            if (lines) {
                                n = value->count;
                                line = lines + n;
                                value->count -= n;
                                while (n != 0) {
                                    --line;
                                    if (line && (line->flags >> 31))
                                        appFreeUni__FPv(line->heapText);
                                    --n;
                                }
                                appFreeUni__FPv(value->lines);
                            }
                        }
                    }
                }
                __dl__FPv(value);
            }
            node->value = 0;
        }
        BannerNode* child = node->right;
        if (child) {
            BannerNode* left;
            while ((left = child->left) != 0)
                child = left;
            node = child;
        } else {
            BannerNode* parent;
            while (node != (parent = (BannerNode*)(node->parent & ~1u))->left)
                node = parent;
            node = parent;
        }
    }
    // Phase 2: postorder cleanup frees key text/nodes, then restores empty
    // count/root/begin. Expanded levels are equivalent ownership stages.
    BannerNode* root = localizedTextRoot;
    if (root) {
        BANNER_LEVEL4(root);
        localizedTextCount = 0;
        localizedTextRoot = 0;
        localizedTextFirst = (BannerNode*)&localizedTextRoot;
    }
    // Phase 3: release every owned shared-file handle. Clearing logical count
    // does not release vector capacity; the following base cleanup does that.
    void** item = fileHandles;
    void** itemEnd;
    while (itemEnd = fileHandles + fileHandleCount, !(itemEnd == item)) {
        if (*item) {
            CSharedFileHandle::Destroy(*item, 1);
            *item = 0;
        }
        ++item;
    }
    unsigned int count = fileHandleCount;
    fileHandleCount -= count;
    // Phase 4: run the now-empty tree/vector base cleanups.
    // Repeated address tests preserve the original inlined destructor shape.
    if (&localizedTextCount) {
        if (&localizedTextCount) {
            if (&localizedTextCount) {
                if (&localizedTextCount) {
                    if (&localizedTextCount) {
                        if (&localizedTextCount) {
                            BannerNode* otherRoot = localizedTextRoot;
                            if (otherRoot) {
                                BANNER_LEVEL4(otherRoot);
                            }
                        }
                    }
                }
            }
        }
    }
    if (&fileHandles) {
        if (&fileHandles) {
            if (&fileHandles) {
                if (&fileHandles) {
                    if (&fileHandles) {
                        if (fileHandles) {
                            // Read as a pointer word, as the original does, so the count is reloaded.
                            unsigned int remaining = (unsigned int)*(void**)&fileHandleCount;
                            fileHandleCount -= remaining;
                            appFreeUni__FPv(fileHandles);
                        }
                    }
                }
            }
        }
    }
}
#pragma pop
#undef BANNER_LEVEL4
#undef BANNER_LEVEL3
#undef BANNER_LEVEL2
#undef BANNER_SUBTREE

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046245C", _ctors_u80293A54_0, MovieController_InitStaticConstants);
STATIC_INIT_ENTRY(".CRT$XCU8046245C", _ctors_u80293A54_1, CBanner_InitStaticConstants);
#pragma pop
