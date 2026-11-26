// COption: field view; constructor helper N14 and destructors live in project_base_and_grid_surface.cpp.
// The float initialized by N14 has no established meaning; keep its offset name.
#ifndef CLASSES_COPTION_H
#define CLASSES_COPTION_H

class COption {
public:
    void* vtbl; // 0x0: N14 installs __vt__7COption
    char pad_004[0xC];
    float hudBaselineY; // 0x10: N14 stores kDefaultHudBaselineY

    ~COption();

    // Clears the option singleton and destroys its base object; frees the instance when shouldDelete > 0.
    void* Destroy(int shouldDelete);
};
#define OFF(f, o) typedef char COption_off_##f[((int)&((COption*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(hudBaselineY, 0x10);
#undef OFF
#define AsCOption(p) ((COption*)(p))

#endif
