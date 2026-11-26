// Revolution V-transform filter, part 2: allocating the frame buffers and reporting their size.

#include "classes/GtlRevolutionVTransFilter.h"

extern "C" {

void* gtl_Alloc(unsigned int);
void* Memory_Copy(void*, const void*, unsigned int);

// The frame header at +0x1C holds the size and each plane's byte offset and 16-bit width and height,
// followed by four reserved bytes. Plane data uses one byte per element; the constructor halves U/V
// dimensions relative to Y (4:2:0). Offsets are from the allocated frame's start, not from the object.

enum {
    kDecoderWorkBytes = 0x1000,
    kPlaneAlignment = 32
};

} // extern "C"

// Allocate decoder scratch and a frame containing a header followed by aligned Y/U/V planes.
// outFrameSize is an ABI pointer slot used as an integer byte count, not a dereferenceable buffer.
// Both output slots must be writable. Initialize once: repeat calls overwrite owned
// allocations. The filter retains scratch/frame ownership and returns a borrowed frame.
// Returns 0 only for frame allocation failure (scratch remains owned); scratch failure
// is unchecked. outFrameSize is still written on failure; outFrameBuffer is untouched.
int gtl::CRevolutionVTransFilter::AllocateFrameBuffers(unsigned char** outFrameBuffer, unsigned char** outFrameSize) {
    // Allocate decoder scratch and start the frame layout immediately after its header.
    decoderWorkBuffer = gtl_Alloc(kDecoderWorkBytes);
    unsigned int headerBytes = (unsigned int)frameHeaderSize;
    *outFrameSize = (unsigned char*)headerBytes;
    yPlaneOffset = headerBytes;

    // Reserve Y, U, V in order, padding each plane's byte count to a multiple of 32.
    // Header alignment is a precondition for aligned starts (the constructor uses 0x20).
    unsigned int planeBytes = yPlaneWidth * yPlaneHeight;
    int remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;
    uPlaneOffset = (unsigned int)*outFrameSize;

    planeBytes = uPlaneWidth * uPlaneHeight;
    remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;
    vPlaneOffset = (unsigned int)*outFrameSize;

    planeBytes = vPlaneWidth * vPlaneHeight;
    remainder = (int)planeBytes % kPlaneAlignment;
    if (remainder != 0)
        planeBytes += kPlaneAlignment - remainder;
    *outFrameSize += planeBytes;

    // Total size and offsets are final. Allocate one frame and copy only the embedded header.
    frameBuffer = gtl_Alloc((unsigned int)*outFrameSize);
    if (frameBuffer == 0)
        return 0;

    Memory_Copy(frameBuffer, &frameHeaderSize, frameHeaderSize);
    // Cache interior pointers for decoding; these must never be freed independently.
    yPlane = (unsigned char*)frameBuffer + yPlaneOffset;
    uPlane = (unsigned char*)frameBuffer + uPlaneOffset;
    vPlane = (unsigned char*)frameBuffer + vPlaneOffset;
    *outFrameBuffer = (unsigned char*)frameBuffer;
    return 1;
}
