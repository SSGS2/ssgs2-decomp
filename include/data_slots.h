// Typed 8-byte data slots for the data companions: the original objects hold some constants and strings in
// 8-byte slots whose value sits in the first four (or eight) bytes. Each union keeps the 8-byte size and
// alignment and puts the real type first, so the bytes, the section and the symbol are unchanged.
#ifndef DATA_SLOTS_H
#define DATA_SLOTS_H
union Slot8Float {
    float v;
    unsigned long long pad;
};
union Slot8Int {
    int v;
    unsigned long long pad;
};
union Slot8UInt {
    unsigned int v;
    unsigned long long pad;
};
union Slot8Byte {
    unsigned char v;
    unsigned long long pad;
};
union Slot8Text {
    char s[8];
    unsigned long long pad;
};
union Slot4Text {
    char s[4];
    unsigned int pad;
};
union Slot8Float2 {
    float v[2];
    unsigned long long pad;
};
union Slot8Int2 {
    int v[2];
    unsigned long long pad;
};
union Slot8UInt2 {
    unsigned int v[2];
    unsigned long long pad;
};
union Slot8Short4 {
    unsigned short v[4];
    unsigned long long pad;
};
#endif
