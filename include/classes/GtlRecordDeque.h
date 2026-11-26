// GtlRecordDeque: a deque of 24-byte records. Its storage comes from the engine's graphics allocator callbacks.
// Methods are shared by the render-state and texture-projection source files.
#ifndef CLASSES_GTLRECORDDEQUE_H
#define CLASSES_GTLRECORDDEQUE_H

class GtlRecordDeque {
public:
    // Setup and teardown
    void InitEmpty();
    void* Dtor(short freeFlag);
    void* DestroyStorage(short deleting);
    void* DestroyStorageVolatile(short volatile deleting);
    void Initialize(void* allocator);

    // Storage growth and record movement
    void AllocateStorage(void* unused, unsigned int count);
    unsigned int CalculateGrowthCapacity(unsigned int additionalCount);
    void PrependRange(char* volatile first, char* last);
    // Copies one 24-byte record; the allocator argument is not used by this record type.
    static void CopyConstructElement(void* allocator, void* destination, void* source);

    void* data;            // 0x0: allocation of capacity * 0x18 bytes
    unsigned int size;     // 0x4: number of live records
    unsigned int capacity; // 0x8: allocated record count
    void* allocator;       // 0xc: passed to CopyConstructElement
    int firstIndex;        // 0x10: index of the first live record
};

#define OFF(f, o) typedef char GtlRecordDeque_off_##f[((int)&((GtlRecordDeque*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(size, 0x4);
OFF(capacity, 0x8);
OFF(allocator, 0xc);
OFF(firstIndex, 0x10);
#undef OFF

#endif
