#include <decomp/ct_member.h>
#include "classes/SoundResourceParameters8043.h"
#include "classes/SoundResource8043.h"
#include "classes/SoundPropertyWords8043.h"
#include "classes/SoundChannelParameters8043.h"
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12length_errorFv(void*, int);
extern const char soundResourceFormatStrings[];
struct SoundResourceStringPool {
    char reserved0[0x14];
    char lengthErrorType[0x37]; // +0x14
};

}
}

// Translation unit 0x8043B91C-0x8043E858 (gtl::CGSPSoundProperty, CGSPSoundBufferResource, CGSPSoundStreamResource and their factories; one original file, data interleaved). The layout views preserve the resource/chunk ownership ABI.
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
#include "sdk_protos.h"
extern "C" {

enum { kPointerVectorMaxCount = 0x3FFFFFFF, kPointerVectorGrowthThreeFifthsLimit = 0x15555555, kPointerVectorGrowthHalfLimit = 0x2AAAAAAA };
enum SoundResourceAbiSizes {
    kSoundPointerSlotBytes = 4, kSoundChunkDescriptorBytes = 32,
    kSoundBaseParameterBytes = 40, kSoundAdpcmBlockBytes = 0x30
};
enum SoundChunkDescriptorSlot { kChunkFormatVersion = 0, kChunkName = 1,
    kChunkSourceData = 2, kChunkBaseParameters = 3, kChunkEmitter = 4,
    kChunkEnvelope = 5, kChunkRolloffCurve = 6, kChunkPlatformExtension = 7 };
// Layout-view map (address suffixes identify overlapping ABI views, not object kinds):
// RetainedResource = refcounted source resource (refs, vtable, offset table); ResourceChunkTable = vector of
// 32-byte ResourceChunkDesc plus the retained source; BufferResourceView = buffer-resource storage seen from the
// factory (sourceSlots owns source references; propertySlots owns decoded property references).
// Obj_8043D2AC is the +4 virtual interface used to select a source/property for a
// destination buffer. Its get/selectProperty slots return borrowed resource-owned objects;
// selectProperty does not allocate a new object here (see vfunc_3).
// Struct_8043D264 is an overlapping +8 data-pointer view used by count/source
// accessors: child at runtime-object +8 is data, but data->child at +8 is a COUNT
// encoded in a pointer-shaped field. Do not dereference the returned count.
// Struct_8043D270Root is the outer +8 data-pointer view for chunk-name lookup;
// its child holds descriptor storage at +4 and descriptor count at +8.
// Obj_*/Struct_*Root = runtime resource with data pointer at +8; Struct_8043D264
// also views its nested source-list/count fields without a separate concrete class.
// Data_8043CFFC = buffer resource storage: retained source owner, owned parsed
// 32-byte descriptors, then owned source/property pointer vectors (sourceSlots/propertySlots).
// ResourceChunkEntry/BorrowedSoundPropertyChunks = borrowed pointers into the retained resource,
// NOT newly decoded property objects. RuntimeSoundProperty = refcounted runtime property
// owning separately allocated base, spatial, curve and envelope blocks.
// BorrowedSoundPropertyChunks is the encoded chunk-pointer bundle accepted by that constructor:
// version, BSIC, EMIT, EV_C, RO_C remain borrowed until decoding completes.
// RuntimeSoundProperty is the destination runtime property, not the source archive.
// The source/property vectors retain one reference per non-null slot; resource
// destruction releases those references before freeing the vector allocations.

void gtl_Audio_Free(void*);
void gtl_ISoundBuffer_SetSource(void*, void*);
int gtl_CGSPSoundProperty_ApplyToBuffer(void*, void*);
void gtl_ISoundBuffer_SetVolume(void*, float);
void gtl_ISoundBuffer_SetPitch(void*, float);
void gtl_ISoundBuffer_SetPan(void*, float);
void gtl_ISoundBuffer_SetLoopRange(void*, unsigned int, unsigned int);
void gtl_ISoundBuffer_SetConeAngles(void*, int, float, float);
void gtl_ISoundBuffer_SetConeGains(void*, int, float, float);
void gtl_ISoundBuffer_SetMinDistance(void*, int, float);
void gtl_ISoundBuffer_SetMaxDistance(void*, int, float);
void gtl_ISoundBuffer_SetDistanceParameterA(void*, int, float);
void gtl_ISoundBuffer_SetDistanceParameterB(void*, int, float);
void gtl_ISoundBuffer_SetDistanceCurve(void*, void*, int);
void gtl_ISoundBuffer_DefaultSuccess(void*);
void gtl_ISoundBuffer_SetEnvelope(void*, void*);
void gtl_ISoundStream_SetVolume(void*, float);
void gtl_ISoundStream_SetPitch(void*, float);
void gtl_ISoundStream_SetPan(void*, float);
void gtl_ISoundStream_SetEnvelope(void*, int);
void gtl_ISoundStream_SetLoopRange(void*, unsigned int, unsigned int);
void SoundStream_DefaultCapabilityTrue(void*);
void gtl_ISoundStream_SetConeAngles(void*, int, float, float);
void gtl_ISoundStream_SetConeGains(void*, int, float, float);
void gtl_ISoundStream_SetMinDistance(void*, int, float);
void gtl_ISoundStream_SetMaxDistance(void*, int, float);
void gtl_ISoundStream_SetDistanceCurve(void*, void*, int);
void gtl_ISoundStream_SetDistanceParameterA(void*, int, float);
void gtl_ISoundStream_SetDistanceParameterB(void*, int, float);

struct VTable_8043D2AC {
    char reserved00[0x1C];
    void* (*get)(void*);
    void* (*selectProperty)(void*, void*);
};
struct Obj_8043D2AC { char reserved00[4]; VTable_8043D2AC* vtable; };
struct ResourceCallBase_8043D2AC { char reserved00[4]; };
struct ResourceCall_8043D2AC : ResourceCallBase_8043D2AC {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void* get(void*);
    virtual void* selectProperty(void*);
};

struct Struct_8043D264 {
    char reserved00[8];
    struct Struct_8043D264* child;
    char reserved0C[8];
    unsigned int* items;
    unsigned int count;
};

struct Struct_8043D270 {
    char reserved00[4];
    unsigned int* data;
    unsigned int count;
};

struct Struct_8043D270Root {
    char reserved00[8];
    Struct_8043D270* child;
};

struct Data_8043CFFC {
    void* owner;
    void* chunkDescriptors;
    unsigned int chunkCount; // chunk count at +8, not a nested pointer
    unsigned int chunkCapacity;
    void* sourceStorage; // retained source-storage object at +0x10
    void* sourceSlots; // owned source-reference vector
    unsigned int sourceCount;
    unsigned int sourceCapacity;
    void* propertySlots; // owned decoded-property-reference vector
    unsigned int propertyCount;
    void* listC;
    unsigned int countC;
};

struct Obj_8043CFFC { char reserved00[4]; void* vtable; Data_8043CFFC* data; };

struct ResourceChunkEntry { unsigned int formatVersionHeaderWord; void* name; char payload[0x18]; };
typedef char ResourceChunkEntry_size[sizeof(ResourceChunkEntry) == 0x20 ? 1 : -1];
extern char __vt__Q23gtl23CGSPSoundBufferResource[];

struct RefVT_8043CFFC { char reserved00[8]; void (*destroy)(void*, int); };
struct Ref_8043CFFC { unsigned int refs; RefVT_8043CFFC* vt; };
struct RefCallBase_8043CFFC { unsigned int refs; };
struct RefCall_8043CFFC : RefCallBase_8043CFFC {
    virtual void destroy(int);
};

// One data block has source/property vectors at the same offsets in both factories.
// This view separates those roles from Struct_8043D264's pointer-shaped count field.
struct SoundResourceListsView {
    void* retainedSourceOwner; // +0: retained source archive
    unsigned int* chunkDescriptors; // +4: owned 32-byte descriptors
    unsigned int chunkCount; // +8
    unsigned int chunkCapacity;
    void* sourceStorage;
    unsigned int* sourceSlots;
    unsigned int sourceCount;
    unsigned int sourceCapacity;
    unsigned int* propertySlots;
    unsigned int propertyCount;
};
struct SoundResourceDataView { char prefix[8]; SoundResourceListsView* resourceData; };

// Borrow resource-owned descriptor/source/property lists without retaining them.
static inline SoundResourceListsView* BorrowResourceLists(void* resource) {
    return ((SoundResourceDataView*)resource)->resourceData;
}

// Release storage allocated by the audio allocator.
void dtor_8043D260(void* p) {
    gtl_Audio_Free(p);
}

// Return the resource chunk count through the original pointer-width ABI.
Struct_8043D264* gtl_CGSPSoundBufferResource_vfunc_1(Struct_8043D264* self) {
    return (Struct_8043D264*)BorrowResourceLists(self)->chunkCount;
}

// Return an indexed buffer source, or null when the index is out of range.
unsigned int* gtl_CGSPSoundBufferResource_GetSourceAt(Struct_8043D264* self, unsigned int index) {
    SoundResourceListsView* data = BorrowResourceLists(self);
    if (index >= data->sourceCount)
        return 0;
    return (unsigned int*)data->sourceSlots[index];
}

// Return an indexed sound property, or null when the index is out of range.
void* gtl_CGSPSoundBufferResource_GetPropertySlot(Struct_8043D264* self, unsigned int index) {
    SoundResourceListsView* data = BorrowResourceLists(self);
    if (index >= data->propertyCount)
        return 0;
    return (void*)data->propertySlots[index];
}

// Return the name stored in an indexed 32-byte resource chunk.
void* gtl_CGSPSoundBufferResource_GetChunkName(Struct_8043D270Root* self, unsigned int index) {
    Struct_8043D270* data = self->child;
    void* item;
    if (index < data->count)
        item = (char*)data->data + index * kSoundChunkDescriptorBytes;
    else
        item = 0;
    if (item != 0)
        return ((ResourceChunkEntry*)item)->name;
    return 0;
}

// Attach this resource and its selected property to a sound buffer.
int gtl_CGSPSoundBufferResource_ApplyToBuffer(Obj_8043D2AC* self, void* arg, void* dst) {
    if (dst == 0)
        return 0;
    void* source = ((ResourceCall_8043D2AC*)self)->get(arg); // slot 0x1C takes the index: r4 still holds arg
    if (source != 0)
        gtl_ISoundBuffer_SetSource(dst, source);
    // The ABI slot returns a borrowed selected property.
    void* selectedProperty = ((ResourceCall_8043D2AC*)self)->selectProperty(arg);
    if (selectedProperty != 0)
        gtl_CGSPSoundProperty_ApplyToBuffer(selectedProperty, dst);
    return 1;
}

// Borrow the current base-parameter block; do not cache it across sound-buffer callbacks.
static inline SoundPropertyWords8043* BorrowBaseParameterWords(void* property) {
    return (SoundPropertyWords8043*)((SoundResourceParameters8043*)property)->baseParameters;
}

// Apply base, spatial and envelope parameters to a non-null sound buffer.
int gtl_CGSPSoundProperty_ApplyToBuffer(void* self, void* dst) {
    if (dst == 0)
        return 0;
    if (((SoundResourceParameters8043*)self)->baseParameters != 0) {
        // 0x8043D34C: property words +0/+4/+8 feed volume/pan/pitch; keep float bit views volatile.
        gtl_ISoundBuffer_SetVolume(dst, *(volatile float*)&BorrowBaseParameterWords(self)->volume);
        gtl_ISoundBuffer_SetPitch(dst, *(volatile float*)&BorrowBaseParameterWords(self)->pitch);
        gtl_ISoundBuffer_SetPan(dst, *(volatile float*)&BorrowBaseParameterWords(self)->pan);
        gtl_ISoundBuffer_SetLoopRange(dst, BorrowBaseParameterWords(self)->loopStart, BorrowBaseParameterWords(self)->loopEnd);
    }
    // Preserve the baseline gate: envelope +0x14 controls spatial application too.
    // channelParameters +0x0C is consumed inside this gate without its own null check.
    if ((*(int*)&((SoundResourceParameters8043*)self)->envelopeParameters) != 0) {
        SoundChannelParameters8043* spatialParameters = (SoundChannelParameters8043*)((SoundResourceParameters8043*)self)->channelParameters;
        gtl_ISoundBuffer_SetConeAngles(dst, 0, spatialParameters->coneInnerAngle, spatialParameters->coneOuterAngle);
        gtl_ISoundBuffer_SetConeGains(dst, 0, spatialParameters->coneInnerGain, spatialParameters->coneOuterGain);
        gtl_ISoundBuffer_SetMinDistance(dst, 0, spatialParameters->minDistance);
        gtl_ISoundBuffer_SetMaxDistance(dst, 0, spatialParameters->maxDistance);
        gtl_ISoundBuffer_SetDistanceParameterA(dst, 0, spatialParameters->distanceParameterA);
        gtl_ISoundBuffer_SetDistanceParameterB(dst, 0, spatialParameters->distanceParameterB);
        void* distanceCurve = ((SoundResourceParameters8043*)self)->distanceCurve;
        if (distanceCurve != 0)
            gtl_ISoundBuffer_SetDistanceCurve(dst, distanceCurve, 0);
        gtl_ISoundBuffer_DefaultSuccess(dst);
    }
    void* envelope = ((SoundResourceParameters8043*)self)->envelopeParameters;
    if (envelope != 0)
        gtl_ISoundBuffer_SetEnvelope(dst, envelope);
    return 1;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Release source/property references, backing storage and optionally the resource itself.
void* gtl_CGSPSoundBufferResource_dtor(void* self, int deleting) {
    if (self != 0) {
        Data_8043CFFC* data = (*(Data_8043CFFC**)&((SoundResource8043*)self)->data);
        ((Obj_8043CFFC*)self)->vtable = (void*)__vt__Q23gtl23CGSPSoundBufferResource;
        if (data != 0 && &data->owner != 0) {
            unsigned int off;
            unsigned int i;
            i = 0;
            off = 0;
            for (; i < data->sourceCount; i++) {
                void* elem = *(void**)((char*)data->sourceSlots + off);
                if (elem != 0) {
                    unsigned int refs = ((Ref_8043CFFC*)elem)->refs - 1;
                    ((Ref_8043CFFC*)elem)->refs = refs;
                    if (refs == 0 && elem != 0)
                        ((RefCall_8043CFFC*)elem)->destroy(1);
                    *(void**)((char*)data->sourceSlots + off) = 0;
                }
                off += kSoundPointerSlotBytes;
            }
            i = 0;
            off = 0;
            for (; i < data->propertyCount; i++) {
                void* elem = *(void**)((char*)data->propertySlots + off);
                if (elem != 0) {
                    unsigned int refs = ((Ref_8043CFFC*)elem)->refs - 1;
                    ((Ref_8043CFFC*)elem)->refs = refs;
                    if (refs == 0 && elem != 0)
                        ((RefCall_8043CFFC*)elem)->destroy(1);
                    *(void**)((char*)data->propertySlots + off) = 0;
                }
                off += kSoundPointerSlotBytes;
            }
            void* single = data->sourceStorage;
            if (single != 0) {
                unsigned int refs = ((Ref_8043CFFC*)single)->refs - 1;
                ((Ref_8043CFFC*)single)->refs = refs;
                if (refs == 0 && single != 0)
                    ((RefCall_8043CFFC*)single)->destroy(1);
                data->sourceStorage = 0;
            }
            if (&data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && data->propertySlots != 0) {
                unsigned int n = data->propertyCount;
                data->propertyCount = data->propertyCount - n;
                gtl_Audio_Free(data->propertySlots);
            }
            if (&data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && data->sourceSlots != 0) {
                unsigned int n = data->sourceCount;
                data->sourceCount = data->sourceCount - n;
                gtl_Audio_Free(data->sourceSlots);
            }
            if (data != 0) {
                void* owner = data->owner;
                if (owner != 0) {
                    unsigned int refs = ((Ref_8043CFFC*)owner)->refs - 1;
                    ((Ref_8043CFFC*)owner)->refs = refs;
                    if (refs == 0 && owner != 0)
                        ((RefCall_8043CFFC*)owner)->destroy(1);
                    data->owner = 0;
                }
                unsigned int n = data->chunkCount;
                data->chunkCount = data->chunkCount - n;
                if (&data->chunkDescriptors != 0 && &data->chunkDescriptors != 0 && &data->chunkDescriptors != 0 && &data->chunkDescriptors != 0 && data->chunkDescriptors != 0) {
                    unsigned int n = data->chunkCount;
                    data->chunkCount = data->chunkCount - n;
                    gtl_Audio_Free(data->chunkDescriptors);
                }
            }
            gtl_Audio_Free(data);
        }
        if (deleting > 0)
            gtl_Audio_Free(self);
    }
    return self;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

// Copy the selected property's 40-byte base parameter block to the caller.
int gtl_CGSPSoundBufferResource_CopyBaseParams(Obj_8043D2AC* self, void* forwardedArg, void* dst) {
    if (dst == 0)
        return 0;
    void* selectedProperty = ((ResourceCall_8043D2AC*)self)->selectProperty(forwardedArg);
    if (selectedProperty != 0) {
        void* p = ((SoundResource8043*)selectedProperty)->data;
        if (p != 0) {
            Memory_Copy(dst, p, kSoundBaseParameterBytes);
            return 1;
        }
    }
    return 0;
}

// Optionally release the stream resource base allocation.
void* StreamBase_dtor(void* self, int deleting) {
    if (self != 0 && deleting > 0)
        gtl_Audio_Free(self);
    return self;
}

struct Struct_8043D598 {
    unsigned int fields[13];
};

#pragma dont_inline on
// Clear all thirteen stream-resource words in the original store order.
void SoundStreamProperties_Clear(Struct_8043D598* self) {
    self->fields[0] = 0;
    self->fields[1] = 0;
    self->fields[3] = 0;
    self->fields[2] = 0;
    self->fields[4] = 0;
    self->fields[5] = 0;
    self->fields[6] = 0;
    self->fields[7] = 0;
    self->fields[8] = 0;
    self->fields[9] = 0;
    self->fields[10] = 0;
    self->fields[11] = 0;
    self->fields[12] = 0;
}
#pragma dont_inline reset

struct Data_8043E118 {
    void* owner;
    void* chunks;
    unsigned int chunkCount;
    char reserved0C[4];
    unsigned int fileNameFlags;
    char reserved14[4];
    void* heapFileName;
    void** sourceSlots; // retained source slots (+0x1C), parallel to decoded properties
    unsigned int sourceCount; // source count
    char reserved24[4];
    void** propertySlots; // retained decoded-property slots (+0x28)
    unsigned int propertyCount; // property count
};

struct VTable_Stream_8043E118 {
    char reserved00[0x20];
    void* (*fn)(void*);
};
struct Obj_8043E118 {
    char reserved00[4];
    VTable_Stream_8043E118* vtable;
    Data_8043E118* data;
};

struct RefVTable_8043E118 {
    char reserved00[8];
    void (*destroy)(void*, int);
};

struct Ref_8043E118 {
    unsigned int refs;
    RefVTable_8043E118* vt;
};

extern char __vt__Q23gtl23CGSPSoundStreamResource[];

#pragma use_lmw_stmw on
#pragma opt_propagation off
// Release stream source/property references, parsed chunks and their source owner.
void* gtl_CGSPSoundStreamResource_dtor(void* self, int deleting) {
    if (self != 0) {
        Data_8043E118* data = ((Obj_8043E118*)self)->data;
        ((Obj_8043E118*)self)->vtable = (VTable_Stream_8043E118*)(void*)__vt__Q23gtl23CGSPSoundStreamResource;
        if (data != 0 && data != 0) {
            unsigned int off;
            unsigned int i;
            i = 0;
            off = 0;
            for (; i < data->sourceCount; i++) {
                void* elem = *(void**)((char*)data->sourceSlots + off);
                if (elem != 0) {
                    unsigned int refs = ((Ref_8043E118*)elem)->refs - 1;
                    ((Ref_8043E118*)elem)->refs = refs;
                    if (refs == 0 && elem != 0)
                        ((RefCall_8043CFFC*)elem)->destroy(1);
                    *(void**)((char*)data->sourceSlots + off) = 0;
                }
                off += kSoundPointerSlotBytes;
            }
            i = 0;
            off = 0;
            for (; i < *(volatile unsigned int*)&data->propertyCount; i++) {
                void* elem = *(void**)((char*)data->propertySlots + off);
                if (elem != 0) {
                    unsigned int refs = ((Ref_8043E118*)elem)->refs - 1;
                    ((Ref_8043E118*)elem)->refs = refs;
                    if (refs == 0 && elem != 0)
                        ((RefCall_8043CFFC*)elem)->destroy(1);
                    *(void**)((char*)data->propertySlots + off) = 0;
                }
                off += kSoundPointerSlotBytes;
            }
            if (&data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && &data->propertySlots != 0 && data->propertySlots != 0) {
                unsigned int n = data->propertyCount;
                data->propertyCount = data->propertyCount - n;
                gtl_Audio_Free(data->propertySlots);
            }
            if (&data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && &data->sourceSlots != 0 && data->sourceSlots != 0) {
                unsigned int n1 = data->sourceCount;
                data->sourceCount = data->sourceCount - n1;
                gtl_Audio_Free(data->sourceSlots);
            }
            if (&data->fileNameFlags != 0 && (data->fileNameFlags >> 31) != 0) {
                gtl_Audio_Free(data->heapFileName);
            }
            if (data != 0) {
                void* elem = data->owner;
                if (elem != 0) {
                    unsigned int refs = ((Ref_8043E118*)elem)->refs - 1;
                    ((Ref_8043E118*)elem)->refs = refs;
                    if (refs == 0 && elem != 0)
                        ((RefCall_8043CFFC*)elem)->destroy(1);
                    data->owner = 0;
                }
                unsigned int n8 = data->chunkCount;
                data->chunkCount = data->chunkCount - n8;
                if (&data->chunks != 0 && &data->chunks != 0 && &data->chunks != 0 && &data->chunks != 0 && data->chunks != 0) {
                    unsigned int n8 = data->chunkCount;
                    data->chunkCount = data->chunkCount - n8;
                    gtl_Audio_Free(data->chunks);
                }
            }
            gtl_Audio_Free(data);
        }
        if (deleting > 0)
            gtl_Audio_Free(self);
    }
    return self;
}
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

struct Data_8043E448 {
    float volume;
    float pitch;
    float pan;
    char reserved0C[0x14];
    unsigned int loopStart;
    unsigned int loopEnd;
};

struct Sub_8043E448 {
    char reserved00[0x30];
    float coneInnerAngle;
    float coneOuterAngle;
    float coneInnerGain;
    float coneOuterGain;
    char reserved40[4];
    float distanceParameterA;
    float distanceParameterB;
    float minDistance;
    float maxDistance;
};

struct Struct_8043E448 {
    char reserved00[8];
    Data_8043E448* baseParameters;
    Sub_8043E448* channelParameters;
    void* distanceCurve;
    int envelope;
};

// Apply base, spatial and envelope parameters to a non-null sound stream.
int gtl_CGSPSoundProperty_ApplyToStream(Struct_8043E448* self, void* dst) {
    if (dst == 0)
        return 0;
    if (self->baseParameters != 0) {
        gtl_ISoundStream_SetVolume(dst, self->baseParameters->volume);
        gtl_ISoundStream_SetPitch(dst, self->baseParameters->pitch);
        gtl_ISoundStream_SetPan(dst, self->baseParameters->pan);
        gtl_ISoundStream_SetLoopRange(dst, self->baseParameters->loopStart, self->baseParameters->loopEnd);
    }
    // As in the buffer path, the envelope word gates spatial parameters.
    if (self->envelope != 0) {
        Sub_8043E448* sub = self->channelParameters;
        gtl_ISoundStream_SetConeAngles(dst, 0, sub->coneInnerAngle, sub->coneOuterAngle);
        gtl_ISoundStream_SetConeGains(dst, 0, sub->coneInnerGain, sub->coneOuterGain);
        gtl_ISoundStream_SetMinDistance(dst, 0, sub->minDistance);
        gtl_ISoundStream_SetMaxDistance(dst, 0, sub->maxDistance);
        gtl_ISoundStream_SetDistanceParameterA(dst, 0, sub->distanceParameterA);
        gtl_ISoundStream_SetDistanceParameterB(dst, 0, sub->distanceParameterB);
        if (self->distanceCurve != 0)
            gtl_ISoundStream_SetDistanceCurve(dst, self->distanceCurve, 0);
        SoundStream_DefaultCapabilityTrue(dst);
    }
    if (self->envelope != 0)
        gtl_ISoundStream_SetEnvelope(dst, self->envelope);
    return 1;
}

extern char __vt__Q23gtl17CGSPSoundProperty[];
void* gtl_Audio_Alloc(unsigned int size);

// Decoded runtime base properties (40 bytes), copied from the BSIC chunk.
struct RuntimeSoundBaseParameters {
    float volume;
    float pitch;
    float pan;
    char reserved0C[4];
    // Four encoded option bytes are widened to words; their value domains are unresolved.
    int options[4];
    int loopStart;
    int loopEnd;
};

// Decoded runtime spatial parameters; leading 0x30 bytes are outside this copy path.
struct RuntimeSoundSpatialParameters {
    char reserved00[0x30];
    float coneInnerAngle;
    float coneOuterAngle;
    float coneInnerGain;
    float coneOuterGain;
    unsigned int distanceCurve;
    float distanceParameterA;
    float distanceParameterB;
    float minDistance;
    float maxDistance;
    char reserved54[4]; // Spatial block allocation includes one unused trailing word.
};

// Runtime distance-curve header owning pointCount distance/gain pairs.
struct RuntimeSoundDistanceCurve {
    unsigned int curveKind;
    unsigned int pointCount;
    void* points;
};

// One curve key: scene-distance input and multiplicative gain output.
struct SoundDistanceGainKey {
    float distance;
    float gain;
};

// Layout consumed by gtl_AudioEnvelope_Initialize; time values remain in native ticks.
struct RuntimeSoundEnvelope {
    int configTag;
    int mode;
    int delayTicks;
    int attackTicks;
    int holdTicks;
    int decayTicks;
    int releaseTicks;
    int sustainUnits;
    int reservedConfig0;
    int reservedConfig1;
};

struct RuntimeSoundProperty {
    int referenceCount;
    void* vtable;
    RuntimeSoundBaseParameters* baseParameters;
    RuntimeSoundSpatialParameters* channelParameters;
    RuntimeSoundDistanceCurve* distanceCurve;
    RuntimeSoundEnvelope* envelope;
};

// Borrowed two-byte format-version slice (not the start of a tagged chunk).
struct SoundChunkVersion {
    unsigned char majorVersion;
    unsigned char minorVersion;
};

// Encoded BSIC chunk, including its 8-byte tag/length prefix.
struct EncodedSoundBaseChunk {
    char reserved00[8];
    float volume;
    float pitch;
    float pan;
    unsigned char encodedOptions[4]; // First two swap positions in the runtime word layout.
    int loopStart;
    int loopEnd;
};

// Encoded EMIT chunk; ordering differs from the runtime spatial block above.
struct EncodedSoundEmitterChunk {
    char reserved00[8];
    float distanceParameterA;
    float distanceParameterB;
    float minDistance;
    float maxDistance;
    float coneInnerAngle;
    float coneOuterAngle;
    float coneInnerGain;
    float coneOuterGain;
};

// Encoded EV_C envelope chunk; byte fields widen into runtime words.
struct EncodedSoundEnvelopeChunk {
    char reserved00[8];
    int delayTicks;
    int attackTicks;
    int holdTicks;
    int decayTicks;
    int releaseTicks;
    unsigned char sustainUnits;
    unsigned char configTag;
    unsigned char mode;
};

// Encoded RO_C curve chunk; points are inline after its count/kind header.
struct EncodedSoundCurveChunk {
    char reserved00[8];
    unsigned int pointCount;
    unsigned int curveKind;
    SoundDistanceGainKey points[1];
};

struct BorrowedSoundPropertyChunks {
    SoundChunkVersion* formatVersion;
    char reserved04[8];
    EncodedSoundBaseChunk* baseParameters;
    EncodedSoundEmitterChunk* channelParameters;
    EncodedSoundEnvelopeChunk* envelope;
    EncodedSoundCurveChunk* distanceCurve;
};

// BSIC stores four whole option bytes, not a packed bit mask. Runtime words
// reorder bytes [A,B,C,D] to [B,A,C,D]; no reader establishes their meanings yet.
enum EncodedSoundOptionSlot { kBsicOption0, kBsicOption1, kBsicOption2, kBsicOption3 };
enum RuntimeSoundOptionSlot { kRuntimeOption0, kRuntimeOption1, kRuntimeOption2, kRuntimeOption3 };
enum { kLegacyLoopIndexMultiplier = 2 };

// Decode BSIC values and convert legacy loop indices into the runtime index scale.
static inline void DecodeSoundBaseParameters(RuntimeSoundProperty* self,
    BorrowedSoundPropertyChunks* src, EncodedSoundBaseChunk* encodedBase) {
    if (encodedBase != 0) {
        self->baseParameters = (RuntimeSoundBaseParameters*)gtl_Audio_Alloc(sizeof(RuntimeSoundBaseParameters));
        SoundChunkVersion* formatVersion = src->formatVersion;
        self->baseParameters->volume = encodedBase->volume;
        self->baseParameters->pitch = encodedBase->pitch;
        self->baseParameters->pan = encodedBase->pan;
        self->baseParameters->options[kRuntimeOption1] = encodedBase->encodedOptions[kBsicOption0];
        self->baseParameters->options[kRuntimeOption0] = encodedBase->encodedOptions[kBsicOption1];
        self->baseParameters->options[kRuntimeOption2] = encodedBase->encodedOptions[kBsicOption2];
        self->baseParameters->options[kRuntimeOption3] = encodedBase->encodedOptions[kBsicOption3];
        self->baseParameters->loopStart = encodedBase->loopStart;
        self->baseParameters->loopEnd = encodedBase->loopEnd;
        if (formatVersion != 0 && ((formatVersion->majorVersion < 1U) || (formatVersion->majorVersion == 1 && formatVersion->minorVersion < 1U))) {
            self->baseParameters->loopStart = self->baseParameters->loopStart * kLegacyLoopIndexMultiplier;
            self->baseParameters->loopEnd = self->baseParameters->loopEnd * kLegacyLoopIndexMultiplier;
        }
    }
}

// Decode EMIT and copy RO_C keys only when an emitter chunk is present.
static inline void DecodeSoundSpatialParameters(RuntimeSoundProperty* self, BorrowedSoundPropertyChunks* src) {
    // Phase 2: reorder spatial parameters and deep-copy optional curve keys.
    EncodedSoundEmitterChunk* encodedEmitter = src->channelParameters;
    if (encodedEmitter != 0) {
        RuntimeSoundSpatialParameters* runtimeSpatial = (RuntimeSoundSpatialParameters*)gtl_Audio_Alloc(sizeof(RuntimeSoundSpatialParameters));
        self->channelParameters = runtimeSpatial;
        EncodedSoundCurveChunk* encodedCurve = src->distanceCurve;
        float coneValues[4];
        coneValues[0] = encodedEmitter->coneInnerAngle;
        coneValues[1] = encodedEmitter->coneOuterAngle;
        coneValues[2] = encodedEmitter->coneInnerGain;
        coneValues[3] = encodedEmitter->coneOuterGain;
        runtimeSpatial->coneInnerAngle = coneValues[0];
        runtimeSpatial->coneOuterAngle = coneValues[1];
        runtimeSpatial->coneInnerGain = coneValues[2];
        runtimeSpatial->coneOuterGain = coneValues[3];
        self->channelParameters->distanceCurve = 0;
        self->channelParameters->distanceParameterA = encodedEmitter->distanceParameterA;
        self->channelParameters->distanceParameterB = encodedEmitter->distanceParameterB;
        self->channelParameters->minDistance = encodedEmitter->minDistance;
        self->channelParameters->maxDistance = encodedEmitter->maxDistance;
        if (encodedCurve != 0) {
            self->distanceCurve = (RuntimeSoundDistanceCurve*)gtl_Audio_Alloc(sizeof(RuntimeSoundDistanceCurve));
            self->distanceCurve->curveKind = encodedCurve->curveKind;
            self->distanceCurve->pointCount = encodedCurve->pointCount;
            self->distanceCurve->points = 0;
            if (encodedCurve->pointCount != 0) {
                self->distanceCurve->points = gtl_Audio_Alloc(encodedCurve->pointCount * sizeof(SoundDistanceGainKey));
                unsigned int keyIndex = 0;
                unsigned int keyByteOffset = 0;
                char* curveCursor = (char*)encodedCurve;
                for (keyIndex = 0; keyIndex < encodedCurve->pointCount; keyIndex++) {
                    *(float*)((char*)self->distanceCurve->points + keyByteOffset) = *(float*)(curveCursor + 0x10);
                    *(float*)((char*)self->distanceCurve->points + keyByteOffset + 4) = *(float*)(curveCursor + 0x14);
                    curveCursor += 8;
                    keyByteOffset += 8;
                }
            }
        }
    }
}

// Widen EV_C configuration bytes and retain the encoded envelope tick units.
static inline void DecodeSoundEnvelope(RuntimeSoundProperty* self, BorrowedSoundPropertyChunks* src) {
    // Phase 3: widen envelope configuration bytes and keep timing in native ticks.
    EncodedSoundEnvelopeChunk* encodedEnvelope = src->envelope;
    if (encodedEnvelope != 0) {
        self->envelope = (RuntimeSoundEnvelope*)gtl_Audio_Alloc(sizeof(RuntimeSoundEnvelope));
        self->envelope->configTag = encodedEnvelope->configTag;
        self->envelope->mode = encodedEnvelope->mode;
        self->envelope->delayTicks = encodedEnvelope->delayTicks;
        self->envelope->attackTicks = encodedEnvelope->attackTicks;
        self->envelope->holdTicks = encodedEnvelope->holdTicks;
        self->envelope->decayTicks = encodedEnvelope->decayTicks;
        self->envelope->releaseTicks = encodedEnvelope->releaseTicks;
        self->envelope->sustainUnits = encodedEnvelope->sustainUnits;
        self->envelope->reservedConfig0 = 0;
        self->envelope->reservedConfig1 = 0;
    }
}

// Decode resource property chunks, copying optional spatial, curve and envelope data.
// Borrow src and every chunk pointer in it; self starts with one reference and
// owns its decoded allocations. A distance curve is copied only inside the EMIT
// branch, even when RO_C exists by itself. Versions before 1.1 double loop indices;
// preserve that compatibility conversion rather than changing the encoded units.
// Example: encoded loop [100,400] becomes [200,800] for version 1.0; version
// 1.1 keeps [100,400]. Absolute units (bytes vs samples) are not established here.
// Splitting the body otherwise makes CodeWarrior inline this exported constructor into factories.
void* gtl_CGSPSoundProperty_Ctor(RuntimeSoundProperty*, BorrowedSoundPropertyChunks*) __attribute__((noinline));
// Initialize reference ownership, then decode each borrowed parameter chunk in phase order.
void* gtl_CGSPSoundProperty_Ctor(RuntimeSoundProperty* self, BorrowedSoundPropertyChunks* src) {
    EncodedSoundBaseChunk* prop = src->baseParameters;
    self->referenceCount = 1;
    self->vtable = (void*)__vt__Q23gtl17CGSPSoundProperty;
    self->baseParameters = 0;
    self->channelParameters = 0;
    self->distanceCurve = 0;
    self->envelope = 0;
    DecodeSoundBaseParameters(self, src, prop);
    DecodeSoundSpatialParameters(self, src);
    DecodeSoundEnvelope(self, src);
    return self;
}

struct RetainedResourceVTable { char reserved00[8]; void (*destroy)(void*, short); };
struct ResourceOffsetTable { char reserved00[8]; unsigned int count; char reserved0C[4]; unsigned int offs[1]; };
struct RetainedResource { unsigned int refs; RetainedResourceVTable* vt; ResourceOffsetTable* data; };
struct ResourceChunkTable { RetainedResource* src; unsigned char* base; unsigned int size; unsigned int cap; };
struct ResourceChunkDesc {
    unsigned int w[8];
    // Use caller-provided placement storage without allocating it.
    static void* operator new(size_t, void* p) { return p; }
    // Leave caller-owned placement storage untouched after construction failure.
    static void operator delete(void*, void*) {}
};

extern char __vt__Q23std11logic_error[], __vt__Q23std12length_error[], soundResourceFormatStrings[], s_sharedCharDeleterVtable[];
extern void* __nw__FUl(unsigned int);
extern void __dla__FPv(void*);
extern "C" void __dt__Q23std9exceptionFv(void*, int);
int gtl_Audio_IndexResourceChunks(void* self, unsigned int* elem, void* names, void* dst);
void* __nwa__FUl(unsigned int size);

struct Allocation_8043C550 {
    char* pointer; char* allocator;
    // Guard an exception-message allocation until its control block takes ownership.
    Allocation_8043C550(char* p, char* a) : pointer(p), allocator(a) {}
    // Delete an exception-message array unless ownership was transferred.
    ~Allocation_8043C550() { if (pointer) __dla__FPv(pointer); }
};
struct ControlBase_8043C550 {
    void* table; unsigned strong, weak;
    // Initialize the exception-message control block reference counts and deleter.
    ControlBase_8043C550() { strong = 1; weak = 1; table = s_sharedCharDeleterVtable; }
};
struct Control_8043C550 : ControlBase_8043C550 {
    char* pointer;
    // Attach the owned message buffer to its initialized control block.
    Control_8043C550(char* p) { pointer = p; }
};
struct ErrorBase_8043C550 {
    void* table;
    // Install the logic-error base interface during exception construction.
    ErrorBase_8043C550() { table = __vt__Q23std11logic_error; }
    // Destroy the exception base without deleting this enclosing allocation.
    ~ErrorBase_8043C550() { __dt__Q23std9exceptionFv(this, -1); }
};
namespace std {
struct length_error : ErrorBase_8043C550 {
    char* text; void* control;
    // Construct an owned length-error message with the matching shared deleter ABI.
    length_error() {
        const char* message = soundResourceFormatStrings;
        struct Allocator { char value; char pad[3]; } allocator;
        allocator.value = 0;
        unsigned int n = strlen(message) + 1;
        text = (char*)__nwa__FUl(2 * n - n);
        {
            char* p = text;
            Allocation_8043C550 allocation(p, &allocator.value);
            control = new Control_8043C550(p);
            allocation.pointer = 0;
        }
        strcpy(text, message);
        table = __vt__Q23std12length_error;
    }
    DECOMP_CT_MEMBER(length_error)
};
}

struct Elem_8043C550 {
    char reserved00[8];
    struct SourceFactoryArgs* src;
    char reserved0C[0x14];
};

// Encoded metadata repacked into the SoundCopyData / SoundRefData source-factory ABI.
struct SourceFactoryArgs {
    char reserved00[8];
    unsigned int name;
    unsigned int format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleData;
    unsigned int sampleBytes;
    unsigned int sampleCount;
    unsigned int loopData;
};

struct SourceFormatInfo {
    unsigned int name;
    unsigned int format;
    unsigned int sampleRate;
    unsigned short sampleFormat;
    unsigned short channelCount;
    unsigned int sampleCount;
    unsigned int loopData;
};

struct SampleBlockRef {
    void* sampleOwner;
    unsigned int sampleData;
    unsigned int sampleBytes;
};

struct SourceInfoPair {
    SourceFormatInfo* a;
    SampleBlockRef* b;
};

struct BufferResourceView {
    char reserved00[4];
    Elem_8043C550* elems;
    unsigned int count;
    char reserved0C[4];
    void* sourceStorage;
    void** sourceSlots; // owned source-reference slots
    unsigned int sourceCount;
    unsigned int sourceCapacity;
    void** propertySlots; // owned decoded-property-reference slots
    unsigned int propertyCount;
    unsigned int propertyCapacity;
};

extern "C" void gtl_Audio_CreateSoundBufferSource(void* pair, void** slot, void* src);
extern "C" void* memcpy(void*, const void*, unsigned int);
extern "C" void* memset(void*, int, unsigned int);

// Throw std::length_error with the sound-resource pool's "vector length error" type name
// (the library's inlined capacity check; never returns).
static inline void ThrowVectorLengthError() {
    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
    std::length_error* error = (std::length_error*)storage.bytes;
    error->__ct();
    exception_runtime::__throw(((exception_runtime::SoundResourceStringPool*)exception_runtime::soundResourceFormatStrings)->lengthErrorType, error,
        (void*)exception_runtime::__dt__Q23std12length_errorFv);
}

// Return the larger capacity-growth operand by reference.
static inline const unsigned int& MaxByRef(const unsigned int& a, const unsigned int& b) { return a < b ? b : a; }

struct Realloc_8043C550 {
    void** start;
    unsigned int elementCount; // number of live elements, NOT an end pointer
    unsigned int elementCapacity;    // allocated element capacity, NOT an end pointer
    unsigned int* capp;
    unsigned int before;
};

// Grow or shrink a list of 4-byte pointers to requestedCount; countSlot/capacitySlot hold its size/capacity.
// Resize pointer storage, zeroing new entries and transferring ownership on growth.
static inline void ResizePointerList(void*** slots, unsigned int* countSlot, unsigned int* capacitySlot, unsigned int requestedCount) {
    // Phase 1: distinguish shrink, in-place growth, and growth needing new storage.
    unsigned int currentCount = *countSlot;
    if (requestedCount > currentCount) {
        unsigned int addedCount = requestedCount - currentCount;
        unsigned int cap = *capacitySlot;
        if (addedCount > cap || currentCount > cap - addedCount) {
            if (currentCount + addedCount - cap > kPointerVectorMaxCount - cap)
                ThrowVectorLengthError();
            unsigned int excess = currentCount + addedCount - cap;
            Realloc_8043C550 growth = { 0, 0, 0, capacitySlot, 0 };
            if (excess > kPointerVectorMaxCount - cap)
                ThrowVectorLengthError();
            unsigned int newcap;
            if (cap < kPointerVectorGrowthThreeFifthsLimit) {
                unsigned int grow = ((cap + 1) * kSoundPointerSlotBytes - (cap + 1)) / 5;
                newcap = cap + MaxByRef(grow, excess);
            } else if (cap < kPointerVectorGrowthHalfLimit) {
                unsigned int grow = (cap + 1) / 2;
                newcap = cap + MaxByRef(grow, excess);
            } else {
                newcap = kPointerVectorMaxCount;
            }
            if (newcap > kPointerVectorMaxCount)
                ThrowVectorLengthError();
            // Allocate and transfer slots before publishing the new allocation.
            growth.start = (void**)gtl_Audio_Alloc(newcap * kSoundPointerSlotBytes);
            growth.elementCapacity = newcap;
            unsigned int oldCount = *countSlot;
            growth.before = oldCount;
            memset((char*)growth.start + oldCount * kSoundPointerSlotBytes + growth.elementCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            void** oldBegin = *slots;
            int oldSize = (oldBegin + *countSlot) - oldBegin;
            growth.elementCount = growth.elementCount + addedCount;
            growth.before = growth.before - oldSize;
            memcpy((char*)growth.start + growth.before * kSoundPointerSlotBytes, oldBegin, oldSize * kSoundPointerSlotBytes);
            memset(oldBegin, 0, oldSize * kSoundPointerSlotBytes);
            growth.elementCount = growth.elementCount + oldSize;
            *countSlot = 0;
            *capacitySlot = growth.elementCapacity;
            *slots = growth.start;
            *countSlot = growth.elementCount;
            if (oldBegin != 0)
                gtl_Audio_Free(oldBegin);
        } else {
            memset((char*)*slots + currentCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            *countSlot += addedCount;
        }
    } else if (requestedCount < currentCount) {
        *countSlot = requestedCount;
    }
}

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
struct ReallocDraft_8043C550 {
    void** start;
    unsigned int elementCount; // number of live elements, NOT an end pointer
    unsigned int elementCapacity;    // allocated element capacity, NOT an end pointer
    unsigned int* capp;
    unsigned int before;
    // Free pointer-vector storage still owned by the growth temporary.
    ~ReallocDraft_8043C550() {
        if (start) {
            elementCount = 0;
            gtl_Audio_Free(start);
        }
    }
};

// Choose bounded pointer-vector capacity; throw length_error on overflow.
static inline unsigned int NextCapacityDraft(unsigned int* capp, unsigned int excess) {
    unsigned int cap = *(volatile unsigned int*)capp;
    if (excess > kPointerVectorMaxCount - cap)
        ThrowVectorLengthError();
    if (cap < kPointerVectorGrowthThreeFifthsLimit) {
        unsigned int grow = ((cap + 1) * 4 - (cap + 1)) / 5;
        cap += MaxByRef(grow, excess);
    } else if (cap < kPointerVectorGrowthHalfLimit) {
        unsigned int grow = (cap + 1) / 2;
        cap += MaxByRef(grow, excess);
    } else {
        cap = kPointerVectorMaxCount;
    }
    return cap;
}

// Resize the buffer-source pointer vector while preserving allocator exception cleanup.
static inline void ResizePointerListDraft(void*** slots, unsigned int* countSlot, unsigned int* capacitySlot, unsigned int requestedCount) {
    unsigned int currentCount = *countSlot;
    if (requestedCount > currentCount) {
        unsigned int addedCount = requestedCount - currentCount;
        unsigned int currentCapacity = *capacitySlot;
        if (addedCount > currentCapacity || currentCount > currentCapacity - addedCount) {
            NextCapacityDraft(capacitySlot, *countSlot + addedCount - currentCapacity);
        } else {
            memset((char*)*slots + currentCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            *countSlot += addedCount;
            return;
        }
        {
            unsigned int excess = *countSlot + addedCount - *capacitySlot;
            ReallocDraft_8043C550 growth;
            growth.start = 0;
            growth.elementCount = 0;
            growth.elementCapacity = 0;
            growth.capp = capacitySlot;
            growth.before = 0;
            unsigned int newcap = NextCapacityDraft(capacitySlot, excess);
            if (newcap > kPointerVectorMaxCount)
                {
                    union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                    std::length_error* error = (std::length_error*)storage.bytes;
                    error->__ct();
                    exception_runtime::__throw(((exception_runtime::SoundResourceStringPool*)exception_runtime::soundResourceFormatStrings)->lengthErrorType, error,
                        (void*)exception_runtime::__dt__Q23std12length_errorFv);
                }
            // Phase 2: construct zero slots and move existing pointers under the unwind guard.
            growth.start = (void**)gtl_Audio_Alloc(newcap * kSoundPointerSlotBytes);
            growth.elementCapacity = newcap;
            unsigned int oldCount = *countSlot;
            growth.before = oldCount;
            memset((char*)growth.start + oldCount * kSoundPointerSlotBytes + growth.elementCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            void** oldBegin = *slots;
            void** oldEnd = oldBegin + *countSlot;
            growth.elementCount += addedCount;
            int oldSize = ((char*)oldEnd - (char*)oldBegin) / kSoundPointerSlotBytes;
            growth.before -= oldSize;
            unsigned int oldBytes = oldSize * kSoundPointerSlotBytes;
            memcpy((char*)growth.start + growth.before * kSoundPointerSlotBytes, oldBegin, oldBytes);
            memset(oldBegin, 0, oldBytes);
            growth.elementCount += oldSize;
            *countSlot = 0;
            // Phase 3: publish new storage; the guard adopts and frees the old allocation.
            unsigned int oldCap = *capacitySlot;
            *capacitySlot = growth.elementCapacity;
            growth.elementCapacity = oldCap;
            void** old = *slots;
            *slots = growth.start;
            growth.start = old;
            *countSlot = growth.elementCount;
            growth.elementCount = 0;
        }
    } else if (requestedCount < currentCount) {
        unsigned int shrink = currentCount - requestedCount;
        *countSlot = *countSlot - shrink;
    }
}

struct ConstructDraft_8043C550 {
    char data[0x18];
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Release runtime object storage through the configured audio allocator.
    static void operator delete(void* p) { gtl_Audio_Free(p); }
    // Decode an indexed chunk into a one-reference runtime sound property.
    ConstructDraft_8043C550(Elem_8043C550* elem) { gtl_CGSPSoundProperty_Ctor((RuntimeSoundProperty*)this, (BorrowedSoundPropertyChunks*)elem); }
};

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Create a buffer source and reference-counted property for each parsed resource chunk.
int gtl_Audio_BuildBufferSources(BufferResourceView* self) {
    // Phase 1: size parallel source/property vectors to the parsed chunk count.
    unsigned int chunkCount = self->count;
    ResizePointerListDraft(&self->sourceSlots, &self->sourceCount, &self->sourceCapacity, chunkCount);
    // Preserved partial clear: chunkCount is an ELEMENT count passed as a BYTE count.
    // Resize initializes newly added pointers; this call does not clear chunkCount pointers.
    memset(self->sourceSlots, 0, chunkCount);
    ResizePointerListDraft(&self->propertySlots, &self->propertyCount, &self->propertyCapacity, chunkCount);
    // Same partial-byte clear for property slots; do not change to chunkCount * sizeof(void*).
    memset(self->propertySlots, 0, chunkCount);
    SourceFormatInfo sourceFormat;
    SampleBlockRef sampleBlock;
    SourceInfoPair pair;
    unsigned int i;
    unsigned int elemOffset;
    unsigned int slotOffset;
    Elem_8043C550* elem;
    i = 0;
    elemOffset = 0;
    slotOffset = 0;
    for (; i < chunkCount; ) {
        if (i < self->count)
            elem = (Elem_8043C550*)((char*)self->elems + elemOffset);
        else
            elem = 0;
        if (elem != 0) {
            // Phase 2: create the source, borrowing samples from retained sourceStorage.
            void** sourceSlot = (void**)((char*)self->sourceSlots + slotOffset);
            if (sourceSlot != 0) {
                SourceFactoryArgs* src = elem->src;
                sourceFormat.name = src->name;
                sourceFormat.format = src->format;
                sourceFormat.sampleRate = src->sampleRate;
                sourceFormat.sampleFormat = src->sampleFormat;
                sourceFormat.channelCount = src->channelCount;
                sourceFormat.sampleCount = src->sampleCount;
                sourceFormat.loopData = src->loopData;
                sampleBlock.sampleOwner = self->sourceStorage;
                sampleBlock.sampleData = src->sampleData;
                sampleBlock.sampleBytes = src->sampleBytes;
                pair.a = &sourceFormat;
                pair.b = &sampleBlock;
                gtl_Audio_CreateSoundBufferSource(&pair, sourceSlot, src);
            }
            // Phase 3: decode independently owned property blocks for the same chunk index.
            ConstructDraft_8043C550* obj;
            void** propertySlots = self->propertySlots;
            obj = new ConstructDraft_8043C550(elem);
            *(ConstructDraft_8043C550**)((char*)propertySlots + slotOffset) = obj;
        }
        elemOffset += kSoundChunkDescriptorBytes;
        slotOffset += kSoundPointerSlotBytes;
        i++;
    }
    return 1;
}
#pragma opt_strength_reduction reset
#pragma opt_lifetimes reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
struct Alloc_8043B91C {
    char value;
    // Initialize the chunk-vector allocator tag.
    Alloc_8043B91C() : value(0) {}
};
// Reduce the logical chunk count without freeing its backing allocation.
static inline void EraseTail(unsigned int* sizep, unsigned int n, const Alloc_8043B91C&) {
    *sizep = *sizep - n;
}
struct Realloc_8043B91C {
    ResourceChunkDesc* start;
    unsigned int elementCount; // number of live elements, NOT an end pointer
    unsigned int elementCapacity;    // allocated element capacity, NOT an end pointer
    unsigned int* capp;
    unsigned int before;
    // Initialize an empty parsed-chunk allocation guard.
    Realloc_8043B91C(unsigned int* p) : start(0), elementCount(0), elementCapacity(0), capp(p), before(0) {}
    // Clear the temporary chunk count and free its backing allocation.
    ~Realloc_8043B91C() {
        if (start) {
            unsigned int n = elementCount;
            EraseTail(&elementCount, n, Alloc_8043B91C());
            gtl_Audio_Free(start);
        }
    }
};
// Choose bounded capacity for 32-byte parsed resource chunks.
static inline unsigned int NextChunkCapacity(unsigned int* capp, unsigned int excess, const Alloc_8043B91C&) {
    unsigned int cap = *capp;
    if (excess > 0x07FFFFFFu - cap)
        ThrowVectorLengthError();
    if (cap < 0x02AAAAAAu) {
        unsigned int grow = ((cap + 1) * 4 - (cap + 1)) / 5;
        cap += MaxByRef(grow, excess);
    } else if (cap < 0x05555554u) {
        unsigned int grow = (cap + 1) / 2;
        cap += MaxByRef(grow, excess);
    } else {
        cap = 0x07FFFFFFu;
    }
    return cap;
}

#pragma use_lmw_stmw on
#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_strength_reduction off
#pragma opt_lifetimes off
// Release the old retained archive before replacing it; src is cleared after destruction.
static inline void ReleaseChunkSourceReference(ResourceChunkTable* self) {
    if (self->src != 0) {
        RetainedResource* old = self->src;
        unsigned int refs = old->refs - 1;
        old->refs = refs;
        if (refs == 0 && old != 0)
            old->vt->destroy(old, 1);
        self->src = 0;
    }
}

// Retain the source resource and parse its indexed chunks into owned 32-byte entries.
// Caller lends src; this table retains one reference even if parsing returns zero.
// A parse failure leaves a partially populated table owned by self. Allocation
// guards clean temporary storage; the table destructor releases the retained archive.
int gtl_Audio_LoadResourceChunks(ResourceChunkTable* self, RetainedResource* src) {
    ReleaseChunkSourceReference(self);
    // Phase 1: replace the retained archive, then logically erase old descriptors.
    unsigned int prevSize = self->size;
    self->src = src;
    EraseTail(&self->size, prevSize, Alloc_8043B91C());
    src->refs = src->refs + 1;
    ResourceOffsetTable* data = self->src->data;
    unsigned int n = data->count;
    unsigned char* names = (unsigned char*)data + 6;
    // Phase 2: ensure descriptor capacity, preserving construction and unwind order.
    // All resize paths join before chunk parsing.
    do {
        if (n > self->size) {
            unsigned int add = n - self->size;
            if (add > self->cap || self->size > self->cap - add) {
                NextChunkCapacity(&self->cap, self->size + add - self->cap, Alloc_8043B91C());
            } else {
                ResourceChunkDesc* p = (ResourceChunkDesc*)self->base + self->size;
                while (add != 0) {
                    ResourceChunkDesc zero = {};
                    new(p) ResourceChunkDesc(zero);
                    --add;
                    ++p;
                    self->size = self->size + 1;
                }
                break;
            }
            {
                unsigned int excess = self->size + add - self->cap;
                Realloc_8043B91C r(&self->cap);
                unsigned int newcap = NextChunkCapacity(&self->cap, excess, Alloc_8043B91C());
                if (newcap > 0x07FFFFFFu)
                    {
                        union { double alignment; unsigned char bytes[sizeof(std::length_error)]; } storage;
                        std::length_error* error = (std::length_error*)storage.bytes;
                        error->__ct();
                        exception_runtime::__throw(((exception_runtime::SoundResourceStringPool*)exception_runtime::soundResourceFormatStrings)->lengthErrorType, error,
                            (void*)exception_runtime::__dt__Q23std12length_errorFv);
                    }
                r.start = (ResourceChunkDesc*)gtl_Audio_Alloc(newcap * 32);
                r.elementCapacity = newcap;
                r.before = self->size;
                ResourceChunkDesc* p = r.start + self->size + r.elementCount;
                while (add != 0) {
                    ResourceChunkDesc zero = {};
                    new(p) ResourceChunkDesc(zero);
                    --add;
                    ++p;
                    ++r.elementCount;
                }
                unsigned char* oldbase = self->base;
                unsigned char* srcEnd = oldbase + self->size * 32;
                ResourceChunkDesc* dstEnd = r.start + r.before;
                while (srcEnd > oldbase) {
                    --dstEnd;
                    srcEnd -= 32;
                    new(dstEnd) ResourceChunkDesc(*(ResourceChunkDesc*)srcEnd);
                    --r.before;
                    ++r.elementCount;
                }
                self->size = 0;
                unsigned int oldCap = self->cap;
                self->cap = r.elementCapacity;
                r.elementCapacity = oldCap;
                unsigned char* old = self->base;
                self->base = (unsigned char*)r.start;
                r.start = (ResourceChunkDesc*)old;
                self->size = r.elementCount;
                r.elementCount = 0;
            }
        } else if (n < self->size) {
            unsigned int shrink = self->size - n;
            EraseTail(&self->size, shrink, Alloc_8043B91C());
        }
    } while (0);
    // Phase 3: parse borrowed indexed chunks into the owned descriptor array.
    ResourceOffsetTable* cursor = data;
    unsigned int i = 0;
    unsigned int offset = 0;
    while (i < data->count) {
        void* elem = (char*)data + cursor->offs[0];
        if (gtl_Audio_IndexResourceChunks(self, (unsigned int*)elem, names, self->base + offset) == 0)
            return 0;
        cursor = (ResourceOffsetTable*)((char*)cursor + 4);
        offset += 32;
        ++i;
    }
    return 1;
}
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma use_lmw_stmw reset

#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

extern "C" void gtl_Audio_CreateSoundStreamSource(void* pair, void** slot);

struct Q_8043D5D4 {
    char reserved00[8];
    unsigned int w[6];
    char blk[2][0x30];
};

struct Elem_8043D5D4 {
    char reserved00[8];
    SourceFactoryArgs* src;
    char reserved0C[0x10];
    Q_8043D5D4* q;
};

struct TmpB_8043D5D4 {
    void* a0;
    unsigned int w[6];
    char blk[2][0x30];
};

struct Pair_8043D5D4 {
    SourceFormatInfo* a;
    TmpB_8043D5D4* b;
};

struct Vec_8043D5D4 {
    char reserved00[4];
    Elem_8043D5D4* elems;
    unsigned int count;
    char reserved0C[4];
    unsigned int flags;
    char reserved14[4];
    void* heapFileName;
    void** sourceSlots; // owned source-reference slots
    unsigned int sourceCount;
    unsigned int sourceCapacity;
    void** propertySlots; // owned decoded-property-reference slots
    unsigned int propertyCount;
    unsigned int propertyCapacity;
    // Borrow the inline or heap resource filename for synchronous source construction.
    void* name() {
        if ((flags >> 31) == 0)
            return (char*)&flags + 1;
        return heapFileName;
    }
};

#pragma inline_depth(8)
#pragma inline_max_size(10000)
#pragma inline_max_total_size(10000)
// Choose bounded pointer-vector capacity for stream sources.
static inline unsigned int NextPointerCapacity(unsigned int* capp, unsigned int excess) {
    return NextCapacityDraft(capp, excess);
}

struct Realloc_8043D5D4 {
    void** start;
    unsigned int elementCount; // number of live elements, NOT an end pointer
    unsigned int elementCapacity;    // allocated element capacity, NOT an end pointer
    unsigned int* capp;
    unsigned int before;
    // Free stream pointer-vector storage still owned by the growth temporary.
    ~Realloc_8043D5D4() {
        if (start) {
            unsigned int n = elementCount;
            EraseTail(&elementCount, n, Alloc_8043B91C());
            gtl_Audio_Free(start);
        }
    }
};
// Resize stream-source storage, preserving old entries and initializing new slots.
static inline void ResizePointerList2(void*** slots, unsigned int* countSlot, unsigned int* capacitySlot, unsigned int requestedCount) {
    unsigned int currentCount = *countSlot;
    if (requestedCount > currentCount) {
        unsigned int addedCount = requestedCount - currentCount;
        if (addedCount > *capacitySlot || currentCount > *capacitySlot - addedCount) {
            NextPointerCapacity(capacitySlot, *(volatile unsigned int*)countSlot + addedCount - *capacitySlot);
        } else {
            memset((char*)*slots + currentCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            *countSlot += addedCount;
            return;
        }
        {
            Realloc_8043D5D4 growth;
            growth.start = 0;
            growth.elementCount = 0;
            growth.elementCapacity = 0;
            growth.capp = capacitySlot;
            growth.before = 0;
            unsigned int excess = *countSlot + addedCount - *capacitySlot;
            unsigned int newcap = NextPointerCapacity(capacitySlot, excess);
            if (newcap > kPointerVectorMaxCount)
                ThrowVectorLengthError();
            growth.start = (void**)gtl_Audio_Alloc(newcap * kSoundPointerSlotBytes);
            growth.elementCapacity = newcap;
            unsigned int oldCount = *countSlot;
            growth.before = oldCount;
            memset((char*)growth.start + oldCount * kSoundPointerSlotBytes + growth.elementCount * kSoundPointerSlotBytes, 0, addedCount * kSoundPointerSlotBytes);
            void** oldBegin = *slots;
            void** oldEnd = oldBegin + *countSlot;
            growth.elementCount += addedCount;
            int oldSize = ((char*)oldEnd - (char*)oldBegin) / kSoundPointerSlotBytes;
            growth.before -= oldSize;
            memcpy((char*)growth.start + growth.before * kSoundPointerSlotBytes, oldBegin, oldSize * kSoundPointerSlotBytes);
            memset(oldBegin, 0, oldSize * kSoundPointerSlotBytes);
            growth.elementCount += oldSize;
            unsigned int oldSizeValue = *countSlot;
            EraseTail(countSlot, oldSizeValue, Alloc_8043B91C());
            unsigned int oldCap = *capacitySlot;
            *capacitySlot = growth.elementCapacity;
            growth.elementCapacity = oldCap;
            void** old = *slots;
            *slots = growth.start;
            growth.start = old;
            *countSlot = growth.elementCount;
            growth.elementCount = 0;
        }
    } else if (requestedCount < currentCount) {
        unsigned int shrink = currentCount - requestedCount;
        EraseTail(countSlot, shrink, Alloc_8043B91C());
    }
}

struct Construct_8043D5D4 {
    char data[0x18];
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Release runtime object storage through the configured audio allocator.
    static void operator delete(void* p) { gtl_Audio_Free(p); }
    // Decode an indexed stream chunk into a one-reference runtime property.
    Construct_8043D5D4(Elem_8043D5D4* elem) { gtl_CGSPSoundProperty_Ctor((RuntimeSoundProperty*)this, (BorrowedSoundPropertyChunks*)elem); }
};

#pragma use_lmw_stmw on
#pragma opt_loop_invariants off
// Build stream sources from format data, file names and cache-flushed ADPCM blocks.
// sourceSlots/sourceCount/sourceCapacity owns created stream-source references; propertySlots/propertyCount/propertyCapacity owns
// one reference-counted decoded property per chunk. Both are parallel to elems.
// The resource destructor releases their objects, then frees vector allocations.
// Source factories consume borrowed stack descriptors synchronously; no stack
// descriptor address is stored by this function. Return 1 is unconditional here.
int gtl_Audio_BuildStreamSources(Vec_8043D5D4* self) {
    // Phase 1: size both output vectors. Existing memset calls take chunkCount
    // BYTES, not chunkCount*sizeof(void*); keep this original behavior unchanged.
    SourceFormatInfo sourceFormat;
    Pair_8043D5D4 sourceArguments;
    unsigned int chunkCount = self->count;
    ResizePointerList2(&self->sourceSlots, &self->sourceCount, &self->sourceCapacity, chunkCount);
    // Preserved partial clear: chunkCount BYTES, not chunkCount pointer slots.
    memset(self->sourceSlots, 0, chunkCount);
    ResizePointerList2(&self->propertySlots, &self->propertyCount, &self->propertyCapacity, chunkCount);
    // Property slots use the same partial-byte clear as buffer resources.
    memset(self->propertySlots, 0, chunkCount);
    unsigned int chunkIndex = 0;
    unsigned int elemOffset = 0;
    unsigned int slotOffset = 0;
    while (chunkIndex < chunkCount) {
        Elem_8043D5D4* chunkEntry;
        if (chunkIndex < self->count)
            chunkEntry = (Elem_8043D5D4*)((char*)self->elems + elemOffset);
        else
            chunkEntry = 0;
        if (chunkEntry != 0) {
            void** sourceSlot = (void**)((char*)self->sourceSlots + slotOffset);
            if (sourceSlot != 0) {
                // Phase 2 per chunk: repack encoded source format into factory ABI.
                SourceFactoryArgs* formatChunk = chunkEntry->src;
                sourceFormat.name = formatChunk->name;
                sourceFormat.format = formatChunk->format;
                sourceFormat.sampleRate = formatChunk->sampleRate;
                sourceFormat.sampleFormat = formatChunk->sampleFormat;
                sourceFormat.channelCount = formatChunk->channelCount;
                sourceFormat.sampleCount = formatChunk->sampleCount;
                sourceFormat.loopData = formatChunk->loopData;
                // Phase 3: add the resource filename, copy six unresolved platform
                // words and two 0x30-byte ADPCM blocks, flushing each for device use.
                Q_8043D5D4* platformChunk = chunkEntry->q;
                TmpB_8043D5D4 platformParameters = {};
                platformParameters.a0 = self->name();
                platformParameters.w[0] = platformChunk->w[0];
                platformParameters.w[1] = platformChunk->w[1];
                platformParameters.w[2] = platformChunk->w[2];
                platformParameters.w[3] = platformChunk->w[3];
                platformParameters.w[4] = platformChunk->w[4];
                platformParameters.w[5] = platformChunk->w[5];
                for (unsigned int channelIndex = 0; channelIndex < 2; channelIndex++) {
                    memcpy(platformParameters.blk[channelIndex], platformChunk->blk[channelIndex], kSoundAdpcmBlockBytes);
                    DCFlushRange(platformParameters.blk[channelIndex], kSoundAdpcmBlockBytes);
                }
                sourceArguments.a = &sourceFormat;
                sourceArguments.b = &platformParameters;
                gtl_Audio_CreateSoundStreamSource(&sourceArguments, sourceSlot);
            }
            // Phase 4: allocate the separate runtime property at the matching index.
            void** propertyList = self->propertySlots;
            Construct_8043D5D4* property = new Construct_8043D5D4(chunkEntry);
            *(Construct_8043D5D4**)((char*)propertyList + slotOffset) = property;
        }
        elemOffset += kSoundChunkDescriptorBytes;
        slotOffset += kSoundPointerSlotBytes;
        chunkIndex++;
    }
    return 1;
}
#pragma use_lmw_stmw reset
#pragma opt_loop_invariants reset
#pragma inline_depth reset
#pragma inline_max_size reset
#pragma inline_max_total_size reset

// Release audio-owned constructor storage.
void dtor_8043C120(void* self) {
    gtl_Audio_Free(self);
}

// Release audio-owned stream constructor storage.
void dtor_8043E35C(void* self) {
    gtl_Audio_Free(self);
}

// Optionally free the sound-property reference base.
void* dtor_8043C4DC(void* self, int deleting) {
    if (self != 0) {
        if (deleting > 0)
            gtl_Audio_Free(self);
    }
    return self;
}

// Return the stream chunk count through the original pointer-width ABI.
void* gtl_CGSPSoundStreamResource_vfunc_1(Obj_8043E118* self) {
    return (void*)self->data->chunkCount;
}

// Return the name stored in an indexed 32-byte stream chunk.
void* gtl_CGSPSoundStreamResource_GetChunkName(Obj_8043E118* self, unsigned int index) {
    Data_8043E118* data = self->data;
    void* item;
    if (index < data->chunkCount)
        item = (char*)data->chunks + index * kSoundChunkDescriptorBytes;
    else
        item = 0;
    if (item != 0)
        return ((ResourceChunkEntry*)item)->name;
    return 0;
}

struct StreamCallBase_8043E590 { char reserved00[4]; };
struct StreamCall_8043E590 : StreamCallBase_8043E590 {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void* get(void*);
};

// Copy the selected stream property's 40-byte base block to the caller.
int gtl_CGSPSoundStreamResource_CopyBaseParams(Obj_8043E118* self, void* index, void* dst) {
    if (dst == 0)
        return 0;
    void* selectedProperty = ((StreamCall_8043E590*)self)->get(index); // slot 0x20 takes the index: r4 still holds it
    if (selectedProperty != 0) {
        void* p = ((SoundResource8043*)selectedProperty)->data;
        if (p != 0) {
            Memory_Copy(dst, p, kSoundBaseParameterBytes);
            return 1;
        }
    }
    return 0;
}

// Return an indexed stream source, or null for an invalid index.
void* gtl_CGSPSoundStreamResource_GetSourceAt(Obj_8043E118* self, unsigned int index) {
    Data_8043E118* data = self->data;
    if (index >= data->sourceCount)
        return 0;
    return data->sourceSlots[index];
}

// Return an indexed stream property, or null for an invalid index.
void* gtl_CGSPSoundStreamResource_GetProperty(Obj_8043E118* self, unsigned int index) {
    Data_8043E118* data = self->data;
    if (index >= data->propertyCount)
        return 0;
    return data->propertySlots[index];
}

extern void __dl__FPv(void*);
void gtl_ISoundStream_SetSource(void*, void*);
void SoundProperties_Clear(void*);

#pragma opt_propagation off
struct ResourceCountStorage { unsigned int data; unsigned int count; };
// Clear and free a vector backing allocation, then optionally delete its owner.
void* ResourceCountStorage_dtor(void* self, int deleting) {
    if (self != 0) {
        unsigned int p;
        unsigned int zero = 0;
        ((ResourceCountStorage*)self)->count = zero;
        if (self != 0) {
            p = *(unsigned int*)self;
            if (p != 0) {
                ((ResourceCountStorage*)self)->count = zero;
                gtl_Audio_Free((void*)p);
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma opt_propagation reset
// Attach this resource and its selected property to a sound stream.
int gtl_CGSPSoundStreamResource_ApplyToStream(Obj_8043E118* self, void* arg, void* dst) {
    if (dst == 0)
        return 0;
    void* r = ((ResourceCall_8043D2AC*)self)->get(arg); // slot 0x1C takes the index: r4 still holds arg
    if (r != 0) {
        gtl_ISoundStream_SetSource(dst, r);
    }
    void* selectedProperty = ((ResourceCall_8043D2AC*)self)->selectProperty(arg);
    if (selectedProperty != 0) {
        gtl_CGSPSoundProperty_ApplyToStream((Struct_8043E448*)selectedProperty, dst);
    }
    return 1;
}

struct Buf_8043E654 {
    char data[0x2C];
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Release constructor storage through the audio-allocation cleanup adapter.
    static void operator delete(void* p) { dtor_8043D260(p); }
    // Clear the newly allocated buffer-resource storage words.
    Buf_8043E654() { SoundProperties_Clear(this); }
};
struct Base_8043E654 {
    unsigned int refs;
    // Destroy the sound-property reference base without deleting the enclosing object.
    ~Base_8043E654() { dtor_8043C4DC(this, 0); }
};
struct Obj_8043E654 : Base_8043E654 {
    void* vt;
    Buf_8043E654* buf;
    // Retain the source storage, parse chunks and build owned buffer sources/properties.
    Obj_8043E654(void* self) {
        refs = 1;
        vt = (void*)__vt__Q23gtl23CGSPSoundBufferResource;
        buf = new Buf_8043E654();
        void* r5 = *(void**)((char*)self + 4);
        void* r4 = *(void**)self;
        ((BufferResourceView*)buf)->sourceStorage = r5;
        void* r3v = *(void**)r5;
        *(unsigned int*)r5 = (unsigned int)r3v + 1;
        if (gtl_Audio_LoadResourceChunks((ResourceChunkTable*)buf, (RetainedResource*)r4) != 0) {
            gtl_Audio_BuildBufferSources((BufferResourceView*)buf);
        }
    }
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Release constructor storage through the audio-allocation cleanup adapter.
    static void operator delete(void* p) { dtor_8043D260(p); }
};
#pragma use_lmw_stmw on
// Allocate a reference-counted buffer resource and build its sources.
int gtl_Audio_CreateSoundBufferResource(void* self, void* out) {
    Obj_8043E654* obj = new Obj_8043E654(self);
    *(void**)out = obj;
    return 1;
}
#pragma use_lmw_stmw reset

// Clear the eleven buffer-resource words in the original store order.
void SoundProperties_Clear(void* self) {
    ((SoundPropertyWords8043*)self)->volume = 0;
    ((SoundPropertyWords8043*)self)->pitch = 0;
    ((SoundPropertyWords8043*)self)->unk_C = 0;
    ((SoundPropertyWords8043*)self)->pan = 0;
    ((SoundPropertyWords8043*)self)->unk_10 = 0;
    ((SoundPropertyWords8043*)self)->unk_14 = 0;
    ((SoundPropertyWords8043*)self)->unk_18 = 0;
    ((SoundPropertyWords8043*)self)->unk_1C = 0;
    ((SoundPropertyWords8043*)self)->loopStart = 0;
    ((SoundPropertyWords8043*)self)->loopEnd = 0;
    ((SoundPropertyWords8043*)self)->unk_28 = 0;
}
// Free the owned envelope, curve, spatial and base parameter blocks.
void* gtl_CGSPSoundProperty_dtor(RuntimeSoundProperty* self, int deleting) {
    if (self != 0) {
        RuntimeSoundEnvelope* data = self->envelope;
        self->vtable = (void*)__vt__Q23gtl17CGSPSoundProperty;
        if (data != 0) {
            gtl_Audio_Free(data);
            self->envelope = 0;
        }
        if (self->distanceCurve != 0) {
            if (self->distanceCurve->points != 0) {
                gtl_Audio_Free(self->distanceCurve->points);
                self->distanceCurve->points = 0;
            }
            if (self->distanceCurve != 0) {
                gtl_Audio_Free(self->distanceCurve);
                self->distanceCurve = 0;
            }
        }
        if (self->channelParameters != 0) {
            gtl_Audio_Free(self->channelParameters);
            self->channelParameters = 0;
        }
        if (self->baseParameters != 0) {
            gtl_Audio_Free(self->baseParameters);
            self->baseParameters = 0;
        }
        if (deleting > 0)
            gtl_Audio_Free(self);
    }
    return self;
}

extern unsigned int s_soundResourceTagGSND, s_soundChunkTagHEAD, s_soundResourceDataChunkTag, s_soundBsicChunkTag;
extern unsigned int s_audioNameChunkTag, s_audioChunkTag_EV_C, s_audioChunkTag_RO_C, s_soundResourceEmitChunkTag, s_soundResourceTagGCEX;

void gtl_String_ReplaceRange(void*, unsigned int, unsigned int, const char*, const char*, const void*);
struct StreamCreateArgs { RetainedResource* source; const char* volatile name; };
struct Alloc_8043E700 { char value; };
#pragma inline_depth(8)
struct String_8043E700 {
    volatile unsigned int words[3];
    // Read the inline or heap resource-filename length.
    unsigned int size() const {
        if ((words[0] >> 31) == 0)
            return *(const unsigned char*)this & 0x7f;
        return words[1];
    }
    // Replace a filename character range using the by-value allocator-tag ABI.
    void replace(unsigned int pos, unsigned int len, const char* name, const Alloc_8043E700& allocator) {
        Alloc_8043E700 copy = allocator;
        unsigned int size = strlen(name);
        gtl_String_ReplaceRange(this, pos, len, name, name + size, &copy);
    }
    // Replace the complete filename with a borrowed C string.
    void assign(const char* name) {
        Alloc_8043E700 allocator;
        replace(0, size(), name, allocator);
    }
};
struct StreamData_8043E700 {
    unsigned int fields[4];
    String_8043E700 str;
    unsigned int tail[6];
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Clear the newly allocated stream-resource storage words.
    StreamData_8043E700() { SoundStreamProperties_Clear((Struct_8043D598*)this); }
};
struct StreamBase_8043E700 {
    unsigned int refs;
    void* vt;
    // Start the runtime stream-resource base with one reference.
    StreamBase_8043E700() { refs = 1; }
    // Destroy the stream-resource base without deleting its enclosing allocation.
    ~StreamBase_8043E700() { StreamBase_dtor(this, 0); }
};
struct Stream_8043E700 : StreamBase_8043E700 {
    StreamData_8043E700* data;
    // Allocate runtime object storage through the configured audio allocator.
    static void* operator new(size_t size) { return gtl_Audio_Alloc(size); }
    // Release runtime object storage through the configured audio allocator.
    static void operator delete(void* p) { gtl_Audio_Free(p); }
    // Copy the resource filename, retain/parse its source and construct stream outputs.
    Stream_8043E700(StreamCreateArgs* args) {
        vt = __vt__Q23gtl23CGSPSoundStreamResource;
        data = new StreamData_8043E700;
        data->str.assign(args->name);
        gtl_Audio_LoadResourceChunks((ResourceChunkTable*)data, args->source);
        gtl_Audio_BuildStreamSources((Vec_8043D5D4*)data);
    }
};
#pragma use_lmw_stmw on
// Allocate a reference-counted stream resource and build its sources.
int gtl_Audio_CreateSoundStreamResource(StreamCreateArgs* args, Stream_8043E700** out) {
    *out = new Stream_8043E700(args);
    return 1;
}
#pragma use_lmw_stmw reset
#pragma inline_depth reset

// Index recognized GSND subchunks into the fixed eight-pointer descriptor.
// Borrow chunk, formatVersionBytes, and name/chunk payloads;
// dst must hold eight writable pointers. No allocation/retain occurs: the source
// owner must outlive all descriptors. Each length/count is trusted, with no bounds
// checks here. Unknown/container tags are skipped; the last duplicate tag wins.
// Slot map: version, name payload, DATA, BSIC, EMIT, EV_C, RO_C, GCEX.
int gtl_Audio_IndexResourceChunks(void* self, unsigned int* chunk, void* formatVersionBytes, void* dst) {
    void** out = (void**)dst;
    memset(out, 0, kSoundChunkDescriptorBytes);
    unsigned int count = chunk[3];
    out[kChunkFormatVersion] = formatVersionBytes;
    unsigned int resourceTag = s_soundResourceTagGSND;
    unsigned int headerTag = s_soundChunkTagHEAD;
    unsigned int dataTag = s_soundResourceDataChunkTag;
    unsigned int baseParametersTag = s_soundBsicChunkTag;
    unsigned int nameTag = s_audioNameChunkTag;
    unsigned int envelopeTag = s_audioChunkTag_EV_C;
    unsigned int rolloffCurveTag = s_audioChunkTag_RO_C;
    unsigned int emitterTag = s_soundResourceEmitChunkTag;
    unsigned int platformExtensionTag = s_soundResourceTagGCEX;
    for (unsigned int i = 0; i < count; i++) {
        unsigned int tag = chunk[0];
        if (tag == resourceTag) {
        } else if (tag == headerTag) {
        } else if (tag == dataTag) {
            out[kChunkSourceData] = chunk;
        } else if (tag == baseParametersTag) {
            out[kChunkBaseParameters] = chunk;
        } else if (tag == nameTag) {
            out[kChunkName] = chunk + 3;
        } else if (tag == envelopeTag) {
            out[kChunkEnvelope] = chunk;
        } else if (tag == rolloffCurveTag) {
            out[kChunkRolloffCurve] = chunk;
        } else if (tag == emitterTag) {
            out[kChunkEmitter] = chunk;
        } else if (tag == platformExtensionTag) {
            out[kChunkPlatformExtension] = chunk;
        }
        chunk = (unsigned int*)((char*)chunk + chunk[1]);
    }
    return 1;
}
} // extern "C"
