// CPangyaMsgResourceObject: layout owned by async_exhibition_and_pet_format.cpp.
// Create (0x80268B6C), loader CPangyaMsgResourceObject_CreateMessageDraw and dtor (0x80268C48) establish this view.
// Plain layout only: no inheritance, virtual methods or emitted runtime objects.
#ifndef CLASSES_CPANGYAMSGRESOURCEOBJECT_H
#define CLASSES_CPANGYAMSGRESOURCEOBJECT_H

class CPangyaMsgResourceObject {
public:
    int referenceCount; // 0x0: Create initializes to one; ownership meaning not established here
    void* vtbl;         // 0x4: Create/dtor install __vt__24CPangyaMsgResourceObject
    void*
        resourceBytes; // 0x8: CPangyaMsgResourceObject_CreateMessageDraw passes this byte vector to Resource_ReadFileIntoByteVector
    unsigned int resourceByteCount;    // 0xC: Resource_ReadFileIntoByteVector resizes by bytes; dtor clears
    unsigned int resourceByteCapacity; // 0x10: Resource_ReadFileIntoByteVector grows the byte buffer
    void* secondaryBytes;              // 0x14: dtor frees this second vector's storage
    int secondaryByteCount;            // 0x18: dtor clears this second vector's count
    void* secondaryByteCapacity;       // 0x1C: Create zeroes this second vector's capacity word
    void*
        messageDraw; // 0x20: CPangyaMsgResourceObject_CreateMessageDraw receives WFixedFont_CreateMsgEcbResource output; dtor calls slot 0(1)

    int Create(void** out);
    int CreateMessageDraw(void* arg);

    ~CPangyaMsgResourceObject();
};

#define OFF(f, o) \
    typedef char CPangyaMsgResourceObject_off_##f[((int)&((CPangyaMsgResourceObject*)0)->f == (o)) ? 1 : -1]
OFF(referenceCount, 0x0);
OFF(vtbl, 0x4);
OFF(resourceBytes, 0x8);
OFF(resourceByteCount, 0xC);
OFF(resourceByteCapacity, 0x10);
OFF(secondaryBytes, 0x14);
OFF(secondaryByteCount, 0x18);
OFF(secondaryByteCapacity, 0x1C);
OFF(messageDraw, 0x20);
#undef OFF
typedef char CPangyaMsgResourceObject_size[(sizeof(CPangyaMsgResourceObject) == 0x24) ? 1 : -1];
#define AsCPangyaMsgResourceObject(p) ((CPangyaMsgResourceObject*)(p))

#endif
