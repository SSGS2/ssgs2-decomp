// TourModelPreviewView: layout view from ModelPreview_Ctor (0x802F5898); provisional type name.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_MODELPREVIEW802F5898_H
#define CLASSES_MODELPREVIEW802F5898_H
#include "game/wvector.h"

struct TourModelPreviewView {
    void* model; // 0x0: ModelPreview_LoadModel loads resource; ModelPreview_ReleaseModel releases
    float matrix[12]; // 0x4: ModelPreview_LoadModel identity; ModelPreview_UpdateAnimation Model_SetMatrixRecursive
    void* animation; // 0x34: ModelPreview_SetAnimationByName stores WPuppet_FindAnimationRangeByName result
    void* camera; // 0x38: ModelPreview_UpdateCamera calls camera methods
    char pad_03c[0x4];
    float heading; // 0x40: ModelPreview_SetHeading uses as axis 1 rotation angle
    char pad_044[0x4];
    float animationTime; // 0x48: ModelPreview_UpdateAnimation adds delta and submits to WPuppet_SetAnimationTimeFromRanges
    float animationStart; // 0x4c: ModelPreview_SetAnimationByName loads first animation word
    float animationEnd; // 0x50: ModelPreview_UpdateAnimation compares elapsed animation time
    WVector cameraOffset; // 0x54: ModelPreview_UpdateCamera copies translation; Y/Z form atan ratio
    float fieldOfView; // 0x60: ModelPreview_UpdateCamera Camera_SetFov
    float savedFieldOfView; // 0x64: ModelPreview_Ctor saves; ModelPreview_ReleaseModel restores
    float savedViewMatrix[12]; // 0x68: ModelPreview_Ctor saves; ModelPreview_ReleaseModel restores
};

#define OFF(f, o) typedef char ModelPreview802F5898_off_##f[((int)&((TourModelPreviewView*)0)->f == (o)) ? 1 : -1]
OFF(model, 0x0);
OFF(matrix, 0x4);
OFF(animation, 0x34);
OFF(camera, 0x38);
OFF(heading, 0x40);
OFF(animationTime, 0x48);
OFF(animationStart, 0x4c);
OFF(animationEnd, 0x50);
OFF(cameraOffset, 0x54);
OFF(fieldOfView, 0x60);
OFF(savedFieldOfView, 0x64);
OFF(savedViewMatrix, 0x68);
#undef OFF
#define AsModelPreview802F5898(p) ((TourModelPreviewView*)(p))

#endif

