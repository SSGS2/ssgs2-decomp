// Generic renderer scratch API, 0x803DC378: discard the three used-word counts.
// InitializeContext (gtl_generic_renderer_init.cpp) installs g_genericRendererContext, clears these counts and the
// separate draw queue, then calls ResizeScratchBuffers (gtl_generic_renderer_scratch_buffers.cpp). That file
// owns allocation/growth/copying; ContextDtor (gtl_generic_renderer_context.cpp) releases storage.
// No gameplay tuning: scratch capacity is supplied to InitializeContext as its
// three-word sizes record. Clearing usage never shrinks capacity or frees data.

// Partial renderer schema, Wii 32-bit words. Each scratch vector is
// {allocation address, used words, capacity words}; used/capacity are element
// counts, four bytes per element (ResizeScratchBuffers multiplies by 4).
// The separate draw queue is reset by ClearDrawQueue (gtl_renderer_draw_queue_clear.cpp).
// The three arrays retain their ordinal identity until their draw consumers
// establish their contents; do not label them vertex/index buffers.
struct RendererScratchCounts {
    char prefix[0x194];
    unsigned int firstData; int firstUsed; unsigned int firstCapacity;
    unsigned int secondData; int secondUsed; unsigned int secondCapacity;
    unsigned int thirdData; int thirdUsed; unsigned int thirdCapacity;
};
typedef char ScratchFirstOffset[((int)&((RendererScratchCounts*)0)->firstUsed == 0x198) ? 1 : -1];
typedef char ScratchSecondOffset[((int)&((RendererScratchCounts*)0)->secondUsed == 0x1A4) ? 1 : -1];
typedef char ScratchThirdOffset[((int)&((RendererScratchCounts*)0)->thirdUsed == 0x1B0) ? 1 : -1];
enum RendererScratchCountOffset {
    kFirstScratchUsed = (int)&((RendererScratchCounts*)0)->firstUsed,
    kSecondScratchUsed = (int)&((RendererScratchCounts*)0)->secondUsed,
    kThirdScratchUsed = (int)&((RendererScratchCounts*)0)->thirdUsed
};

extern "C" {

extern void* g_genericRendererContext;

// Requires an installed live renderer and exclusive access to its scratch state.
// Clears used counts only; leaves allocations, contents, capacities and draw queue
// untouched. The repeated signed/unsigned read implements vector erase-all as
// count -= count. Retain both source loads and their types: replacing with zero
// can remove loads in the matched object, even though ordinary RAM ends at zero.
// Use raw signed/unsigned lvalues at these checked field offsets: accessing the
// same typed member twice lets CW combine the loads and changes original code.
void gtl_GenericRenderer_ClearScratchCounts(void) {
    *(int*)((char*)g_genericRendererContext + kFirstScratchUsed) = *(int*)((char*)g_genericRendererContext + kFirstScratchUsed) - *(unsigned int*)((char*)g_genericRendererContext + kFirstScratchUsed);
    *(int*)((char*)g_genericRendererContext + kSecondScratchUsed) = *(int*)((char*)g_genericRendererContext + kSecondScratchUsed) - *(unsigned int*)((char*)g_genericRendererContext + kSecondScratchUsed);
    *(int*)((char*)g_genericRendererContext + kThirdScratchUsed) = *(int*)((char*)g_genericRendererContext + kThirdScratchUsed) - *(unsigned int*)((char*)g_genericRendererContext + kThirdScratchUsed);
}

} // extern "C"
