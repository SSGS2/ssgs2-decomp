// StateObj layout used by the CGolfTask loading and pause members; original class name unknown.
#ifndef CLASSES_STATEOBJ80170224_H
#define CLASSES_STATEOBJ80170224_H
#include "game/types.h"
struct StateObj {
    char unk0[0x3A5];
    unsigned char unk3A5;
    char unk3A6[0xe];
    float delayTimer; // 0x3b4: 0x80170D08 subtracts delta; resets to -10 on expiry.
    void* pauseScoreMenu;
    union {
        char unk3BC[4];    // Retain existing member name.
        int messageActive; // 0x3bc: 0x80170D08 starts/polls/completes message processing.
    };
    void* loadingDraw;
    int loadingDrawFinished;
    void* preloadFiles[6];
    void* variantPreloadFile;
    void* inGameTexturePack1;
    void* inGameTexturePack2;
    void* tutorialTexturePack;
};
#define STATEOBJ80170224_OFF(f, o) typedef char StateObj80170224_off_##f[((int)&((StateObj*)0)->f == (o)) ? 1 : -1]
STATEOBJ80170224_OFF(unk0, 0x0);
STATEOBJ80170224_OFF(unk3A5, 0x3a5);
STATEOBJ80170224_OFF(unk3A6, 0x3a6);
STATEOBJ80170224_OFF(delayTimer, 0x3b4);
STATEOBJ80170224_OFF(pauseScoreMenu, 0x3b8);
STATEOBJ80170224_OFF(unk3BC, 0x3bc);
STATEOBJ80170224_OFF(messageActive, 0x3bc);
STATEOBJ80170224_OFF(loadingDraw, 0x3c0);
STATEOBJ80170224_OFF(loadingDrawFinished, 0x3c4);
STATEOBJ80170224_OFF(preloadFiles, 0x3c8);
STATEOBJ80170224_OFF(variantPreloadFile, 0x3e0);
STATEOBJ80170224_OFF(inGameTexturePack1, 0x3e4);
STATEOBJ80170224_OFF(inGameTexturePack2, 0x3e8);
STATEOBJ80170224_OFF(tutorialTexturePack, 0x3ec);
#undef STATEOBJ80170224_OFF
#endif
