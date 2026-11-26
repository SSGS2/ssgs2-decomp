// Resource-file chunk sort: three-record pivot preparation for SortByOffset and
// SortByOffsetRecursive (gtl_image_font_resource_adapters.cpp/gtl_resource_chunk_sort.cpp). No gameplay tuning knobs.
// Mod lookup: ordering key is GtlChunk::fileRelativeOffset; change comparison
// policy together with those callers' partition/insertion comparisons.

#include "types.h"

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off

extern "C" {

struct GtlChunk {
    s32 payloadWord0;       // +0: opaque to sorting; directory entries use a tag here.
    s32 elementCount;       // +4: count * elementBytes is payload byte size.
    s32 elementBytes;       // +8: stride in bytes (gtl_image_font_resource_adapters.cpp resource writer).
    u32 fileRelativeOffset; // +C: unsigned byte offset, not a process pointer.
};

// Reorder the 16-byte RECORDS so offset(**firstSlot) <= offset(**middleSlot) <= offset(**lastSlot).
// firstSlot/lastSlot/middleSlot and their pointees must be valid writable storage. Pointer slots
// are borrowed, never assigned; complete records move, preserving payload/key
// association. Require three pairwise-disjoint records, and pointer-slot storage
// must not overlap any record. In particular firstRecord==lastRecord with a different middleRecord cannot satisfy
// the advertised firstRecord<=middleRecord<=lastRecord postcondition; aliased pivots are not a supported input.
// Equal keys are permitted; no stable-order guarantee. No allocations, callbacks,
// ownership transfers or error result. Native Wii words (big endian), four-byte
// alignment; this is an in-memory resource directory view, not endian conversion.
void gtl_ResourceChunks_SortThreeByOffset(GtlChunk** firstSlot, GtlChunk** lastSlot, GtlChunk** middleSlot) {
    GtlChunk* lastRecord;
    u32 cOff;
    u32 aOff;
    u32 bOff;
    GtlChunk* middleRecord;
    GtlChunk* firstRecord;
    firstRecord = *firstSlot;
    middleRecord = *middleSlot;
    aOff = firstRecord->fileRelativeOffset;
    cOff = middleRecord->fileRelativeOffset;
    lastRecord = *lastSlot;
    bOff = lastRecord->fileRelativeOffset;
    // Snapshot predicates before mutation: target order is firstRecord, middleRecord, lastRecord, not firstRecord, lastRecord, middleRecord.
    int aBeforeC = aOff <= cOff;
    int cBeforeB = cOff <= bOff;

    if (aBeforeC && cBeforeB) {
        return;
    }
    if (!aBeforeC && !cBeforeB) {
        GtlChunk tmp = *firstRecord;
        *firstRecord = *lastRecord;
        *lastRecord = tmp;
        return;
    }
    if (bOff < aOff) {
        GtlChunk tmp = *firstRecord;
        *firstRecord = *lastRecord;
        *lastRecord = tmp;
    }
    if (aBeforeC) {
        GtlChunk* q;
        GtlChunk* p;
        p = *lastSlot;
        q = *middleSlot;
        GtlChunk tmp = *p;
        *p = *q;
        *q = tmp;
        return;
    }
    GtlChunk* q;
    GtlChunk* p;
    p = *firstSlot;
    q = *middleSlot;
    GtlChunk tmp = *p;
    *p = *q;
    *q = tmp;
}

} // extern "C"
