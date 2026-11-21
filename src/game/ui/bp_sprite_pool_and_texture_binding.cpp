#include <decomp/static_init.h>
#include <decomp/ct_member.h>
// BP sprite pool and texture binding API (2D/menu graphics).
// Mod/API index:
// - Frame reuse: BpSpritePool_MarkAllInactive clears every pooled sprite
//   activation flag before the next acquire/activate/draw cycle.
// - Build a quad: Acquire -> BeginVertices -> ActivateAndClearVertexCount ->
//   AppendVertex four times -> DrawActiveSprites. Acquire returns a borrowed,
//   unclaimed entry; serialize the acquire/activate sequence with other users.
// - Texture path/readiness: CBpTex_BindTexturePath -> CBpTex_IsLoaded -> Bind;
//   request success does not mean load completion. ReleaseSources drops owners.
// - Texture dimensions/format: GetTextureDescriptor and schema below; shared
//   producer is shared_texture_resource.cpp -> gtl_Image_GetPlatform7Description (gtl_texture_vertex_formats.cpp).
// - Vertex count limit and render state table: s_bpSpriteVertexCountLimit and
//   kBpSpriteTextureStageStates in this unit's data companion. Storage still
//   holds exactly four vertices and drawing still submits four: keep together.
// The pool owns sprites, sprites borrow texture bindings, bindings own retained
// texture sources. See each release/destructor for their different free APIs.
#include "MSL/vector.h"
typedef std::vector<void*, app::app_allocator<void*> > SpritePtrVector;
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthErrorExceptionStrings[];
}
}

// Translation unit 0x800C7630-0x800C850C (24 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/exception.h"
#include <decomp/cw_bitfields.h>
extern "C" {

extern float kFloat_6_2831855;
extern float s_bpSpritePoolFullTurn;
extern float s_bpTextureBindingFullTurn;
extern float s_bpSpriteFullTurn;
extern char __vt__6CBpTex[];
extern char __vt__13CBpSpriteCtrl[];
extern "C" void* ISharedResource_GetTextureDescription(void*);
extern "C" int Get__15ISharedResourceFv(void*);
extern "C" void* appMallocDummy__FUl(unsigned int);
extern "C" void* __nwa__FUl(unsigned int);
extern "C" void* __nw__FUl(unsigned int);
extern "C" void __dla__FPv(void*);
extern "C" void appFreeUni__FPv(void*);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);
extern char __vt__9CBpSprite[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];
extern char __vt__Q23std12length_error[];
extern char vectorLengthErrorExceptionStrings[];
extern "C" void __dt__Q33std6detail16char_array_guardFv(void*, int);
extern "C" void dtor_80046988(void*, int);
extern "C" void Engine_SetTextureStageStateList(int, void*, int);
extern "C" void Engine_SetCachedRenderState(int, int);
extern "C" void Engine_SetFlexibleVertexFormat(int);
extern "C" void Engine_SetType7Resource(int);
extern "C" void* GX_GetPrimitiveCount(int, int);
extern "C" void DrawPrimitiveUP(int, void*, void*, int);
extern "C" void __dl__FPv(void*);
extern "C" void appFreeUni__FPv(void*);
extern "C" int CBpSprite_Draw(void*);
extern "C" void CubeResourceStorage_dtor(void*, int);
extern "C" int Engine_SetTextureStageResource(int, int);
extern "C" int Engine_SetTexturePalette(int, int);
extern "C" int ISharedResource_GetLocked(void*);
extern "C" int ISharedResource_GetPaletteObject(void*);
extern "C" void* CreateSharedTextureResource__FPCci(void*, int);
extern int s_bpSpriteVertexCountLimit;
extern char kBpSpriteTextureStageStates[];

// Four 24-byte vertices follow the sprite header; words retain float bit patterns.
// Pool protocol: reset marks all entries inactive; acquire returns a BORROWED
// inactive entry but does not claim it. Commit clears its vertex count and marks
// it active, then append fills vertices. Draw always submits four vertices.
// The pool owns both the pointer array and sprite objects. A sprite borrows its
// texture binding; deleting the sprite does not release that binding.
// Vertices carry caller-supplied XYZ float bits, RGBA bytes, and UV float bits;
// this unit does not convert coordinate spaces or apply a transform itself.
struct SpriteVertexStorage { unsigned int words[6]; };
struct SpriteFields {
    void* vtable;
    void* textureBinding;
    int active;
    int vertexCount;
    SpriteVertexStorage vertices[4];
    int drawStateOverride;
};
enum SpriteDrawConstants {
    kSpriteVertexCapacity = 4,
    kSpriteVertexStride = 0x18,
    kTriangleStrip = 0x98,
    kSpriteVertexFormat = (0x12 << 16) - 0xFFB,
    kSpriteDrawState = 0xE,
    kSpriteOverrideValue = 1,
    kSpriteDefaultValue = 5
};

struct SpritePoolDtorView {
    void* vtable;
    void** entries;
    unsigned int count;
    int capacity;
    char lock[0x10];
};

struct InstanceLock_u800C7630 {
    void* lock;
    // Acquire the pool's borrowed OS mutex for this scope.
    InstanceLock_u800C7630(void* l) : lock(l) { OSLockMutex((OSMutex*)l); }
    // Release the pool mutex after the operation or during exception unwind.
    ~InstanceLock_u800C7630() { OSUnlockMutex((OSMutex*)lock); }
};

struct WInstanceLock {
    void* lock;
    // Hold the pool mutex across all active-sprite draw calls.
    WInstanceLock(void* l) : lock(l) { OSLockMutex((OSMutex*)l); }
    // Release the borrowed mutex on normal return or exception unwind.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};

struct SpritePoolAcquireView { void* vtable; void** entries; unsigned int count; unsigned int capacity; char lock[0x10]; };

struct C7B78Buffer {
    void* data;
    char* allocator;
    // Temporarily own the exception text allocation during control-block construction.
    C7B78Buffer(void* p, char* a) : data(p), allocator(a) {}
    // Free text unless its ownership was transferred by clearing data.
    ~C7B78Buffer() { if (data) __dla__FPv(data); }
};
struct C7B78Ref {
    void* vtable;
    int refs, owners;
    void* data;
    // Retain a new shared-message allocation with initial reference counts of one.
    C7B78Ref(void* p) { refs = 1; owners = 1; vtable = s_sharedCharDeleterVtable; data = p; }
};
struct C7B78ErrorBase : std::exception {
    // Install the exception type's dispatch table in the reconstructed ABI storage.
    void SetTable(const void* vt) { *(const void**)this = vt; }

    char* message;
    C7B78Ref* ref;
    // Allocate a shared copy of the borrowed exception message.
    C7B78ErrorBase(const char* text) {
        SetTable(__vt__Q23std11logic_error);
        struct Allocator { char value; char pad[3]; } alloc;
        alloc.value = 0;
        unsigned int n = strlen(text) + 1;
        char* p = static_cast<char*>(__nwa__FUl(2*n-n));
        message = p;
        {
            C7B78Buffer guard(p, reinterpret_cast<char*>(&alloc));
            ref = new C7B78Ref(p);
            guard.data = 0;
        }
        strcpy(message, text);
    }
};
} // extern C
namespace std {
struct length_error : C7B78ErrorBase {
    // Build the message-owning base then install length_error dispatch.
    length_error(const char* text) : C7B78ErrorBase(text) { SetTable(__vt__Q23std12length_error); }
    ~length_error();
    DECOMP_CT_MEMBER(length_error)
};
}
extern "C" {
extern void __dt__Q23std12length_errorFv();
struct C7B78Storage {
    void** data;
    unsigned int count, capacity;
    unsigned int* allocator;
    unsigned int position;
    // Start an empty insertion guard borrowing the vector's allocator/capacity word.
    C7B78Storage(unsigned int* a) : data(0), count(0), capacity(0), allocator(a), position(0) {}
    // Free the guard's pointer array without destroying sprite pointees.
    ~C7B78Storage() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Select the larger vector growth bound without copying its storage.
inline const unsigned int& C7B78Max(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Construct and throw a vector length error while preserving exception cleanup.
inline void C7B78Throw(const char* text) {
    {
        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(text);
        exception_runtime::__throw(exception_runtime::vectorLengthErrorExceptionStrings + 0x14, error,
            (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Reuse an inactive sprite or append a newly allocated sprite under the pool mutex.
void* BpSpritePool_Acquire(SpritePoolAcquireView* self) {
    unsigned int reserved, used;
    void** oldData;
    int count;
    unsigned int bytes;
    void* obj;
    InstanceLock_u800C7630 lock(self->lock);
    void** end = self->entries + self->count;
    void** cur = self->entries;
    // Phase 1: reuse storage; the caller must commit before another acquire can reuse it.
    while (end != cur) {
        obj = *cur;
        if (((SpriteFields*)obj)->active == 0) {
            ((SpriteFields*)obj)->drawStateOverride = 0;
            return obj;
        }
        cur += 1;
    }
    // Phase 2: construct a header with uninitialized vertex storage.
    void* result = __nw__FUl(0x74);
    if (0 != result) {
        ((SpriteFields*)result)->vtable = __vt__9CBpSprite;
        (*(unsigned int*)&((SpriteFields*)result)->textureBinding) = 0;
        (*(unsigned int*)&((SpriteFields*)result)->active) = 0;
        (*(unsigned int*)&((SpriteFields*)result)->vertexCount) = 0;
        (*(unsigned int*)&((SpriteFields*)result)->drawStateOverride) = 0;
    }

    // Phase 3: append the owned sprite, growing the pointer array if necessary.
    used = self->count;
    reserved = self->capacity;
    if (used < reserved) {
        self->count = ++used;
        self->entries[self->count - 1] = result;
    } else {
        unsigned int growth1;
        unsigned int growth2;
        unsigned int need;
        if (0x3fffffffU - reserved < 1) {
            {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct("vector length error");
                exception_runtime::__throw(exception_runtime::vectorLengthErrorExceptionStrings + 0x14, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
        }
        C7B78Storage storage(&self->capacity);
        need = self->count + 1 - self->capacity;
        unsigned int cap = self->capacity;
        if (need > 0x3fffffffU - cap) C7B78Throw("vector length error");
        if (cap < 0x15555555U) {
            growth1 = (3 * (cap + 1)) / 5;
            cap += C7B78Max(growth1, need);
        } else if (cap < 0x2aaaaaaaU) {
            growth2 = ((cap + 1)) / 2;
            cap += C7B78Max(growth2, need);
        } else {
            cap = 0x3fffffffU;
        }
        if (cap > 0x3fffffffU) {
            {
                union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                std::length_error* error = (std::length_error*)storage.bytes;
                error->__ct("vector length error");
                exception_runtime::__throw(exception_runtime::vectorLengthErrorExceptionStrings + 0x14, error,
                    (void*)exception_runtime::__dt__Q23std12length_errorFv);
            }
        }

        // Build replacement storage before transferring ownership. The guard frees
        // the old pointer array after the swap, without deleting its sprite payloads.
        storage.data = static_cast<void**>(appMallocDummy__FUl((cap) * 4));
        storage.position = self->count;
        (storage.position + storage.data)[storage.count] = result;
        storage.capacity = cap;
        ++storage.count;
        oldData = self->entries;
        void** oldEnd = (self->count + oldData);
        count = (reinterpret_cast<char*>(oldEnd) - reinterpret_cast<char*>(oldData)) / 4;
        storage.position -= count;
        bytes = count * 4; memcpy(storage.data + storage.position, oldData, bytes);
        memset(oldData, 0, bytes);
        storage.count += count;
        self->count = 0;
        unsigned int oldCapacity = self->capacity;
        self->capacity = storage.capacity;
        storage.capacity = oldCapacity;
        void** old = self->entries;
        self->entries = storage.data;
        storage.data = old;
        self->count = storage.count;
        storage.count = 0;
    }
    return result;
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
struct VCall_767C { 
virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
};
// Bind the sprite texture, draw its four-vertex strip, and restore the overridden render state.
int CBpSprite_Draw(void* self) {
    // Texture stage zero is explicitly cleared after drawing; the optional state
    // override restores value 5 rather than saving/restoring arbitrary prior state.
    if (((SpriteFields*)self)->textureBinding != 0) {
        unsigned int descriptorWord1, descriptorWord0;
        unsigned int* source;
        source = reinterpret_cast<unsigned int*>(kBpSpriteTextureStageStates);
        descriptorWord0 = *source++;
        descriptorWord1 = *source++;
        int words[12];
        words[0] = descriptorWord0; words[1] = descriptorWord1; words[2] = source[0];
        unsigned int descriptorWord3 = source[1]; words[3] = descriptorWord3;
        unsigned int descriptorWord4 = source[2]; words[4] = descriptorWord4;
        words[5] = source[3]; words[6] = source[4]; words[7] = source[5];
        words[8] = source[6]; words[9] = source[7]; words[10] = source[8]; words[11] = source[9];
        Engine_SetTextureStageStateList(0, words, 6);
        void* object = ((SpriteFields*)self)->textureBinding;
        reinterpret_cast<VCall_767C*>(object)->s3();
    } else {
        Engine_SetTextureStageResource(0, 0);
        Engine_SetTexturePalette(0, 0);
    }
    int enabled = 0;
    if (((SpriteFields*)self)->drawStateOverride != 0) {
        Engine_SetCachedRenderState(kSpriteDrawState, kSpriteOverrideValue);
        enabled = 1;
    }
    Engine_SetFlexibleVertexFormat(kSpriteVertexFormat);
    Engine_SetType7Resource(0);
    void* primitiveCount = GX_GetPrimitiveCount(kSpriteVertexCapacity, kTriangleStrip);
    DrawPrimitiveUP(kTriangleStrip, primitiveCount, ((SpriteFields*)self)->vertices, kSpriteVertexStride);
    Engine_SetTextureStageResource(0, 0);
    Engine_SetTexturePalette(0, 0);
    if (enabled) Engine_SetCachedRenderState(kSpriteDrawState, kSpriteDefaultValue);
    return 1;
}
#pragma pop

#pragma opt_common_subs on
#pragma opt_propagation on
#pragma opt_strength_reduction on
struct C7958Virt { 
virtual void s0(int); virtual void s1(); virtual void s2();
};
// Delete pooled sprites under the mutex, free the pointer vector, and optionally free self.
void* CBpSpriteCtrl_dtor(SpritePoolDtorView* self, int flag) {
    if (self != 0) {
        self->vtable = __vt__13CBpSpriteCtrl;
        {
            WInstanceLock guard(self->lock);
            void** entry = self->entries;
            while (entry != self->entries + self->count) {
                void* object = *entry;
                if (object != 0) {
                    if (object != 0) {
                        reinterpret_cast<C7958Virt*>(object)->s0(1);
                    }
                    *entry = 0;
                }
                ++entry;
            }
            void** allocation = self->entries;
            self->capacity = 0;
            self->entries = 0;
            self->count = 0;
            if (allocation != 0) appFreeUni__FPv(allocation);
        }
        ((SpritePtrVector*)&self->entries)->~SpritePtrVector();
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_strength_reduction reset

// Descriptor schema: seven native 32-bit words (0x1C bytes), 4-byte aligned.
// +00 width pixels; +04 height pixels; +08 producer writes zero (role unknown);
// +0C mip-level count; +10 platform-7 format ID; +14 producer writes tag 3
// (role unknown); +18 opaque trailing word. The shared producer in gtl_texture_vertex_formats.cpp
// writes only the FIRST SIX words; this copier forwards word 6 unchanged, even
// if the producer left it uninitialized. Never interpret it as a valid dimension
// or initialize it as part of a readability edit. Direct sources copy all seven
// stored words instead of consulting a shared object. No endian/float conversion.
struct TextureBindingView {
    void* vtable;
    int sharedTextureResource; // ISharedResource address, kept as a word for this call ABI.
    int descriptorWord0, descriptorWord1, descriptorWord2, descriptorWord3, descriptorWord4, descriptorWord5, descriptorWord6;
    int directTextureResource; // Direct texture resource address, with packed shared refcount.
};

struct SpritePoolInitView {
    void* vtable;
    void* member;
    int count;
    int capacity;
    char mutex[0x10];
};

struct SpritePoolResetView {
    void* vtable;
    void** entries;
    unsigned int count;
    int capacity;
    char mutex[0x10];
};

struct SpritePoolDrawView {
    void* vtable;
    void** entries;
    unsigned int count;
    int capacity;
    char mutex[0x10];
};

struct SpriteVertexBits {
    int positionX, positionY, positionZ;
    unsigned char color[4];
    int texCoordU, texCoordV;
};
struct SpriteVertexBufferView { char spriteHeader[12]; int count; SpriteVertexBits items[1]; };

// Initialize an empty texture binding with neither a shared file nor a direct resource.
void CBpTex_Init(TextureBindingView* self) {
    self->sharedTextureResource = 0;
    self->vtable = __vt__6CBpTex;
    self->directTextureResource = 0;
}

// Return shared-file texture metadata; a direct texture has no metadata pointer here.
void* BpSprite_GetTextureDescription(TextureBindingView* self) {
    if (self->directTextureResource != 0) {
        return 0;
    }
    return ISharedResource_GetTextureDescription(*(void**)&self->sharedTextureResource);
}

// Free a sprite when called as a deleting destructor.
void* CBpSprite_dtor(void* self, int flag) {
    if (self != 0 && flag > 0) {
        __dl__FPv(self);
    }
    return self;
}

// SpriteBuffer is the sprite pointer vector: destroy it and free the object too when flag > 0.
void* __dt__12SpriteBufferFv(SpritePtrVector* self, int flag) {
    return std::__vector_deleting_dtor(self, flag);
}

// Return the texture interface's constant zero result.
int CBpTex_vfunc_2() {
    return 0;
}

struct ObjectPoolView {
    void* vtable;
    void** entries;
    unsigned int count;
    int capacity;
    char lock[0x10];
};

struct VirtualCallTrial_800C806C {
virtual void invoke(int);
};
// Delete every virtual object and release the pointer array while holding its mutex.
int ObjectArray_DeleteAllAndClear(ObjectPoolView* self) {
    WInstanceLock guard(self->lock);
    void** entry = self->entries;
    while (entry != self->entries + self->count) {
        void* object = *entry;
        if (object != 0) {
            if (object != 0) {
                reinterpret_cast<VirtualCallTrial_800C806C*>(object)->invoke(1);
            }
            *entry = 0;
        }
        ++entry;
    }
    void* entries = self->entries;
    self->capacity = 0;
    self->entries = 0;
    self->count = 0;
    if (entries != 0) appFreeUni__FPv(entries);
    return 1;
}

struct D8154Virt {
    virtual void s0();
    virtual void s1();
};
struct PackedResourceRefCount {
    CW_BITS2(
        unsigned int hi : 16,
        unsigned int lo : 16)
};
struct TextureBindingDtorView {
    void* vtable;
    D8154Virt* sharedTextureResource;
    int w[7];
    PackedResourceRefCount* directTextureResource;
};

// Release the shared texture and direct resource reference, then optionally free the binding.
void* CBpTex_dtor(TextureBindingDtorView* self, int flag) {
    if (self != 0) {
        self->vtable = __vt__6CBpTex;
        if (self->sharedTextureResource != 0) {
            self->sharedTextureResource->s1();
            self->sharedTextureResource = 0;
        }
        if (self->directTextureResource != 0) {
            PackedResourceRefCount* p = self->directTextureResource;
            p->hi = p->hi - 1;
            if (p->hi == 0) {
                CubeResourceStorage_dtor(p, 1);
            }
            self->directTextureResource = 0;
        }
        if (flag > 0) __dl__FPv(self);
    }
    return self;
}

#pragma opt_propagation off
// Copy the descriptor above into seven caller-owned writable words. out==null
// returns 0 without dereferencing self or changing anything. Otherwise self must
// be valid with either direct metadata populated or a live shared source (no
// readiness/null-description check here); return 1 means copied, not loaded.
// No retained pointer is returned. Shared source/destination must not overlap;
// direct reads are all staged before writes, a sequencing detail to preserve.
int BpSprite_GetTextureDescriptor(TextureBindingView* self, int* out) {
    if (out == 0) return 0;
    if (self->directTextureResource) {
        unsigned int trailingWord, producerTag, format, mipLevels, zeroWord, height, width;
        int* p = &self->descriptorWord0;
        width = p[0]; height = p[1]; zeroWord = p[2]; mipLevels = p[3];
        format = p[4]; producerTag = p[5]; trailingWord = p[6];
        out[0] = width; out[1] = height; out[2] = zeroWord; out[3] = mipLevels;
        out[4] = format; out[5] = producerTag; out[6] = trailingWord;
        return 1;
    }
    int* values = static_cast<int*>(ISharedResource_GetTextureDescription(reinterpret_cast<void*>(self->sharedTextureResource)));
    out[0] = values[0]; out[1] = values[1]; out[2] = values[2];
    out[3] = values[3]; out[4] = values[4]; out[5] = values[5]; out[6] = values[6];
    return 1;
}

#pragma opt_propagation reset

// Initialize the texture-binding unit's angle constant.
void BpTextureBinding_InitializeConstants() {
    s_bpTextureBindingFullTurn = kFloat_6_2831855;
}

// Initialize the final sprite unit's angle constant.
void BpSprite_InitializeConstants() {
    s_bpSpriteFullTurn = kFloat_6_2831855;
}

// Initialize the sprite-pool unit's angle constant.
void BpSpritePool_InitializeConstants() {
    s_bpSpritePoolFullTurn = kFloat_6_2831855;
}

// Store the texture-binding address used when this sprite is drawn.
int CBpSprite_BeginVertices(void* self, int textureBindingAddress) {
    (*(int*)&((SpriteFields*)self)->textureBinding) = textureBindingAddress;
    return 1;
}

#pragma opt_propagation off
// Copy position/color/UV bit patterns into the next slot and clamp the resulting vertex count.
int CBpSprite_AppendVertex(SpriteVertexBufferView* self, SpriteVertexBits* item) {
    // Precondition: count indexes writable storage BEFORE increment/clamp. The
    // post-write count clamp is not a bounds check for this store.
    SpriteVertexBits* dest;
    int positionXBits, positionYBits, texCoordUBits, texCoordVBits;
    int next;
    int* result = &next;
    positionXBits = item->positionX;
    dest = &self->items[self->count];
    positionYBits = item->positionY;
    dest->positionX = positionXBits;
    dest->positionY = positionYBits;
    dest->positionZ = item->positionZ;
    dest->color[0] = item->color[0];
    dest->color[1] = item->color[1];
    dest->color[2] = item->color[2];
    dest->color[3] = item->color[3];
    texCoordVBits = item->texCoordV;
    texCoordUBits = item->texCoordU;
    dest->texCoordU = texCoordUBits;
    dest->texCoordV = texCoordVBits;
    next = self->count + 1;
    if (s_bpSpriteVertexCountLimit < next) result = &s_bpSpriteVertexCountLimit;
    self->count = *result;
    return 0;
}
#pragma opt_propagation reset

// Draw every committed sprite while holding the pool mutex.
int BpSpritePool_DrawActiveSprites(SpritePoolDrawView* self) {
    void* lock = self->mutex;
    WInstanceLock guard(lock);
    void** entry = self->entries;
    while (entry != self->entries + self->count) {
        void* obj = *entry;
        if (((SpriteFields*)obj)->active != 0) {
            CBpSprite_Draw(obj);
        }
        ++entry;
    }
    return 1;
}

// Clear the vertex count and mark the sprite active.
int CBpSprite_ActivateAndClearVertexCount(void* self) {
    ((SpriteFields*)self)->vertexCount = 0;
    ((SpriteFields*)self)->active = 1;
    return 1;
}

}
struct SpriteBuffer {
    void* member;
    int count, capacity;
    // Initialize the vector allocation; its enclosing constructor clears size/capacity.
    SpriteBuffer() : member(0) {}
    ~SpriteBuffer();
};
struct CBpSpriteCtrl {
    void* vtable;
    SpriteBuffer buffer;
    char mutex[0x10];
    CBpSpriteCtrl();
};
// Initialize an empty sprite pool and its instance mutex.
CBpSpriteCtrl::CBpSpriteCtrl() {
    SpritePoolInitView* self = (SpritePoolInitView*)this;
    self->vtable = __vt__13CBpSpriteCtrl;
    self->count = 0;
    self->capacity = 0;
    OSInitMutex((OSMutex*)self->mutex);
}
extern "C" {

// Mark all entries inactive under the pool mutex; vertex counts, vertex bytes,
// texture bindings, pool size/capacity and ownership survive. Always returns 1.
// Semantic name: BpSpritePool_MarkAllInactive. The
// legacy exported spelling below must not be read as a vertex-count reset:
// only ActivateAndClearVertexCount clears a sprite's count, upon committing it.
int BpSpritePool_MarkAllInactive(SpritePoolResetView* self) {
    void* mutex = self->mutex;
    OSLockMutex((OSMutex*)mutex);
    void** entry = self->entries;
    int zero = 0;
    while (entry != self->entries + self->count) {
        void* obj = *entry;
        ++entry;
        ((SpriteFields*)obj)->active = zero;
    }
    OSUnlockMutex((OSMutex*)mutex);
    return 1;
}

// Report readiness of a direct texture or its shared texture resource.
int CBpTex_IsLoaded(TextureBindingView* self) {
    if (self->directTextureResource != 0) {
        return 1;
    }
    return Get__15ISharedResourceFv(reinterpret_cast<void*>(self->sharedTextureResource)) != 0;
}

// Bind a direct texture, or resolve and bind the shared resource and hardware texture.
int CBpTex_Bind(const TextureBindingView* self) {
    // Only the direct-resource branch defines a return value. Existing callers
    // use this operation for binding side effects; do not interpret the shared path's result.
    if (self->directTextureResource != 0) {
        Engine_SetTextureStageResource(0, self->directTextureResource);
        return 1;
    }
    int value = ISharedResource_GetLocked(reinterpret_cast<void*>(self->sharedTextureResource));
    Engine_SetTextureStageResource(0, value);
    int status = ISharedResource_GetPaletteObject(reinterpret_cast<void*>(self->sharedTextureResource));
    if (status != 0) {
        status = ISharedResource_GetPaletteObject(reinterpret_cast<void*>(self->sharedTextureResource));
        Engine_SetTexturePalette(0, status);
    } else {
        Engine_SetTexturePalette(0, 0);
    }
}

struct C82B8Virt { 
virtual void s0(); virtual void s1(); virtual void s2(); virtual void s3();
};
// Release both texture sources and clear their stored addresses.
void BpTextureBinding_ReleaseSources(TextureBindingView* self) {
    if (self->sharedTextureResource != 0) {
        reinterpret_cast<C82B8Virt*>(self->sharedTextureResource)->s1();
        self->sharedTextureResource = 0;
    }
    PackedResourceRefCount* p = reinterpret_cast<PackedResourceRefCount*>(self->directTextureResource);
    if (p != 0) {
        p->hi = p->hi - 1;
        if (p->hi == 0) {
            CubeResourceStorage_dtor(p, 1);
        }
        self->directTextureResource = 0;
    }
}

// Release existing texture sources and request a shared texture by path.
int CBpTex_BindTexturePath(TextureBindingView* self, void* texturePath) {
    // Return 1 means the request was issued, not that the texture is loaded;
    // readiness is queried separately by CBpTex_IsLoaded.
    if (self->sharedTextureResource != 0) {
        reinterpret_cast<C82B8Virt*>(self->sharedTextureResource)->s1();
        self->sharedTextureResource = 0;
    }
    PackedResourceRefCount* p = reinterpret_cast<PackedResourceRefCount*>(self->directTextureResource);
    if (p != 0) {
        p->hi = p->hi - 1;
        if (p->hi == 0) CubeResourceStorage_dtor(p, 1);
        self->directTextureResource = 0;
    }
    self->sharedTextureResource = reinterpret_cast<int>(CreateSharedTextureResource__FPCci(texturePath, 0));
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620B0", _ctors_u800C7630_0, BpSpritePool_InitializeConstants);
STATIC_INIT_ENTRY(".CRT$XCU804620B0", _ctors_u800C7630_1, BpTextureBinding_InitializeConstants);
STATIC_INIT_ENTRY(".CRT$XCU804620B0", _ctors_u800C7630_2, BpSprite_InitializeConstants);
#pragma pop
