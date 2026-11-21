// TiXmlDeclaration: methods owned by tinyxml_print_and_clone.cpp; no inheritance or emitted code.
// vfunc_1 (0x802A5448) / vfunc_2 (0x802A5578) print three XML declaration strings.
// Each string is the pointer/length pair copied by vfunc_5 (0x802A5784).
#ifndef CLASSES_TIXMLDECLARATIONLAYOUT_H
#define CLASSES_TIXMLDECLARATIONLAYOUT_H
struct TiXmlDeclarationString {
    char* data;
    int length;
};
struct TiXmlDeclarationLayout {
    char node[0x28];
    TiXmlDeclarationString version; // 0x28: first declaration attribute
    TiXmlDeclarationString encoding; // 0x30: second declaration attribute
    TiXmlDeclarationString standalone; // 0x38: third declaration attribute
};
#define OFF(f, o) typedef char TiXmlDeclarationLayout_off_##f[((int)&((TiXmlDeclarationLayout*)0)->f == (o)) ? 1 : -1]
OFF(node, 0x0);
OFF(version, 0x28);
OFF(encoding, 0x30);
OFF(standalone, 0x38);
#undef OFF
#define OFF(f, o) typedef char TiXmlDeclarationString_off_##f[((int)&((TiXmlDeclarationString*)0)->f == (o)) ? 1 : -1]
OFF(data, 0x0);
OFF(length, 0x4);
#undef OFF
#define AsTiXmlDeclaration(p) ((TiXmlDeclarationLayout*)(p))
#endif
