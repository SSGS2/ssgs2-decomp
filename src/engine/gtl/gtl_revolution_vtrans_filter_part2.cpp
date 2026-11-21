// Translation unit 0x803FA17C-0x803FA2E4 (1 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

void* gtl_Alloc(unsigned int);
void* Memory_Copy(void*, const void*, unsigned int);

struct gtl_CRevolutionVTransFilter {
    char filterBaseStorage[0x1C];
    unsigned int frameHeaderSize; // Bytes copied from this embedded frame header, not a source pointer.
    unsigned int yPlaneOffset;
    unsigned short yPlaneWidth;
    unsigned short yPlaneHeight;
    unsigned int uPlaneOffset;
    unsigned short uPlaneWidth;
    unsigned short uPlaneHeight;
    unsigned int vPlaneOffset;
    unsigned short vPlaneWidth;
    unsigned short vPlaneHeight;
    char planeReserved[4];
    unsigned char* yPlane;
    unsigned char* uPlane;
    unsigned char* vPlane;
    void* decoderWorkBuffer;
    void* frameBuffer;
};
// Embedded 0x20-byte frame header starts at self+0x1C: size, then each plane's
// byte offset and 16-bit width/height, followed by four reserved bytes.
// Plane data uses one byte per element; the constructor halves U/V dimensions
// relative to Y (4:2:0). Offsets are from the allocated frame's start, not from self.

enum { kDecoderWorkBytes = 0x1000, kPlaneAlignment = 32 };

// Allocate decoder scratch and a frame containing a header followed by aligned Y/U/V planes.
// outFrameSize is an ABI pointer slot used as an integer byte count, not a dereferenceable buffer.
// Both output slots must be writable. Initialize once: repeat calls overwrite owned
// allocations. The filter retains scratch/frame ownership and returns a borrowed frame.
// Returns 0 only for frame allocation failure (scratch remains owned); scratch failure
// is unchecked. outFrameSize is still written on failure; outFrameBuffer is untouched.
int gtl_CRevolutionVTransFilter_AllocateFrameBuffers(gtl_CRevolutionVTransFilter* self,
                                        unsigned char** outFrameBuffer,
                                        unsigned char** outFrameSize) {
    // Allocate decoder scratch and start the frame layout immediately after its header.
    self->decoderWorkBuffer = gtl_Alloc(kDecoderWorkBytes);
    unsigned int headerBytes = (unsigned int)self->frameHeaderSize;
    *outFrameSize = (unsigned char*)headerBytes;
    self->yPlaneOffset = headerBytes;

    // Reserve Y, U, V in order, padding each plane's byte count to a multiple of 32.
    // Header alignment is a precondition for aligned starts (the ctor uses 0x20).
    unsigned int planeBytes = self->yPlaneWidth * self->yPlaneHeight;
    int remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;
    self->uPlaneOffset = (unsigned int)*outFrameSize;

    planeBytes = self->uPlaneWidth * self->uPlaneHeight;
    remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;
    self->vPlaneOffset = (unsigned int)*outFrameSize;

    planeBytes = self->vPlaneWidth * self->vPlaneHeight;
    remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;

    // Total size/offsets are final. Allocate one frame and copy only the embedded header.
    self->frameBuffer = gtl_Alloc((unsigned int)*outFrameSize);
    if (self->frameBuffer == 0)
        return 0;

    Memory_Copy(self->frameBuffer, &self->frameHeaderSize, self->frameHeaderSize);
    // Cache interior pointers for decoding; these must never be freed independently.
    self->yPlane = (unsigned char*)self->frameBuffer + self->yPlaneOffset;
    self->uPlane = (unsigned char*)self->frameBuffer + self->uPlaneOffset;
    self->vPlane = (unsigned char*)self->frameBuffer + self->vPlaneOffset;
    *outFrameBuffer = (unsigned char*)self->frameBuffer;
    return 1;
}

} // extern "C"
