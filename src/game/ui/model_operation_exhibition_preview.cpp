// Translation unit 0x802D95DC-0x802D9CC4 (13 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/types.h"

extern "C" {

extern int CAsyncExhibition_UpdatePointerRotation(void* p);
extern int CAsyncExhibition_OffsetRotation(void* p, void* offset);
extern int CAsyncExhibition_PollReady(void* p);
extern void __dl__FPv(void* p, int n);
extern float CExhibition_model_pos_GetCameraDistanceLimit1(void* p, int id);
extern float CExhibition_model_pos_GetCameraDistanceLimit2(void* p, int id);
extern float CExhibition_model_pos_GetCameraDistance(void* p, int id);
extern float CExhibition_model_pos_GetViewportCenterX(void* p, int id);
extern float CExhibition_model_pos_GetViewportBottom(void* p, int id);
extern float ExhibitionModelPosition_GetViewportBottomLimit1(void* p, int id);
extern float ExhibitionModelPosition_GetViewportBottomLimit2(void* p, int id);
extern float CExhibition_model_pos_GetRotationX(void* p, int id);
extern float CExhibition_model_pos_GetRotationLimit1(void* p, int id);
extern float CExhibition_model_pos_GetRotationLimit2(void* p, int id);
extern float CExhibition_model_pos_GetRotationY(void* p, int id);
extern void CAsyncExhibition_SetViewport(void* p, void* rect, float v);
extern void CAsyncExhibition_SetCameraDistanceLimits(void* p, float a, float b);
extern void CAsyncExhibition_SetRotationLimits(void* p, float a, float b);
extern void CAsyncExhibition_SetRotation(void* p, void* vec);
extern void CAsyncExhibition_SetViewportYLimits(void* p, int a, int b);
extern unsigned long Runtime_ConvertDoubleToUnsigned(double v);
extern float kFloat_1_ModelOperationCamera;
extern float kModelOperationPresetRotationZ;
extern float kFloat_0_5_ExhibitionViewport;

enum { kDefaultPreviewLeft = 300, kDefaultPreviewTop = 100,
       kDefaultPreviewRight = 400, kDefaultPreviewBottom = 350,
       kPresetViewportHeight = 250, kInvalidCameraPreset = -1,
       kPreviewVertexFormat = 0xF005, kTriangleStrip = 0x98 };

typedef struct ModelOperationState ModelOperationState;

struct ModelOperationState {
// Model-operation vtable: 0x08 exhibition deletion, 0x0C exhibition update, 0x10 exhibition draw / operation gate.
    virtual void vfn08(int); // virtual slot 0x08: exhibition deletion
    virtual void vfn0C(float); // virtual slot 0x0C: exhibition update
    virtual int vfn10(); // virtual slot 0x10: exhibition draw / operation gate
    virtual void vfn14(int);
    char reserved04[0x4];
    void* cameraPresets;
    unsigned char interactiveRotation;
    char padding0D[0x3];
    void* exhibition;
};

typedef struct PreviewViewportRect PreviewViewportRect;

struct PreviewViewportRect {
    int left;
    int top;
    int right;
    int bottom;
};

extern int ModelOperationBase_ApplyCameraPreset(ModelOperationState* self, int id);

// Apply pointer-driven rotation when the asynchronous preview exists.
int ModelOperationBase_UpdatePointerRotation(ModelOperationState* self) {
    if (self->exhibition != 0) {
        return CAsyncExhibition_UpdatePointerRotation(self->exhibition);
    }
    return 0;
}

// Offset the current preview rotation by the supplied vector.
int ModelOperationBase_OffsetRotation(ModelOperationState* self, void* offset) {
    if (self->exhibition != 0) {
        return CAsyncExhibition_OffsetRotation(self->exhibition, offset);
    }
    return 0;
}

// Return the default shop model viewport in screen coordinates.
int ModelOperationBase_GetDefaultViewport(int unused, PreviewViewportRect* viewport) {
    viewport->top = kDefaultPreviewTop;
    viewport->bottom = kDefaultPreviewBottom;
    viewport->right = kDefaultPreviewRight;
    viewport->left = kDefaultPreviewLeft;
    return 1;
}

// Enable pointer rotation and its viewport overlay.
int ModelOperationBase_EnableInteractiveRotation(ModelOperationState* self) {
    self->interactiveRotation = 1;
    return 1;
}

// Apply a preset and disable interactive rotation.
int ModelOperationBase_ApplyFixedCameraPreset(ModelOperationState* self, int presetId) {
    ModelOperationBase_ApplyCameraPreset(self, presetId);
    self->interactiveRotation = 0;
    int result = 1;
    return result;
}

// Optionally rotate from input, then advance the asynchronous exhibition.
int ModelOperationBase_Update(ModelOperationState* self, float deltaTime) {
    int result;
    if (self->exhibition == 0) {
        result = 0;
    } else {
        if (self->interactiveRotation != 0) {
            if (self->exhibition != 0) {
                CAsyncExhibition_UpdatePointerRotation(self->exhibition);
            }
        }
        ((ModelOperationState*)self->exhibition)->vfn0C(deltaTime);
        result = 1;
    }
    return result;
}

typedef struct Vtx Vtx;

struct Vtx {
    float x;
    float y;
    int z;
    int packedColor; // Diffuse color word in vertex format 0xF005; keep the original word type.
};

typedef struct Rect Rect;

struct Rect {
    int x0;
    int y0;
    int x1;
    int y1;
};

typedef struct Quad Quad;

struct Quad {
    Vtx v[4];
};

extern Rect s_modelOperationOverlayRect;
extern Quad s_modelOperationOverlayVertices;
extern void Engine_SetFlexibleVertexFormat(unsigned int arg);
extern void DrawPrimitiveUP(int prim, int count, void* verts, int stride);

// Draw the interactive viewport overlay, then render the exhibition.
int ModelOperationBase_Draw(ModelOperationState* self) {
    unsigned int result;
    if (0 == self->exhibition) {
        result = 0;
    } else {
        if (self->interactiveRotation) {
            Rect rect = s_modelOperationOverlayRect;

            Quad q = s_modelOperationOverlayVertices;
            q.v[0].x = (float)rect.x0;
            q.v[0].y = (float)rect.y0; q.v[1].x = (float)rect.x1;
            q.v[1].y = (float)rect.y0;
            q.v[2].x = (float)rect.x0;
            q.v[2].y = (float)rect.y1;
            q.v[3].x = (float)rect.x1;
            q.v[3].y = (float)rect.y1;
            Engine_SetFlexibleVertexFormat(kPreviewVertexFormat);
            DrawPrimitiveUP(kTriangleStrip, 2, &q, sizeof(Vtx));
        }
        ((ModelOperationState*)self->exhibition)->vfn10();
        result = 1;
    }
    return result;
}

// Report whether pointer rotation and its viewport overlay are enabled.
int ModelOperationBase_IsInteractiveRotationEnabled(ModelOperationState* self) {
    return self->interactiveRotation;
}

// Apply preset distance, rotation and viewport limits to the active preview.
int ModelOperationBase_ApplyCameraPreset(ModelOperationState* self, int presetId) {
    if (0 == self->cameraPresets) {
        return 0;
    }
    if (presetId == kInvalidCameraPreset) {
        return 0;
    }
    float minimumDistance = CExhibition_model_pos_GetCameraDistanceLimit1(self->cameraPresets, presetId);
    float maximumDistance = CExhibition_model_pos_GetCameraDistanceLimit2(self->cameraPresets, presetId);
    float minimumRotation = CExhibition_model_pos_GetRotationLimit1(self->cameraPresets, presetId);
    float maximumRotation = CExhibition_model_pos_GetRotationLimit2(self->cameraPresets, presetId);
    float rotationX = CExhibition_model_pos_GetRotationX(self->cameraPresets, presetId);
    float rotationY = CExhibition_model_pos_GetRotationY(self->cameraPresets, presetId);
    float cameraDistance = CExhibition_model_pos_GetCameraDistance(self->cameraPresets, presetId);
    float viewportCenterX = CExhibition_model_pos_GetViewportCenterX(self->cameraPresets, presetId);
    float viewportBottom = CExhibition_model_pos_GetViewportBottom(self->cameraPresets, presetId);
    float maximumViewportBottom = ExhibitionModelPosition_GetViewportBottomLimit2(self->cameraPresets, presetId);
    float minimumViewportBottom = ExhibitionModelPosition_GetViewportBottomLimit1(self->cameraPresets, presetId);

    int viewportHeight = kPresetViewportHeight;
    float viewportHeightFloat = (float)viewportHeight;
    Rect viewport;
    viewport.x0 = (int)(viewportCenterX - kFloat_1_ModelOperationCamera);
    viewport.x1 = (int)(kFloat_1_ModelOperationCamera + viewportCenterX);
    viewport.y1 = (int)viewportBottom;
    viewport.y0 = (int)(viewportBottom - viewportHeightFloat);
    Vec3f rotation;
    rotation.x = rotationX;
    rotation.y = rotationY;
    rotation.z = kModelOperationPresetRotationZ;

    CAsyncExhibition_SetCameraDistanceLimits(self->exhibition, minimumDistance, maximumDistance);
    CAsyncExhibition_SetRotationLimits(self->exhibition, minimumRotation, maximumRotation);
    CAsyncExhibition_SetRotation(self->exhibition, &rotation);
    CAsyncExhibition_SetViewport(self->exhibition, &viewport, cameraDistance);
    maximumViewportBottom -= viewportHeightFloat * kFloat_0_5_ExhibitionViewport;
    minimumViewportBottom -= viewportHeightFloat * kFloat_0_5_ExhibitionViewport;
    CAsyncExhibition_SetViewportYLimits(self->exhibition, Runtime_ConvertDoubleToUnsigned(minimumViewportBottom), Runtime_ConvertDoubleToUnsigned(maximumViewportBottom));
    return 1;
}

// Treat an absent preview as ready; otherwise poll its asynchronous load.
int ShopPreview_PollReady(ModelOperationState* self) {
    int result = 1;
    if (self->exhibition != 0) {
        if (CAsyncExhibition_PollReady(self->exhibition) == 0) {
            result = 0;
        }
    }
    return result;
}

// Delete the camera-preset object when called as a deleting destructor.
ModelOperationState* CExhibition_model_pos_dtor(ModelOperationState* self, int deleting) {
    if (self != 0 && deleting > 0) {
        __dl__FPv(self, deleting);
    }
    return self;
}

// Invoke virtual slot 0x10 and forward the argument only when it succeeds.
int ModelOperationBase_TryApplyArgument(ModelOperationState* self, int arg) {
    if (self->vfn10() == 0) {
        return 0;
    }
    self->vfn14(arg);
    return 1;
}

// Destroy the asynchronous exhibition and clear the owning pointer.
int ModelOperationBase_ReleaseExhibition(ModelOperationState* self) {
    if (self->exhibition != 0) {
        if (self->exhibition != 0) {
            ((ModelOperationState*)self->exhibition)->vfn08(1);
        }
        self->exhibition = 0;
    }
    return 1;
}


// Guard the local views used by this unit against accidental layout changes.
#define POLISH_OFFSET(T, F, O) typedef char polish_##T##_##F[((int)&((T*)0)->F == (O)) ? 1 : -1]
POLISH_OFFSET(ModelOperationState, cameraPresets, 0x8);
POLISH_OFFSET(ModelOperationState, interactiveRotation, 0xc);
POLISH_OFFSET(ModelOperationState, exhibition, 0x10);
#undef POLISH_OFFSET
typedef char polish_size_ModelOperationState[(sizeof(ModelOperationState) == 0x14) ? 1 : -1];

} // extern "C"
