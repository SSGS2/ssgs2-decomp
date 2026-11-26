// gtl::IHeapObject: reference-counted heap object holding an allocator and two memory notices.
#ifndef CLASSES_GTLHEAPOBJECT_H
#define CLASSES_GTLHEAPOBJECT_H

namespace gtl {
class IHeapObject {
public:
    static int Create(void* descriptor, void* out);
    void* Destroy(int deleting);

    int references;      // 0x0
    void* vtbl;          // 0x4
    void* allocator;     // 0x8
    void* mainNotice;    // 0xC
    void* localNotice;   // 0x10
    void* mainMemory;    // 0x14
    void* mainArgument;  // 0x18
    void* localMemory;   // 0x1C
    void* localArgument; // 0x20
    int unk_24;          // 0x24
    int flags;           // 0x28
};
} // namespace gtl

#define OFF(f, o) typedef char IHeapObject_off_##f[((int)&((gtl::IHeapObject*)0)->f == (o)) ? 1 : -1]
OFF(references, 0x0);
OFF(vtbl, 0x4);
OFF(allocator, 0x8);
OFF(mainNotice, 0xC);
OFF(localNotice, 0x10);
OFF(mainMemory, 0x14);
OFF(mainArgument, 0x18);
OFF(localMemory, 0x1C);
OFF(localArgument, 0x20);
OFF(unk_24, 0x24);
OFF(flags, 0x28);
#undef OFF

#endif
