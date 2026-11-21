typedef unsigned long new_size_t;
#include "classes/CPreloadWPuppetAsync.h"
#include "classes/CPreloadWPuppet.h"

#define AsCPreloadWPuppet(p) ((CPreloadWPuppet*)(p))

#define AsCPreloadWPuppetAsync(p) ((CPreloadWPuppetAsync*)(p))
// Translation unit 0x80270828-0x80271734 (9 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
extern void CPreloadParent_Ctor(void*);
extern void CPreloadParent_dtor(void*, int);
extern int OwnedObjectList_DestroyAll(void*);
extern void* CPreloadFile_GetLoadedData(void*);
extern void* __nwa__FUl(unsigned int);
extern void CPreloadFile_Read(void*, void*, void*);
extern int CPreloadWPuppet_LoadMeshChunk(void*, void*, void*);
extern int CPreloadWPuppet_LoadExternalFileReferences(void*, void*, void*);
extern void __dla__FPv(void*);
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void* Chunk_FindByTag(void*, void*, void*);
extern void* CPetFormatExternalFile_Ctor(void*, void*);
extern void* CPetFormatExternalFile_GetPayload(void*);
extern void* CPetFormatExternalFile_GetChunkData(void*);
extern void CPetFormatExternalFile_dtor(void*, int);
extern void* CPetFormatMesh_Ctor(void*, void*);
extern void* CPetFormatMesh_GetFirstDataWord(void*);
extern void* CPetFormatMesh_GetSecondaryData(void*);
extern void CPetFormatMesh_dtor(void*, int);
extern void* gtl_IGenericModelingResource_CreateFromMemory(void*, void*, void*, void*);
extern int CPreloadWPuppet_QueueMeshMaterialTextures(void*, void*);
extern unsigned int gtl_IGenericResourceFile_GetChunkElementCount(void*, unsigned int);
extern void* gtl_IGenericResourceFile_GetChunkElement(void*, unsigned int, unsigned int);
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
extern void CPreloadWPuppet_QueueExternalResource(void*, char*);
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

struct ExternalFileFormat { char data[8]; };
struct ExternalFileTable { unsigned int header; unsigned short count; };
struct ExternalFileEntry { unsigned int nameOffset; unsigned short nameLength; unsigned short reserved; };
// Byte access here retains the compiler's local cleanup symbol discriminator.
static const int kExternalNameLengthOffset = (int)&((ExternalFileEntry*)0)->nameLength;
static const int kExternalFileCountOffset = (int)&((ExternalFileTable*)0)->count;
struct RefCountedModelResource { int references; void** vtable; };
struct MeshChunkFormat {
    char header[8];
    int loadOptions[7];
    char remainingState[4];
};

// Read EXTF names, terminate each copied name, and queue its external resource.
int CPreloadWPuppet_LoadExternalFileReferences(void* self, void* resource, void* fileSize) {
    if (resource == 0) return 0;
    void* fileReference;
    char* externalChunk = (char*)Chunk_FindByTag(resource, fileSize, puppetChunkResourceStrings + kExternalFileTagOffset);
    if (externalChunk == 0) return 1;
    ExternalFileFormat externalFormat;
    CPetFormatExternalFile_Ctor(&externalFormat, externalChunk);
    void* fileTable = CPetFormatExternalFile_GetPayload(&externalFormat);
    if (*(unsigned short*)((char*)fileTable + kExternalFileCountOffset) == 0) {
        CPetFormatExternalFile_dtor(&externalFormat, -1);
        return 1;
    }
    fileReference = CPetFormatExternalFile_GetChunkData(&externalFormat);
    int referenceIndex = 0;
    while (referenceIndex < *(unsigned short*)((char*)fileTable + kExternalFileCountOffset)) {
        if (*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset) != 0) {
            char* buffer = (char*)((void* (*)(unsigned int))__nwa__FUl)((unsigned int)*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset) + 1);
            Memory_Copy(buffer, externalChunk + *(unsigned int*)fileReference, *(unsigned short*)((char*)fileReference + kExternalNameLengthOffset));
            buffer[*(unsigned short*)((char*)fileReference + kExternalNameLengthOffset)] = 0;
            CPreloadWPuppet_QueueExternalResource(self, buffer);
            if (buffer != 0) __dla__FPv(buffer);
        }
        fileReference = (char*)fileReference + sizeof(ExternalFileEntry);
        ++referenceIndex;
    }
    CPetFormatExternalFile_dtor(&externalFormat, -1);
    return 1;
}

// Create the MESH modeling resource, queue its material textures, then release the temporary reference.
int CPreloadWPuppet_LoadMeshChunk(void* self, void* resource, void* fileSize) {
    if (resource == 0) return 0;
    void* entry = Chunk_FindByTag(resource, fileSize, puppetChunkResourceStrings);
    if (entry == 0) return 1;
    MeshChunkFormat local;
    CPetFormatMesh_Ctor(&local, entry);
    // Clear the seven load-option words before creating the resource.
    local.loadOptions[0] = 0;
    local.loadOptions[1] = 0;
    local.loadOptions[2] = 0;
    local.loadOptions[3] = 0;
    local.loadOptions[4] = 0;
    local.loadOptions[5] = 0;
    local.loadOptions[6] = 0;
    void* list = CPetFormatMesh_GetFirstDataWord(&local);
    void* tmp = CPetFormatMesh_GetSecondaryData(&local);
    void* out;
    gtl_IGenericModelingResource_CreateFromMemory(tmp, list, &out, local.loadOptions);
    if (out == 0) {
        CPetFormatMesh_dtor(&local, -1);
        return 0;
    }
    int result = 0;
    if (CPreloadWPuppet_QueueMeshMaterialTextures(self, out)) result = 1;
    if (out != 0) {
        int refs = --((RefCountedModelResource*)out)->references;
        if (refs == 0) {
            void** vtbl = ((RefCountedModelResource*)out)->vtable; // word 0 is the reference count, the class table follows
            ((void (*)(void*, int))vtbl[2])(out, 1);
        }
    }
    out = 0;
    CPetFormatMesh_dtor(&local, -1);
    return result;
}

// Minimal layout of an MSL std::vector<int, app_allocator>: {data, size, capacity}. Used here for the puppet's owned
// texture-resource pointers (cast to int) and for the list of texture ids already visited.
struct IntVector { int* data; unsigned int size; unsigned int capacity; };
// Puppet object view: vtable followed by the list of texture resource pointers it owns (released with the puppet).
struct PuppetPreloadOwnedResources { void* vtable; IntVector objects; };
// Two-word view of a buffer owner: heap pointer plus a size that the compiler writes through a volatile store.
struct VolatileOwnedBuffer { void* data; volatile int size; };
enum IntVectorCapacityLimits {
    kMaxIntVectorCapacity = 0x3FFFFFFF,
    kIntVectorThreeFifthsLimit = 0x15555555,
    kIntVectorOneHalfLimit = 0x2AAAAAAA
};
enum PuppetChunkTags {
    kTextureNamesChunk = 0x6e786574, // "texn" in the resource's tag byte order
    kModelChunk = 0x006c646d, kStringDataChunk = 0x00646273,
    kMaterialsChunk = 0x6c72746d, kMeshChunk = 0x0068736d,
    kPrimitiveChunk = 0x006d7270, kNoTexture = 0xff
};
struct PuppetModelChunk { unsigned short meshCount, firstMesh; };
struct PuppetMeshChunk { unsigned int firstPrimitive, primitiveCount; };
struct PuppetPrimitiveChunk { unsigned int header; unsigned short materialIndex; };
struct PuppetMaterial { char renderState[12]; unsigned char textureIds[4]; char remainingState[12]; };
static const int kMaterialTextureIdsOffset = (int)&((PuppetMaterial*)0)->textureIds;
struct PuppetTextureName { unsigned int header, stringOffset; };

// The next five types reproduce, step by step, the inlined construction of "throw std::length_error(vector length
// error)": copy the message into a heap string owned by a ref-counted control block, then build the exception.
// Reading tip: ThrowLengthError() below is the only thing callers need to know about.
struct ThrowAllocation {
    char* pointer; char* allocator;
    ThrowAllocation(char* p, char* a) : pointer(p), allocator(a) {}
    ~ThrowAllocation() { if (pointer) __dla__FPv(pointer); }
};
struct ThrowControlBase {
    void* table; unsigned int strong, weak;
    ThrowControlBase() { strong = 1; weak = 1; table = (void*)s_sharedCharDeleterVtable; }
};
struct ThrowControl : ThrowControlBase {
    char* pointer;
    ThrowControl(char* p) { pointer = p; }
};
struct ThrowErrorBase {
    void* table;
    ThrowErrorBase() { table = (void*)__vt__Q23std11logic_error; }
    ~ThrowErrorBase() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : ThrowErrorBase {
    char* text; void* control;
    length_error() {
        const char* message = puppetChunkResourceStrings + kVectorErrorOffset;
        struct Allocator { char value; char pad[3]; } allocator;
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
    ~length_error() { dtor_80046988(this, -1); }
};
}
inline void* operator new(new_size_t, void* p) { return p; }

struct VecGrowScratch {
    int* data; unsigned int size; unsigned int capacity;
    unsigned int* owner; volatile unsigned int offset;
    VecGrowScratch(unsigned int* p) : data(0), size(0), capacity(0), owner(p), offset(0) {}
    ~VecGrowScratch() { if (data) { size = 0; appFreeUni__FPv(data); } }
};
inline const unsigned int& MaxUnsigned(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }
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
        if (kMaxIntVectorCapacity - v->capacity < 1) ThrowLengthError();
        unsigned int needed = v->size + 1 - v->capacity;
        VecGrowScratch temp(&v->capacity);
        unsigned int capacity = v->capacity;
        if (needed > kMaxIntVectorCapacity - capacity) ThrowLengthError();
        if (capacity < kIntVectorThreeFifthsLimit) {
            unsigned int t = (3 * (capacity + 1)) / 5;
            capacity += MaxUnsigned(t, needed);
        } else if (capacity < kIntVectorOneHalfLimit) {
            unsigned int t2 = (capacity + 1) >> 1;
            capacity += MaxUnsigned(t2, needed);
        } else {
            capacity = kMaxIntVectorCapacity;
        }
        if (capacity > kMaxIntVectorCapacity) ThrowLengthError();
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
int CPreloadWPuppet_QueueMeshMaterialTextures(void* self, void* resource) {
    if (resource == 0) return 0;
    if (gtl_IGenericResourceFile_GetChunkElementCount(resource, kTextureNamesChunk) == 0) return 0;
    unsigned int modelCount = (unsigned int)gtl_IGenericResourceFile_GetChunkElementCount(resource, kModelChunk);
    unsigned int stringDataAddress = (unsigned int)gtl_IGenericResourceFile_GetChunkElement(resource, kStringDataChunk, 0);
    PuppetMaterial* materials = (PuppetMaterial*)gtl_IGenericResourceFile_GetChunkElement(resource, kMaterialsChunk, 0);
    IntVector* objects = &((PuppetPreloadOwnedResources*)self)->objects;
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
        PuppetModelChunk* model = (PuppetModelChunk*)gtl_IGenericResourceFile_GetChunkElement(resource, kModelChunk, modelIndex);
        for (int meshIndex = 0; meshIndex < (int)model->meshCount; ++meshIndex) {
            PuppetMeshChunk* mesh = (PuppetMeshChunk*)gtl_IGenericResourceFile_GetChunkElement(resource, kMeshChunk, model->firstMesh + meshIndex);
            for (unsigned int primitiveIndex = 0; primitiveIndex < mesh->primitiveCount; ++primitiveIndex) {
                PuppetPrimitiveChunk* primitive = (PuppetPrimitiveChunk*)gtl_IGenericResourceFile_GetChunkElement(resource, kPrimitiveChunk, mesh->firstPrimitive + primitiveIndex);
                for (int textureSlot = 0; textureSlot < 4; ++textureSlot) {
                    unsigned char textureId = ((unsigned char*)materials)[primitive->materialIndex * sizeof(PuppetMaterial) + kMaterialTextureIdsOffset + textureSlot];
                    if (textureId == kNoTexture) continue;
                    unsigned int* seenTexture = (unsigned int*)indices.data;
                    unsigned int* seenTexturesEnd = seenTexture + indices.size;
                    while (seenTexture != seenTexturesEnd && textureId != *seenTexture) ++seenTexture;
                    if (seenTexture != seenTexturesEnd) continue;
                    PuppetTextureName* textureName = (PuppetTextureName*)gtl_IGenericResourceFile_GetChunkElement(resource, kTextureNamesChunk, textureId);
                    unsigned int textureNameAddress = stringDataAddress + textureName->stringOffset;
                    // Virtual slot 9 of the puppet resolves a texture name to a resource (0 = not available).
                    void* textureResource = ((void* (*)(void*, unsigned int))(*(void***)self)[9])(self, textureNameAddress);
                    if (textureResource == 0) continue;
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
int CPreloadWPuppet_OpenSourceFile(void* self, void* arg) {
    void* p = AsCPreloadWPuppetAsync(self)->sourceFile;
    if (p != 0) {
        // (the repeated null test is inlined owner cleanup)
        if (p != 0) ((PreloadFileDeleteInterface*)p)->destroy(1);
        AsCPreloadWPuppetAsync(self)->sourceFile = 0;
    }
    p = ((PuppetFileOpenInterface*)self)->openFile(arg);
    AsCPreloadWPuppetAsync(self)->sourceFile = p;
    if (p == 0) return 0;
    return 1;
}

// Release the owned source file, destroy the preload base, and optionally free the object.
void* CPreloadWPuppet_dtor(void* self, int flags) {
    if (self != 0) {
        *(void**)self = (void*)__vt__15CPreloadWPuppet;
        void* p = AsCPreloadWPuppet(self)->file;
        if (p != 0) {
            if (p != 0) ((PreloadFileDeleteInterface*)p)->destroy(1);
            AsCPreloadWPuppet(self)->file = 0;
        }
        CPreloadParent_dtor(self, 0);
        if (flags > 0) __dl__FPv(self);
    }
    return self;
}

// Read the entire source file, queue mesh and external resources, then free the temporary file buffer.
int CPreloadWPuppet_LoadFileResources(void* self) {
    if (OwnedObjectList_DestroyAll(self) == 0) return 0;
    if (((CPreloadWPuppet*)self)->file == 0) return 0;
    // The existing file API returns its byte count through a pointer-sized value.
    void* fileSize = CPreloadFile_GetLoadedData(((CPreloadWPuppet*)self)->file);
    void* fileBuffer = __nwa__FUl((unsigned int)fileSize);
    if (fileBuffer == 0) return 0;
    void* sourceFile = ((CPreloadWPuppet*)self)->file;
    int result = 0;
    CPreloadFile_Read(sourceFile, fileBuffer, fileSize);
// Model files are big-endian on the Wii.
    if (CPreloadWPuppet_LoadMeshChunk(self, fileBuffer, fileSize) && CPreloadWPuppet_LoadExternalFileReferences(self, fileBuffer, fileSize)) result = 1;
    if (fileBuffer != 0) __dla__FPv(fileBuffer);
    return result;
}

// Poll the owned file for readiness; an absent file is already ready.
int CPreloadWPuppet_PollFileReady(void* self) {
    PreloadFilePollInterface* p = (PreloadFilePollInterface*)((CPreloadWPuppet*)self)->file;
    if (p != 0) {
        return p->pollReady();
    }
    return 1;
}

// Construct the preload base and initialize an empty source-file handle.
void* CPreloadWPuppet_Ctor(void* self) {
    CPreloadParent_Ctor(self);
    (*(int*)&AsCPreloadWPuppet(self)->file) = 0;
    *(void**)self = (void*)__vt__15CPreloadWPuppet;
    return self;
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
