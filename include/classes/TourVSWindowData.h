// TourVSWindowData: existing non-virtual layout moved from tour_vs_window_part1.cpp.
// CTourVSWindow::CTourVSWindow and CTourVSWindow::Destroy (0x802F63B4) initialize it;
// animation/render functions in the same unit use these names. No field renamed.
#ifndef CLASSES_TOURVSWINDOWDATA_H
#define CLASSES_TOURVSWINDOWDATA_H

struct TourVSWindowData {
    void* vt; // 0x00
    char unk04[0x14];
    char fadeControl[4];   // 0x18
    unsigned char opacity; // 0x1C
    char unk1D[3];
    int state;             // 0x20
    int status;            // 0x24
    unsigned int layoutId; // 0x28
    char unk2C[8];
    void* entries;            // 0x34
    float panelSplitX;        // 0x38
    float middle;             // 0x3C
    float edge;               // 0x40
    int entryA;               // 0x44
    int entryB;               // 0x48
    int playerOneNameEnabled; // 0x4C
    int playerTwoNameEnabled; // 0x50
    float time;               // 0x54
    float speed;              // 0x58
    int scrollGroupEnabled;   // 0x5C
    short mode;               // 0x60
    unsigned char alphaA;     // 0x62
    unsigned char alphaB;     // 0x63
};

#define OFF(f, o) typedef char TourVSWindowData_off_##f[((int)&((TourVSWindowData*)0)->f == (o)) ? 1 : -1]
OFF(vt, 0x0);
OFF(fadeControl, 0x18);
OFF(opacity, 0x1C);
OFF(state, 0x20);
OFF(status, 0x24);
OFF(layoutId, 0x28);
OFF(entries, 0x34);
OFF(panelSplitX, 0x38);
OFF(middle, 0x3C);
OFF(edge, 0x40);
OFF(entryA, 0x44);
OFF(entryB, 0x48);
OFF(playerOneNameEnabled, 0x4C);
OFF(playerTwoNameEnabled, 0x50);
OFF(time, 0x54);
OFF(speed, 0x58);
OFF(scrollGroupEnabled, 0x5C);
OFF(mode, 0x60);
OFF(alphaA, 0x62);
OFF(alphaB, 0x63);
#undef OFF

#endif
