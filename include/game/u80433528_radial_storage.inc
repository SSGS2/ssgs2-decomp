// Resize radial CPU vertices before geometry generation; requires the local vec.
// Private statement fragment, included at the original scope to preserve the O0 stack layout.

// Resize temporary vertex storage to 64*64 cells * 6 vertices.
// Growth constructs a new tail, prepends the old range, then swaps
// ownership; the local guard releases displaced storage.
if (vec.size < kRadialGridVertices) {
    unsigned int n = kRadialGridVertices - vec.size;
    unsigned char allocatorTag = 0;
    unsigned char valueTag = allocatorTag;
    unsigned int* capacity = &vec.cap.value;
    unsigned int* checkedCapacity;
    // 0x80433528: finish in-place growth before the shared rendering setup.
    do {
        if (n > *capacity || (checkedCapacity = &vec.cap.value, vec.size > *checkedCapacity - n)) {
            unsigned int* growthCapacity = &vec.cap.value;
            unsigned int extra = vec.size + n - *growthCapacity;
            unsigned char growthTag = 0;
            unsigned char growthValue = growthTag;
            void* unused = CalculateGrowthCapacity__14GtlRecordDequeFUi(&vec, extra);
            (void)unused;
        } else {
            RadialVertex* p = vec.begin + vec.size;
            unsigned int left = n;
            unsigned char fillTag = 0;
            unsigned char fillValue = fillTag;
            while (left != 0) {
                unsigned int* allocator = &vec.cap.value;
                RadialVertex t = {0, 0, 0, 0, 0, 0};
                CopyConstructElement__14GtlRecordDequeFPvPvPv(allocator, p, &t);
                left--;
                p++;
                vec.size++;
            }
            break;
        }
        {
            VertexReplacementBuffer nv;
            unsigned int* allocator = &vec.cap.value;
            Initialize__14GtlRecordDequeFPv(&nv, allocator);
            unsigned int* growthCapacity = &vec.cap.value;
            AllocateStorage__14GtlRecordDequeFPvUi(&nv, vec.size + n, CalculateGrowthCapacity__14GtlRecordDequeFUi(&vec, vec.size + n - *growthCapacity));
            nv.pos = vec.size;
            RadialVertex* p = nv.begin + nv.pos + nv.size;
            unsigned int left = n;
            unsigned char fillTag = 0;
            unsigned char fillValue = fillTag;
            while (left != 0) {
                void* allocator = nv.alloc;
                RadialVertex t = {0, 0, 0, 0, 0, 0};
                CopyConstructElement__14GtlRecordDequeFPvPvPv(allocator, p, &t);
                left--;
                p++;
                nv.size++;
            }
            RadialVertex* first = vec.begin;
            RadialVertex* last = vec.begin + vec.size;
            PrependRange__14GtlRecordDequeFPcPc(&nv, first, last);
            unsigned char swapTag = 0;
            unsigned char swapValue = swapTag;
            unsigned int* newCapacity = &nv.cap;
            Capacity* oldCapacity = &vec.cap;
            vec.size = 0;
            Swap_Int32(oldCapacity, newCapacity);
            Pointer_Swap(&vec, &nv);
            Swap_Int32(&vec.size, &nv.size);
            dtor_804359C4(&nv, -1);
        }
    } while (0);
} else if (vec.size > kRadialGridVertices) {
    unsigned int d = vec.size - kRadialGridVertices;
    unsigned char eraseTag = 0;
    unsigned char eraseValue = eraseTag;
    vec.size = vec.size - d;
}
