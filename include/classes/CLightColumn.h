// CLightColumn: the light column that marks the hole during a round.
//
// It draws one of five score-selected marker frames and, when the camera is
// close, a scrolling strip behind it. The class is declared without virtual
// functions on purpose: its vtable still comes from the original game data, so
// `vtbl` is spelled out as a member.
#ifndef CLASSES_CLIGHTCOLUMN_H
#define CLASSES_CLIGHTCOLUMN_H

#include "game/types.h"

class CLightColumn {
public:
    // One textured quad. The draw code fills a quad in and then hands its
    // address to the view, which reads the fields below.
    struct Quad {
        float minU, minV, maxU, maxV;   // 0x00
        int argb;                       // 0x10
        int frame;                      // 0x14
        float left, bottom, right, top; // 0x18
        Vec3f position;                 // 0x28
        unsigned char enabled;          // 0x34
        char pad_35[3];
    };

    // Set up the owned quad pointers and the vtable.
    void Init();
    // Load the five marker frames and the scrolling strip.
    void LoadBitmaps();
    // Follow the hole and the camera: width, texture scroll and fade.
    void UpdateWidthScrollAndFade(float delta);
    void Draw();

    // Release the bitmaps and owned sprites. A delete flag above zero also frees
    // the object; either way the address is returned.
    void* Destroy(int flags);

    void* vtbl;                         // 0x00
    char pad_004[0x10];                 // 0x04
    int unk_14;                         // 0x14
    unsigned char initialized;          // 0x18
    char pad_019[3];                    // 0x19
    void* ownedObject1;                 // 0x1c
    void* ownedObject2;                 // 0x20
    char pad_024[0xc];                  // 0x24
    void* vertexPointers[4];            // 0x30
    int vertexListTerminator;           // 0x40
    Quad quads[4];                      // 0x44
    int fadeMode;                       // 0x124
    float columnHeight;                 // 0x128
    float columnWidthScale;             // 0x12c
    unsigned char scrollingColumnAlpha; // 0x130
    unsigned char frameColumnAlpha;     // 0x131
    char pad_132[0x2];                  // 0x132
    void* frameBitmaps[5];              // 0x134
    void* scrollingBitmap;              // 0x148
    float textureScrollV;               // 0x14c
};

#define CLIGHTCOLUMN_OFF(f, o) typedef char CLightColumn_off_##f[((int)&((CLightColumn*)0)->f == (o)) ? 1 : -1]
CLIGHTCOLUMN_OFF(unk_14, 0x14);
CLIGHTCOLUMN_OFF(initialized, 0x18);
CLIGHTCOLUMN_OFF(ownedObject1, 0x1c);
CLIGHTCOLUMN_OFF(ownedObject2, 0x20);
CLIGHTCOLUMN_OFF(vertexPointers, 0x30);
CLIGHTCOLUMN_OFF(vertexListTerminator, 0x40);
CLIGHTCOLUMN_OFF(quads, 0x44);
CLIGHTCOLUMN_OFF(fadeMode, 0x124);
CLIGHTCOLUMN_OFF(columnHeight, 0x128);
CLIGHTCOLUMN_OFF(columnWidthScale, 0x12c);
CLIGHTCOLUMN_OFF(scrollingColumnAlpha, 0x130);
CLIGHTCOLUMN_OFF(frameColumnAlpha, 0x131);
CLIGHTCOLUMN_OFF(frameBitmaps, 0x134);
CLIGHTCOLUMN_OFF(scrollingBitmap, 0x148);
CLIGHTCOLUMN_OFF(textureScrollV, 0x14c);
#undef CLIGHTCOLUMN_OFF
typedef char CLightColumn_size[(sizeof(CLightColumn) == 0x150) ? 1 : -1];
typedef char CLightColumn_quad_size[(sizeof(CLightColumn::Quad) == 0x38) ? 1 : -1];

#endif // CLASSES_CLIGHTCOLUMN_H
