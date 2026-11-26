// Table records read by CExhibition_model_pos getters in unplayable_ball_drop_sequence.cpp.
// Names follow the getters at 0x8029BE00..0x8029BF88; unknown getters stay unnamed.
#ifndef CLASSES_EXHIBITIONMODELPOSITIONRECORD_H
#define CLASSES_EXHIBITIONMODELPOSITIONRECORD_H
struct ExhibitionModelPositionRecord {
    char pad_0[4];
    float cameraDistance;
    float cameraDistanceLimit1;
    float cameraDistanceLimit2;
    float viewportCenterX;
    float viewportBottom;
    float viewportBottomLimit1;
    float viewportBottomLimit2;
    float rotationX;
};
#define OFF(f, o) \
    typedef char ExhibitionModelPositionRecord_off_##f[((int)&((ExhibitionModelPositionRecord*)0)->f == (o)) ? 1 : -1]
OFF(cameraDistance, 0x4);
OFF(cameraDistanceLimit1, 0x8);
OFF(cameraDistanceLimit2, 0xC);
OFF(viewportCenterX, 0x10);
OFF(viewportBottom, 0x14);
OFF(viewportBottomLimit1, 0x18);
OFF(viewportBottomLimit2, 0x1C);
OFF(rotationX, 0x20);
#undef OFF
#endif
