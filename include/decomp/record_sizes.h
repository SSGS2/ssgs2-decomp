#ifndef DECOMP_RECORD_SIZES_H
#define DECOMP_RECORD_SIZES_H
enum OriginalRecordBytes {
    kMatrix3x4Bytes = 48, /* Matrix_SetAxisRotation, Matrix_BuildCameraTransform, Matrix_SetNegativeAxisAngleRotation:
                                        all end in Matrix_SetBasisElements (12 float stores); 3x4 float matrix */
    kViewportBytes = 24,  /* Engine_GetViewport / Viewport_Copy: u32 x, y, width, height; float nearZ, farZ */
    kProjectedPointBytes = 0x38,   /* WView_ProjectWorldPoint/ViewPoint: screen x, y, z ...; clip flags at +0x34 */
    kVector3Bytes = 12,            /* Vector_CopyComponents, WVector difference: three floats */
    kUIGroupPositionBytes = 16,    /* UIGroup_GetPosition: a 16-byte vector */
    kFxAttachmentInitBytes = 40,   /* FxAttachment_Init */
    kReadItemDataByKindBytes = 24, /* gtl::IInputDevice::ReadItemDataByKind */
    kRingBuffer32HeaderBytes = 20, /* RingBuffer32_InitializeHeader */
    kPointerPositionBytes = 8,     /* InputManager_GetPointerPosition, Layout_GetElementPosition: two floats */
    kBoosterGateTagBytes = 32      /* char tag[32] at course mesh +0x8C (file) / +0xBA (runtime) */
};
#endif
