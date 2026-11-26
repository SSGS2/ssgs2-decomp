// gtl::IG3DXBuffer: a reference-counted graphics buffer. Init records the data argument and allocates the
// buffer; Destroy frees the buffer. Methods are in gtl_render_state_and_primitive.cpp.
#ifndef CLASSES_GTLBUFFER_H
#define CLASSES_GTLBUFFER_H

namespace gtl {

class IG3DXBuffer {
public:
    // Allocate and construct a buffer for data and store it in *outBuffer. Always returns 1.
    static int Create(void* data, void** outBuffer);
    // Start the reference count at one, store the data argument, and allocate the buffer. Returns this.
    void* Init(void* data);
    // Free the owned buffer, then this object when freeFlag is positive. Returns this.
    void* Destroy(volatile short freeFlag);

    unsigned int referenceCount; // 0x0
    void* vtable;                // 0x4
    unsigned int buffer;         // 0x8
    void* sourceData;            // 0xC
};

#define GTL_IG3DXBUFFER_OFF(f, o) \
    typedef char gtl_IG3DXBuffer_off_##f[((int)&((gtl::IG3DXBuffer*)0)->f == (o)) ? 1 : -1]
GTL_IG3DXBUFFER_OFF(referenceCount, 0x0);
GTL_IG3DXBUFFER_OFF(vtable, 0x4);
GTL_IG3DXBUFFER_OFF(buffer, 0x8);
GTL_IG3DXBUFFER_OFF(sourceData, 0xC);
#undef GTL_IG3DXBUFFER_OFF

} // namespace gtl

#endif // CLASSES_GTLBUFFER_H
