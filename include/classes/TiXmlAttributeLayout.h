// TiXmlAttributeLayout: layout view owned by tinyxml_print_and_clone.cpp.
// Evidence: TiXmlAttribute_PrintToStream; TiXmlElement_CloneElement. No virtual declarations or runtime objects.
#ifndef CLASSES_TIXMLATTRIBUTELAYOUT_H
#define CLASSES_TIXMLATTRIBUTELAYOUT_H

class TiXmlAttributeLayout {
public:
    char pad_0[0x8];
    const char* name;  // 0x8; TiXmlAttribute_PrintToStream
    int nameLength;    // 0xC; TiXmlAttribute_PrintToStream
    const char* value; // 0x10; TiXmlAttribute_PrintToStream
    int valueLength;   // 0x14; TiXmlAttribute_PrintToStream
    char pad_18[0x4];
    void* next; // 0x1C; TiXmlAttribute_PrintToStream
};

#define OFF(f, o) typedef char TiXmlAttributeLayout_off_##f[((int)&((TiXmlAttributeLayout*)0)->f == (o)) ? 1 : -1]
OFF(name, 0x8);
OFF(nameLength, 0xC);
OFF(value, 0x10);
OFF(valueLength, 0x14);
OFF(next, 0x1C);
#undef OFF

#endif
