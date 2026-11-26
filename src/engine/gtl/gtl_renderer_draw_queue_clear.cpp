// Renderer draw-queue teardown. InitializeContext (gtl_generic_renderer_init.cpp) invokes this before
// reusing scratch buffers. Queue allocation/capacity belongs to ContextCtor,
// ResizeScratchBuffers (gtl_generic_renderer_scratch_buffers.cpp) and ContextDtor (gtl_generic_renderer_context.cpp), not this routine.
// Mod lookup: reference release order is below; storage sizing is in ResizeScratchBuffers.

typedef unsigned int u32;

struct GenericRefBase {
    int rc;
};

struct GenericRef : GenericRefBase {
    virtual void Release(int);
};

struct GenericDrawEntry {
    char reserved00[8];
    GenericRef* a; // +8: first retained resource; concrete renderer role unresolved.
    GenericRef* b; // +C: second retained resource; concrete renderer role unresolved.
    char unk10[0x40];
};

struct GenericRenderer {
    char reserved00[0x184];
    GenericDrawEntry* queue;
    char reserved188[8];
    u32 count;
};

extern "C" {

extern GenericRenderer* g_genericRendererContext;

// Consume count live entries in 0x50-byte slots, dropping +8 then +C references.
// Renderer, queue and BOTH resource pointers in every live entry are required:
// the rc decrement precedes the null test. Release(1) is the deleting virtual
// operation, called only on transition to zero. Aliased resources need one
// retained reference per slot. No storage is freed or entry pointer cleared.
// Count at +0x190 is published as zero only after all releases finish. A throwing
// callback leaves partial decrements and the old count; there is no rollback.
// Callbacks must not reenter clearing or invalidate queue/live entries. The loop
// reloads global context/count, so caching these would change callback behavior.
void gtl_GenericRenderer_ClearDrawQueue(void) {
    GenericDrawEntry* entry;
    GenericRef* resource;
    u32 entryByteOffset = 0;
    u32 entryIndex = 0;
    while (entryIndex < g_genericRendererContext->count) {
        entry = (GenericDrawEntry*)((char*)g_genericRendererContext->queue + entryByteOffset);
        resource = entry->a;
        if (--resource->rc == 0 && resource) {
            resource->Release(1);
        }
        resource = entry->b;
        if (--resource->rc == 0 && resource) {
            resource->Release(1);
        }
        entryByteOffset += sizeof(GenericDrawEntry);
        entryIndex++;
    }
    g_genericRendererContext->count = 0;
}

} // extern "C"
