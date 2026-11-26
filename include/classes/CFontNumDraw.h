#ifndef CLASSES_CFONTNUMDRAW_H
#define CLASSES_CFONTNUMDRAW_H

#include "game/types.h"

struct NumericEntryDefinition;
struct NumericEntryConstruction;
struct SelectedFontPair;
struct PtrVector;
class CFontNumDraw {
public:
    void* vtbl;          // 0x0: original vtable pointer
    char pad_004[0x4];   // 0x4
    void* unk_8;         // 0x8
    char pad_00C[0x4];   // 0xc
    unsigned int unk_10; // 0x10
    char pad_014[0x4];   // 0x14
    void* unk_18;        // 0x18
    void* unk_1C;        // 0x1c

    void vfunc_5();

    static int FONT_NUM_DRAW_INFO_vfunc_0();

    // Allocate a numeric text table and append it to the owning pointer vector.
    static CFontNumDraw* CreateInVector(PtrVector* tables);

    int CreateNumericEntry(NumericEntryDefinition* definition, void* data, NumericEntryConstruction** out);

    void* Destroy(int shouldDelete);
    int RegisterEntries(const void* definitions, int count, void* groupProvider, const void* resourceName);

    void vfunc_4();
    void vfunc_2();
    void vfunc_1();

    void SelectFont(const char* name, SelectedFontPair* result, void* atlasOptions);
};

#define CFONTNUMDRAW_OFF(f, o) typedef char CFontNumDraw_off_##f[((int)&((CFontNumDraw*)0)->f == (o)) ? 1 : -1]
CFONTNUMDRAW_OFF(unk_8, 0x8);
CFONTNUMDRAW_OFF(unk_10, 0x10);
CFONTNUMDRAW_OFF(unk_18, 0x18);
CFONTNUMDRAW_OFF(unk_1C, 0x1c);
#undef CFONTNUMDRAW_OFF

#endif // CLASSES_CFONTNUMDRAW_H
