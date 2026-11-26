// A character or model preview: owns the exhibition that renders the model, the
// selection it last resolved to, the attachment slots, and the state of the
// loading indicator and pointer-driven input.
#ifndef CLASSES_CASYNCEXHIBITION_H
#define CLASSES_CASYNCEXHIBITION_H

#include "game/types.h"

class CAsyncExhibition {
public:
    void* vtbl;                             // 0x0: original vtable pointer
    int viewportLeft;                       // 0x4
    int viewportTop;                        // 0x8
    int viewportRight;                      // 0xc
    int viewportBottom;                     // 0x10
    float cameraDistance;                   // 0x14
    unsigned char previewMode;              // 0x18
    char pad_019[0x3];                      // 0x19
    void* preloadObject;                    // 0x1c
    unsigned char preloaderExternallyOwned; // 0x20
    char pad_021[0x3];                      // 0x21
    char displayDescriptor
        [0xc8]; // 0x24: CAsyncExhibition_Ctor calls ResolvedItemSelection_Clear; CAsyncExhibition_InstallResolvedSelectionCallback supplies it to CExhibition::CopyResolvedAvatarSelection
    void* attachments
        [5]; // 0xec: ToggleAttachment checks/updates one of five slots after CExhibition::SetAttachment; ctor clears 0x14 bytes
    float previousPointerX;              // 0x100
    float previousPointerY;              // 0x104
    float dragStartDepth;                // 0x108
    unsigned char pointerDragActive;     // 0x10c
    char pad_10D[0x3];                   // 0x10d
    void* exhibition;                    // 0x110
    float loadingIndicatorX;             // 0x114
    float loadingIndicatorY;             // 0x118
    int bpEntryId;                       // 0x11c
    unsigned char loadingIndicatorShown; // 0x120

    void* CopyValidatedItemChanges(void* other, unsigned int key);
    int UpdateLoadingStateAndIndicator(float delta);
    int PollReady();
    void SetScriptEnabled(int value);
    int Draw();
    int HandleCameraInput();
    int UpdatePointerRotation();
    void SetLoadingIndicatorPosition(const float* position);

    // Attachments, texture variants and rotation of the model being shown.
    int ToggleAttachment(unsigned int index, void* value);
    int ApplyTextureVariant(void* source, void* arg);
    int OffsetRotation(Vec3f* offset);

    void* vfunc_9();
    void* SetViewport(void* rectangle, float amount);
    int SetCameraDistanceLimits(float minimumDistance, float maximumDistance);
    int SetRotationLimits(float minimumRotation, float maximumRotation);
    int SetRotation(void* vec);
    int SetViewportYLimits(unsigned int minimumYBits, unsigned int maximumYBits);
    static int PlayFixedNameAnimation(void* preview, void* q);
    void* GetExhibition();
    static int ReplaceExhibition(void* preview, void* viewportRectangle, float cameraDistance,
                                 unsigned char previewMode);
    static int TryInstallPreloadedObject(void* preview);

    ~CAsyncExhibition();

    CAsyncExhibition();
};

#define CASYNCEXHIBITION_OFF(f, o) \
    typedef char CAsyncExhibition_off_##f[((int)&((CAsyncExhibition*)0)->f == (o)) ? 1 : -1]
CASYNCEXHIBITION_OFF(viewportLeft, 0x4);
CASYNCEXHIBITION_OFF(viewportTop, 0x8);
CASYNCEXHIBITION_OFF(viewportRight, 0xc);
CASYNCEXHIBITION_OFF(viewportBottom, 0x10);
CASYNCEXHIBITION_OFF(cameraDistance, 0x14);
CASYNCEXHIBITION_OFF(previewMode, 0x18);
CASYNCEXHIBITION_OFF(preloadObject, 0x1c);
CASYNCEXHIBITION_OFF(preloaderExternallyOwned, 0x20);
CASYNCEXHIBITION_OFF(displayDescriptor, 0x24);
CASYNCEXHIBITION_OFF(attachments, 0xec);
CASYNCEXHIBITION_OFF(previousPointerX, 0x100);
CASYNCEXHIBITION_OFF(previousPointerY, 0x104);
CASYNCEXHIBITION_OFF(dragStartDepth, 0x108);
CASYNCEXHIBITION_OFF(pointerDragActive, 0x10c);
CASYNCEXHIBITION_OFF(exhibition, 0x110);
CASYNCEXHIBITION_OFF(loadingIndicatorX, 0x114);
CASYNCEXHIBITION_OFF(loadingIndicatorY, 0x118);
CASYNCEXHIBITION_OFF(bpEntryId, 0x11c);
CASYNCEXHIBITION_OFF(loadingIndicatorShown, 0x120);
#undef CASYNCEXHIBITION_OFF
typedef char CAsyncExhibition_size[(sizeof(CAsyncExhibition) == 0x124) ? 1 : -1];

#endif // CLASSES_CASYNCEXHIBITION_H
