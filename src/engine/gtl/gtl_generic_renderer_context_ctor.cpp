// GTL renderer context: initialize the four owned scratch-buffer descriptors.
// API/mod lookup: ResizeScratchBuffers (gtl_generic_renderer_scratch_buffers.cpp) allocates these arrays;
// ContextDtor (gtl_generic_renderer_context.cpp) frees them in reverse order using GenericResource_Free.
// ClearDrawQueue (gtl_renderer_draw_queue_clear.cpp) releases references in the first array's 0x50-byte
// draw records and resets the DISTINCT active count at +0x190.
// This constructor exposes no rendering/frame-rate knob and does not allocate.
// Buffer sizing belongs in ResizeScratchBuffers, not these zero initializers.
// Translation unit 0x803DD064-0x803DD09C (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

struct GtlGenericRenderer {
    // Wii 32-bit layout. Each named descriptor group
    // is {allocation address, constructed element count, allocation capacity}.
    // +000..183 contains culling state, matrices and mode; left untouched.
    char unk0[0x184];
    int drawRecordStorage; // draw-record allocation (0x50 bytes per element)
    int drawRecordCount; // constructed draw-record count; not the active draw count
    int drawRecordCapacity; // draw-record capacity, in elements
    char activeDrawCount[4]; // active draw count; initialized by the higher layer
    int firstScratchStorage; // first u32 scratch-array allocation
    int firstScratchCount; // first array element count
    int firstScratchCapacity; // first array capacity, in u32 elements
    int secondScratchStorage; // second u32 scratch-array allocation
    int secondScratchCount; // second array element count
    int secondScratchCapacity; // second array capacity, in u32 elements
    int thirdScratchStorage; // third u32 scratch-array allocation
    int thirdScratchCount; // third array element count
    int thirdScratchCapacity; // third array capacity, in u32 elements
};

extern "C" {

// Required writable self spans at least 0x1B8 bytes, with no live allocations in
// these four descriptors. Writes only their 12 words: empty/null, count=cap=0.
// Other bytes, notably +0x190, stay untouched; caller must initialize them before
// drawing. Re-running this on live buffers loses ownership and leaks them.
// Returns no status; cannot report allocation failure because none is attempted.
void gtl_GenericRenderer_ContextCtor(GtlGenericRenderer* self) {
    self->drawRecordStorage = 0;
    self->drawRecordCount = 0;
    self->drawRecordCapacity = 0;
    self->firstScratchStorage = 0;
    self->firstScratchCount = 0;
    self->firstScratchCapacity = 0;
    self->secondScratchStorage = 0;
    self->secondScratchCount = 0;
    self->secondScratchCapacity = 0;
    self->thirdScratchStorage = 0;
    self->thirdScratchCount = 0;
    self->thirdScratchCapacity = 0;
}

} // extern "C"
