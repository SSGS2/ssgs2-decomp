// RingCursor803F: layout-only view owned by gtl_pointer_ring_and_sample_deque.cpp (RingCursor_Increment, RingCursor_SubtractOffset).
// No virtual methods, constructors, or generated RTTI.
#ifndef CLASSES_RINGCURSOR803F_H
#define CLASSES_RINGCURSOR803F_H

class RingCursor803F {
public:
    unsigned int currentAddress; // 0x0: RingCursor_Increment advances one four-byte element
    unsigned int beginAddress; // 0x4: RingCursor_Increment wraps here at limitAddress
    unsigned int splitBoundaryAddress; // 0x8: RingCursor_SubtractOffset copies unchanged
    unsigned int limitAddress; // 0xc: RingCursor_Increment compares with advanced address
};

#define OFF(f, o) typedef char RingCursor803F_off_##f[((int)&((RingCursor803F*)0)->f == (o)) ? 1 : -1]
OFF(currentAddress, 0x0);
OFF(beginAddress, 0x4);
OFF(splitBoundaryAddress, 0x8);
OFF(limitAddress, 0xc);
#undef OFF

#endif
