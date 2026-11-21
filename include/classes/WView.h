// WView: methods live in wview_camera_projection.cpp.
// Non-virtual layout view; original function addresses are cited per field.
#ifndef CLASSES_WVIEW_H
#define CLASSES_WVIEW_H

class WView {
public:
    char pad_000[0x10];
    void* targetSlot; // 0x10: WView_ctor literal load/store
    float inputTransform[12]; // 0x14: WView_DrawBox: Matrix_TransformPoint
    float viewMatrix[12]; // 0x44: WView_ctor: Matrix_SetIdentityAffine
    float appliedViewMatrix[12]; // 0x74: Camera_Apply: copies affine rows to savedViewMatrix
    void* renderTarget; // 0xA4: WView_ctor literal load/store
    float frustumPlanes[48]; // 0xA8: Camera_BuildFrustumPlanes: four-float plane records
    float halfViewportWidth; // 0x168: Camera_BuildFrustumPlanes literal load/store
    float halfViewportHeight; // 0x16C: Camera_BuildFrustumPlanes literal load/store
    float screenCenterX; // 0x170: Camera_BuildFrustumPlanes literal load/store
    float screenCenterY; // 0x174: Camera_BuildFrustumPlanes literal load/store
    float nearClip; // 0x178: WView_ctor literal load/store
    float farClip; // 0x17C: WView_ctor literal load/store
    float nearDepthProjectionRatio; // 0x180: WView_ctor literal load/store
    float farDepthProjectionRatio; // 0x184: WView_ctor literal load/store
    float viewportWidth; // 0x188: WView_ctor literal load/store
    float viewportHeight; // 0x18C: WView_ctor literal load/store
    int unk_190; // 0x190: WView_ctor literal load/store
    unsigned char unk_194; // 0x194: WView_ctor literal load/store
    unsigned char customViewport; // 0x195: WView_ctor literal load/store
    char pad_196[0x2];
    float viewportLeft; // 0x198: WView_ctor literal load/store
    float viewportTop; // 0x19C: WView_ctor literal load/store
    float viewportExtentX; // 0x1A0: WView_ctor literal load/store
    float viewportExtentY; // 0x1A4: WView_ctor literal load/store
    float depthProjectionScale; // 0x1A8: WView_ctor literal load/store
    float depthProjectionOffset; // 0x1AC: WView_ctor literal load/store
    float fieldOfView; // 0x1B0: WView_ctor literal load/store
    char pad_1B4[0x1810];
    float pendingViewMatrix[12]; // 0x19C4: WView_SetViewMatrix: Matrix_CopyAffine destination
    unsigned int dirtyFlags; // 0x19F4: WView_ctor literal load/store
    float primitiveSubmissionGate; // 0x19F8: WView_ctor literal load/store
    float projectionLeft; // 0x19FC: WView_ctor literal load/store
    float projectionRight; // 0x1A00: WView_ctor literal load/store
    float projectionMinY; // 0x1A04: WView_ctor literal load/store
    float projectionMaxY; // 0x1A08: WView_ctor literal load/store
    unsigned char unk_1A0C; // 0x1A0C: WView_ctor literal load/store
    unsigned char suppressOptionalSubmitFlag; // 0x1A0D: WView_ctor literal load/store
    char pad_1A0E[0x2];
    float projectionOffsetX; // 0x1A10: WView_ctor literal load/store
    float projectionOffsetY; // 0x1A14: WView_ctor literal load/store
    int unk_1A18; // 0x1A18: WView_ctor literal load/store
    float unk_1A1C; // 0x1A1C: WView_ctor literal load/store
    float unk_1A20; // 0x1A20: WView_ctor literal load/store
    float unk_1A24; // 0x1A24: WView_ctor literal load/store
    int unk_1A28; // 0x1A28: WView_ctor literal load/store
    int unk_1A2C; // 0x1A2C: WView_ctor literal load/store
    unsigned int unk_1A30; // 0x1A30: WView_ctor literal load/store
    int matrixUpdateFlags; // 0x1A34: WView_ctor literal load/store
    float savedViewMatrix[16]; // 0x1A38: Camera_Apply: 4x4 view matrix
    float projectionMatrix[16]; // 0x1A78: Camera_Apply: MtxPerspectiveFovRH destination
};

#define WVIEW_OFF(f, o) typedef char WView_off_##f[((int)&((WView*)0)->f == (o)) ? 1 : -1]
WVIEW_OFF(targetSlot, 0x10);
WVIEW_OFF(inputTransform, 0x14);
WVIEW_OFF(viewMatrix, 0x44);
WVIEW_OFF(appliedViewMatrix, 0x74);
WVIEW_OFF(renderTarget, 0xA4);
WVIEW_OFF(frustumPlanes, 0xA8);
WVIEW_OFF(halfViewportWidth, 0x168);
WVIEW_OFF(halfViewportHeight, 0x16C);
WVIEW_OFF(screenCenterX, 0x170);
WVIEW_OFF(screenCenterY, 0x174);
WVIEW_OFF(nearClip, 0x178);
WVIEW_OFF(farClip, 0x17C);
WVIEW_OFF(nearDepthProjectionRatio, 0x180);
WVIEW_OFF(farDepthProjectionRatio, 0x184);
WVIEW_OFF(viewportWidth, 0x188);
WVIEW_OFF(viewportHeight, 0x18C);
WVIEW_OFF(unk_190, 0x190);
WVIEW_OFF(unk_194, 0x194);
WVIEW_OFF(customViewport, 0x195);
WVIEW_OFF(viewportLeft, 0x198);
WVIEW_OFF(viewportTop, 0x19C);
WVIEW_OFF(viewportExtentX, 0x1A0);
WVIEW_OFF(viewportExtentY, 0x1A4);
WVIEW_OFF(depthProjectionScale, 0x1A8);
WVIEW_OFF(depthProjectionOffset, 0x1AC);
WVIEW_OFF(fieldOfView, 0x1B0);
WVIEW_OFF(pendingViewMatrix, 0x19C4);
WVIEW_OFF(dirtyFlags, 0x19F4);
WVIEW_OFF(primitiveSubmissionGate, 0x19F8);
WVIEW_OFF(projectionLeft, 0x19FC);
WVIEW_OFF(projectionRight, 0x1A00);
WVIEW_OFF(projectionMinY, 0x1A04);
WVIEW_OFF(projectionMaxY, 0x1A08);
WVIEW_OFF(unk_1A0C, 0x1A0C);
WVIEW_OFF(suppressOptionalSubmitFlag, 0x1A0D);
WVIEW_OFF(projectionOffsetX, 0x1A10);
WVIEW_OFF(projectionOffsetY, 0x1A14);
WVIEW_OFF(unk_1A18, 0x1A18);
WVIEW_OFF(unk_1A1C, 0x1A1C);
WVIEW_OFF(unk_1A20, 0x1A20);
WVIEW_OFF(unk_1A24, 0x1A24);
WVIEW_OFF(unk_1A28, 0x1A28);
WVIEW_OFF(unk_1A2C, 0x1A2C);
WVIEW_OFF(unk_1A30, 0x1A30);
WVIEW_OFF(matrixUpdateFlags, 0x1A34);
WVIEW_OFF(savedViewMatrix, 0x1A38);
WVIEW_OFF(projectionMatrix, 0x1A78);
#undef WVIEW_OFF

#endif // CLASSES_WVIEW_H
