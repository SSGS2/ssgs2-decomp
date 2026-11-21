typedef unsigned long new_size_t;
// Translation unit 0x80046A38-0x80047498 (13 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

extern "C" {

enum AnimationChunkConstants { kSceneChunk = 0x6E6373, kMaterialPatternChunk = 0x7074, kArchiveReadMode = 3, kArrayCookieSize = 0x10 };

struct PetAnimationState {
    void* reservedHeader;
    void* vtable;
    struct Entry* entries;
    unsigned short entryCount;
    char countPadding[2];
    float amount;
    unsigned short mode;
};

struct AnimationResourceVtable {
    void* a;
    void* b;
    void (*dtor)(struct AnimationResourceRef*, int);
};

struct AnimationResourceRef {
    int refs;
    struct AnimationResourceVtable* vtable;
};



struct Entry {
    Entry();
    static void* operator new[](new_size_t size);
    unsigned short mode;
    char modePadding[2];
    char* name;
    AnimationResourceRef* resource;
};

void __dl__FPv(void*);
void CPetAnimation_Clear(PetAnimationState*);
void __dla__FPv(void*);
int strcmp(const char*, const char*);
int gtl_IGenericAnimationResource_EvaluateTransforms(AnimationResourceRef*, int, int, float);
int strlen(const char*);
void Memory_Copy(char*, const char*, unsigned);
extern char __vt__13CPetAnimation[];

// Trivial helper destruction; only a positive delete flag frees the object itself.
void* IUnknownCustom_dtor(void* self, int deleteStorage) {
    if (self && deleteStorage > 0) {
        __dl__FPv(self);
    }
    return self;
}

unsigned short CPetAnimation_GetEntryCount(PetAnimationState* self) {
    return self->entryCount;
}

#pragma function_align 4
// Find the resource whose mode matches the current animation selection.
void* CPetAnimation_GetSelectedResource(PetAnimationState* self) {
    for (int i = 0; i < self->entryCount; ++i) {
        Entry* entry = &self->entries[i];
        if (entry->mode == self->mode) {
            return entry->resource;
        }
    }
    return 0;
}
// Restore the animation vtable, release entries, and optionally free the owner.
PetAnimationState* CPetAnimation_dtor(PetAnimationState* self, int deleteStorage) {
    if (self) {
        self->vtable = __vt__13CPetAnimation;
        CPetAnimation_Clear(self);
        if (deleteStorage > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Store the selected animation mode and its evaluation amount together.
int CPetAnimation_SetModeAndAmount(PetAnimationState* self, unsigned short mode, float amount) {
    self->mode = mode;
    self->amount = amount;
    return 1;
}

float CPetAnimation_GetAmount(PetAnimationState* self) {
    return self->amount;
}

// Return the selected entry's name-buffer size, including the NUL, or zero if absent.
int AnimTable_GetNameSize(PetAnimationState* self, unsigned short entryIndex) {
    Entry* entry;
    if (!self->entries) {
        entry = 0;
    } else if (self->entryCount <= entryIndex) {
        entry = 0;
    } else {
        entry = &self->entries[entryIndex];
    }
    if (!entry) return 0;
    if (entry->name) return strlen(entry->name) + 1;
    return 0;
}

struct RefObject {
    int refs;
    virtual ~RefObject();
};

// Free entry names and release shared resource references before deleting the entry array.
void CPetAnimation_Clear(PetAnimationState* self) {
    if (self->entries) {
        Entry* entry;
        for (int i = 0; i < self->entryCount; ++i) {
            entry = &self->entries[i];
            if (entry->name) {
                __dla__FPv(entry->name);
                entry->name = 0;
            }
            AnimationResourceRef* resourceRef = entry->resource;
            if (resourceRef) {
                if (--resourceRef->refs == 0) {
                    delete (RefObject*)resourceRef;
                }
                entry->resource = 0;
            }
        }
        Entry* entries = self->entries;
        if (entries) {
            if (entries) __dla__FPv((char*)entries - kArrayCookieSize);
            self->entries = 0;
        }
        self->entryCount = 0;
    }
}


struct SceneScalar { char pad[8]; float value; };
struct MaterialMatrix { unsigned words[16]; };
SceneScalar* gtl_IGenericResourceFile_GetChunkElement(AnimationResourceRef*, unsigned, int);
unsigned gtl_IGenericResourceFile_GetChunkElementCount(AnimationResourceRef*, unsigned);
void* __nwa__FUl(unsigned);
int gtl_IGenericAnimationResource_EvaluateMaterials(float, AnimationResourceRef*, MaterialMatrix*, int);

inline Entry* findEntry(PetAnimationState* self, const char* key) {
    if (!key) return 0;
    unsigned entryCount = self->entryCount;
    int entryByteOffset;
    unsigned i = 0;
    entryByteOffset = 0;
    for (; i < entryCount; ++i) {
        Entry* entry;
        if (!self->entries) entry = 0;
        else if (entryCount <= i) entry = 0;
        else entry = (Entry*)((char*)self->entries + entryByteOffset);
        if (!strcmp(key, entry->name)) return entry;
        entryByteOffset += sizeof(Entry);
    }
    return 0;
}

// Evaluate transforms on a named resource; missing names/resources fail without a call.
int CPetAnimation_EvaluateNamedTransforms(PetAnimationState* self, int evaluationArg0, int evaluationArg1, float amount, const char* key) {
    Entry* entry = findEntry(self, key);
    if (!entry) return 0;
    if (!entry->resource) return 0;
    int evaluated = gtl_IGenericAnimationResource_EvaluateTransforms(entry->resource, evaluationArg0, evaluationArg1, amount);
    return evaluated != 0;
}

// Read the first scene chunk's scalar value from a named animation resource.
int CPetAnimation_GetValueByName(PetAnimationState* self, float* out, const char* key) {
    if (!out || !key) return 0;
    Entry* entry = findEntry(self, key);
    if (!entry) return 0;
    if (!out || !entry) return 0;
    if (!entry->resource) return 0;
    SceneScalar* resource = gtl_IGenericResourceFile_GetChunkElement(entry->resource, kSceneChunk, 0);
    if (!resource) return 0;
    *out = resource->value;
    return 1;
}

// Evaluate all material matrices and copy the requested one; matrixIndex must be in range.
int CPetAnimation_EvaluateNamedMaterialMatrix(PetAnimationState* self, MaterialMatrix* out, int evaluationArg, float amount, const char* key, unsigned matrixIndex) {
    if (!out) return 0;
    Entry* entry = findEntry(self, key);
    if (!entry) return 0;
    if (!entry->resource) return 0;
    unsigned matrixCount = gtl_IGenericResourceFile_GetChunkElementCount(entry->resource, kMaterialPatternChunk);
    if (!matrixCount) return 0;
    MaterialMatrix* buffer = (MaterialMatrix*)__nwa__FUl(matrixCount * sizeof(MaterialMatrix));
// The callee does not read r5 (evaluationArg).
    int evaluated = gtl_IGenericAnimationResource_EvaluateMaterials(amount, entry->resource, buffer, evaluationArg) != 0;
    if (evaluated) {
        *out = buffer[matrixIndex];
        evaluated = 1;
    }
    if (buffer) __dla__FPv(buffer);
    return evaluated;
}

// Copy a complete NUL-terminated entry name only when the caller's buffer is large enough.
int CPetAnimation_CopyEntryName(PetAnimationState* self, char* destination, unsigned short entryIndex, unsigned size) {
    if (!destination) return 0;
    Entry* ents = self->entries;
    Entry* entry;
    if (!ents) {
        entry = 0;
    } else if (self->entryCount <= entryIndex) {
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
    if (!nameLength) return 0;
    if (size < (unsigned)nameLength) return 0;
    if (!ents) {
        entry = 0;
    } else if (self->entryCount <= entryIndex) {
        entry = 0;
    } else {
        entry = &ents[entryIndex];
    }
    if (!entry) return 0;
    if (!entry->name) return 0;
    Memory_Copy(destination, entry->name, nameLength);
    return 1;
}

inline void* Entry::operator new[](new_size_t size) { return __nwa__FUl(size); }

struct AnimationChunk {
    unsigned chunkHeader[2];
    unsigned offset;
    unsigned short count;
    unsigned char packed;
};
struct AnimationChunkEntry {
    unsigned size;
    unsigned data;
    unsigned name;
    unsigned short mode;
    unsigned short length;
};
void* __construct_new_array(void*, void (*)(void*), void*, unsigned, unsigned);
void Memory_Clear(void*, unsigned);
void* Archive_Open(void*, unsigned, int);
void Archive_GetEntryInfo(void*, int, void*);
void Archive_ReadEntry(void*, int, void*, unsigned, int);
void Archive_Close(void*);
void IPetMesh_ScaleResource(void*, float);
void gtl_IGenericAnimationResource_CreateFromMemory(void*, unsigned, AnimationResourceRef**);
struct SceneFrameRange { unsigned chunkHeader; float start; float end; };

extern const float kFloat_0_PetAnimation, kFloat_0_32_PetAnimation, kFloat_30_PetAnimation;

// Build named animation entries, sharing duplicate names and optionally unpacking archive data.
// Embedded offsets are relative to input; only newly created resources add to the duration.
float CPetAnimation_LoadEntriesFromChunk(PetAnimationState* self, AnimationChunk* input) {
    float totalDuration = kFloat_0_PetAnimation;
    self->entryCount = input->count;
    if (!self->entryCount) return totalDuration;
    self->entries = new Entry[self->entryCount];
    AnimationChunkEntry* source;
    int offset;
    Entry* entry;
    int i = 0;
    offset = 0;
    source = (AnimationChunkEntry*)((char*)input + input->offset);
    for (; i < self->entryCount; ++source, offset += sizeof(Entry), ++i) {
        entry = (Entry*)((char*)self->entries + offset);
        entry->resource = 0;
        entry->mode = source->mode;
        entry->name = (char*)__nwa__FUl(source->length + 1);
        Memory_Clear(entry->name, source->length + 1);
        Memory_Copy(entry->name, (char*)input + source->name, source->length);
        int reusedResource = 0;
        for (int j = i - 1; j >= 0; --j) {
            Entry* prior = &self->entries[j];
            if (!strcmp(prior->name, entry->name)) {
                AnimationResourceRef* resource = entry->resource;
                if (resource) {
                    if (--resource->refs == 0) delete (RefObject*)resource;
                    entry->resource = 0;
                }
                entry->resource = prior->resource;
                ++entry->resource->refs;
                reusedResource = 1;
                break;
            }
        }
        if (!reusedResource) {
            AnimationResourceRef* resource = entry->resource;
            if (resource) {
                if (--resource->refs == 0) delete (RefObject*)resource;
                entry->resource = 0;
            }
            if (source->size) {
                void* data = 0;
                unsigned size = 0;
                void* embeddedData = (char*)input + source->data;
                if (input->packed) {
                    void* archive = Archive_Open(embeddedData, source->size, kArchiveReadMode);
                    if (archive) {
                        struct Info { char pad[0x128]; unsigned size; char tail[0x08]; } info;
                        Archive_GetEntryInfo(archive, -1, &info);
                        Archive_GetEntryInfo(archive, 0, &info);
                        size = info.size;
                        data = __nwa__FUl(size);
                        if (data) Archive_ReadEntry(archive, 0, data, info.size, kArchiveReadMode);
                    }
                    if (archive) Archive_Close(archive);
                } else {
                    size = source->size;
                    data = embeddedData;
                }
                if (data) {
                    IPetMesh_ScaleResource(data, kFloat_0_32_PetAnimation);
                    gtl_IGenericAnimationResource_CreateFromMemory(data, size, &entry->resource);
                    if (input->packed) {
                        if (data) __dla__FPv(data);
                    }
                    SceneFrameRange* frameRange = (SceneFrameRange*)gtl_IGenericResourceFile_GetChunkElement(entry->resource, kSceneChunk, 0);
                    totalDuration += (frameRange->end - frameRange->start) / kFloat_30_PetAnimation;
                }
            }
        }
    }
    return totalDuration;
}
} // extern "C"














