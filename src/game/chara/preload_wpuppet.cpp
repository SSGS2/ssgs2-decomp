// Puppet preloading from a source file: queuing external file references and mesh material
// textures, loading mesh chunks, and the file buffers and int vector helpers.

typedef unsigned long new_size_t;
#include "classes/CPreloadWPuppetAsync.h"
#include "classes/CPreloadWPuppet.h"
#include "classes/CPetFormatExternalFile.h"
#include "classes/CPetFormatMesh.h"
#include "classes/GtlIGenericModelingResource.h"
#include "classes/CPreloadFile.h"

#define AsCPreloadWPuppetAsync(p) ((CPreloadWPuppetAsync*)(p))

extern "C" {
extern int OwnedObjectList_DestroyAll(void*);
extern void* __nwa__FUl(unsigned int);
extern void __dla__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void* Chunk_FindByTag(void*, void*, void*);
extern "C" unsigned int GetChunkElementCount__Q23gtl20IGenericResourceFileFi(void*, unsigned int);
extern "C" void* GetChunkElement__Q23gtl20IGenericResourceFileFii(void*, unsigned int, unsigned int);
extern void* appMallocDummy__FUl(unsigned int);
extern void* __nw__FUl(unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void __throw(const char*, void*, void*) __attribute__((noreturn));
extern unsigned int strlen(const char*);
extern char* strcpy(char*, const char*);
extern void __dt__Q23std12length_errorFv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
extern void dtor_80046988(void*, int);
extern void* Memory_Copy(void*, void*, unsigned short);

extern char __vt__15CPreloadWPuppet[];
struct PuppetChunkResourceStringPool {
    char meshTag[5];
    char vectorLengthError[20];
    char lengthErrorType[55];
    char externalFileTag[5];
    char remainingStrings[0x110 - 0x55];
};
typedef char PuppetChunkResourceStringPool_size[(sizeof(PuppetChunkResourceStringPool) == 0x110) ? 1 : -1];
extern char puppetChunkResourceStrings[];
static const int kExternalFileTagOffset = (int)&((PuppetChunkResourceStringPool*)0)->externalFileTag;
static const int kVectorErrorOffset = (int)&((PuppetChunkResourceStringPool*)0)->vectorLengthError;
static const int kLengthErrorTypeOffset = (int)&((PuppetChunkResourceStringPool*)0)->lengthErrorType;
extern char __vt__Q23std12length_error[];
extern char __vt__Q23std11logic_error[];
extern char s_sharedCharDeleterVtable[];

struct ExternalFileFormat {
    char data[8];
};
struct ExternalFileTable {
    unsigned int header;
    unsigned short count;
};
struct ExternalFileEntry {
    unsigned int nameOffset;
    unsigned short nameLength;
    unsigned short reserved;
};
// Byte access here retains the compiler's local cleanup symbol discriminator.
static const int kExternalNameLengthOffset = (int)&((ExternalFileEntry*)0)->nameLength;
static const int kExternalFileCountOffset = (int)&((ExternalFileTable*)0)->count;
struct RefCountedModelResource {
    int references;
    void** vtable;
};
struct MeshChunkFormat {
    char header[8];
    int loadOptions[7];
    char remainingState[4];
};

// Read EXTF names, terminate each copied name, and queue its external resource.
int CPreloadWPuppet::LoadExternalFileReferences(void* resource, void* fileSize) {
    if (resource == 0)
        return 0;
    void* fileReference;
    char* externalChunk =
        (char*)Chunk_FindByTag(resource, fileSize, puppetChunkResourceStrings + kExternalFileTagOffset);
    if (externalChunk == 0)
        return 1;
    ExternalFileFormat externalFormat;
    ((CPetFormatExternalFile*)&externalFormat)->__ct((int)externalChunk);
    void* fileTable = ((CPetFormatExternalFile*)(&externalFormat))->GetPayload();
    if (*(unsigned short*)((char*)fileTable + kExternalFileCountOffset) == 0) {
        ((CPetFormatExternalFile*)&externalFormat)->~CPetFormatExternalFile();
        return 1;
    }
    fileReference = ((CPetFormatExternalFile*)(&externalFormat))->GetChunkData();
    int referenceIndex = 0;
    while (referenceIndex < *(unsigned short*)((char*)fileTable + kExternalFileCountOffset)) {
        if (*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset) != 0) {
            char* buffer = (char*)((void* (*)(unsigned int))__nwa__FUl)(
                (unsigned int)*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset) + 1);
            Memory_Copy(buffer, externalChunk + *(unsigned int*)fileReference,
                        *(unsigned short*)((char*)fileReference + kExternalNameLengthOffset));
            buffer[*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset)] = 0;
            this->QueueExternalResource(buffer);
            if (buffer != 0)
                __dla__FPv(buffer);
        }
        fileReference = (char*)fileReference + sizeof(ExternalFileEntry);
        ++referenceIndex;
    }
    ((CPetFormatExternalFile*)&externalFormat)->~CPetFormatExternalFile();
    return 1;
}

// Create the MESH modeling resource, queue its material textures, then release the temporary reference.
int CPreloadWPuppet::LoadMeshChunk(void* resource, void* fileSize) {
    if (resource == 0)
        return 0;
    void* entry = Chunk_FindByTag(resource, fileSize, puppetChunkResourceStrings);
    if (entry == 0)
        return 1;
    MeshChunkFormat local;
    ((CPetFormatMesh*)&local)->__ct((int)entry);
    // Clear the seven load-option words before creating the resource.
    local.loadOptions[0] = 0;
    local.loadOptions[1] = 0;
    local.loadOptions[2] = 0;
    local.loadOptions[3] = 0;
    local.loadOptions[4] = 0;
    local.loadOptions[5] = 0;
    local.loadOptions[6] = 0;
    void* list = ((CPetFormatMesh*)(&local))->GetFirstDataWord();
    void* tmp = ((CPetFormatMesh*)(&local))->GetSecondaryData();
    void* out;
    gtl::IGenericModelingResource::CreateFromMemory(tmp, (int)(long)list, &out, local.loadOptions);
    if (out == 0) {
        ((CPetFormatMesh*)&local)->~CPetFormatMesh();
        return 0;
    }
    int result = 0;
    if (QueueMeshMaterialTextures(out))
        result = 1;
    if (out != 0) {
        int refs = --((RefCountedModelResource*)out)->references;
        if (refs == 0) {
            void** vtbl =
                ((RefCountedModelResource*)out)->vtable; // word 0 is the reference count, the class table follows
            ((void (*)(void*, int))vtbl[2])(out, 1);
        }
    }
    out = 0;
    ((CPetFormatMesh*)&local)->~CPetFormatMesh();
    return result;
}

// Minimal layout of an MSL std::vector<int, app_allocator>: {data, size, capacity}. Used here for the puppet's owned
// texture-resource pointers (cast to int) and for the list of texture ids already visited.
struct IntVector {
    int* data;
    unsigned int size;
    unsigned int capacity;
};
// Puppet object view: vtable followed by the list of texture resource pointers it owns (released with the puppet).
struct PuppetPreloadOwnedResources {
    void* vtable;
    IntVector objects;
};
// Two-word view of a buffer owner: heap pointer plus a size that the compiler writes through a volatile store.
struct VolatileOwnedBuffer {
    void* data;
    volatile int size;
};
enum IntVectorCapacityLimits {
    kMaxIntVectorCapacity = 0x3FFFFFFF,
    kIntVectorThreeFifthsLimit = 0x15555555,
    kIntVectorOneHalfLimit = 0x2AAAAAAA
};
enum PuppetChunkTags {
    kTextureNamesChunk = 0x6e786574, // "texn" in the resource's tag byte order
    kModelChunk = 0x006c646d,
    kStringDataChunk = 0x00646273,
    kMaterialsChunk = 0x6c72746d,
    kMeshChunk = 0x0068736d,
    kPrimitiveChunk = 0x006d7270,
    kNoTexture = 0xff
};
struct PuppetModelChunk {
    unsigned short meshCount, firstMesh;
};
struct PuppetMeshChunk {
    unsigned int firstPrimitive, primitiveCount;
};
struct PuppetPrimitiveChunk {
    unsigned int header;
    unsigned short materialIndex;
};
struct PuppetMaterial {
    char renderState[12];
    unsigned char textureIds[4];
    char remainingState[12];
};
static const int kMaterialTextureIdsOffset = (int)&((PuppetMaterial*)0)->textureIds;
struct PuppetTextureName {
    unsigned int header, stringOffset;
};

// The next five types reproduce, step by step, the inlined construction of "throw std::length_error(vector length
// error)": copy the message into a heap string owned by a ref-counted control block, then build the exception.
// Reading tip: ThrowLengthError() below is the only thing callers need to know about.
struct ThrowAllocation {
    char* pointer;
    char* allocator;
    ThrowAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    ~ThrowAllocation() {
        if (pointer)
            __dla__FPv(pointer);
    }
};
struct ThrowControlBase {
    void* table;
    unsigned int strong, weak;
    ThrowControlBase() {
        strong = 1;
        weak = 1;
        table = (void*)s_sharedCharDeleterVtable;
    }
};
struct ThrowControl : ThrowControlBase {
    char* pointer;
    ThrowControl(char* p) {
        pointer = p;
    }
};
struct ThrowErrorBase {
    void* table;
    ThrowErrorBase() {
        table = (void*)__vt__Q23std11logic_error;
    }
    ~ThrowErrorBase() {
        __dt__Q23std9exceptionFv(this, -1);
    }
};
namespace std {
struct length_error : ThrowErrorBase {
    char* text;
    void* control;
    length_error() {
        const char* message = puppetChunkResourceStrings + kVectorErrorOffset;
        struct Allocator {
            char value;
            char pad[3];
        } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)((void* (*)(unsigned long))__nwa__FUl)(2 * n - n);
        {
            char* p = text;
            ThrowAllocation allocation(p, &allocator.value);
            control = new ThrowControl(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = (void*)__vt__Q23std12length_error;
    }
    ~length_error() {
        dtor_80046988(this, -1);
    }
};
}
inline void* operator new(new_size_t, void* p) {
    return p;
}

struct VecGrowScratch {
    int* data;
    unsigned int size;
    unsigned int capacity;
    unsigned int* owner;
    volatile unsigned int offset;
    VecGrowScratch(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    ~VecGrowScratch() {
        if (data) {
            size = 0;
            appFreeUni__FPv(data);
        }
    }
};
inline const unsigned int& MaxUnsigned(const unsigned int& a, const unsigned int& b) {
    return a < b ? b : a;
}
// Throws std::length_error("vector length error"); the exception object lives in a 12-byte stack slot.
inline void ThrowLengthError() {
    unsigned int storage[3];
    std::length_error* error = new (storage) std::length_error();
    __throw(puppetChunkResourceStrings + kLengthErrorTypeOffset, error, (void*)__dt__Q23std12length_errorFv);
}

// std::vector<int>::push_back as the compiler inlined it. Fast path: room left, store and bump size.
// Slow path: grow by max(needed, 3/5 of capacity) while capacity < max/3, by max(needed, 1/2) while < 2*max/3,
// otherwise to the maximum (throwing length_error on overflow); allocate, place the new element, copy the old
// elements, zero the old block, then swap buffers (the scratch object frees the old block on scope exit).
inline void VecPushBack(IntVector* v, int value) {
    unsigned int size = v->size;

    if (v->capacity > v->size) {
        unsigned int next = size + 1;
        v->size = next;
        v->data[next - 1] = value;
    } else {
        if (kMaxIntVectorCapacity - v->capacity < 1)
            ThrowLengthError();
        unsigned int needed = v->size + 1 - v->capacity;
        VecGrowScratch temp(&v->capacity);
        unsigned int capacity = v->capacity;
        if (needed > kMaxIntVectorCapacity - capacity)
            ThrowLengthError();
        if (capacity < kIntVectorThreeFifthsLimit) {
            unsigned int t = (3 * (capacity + 1)) / 5;
            capacity += MaxUnsigned(t, needed);
        } else if (capacity < kIntVectorOneHalfLimit) {
            unsigned int t2 = (capacity + 1) >> 1;
            capacity += MaxUnsigned(t2, needed);
        } else {
            capacity = kMaxIntVectorCapacity;
        }
        if (capacity > kMaxIntVectorCapacity)
            ThrowLengthError();
        int* data = (int*)appMallocDummy__FUl(capacity * 4);
        temp.data = data;
        unsigned int oldSize = v->size;
        temp.offset = oldSize;
        *(int*)((char*)(data + oldSize) + temp.size * 4) = value;
        int* oldData = v->data;
        temp.capacity = capacity;
        temp.size = temp.size + 1;
        unsigned int count = (unsigned int)(((char*)(oldSize + oldData) - (char*)oldData) / 4);
        unsigned int offset = oldSize - count;
        temp.offset = offset;
        memcpy(data + offset, oldData, count * 4);
        memset(oldData, 0, count * 4);
        unsigned int newSize = count + temp.size;
        temp.size = newSize;
        v->size = 0;
        int* newData = temp.data;
        unsigned int newCapacity = temp.capacity;
        unsigned int oldCapacity = v->capacity;
        temp.data = oldData;
        v->capacity = newCapacity;
        temp.capacity = oldCapacity;
        v->data = newData;
        v->size = newSize;
        temp.size = 0;
    }
}

#pragma push
#pragma use_lmw_stmw on
// Visit model meshes and primitives, loading each referenced material texture only once.
int CPreloadWPuppet::QueueMeshMaterialTextures(void* resource) {
    if (resource == 0)
        return 0;
    if (GetChunkElementCount__Q23gtl20IGenericResourceFileFi(resource, kTextureNamesChunk) == 0)
        return 0;
    unsigned int modelCount = (unsigned int)GetChunkElementCount__Q23gtl20IGenericResourceFileFi(resource, kModelChunk);
    unsigned int stringDataAddress =
        (unsigned int)GetChunkElement__Q23gtl20IGenericResourceFileFii(resource, kStringDataChunk, 0);
    PuppetMaterial* materials =
        (PuppetMaterial*)GetChunkElement__Q23gtl20IGenericResourceFileFii(resource, kMaterialsChunk, 0);
    IntVector* objects = &((PuppetPreloadOwnedResources*)this)->objects;
    IntVector indices = {0, 0, 0};
    struct IndexCleanup {
        IntVector* value;
        IndexCleanup(IntVector* indices) : value(indices) {}
        ~IndexCleanup() {
            if (value->data) {
                value->size = 0;
                appFreeUni__FPv(value->data);
            }
        }
    } cleanup(&indices);
    // For every model -> mesh -> primitive, look up the primitive's material and request each of its (up to 4)
    // textures once; "indices" records texture ids already requested, "objects" collects the resulting resources.
    for (unsigned int modelIndex = 0; modelIndex < modelCount; ++modelIndex) {
        PuppetModelChunk* model =
            (PuppetModelChunk*)GetChunkElement__Q23gtl20IGenericResourceFileFii(resource, kModelChunk, modelIndex);
        for (int meshIndex = 0; meshIndex < (int)model->meshCount; ++meshIndex) {
            PuppetMeshChunk* mesh = (PuppetMeshChunk*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
                resource, kMeshChunk, model->firstMesh + meshIndex);
            for (unsigned int primitiveIndex = 0; primitiveIndex < mesh->primitiveCount; ++primitiveIndex) {
                PuppetPrimitiveChunk* primitive =
                    (PuppetPrimitiveChunk*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
                        resource, kPrimitiveChunk, mesh->firstPrimitive + primitiveIndex);
                for (int textureSlot = 0; textureSlot < 4; ++textureSlot) {
                    unsigned char textureId =
                        ((unsigned char*)materials)[primitive->materialIndex * sizeof(PuppetMaterial) +
                                                    kMaterialTextureIdsOffset + textureSlot];
                    if (textureId == kNoTexture)
                        continue;
                    unsigned int* seenTexture = (unsigned int*)indices.data;
                    unsigned int* seenTexturesEnd = seenTexture + indices.size;
                    while (seenTexture != seenTexturesEnd && textureId != *seenTexture)
                        ++seenTexture;
                    if (seenTexture != seenTexturesEnd)
                        continue;
                    PuppetTextureName* textureName =
                        (PuppetTextureName*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
                            resource, kTextureNamesChunk, textureId);
                    unsigned int textureNameAddress = stringDataAddress + textureName->stringOffset;
                    // Virtual slot 9 of the puppet resolves a texture name to a resource (0 = not available).
                    void* textureResource =
                        ((void* (*)(void*, unsigned int))(*(void***)this)[9])(this, textureNameAddress);
                    if (textureResource == 0)
                        continue;
                    VecPushBack(objects, (int)textureResource);
                    VecPushBack(&indices, (int)textureId);
                }
            }
        }
    }
    return 1;
}
#pragma pop

struct PreloadFilePollInterface {
    virtual void v0();
    virtual void v1();
    virtual int pollReady();
    virtual void v3();
    virtual int v4();
};

struct PreloadFileDeleteInterface {
    virtual void destroy(int deleting);
};

struct PuppetFileOpenInterface {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void v3();
    virtual void* openFile(void* request);
};

// Release the previous source file and open a replacement through the preload virtual hook.
int CPreloadWPuppet::OpenSourceFile(void* arg) {
    void* p = AsCPreloadWPuppetAsync(this)->sourceFile;
    if (p != 0) {
        // (the repeated null test is inlined owner cleanup)
        if (p != 0)
            ((PreloadFileDeleteInterface*)p)->destroy(1);
        AsCPreloadWPuppetAsync(this)->sourceFile = 0;
    }
    p = ((PuppetFileOpenInterface*)this)->openFile(arg);
    AsCPreloadWPuppetAsync(this)->sourceFile = p;
    if (p == 0)
        return 0;
    return 1;
}

// Release the owned source file, destroy the preload base, and optionally free the object.
CPreloadWPuppet::~CPreloadWPuppet() {
    *(void**)this = (void*)__vt__15CPreloadWPuppet;
    void* p = file;
    if (p != 0) {
        if (p != 0)
            ((PreloadFileDeleteInterface*)p)->destroy(1);
        file = 0;
    }
}

// Read the entire source file, queue mesh and external resources, then free the temporary file buffer.
int CPreloadWPuppet::LoadFileResources() {
    if (OwnedObjectList_DestroyAll(this) == 0)
        return 0;
    if (file == 0)
        return 0;
    // The existing file API returns its byte count through a pointer-sized value.
    void* fileSize = ((CPreloadFile*)file)->GetLoadedData();
    void* fileBuffer = __nwa__FUl((unsigned int)fileSize);
    if (fileBuffer == 0)
        return 0;
    void* sourceFile = file;
    int result = 0;
    ((CPreloadFile*)sourceFile)->Read(fileBuffer, fileSize);
    // Model files are big-endian on the Wii.
    if (LoadMeshChunk(fileBuffer, fileSize) && LoadExternalFileReferences(fileBuffer, fileSize))
        result = 1;
    if (fileBuffer != 0)
        __dla__FPv(fileBuffer);
    return result;
}

// Poll the owned file for readiness; an absent file is already ready.
int CPreloadWPuppet::PollFileReady() {
    PreloadFilePollInterface* p = (PreloadFilePollInterface*)file;
    if (p != 0) {
        return p->pollReady();
    }
    return 1;
}

// Construct the preload base and initialize an empty source-file handle.
CPreloadWPuppet::CPreloadWPuppet() {
    (*(int*)&file) = 0;
    *(void**)this = (void*)__vt__15CPreloadWPuppet;
}

// Free owned vector storage, clear its volatile size, and optionally delete the wrapper.
// Repeated base-destructor guards retain the original generated branch layout.
void* VolatileOwnedBuffer_dtor(void* self, int flags) {
    // The stacked identical "if (self != 0)" tests are the inlined base-class destructor chain; in plain C this is
    // "if (self) { free(self->data); self->size = 0; }".
    if (self != 0) {
        if (self != 0) {
            if (self != 0) {
                if (self != 0) {
                    if (self != 0) {
                        {
                            if (*(void**)self != 0) {
                                void* allocation = *(void**)self;
                                int oldSize = ((VolatileOwnedBuffer*)self)->size;
                                ((VolatileOwnedBuffer*)self)->size = oldSize - oldSize;
                                appFreeUni__FPv(allocation);
                            }
                        }
                    }
                }
            }
        }
    }
    if (self != 0 && flags > 0) {
        __dl__FPv(self);
    }
    return self;
}
} // extern "C"
