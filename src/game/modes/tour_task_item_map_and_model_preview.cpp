#include <decomp/static_init.h>
#include <decomp/local_class.h>
#define PREVIEW_FIELD_OFFSET(field) ((int)&((TourModelPreviewView*)0)->field)
enum { kPreviewTranslationOffset = 0x28, kPreviewCameraOffset = 0x54 };

enum ItemMapDirection { kMapLeft = 0, kMapRight = 1, kMapUp = 2, kMapDown = 3 };
enum { kItemMapColumns = 5, kItemMapLastColumn = 4, kItemMapLastRow = 6 };

// Additional fields not yet present in OpenItemEntry.h; flags live in separate storage.
struct ItemMapEntryView {
    void* record;
    short index;
    char padding006[2];
    float position[2];
    char padding010[8];
    unsigned short* flags;
};
struct PreviewAnimationRange {
    float start, end;
    char padding008[0x10];
    void* name;
    void* nextName;
};
// Copy representation for the 0x68-byte render-parameter block; no constructors.
struct PreviewRenderParams {
    float prefix;
    float colour[4];
    char padding014[0x2c];
    float lightDirection[3];
    char padding04c[0x1c];
};

#include "classes/TourModelPreviewView.h"
#include "classes/WView.h"
// ModelPreview_Ctor (0x802F5898): camera returned by ResourceManager_CreateWView.
#include "classes/OpenItemEntry.h"
#define AsOpenItemEntry(p) ((OpenItemEntry*)(p))

#include "game/wvector.h"
#include "classes/CTourTask.h"
#include <decomp/cw_bitfields.h>

#define AsCTourTask(p) ((CTourTask*)(p))
// Translation unit 0x802F562C-0x802F61E4 (30 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {
void gtl_Vec2_Copy(void*, void*);
void Vec3_CopySourceToDestination(void*, void*);
void* CTask_Ctor(void*);

extern const float kFloat_6_2831855;
extern float s_previewUnitTwoPi0;
extern float s_previewUnitTwoPi1;
extern const float kPreviewFovAspectScale;
extern const float kPreviewZero;
extern const float kPreviewDefaultCameraY;
extern const float kPreviewDefaultCameraZ;
extern const float kPreviewDefaultFieldOfView;
extern const float kPreviewOne;
extern const float kPreviewLightDiagonalComponent;
extern const float kPreviewLightRotationY;
extern char __vt__9CTourTask[];
extern int g_defaultActorSkipMask;
extern int g_actorSkipMask;
extern void* g_resourceManager;
void ResourceManager_ReleaseObject(void*, void*, int);
void Camera_SetFov(void*, float);
void WView_SetViewMatrix(void*, void*);
void WView_InvalidateViewAndProjection(void*);
void Camera_BuildFrustumPlanes(void*);
int strcmp(const char*, const char*);
void* WPuppet_FindAnimationRangeByName(void*, const char*);
void Matrix_SetIdentityAffine(void*);
void* ResourceManager_LoadByExtension(void*, const char*, int, int, int);
void PSMTX44Identity(void*);
void* ModelRenderParams_GetShared();
void ModelRenderParams_Copy(void*, void*);
void Mtx44_TransformDirection(void*, void*, void*);
void Mtx44_SetRotationY(void*, float);
void Vec3_Normalize(void*, void*);
void WPuppet_SetRenderParams(void*, void*);
void WPuppet_SetRenderParamsEnabled(void*, int);
void __as__7WVectorFRC7WVector(void*, void*);
void __dl__FPv(void*);
void Matrix_SetBasisElements(void*, float, float, float, float, float, float, float, float, float);
int Matrix_SetAxisRotation(void*, float, int);
double atan(double);
void Matrix_CopyAffine(void*, void*);
void* ResourceManager_CreateWView(void*, int);
void WPuppet_SetAnimationTimeFromRanges(void*, int, float, int, int);
void Model_SetMatrixRecursive(void*, void*, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);

// Set the separate navigation flag for one edge of an item-map cell.
void OpenItemEntry_SetDirectionEnabled(void* entry, int direction, int enabled) {
    switch (direction) {
        case kMapLeft: {
            struct S { CW_BITS3( unsigned short pad:7, unsigned short directionEnabled:1, unsigned short rest:8) };
            S* flags = (S*)((ItemMapEntryView*)entry)->flags;
            flags->directionEnabled = (unsigned int)enabled;
            break;
        }
        case kMapRight: {
            struct S { CW_BITS3( unsigned short pad:8, unsigned short directionEnabled:1, unsigned short rest:7) };
            S* flags = (S*)((ItemMapEntryView*)entry)->flags;
            flags->directionEnabled = (unsigned int)enabled;
            break;
        }
        case kMapUp: {
            struct S { CW_BITS3( unsigned short pad:9, unsigned short directionEnabled:1, unsigned short rest:6) };
            S* flags = (S*)((ItemMapEntryView*)entry)->flags;
            flags->directionEnabled = (unsigned int)enabled;
            break;
        }
        case kMapDown: {
            struct S { CW_BITS3( unsigned short pad:10, unsigned short directionEnabled:1, unsigned short rest:5) };
            S* flags = (S*)((ItemMapEntryView*)entry)->flags;
            flags->directionEnabled = (unsigned int)enabled;
            break;
        }
    }
}

// Store the item-map cell position.
void OpenItemEntry_SetPosition(void* entry, void* position) {
    gtl_Vec2_Copy(((ItemMapEntryView*)entry)->position, position);
}

// Copy the item-map cell position to the caller.
void OpenItemEntry_GetPosition(void* entry, void* position) {
    gtl_Vec2_Copy(position, ((ItemMapEntryView*)entry)->position);
}

// Bind the item-map cell to its item record.
void OpenItemEntry_SetRecord(void* entry, void* record) {
    AsOpenItemEntry(entry)->record = record;
}

// Read item-map record flag bit 15; the record owns this flag word.
int OpenItemEntry_GetFlagBit15(void* entry) {
    unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
    return (flags[0] >> 15) & 1;
}

// Read item-map record flag bit 14; the record owns this flag word.
int OpenItemEntry_GetFlagBit14(void* entry) {
    unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
    return (flags[0] >> 14) & 1;
}

// Read item-map record flag bit 13; the record owns this flag word.
int OpenItemEntry_GetFlagBit13(void* entry) {
    unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
    return (flags[0] >> 13) & 1;
}

// Return success unconditionally for this preview interface hook.
int ModelPreview_ReturnSuccess(void) {
    return 1;
}

// Initialize the first preview angle-wrap constant to one full turn.
void PreviewUnit_InitTwoPi0(void) {
    s_previewUnitTwoPi0 = kFloat_6_2831855;
}

// Initialize the second preview angle-wrap constant to one full turn.
void PreviewUnit_InitTwoPi1(void) {
    s_previewUnitTwoPi1 = kFloat_6_2831855;
}

// Read item-map record flag bit 10; the record owns this flag word.
int OpenItemEntry_GetFlagBit10(void* entry) {
    unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
    return (flags[0] >> 10) & 1;
}

// Set the translation component of the preview model matrix.
void ModelPreview_SetPosition(void* preview, void* position) {
    Vec3_CopySourceToDestination(position, (char*)&AsModelPreview802F5898(preview)->matrix[9]);
}

// Write item-map record flag bit 15 without changing the other flags.
void OpenItemEntry_SetFlagBit15(void* entry, int enabled) {
    struct BF { CW_BITS2( unsigned short high:1, unsigned short low:15) };
    BF* flags = (BF*)((ItemMapEntryView*)entry)->flags;
    flags->high = (unsigned int)enabled;
}

// Write item-map record flag bit 14 without changing the other flags.
void OpenItemEntry_SetFlagBit14(void* entry, int enabled) {
    struct BF { CW_BITS3( unsigned short b15:1, unsigned short b14:1, unsigned short rest:14) };
    BF* flags = (BF*)((ItemMapEntryView*)entry)->flags;
    flags->b14 = (unsigned int)enabled;
}

// Write item-map record flag bit 13 without changing the other flags.
void OpenItemEntry_SetFlagBit13(void* entry, int enabled) {
    struct BF { CW_BITS4( unsigned short b15:1, unsigned short b14:1, unsigned short b13:1, unsigned short rest:13) };
    BF* flags = (BF*)((ItemMapEntryView*)entry)->flags;
    flags->b13 = (unsigned int)enabled;
}

// Write item-map record flag bit 10 without changing the other flags.
void OpenItemEntry_SetFlagBit10(void* entry, int enabled) {
    struct BF { CW_BITS3( unsigned short top5:5, unsigned short b10:1, unsigned short rest:10) };
    BF* flags = (BF*)((ItemMapEntryView*)entry)->flags;
    flags->b10 = (unsigned int)enabled;
}

// Return the selected animation name when both model and range exist.
void* ModelPreview_GetAnimationName(void* preview) {
    // Original 0x802F5D34: missing model and animation share the null result.
    void* p;
    if (AsModelPreview802F5898(preview)->model == 0 ||
        (p = AsModelPreview802F5898(preview)->animation) == 0) return 0;
    return ((PreviewAnimationRange*)p)->name;
}

// Render the loaded preview model through its camera.
void ModelPreview_Render(void* a) {
    void* p = AsModelPreview802F5898(a)->model;
    if (!p) return;
    void* q = AsModelPreview802F5898(a)->camera;
    struct V { 
    virtual void a0() DECOMP_LOCAL_VIRTUAL; virtual void a1() DECOMP_LOCAL_VIRTUAL; virtual void a2() DECOMP_LOCAL_VIRTUAL; virtual void a3() DECOMP_LOCAL_VIRTUAL; virtual void m(void*, int, float, int, int, int) DECOMP_LOCAL_VIRTUAL;
    };
    ((V*)p)->m(q, 1, kPreviewOne, 0, 1, 0);
}

struct TourTask { void* v0; void* v1; void* v2; };
// Construct the tour task interfaces and reset the actor skip masks.
void* CTourTask_Ctor(void* storage) {
    void* self = storage;
    CTask_Ctor(self);
    char* taskVtables = (char*)__vt__9CTourTask;
    AsCTourTask(self)->vtbl = (void*)taskVtables;
    AsCTourTask(self)->secondaryVtable = (void*)(taskVtables + 0x10);
    AsCTourTask(self)->tertiaryVtable = (void*)(taskVtables + 0x1c);
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
    return self;
}

// Release the preview model and restore the saved camera view and field of view.
void ModelPreview_ReleaseModel(void* preview) {
    if (AsModelPreview802F5898(preview)->model != 0) {
        ResourceManager_ReleaseObject(g_resourceManager, AsModelPreview802F5898(preview)->model, 0);
        AsModelPreview802F5898(preview)->model = 0;
    }
    Camera_SetFov(AsModelPreview802F5898(preview)->camera, AsModelPreview802F5898(preview)->savedFieldOfView);
    WView_SetViewMatrix(AsModelPreview802F5898(preview)->camera, (char*)&AsModelPreview802F5898(preview)->savedViewMatrix);
    Camera_BuildFrustumPlanes(AsModelPreview802F5898(preview)->camera);
}

// Compare a non-null requested animation name with the active preview range.
int ModelPreview_IsAnimationNamed(void* preview, const char* animationName) {
    // Original 0x802F5D5C: select preview nullable animation name before comparing.
    if (AsModelPreview802F5898(preview)->model == 0 ||
        (preview = AsModelPreview802F5898(preview)->animation) == 0) preview = 0;
    else preview = ((PreviewAnimationRange*)preview)->name;
    int r = 0;
    if (preview != 0 && animationName != 0) {
        if (strcmp((const char*)preview, animationName) == 0) r = 1;
    }
    return r;
}

// Build the preview camera view from its offset and apply its field of view.
void ModelPreview_UpdateCamera(void* self) {
    float viewMatrix[12];
    Matrix_SetBasisElements(viewMatrix, kPreviewOne, kPreviewZero, kPreviewZero, kPreviewZero, kPreviewOne, kPreviewZero, kPreviewZero, kPreviewZero, kPreviewOne);
    *(WVector*)&viewMatrix[9] = AsModelPreview802F5898(self)->cameraOffset;
    float elevationRatio = AsModelPreview802F5898(self)->cameraOffset.y / AsModelPreview802F5898(self)->cameraOffset.z;
    float elevationRadians = (float)atan((double)elevationRatio);
    viewMatrix[7] = elevationRadians;
    WView_InvalidateViewAndProjection(AsModelPreview802F5898(self)->camera);
    WView_SetViewMatrix(AsModelPreview802F5898(self)->camera, viewMatrix);
    Camera_SetFov(AsModelPreview802F5898(self)->camera, AsModelPreview802F5898(self)->fieldOfView);
    Camera_BuildFrustumPlanes(AsModelPreview802F5898(self)->camera);
}

// Release model and camera resources, optionally freeing the preview object.
void* ModelPreview_Dtor(void* preview, int deleting) {
    void* self = preview;
    if (self != 0) {
        if (AsModelPreview802F5898(self)->model != 0) {
            ResourceManager_ReleaseObject(g_resourceManager, AsModelPreview802F5898(self)->model, 0);
            AsModelPreview802F5898(self)->model = 0;
        }
        Camera_SetFov(AsModelPreview802F5898(self)->camera, AsModelPreview802F5898(self)->savedFieldOfView);
        WView_SetViewMatrix(AsModelPreview802F5898(self)->camera, (char*)&AsModelPreview802F5898(self)->savedViewMatrix);
        Camera_BuildFrustumPlanes(AsModelPreview802F5898(self)->camera);
        if (AsModelPreview802F5898(self)->camera != 0) {
            if (AsModelPreview802F5898(self)->camera != 0) {
                struct V { 
                virtual void m2(int) DECOMP_LOCAL_VIRTUAL;
                };
                ((V*)AsModelPreview802F5898(self)->camera)->m2(1);
            }
            AsModelPreview802F5898(self)->camera = 0;
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Rotate the preview model about Y while preserving its translation.
void ModelPreview_SetHeading(void* preview, float headingRadians) {
    char* base = (char*)preview;
    float savedPosition[4];
    float rotation[13];
    *(WVector*)savedPosition = *(WVector*)(base + kPreviewTranslationOffset);
    AsModelPreview802F5898(base)->heading = headingRadians;
    Matrix_SetAxisRotation(rotation, headingRadians, 1);
    Matrix_CopyAffine(AsModelPreview802F5898(base)->matrix, rotation);
    *(WVector*)(base + kPreviewTranslationOffset) = *(WVector*)savedPosition;
}
// Select a different named animation range at its first frame. An unchanged
// name or failed lookup leaves the current range and playback time untouched.
void ModelPreview_SetAnimationByName(void* preview, void* animationName) {
    void* self = preview;
    void* name = animationName;
    void* p;
    int same;
    // Original 0x802F5C5C: all failed lookups leave the selected range alone.
    do {
        if (AsModelPreview802F5898(self)->model == 0) break;
        if (name == 0) break;
        p = WPuppet_FindAnimationRangeByName(AsModelPreview802F5898(self)->model, (const char*)name);
        if (p == 0) break;
        {
            void* cn;
            if (AsModelPreview802F5898(self)->model == 0 ||
                (cn = AsModelPreview802F5898(self)->animation) == 0) cn = 0;
            else cn = ((PreviewAnimationRange*)cn)->name;
            same = 0;
            if (cn != 0 && name != 0) {
                if (strcmp((const char*)cn, (const char*)name) == 0) same = 1;
            }
        }
        if (same != 0) break;
        AsModelPreview802F5898(self)->animation = p;
        float f0 = ((PreviewAnimationRange*)p)->start;
        AsModelPreview802F5898(self)->animationStart = f0;
        AsModelPreview802F5898(self)->animationTime = f0;
        AsModelPreview802F5898(self)->animationEnd = ((PreviewAnimationRange*)p)->end;
    } while (0);
}

// Release the old model, reset preview defaults and load the requested resource.
void ModelPreview_LoadModel(void* preview, void* modelName) {
    void* self = preview;
    void* arg = modelName;
    if (AsModelPreview802F5898(self)->model != 0) {
        ResourceManager_ReleaseObject(g_resourceManager, AsModelPreview802F5898(self)->model, 0);
        AsModelPreview802F5898(self)->model = 0;
    }
    Camera_SetFov(AsModelPreview802F5898(self)->camera, AsModelPreview802F5898(self)->savedFieldOfView);
    WView_SetViewMatrix(AsModelPreview802F5898(self)->camera, (char*)&AsModelPreview802F5898(self)->savedViewMatrix);
    Camera_BuildFrustumPlanes(AsModelPreview802F5898(self)->camera);
    AsModelPreview802F5898(self)->animation = 0;
    Matrix_SetIdentityAffine((char*)&AsModelPreview802F5898(self)->matrix);
    float tmp[3];
    tmp[0] = kPreviewZero;
    tmp[1] = kPreviewDefaultCameraY;
    tmp[2] = kPreviewDefaultCameraZ;
    AsModelPreview802F5898(self)->cameraOffset = *(WVector*)tmp;
    float f0 = kPreviewZero;
    float f1 = kPreviewDefaultFieldOfView;
    AsModelPreview802F5898(self)->animationTime = f0;
    AsModelPreview802F5898(self)->fieldOfView = f1;
    AsModelPreview802F5898(self)->animationStart = f0;
    AsModelPreview802F5898(self)->animationEnd = f0;
    void* r = ResourceManager_LoadByExtension(g_resourceManager, (const char*)arg, 0, 0, 0);
    AsModelPreview802F5898(self)->model = r;
}

// Copy shared render parameters, set white colour and rotate/normalize the light direction.
void ModelPreview_SetupRenderParams(void* preview) {
    void* self = preview;
    if (AsModelPreview802F5898(self)->model == 0) return;
    char lightTransform[0x40];
    PSMTX44Identity(lightTransform);
    struct Transform { char data[0x68]; };
    Transform copied;
    Transform first;
    ModelRenderParams_Copy(&first, ModelRenderParams_GetShared());
    copied = first;
    ((PreviewRenderParams*)&copied)->lightDirection[1] = kPreviewLightDiagonalComponent;
    ((PreviewRenderParams*)&copied)->lightDirection[0] = kPreviewZero;
    ((PreviewRenderParams*)&copied)->lightDirection[2] = kPreviewLightDiagonalComponent;
    ((PreviewRenderParams*)&copied)->colour[0] = kPreviewOne;
    ((PreviewRenderParams*)&copied)->colour[1] = kPreviewOne;
    ((PreviewRenderParams*)&copied)->colour[2] = kPreviewOne;
    ((PreviewRenderParams*)&copied)->colour[3] = kPreviewOne;
    Mtx44_TransformDirection(((PreviewRenderParams*)&copied)->lightDirection, ((PreviewRenderParams*)&copied)->lightDirection, lightTransform);
    char rotation[0x40];
    Mtx44_SetRotationY(rotation, kPreviewLightRotationY);
    Mtx44_TransformDirection(((PreviewRenderParams*)&copied)->lightDirection, ((PreviewRenderParams*)&copied)->lightDirection, rotation);
    Vec3_Normalize(((PreviewRenderParams*)&copied)->lightDirection, ((PreviewRenderParams*)&copied)->lightDirection);
    WPuppet_SetRenderParams(AsModelPreview802F5898(self)->model, &copied);
    WPuppet_SetRenderParamsEnabled(AsModelPreview802F5898(self)->model, 1);
}


// Return the enabled adjacent cell in the five-column, seven-row item map, or -1.
int OpenItemEntry_GetNeighborIndex(void* entry, int direction) {
    int neighborIndex = -1;
    switch (direction) {
        case kMapLeft: {
            short index = AsOpenItemEntry(entry)->index;
            int column = index % kItemMapColumns;
            if (column > 0) {
                unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
                if ((flags[0] >> 8) & 1) {
                    neighborIndex = (short)(index - 1);
                }
            }
            break;
        }
        case kMapRight: {
            short index = AsOpenItemEntry(entry)->index;
            int column = index % kItemMapColumns;
            if (column < kItemMapLastColumn) {
                unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
                if ((flags[0] >> 7) & 1) {
                    neighborIndex = (short)(index + 1);
                }
            }
            break;
        }
        case kMapUp: {
            short index = AsOpenItemEntry(entry)->index;
            int row = index / kItemMapColumns;
            if (row > 0) {
                unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
                if ((flags[0] >> 6) & 1) {
                    neighborIndex = (short)(index - kItemMapColumns);
                }
            }
            break;
        }
        case kMapDown: {
            short index = AsOpenItemEntry(entry)->index;
            int row = index / kItemMapColumns;
            if (row < kItemMapLastRow) {
                unsigned short* flags = (unsigned short*)((ItemMapEntryView*)entry)->flags;
                if ((flags[0] >> 5) & 1) {
                    neighborIndex = (short)(index + kItemMapColumns);
                }
            }
            break;
        }
    }
    return neighborIndex;
}

// Create a preview camera, save its view and optionally load an initial model.
void* ModelPreview_Ctor(void* self, void* initialModelName) {
    char* position = (char*)self;
    AsModelPreview802F5898(position)->model = 0;
    void* camera = ResourceManager_CreateWView(g_resourceManager, 0);
    AsModelPreview802F5898(position)->camera = camera;
    float aspectRatio = ((WView*)camera)->viewportWidth / ((WView*)camera)->viewportHeight;
    AsModelPreview802F5898(position)->savedFieldOfView = ((WView*)camera)->fieldOfView / (kPreviewFovAspectScale * aspectRatio);
    Matrix_CopyAffine(position + PREVIEW_FIELD_OFFSET(savedViewMatrix), (char*)&((WView*)camera)->viewMatrix);
    (*(int*)&AsModelPreview802F5898(position)->animation) = 0;
    Matrix_SetIdentityAffine(position + PREVIEW_FIELD_OFFSET(matrix));
    float cameraOffset[3];
    cameraOffset[0] = kPreviewZero;
    cameraOffset[1] = kPreviewDefaultCameraY;
    cameraOffset[2] = kPreviewDefaultCameraZ;
    AsModelPreview802F5898(position)->cameraOffset = *(WVector*)cameraOffset;
    {
        float f0 = kPreviewZero;
        float f1 = kPreviewDefaultFieldOfView;
        AsModelPreview802F5898(position)->animationTime = f0;
        AsModelPreview802F5898(position)->fieldOfView = f1;
        AsModelPreview802F5898(position)->animationStart = f0;
        AsModelPreview802F5898(position)->animationEnd = f0;
    }
    // Original 0x802F5898: optional model loading follows common initialization.
    if (initialModelName != 0) {
        if (AsModelPreview802F5898(position)->model != 0) {
            ResourceManager_ReleaseObject(g_resourceManager, AsModelPreview802F5898(position)->model, 0);
            AsModelPreview802F5898(position)->model = 0;
        }
        Camera_SetFov(AsModelPreview802F5898(position)->camera, AsModelPreview802F5898(position)->savedFieldOfView);
        WView_SetViewMatrix(AsModelPreview802F5898(position)->camera, position + PREVIEW_FIELD_OFFSET(savedViewMatrix));
        Camera_BuildFrustumPlanes(AsModelPreview802F5898(position)->camera);
        (*(int*)&AsModelPreview802F5898(position)->animation) = 0;
        Matrix_SetIdentityAffine(position + PREVIEW_FIELD_OFFSET(matrix));
        float loadCameraOffset[3];
        loadCameraOffset[0] = kPreviewZero;
        loadCameraOffset[1] = kPreviewDefaultCameraY;
        loadCameraOffset[2] = kPreviewDefaultCameraZ;
        *(WVector*)(position + kPreviewCameraOffset) = *(WVector*)loadCameraOffset;
        {
            float f0 = kPreviewZero;
            float f1 = kPreviewDefaultFieldOfView;
            AsModelPreview802F5898(position)->animationTime = f0;
            AsModelPreview802F5898(position)->fieldOfView = f1;
            AsModelPreview802F5898(position)->animationStart = f0;
            AsModelPreview802F5898(position)->animationEnd = f0;
        }
        AsModelPreview802F5898(position)->model = ResourceManager_LoadByExtension(g_resourceManager, (const char*)initialModelName, 0, 0, 0);
    }
    return self;
}

// Advance the selected range, loop or follow its next name, then update model matrices.
void ModelPreview_UpdateAnimation(void* self, float delta) {
    void* preview = self;
    void* modelOrRange = AsModelPreview802F5898(self)->model;
    // Original 0x802F5DCC: range transitions share the final model update.
    if (modelOrRange != 0) {
        do {
            void* range = AsModelPreview802F5898(self)->animation;
            if (range == 0) break;
            {
                float f0 = AsModelPreview802F5898(self)->animationEnd;
                float f2 = AsModelPreview802F5898(self)->animationTime;
                float f1 = f2 + delta;
                AsModelPreview802F5898(self)->animationTime = f1;
                if (!(f0 <= f1)) break;
            }
            void* nextAnimationName = ((PreviewAnimationRange*)range)->nextName;
            void* namePtr;
            if (modelOrRange != 0 && range != 0) namePtr = ((PreviewAnimationRange*)range)->name;
            else namePtr = 0;
            int sameAnimation = 0;
            if (namePtr != 0 && nextAnimationName != 0) {
                if (strcmp((const char*)namePtr, (const char*)nextAnimationName) == 0) sameAnimation = 1;
            }
            if (sameAnimation != 0) {
                float f2 = AsModelPreview802F5898(preview)->animationTime;
                float f1 = AsModelPreview802F5898(preview)->animationEnd;
                float f0 = AsModelPreview802F5898(preview)->animationStart;
                f1 = f2 - f1;
                f0 = f0 + f1;
                AsModelPreview802F5898(preview)->animationTime = f0;
                break;
            }
            if (modelOrRange == 0) break;
            if (nextAnimationName == 0) break;
            void* nextRange = WPuppet_FindAnimationRangeByName(modelOrRange, (const char*)nextAnimationName);
            modelOrRange = nextRange;
            if (nextRange == 0) break;
            void* currentName;
            if (AsModelPreview802F5898(preview)->model == 0 ||
                (currentName = AsModelPreview802F5898(preview)->animation) == 0) currentName = 0;
            else currentName = ((PreviewAnimationRange*)currentName)->name;
            sameAnimation = 0;
            if (currentName != 0 && nextAnimationName != 0) {
                if (strcmp((const char*)currentName, (const char*)nextAnimationName) == 0) sameAnimation = 1;
            }
            if (sameAnimation != 0) break;
            AsModelPreview802F5898(preview)->animation = modelOrRange;
            float f0 = ((PreviewAnimationRange*)modelOrRange)->start;
            AsModelPreview802F5898(preview)->animationStart = f0;
            AsModelPreview802F5898(preview)->animationTime = f0;
            float f0b = ((PreviewAnimationRange*)modelOrRange)->end;
            AsModelPreview802F5898(preview)->animationEnd = f0b;

        } while (0);
        {
            void* obj = AsModelPreview802F5898(preview)->model;
            WPuppet_SetAnimationTimeFromRanges(obj, 0, AsModelPreview802F5898(preview)->animationTime, 0, 0);
            obj = AsModelPreview802F5898(preview)->model;
            Model_SetMatrixRecursive(obj, obj, (char*)&AsModelPreview802F5898(preview)->matrix);
            obj = AsModelPreview802F5898(preview)->model;
            WPuppet_UpdateBoneWorldMatrices(obj, 0, 0, kPreviewOne);
        }
    }
}


} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462590", _ctors_u802F562C_0, PreviewUnit_InitTwoPi0);
STATIC_INIT_ENTRY(".CRT$XCU80462590", _ctors_u802F562C_1, PreviewUnit_InitTwoPi1);
#pragma pop

