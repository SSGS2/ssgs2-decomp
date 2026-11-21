// Layout view for TourBossPreview_InitState/51F0/5170/5228/5378 in tour_board_preview_controller.cpp.
// Original class name unknown. Separate from the smaller CCameraController.
#ifndef CLASSES_CAMERASTATE802F5148_H
#define CLASSES_CAMERASTATE802F5148_H
#include "game/types.h"
class TourBoardPreviewState {
public:
    void* previewNode; // 0x0: 0x802F5228 reads preview-node bytes 0xd/0xe to select the model preview.
    unsigned short unk_4; // 0x4: 0x802F5148 clears halfword.
    char unk_6[2];
    float position[2]; // 0x8: 0x802F5378 passes to Vec2 operations.
    float displacement[2]; // 0x10: 0x802F5378 length and subtraction.
    void* flags; // 0x18: 0x802F51F0 dereferences a halfword of flags.
    char unk_1C[0x50];
    void* modelPreview; // 0x6c: 0x802F5228 allocates CameraRig; 0x802F5170 destroys it.
    float heading; // 0x70: 0x802F5378 wraps angle and calls ModelPreview_SetHeading.
};
#define CAMERASTATE802F5148_OFF(f,o) typedef char CameraState802F5148_off_##f[((int)&((TourBoardPreviewState*)0)->f == (o)) ? 1 : -1]
CAMERASTATE802F5148_OFF(previewNode,0x0);
CAMERASTATE802F5148_OFF(unk_4,0x4);
CAMERASTATE802F5148_OFF(unk_6,0x6);
CAMERASTATE802F5148_OFF(position,0x8);
CAMERASTATE802F5148_OFF(displacement,0x10);
CAMERASTATE802F5148_OFF(flags,0x18);
CAMERASTATE802F5148_OFF(unk_1C,0x1c);
CAMERASTATE802F5148_OFF(modelPreview,0x6c);
CAMERASTATE802F5148_OFF(heading,0x70);
#undef CAMERASTATE802F5148_OFF
#endif
