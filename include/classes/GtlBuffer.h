// GtlBuffer: partial layout for methods in the address-named translation unit.
// Field comments identify the function providing layout evidence.
#ifndef CLASSES_GTLBUFFER_H
#define CLASSES_GTLBUFFER_H

class GtlBuffer {
public:
    unsigned int referenceCount; // 0x0: gtl_IG3DXBuffer_Ctor initializes to one
    void* vtable; // 0x4: gtl_IG3DXBuffer_dtor
    unsigned int buffer; // 0x8: gtl_IG3DXBuffer_dtor
    void* sourceData; // 0xc: gtl_IG3DXBuffer_Ctor stores its data argument
};

#define OFF(f, o) typedef char GtlBuffer_off_##f[((int)&((GtlBuffer*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtable, 0x4);
OFF(buffer, 0x8);
OFF(sourceData, 0xC);
#undef OFF

#endif
