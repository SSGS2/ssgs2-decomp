// GtlHeapObject: layout view owned by gtl_heap_allocators.cpp.
// Evidence: gtl_IHeapObject_dtor; gtl_IHeapObject_Create. No virtual declarations or runtime objects.
#ifndef CLASSES_GTLHEAPOBJECT_H
#define CLASSES_GTLHEAPOBJECT_H

class GtlHeapObject {
public:
    int references; // 0x0; gtl_IHeapObject_dtor
    void* vtable; // 0x4; gtl_IHeapObject_dtor
    void* allocator; // 0x8; gtl_IHeapObject_dtor
    void* mainNotice; // 0xC; gtl_IHeapObject_dtor
    void* localNotice; // 0x10; gtl_IHeapObject_dtor
    void* mainMemory; // 0x14; gtl_IHeapObject_dtor
    void* mainArgument; // 0x18; gtl_IHeapObject_dtor
    void* localMemory; // 0x1C; gtl_IHeapObject_dtor
    void* localArgument; // 0x20; gtl_IHeapObject_dtor
    int unk_24; // 0x24; gtl_IHeapObject_dtor
    int flags; // 0x28; gtl_IHeapObject_dtor
};

#define OFF(f, o) typedef char GtlHeapObject_off_##f[((int)&((GtlHeapObject*)0)->f == (o)) ? 1 : -1]
OFF(references, 0x0);
OFF(vtable, 0x4);
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
