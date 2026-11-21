// Resource-file chunk sort: three-record pivot preparation for SortByOffset and
// SortByOffsetRecursive (gtl_image_font_resource_adapters.cpp/gtl_resource_chunk_sort.cpp). No gameplay tuning knobs.
// Mod lookup: ordering key is GtlChunk::fileRelativeOffset; change comparison
// policy together with those callers' partition/insertion comparisons.
// Translation unit 0x803D7E18-0x803D7FE8 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"

#pragma opt_propagation off
#pragma opt_common_subs off
#pragma opt_lifetimes off

extern "C" {

struct GtlChunk {
    s32 payloadWord0; // +0: opaque to sorting; directory entries use a tag here.
    s32 elementCount; // +4: count * elementBytes is payload byte size.
    s32 elementBytes; // +8: stride in bytes (gtl_image_font_resource_adapters.cpp resource writer).
    u32 fileRelativeOffset; // +C: unsigned byte offset, not a process pointer.
};

// Reorder the 16-byte RECORDS so offset(**arg0) <= offset(**arg2) <= offset(**arg1).
// arg0/arg1/arg2 and their pointees must be valid writable storage. Pointer slots
// are borrowed, never assigned; complete records move, preserving payload/key
// association. Require three pairwise-disjoint records, and pointer-slot storage
// must not overlap any record. In particular a==b with a different c cannot satisfy
// the advertised a<=c<=b postcondition; aliased pivots are not a supported input.
// Equal keys are permitted; no stable-order guarantee. No allocations, callbacks,
// ownership transfers or error result. Native Wii words (big endian), four-byte
// alignment; this is an in-memory resource directory view, not endian conversion.
void gtl_ResourceChunks_SortThreeByOffset(GtlChunk **arg0, GtlChunk **arg1, GtlChunk **arg2) {
    GtlChunk *b;
    u32 cOff;
    u32 aOff;
    u32 bOff;
    GtlChunk *c;
    GtlChunk *a;
    a = *arg0;
    c = *arg2;
    aOff = a->fileRelativeOffset;
    cOff = c->fileRelativeOffset;
    b = *arg1;
    bOff = b->fileRelativeOffset;
    // Snapshot predicates before mutation: target order is a, c, b, not a, b, c.
    int aBeforeC = aOff <= cOff;
    int cBeforeB = cOff <= bOff;

    if (aBeforeC && cBeforeB) {
        return;
    }
    if (!aBeforeC && !cBeforeB) {
        GtlChunk tmp = *a;
        *a = *b;
        *b = tmp;
        return;
    }
    if (bOff < aOff) {
        GtlChunk tmp = *a;
        *a = *b;
        *b = tmp;
    }
    if (aBeforeC) {
        GtlChunk *q;
        GtlChunk *p;
        p = *arg1;
        q = *arg2;
        GtlChunk tmp = *p;
        *p = *q;
        *q = tmp;
        return;
    }
    GtlChunk *q;
    GtlChunk *p;
    p = *arg0;
    q = *arg2;
    GtlChunk tmp = *p;
    *p = *q;
    *q = tmp;
}

} // extern "C"
