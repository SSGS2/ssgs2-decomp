// Basic types the game uses everywhere. Shared by the decompiled sources so each file does not keep its own copy.
#ifndef GAME_TYPES_H
#define GAME_TYPES_H

// The game passes and returns 3-float vectors BY VALUE, so every file must see the same 12-byte type.
typedef struct Vec3f {
    float x, y, z;
} Vec3f;

// Copy idiom: a 12-byte vector copy only matches as `*(Arr3*)dst = *(Arr3*)src;` (plain Vec3f assignment compiles
// to float loads and stores instead).
struct Arr3 {
    float v[3];
};

#endif
