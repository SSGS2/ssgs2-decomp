// ByteVectorState: layout view; effect_fade_and_camera_script.cpp: ByteVector_ResizeZeroFilled resizes a byte buffer; ByteVector_AppendByte appends and grows it
// No inheritance, virtual declarations, constructors, or emitted runtime objects.
#ifndef CLASSES_BYTEVECTORSTATE_H
#define CLASSES_BYTEVECTORSTATE_H

struct ByteVectorState {
    void* data;            // 0x0
    unsigned int size;     // 0x4
    unsigned int capacity; // 0x8
};

#define OFF(f, o) typedef char ByteVectorState_off_##f[((int)&((ByteVectorState*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(size, 0x4);
OFF(capacity, 0x8);
#undef OFF
#define AsByteVectorState(p) ((ByteVectorState*)(p))

#endif
