// SPD sprite manager: the sprite controllers (CSpdCtrl) that hold square and sprite item lists, and the
// sprite and solid-color square objects (CSpdSprite, CSpdSqar) with their geometry.
//
// Sprite setters and getters (position, rotation, color, scale, origin) and the cloning of sprites.

#include <decomp/ct_member.h>
#include "MSL/error_message.h"
#include "MSL/vector.h"
typedef std::vector<void*, app::app_allocator<void*> > SpdPointerVector;
enum {
    kFlipVertical = 1,
    kFlipHorizontal = 2,
    kFlipBoth = 3
};
enum {
    kMaxPointerElements = 0x3FFFFFFF,
    kThreeFifthsGrowthLimit = 0x15555555,
    kHalfGrowthLimit = 0x2AAAAAAA
};
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char vectorLengthExceptionStrings[];
}
}

#include "sdk_protos.h"
#include "game/types.h"
#include "game/exception.h"
#include "classes/CBpSprite.h"
#include "classes/CSpdSprite.h"
#include "classes/CSpdSqar.h"
#include "classes/CSpdCtrl.h"
#include "classes/CBpSpriteCtrl.h"

// Return caller-supplied placement storage without allocation.
inline void* operator new(size_t, void* p) {
    return p;
}

// Declarations used by the member functions below. The namespace block that
// follows declares the same objects and functions.
extern "C" {
extern char __vt__8CSpdSqar[];
extern char __vt__10CSpdSprite[];
extern char __vt__8CSpdCtrl[];
extern void __ct__13CBpSpriteCtrlFv(void*);
extern void ObjectArray_DeleteAllAndClear(void*);
extern void __dl__FPv(void*);
extern float kFloat_0_Layout;
extern const float kFloat_1_Layout;
extern void PSMTX44Identity(void*);
extern void Mtx44_SetTranslation(void*, float, float, float);
extern void Mtx44_SetScale(void*, float, float, float);
extern void PSMTX44Concat(void*, void*, void*);
extern void Mtx44_SetRotationZ(void*, float);
extern void Mtx44_TransformPointPerspective(void*, void*, void*);
extern void BpSprite_ApplyRenderModeFlags(int, void*); // CBpSprite_SetFlipMode.
}

// ---- merged from spd_sprite_manager.cpp ----
namespace ns0 {

extern "C" {

extern char __vt__8CSpdSqar[];
extern char __vt__10CSpdSprite[];
extern char __vt__8CSpdCtrl[];

extern void __ct__13CBpSpriteCtrlFv(void*);
extern void BpSprite_GetTextureDescriptor(void*, unsigned int*);
extern void ObjectArray_DeleteAllAndClear(void*);
extern void* BpSpritePool_Acquire(void*);
extern void* appMallocDummy__FUl(unsigned int);
extern void appFreeUni__FPv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);

struct SpdDrawItem {
    char padding00[0x2C];
    void* ptr;
};

struct SpdSquare {
    void** vtable;
    float rotation;
    unsigned int color;
    float scaleX, scaleY, positionX, positionY, sourceWidth, sourceHeight, originX, originY;
    void* sprite;
    void* dirty;
};

struct SpdSquareList {
    char padding00[0x10];
    void** data;
    unsigned int size;
    unsigned int capacity;
    void* source;
};

extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[],
    vectorLengthExceptionStrings[];
extern size_t strlen(const char*);
extern char* strcpy(char*, const char*);
extern void* memcpy(void*, const void*, unsigned long);
extern void* memset(void*, int, unsigned long);
extern void* __nwa__FUl(unsigned long);
extern void __throw(void*, void*, void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
struct VecAllocation : ::std::detail::__message_buffer_guard {
    VecAllocation(char* p, char* a) : ::std::detail::__message_buffer_guard(p, a) {}
    ~VecAllocation() {
        if (pointer)
            delete[] pointer;
    }
};
typedef ::std::detail::__message_control VecControl;
struct VecErrorBase {
    void* table;
    // Install the logic_error base vtable.
    VecErrorBase() {
        table = __vt__Q23std11logic_error;
    }
    // Destroy the standard exception base without freeing this stack wrapper.
    ~VecErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : VecErrorBase {
    ::std::__error_message<VecAllocation, VecControl> message_;
    // Copy the vector diagnostic into shared owned storage and install length_error.
    length_error() {
        const char* message = vectorLengthExceptionStrings;
        message_.assign(message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
struct VecBuffer {
    void** data;
    volatile unsigned size;
    unsigned capacity;
    unsigned* owner;
    volatile unsigned offset;
    // Initialize an empty temporary pointer-buffer owner associated with capacity p.
    VecBuffer(unsigned* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    // Free the allocation currently held by this temporary owner.
    ~VecBuffer() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
// Return the larger unsigned capacity candidate by reference.
inline const unsigned& VecMax(const unsigned& a, const unsigned& b) {
    return a < b ? b : a;
}
// Construct and throw the vector length_error with runtime cleanup metadata.
inline void VecThrow() {
    {
        union {
            double alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct();
        exception_runtime::__throw(exception_runtime::vectorLengthExceptionStrings + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}

// Clone square geometry into a newly acquired sprite and append it; index must select an existing square.
int SpdSquareList_CloneEntry(SpdSquareList* self, unsigned int index) {
    SpdSquare* item = (index >= self->size) ? 0 : (SpdSquare*)self->data[index];
    // Phase 1: allocate square geometry, acquire its backing sprite and copy source geometry.
    SpdSquare* copy = (SpdSquare*)__nw__FUl(0x34);
    if (copy) {
        copy->sprite = 0;
        copy->dirty = 0;
        copy->vtable = (void**)__vt__8CSpdSqar;
    }
    void* resource = BpSpritePool_Acquire(self->source);
    if (&copy->rotation)
        copy->rotation = item->rotation;
    if (&copy->color)
        copy->color = item->color;
    if (&copy->scaleX) {
        copy->scaleX = item->scaleX;
        copy->scaleY = item->scaleY;
    }
    if (&copy->originX) {
        copy->originX = item->originX;
        copy->originY = item->originY;
    }
    if (&copy->positionX) {
        copy->positionX = item->positionX;
        copy->positionY = item->positionY;
    }
    if (&copy->sourceWidth) {
        copy->sourceWidth = item->sourceWidth;
        copy->sourceHeight = item->sourceHeight;
    }
    copy->sprite = resource;
    ((void (*)(SpdSquare*))copy->vtable[3])(copy);
    // Phase 2: append the owned geometry pointer; reallocation transfers pointer slots,
    // not the pointed-to geometry. The temporary owns the old allocation after commit.
    unsigned int size = self->size;
    if (size < self->capacity) {

        int next = (size + 1);
        *(volatile unsigned*)&self->size = next;
        void** data = self->data;
        data[next - 1] = copy;
    } else {
        if ((unsigned int)kMaxPointerElements - self->capacity < 1)
            VecThrow();
        unsigned int needed = self->size + 1 - self->capacity;
        __typeof__(self->capacity) capacityBeforeGrowth = self->capacity;
        __typeof__(self->capacity) previousCapacity = self->capacity;
        VecBuffer temp(&self->capacity);
        unsigned int capacity = previousCapacity;
        if (needed > (unsigned int)kMaxPointerElements - capacity) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::vectorLengthExceptionStrings + 0x14, error,
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        if (capacity < (unsigned int)kThreeFifthsGrowthLimit) {
            capacity += VecMax((3 * ((capacity + 1))) / 5, needed);
        } else if (capacity < (unsigned int)kHalfGrowthLimit) {
            capacity += VecMax(((capacity + 1)) / 2, needed);
        } else
            capacity = (unsigned int)kMaxPointerElements;
        if (capacity > (unsigned int)kMaxPointerElements) {
            union {
                double alignment;
                unsigned char bytes[sizeof(std::length_error)];
            } storage;
            std::length_error* error = (std::length_error*)storage.bytes;
            error->__ct();
            exception_runtime::__throw(exception_runtime::vectorLengthExceptionStrings + 0x14, error,
                                       (void*)exception_runtime::__dt__Q23std12length_errorFv);
        }
        void** data = (void**)appMallocDummy__FUl((4 * capacity));
        unsigned int oldSize = self->size;
        temp.offset = oldSize;
        ((oldSize + data))[temp.size++] = copy;
        void** oldData = self->data;

        temp.capacity = capacity;
        temp.data = data;
        int count = ((char*)(self->size + oldData) - (char*)oldData) / 4;
        unsigned int offset = oldSize - count;
        temp.offset = offset;
        int bytes = (4 * count);
        memcpy((offset + data), oldData, bytes);
        memset(oldData, 0, bytes);
        int newSize = (temp.size + count);
        data = temp.data;
        __typeof__(self->capacity) capacityBeforeSwap = self->capacity;
        temp.size = newSize;
        oldData = self->data;
        temp.data = oldData;

        __typeof__(capacityBeforeSwap) oldCapacity = self->capacity;
        self->capacity = (temp.capacity);
        self->size = 0;

        temp.capacity = (oldCapacity);
        self->data = data;
        self->size = newSize;
        temp.size = 0;
    }
    return self->size - 1;
}

// Draw the borrowed backing BP sprite and return success.
int SimpleSpd_DrawBpSprite(SpdDrawItem* self) {
    if (self->ptr)
        ((CBpSprite*)(self->ptr))->Draw();
    return 1;
}

} // C linkage
} // namespace ns0

// The bodies are empty: the deleting destructor frees the object.
CSpdSprite::~CSpdSprite() {}
CSpdSqar::~CSpdSqar() {}

// Initialize two empty item lists and acquire an owned BP sprite-controller pool.
CSpdCtrl::CSpdCtrl() {
    sprites.data = 0;
    vtbl = __vt__8CSpdCtrl;
    sprites.count = 0;
    sprites.capacity = 0;
    squares.data = 0;
    squares.count = 0;
    squares.capacity = 0;
    context = 0;
    resourceFlags = 0;
    struct AllocatedSpriteCtrl {
        char data[0x28];
        // Construct the separately owned BP sprite-controller pool.
        AllocatedSpriteCtrl() {
            ((CBpSpriteCtrl*)this)->__ct();
        }
    };
    void* ptr = new AllocatedSpriteCtrl;
    spritePool = ptr;
}
extern "C" {

// SpdListA is a pointer vector; destroy it and free the object too when deletingFlag > 0.
void* __dt__8SpdListAFv(SpdPointerVector* self, int deletingFlag) {
    return ::std::__vector_deleting_dtor(self, deletingFlag); // ::std: this unit nests its own std inside ns0
}

// SpdListB is a pointer vector; destroy it and free the object too when deletingFlag > 0.
void* __dt__8SpdListBFv(SpdPointerVector* self, int deletingFlag) {
    return ::std::__vector_deleting_dtor(self, deletingFlag); // ::std: this unit nests its own std inside ns0
}

} // C linkage

// Initialize an unbound square with its vtable and cleared sprite/dirty state.
CSpdSqar::CSpdSqar() {
    sprite = 0;
    dirty = 0;
    vtbl = __vt__8CSpdSqar;
}

namespace ns0 {
extern "C" {

struct SpdPositionFields {
    char padding00[0x14];
    float x;
    float y;
    char padding1C[0x14];
    int dirty;
};

// +0x04 is the Z rotation in radians (fed directly to Mtx44_SetRotationZ).
// This and SpdRotationValue are partial views of the geometry, not owned wrappers.
struct SpdRotationFields {
    char padding00[4];
    float rotationRadians;
    char padding08[0x28];
    int dirty;
};

// Set Z rotation in radians and mark geometry dirty; always return success.
int SpdGeometry_SetRotationRadians(SpdRotationFields* self, float value) {
    self->rotationRadians = value;
    self->dirty = 1;
    return 1;
}

struct SpdColorFields {
    char padding00[8];
    void* value;
    char padding0C[0x24];
    int dirty;
};

// Store the packed color word (pointer-shaped ABI), mark geometry dirty and return 1.
int Sprite_SetPackedColor(SpdColorFields* self, void* value) {
    self->value = value;
    self->dirty = 1;
    return 1;
}

struct Vec2f {
    float x;
    float y;
};

// Copy sprite XY position from the borrowed vector, mark dirty and return 1.
int Sprite_SetPosition2D(SpdPositionFields* self, Vec2f* value) {
    float y, x;
    x = value->x;
    y = value->y;
    self->x = x;
    self->y = y;
    self->dirty = 1;
    return 1;
}

// Copy sprite XY position; return 0 for null output, otherwise 1.
int Sprite_GetPosition2D(SpdPositionFields* self, Vec2f* out) {
    if (out == 0)
        return 0;
    float y;
    float x;
    x = self->x;
    y = self->y;
    out->x = x;
    out->y = y;
    return 1;
}

struct SpdRotationValue {
    char padding00[4];
    float rotationRadians;
};

// Copy Z rotation in radians to a borrowed output; return 0 for a null output, 1 otherwise.
int SpdGeometry_GetRotationRadians(SpdRotationValue* self, float* out) {
    if (out == 0)
        return 0;
    *out = self->rotationRadians;
    return 1;
}

struct SpriteTextureSizeFields {
    char padding00[0x48];
    float x;
    float y;
};

struct SpriteTextureRegionFields {
    char padding00[0x1C];
    float left, top;
    char padding24[0x10];
    int source;
    float scaleX, scaleY;
    float right, bottom;
    float width, height;
};

extern const float kFloat_1_Layout;
extern double kSpriteUnsignedFloatConversionBias; // kSpriteUnsignedFloatConversionBias.
extern float kFloat_0_Layout;

// Borrow the texture source and normalize pixel bounds by its dimensions (or 1 if null).
int TextureRegion_SetSourceAndNormalizeBounds(SpriteTextureRegionFields* self, void* value) {
    float width = kFloat_1_Layout;
    float height = kFloat_1_Layout;

    self->source = (int)value;
    if (value) {
        unsigned int dimensions[8];
        BpSprite_GetTextureDescriptor(value, dimensions);
        width = (float)dimensions[0];
        height = (float)dimensions[1];
    }
    self->scaleX = self->width / width;
    self->scaleY = self->height / height;
    self->right = self->scaleX + self->left / width;
    self->bottom = self->scaleY + self->top / height;
    return 1;
}

struct SpdSquareResource {
    float x, y;
    unsigned short w, h;
    float a, b, c, d;
    float e;
    unsigned char color0, color1, color2, color3;
};
struct SpdSquareGeometry {
    virtual ~SpdSquareGeometry();
    virtual void update();
    float rotation;
    unsigned int color;
    float scaleX, scaleY, x, y, width, height;
    float originX, originY;
    void* sprite;
};
#pragma opt_propagation off
#pragma opt_common_subs off
// Expand a square resource into scaled geometry, pack its color, and rebuild its sprite.
int SpdItem_InitializeScaledGeometry(SpdSquareGeometry* out, void* sprite, const SpdSquareResource* in) {
    float v, u, y, x, d, width, height, c, rotation;
    unsigned int color1, color0, high, color2, color3, low, color;
    width = in->a * (float)in->w;
    height = in->b * (float)in->h;
    rotation = in->e;
    u = (float)in->w;
    c = in->c;
    v = (float)in->h;
    d = in->d;
    x = in->x;
    y = in->y;
    color1 = in->color1;
    color0 = in->color0;
    high = (color0 << 24) | (color1 << 16);
    color2 = in->color2;
    color3 = in->color3;
    low = (color2 << 8) | color3;
    color = low | high;
    out->rotation = rotation;
    out->color = color;
    out->scaleX = c;
    out->scaleY = d;
    out->originX = width;
    out->originY = height;
    out->x = x;
    out->y = y;
    out->width = u;
    out->height = v;
    out->sprite = sprite;
    out->update();
    return 1;
}
#pragma opt_propagation reset
#pragma opt_common_subs reset

// Copy texture-region XY size and return success.
int Sprite_SetTextureRegionSize2D(SpriteTextureSizeFields* self, Vec2f* value) {
    float y, x;
    x = value->x;
    y = value->y;
    self->x = x;
    self->y = y;
    return 1;
}

struct SpdScaleFields {
    char padding00[0x0C];
    float x;
    float y;
    char padding14[0x1C];
    int dirty;
};

// Copy dimensionless XY scale, mark geometry dirty and return 1.
int Sprite_SetScale2D(SpdScaleFields* self, const Vec2f* value) {
    self->x = value->x;
    self->y = value->y;
    self->dirty = 1;
    return 1;
}

struct SpdOriginFields {
    char padding00[0x24];
    float x;
    float y;
    char padding2C[4];
    int dirty;
};

// Copy local XY pivot, mark geometry dirty and return 1.
int Sprite_SetOrigin2D(SpdOriginFields* self, const Vec2f* value) {
    self->x = value->x;
    self->y = value->y;
    self->dirty = 1;
    return 1;
}

struct SpdColorValue {
    char padding00[8];
    void* value;
};

// Copy the packed color word; return 0 for null output, otherwise 1.
int Sprite_GetPackedColor(SpdColorValue* self, void** out) {
    if (out == 0)
        return 0;
    *out = self->value;
    return 1;
}

struct SpdSourceSizeFields {
    char padding00[0x1C];
    float x;
    float y;
    char padding24[0x0C];
    int dirty;
};

// Copy untransformed source XY size, mark geometry dirty and return 1.
int Sprite_SetSourceSize2D(SpdSourceSizeFields* self, Vec2f* value) {
    float y, x;
    x = value->x;
    y = value->y;
    self->x = x;
    self->y = y;
    self->dirty = 1;
    return 1;
}

// Copy dimensionless XY scale; return 0 for null output, otherwise 1.
int Sprite_GetScale2D(SpdScaleFields* self, Vec2f* out) {
    if (out == 0)
        return 0;
    float y;
    float x;
    x = self->x;
    y = self->y;
    out->x = x;
    out->y = y;
    return 1;
}

// Copy local XY pivot; return 0 for null output, otherwise 1.
int Sprite_GetOrigin2D(SpdOriginFields* self, Vec2f* out) {
    if (out == 0)
        return 0;
    float y;
    float x;
    x = self->x;
    y = self->y;
    out->x = x;
    out->y = y;
    return 1;
}

// Copy untransformed source XY size; return 0 for null output, otherwise 1.
int Sprite_GetSourceSize2D(SpdSourceSizeFields* self, Vec2f* out) {
    if (out == 0)
        return 0;
    float y;
    float x;
    x = self->x;
    y = self->y;
    out->x = x;
    out->y = y;
    return 1;
}

struct SpdSpriteResource {
    float x, y;
    unsigned short w0, h0, w1, h1;
    float width, height, left, top, rotation;
    unsigned char c0, c1, c2, c3, c4;
};

struct SpdSpriteGeometry {
    virtual ~SpdSpriteGeometry();
    virtual void update();
    float rotation;
    unsigned int color;
    float scaleX, scaleY, x, y;
    float w0cv, h0cv;
    float originX, originY;
    int sprite;
    int dirty;
    int source;
    float uvWidth, uvHeight, right, bottom;
    float w1cv, h1cv;
    int flipMode;
};

// Populate sprite geometry and UV bounds from a resource entry, then rebuild its vertices.
int SpdItem_InitializeFromResourceEntry(SpdSpriteGeometry* self, int sprite, const SpdSpriteResource* in,
                                        void* source) {
    float texY;
    float texX;
    self->sprite = sprite;
    self->source = (int)source;
    self->rotation = in->rotation;
    self->color = (in->c2 << 24) | (in->c3 << 16) | (in->c4 << 8) | in->c0;
    self->w1cv = (float)in->w1;
    self->h1cv = (float)in->h1;
    self->scaleX = in->left;
    self->scaleY = in->top;
    self->originX = in->width * (float)in->w0;
    self->originY = in->height * (float)in->h0;
    self->x = in->x;
    self->y = in->y;
    self->w0cv = (float)in->w0;
    self->h0cv = (float)in->h0;
    self->flipMode = in->c1;
    self->sprite = sprite;
    texX = kFloat_1_Layout;
    texY = kFloat_1_Layout;
    if (source) {
        unsigned int dimensions[7];
        BpSprite_GetTextureDescriptor(source, dimensions);
        texX = (float)dimensions[0];
        texY = (float)dimensions[1];
    }
    self->uvWidth = self->w1cv / texX;
    self->uvHeight = self->h1cv / texY;
    self->right = self->uvWidth + self->w0cv / texX;
    self->bottom = self->uvHeight + self->h0cv / texY;
    self->update();
    return 1;
}

typedef Vec3f SpdVertexPosition;
struct SpdVertexColor {
    unsigned char c0, c1, c2, c3;
    float x, y;
};

} // C linkage
} // namespace ns0

// Rebuild four transformed textured vertices, applying the requested UV flip, and clear the dirty flag.
int CSpdSprite::RebuildVertices() {
    sprite->BeginVertices((int)source);
    if (flipMode == kFlipHorizontal) {
        float temp = u0;
        u0 = u1;
        u1 = temp;
    } else if (flipMode == kFlipVertical) {
        float temp = v0;
        v0 = v1;
        v1 = temp;
    } else if (flipMode == kFlipBoth) {
        float temp = u0;
        __typeof__(v0) savedV0 = v0;
        u0 = u1;
        u1 = temp;
        temp = savedV0;
        v0 = v1;
        v1 = temp;
    }
    BpSprite_ApplyRenderModeFlags(flipMode, sprite);

    char transform[0x40], translationMatrix[0x40], rotationMatrix[0x40], scaleMatrix[0x40];
    struct Vertex {
        float x, y, z;
        unsigned char color[4];
        float u, v;
    } vertex;
    vertex.color[0] = (unsigned char)(color >> 24);
    vertex.color[1] = (unsigned char)(color >> 16);
    vertex.color[2] = (unsigned char)(color >> 8);
    vertex.color[3] = (unsigned char)color;
    // Sprite-local XY corners -> pivot-relative -> scaled -> Z-rotated -> positioned.
    // Keep the concat calls in this order; the engine wrapper writes its FIRST argument.
    // Byte buffers are 16-float matrix storage; their declaration order preserves stack layout.
    PSMTX44Identity(transform);
    Mtx44_SetTranslation(transform, -originX, -originY, kFloat_0_Layout);
    Mtx44_SetScale(scaleMatrix, scaleX, scaleY, kFloat_1_Layout);
    PSMTX44Concat(transform, transform, scaleMatrix);
    Mtx44_SetRotationZ(rotationMatrix, rotation);
    PSMTX44Concat(transform, transform, rotationMatrix);
    Mtx44_SetTranslation(translationMatrix, positionX, positionY, kFloat_0_Layout);
    PSMTX44Concat(transform, transform, translationMatrix);

    vertex.u = u0;
    vertex.v = v0;
    vertex.x = kFloat_0_Layout;
    vertex.y = kFloat_0_Layout;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = u1;
    vertex.v = v0;
    vertex.x = vertexX;
    vertex.y = kFloat_0_Layout;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = u0;
    vertex.v = v1;
    vertex.x = kFloat_0_Layout;
    vertex.y = vertexY;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = u1;
    vertex.v = v1;
    vertex.x = vertexX;
    vertex.y = vertexY;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    sprite->ActivateAndClearVertexCount();
    dirty = 0;
    return 1;
}

// Rebuild four transformed solid-colour square vertices and clear the dirty flag.
int CSpdSqar::DrawSquare() {
    sprite->BeginVertices(0);
    char transform[0x40];
    char translationMatrix[0x40];
    char rotationMatrix[0x40];
    char scaleMatrix[0x40];
    // Sprite-local XY corners -> pivot-relative -> scaled -> Z-rotated -> positioned.
    // Keep the concat calls in this order; the engine wrapper writes its FIRST argument.
    // Byte buffers are 16-float matrix storage; their declaration order preserves stack layout.
    PSMTX44Identity(transform);
    Mtx44_SetTranslation(transform, -originX, -originY, kFloat_0_Layout);
    Mtx44_SetScale(scaleMatrix, scaleX, scaleY, kFloat_1_Layout);
    PSMTX44Concat(transform, transform, scaleMatrix);
    Mtx44_SetRotationZ(rotationMatrix, rotation);
    PSMTX44Concat(transform, transform, rotationMatrix);
    Mtx44_SetTranslation(translationMatrix, positionX, positionY, kFloat_0_Layout);
    PSMTX44Concat(transform, transform, translationMatrix);
    struct VertexColor {
        float x, y, z;
        unsigned char c0, c1, c2, c3;
        float u, v;
    } vertex;
    vertex.c0 = (unsigned char)(color >> 24);
    vertex.c1 = (unsigned char)(color >> 16);
    vertex.c2 = (unsigned char)(color >> 8);
    vertex.c3 = (unsigned char)color;
    vertex.u = kFloat_0_Layout;
    vertex.v = kFloat_0_Layout;
    vertex.x = kFloat_0_Layout;
    vertex.y = kFloat_0_Layout;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = kFloat_0_Layout;
    vertex.v = kFloat_0_Layout;
    vertex.x = width;
    vertex.y = kFloat_0_Layout;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = kFloat_0_Layout;
    vertex.v = kFloat_0_Layout;
    vertex.x = kFloat_0_Layout;
    vertex.y = height;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    vertex.u = kFloat_0_Layout;
    vertex.v = kFloat_0_Layout;
    vertex.x = width;
    vertex.y = height;
    vertex.z = kFloat_0_Layout;
    Mtx44_TransformPointPerspective(&vertex, &vertex, transform);
    sprite->AppendVertex(&vertex);
    sprite->ActivateAndClearVertexCount();
    dirty = 0;
    return 1;
}

// Initialize an unbound textured sprite with its vtable and cleared sprite, dirty and source state.
CSpdSprite::CSpdSprite() {
    sprite = 0;
    dirty = 0;
    vtbl = __vt__10CSpdSprite;
    source = 0;
}

struct SpdOwnedItem {
    virtual ~SpdOwnedItem();
};

// Delete both item lists and their sprite pool, release list storage, and optionally free the controller.
void* CSpdCtrl::Destroy(int deletingFlag) {
    if (this) {
        vtbl = __vt__8CSpdCtrl;
        void** p = squares.data;
        for (; p != squares.data + *(volatile int*)&squares.count; ++p)
            delete (SpdOwnedItem*)*p;
        p = sprites.data;
        for (; p != sprites.data + *(volatile int*)&sprites.count; ++p)
            delete (SpdOwnedItem*)*p;
        if (spritePool)
            ObjectArray_DeleteAllAndClear(spritePool);
        delete (SpdOwnedItem*)spritePool;
        ((SpdPointerVector*)&squares)->~SpdPointerVector();
        ((SpdPointerVector*)&sprites)->~SpdPointerVector();
        if (deletingFlag > 0)
            __dl__FPv(this);
    }
    return this;
}

// ---- merged from spd_sprite_manager.cpp ----
namespace ns1 {

#pragma dont_inline on
extern "C" {

struct Object {
    char padding00[4];
    unsigned int field04;
    char padding08[8];
    unsigned int* entries;
    unsigned int count;
    char padding18[0x14];
    unsigned int field2C;
};

// Return the resource descriptor flags word without modifying it.
unsigned int SpdManager_GetResourceFlags(Object* self) {
    return self->field2C;
}

// Return the stored logical count from this vector view.
unsigned int Vector_GetCount(Object* self) {
    return self->field04;
}

// Return a borrowed square pointer as a word, or zero when index is out of range.
unsigned int SpdManager_GetSquareAt(Object* self, unsigned int index) {
    if (index >= self->count)
        return 0;
    return self->entries[index];
}

} // extern "C"

#pragma dont_inline on
extern "C" {
struct Item;
struct ItemVTable {
    void (*pad0)();
    void (*pad1)();
    void (*destroy)(Item*, int);
};
struct Item {
    virtual ~Item();
};
struct Array {
    Item** data;
    unsigned int count;
    unsigned int capacity;
};
struct Manager {
    unsigned int vtable;
    Array first;
    Array second;
    void* resource;
    void* context;
    unsigned int field24;
    unsigned int field28;
    unsigned int field2C;
};
struct Input {
    char padding00[0x18];
    unsigned short countFirst;
    unsigned short countSecond;
};
void ObjectArray_DeleteAllAndClear(void*);
void* __nw__FUl(unsigned long);
void* SpdResource_GetSquareEntry(Input*, unsigned int);
void* SpdResource_GetSpriteEntry(Input*, unsigned int);
void* BpSpritePool_Acquire(void*);
void SpdItem_InitializeScaledGeometry(void*, void*, void*);
void SpdItem_InitializeFromResourceEntry(void*, void*, void*, void*);
void SpdSquareVector_Append(Array*, void**);
void SpdSpriteVector_Append(Array*, void**);
unsigned int SpdResource_GetOptionalSection(Input*);
// Constructors, called by symbol: a placement new here would call the operator out of line.

// Return a borrowed sprite pointer, or null when index is out of range.
Item* SpdManager_GetSpriteAt(Manager* self, unsigned int index) {
    if (index >= self->first.count)
        return 0;
    return self->first.data[index];
}
// Delete every square and sprite and clear the pooled sprite resources.
int Manager_DeleteAllItems(Manager* self) {
    for (Item** p = self->second.data; p != self->second.data + self->second.count; ++p) {
        delete *p;
    }
    for (Item** p = self->first.data; p != self->first.data + self->first.count; ++p) {
        delete *p;
    }
    if (self->resource)
        ObjectArray_DeleteAllAndClear(self->resource);
    return 1;
}
// Replace both item lists from the resource, preserving their counts and resource flags.
int SpdManager_BuildItemsFromResource(Manager* self, Input* input, void* context) {
    if (!input)
        return 0;
    // The .spd is big-endian on disc.
    if (!self->resource)
        return 0;
    Manager_DeleteAllItems(self);
    self->context = context;
    void* allocated;
    for (unsigned int i = 0; i < input->countSecond; ++i) {
        allocated = __nw__FUl(0x34);
        if (allocated)
            ((CSpdSqar*)allocated)->__ct();
        void* item = allocated;
        void* data = SpdResource_GetSquareEntry(input, i);
        SpdItem_InitializeScaledGeometry(item, BpSpritePool_Acquire(self->resource), data);
        SpdSquareVector_Append(&self->second, &item);
    }
    self->field24 = Vector_GetCount((Object*)&self->second); // Vector_GetCount.
    for (unsigned int i = 0; i < input->countFirst; ++i) {
        allocated = __nw__FUl(0x54);
        if (allocated)
            ((CSpdSprite*)allocated)->__ct();
        void* item = allocated;
        void* data = SpdResource_GetSpriteEntry(input, i);
        SpdItem_InitializeFromResourceEntry(item, BpSpritePool_Acquire(self->resource), data, self->context);
        SpdSpriteVector_Append(&self->first, &item);
    }
    self->field28 = Vector_GetCount((Object*)&self->first);
    self->field2C = SpdResource_GetOptionalSection(input);
    return 1;
}
}

extern "C" {
extern char vectorLengthExceptionStrings[];
extern char __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12length_error[];
size_t strlen(const char*);
char* strcpy(char*, const char*);
void* memcpy(void*, const void*, unsigned long);
void* memset(void*, int, unsigned long);
void* __nwa__FUl(unsigned long);
void* appMallocDummy__FUl(unsigned long);
void appFreeUni__FPv(void*);
void __dla__FPv(void*, int);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
void __dt__Q33std6detail16char_array_guardFv(void*, int);
void dtor_80046988(void*, int);
void __dt__Q23std12length_errorFv();
void __throw(void*, void*, void (*)()) __attribute__((noreturn));
}
#pragma opt_propagation off
#pragma dont_inline off
struct ErrorBase : std::exception {
    // Install the exception object vtable at its ABI address.
    void SetTable(const void* vt) {
        *(const void**)this = vt;
    }

    char* text;
    void* control;
};
struct BufferGuard {
    char* data;
    char* dirty;
    // Run the retained array-cleanup call when the guard data is non-null.
    ~BufferGuard() {
        if (data)
            __dla__FPv(this, -1);
    }
};
struct Control {
    void* vtable;
    unsigned int refs, weak;
    char* text;
    // Initialize shared diagnostic text ownership and reference counters.
    Control(char* p) : refs(1), weak(1) {
        vtable = s_sharedCharDeleterVtable;
        text = p;
    }
};
namespace std {
struct length_error : ErrorBase {
    // Copy a borrowed diagnostic into shared storage and install the length_error vtable.
    length_error(const char* str) {
        *(void**)this = __vt__Q23std11logic_error;

        struct Flag {
            char value;
            char pad[3];
        };
        Flag dirty;
        dirty.value = 0;
        unsigned long n = strlen(str) + 1;
        char* allocated = (char*)__nwa__FUl(2 * n - n);
        text = allocated;
        {
            BufferGuard guard;
            guard.data = allocated;
            guard.dirty = &dirty.value;
            control = new Control(allocated);
            guard.data = 0;
        }
        strcpy(text, str);
        *(void**)this = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}
#pragma opt_propagation off
// Reallocation owner: data[offset..offset+count) holds constructed pointer slots.
// transfer prepends old slots, clears the old range, and makes offset zero. On commit
// self takes the new buffer; this owner takes the old allocation for destruction.
struct Storage {
    Item** data;
    unsigned int count, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Initialize an empty temporary pointer-buffer owner tied to capacity p.
    Storage(unsigned int* p) : data(0), count(0), capacity(0), owner(p), offset(0) {}
    // Free this owner's current buffer (old buffer after commit, new buffer on unwind).
    ~Storage() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
    // Prepend [first,last) into the temporary, zero old slots and extend constructed count.
    void transfer(Item** first, Item** last) {
        int n = last - first;
        unsigned long bytes = n * 4;
        offset -= n;
        memcpy(data + offset, first, bytes);
        memset(first, 0, bytes);
        *(volatile unsigned int*)&count = count + n;
    }
};
// Select the larger capacity candidate by reference.
inline const unsigned int& maximum(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Construct and throw length_error for the supplied borrowed diagnostic string.
inline void capacityError(const char* message) {
    {
        union {
            int alignment;
            unsigned char bytes[sizeof(std::length_error)];
        } storage;
        std::length_error* error = (std::length_error*)storage.bytes;
        error->__ct(message);
        exception_runtime::__throw(exception_runtime::vectorLengthExceptionStrings + 0x14, error,
                                   (void*)exception_runtime::__dt__Q23std12length_errorFv);
    }
}
#pragma use_lmw_stmw on
// Append a square, growing and transferring the pointer buffer with exception cleanup if necessary.
extern "C" void SpdSquareVector_Append(Array* self, void** item) {
    int count = self->count;

    // Fast path: capacity already covers the new pointer slot.
    if (count < (self->capacity)) {
        unsigned int index = ++self->count;
        Item* value = (Item*)*item;
        self->data[index - 1] = value;
    } else {
        unsigned int need;
        if ((unsigned int)kMaxPointerElements - self->capacity < 1) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }

        unsigned int growth2;

        // Reserve phase: checked growth, with Storage owning any uncommitted allocation.
        Storage storage(&self->capacity);
        __typeof__(self->capacity) capacityBeforeAppend = self->capacity;
        __typeof__(capacityBeforeAppend) existingCapacity = self->capacity;
        need = (1 + self->count) - existingCapacity;
        unsigned int cap = *storage.owner;
        if (need > (unsigned int)kMaxPointerElements - cap) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }
        if (cap < (unsigned int)kThreeFifthsGrowthLimit) {
            unsigned int growth1;
            growth1 = 3 * (1 + cap) / 5;
            cap += maximum(growth1, need);
        } else if (cap < (unsigned int)kHalfGrowthLimit) {
            growth2 = ((1 + cap)) / 2;
            cap += maximum(growth2, need);
        } else
            cap = (unsigned int)kMaxPointerElements;
        if (cap > (unsigned int)kMaxPointerElements) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }
        // Construct the appended slot first, then prepend/transfer the old pointer range.
        Item** allocated = (Item**)appMallocDummy__FUl((cap * 4));
        *(volatile unsigned int*)&storage.offset = self->count;
        storage.data = allocated;
        storage.capacity = cap;
        (allocated + storage.offset)[storage.count] = (Item*)*item;
        storage.count++;
        storage.transfer(self->data, (self->data + self->count));

        *(volatile unsigned int*)&self->count = 0;
        Item** old = self->data;
        __typeof__(self->capacity) oldCapacity = self->capacity;
        // Commit phase: exchange allocation ownership; Storage frees only the old buffer.
        self->capacity = storage.capacity;
        storage.capacity = (oldCapacity);
        self->data = storage.data;
        storage.data = old;
        self->count = storage.count;
        storage.count = 0;
    }
}

// Append a sprite, growing and transferring the pointer buffer with exception cleanup if necessary.
extern "C" void SpdSpriteVector_Append(Array* self, void** item) {
    unsigned int count = self->count;

    // Fast path: capacity already covers the new pointer slot.
    if (count < (self->capacity)) {
        unsigned int index = ++self->count;

        Item* value = (Item*)*item;
        self->data[index - 1] = value;
    } else {
        unsigned int need;
        int growth1;
        if ((unsigned int)kMaxPointerElements - self->capacity < 1) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }

        // Reserve phase: checked growth, with Storage owning any uncommitted allocation.
        Storage storage(&self->capacity);
        need = self->count + 1 - self->capacity;
        unsigned int cap = *storage.owner;
        if (need > (unsigned int)kMaxPointerElements - cap) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }
        if (cap < (unsigned int)kThreeFifthsGrowthLimit) {
            growth1 = 3 * ((cap + 1)) / 5;
            cap += maximum(growth1, need);
        } else if (cap < (unsigned int)kHalfGrowthLimit) {
            unsigned int growth2;
            growth2 = (1 + cap) / 2;
            cap += maximum(growth2, need);
        } else
            cap = (unsigned int)kMaxPointerElements;
        if (cap > (unsigned int)kMaxPointerElements) {
            const char* message = vectorLengthExceptionStrings;
            capacityError(message);
        }
        // Construct the appended slot first, then prepend/transfer the old pointer range.
        Item** allocated = (Item**)appMallocDummy__FUl((cap * 4));
        *(volatile unsigned int*)&storage.offset = self->count;

        (storage.offset + allocated)[storage.count] = (Item*)*item;
        storage.data = allocated;
        storage.capacity = cap;

        storage.count += 1;
        storage.transfer(self->data, (self->count + self->data));
        __typeof__(storage.data) newData = storage.data;
        Item** old = self->data;
        *(volatile unsigned int*)&self->count = 0;
        __typeof__(newData) newDataBeforeSwap = storage.data;
        __typeof__(self->capacity) oldCapacityBeforeSwap = self->capacity;
        int oldCap = oldCapacityBeforeSwap;

        // Commit phase: exchange allocation ownership; Storage frees only the old buffer.
        self->capacity = storage.capacity;
        storage.capacity = oldCap;
        storage.data = old;
        self->count = storage.count;
        self->data = newDataBeforeSwap;
        storage.count = 0;
    }
}

#pragma opt_propagation off
extern "C" {
extern char __vt__10CSpdSprite[];
void BpSprite_GetTextureDescriptor(void*, unsigned int*);
}
struct CloneDispatch {
    virtual ~CloneDispatch();
    virtual void update();
};
struct Clone {
    void* vtable;
    // Dispatch geometry rebuild through the installed clone vtable.
    void update() {
        ((CloneDispatch*)this)->update();
    }
    float rotation;
    unsigned int color;
    float scaleX, scaleY, positionX, positionY, sourceOriginX, sourceOriginY, originX, originY;
    void* resource;
    unsigned int dirty;
    void* context;
    float rectX, rectY, rectZ, rectW;
    float sourceSizeX, sourceSizeY;
    unsigned int flipMode;
    // Initialize the clone wrapper with no backing sprite or context; geometry remains uninitialized.
    Clone() {
        resource = 0;
        dirty = 0;
        *(void**)this = __vt__10CSpdSprite;
        context = 0;
    }
};
struct CloneStorage {
    Item** data;
    unsigned int count, capacity;
    unsigned int* owner;
    unsigned int offset;
    // Initialize an empty clone pointer-buffer owner tied to capacity p.
    CloneStorage(unsigned int* p) : data(0), count(0), capacity(0), owner(p), offset(0) {}
    // Free the allocation currently owned by the clone reallocation temporary.
    ~CloneStorage() {
        if (data) {
            count = 0;
            appFreeUni__FPv(data);
        }
    }
    // Prepend [first,last) into the temporary, zero old slots and extend constructed count.
    void transfer(Item** first, Item** last) {
        unsigned long bytes;
        int n = last - first;
        bytes = n * 4;
        offset -= n;
        memcpy(data + offset, first, bytes);
        memset(first, 0, bytes);
        *(volatile unsigned int*)&count = count + n;
    }
};
} // namespace ns1

// Clone indexed sprite geometry, acquire a backing sprite, and register the owned clone.
unsigned int CSpdCtrl::CloneAndRegister(unsigned int index) {
    using namespace ns1;
    void* textureSource;
    Clone* original;
    if (index >= sprites.count)
        original = 0;
    else
        original = (Clone*)sprites.data[index];
    Clone* cloned = new Clone;
    textureSource = context;
    cloned->resource = BpSpritePool_Acquire(spritePool);
    cloned->context = textureSource;
    if (&cloned->rotation)
        cloned->rotation = original->rotation;
    if (&cloned->color)
        cloned->color = original->color;
    if (&cloned->scaleX) {
        cloned->scaleX = original->scaleX;
        cloned->scaleY = original->scaleY;
    }
    if (&cloned->originX) {
        cloned->originX = original->originX;
        cloned->originY = original->originY;
    }
    if (&cloned->positionX) {
        cloned->positionX = original->positionX;
        cloned->positionY = original->positionY;
    }
    if (&cloned->sourceOriginX) {
        cloned->sourceOriginX = original->sourceOriginX;
        cloned->sourceOriginY = original->sourceOriginY;
    }
    if (&cloned->sourceSizeX) {
        cloned->sourceSizeX = original->sourceSizeX;
        cloned->sourceSizeY = original->sourceSizeY;
    }
    if (&cloned->rectX) {
        cloned->rectX = original->rectX;
        cloned->rectY = original->rectY;
        cloned->rectZ = original->rectZ;
        cloned->rectW = original->rectW;
    }
    cloned->flipMode = original->flipMode;
    float width = kFloat_1_Layout, height = kFloat_1_Layout;
    if (cloned->context) {
        unsigned int dimensions[8];
        BpSprite_GetTextureDescriptor(cloned->context, dimensions);
        width = (float)dimensions[0];
        height = (float)dimensions[1];
    }
    cloned->rectX = cloned->sourceSizeX / width;
    cloned->rectY = cloned->sourceSizeY / height;
    cloned->rectZ = cloned->rectX + cloned->sourceOriginX / width;
    cloned->rectW = cloned->rectY + cloned->sourceOriginY / height;
    cloned->update();
    Array* self = (Array*)&sprites;
    {
        Item* const item = (Item*)cloned;
        unsigned int count = self->count;
        unsigned int capacity = self->capacity;
        if (count < capacity) {
            unsigned int index = ++self->count;
            Item* value = item;
            self->data[index - 1] = value;
        } else {
            if ((unsigned int)kMaxPointerElements - self->capacity < 1) {
                const char* message = vectorLengthExceptionStrings;
                capacityError(message);
            }
            unsigned int growth1;
            unsigned int growth2;
            unsigned int need;
            CloneStorage storage(&self->capacity);
            need = self->count + 1 - self->capacity;
            unsigned int cap = *storage.owner;
            if (need > (unsigned int)kMaxPointerElements - cap) {
                const char* message = vectorLengthExceptionStrings;
                capacityError(message);
            }
            if (cap < (unsigned int)kThreeFifthsGrowthLimit) {
                growth1 = 3 * (cap + 1) / 5;
                cap += maximum(growth1, need);
            } else if (cap < (unsigned int)kHalfGrowthLimit) {
                growth2 = (cap + 1) / 2;
                cap += maximum(growth2, need);
            } else
                cap = (unsigned int)kMaxPointerElements;
            if (cap > (unsigned int)kMaxPointerElements) {
                const char* message = vectorLengthExceptionStrings;
                capacityError(message);
            }
            Item** allocated = (Item**)appMallocDummy__FUl(cap * 4);
            *(volatile unsigned int*)&storage.offset = self->count;
            (allocated + storage.offset)[storage.count] = item;
            storage.data = allocated;
            storage.capacity = cap;
            ++storage.count;
            storage.transfer(self->data, self->data + self->count);
            *(volatile unsigned int*)&self->count = 0;
            Item** old = self->data;
            unsigned int oldCap = self->capacity;
            // Commit phase: exchange allocation ownership; Storage frees only the old buffer.
            self->capacity = storage.capacity;
            storage.capacity = oldCap;
            self->data = storage.data;
            storage.data = old;
            self->count = storage.count;
            storage.count = 0;
        }
    }
    return self->count - 1;
}
