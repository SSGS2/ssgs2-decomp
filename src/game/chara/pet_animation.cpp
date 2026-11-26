// CPetAnimation: a reference-counted table of named animation entries. Each entry
// pairs a mode with a name and a shared animation resource. This file loads the
// table from a model chunk, looks entries up by name and evaluates their resources.
#include "classes/CPetAnimation.h"

typedef unsigned long new_size_t;

#pragma use_lmw_stmw on

enum {
    kSceneChunk = 0x6E6373,         // 'scn'
    kMaterialPatternChunk = 0x7074, // 'pt'
    kArchiveReadMode = 3,
    kArrayCookieSize = 0x10 // header the array allocator puts before an array
};

// A 4x4 matrix of words, copied as a block.
struct PetMaterialMatrix {
    unsigned words[16];
};
// The first scene chunk element: a frame range.
struct SceneFrameRange {
    unsigned chunkHeader;
    float start;
    float end;
};
struct SceneScalar {
    char pad[8];
    float value;
};

extern "C" {
struct AnimationResourceRef;
struct AnimationResourceVtable {
    void* a;
    void* b;
    void (*dtor)(AnimationResourceRef*, int);
};
// A reference-counted animation resource.
struct AnimationResourceRef {
    int refs;
    AnimationResourceVtable* vtable;
};
// Reached through its virtual destructor when the last reference is dropped.
struct RefObject {
    int refs;
    virtual ~RefObject();
};

void __dl__FPv(void*);
void __dla__FPv(void*);
void* __nwa__FUl(unsigned);
int strcmp(const char*, const char*);
int strlen(const char*);
void Memory_Copy(char*, const char*, unsigned);
void Memory_Clear(void*, unsigned);
extern char __vt__13CPetAnimation[];

extern "C" int EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(AnimationResourceRef*, int, int, float);
extern "C" int EvaluateMaterials__Q23gtl25IGenericAnimationResourceCFPvf(float, AnimationResourceRef*,
                                                                         PetMaterialMatrix*, int);
extern "C" void CreateFromMemory__Q23gtl25IGenericAnimationResourceFPviPPv(void*, unsigned, AnimationResourceRef**);
extern "C" void* GetChunkElement__Q23gtl20IGenericResourceFileFii(AnimationResourceRef*, unsigned, int);
extern "C" unsigned GetChunkElementCount__Q23gtl20IGenericResourceFileFi(AnimationResourceRef*, unsigned);
void* Archive_Open(void*, unsigned, int);
void Archive_GetEntryInfo(void*, int, void*);
void Archive_ReadEntry(void*, int, void*, unsigned, int);
void Archive_Close(void*);
void IPetMesh_ScaleResource(void*, float);

extern const float kFloat_0_PetAnimation, kFloat_0_32_PetAnimation, kFloat_30_PetAnimation;
}

#define kZeroDuration kFloat_0_PetAnimation
#define kResourceScale kFloat_0_32_PetAnimation // scale applied to each resource as it is loaded
#define kFramesPerSecond kFloat_30_PetAnimation

// One named animation. Named Entry because its constructor is a separate symbol
// of the original program.
struct Entry {
    Entry();
    static void* operator new[](new_size_t size);
    unsigned short mode;
    char pad_002[2];
    char* name;
    AnimationResourceRef* resource;
};

inline void* Entry::operator new[](new_size_t size) {
    return __nwa__FUl(size);
}

// Chunk layout: a header, then `count` source entries starting at `offset`. Every
// offset inside an entry is relative to the start of the chunk.
struct PetAnimationChunk {
    unsigned chunkHeader[2];
    unsigned offset;
    unsigned short count;
    unsigned char packed; // the entry data is stored in an archive
};
struct PetAnimationSourceEntry {
    unsigned size;
    unsigned data;
    unsigned name;
    unsigned short mode;
    unsigned short length;
};

// Drops one reference to an entry's resource and deletes it at zero.
static inline void ReleaseResource(Entry* entry) {
    AnimationResourceRef* resource = entry->resource;
    if (resource) {
        if (--resource->refs == 0) {
            delete (RefObject*)resource;
        }
        entry->resource = 0;
    }
}

// Finds the entry with the given name, or null.
static inline Entry* FindEntry(CPetAnimation* self, const char* key) {
    if (!key)
        return 0;
    unsigned entryCount = self->entryCount;
    int entryByteOffset;
    unsigned i = 0;
    entryByteOffset = 0;
    for (; i < entryCount; ++i) {
        Entry* entry;
        if (!self->entries)
            entry = 0;
        else if (entryCount <= i)
            entry = 0;
        else
            entry = (Entry*)((char*)self->entries + entryByteOffset);
        if (!strcmp(key, entry->name))
            return entry;
        entryByteOffset += sizeof(Entry);
    }
    return 0;
}

// The functions of this unit are 4-byte aligned.
#pragma function_align 4
// Restore the vtable and release the entries.
CPetAnimation::~CPetAnimation() {
    vtbl = __vt__13CPetAnimation;
    Clear();
}

// Build named entries, sharing duplicate names and optionally unpacking archive data.
// Only newly created resources add to the returned duration.
float CPetAnimation::LoadEntriesFromChunk(PetAnimationChunk* input) {
    float totalDuration = kZeroDuration;
    entryCount = input->count;
    if (!entryCount)
        return totalDuration;
    entries = new Entry[entryCount];
    PetAnimationSourceEntry* source;
    int offset;
    Entry* entry;
    int i = 0;
    offset = 0;
    source = (PetAnimationSourceEntry*)((char*)input + input->offset);
    for (; i < entryCount; ++source, offset += sizeof(Entry), ++i) {
        entry = (Entry*)((char*)entries + offset);
        entry->resource = 0;
        entry->mode = source->mode;
        entry->name = (char*)__nwa__FUl(source->length + 1);
        Memory_Clear(entry->name, source->length + 1);
        Memory_Copy(entry->name, (char*)input + source->name, source->length);
        int reusedResource = 0;
        for (int j = i - 1; j >= 0; --j) {
            Entry* prior = &entries[j];
            if (!strcmp(prior->name, entry->name)) {
                ReleaseResource(entry);
                entry->resource = prior->resource;
                ++entry->resource->refs;
                reusedResource = 1;
                break;
            }
        }
        if (!reusedResource) {
            ReleaseResource(entry);
            if (source->size) {
                void* data = 0;
                unsigned size = 0;
                void* embeddedData = (char*)input + source->data;
                if (input->packed) {
                    void* archive = Archive_Open(embeddedData, source->size, kArchiveReadMode);
                    if (archive) {
                        struct Info {
                            char pad[0x128];
                            unsigned size;
                            char tail[0x08];
                        } info;
                        Archive_GetEntryInfo(archive, -1, &info);
                        Archive_GetEntryInfo(archive, 0, &info);
                        size = info.size;
                        data = __nwa__FUl(size);
                        if (data)
                            Archive_ReadEntry(archive, 0, data, info.size, kArchiveReadMode);
                    }
                    if (archive)
                        Archive_Close(archive);
                } else {
                    size = source->size;
                    data = embeddedData;
                }
                if (data) {
                    IPetMesh_ScaleResource(data, kResourceScale);
                    CreateFromMemory__Q23gtl25IGenericAnimationResourceFPviPPv(data, size, &entry->resource);
                    if (input->packed) {
                        if (data)
                            __dla__FPv(data);
                    }
                    SceneFrameRange* frameRange = (SceneFrameRange*)GetChunkElement__Q23gtl20IGenericResourceFileFii(
                        entry->resource, kSceneChunk, 0);
                    totalDuration += (frameRange->end - frameRange->start) / kFramesPerSecond;
                }
            }
        }
    }
    return totalDuration;
}

// Find the resource whose mode matches the current selection.
void* CPetAnimation::GetSelectedResource() {
    for (int i = 0; i < entryCount; ++i) {
        Entry* entry = &entries[i];
        if (entry->mode == mode) {
            return entry->resource;
        }
    }
    return 0;
}

int CPetAnimation::GetEntryCount() {
    return entryCount;
}

// Store the selected mode and its evaluation amount together.
int CPetAnimation::SetModeAndAmount(unsigned short newMode, float newAmount) {
    mode = newMode;
    amount = newAmount;
    return 1;
}

float CPetAnimation::GetAmount() {
    return amount;
}

// Read the first scene chunk's scalar value from a named resource.
int CPetAnimation::GetValueByName(float* out, const char* key) {
    if (!out || !key)
        return 0;
    Entry* entry = FindEntry(this, key);
    if (!entry)
        return 0;
    if (!out || !entry)
        return 0; // repeated check: the original compiles it this way
    if (!entry->resource)
        return 0;
    SceneScalar* scalar =
        (SceneScalar*)GetChunkElement__Q23gtl20IGenericResourceFileFii(entry->resource, kSceneChunk, 0);
    if (!scalar)
        return 0;
    *out = scalar->value;
    return 1;
}

// Evaluate transforms on a named resource; missing names or resources fail without a call.
int CPetAnimation::EvaluateNamedTransforms(int evaluationArg0, int evaluationArg1, float evaluationAmount,
                                           const char* key) {
    Entry* entry = FindEntry(this, key);
    if (!entry)
        return 0;
    if (!entry->resource)
        return 0;
    int evaluated = EvaluateTransforms__Q23gtl25IGenericAnimationResourceCFPvPvf(entry->resource, evaluationArg0,
                                                                                 evaluationArg1, evaluationAmount);
    return evaluated != 0;
}

// Evaluate all material matrices of a named resource and copy the requested one;
// matrixIndex must be in range.
int CPetAnimation::EvaluateNamedMaterialMatrix(PetMaterialMatrix* out, int evaluationArg, float evaluationAmount,
                                               const char* key, unsigned matrixIndex) {
    if (!out)
        return 0;
    Entry* entry = FindEntry(this, key);
    if (!entry)
        return 0;
    if (!entry->resource)
        return 0;
    unsigned matrixCount = GetChunkElementCount__Q23gtl20IGenericResourceFileFi(entry->resource, kMaterialPatternChunk);
    if (!matrixCount)
        return 0;
    PetMaterialMatrix* buffer = (PetMaterialMatrix*)__nwa__FUl(matrixCount * sizeof(PetMaterialMatrix));
    int evaluated = EvaluateMaterials__Q23gtl25IGenericAnimationResourceCFPvf(evaluationAmount, entry->resource, buffer,
                                                                              evaluationArg) != 0;
    if (evaluated) {
        *out = buffer[matrixIndex];
        evaluated = 1;
    }
    if (buffer)
        __dla__FPv(buffer);
    return evaluated;
}

// Return the entry's name-buffer size, including the NUL, or zero if absent.
int CPetAnimation::GetEntryNameSize(unsigned short entryIndex) {
    Entry* entry;
    if (!entries) {
        entry = 0;
    } else if (entryCount <= entryIndex) {
        entry = 0;
    } else {
        entry = &entries[entryIndex];
    }
    if (!entry)
        return 0;
    if (entry->name)
        return strlen(entry->name) + 1;
    return 0;
}

// Copy a complete NUL-terminated entry name only when the caller's buffer is large enough.
int CPetAnimation::CopyEntryName(char* destination, unsigned short entryIndex, unsigned size) {
    if (!destination)
        return 0;
    Entry* ents = entries;
    Entry* entry;
    if (!ents) {
        entry = 0;
    } else if (entryCount <= entryIndex) {
        entry = 0;
    } else {
        entry = &ents[entryIndex];
    }
    int nameLength;
    if (!entry) {
        nameLength = 0;
    } else {
        nameLength = entry->name ? strlen(entry->name) + 1 : 0;
    }
    if (!nameLength)
        return 0;
    if (size < (unsigned)nameLength)
        return 0;
    // The entry is looked up a second time: the original compiles the bounds check twice.
    if (!ents) {
        entry = 0;
    } else if (entryCount <= entryIndex) {
        entry = 0;
    } else {
        entry = &ents[entryIndex];
    }
    if (!entry)
        return 0;
    if (!entry->name)
        return 0;
    Memory_Copy(destination, entry->name, nameLength);
    return 1;
}

// Free entry names and release shared resources before deleting the entry array.
void CPetAnimation::Clear() {
    if (entries) {
        Entry* entry;
        for (int i = 0; i < entryCount; ++i) {
            entry = &entries[i];
            if (entry->name) {
                __dla__FPv(entry->name);
                entry->name = 0;
            }
            ReleaseResource(entry);
        }
        Entry* table = entries;
        if (table) {
            if (table)
                __dla__FPv((char*)table - kArrayCookieSize);
            entries = 0;
        }
        entryCount = 0;
    }
}

extern "C" {
// Trivial helper destruction; only a positive delete flag frees the object itself.
void* IUnknownCustom_dtor(void* self, int deleteStorage) {
    if (self && deleteStorage > 0) {
        __dl__FPv(self);
    }
    return self;
}
}
