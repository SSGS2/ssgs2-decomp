// Construct and resize projection bytes; requires local vb and bytesPerTexel.
// Private statement fragment, included at the original scope to preserve the O0 stack layout.

ProjectionByteBuffer* construction = &vb;
construction->begin = 0;
construction->size = 0;
construction->cap.value = 0;
ProjectionByteBuffer* storageOwner = &vb;
ProjectionByteBuffer* allocatorOwner = &vb;
ProjectionByteBuffer* initializedStorage = construction;
Capacity* firstCapacity = &construction->cap;
Capacity* secondCapacity = &construction->cap;
Capacity* thirdCapacity = &construction->cap;
ProjectionByteBuffer* initializedOwner = construction;
ProjectionByteBuffer* allocatorBase = construction;
ProjectionByteBuffer* dataOwner = construction;
ProjectionByteBuffer* sizeOwner = &vb;
ProjectionByteBuffer* capacityOwner = &vb;
// Resize byte storage to bytes-per-texel * 256*256, zero filling.
if ((bytesPerTexel << 16) > vb.size) {
    unsigned int d = (bytesPerTexel << 16) - vb.size;
    unsigned char allocatorTag = 0;
    unsigned char valueTag = allocatorTag;
    unsigned int* capacity = &vb.cap.value;
    unsigned int* checkedCapacity;
    do {
        if (d > *capacity || (checkedCapacity = &vb.cap.value, vb.size > *checkedCapacity - d)) {
            unsigned int* growthCapacity = &vb.cap.value;
            unsigned int extra = vb.size + d - *growthCapacity;
            unsigned char growthTag = 0;
            unsigned char growthValue = growthTag;
            unsigned int unused = ByteBuffer_CalculateGrowthCapacity(&vb, extra);
            (void)unused;
        } else {
            unsigned char* end = vb.begin + vb.size;
            memset(end, 0, d);
            vb.size += d;
            break;
        }
        {
            ByteReplacementBuffer nb;
            // nb constructs the replacement and owns it until
            // the pointer/size/capacity swaps commit below.
            ByteReplacementBuffer* object = &nb;
            void* allocator = &vb.cap.value;
            object->data = 0;
            object->constructedBytes = 0;
            object->capacityBytes = 0;
            object->alloc = allocator;
            object->liveBeginOffsetBytes = 0;
            unsigned int* growthCapacity = &vb.cap.value;
            unsigned int cnt = ByteBuffer_CalculateGrowthCapacity(&vb, vb.size + d - *growthCapacity);
            volatile unsigned int maximum = 0xFFFFFFFFu;
            unsigned char allocateTag = 0;
            unsigned char allocateValue = allocateTag;
            unsigned char allocateState = allocateTag;
            void* owner = nb.alloc;
            volatile unsigned int bound = maximum;
            unsigned int boundCopy = bound;
            unsigned int limit = boundCopy;
            if (cnt > limit)
                std_ThrowLengthError();
            void* allocatorOwner = nb.alloc;
            nb.data = (unsigned char*)gtl_Graphics_AllocateWithCallback(cnt, allocatorOwner);
            nb.capacityBytes = cnt;
            nb.liveBeginOffsetBytes = vb.size;
            unsigned char fillTag = 0;
            unsigned char fillValue = fillTag;
            memset(nb.constructedBytes + (nb.data + vb.size), 0, d);
            unsigned char* end = vb.begin + vb.size;
            unsigned char* first = vb.begin;
            unsigned char* sourceEnd = end;
            unsigned char* sourceFirst = first;
            unsigned int osz = sourceEnd - sourceFirst;
            nb.liveBeginOffsetBytes = nb.liveBeginOffsetBytes - osz;
            nb.constructedBytes = nb.constructedBytes + d;
            unsigned char copyTag = 0;
            unsigned char copyValue = copyTag;
            memcpy(nb.data + nb.liveBeginOffsetBytes, sourceFirst, osz);
            memset(sourceFirst, 0, osz);
            nb.constructedBytes = nb.constructedBytes + osz;
            unsigned char swapTag = 0;
            unsigned char swapValue = swapTag;
            unsigned int* newCapacity = &nb.capacityBytes;
            // Transfer the new bytes to vb, leaving old storage
            // in nb for its allocator-specific destructor.
            Capacity* oldCapacity = &vb.cap;
            vb.size = 0;
            Swap_Int32(oldCapacity, newCapacity);
            ProjectionByteBuffer* oldObject = &vb;
            ByteReplacementBuffer* newObject = &nb;
            unsigned char* tmp = oldObject->begin;
            oldObject->begin = newObject->data;
            newObject->data = tmp;
            Swap_Int32(&vb.size, &nb.constructedBytes);
            dtor_8042AE7C(&nb, -1);
        }
    } while (0);
} else if ((bytesPerTexel << 16) < vb.size) {
    unsigned int d = vb.size - (bytesPerTexel << 16);
    unsigned char eraseTag = 0;
    unsigned char eraseValue = eraseTag;
    vb.size = vb.size - d;
}
