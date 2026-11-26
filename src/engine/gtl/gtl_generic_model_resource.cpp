// Model resource construction: retain an archive, load graphics handles, then
// optionally rebuild an archive without selected graphics chunk families.
// Modding lookup: ModelParams::omitChunkFamilies at +0x14 selects omission families below;
// gtl_GenericModel_LoadGraphicsResources consumes the original archive FIRST.
// gtl_ResourceFile_RebuildChunks (gtl_image_font_resource_adapters.cpp) consumes the surviving ChunkRefs.
// That callee sorts descriptors by tag, drops null data/zero count/zero stride,
// copies count*stride bytes per survivor, and aligns rebuilt payloads to 16.
// This is resource retention policy, not a per-frame visibility setting.

#include "game/stdexcept_local.h"
#include "classes/GtlIGenericModelingResource.h"

// Native Wii 16-byte rebuild descriptor: borrowed data pointer, element count,
// bytes per element, numeric tag. Do not confuse it with the serialized record
// below (tag/count/stride/offset); the field orders differ.
struct ChunkRef {
    void* data;
    unsigned int elementCount;       // element count, copied from ChunkEntry::elementCount
    unsigned int elementStrideBytes; // element stride in bytes, copied from ChunkEntry::elementStrideBytes
    unsigned int tag;
};

// Big-endian Wii archive directory entry, 16 bytes. offset is measured from the
// archive data/table base returned by Object_GetField8, not this entry's address.
// Numeric tags pack the file's character codes low-byte first (e.g. 0x786574 tex).
struct ChunkEntry {
    unsigned int tag;
    unsigned int elementCount;       // element count
    unsigned int elementStrideBytes; // byte stride; payload length is count*stride in rebuild
    unsigned int offset;
};

struct ChunkTable {
    char reserved00[0x14];
    unsigned int count;
    char unk18[0x8];
    ChunkEntry entries[1];
};

struct RefCounted {
    int refCount;
    void** vtable;
};

struct ModelParams {
    char reserved00[0x14];
    // Nonzero triggers rebuilding, even if only unrecognized bits are set.
    // 0x1 omit vbh/vbd (vertex buffer header/data), 0x2 ibh/ibd (index buffer),
    // 0x4 tex/texd (texture), 0x8 pal/pald (palette). Other tags always survive.
    // Only these four bit meanings are established; do not infer reserved bits.
    unsigned int omitChunkFamilies;
};

enum ModelChunkOmission {
    kOmitVertexBufferChunks = 1,
    kOmitIndexBufferChunks = 2,
    kOmitTextureChunks = 4,
    kOmitPaletteChunks = 8
};

// Local expansion of the library's resize/growth transaction. vec1 is the
// selected-descriptor vector, vec2 temporary replacement storage. The two zero
// records and flags retain construction/exception statement order; they carry
// no chunk-selection policy. n48 is required additional capacity; n50/n4c are
// alternative growth increments. tbl copies the rebuild format's three words.
struct GenFrame {
    unsigned char flags[0x48];
    unsigned int n48;
    unsigned int n4c;
    unsigned int n50;
    ChunkRef zeroTmp2;
    ChunkRef zeroTmp;
    unsigned int tbl[3];
    ChunkRef* vec1Begin;
    unsigned int vec1Size;
    unsigned int vec1Cap;
    ChunkRef* vec2Begin;
    unsigned int vec2Size;
    unsigned int vec2Cap;
    const unsigned int* vec2Ref;
    unsigned int vec2Copy;
};

extern "C" {

extern char __vt__Q23gtl20IGenericResourceFile[];
extern char __vt__Q23gtl24IGenericModelingResource[];
extern unsigned int s_rebuildChunkHeaderDefaults[3];
extern char s_lengthErrorInfo[];

void gtl_GenericModel_LoadGraphicsResources(void* archive, void* member, const ModelParams* desc);
void* Object_GetField8(void* obj);
void gtl_ResourceFile_RebuildChunks(void** out, void* chunks, int count, int* table);
void* __nw__FUl(unsigned int size);
void __dl__FPv(void* p);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Required storage, nonnull retained archive and readable params through
// +0x14. Returns this; archive refcount is incremented before graphics loading.
// flags==0 keeps that reference. With flags!=0, count/filter in source order,
// build borrowed descriptors, RELEASE the original reference, clear parent,
// then ask RebuildChunks to publish a new parent. Caller must keep another
// archive reference alive while those descriptors still point into its bytes.
// No status/rollback or catch is provided here: allocations/length_error and
// loader exceptions propagate after partial initialization. In particular a
// rebuild failure occurs after parent was cleared. Constructor cleanup must
// not be assumed to restore the original archive.
// Phase map: retain/zero/load -> count survivors -> resize descriptor vector
// -> fill survivors -> release old archive -> rebuild -> free descriptors.
gtl::IGenericModelingResource* gtl::IGenericModelingResource::Init(void* archive, const ModelParams* params) {
    unsigned int i;
    unsigned int n;
    unsigned int ZERO = 0;
    GenFrame f;

    this->refCount = 1;
    this->vtable = (void*)__vt__Q23gtl20IGenericResourceFile;
    this->backingResource = archive;
    ((RefCounted*)archive)->refCount += 1;
    this->vtable = (void*)__vt__Q23gtl24IGenericModelingResource;
    unsigned int* zeroDst = (unsigned int*)&textures;
    zeroDst[0] = ZERO;
    zeroDst[1] = ZERO;
    zeroDst[2] = ZERO;
    zeroDst[3] = ZERO;
    zeroDst[4] = ZERO;
    zeroDst[5] = ZERO;
    zeroDst[6] = ZERO;
    zeroDst[7] = ZERO;
    zeroDst[8] = ZERO;
    zeroDst[9] = ZERO;
    zeroDst[10] = ZERO;
    zeroDst[11] = ZERO;
    zeroDst[12] = ZERO;
    zeroDst[13] = ZERO;
    zeroDst[14] = ZERO;
    zeroDst[15] = ZERO;
    zeroDst[16] = ZERO;
    zeroDst[17] = ZERO;
    gtl_GenericModel_LoadGraphicsResources(this->backingResource, (void*)zeroDst, params);
    if (params->omitChunkFamilies != 0) {

        f.vec1Begin = 0;
        f.vec1Size = ZERO;
        f.vec1Cap = ZERO;
        n = 0;
        ChunkTable* table = (ChunkTable*)Object_GetField8(archive);
        for (i = 0; i < table->count; i++) {
            ChunkEntry* e = &table->entries[i];
            switch ((int)e->tag) {
            case 0x686269:
            case 0x646269:
                if (params->omitChunkFamilies & kOmitIndexBufferChunks)
                    continue;
                break;
            case 0x646276:
            case 0x686276:
                if (params->omitChunkFamilies & kOmitVertexBufferChunks)
                    continue;
                break;
            case 0x786574:
            case 0x64786574:
                if (params->omitChunkFamilies & kOmitTextureChunks)
                    continue;
                break;
            case 0x6c6170:
            case 0x646c6170:
                if (params->omitChunkFamilies & kOmitPaletteChunks)
                    continue;
                break;
            }
            n++;
        }

        // n is the SURVIVOR count. Resize creates n zero descriptors; it does not
        // copy or remove archive chunks. Both selection loops must keep identical
        // tag/mask policy so written==vec1Size at the rebuild call.
        unsigned int size = f.vec1Size;
        if (n > size) {
            n -= size;
            f.flags[0x10] = ZERO;
            // Keep these shared lexical scopes: a structured if/else rewrite was
            // was tried and changed the constructor's unwind entry.
            // The two paths below only size storage; chunk policy remains outside.
            if (n <= f.vec1Cap && size <= f.vec1Cap - n) {
                goto zeroFillExistingCapacity;
            }
            {
                f.flags[0x3c] = ZERO;
                if (f.vec1Size + n - f.vec1Cap > 0x0FFFFFFFu - f.vec1Cap) {
                    f.flags[0x44] = ZERO;
                    f.flags[0x40] = ZERO;
                    std::length_error(s_lengthErrorInfo, s_lengthErrorInfo + 0x14);
                }
                goto allocateAndPublishDescriptors;
            zeroFillExistingCapacity:
                f.flags[0x38] = ZERO;
                ChunkRef* p = f.vec1Begin + size;
                while (n != 0) {
                    f.zeroTmp.data = 0;
                    f.zeroTmp.elementCount = 0;
                    f.zeroTmp.elementStrideBytes = 0;
                    f.zeroTmp.tag = 0;
                    if (p) {
                        try {
                            p->data = 0;
                            p->elementCount = 0;
                            p->elementStrideBytes = 0;
                            p->tag = 0;
                        } catch (...) {
                            throw;
                        }
                    }
                    n--;
                    p++;
                    f.vec1Size++;
                }
                goto descriptorsSized;
            allocateAndPublishDescriptors:
                {
                    unsigned int cap;
                    f.vec2Begin = 0;
                    f.vec2Size = ZERO;
                    f.vec2Cap = ZERO;
                    f.vec2Ref = &f.vec1Cap;
                    f.vec2Copy = ZERO;
                    cap = f.vec1Cap;
                    f.n48 = f.vec1Size + n - f.vec1Cap;
                    if (f.n48 > 0x0FFFFFFFu - cap) {
                        f.flags[0x14] = ZERO;
                        f.flags[0x18] = ZERO;
                        std::length_error(s_lengthErrorInfo, s_lengthErrorInfo + 0x14);
                    }

                    unsigned int grow;
                    if (cap < 0x05555555u) {
                        f.n50 = (3u * (cap + 1)) / 5u;
                        const unsigned int* growSrc = (f.n50 < f.n48) ? &f.n48 : &f.n50;
                        grow = cap + *growSrc;
                    } else if (cap < 0x0AAAAAAAu) {
                        f.n4c = (cap + 1) / 2u;
                        const unsigned int* growSrc = (f.n4c < f.n48) ? &f.n48 : &f.n4c;
                        grow = cap + *growSrc;
                    } else {
                        grow = 0x0FFFFFFFu;
                    }
                    f.flags[0x24] = ZERO;
                    if (grow > 0x0FFFFFFFu) {
                        f.flags[0x1c] = ZERO;
                        f.flags[0x20] = ZERO;
                        std::length_error(s_lengthErrorInfo, s_lengthErrorInfo + 0x14);
                    }

                    f.vec2Begin = (ChunkRef*)__nw__FUl(grow * 16);
                    f.vec2Cap = grow;
                    f.vec2Copy = f.vec1Size;
                    f.flags[0x28] = ZERO;
                    ChunkRef* q = f.vec2Begin + f.vec2Copy + f.vec2Size;
                    while (n != 0) {
                        f.zeroTmp2.data = 0;
                        f.zeroTmp2.elementCount = 0;
                        f.zeroTmp2.elementStrideBytes = 0;
                        f.zeroTmp2.tag = 0;
                        if (q) {
                            try {
                                q->data = 0;
                                q->elementCount = 0;
                                q->elementStrideBytes = 0;
                                q->tag = 0;
                            } catch (...) {
                                throw;
                            }
                        }
                        n--;
                        q++;
                        f.vec2Size++;
                    }

                    f.flags[0x2c] = ZERO;
                    ChunkRef* src = f.vec1Begin + f.vec1Size;
                    ChunkRef* dst = f.vec2Begin + f.vec2Copy;
                    while (src > f.vec1Begin) {
                        --dst;
                        --src;
                        if (dst) {
                            try {
                                *dst = *src;
                            } catch (...) {
                                throw;
                            }
                        }
                        f.vec2Copy--;
                        f.vec2Size++;
                    }

                    ChunkRef* oldBegin = f.vec1Begin;
                    unsigned int oldCap = f.vec1Cap;
                    f.flags[0x30] = ZERO;
                    f.vec1Cap = f.vec2Cap;
                    f.vec2Cap = oldCap;
                    f.vec1Begin = f.vec2Begin;
                    f.vec2Begin = oldBegin;
                    f.vec1Size = f.vec2Size;
                    f.vec2Size = ZERO;
                    if (oldBegin) {
                        unsigned int z = f.vec2Size;
                        f.flags[0x34] = ZERO;
                        f.vec2Size -= z;
                        __dl__FPv(oldBegin);
                    }
                }
            }
        } else if (n < size) {
            unsigned int d = size - n;
            f.flags[0x0C] = ZERO;
            f.vec1Size = size - d;
        }
    descriptorsSized:;

        // Allocation is complete. Fill only survivors, preserving directory order;
        // data pointers borrow the original archive until the synchronous rebuild.
        unsigned int written = 0;
        for (i = 0; i < table->count; i++) {
            ChunkEntry* e = &table->entries[i];
            switch ((int)e->tag) {
            case 0x686269:
            case 0x646269:
                if (params->omitChunkFamilies & kOmitIndexBufferChunks)
                    continue;
                break;
            case 0x646276:
            case 0x686276:
                if (params->omitChunkFamilies & kOmitVertexBufferChunks)
                    continue;
                break;
            case 0x786574:
            case 0x64786574:
                if (params->omitChunkFamilies & kOmitTextureChunks)
                    continue;
                break;
            case 0x6c6170:
            case 0x646c6170:
                if (params->omitChunkFamilies & kOmitPaletteChunks)
                    continue;
                break;
            }
            ChunkRef* d = f.vec1Begin + written;
            d->data = (char*)table + e->offset;
            d->elementCount = e->elementCount;
            d->elementStrideBytes = e->elementStrideBytes;
            d->tag = e->tag;
            written++;
        }

        RefCounted* parent = (RefCounted*)this->backingResource;
        if (--parent->refCount == 0 && parent != 0) {
            short one = 1;
            ((void (*)(void*, short))(parent->vtable[2]))(parent, one);
        }
        this->backingResource = 0;
        const unsigned int* tblSrc = s_rebuildChunkHeaderDefaults;
        f.tbl[0] = *tblSrc++;
        f.tbl[1] = *tblSrc++;
        f.tbl[2] = *tblSrc;
        gtl_ResourceFile_RebuildChunks((void**)&this->backingResource, f.vec1Begin, (int)f.vec1Size, (int*)f.tbl);
        if (f.vec1Begin) {
            unsigned int cnt = f.vec1Size;
            f.flags[0x08] = ZERO;
            f.vec1Size -= cnt;
            __dl__FPv(f.vec1Begin);
        }
    }
    return this;
}
#pragma pop

extern "C" {}
