// TiXmlNodeLayout: layout view owned by tinyxml_print_and_clone.cpp.
// Evidence: TiXmlDocument_Clone; TiXmlElement_CloneElement; clone routines. No virtual declarations or runtime objects.
#ifndef CLASSES_TIXMLNODELAYOUT_H
#define CLASSES_TIXMLNODELAYOUT_H

class TiXmlNodeLayout {
public:
    void* vtable; // 0x0; TiXmlDocument_Clone
    void* parent; // 0x4; TiXmlDocument_Clone
    int nodeType; // 0x8; TiXmlDocument_Clone
    void* firstChild; // 0xC; TiXmlDocument_Clone
    void* lastChild; // 0x10; TiXmlDocument_Clone
    const char* value; // 0x14; TiXmlDocument_Clone
    int valueCapacity; // 0x18; TiXmlDocument_Clone
    void* previousSibling; // 0x1C; TiXmlDocument_Clone
    void* nextSibling; // 0x20; TiXmlDocument_Clone
    int userData; // 0x24; TiXmlDocument_Clone
    unsigned char documentError; // 0x28; TiXmlDocument_Clone
    char pad_29[0x3];
    int documentErrorId; // 0x2C; TiXmlDocument_Clone
    char pad_30[0x14];
    void* firstAttribute; // 0x44; TiXmlDocument_Clone
};

#define OFF(f, o) typedef char TiXmlNodeLayout_off_##f[((int)&((TiXmlNodeLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(parent, 0x4);
OFF(nodeType, 0x8);
OFF(firstChild, 0xC);
OFF(lastChild, 0x10);
OFF(value, 0x14);
OFF(valueCapacity, 0x18);
OFF(previousSibling, 0x1C);
OFF(nextSibling, 0x20);
OFF(userData, 0x24);
OFF(documentError, 0x28);
OFF(documentErrorId, 0x2C);
OFF(firstAttribute, 0x44);
#undef OFF

#endif
