// gtl generic renderer scratch storage; InitializeContext (gtl_generic_renderer_init.cpp) calls
// this after clearing counts. Mod lookup: ResizeSizes maps three requested slot
// capacities to the vectors below; their first two also size the draw-record
// backing array. ClearDrawQueue (gtl_renderer_draw_queue_clear.cpp) releases refs in ACTIVE draw records;
// that active count is +190, distinct from backing record count +188 here.
// No gameplay tuning. Allocator/free: gtl_GenericResource_Allocate/Free;
// context destruction frees all four backing allocations (gtl_generic_renderer_context.cpp).

#include "game/stdexcept_local.h"

typedef unsigned int u32;
typedef unsigned char u8;

// 0x50-byte draw-record backing element; +8/+C are resource refs in active
// records (gtl_renderer_draw_queue_clear.cpp). Resizing copies bits, never retains/releases those refs.
struct ScratchEntry {
    u32 reserved00[0x14];
};

struct ScratchRenderer {
    char reserved000[0x184];
    ScratchEntry* drawRecords;
    u32 drawRecordCount;
    u32 drawRecordCapacity;
    char activeDrawCount190[4]; // separate queue ownership; not resized here
    u32* firstScratch;
    u32 firstScratchCount;
    u32 firstScratchCapacity;
    u32* secondScratch;
    u32 secondScratchCount;
    u32 secondScratchCapacity;
    u32* thirdScratch;
    u32 thirdScratchCount;
    u32 thirdScratchCapacity;
};

// Native u32 counts, not bytes. Actual draw-category names are unrecovered;
// first/second/third identify the EXACT collections rather than guessing opaque/
// transparent categories. +194/+1A0/+1AC are ALLOCATION POINTERS; used counts
// +198/+1A4/+1B0 are cleared by gtl_generic_renderer_scratch_clear.cpp; capacities are +19C/+1A8/+1B4.
// Consumer boundary: DrawQueuedPrimitives (gtl_model_culling_billboard_curve.cpp)
// does not read these lists. No implemented producer here inserts a word in any
// of these three lists. Consequently their element interpretation (indices,
// addresses, sort keys, etc.) cannot be established from the available source.
// Capacity edits need only preserve word storage/counts; edits interpreting
// elements must first recover that draw consumer. Do not infer vertex/index
// buffer semantics from the renderer's separate GPU-buffer APIs.
struct ResizeSizes {
    u32 firstScratchSlots;  // +00 -> vector +194; also contributes draw records
    u32 secondScratchSlots; // +04 -> vector +1A0; also contributes draw records
    u32 thirdScratchSlots;  // +08 -> vector +1AC; no draw-record contribution
};

extern "C" void gtl_GenericResource_Free(void* p);

struct TmpVec {
    void* begin;
    u32 size;
    u32 cap;
    // Empty storage guard; it owns allocation bytes, never the stored word payloads.
    TmpVec() {
        begin = 0;
        size = 0;
        cap = 0;
    }
    // Free replacement on unwind, or old allocation after ownership exchange.
    ~TmpVec() {
        if (begin) {
            u32 count = size;
            size -= count;
            gtl_GenericResource_Free(begin);
        }
    }
};

struct GrowTmpVec : TmpVec {
    u32* owner;
    u32 offset;
    // Track the old-prefix transfer boundary while constructing a new record array.
    GrowTmpVec(u32* p) : owner(p), offset(0) {}
};

extern "C" {

void* gtl_GenericResource_Allocate(u32 size);
void gtl_GenericResource_Free(void* p);
void* memcpy(void* dst, const void* src, u32 n);
void* memset(void* dst, int c, u32 n);

extern ScratchRenderer* g_genericRendererContext;
extern char s_sharedStringPool[];

#pragma push
#pragma use_lmw_stmw on
#pragma opt_propagation off
// Prepare draw-record backing size first, then reserve each word vector in order.
// sizes is required and must not alias the renderer or any backing allocation;
// g_genericRendererContext must name an initialized, exclusively accessed context. Existing
// counts must not exceed capacities. first+second must not overflow u32 and
// must fit 0x03333333 records; each word request must fit 0x3FFFFFFF slots.
// No status result: length_error/allocator exceptions propagate. Allocation must
// return writable storage or throw; null is not handled. Each collection commits
// independently: failure in a later collection retains earlier successful changes.
// No global rollback, no payload destruction, no reference-count adjustments.
// For live draw records, caller must settle ownership before shrinking backing.
// New draw records are zeroed; old records survive; word-vector live counts are
// preserved (reserve only, no added live entries). Reallocation invalidates slot
// pointers. Retired allocations are freed by TmpVec/GrowTmpVec after publication.
// Volatile duplicate capacity checks and try/rethrow scopes preserve actual
// preflight reads and exception cleanup; they are not separate resize requests.
void gtl_GenericRenderer_ResizeScratchBuffers(const ResizeSizes* sizes) {
    u32 requestedDrawRecords = sizes->firstScratchSlots + sizes->secondScratchSlots;
    ScratchRenderer* r = g_genericRendererContext;

    // Phase 1: resize backing records, not the active draw queue at +190.
    if (requestedDrawRecords > r->drawRecordCount) {
        u32 excess = requestedDrawRecords - r->drawRecordCount;
        if (excess > r->drawRecordCapacity || r->drawRecordCount > r->drawRecordCapacity - excess) {
            volatile ScratchRenderer* view = r;
            u32 need0 = view->drawRecordCount + excess - r->drawRecordCapacity;
            u32 cap0 = view->drawRecordCapacity;
            u32 dead = cap0;
            if (need0 > 0x3333333 - cap0) {
                std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
            }
            if (cap0 < 0x1111111) {
                u32 t = 3 * (cap0 + 1) / 5;
                const u32* use = (t >= need0) ? &t : &need0;
                dead += *use;
            } else if (cap0 < 0x2222222) {
                u32 t = (cap0 + 1) / 2;
                const u32* use = (t >= need0) ? &t : &need0;
                dead += *use;
            } else {
                dead = 0x3333333;
            }
            GrowTmpVec tmp(&r->drawRecordCapacity);
            u32 need = view->drawRecordCount + excess - view->drawRecordCapacity;
            u32 cap = r->drawRecordCapacity;
            if (need > 0x3333333 - cap) {
                std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
            }
            u32 newcap;
            if (cap < 0x1111111) {
                u32 t = 3 * (cap + 1) / 5;
                const u32* use = (t >= need) ? &t : &need;
                newcap = cap + *use;
            } else if (cap < 0x2222222) {
                u32 t = (cap + 1) / 2;
                const u32* use = (t >= need) ? &t : &need;
                newcap = cap + *use;
            } else {
                newcap = 0x3333333;
            }
            if (newcap > 0x3333333) {
                std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
            }
            // Stage zero-filled new tail, then transfer existing prefix backwards.
            tmp.begin = gtl_GenericResource_Allocate(newcap * 0x50);
            tmp.cap = newcap;
            tmp.offset = r->drawRecordCount;
            ScratchEntry* p = (ScratchEntry*)tmp.begin + tmp.offset + tmp.size;
            while (excess != 0) {
                ScratchEntry z = {};
                if (p) {
                    try {
                        *p = z;
                    } catch (...) {
                        throw;
                    }
                }
                p++;
                tmp.size++;
                excess--;
            }

            ScratchEntry* oldPtr = r->drawRecords;
            ScratchEntry* src = oldPtr + r->drawRecordCount;
            ScratchEntry* d = (ScratchEntry*)tmp.begin + tmp.offset;
            while (src > oldPtr) {
                src--;
                d--;
                if (d) {
                    try {
                        *d = *src;
                    } catch (...) {
                        throw;
                    }
                }
                tmp.offset--;
                tmp.size++;
            }

            // Commit pointer/capacity/count; swap old allocation into cleanup guard.
            u32 size = tmp.size;
            r->drawRecordCount = 0;
            u32 newCap = tmp.cap;
            u32 oldCap = r->drawRecordCapacity;
            ScratchEntry* oldP = r->drawRecords;
            r->drawRecordCapacity = newCap;
            r->drawRecords = (ScratchEntry*)tmp.begin;
            tmp.cap = oldCap;
            tmp.begin = oldP;
            r->drawRecordCount = size;
            tmp.size = 0;
        } else {
            ScratchEntry* p = r->drawRecords + r->drawRecordCount;
            while (excess != 0) {
                ScratchEntry tmp = {};
                if (p) {
                    try {
                        *p = tmp;
                    } catch (...) {
                        throw;
                    }
                }
                p++;
                r->drawRecordCount++;
                excess--;
            }
        }
    } else if (requestedDrawRecords < r->drawRecordCount) {
        u32 difference = r->drawRecordCount - requestedDrawRecords;
        r->drawRecordCount -= difference;
    }

    r = g_genericRendererContext;
    // Phase 2: reserve firstScratch (+194), retaining its current live count.
    // Each reserve below allocates first, copies live words, clears old words,
    // publishes new pointer/capacity/count, then frees old storage at scope exit.
    u32 a = sizes->firstScratchSlots;
    if (a > r->firstScratchCapacity) {
        TmpVec tmp;
        if (a > 0x3fffffff) {
            std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
        }
        tmp.begin = gtl_GenericResource_Allocate(a * 4);
        tmp.cap = a;
        u32 count = (u32)(((const char*)(r->firstScratch + r->firstScratchCount) - (const char*)r->firstScratch) / 4);
        u32* oldPtr = r->firstScratch;
        memcpy((u32*)tmp.begin + tmp.size, oldPtr, count * 4);
        memset(oldPtr, 0, count * 4);
        r->firstScratchCount = 0;
        u32 size = tmp.size;
        u32 oldCap = r->firstScratchCapacity;
        u32 newCap = tmp.cap;
        size += count;
        u32* newPtr = (u32*)tmp.begin;
        r->firstScratchCapacity = newCap;
        u32* oldP = r->firstScratch;
        tmp.size = size;
        r->firstScratch = newPtr;
        tmp.size = r->firstScratchCount;
        tmp.cap = oldCap;
        tmp.begin = oldP;
        r->firstScratchCount = size;
    }

    r = g_genericRendererContext;
    // Phase 3: reserve secondScratch (+1A0) after phase 2 completes without throwing.
    // A throw here retains earlier commits; a phase-2 throw skips this phase.
    u32 b = sizes->secondScratchSlots;
    if (b > r->secondScratchCapacity) {
        TmpVec tmp;
        if (b > 0x3fffffff) {
            std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
        }
        tmp.begin = gtl_GenericResource_Allocate(b * 4);
        tmp.cap = b;
        u32 count =
            (u32)(((const char*)(r->secondScratch + r->secondScratchCount) - (const char*)r->secondScratch) / 4);
        u32* oldPtr = r->secondScratch;
        memcpy((u32*)tmp.begin + tmp.size, oldPtr, count * 4);
        memset(oldPtr, 0, count * 4);
        r->secondScratchCount = 0;
        u32 size = tmp.size;
        u32 oldCap = r->secondScratchCapacity;
        u32 newCap = tmp.cap;
        size += count;
        u32* newPtr = (u32*)tmp.begin;
        r->secondScratchCapacity = newCap;
        u32* oldP = r->secondScratch;
        tmp.size = size;
        r->secondScratch = newPtr;
        tmp.size = r->secondScratchCount;
        tmp.cap = oldCap;
        tmp.begin = oldP;
        r->secondScratchCount = size;
    }

    r = g_genericRendererContext;
    // Phase 4: reserve thirdScratch (+1AC); earlier commits survive failure here.
    u32 c = sizes->thirdScratchSlots;
    if (c > r->thirdScratchCapacity) {
        TmpVec tmp;
        if (c > 0x3fffffff) {
            std::length_error(s_sharedStringPool, s_sharedStringPool + 0x14);
        }
        tmp.begin = gtl_GenericResource_Allocate(c * 4);
        tmp.cap = c;
        u32 count = (u32)(((const char*)(r->thirdScratch + r->thirdScratchCount) - (const char*)r->thirdScratch) / 4);
        u32* oldPtr = r->thirdScratch;
        memcpy((u32*)tmp.begin + tmp.size, oldPtr, count * 4);
        memset(oldPtr, 0, count * 4);
        r->thirdScratchCount = 0;
        u32 size = tmp.size;
        u32 oldCap = r->thirdScratchCapacity;
        u32 newCap = tmp.cap;
        size += count;
        u32* newPtr = (u32*)tmp.begin;
        r->thirdScratchCapacity = newCap;
        u32* oldP = r->thirdScratch;
        tmp.size = size;
        r->thirdScratch = newPtr;
        tmp.size = r->thirdScratchCount;
        tmp.cap = oldCap;
        tmp.begin = oldP;
        r->thirdScratchCount = size;
    }
}

#pragma pop
} // extern "C"
