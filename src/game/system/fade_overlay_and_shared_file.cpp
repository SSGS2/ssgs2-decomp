#include <decomp/static_init.h>
// Frame rate / captured-frame fade overlay, plus shared-file handle factories.
// FadeState_StartFadeIn/Out choose alpha direction; FadeState_Update in
// vcg_motion_player_and_fade.cpp changes alpha once per update and services captureRequested
// as a pending framebuffer capture. FadeState_Draw consumes current viewport size.
// Shared layouts: FadeState, CSharedFile, CSharedFileSync. Pool: u8023DCA0_data.cpp.
// Modder lookup:
// - Fade duration: nonnegative UPDATE CALL COUNT, not seconds or VI retraces.
//   StartFadeIn/Out set alphaStep=255/duration; at 30 updates/s, 30 calls is
//   about one second. Changing presentation rate alone does not rescale this.
// - Overlay size: FadeState_GetInstance captures viewport width/height only at
//   first initialization; Draw sizes a top-left-origin quad from the CURRENT
//   viewport dimensions. Runtime resizing also needs a texture recreation policy.
// - Texture format: kOverlayNativeRgba8TextureFormat=0x80000006 encodes the engine's
//   native-format flag (bit 31) plus GX RGBA8 format 6. The framebuffer-copy path
//   (gx_renderer_state_and_efb_copy.cpp) masks off bit 31 before GXSetTexCopyDst. Keep format, tiled
//   allocation/mip sizing and lock/copy consumers in gtl_graphics_texture_buffer_resources.cpp consistent.
// - UV/vertex format: s_fadeOverlayQuadVertices supplies four 24-byte XYZ/RGBA/UV
//   vertices; Draw changes only x/y/alpha. Alpha is byte 3 of big-endian RGBA.
// - File buffers: byteCount is the historical ABI name of a buffer-backed file
//   HANDLE factory, not a size query.
// Preserve alpha direction, endpoint clamping in Update, reference ownership,
// capture-before-alpha sequencing and renderer-state restore order. FadeIn means
// increasing overlay alpha; it must not be described as revealing the scene.
#include "classes/FadeState.h"
// Borrow the fade-state layout; direct expansion preserves FadeState_Draw's
// original load scheduling (the inline-function form changes its generated code).
#define AsFadeState(p) ((FadeState*)(p))
// Translation unit 0x8023DCA0-0x8023E86C (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/singletons.h"
#include "classes/CSharedFile.h"
#include "classes/CSetRenderState.h"
#include "classes/CSharedFileSync.h"
#include <decomp/cw_bitfields.h>

// Borrow synchronous-file fields; the file remains responsible for its buffer.
static inline CSharedFileSync* AsCSharedFileSync(void* p) { return (CSharedFileSync*)p; }

// Borrow a renderer-state guard without restoring or transferring it.
static inline CSetRenderState* AsCSetRenderState(void* p) { return (CSetRenderState*)p; }

// Borrow the shared-file base; retaining/releasing is explicit in its callers.
static inline CSharedFile* AsCSharedFile(void* p) { return (CSharedFile*)p; }
// Explicit renderer API records; the stack copy remains word-typed for identical bulk copies.
struct RenderStateSnapshot { void* vtable; int stateId; int savedValue; };
struct OverlayViewport { unsigned int x, y, width, height; float nearDepth, farDepth; };
struct OverlayVertex {
    float x, y, z;
    union { unsigned int color; unsigned char colorBytes[4]; };
    float u, v;
};
struct VertexBlock { int words[24]; };
struct FadeDrawFrame {
    RenderStateSnapshot depthTest, depthWrite;
    OverlayViewport viewport;
    union { VertexBlock vertexCopy; OverlayVertex vertices[4]; } quad;
};
struct OverlayLockedRect { int pitch; void* data; };
struct OverlayLevelDescription {
    unsigned int format;
    char pad_004[0x18];
    unsigned int width, height;
};
struct FadeTextureFrame {
    OverlayLockedRect lock;
    OverlayViewport viewport;
    OverlayLevelDescription level;
    char tail[0xC];
};
enum FadePhase { kFadeTransparent = 0, kFadeIncreasingAlpha = 1, kFadeOpaque = 2, kFadeDecreasingAlpha = 3 };
enum {
    // Engine_ApplyStateBeforeDraw (gx_renderer_state_and_efb_copy.cpp): state 5 controls GX depth writes,
    // state 6 controls GX depth testing. Draw disables both, then restores 6,5.
    kOverlayDepthWriteState = 5, kOverlayDepthTestState = 6, kOverlayVertexFormat = 0x11F005,
    kOverlayNativeRgba8TextureFormat = 0x80000006u, kOverlayPrimitive = 0x98,
    kOverlayVertexCount = 4, kOverlayAlphaByte = 3,
    kInlinePathLengthMask = 0x7F, kHeapPathShift = 31
};
typedef char FadeDrawFrame_size[(sizeof(FadeDrawFrame) == 0x90) ? 1 : -1];
typedef char OverlayVertex_size[(sizeof(OverlayVertex) == 0x18) ? 1 : -1];
typedef char FadeTextureFrame_size[(sizeof(FadeTextureFrame) == 0x50) ? 1 : -1];

extern "C" {

extern float s_fadeFileUnitTwoPiCache0;
extern float s_fadeFileUnitTwoPiCache1;
extern float s_fadeFileUnitTwoPiCache2;
extern float s_fadeFileUnitTwoPiCache3;
extern const float kFloat_6_2831855;
extern char __vt__15CSetRenderState[];
extern char __vt__11CSharedFile[];
int __cntlzw(unsigned int);
int __rlwnm(int, int, int, int);

void FadeState_Draw(void*);
void ResourceState_Dtor(void*, int);
void __dla__FPv(void*);
void __dl__FPv(void*);
int Engine_SetCachedRenderState(int, int);
void* CSharedFileRegistry_GetInstance(void*);
void* SharedFileRegistry_ReleaseFile(void*, void*);

extern const float kFloat_255_Fade;
extern const float kFloat_0_805E0218;
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[];
extern char s_fadeOverlayTextureDtorRegistration[];
extern int s_eventParameterStorageEntries[];
extern const int s_fadeOverlayQuadVertices[];
extern const double kSignedIntToDoubleBias_FadeFileUnit;
extern unsigned char s_fadeOverlayTextureReady;
extern unsigned char s_clearFramebufferTextureOnCreate;
extern unsigned char s_releaseFadeResourceOnCompletion;
void CubeResourceStorage_dtor(void*, int);
void* __nw__FUl(unsigned int);
void* SharedFileCache_GetOrCreate(void*, void*, int);
void* SharedFileCache_GetOrCreateFromBuffer(void*, void*, void*, int);

struct HalfWord { unsigned short hi; unsigned short lo; };
struct VTableObj {
    virtual int m0();
    virtual int m1();
    virtual int m2();
    virtual int isLoaded();
    virtual int releaseBuffer();
};

void* __nwa__FUl(int);
void appFreeUni__FPv(void*);
void* String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, void*);
void Engine_GetViewport(void*);
int Texture_Create2D(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, void*, unsigned int);
void Engine_GetRenderState(int, void*);
int Engine_SetCachedRenderState(int, int);
int Texture_GetLevelDescription(void*, int, void*);
int Texture_LockLevel(void*, int, void*, int, int);
void* Texture_CalculateMipStorageSize(void*, void*, int, int);
void* Memory_Clear(void*, void*);
int Texture_UnlockLevel(void*, int);
void Engine_SetFlexibleVertexFormat(int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);
void UIRender_SetDefaultState(void);
void Engine_SetTextureStageResource(int, void*);
// Integer entry counts, not a floating-point constant. The defining symbol retains its ABI name.
extern const unsigned int kEventParameterCategoryCounts[2];
void* __register_global_object(void*, void*, void*);
unsigned int strlen(const char*);

// Compatibility hook: this build does not store event-flag writes.
void EventFlag_SetStub(unsigned char, unsigned char, unsigned char) {
}

// Read the current overlay-alpha phase 0..3; preserve the distinction between idle and opaque endpoints.
int FadeState_GetState(void* p) {
    return AsFadeState(p)->state;
}

// Compatibility hook: this build exposes no event-flag storage.
void* EventFlag_GetStoragePointerStub(unsigned char, unsigned char) {
    return (void*)0;
}

// Store the alternate-pass draw gate; preserve byte storage and return 1 regardless of its value.
int FadeState_SetAlternateDrawEnabled(void* p, unsigned char v) {
    AsFadeState(p)->alternateDrawEnabled = v;
    return 1;
}

// Initialize this fade/file helper's independent cached full-turn angle.
void FadeFileUnit_InitTwoPiCache0(void) {
    s_fadeFileUnitTwoPiCache0 = kFloat_6_2831855;
}

// Initialize this fade/file helper's independent cached full-turn angle.
void FadeFileUnit_InitTwoPiCache1(void) {
    s_fadeFileUnitTwoPiCache1 = kFloat_6_2831855;
}

// Initialize this fade/file helper's independent cached full-turn angle.
void FadeFileUnit_InitTwoPiCache2(void) {
    s_fadeFileUnitTwoPiCache2 = kFloat_6_2831855;
}

// Initialize this fade/file helper's independent cached full-turn angle.
void FadeFileUnit_InitTwoPiCache3(void) {
    s_fadeFileUnitTwoPiCache3 = kFloat_6_2831855;
}

// Latch object-local texture release at transparent completion; preserve deferred ownership release.
int FadeState_RequestReleaseOnFadeCompletion(void* p) {
    AsFadeState(p)->releaseResourceOnCompletion = 1;
    return 1;
}

// Retain a required shared-file object; preserve its separate registry/handle reference counts.
void CSharedFile_AddRef(void* p) {
    AsCSharedFile(p)->referenceCount = AsCSharedFile(p)->referenceCount + 1;
}

// Drop one positive shared-file reference and return 1 only at zero; caller must perform destruction separately.
int CSharedFile_ReleaseRefIsZero(void* p) {
    int v = AsCSharedFile(p)->referenceCount;
    v = v - 1;
    AsCSharedFile(p)->referenceCount = v;
    return __rlwnm(1, __cntlzw((unsigned int)v), 31, 31);
}

// Borrow path text until path replacement/file destruction; preserve inline/heap selection by bit 31.
void* CSharedFile_GetPath(void* p) {
    unsigned int v = AsCSharedFile(p)->pathFlags;
    if ((v >> kHeapPathShift) == 0) {
        return (char*)&AsCSharedFile(p)->pathFlags + 1;
    }
    return AsCSharedFile(p)->pathData;
}

// Latch a framebuffer-capture request in endpoint phases; preserve refusal to capture during either alpha ramp.
// Returns 1 even when no request is accepted; Update consumes/clears the pending
// byte after attempting capture. captureRequested latches this operation;
// it does not request an immediate scene-state transition.
int FadeState_RequestFramebufferCapture(void* p) {
    int v = AsFadeState(p)->state;
    // Original 0x8023DEF8: only idle fade states accept this request.
    if (v != kFadeIncreasingAlpha && v != kFadeDecreasingAlpha) AsFadeState(p)->captureRequested = 1;
    return 1;
}

// Initialize the shared-file base and its empty path.
void CSharedFile_Ctor(void* p) {
    AsCSharedFile(p)->unk_4 = 0;
    AsCSharedFile(p)->vtable = __vt__11CSharedFile;
    AsCSharedFile(p)->referenceCount = 0;
    (*(int*)&AsCSharedFile(p)->pathFlags) = 0;
    AsCSharedFile(p)->pathLength = 0;
    (*(int*)&AsCSharedFile(p)->pathData) = 0;
}

// Draw in the alternate phase while enabled, otherwise return to primary drawing.
int FadeState_DrawAlternatePhase(void* p) {
    unsigned char b = AsFadeState(p)->alternateDrawPhase;
    if (b != 0) {
        unsigned char b2 = AsFadeState(p)->alternateDrawEnabled;
        if (b2 == 0) AsFadeState(p)->alternateDrawPhase = 0;
        else FadeState_Draw(p);
    }
    return 1;
}

// Delete owned array storage and clear its pointer; preserve success=1 for an already-empty buffer.
int CSharedFile_FreeBuffer(void* p) {
    void* tmp = AsCSharedFileSync(p)->buffer;
    if (tmp != 0) {
        __dla__FPv(tmp);
        AsCSharedFileSync(p)->buffer = 0;
    }
    return 1;
}

// Borrow loaded bytes after readiness succeeds, or return null; preserve virtual readiness before accessing buffer.
void* CSharedFile_GetLoadedBuffer(void* p) {
    if (static_cast<VTableObj*>(p)->isLoaded()) {
        return AsCSharedFileSync(p)->buffer;
    }
    return 0;
}

// Draw in the primary phase, then hand off when alternate drawing is enabled.
int FadeState_DrawPrimaryPhase(void* p) {
    if (AsFadeState(p)->alternateDrawPhase == 0) {
        FadeState_Draw(p);
        if (AsFadeState(p)->alternateDrawEnabled != 0) {
            AsFadeState(p)->alternateDrawPhase = 1;
        }
    }
    return 1;
}

// Restore the saved renderer state and optionally delete the state guard.
void* CSetRenderState_dtor(void* p, int flag) {
    if (p != 0) {
        ((RenderStateSnapshot*)p)->vtable = (void*)__vt__15CSetRenderState;
        Engine_SetCachedRenderState(((RenderStateSnapshot*)p)->stateId, ((RenderStateSnapshot*)p)->savedValue);
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

// Increase overlay alpha over duration update calls; preserve the current alpha when starting a ramp.
// p is required; duration>=0. Zero snaps opaque. An already-increasing phase
// ignores the request (even zero); other phases keep their existing alpha.
// Returns 1 unconditionally. No range validation: negative durations reverse
// the step and may never reach the endpoint; they are outside the intended domain.
int FadeState_StartFadeIn(void* p, int duration) {
    if (AsFadeState(p)->state != kFadeIncreasingAlpha) {
        if (duration == 0) {
            AsFadeState(p)->alpha = kFloat_255_Fade;
            AsFadeState(p)->alphaStep = kFloat_0_805E0218;
            AsFadeState(p)->state = kFadeOpaque;
        } else {
            float f = (float)duration;
            AsFadeState(p)->state = kFadeIncreasingAlpha;
            AsFadeState(p)->alphaStep = kFloat_255_Fade / f;
        }
    }
    return 1;
}

// Release the handle's cached file before optional deletion.
void* CSharedFileHandle_Destroy(void* p, int flag) {
    if (p != 0) {
        {
            void* tmp = *(void**)p;
            if (tmp != 0) {
                void* r = CSharedFileRegistry_GetInstance(p);
                SharedFileRegistry_ReleaseFile(r, tmp);
            }
        }
        if (flag > 0) __dl__FPv(p);
    }
    return p;
}

// Replace file storage with owned array bytes; preserve virtual release before zero-size/failure handling.
// encodedByteCount is a 32-bit integer byte count in a pointer-shaped ABI word;
// it is never dereferenced. Return 0 on failed release, zero size or null allocation;
// failed release leaves storage to its implementation, other failures occur after
// release. Success returns 1; CSharedFile_FreeBuffer uses __dla__FPv to release it.
int CSharedFile_AllocateBuffer(void* self, void* encodedByteCount) {
    if (!static_cast<VTableObj*>(self)->releaseBuffer()) {
        return 0;
    }
    if (encodedByteCount == 0) {
        return 0;
    }
    void* p = __nwa__FUl((int)encodedByteCount);
    AsCSharedFileSync(self)->buffer = p;
    return p != 0 ? 1 : 0;
}

// Release virtual file storage and the heap path before optional deletion.
void* CSharedFile_dtor(void* self, int flag) {
    if (self != 0) {
        *(void**)self = (void*)__vt__11CSharedFile;
        static_cast<VTableObj*>(self)->releaseBuffer();
        if ((int)self + 0xc != 0) {
            if (AsCSharedFile(self)->pathFlags >> kHeapPathShift) {
                appFreeUni__FPv(AsCSharedFile(self)->pathData);
            }
        }
        if (flag > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

struct StringOwnerE564 {
    char pad[12];
    union { unsigned int flags; unsigned char bytes[4]; } value;
    int count;
};

// Copy a required null-terminated path into the file string; preserve string allocator-byte and replacement order.
// str is borrowed for the call; the string owns its copy. The volatile byte below
// is original allocator scratch, not a path character; do not initialize/remove
// the read as a readability fix, since that changes the existing instruction stream.
int CSharedFile_SetPath(StringOwnerE564* self, const char* str) {
    int count;
    unsigned int flags = self->value.flags;
    if ((flags >> kHeapPathShift) == 0) {
        count = self->value.bytes[0] & kInlinePathLengthMask;
    } else {
        count = self->count;
    }
    volatile int tmp;
    volatile unsigned char alloc = *(volatile unsigned char*)&tmp;
    int len = (int)strlen(str);
    String_ReplaceRange(&self->value, 0, (unsigned)count, str, str + len, (void*)&alloc);
    return 1;
}

// Create a handle to a cached path resource, releasing the handle if lookup fails.
// path is a borrowed CString object (NOT a char buffer), nullable for failure.
// cacheOptions is a Boolean: 0 creates CSharedFileAsync, any nonzero value creates
// CSharedFileSync on a cache miss. An existing path retains its previous mode.
// See SharedFileCache_GetOrCreate (shared_file_registry.cpp); no flags are decoded here.
// Release the returned owner with CSharedFileHandle_Destroy(handle,1).
void* SharedFile_Create(void* path, int cacheOptions) {
    void* obj = __nw__FUl(4);
    if (obj != 0) {
        *(int*)obj = 0;
    }
    if (obj == 0) return 0;
    int ok;
    if (path == 0) {
        ok = 0;
    } else {
        void* reg = ((void*(*)())CSharedFileRegistry_GetInstance)();
        void* res = SharedFileCache_GetOrCreate(reg, path, cacheOptions);
        *(void**)obj = res;
        ok = (res != 0);
    }
    if (!ok) {
        if (obj != 0) {
            void* tmp = *(void**)obj;
            if (tmp != 0) {
                void* reg = ((void*(*)())CSharedFileRegistry_GetInstance)();
                SharedFileRegistry_ReleaseFile(reg, tmp);
            }
            __dl__FPv(obj);
        }
        return 0;
    }
    return obj;
}

// Create a retained cached-file handle from borrowed bytes; preserve path-first cache lookup.
// path is a nullable CString object; bufferByteCount is bytes, not options. On a miss,
// the cache copies buffer[0..bufferByteCount); a hit ignores buffer/count. Null
// path/allocation/cache failure returns null after freeing the wrapper. Caller
// destroys a successful wrapper with CSharedFileHandle_Destroy(handle,1).
// Accepted miss input: 1..INT_MAX bytes of readable, stable storage at buffer,
// with a size the allocator can satisfy; buffer must not be null. Zero/null
// fails on a miss. Negative counts are NOT validated: their unsigned conversion
// requests a huge allocation/copy, so they are outside the accepted domain.
// A cache hit bypasses these checks; callers must not rely on that for validity.
void* byteCount(void* path, void* buffer, int bufferByteCount) {
    void* obj = __nw__FUl(4);
    if (obj != 0) {
        *(int*)obj = 0;
    }
    if (obj == 0) return 0;
    int ok;
    if (path == 0) {
        ok = 0;
    } else {
        void* reg = ((void*(*)())CSharedFileRegistry_GetInstance)();
        void* res = SharedFileCache_GetOrCreateFromBuffer(reg, path, buffer, bufferByteCount);
        *(void**)obj = res;
        ok = (res != 0);
    }
    if (!ok) {
        if (obj != 0) {
            void* tmp = *(void**)obj;
            if (tmp != 0) {
                void* reg = ((void*(*)())CSharedFileRegistry_GetInstance)();
                SharedFileRegistry_ReleaseFile(reg, tmp);
            }
            __dl__FPv(obj);
        }
        return 0;
    }
    return obj;
}

// Decrease overlay alpha over duration update calls; preserve conditional texture release at zero.
// self is required, duration>=0; returns 1. An already-decreasing phase ignores
// the request. Zero snaps transparent, clears the step and releases the resource
// only if a global/object release flag requests it; a nonzero ramp starts at the
// CURRENT alpha and Update performs endpoint release. No seconds conversion occurs.
int FadeState_StartFadeOut(void* self, int duration) {
    if (AsFadeState(self)->state != kFadeDecreasingAlpha) {
        AsFadeState(self)->state = kFadeDecreasingAlpha;
        if (duration == 0) {
            float z = kFloat_0_805E0218;
            AsFadeState(self)->state = kFadeTransparent;
            AsFadeState(self)->alpha = z;
            AsFadeState(self)->alphaStep = z;
            if (s_releaseFadeResourceOnCompletion != 0 || AsFadeState(self)->releaseResourceOnCompletion != 0) {
                void* ptr = *(void**)self;
                if (ptr != 0) {
                    unsigned int v = *(unsigned int*)ptr;
                    unsigned int hi = v >> 16;
                    hi = hi - 1;
                    struct RefBits { CW_BITS2( unsigned int hi:16, unsigned int lo:16) };
                    ((RefBits*)&v)->hi = hi;
                    *(unsigned int*)ptr = v;
                    if ((v >> 16) == 0) {
                        CubeResourceStorage_dtor(ptr, 1);
                    }
                    *(void**)self = 0;
                }
                s_fadeOverlayTextureReady = 0;
                AsFadeState(self)->releaseResourceOnCompletion = 0;
            }
        } else {
            float f = (float)duration;
            AsFadeState(self)->state = kFadeDecreasingAlpha;
            AsFadeState(self)->alphaStep = kFloat_255_Fade / f;
        }
    }
    return 1;
}

// Read a script parameter VALUE word after summing preceding category counts.
// Producer/consumer map: SC_SET_VAR_PARA in event_conte_script_interpreter.cpp passes command bytes [2]
// (category) and [3] (entry), copies this returned word into its operandByte
// variable slot, then SC_VAR_JMP passes the slot bits to Int32_CompareByMode
// (story_event_script_shared_state.cpp). That comparator compares integer values; it never dereferences
// them. The ABI name and the caller's void* temporary do not make this a pointer
// to mutable parameter storage. Binding snapshots the word, with no ownership.
// Storage producer visible in source: zero-initialized 256-word backing array
// src/ or include/. No gameplay names for categories or individual entries are
// established. A future parameter mod must supply/identify its writer as well
// as updating the script binding; changing an entry affects subsequent bindings,
// not already bound slots. Zero is a legitimate value, not a missing-entry code.
// Prefix counts {2,0} mean category 0 starts at word 0, categories 1 and 2 at
// word 2. They do not enforce per-category entry bounds. categoryIndex<=2 and
// final index<256 are required to keep both reads in bounds. Prefix addition
// wraps at 16 bits; no validation or status result is supplied here.
int EventParameter_GetValueWord(unsigned char categoryIndex, unsigned short entryIndex) {
    unsigned char category = 0;
    for (; category < categoryIndex; ++category) {
        entryIndex += kEventParameterCategoryCounts[category];
    }
    return s_eventParameterStorageEntries[entryIndex & 0xffff];
}

// Lazily create the capture texture at the current viewport size; preserve publication even on failure.
// Returns borrowed global storage, never null after publication; resource/ready
// may remain null/false when create/describe/lock/unlock fails. The initialized
// guard still latches, so another GetInstance call does not retry this path.
void* FadeState_GetInstance(void) {
    // Original 0x8023DCA0 unit: initialize the singleton and then publish it.
    if (Sing_805DC608() == 0) {
        if (s_fadeOverlayInitialized == 0) {
            float zeroAlpha = kFloat_0_805E0218;
            char* overlay = s_fadeOverlay;
            (*(int*)&AsFadeState(s_fadeOverlay)->resource) = 0;
            AsFadeState(overlay)->alternateDrawPhase = 0;
            AsFadeState(overlay)->alternateDrawEnabled = 0;
            AsFadeState(overlay)->releaseResourceOnCompletion = 0;
            AsFadeState(overlay)->alpha = zeroAlpha;
            AsFadeState(overlay)->alphaStep = zeroAlpha;
            AsFadeState(overlay)->state = kFadeTransparent;
            AsFadeState(overlay)->captureRequested = 0;
            FadeTextureFrame frame;
            Engine_GetViewport(&frame.viewport);
            unsigned int viewportWidth = frame.viewport.width;
            unsigned int viewportHeight = frame.viewport.height;
            int ok = Texture_Create2D(viewportWidth, viewportHeight, 1, 0, kOverlayNativeRgba8TextureFormat, 0, (void*)overlay, 0);
            do {
                if (ok == 0) break;
                if (s_clearFramebufferTextureOnCreate != 0) {
                    if (Texture_GetLevelDescription(*(void**)s_fadeOverlay, 0, &frame.level) == 0) break;
                    if (Texture_LockLevel(*(void**)s_fadeOverlay, 0, &frame.lock, 0, 0) == 0) break;
                    void* mipStorageBytes = Texture_CalculateMipStorageSize((void*)frame.level.width, (void*)frame.level.height, 1, (int)frame.level.format);
                    Memory_Clear(frame.lock.data, mipStorageBytes);
                    if (Texture_UnlockLevel(*(void**)s_fadeOverlay, 0) == 0) break;
                }
                s_fadeOverlayTextureReady = 1;
            } while (0);
            __register_global_object(s_fadeOverlay, (void*)ResourceState_Dtor, s_fadeOverlayTextureDtorRegistration);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = (void*)s_fadeOverlay;
    }
    return Sing_805DC608();
}

// Draw the captured frame across the current viewport; preserve state-6 then state-5 restoration.
// self is required and borrowed. XY are viewport-local pixels (+X right,+Y down),
// not world coordinates; viewport x/y are not added. No draw if resource is null
// or alpha is zero, but default-state setup and both state restores still occur.
void FadeState_Draw(void* self) {
    UIRender_SetDefaultState();
    char* base = __vt__15CSetRenderState;
    FadeDrawFrame frame;
    frame.depthWrite.vtable = base;
    frame.depthWrite.stateId = kOverlayDepthWriteState;
    frame.depthWrite.savedValue = -1;
    Engine_GetRenderState(kOverlayDepthWriteState, &frame.depthWrite.savedValue);
    int stateId = frame.depthWrite.stateId;
    Engine_SetCachedRenderState(stateId, 0);
    frame.depthTest.vtable = base;
    frame.depthTest.stateId = kOverlayDepthTestState;
    frame.depthTest.savedValue = -1;
    Engine_GetRenderState(kOverlayDepthTestState, &frame.depthTest.savedValue);
    stateId = frame.depthTest.stateId;
    Engine_SetCachedRenderState(stateId, 0);
    Engine_GetViewport(&frame.viewport);
    frame.quad.vertexCopy = *(VertexBlock*)s_fadeOverlayQuadVertices;
    float alpha = AsFadeState(self)->alpha;
    unsigned int width = frame.viewport.width;
    unsigned int height = frame.viewport.height;
    frame.quad.vertices[0].colorBytes[kOverlayAlphaByte] = (unsigned char)(int)alpha;
    frame.quad.vertices[1].x = (float)width;
    frame.quad.vertices[1].colorBytes[kOverlayAlphaByte] = (unsigned char)(int)AsFadeState(self)->alpha;
    frame.quad.vertices[2].y = (float)height;
    frame.quad.vertices[2].colorBytes[kOverlayAlphaByte] = (unsigned char)(int)AsFadeState(self)->alpha;
    frame.quad.vertices[3].x = (float)width;
    frame.quad.vertices[3].y = (float)height;
    frame.quad.vertices[3].colorBytes[kOverlayAlphaByte] = (unsigned char)(int)AsFadeState(self)->alpha;
    int textureId = (*(int*)&AsFadeState(self)->resource);
    if (textureId != 0 && AsFadeState(self)->alpha != kFloat_0_805E0218) {
        Engine_SetFlexibleVertexFormat(kOverlayVertexFormat);
        Engine_SetTextureStageResource(0, AsFadeState(self)->resource);
        void* primitiveCount = GX_GetPrimitiveCount(kOverlayVertexCount, kOverlayPrimitive);
        DrawPrimitiveUP(kOverlayPrimitive, primitiveCount, frame.quad.vertices, sizeof(OverlayVertex));
    }
    char* stateVtable = __vt__15CSetRenderState;
    int restoreDepthTest = frame.depthTest.stateId;
    int savedDepthTest = frame.depthTest.savedValue;
    frame.depthTest.vtable = stateVtable;
    Engine_SetCachedRenderState(restoreDepthTest, savedDepthTest);
    frame.depthWrite.vtable = stateVtable;
    int restoreDepthWrite = frame.depthWrite.stateId;
    int savedDepthWrite = frame.depthWrite.savedValue;
    Engine_SetCachedRenderState(restoreDepthWrite, savedDepthWrite);
    return;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046228C", _ctors_u8023DCA0_0, FadeFileUnit_InitTwoPiCache0);
STATIC_INIT_ENTRY(".CRT$XCU8046228C", _ctors_u8023DCA0_1, FadeFileUnit_InitTwoPiCache1);
STATIC_INIT_ENTRY(".CRT$XCU8046228C", _ctors_u8023DCA0_2, FadeFileUnit_InitTwoPiCache2);
STATIC_INIT_ENTRY(".CRT$XCU8046228C", _ctors_u8023DCA0_3, FadeFileUnit_InitTwoPiCache3);
#pragma pop
