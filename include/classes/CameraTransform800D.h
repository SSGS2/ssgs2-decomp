// Global affine camera transform s_cameraTransform, used by CCamera methods in camera_movement_and_input.cpp.
// CCamera::UpdateForwardMovement passes +0x24 to vector operations; CCamera::HandleMovementInput copies that position.
#ifndef CLASSES_CAMERATRANSFORM800D_H
#define CLASSES_CAMERATRANSFORM800D_H
struct CameraTransform800D {
    float basis[9];
    float position[3]; // 0x24: affine translation; +0x28 is camera height.
};
#define OFF(f, o) typedef char CameraTransform800D_off_##f[((int)&((CameraTransform800D*)0)->f == (o)) ? 1 : -1]
OFF(basis, 0x0);
OFF(position, 0x24);
#undef OFF
#define AsCameraTransform800D(p) ((CameraTransform800D*)(p))
#endif
