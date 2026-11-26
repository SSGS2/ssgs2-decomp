// Golf aiming and landing markers: the tutorial target, the other players'
// markers, the colored aim marker with its distance label, the fading pulse at
// the landing point, and the marker's message handler.
//
// Positions are world space with Y up; the screen records hold pixels. Drawing
// needs the shared document and a loaded set of nine marker textures.
#include "classes/CGolfDoc.h"
#include "classes/CSharedDoc.h"
#include "classes/CSpot.h"
#include "classes/WView.h"
#include "game/types.h"
#include "game/wvector.h"

extern "C" {

void* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, int);
void* Camera_GetRenderState(void*);
void* GolfBall__Fv();
void* GolfPlayer_GetEntryByIndex(void*, int);
int SharedDoc_IsPlayerFlag0Set(int);
double atan2(float, float);
int GX_GetPrimitiveCount(int, int);

// Engine submission.
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetType7Resource(int);
void SetTransform(int, void*);
void DrawPrimitiveUP(int, int, void*, int);

// Geometry and matrices.
void* Matrix_SetAxisRotation(void*, float, int);
void* Matrix_TransformPoint(void*, void*, void*);
void Matrix_CopyAffine(void*, void*);
void Matrix3x3_ScaleAssign(void*, float);
void Model_SetMatrixRecursive(void*, int, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void Vec3_CopyToOutput(void*, void*);
void WView_ProjectViewPoint(void*, void*, void*, int);
void WView_ProjectWorldPoint(void*, void*, void*);
void WView_ProjectViewPointWithoutClipTest(void*, void*, void*);

// WVector operators, called directly where the code works on raw storage.
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void* __ml__FfRC7WVector(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
float Vector_SignedAngleXZ(void*, void*);
unsigned long long Vec2_ScaleByFloat(void*, float);
unsigned long long Vec2_Add(void*, void*);

// 2D overlays and debug text.
void Sprite_DrawRotatedQuadWithViewportRect(void*, void*, void*, float, float, float, void*, int, unsigned int, int);
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, unsigned int, int);
void DebugText_SetColor(void*, unsigned int);
void DebugText_SetWorldPosition(void*, void*);
void DebugText_DrawFormattedAtWorldPosition(void*, void*, int, void*, ...);

extern void* g_view;
extern CSharedDoc* g_sharedDoc;
extern unsigned int spotRenderStateDefaults[];
extern unsigned int s_markerTextureStageStates[];
extern unsigned int s_spotTextureStageParameters[];
extern unsigned int s_spotHeightGradient[];
extern char s_sceneLighting[];
extern char kIdentityMatrix4x4[];
extern const char spotIndicatorStrings[];

// Float constants, still linked from the original data.
extern float kFloat_0_05_Spot, kFloat_0_5_Marker, kFloat_1_spot, kFloat_0_spot, kFloat_0_8125_Spot;
extern float kFloat_0_6875_Spot, kFloat_8_Spot, kSpotEndpointSize, kFloat_26_SpotWidth;
extern float kFloat_11_Spot, kSpotWorldUnitsPerYard, kFloat_0_9144_Spot, kFloat_neg4_Spot;
extern float kFloat_4_Spot, kSpotComparisonEpsilon, kFloat_255_SpotColor, kFloat_neg1_U800EFEAC;
extern float kFloat_1_5707964_Marker, kFloat_2_Marker, s_spotPositionBlendTarget, s_markerCompletedPositionBlend;
extern const float kFloat_0_4_Spot;

} // extern "C"

#define kZero kFloat_0_spot
#define kOne kFloat_1_spot
#define kHalf kFloat_0_5_Marker
#define kHalfPi kFloat_1_5707964_Marker
#define kNegativeOne kFloat_neg1_U800EFEAC
#define kPulseResetPhase kFloat_0_4_Spot

// Records this file reads from classes whose headers do not declare them.
//
// The tutorial marker's model position, which lies between the model pointer and
// the visibility flag in the golf document.
struct TutorialMarkerPosition {
    char pad_000[0xAC];
    float position[3]; // 0xAC
};
// One player record in the golf document: its status, world position, the holes
// it is active on and its packed marker color.
struct GolfPlayerEntry {
    char pad_00[8];
    int status; // 0x08
    char pad_0C[0x10];
    float position[3];             // 0x1C
    unsigned char activeHoles[18]; // 0x28
    char pad_3A[0x5A];
    unsigned int color; // 0x94
};
// The club range overlay's color and world position.
struct ClubRangeOverlay {
    char pad_00[0xC];
    unsigned int color; // 0x0C
    char pad_10[8];
    WVector position; // 0x18
};
// The render state's view and projection matrices.
struct MarkerRenderState {
    char pad_000[0x20];
    float view[16];       // 0x20
    float projection[16]; // 0x60
};
// The current marker instance table: a mode flag the pulse draw tests.
struct MarkerInstanceMode {
    char pad_00[0xA];
    unsigned char mode; // 0x0A
};
struct MarkerInstanceTable {
    unsigned int prefix;         // 0x00
    MarkerInstanceMode* current; // 0x04
};

// The distance format in the marker string pool: "%.0fy".
enum SpotStringOffset {
    kDistanceFormat = 0xED
};

// The nine marker textures are driven through their own vtables.
struct MarkerObject {
    virtual void slot0();
    virtual void slot1();
    virtual void show();
    virtual void draw();
    virtual int ready();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual int blocked();
};
// The tutorial marker model's pane interface.
struct MarkerPane {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void setup(void*, int, int);
    virtual void render(void*, int, int, int, int, float);
};
// The texture object's vtable as used by the pulse draw.
struct MarkerTextureVtable {
    void* earlierSlots[28];
    void* (*getStageResource)(void*, void*);
    void* (*getPaletteObject)(void*, void*);
};

// A strip vertex: position, packed color and texture coordinates.
struct MarkerVertex {
    float x, y, z;
    unsigned int color;
    float u, v;
};
struct MarkerQuad {
    MarkerVertex v[4];
};
// A projected view point: x, y, z, then the remaining record and its clip flags.
struct MarkerProjection {
    float x, y, z;
    char rest[0x28];
    unsigned char flags;
    char pad[3];
};
// A 3x4 transform.
struct MarkerMatrix {
    float data[12];
};
// The height-color ramp and the two default quads in the same data block.
struct SpotRenderTemplates {
    unsigned int heightColors[5];
    char pad_014[0xA8 - 5 * sizeof(unsigned int)];
    MarkerQuad screenPlayer; // 0xA8
    MarkerQuad worldPlayer;  // 0x108
};

// Screen-space vectors: three floats, as WVector.
typedef Vec3f MarkerVec;

enum SpotDrawConstants {
    kSharedPlayerSprite = 8,
    kFlaggedPlayerSpriteOffset = 4,
    kTriangleStrip = 0x98,
    kQuadVertexCount = 4,
    kMarkerVertexBytes = sizeof(MarkerVertex),
    kScreenMarkerFormat = 0x11F005,
    kWorldMarkerFormat = 0x11F001,
    kTransformView = 0,
    kTransformProjection = 1,
    kTransformWorld = 2,
    kTransformTexture0 = 11,
    kProjectedClipMask = 3,
    kAimViewFlag = 0x200,
    kOtherPlayerViewFlags = 0x8CC2,
    kNoGroundHit = -1,
    kColorWeightTotal = 255
};
static const unsigned int kInvalidTargetColor = 0xFFFF0000;
static const unsigned int kPlayerMarkerAlpha = 0x80000000;
static const unsigned int kArrowShadowColor = 0xA0000000;
enum ColorChannelMask {
    kRedBlueChannels = 0x00FF00FF,
    kAlphaGreenChannels = 0xFF00FF00
};

// Game modes 0 and 6 show the shared flagged-player sprite; modes 1 and 7 show
// each player's own. Party player status 1 is visible in party modes 1 and 2.
enum SpotVisibilityCode {
    kFlaggedSharedSpriteMode0 = 0,
    kFlaggedPlayerSpriteMode1 = 1,
    kFlaggedSharedSpriteMode6 = 6,
    kFlaggedPlayerSpriteMode7 = 7,
    kMarkerPartyMode1 = 1,
    kMarkerPartyMode2 = 2,
    kMarkerNormalPlayerStatus = 0,
    kMarkerPartyPlayerStatus1 = 1
};

enum SpotMessageType {
    kAddHeading = 0,
    kGetPosition = 2,
    kEnableMarker = 3,
    kDisableMarker = 4,
    kDirtyTarget = 5,
    kSetAimTransition = 6,
    kSetPositionBlend = 7,
    kSetEnabledFromInteger = 9,
    kGetTargetAngle = 11,
    kSetHeading = 0x2717
};

static inline CGolfDoc* GolfDoc() {
    return (CGolfDoc*)g_sharedDoc->golfDoc;
}

// The active player entry; the caller keeps the index below the player count.
static inline GolfPlayerEntry* PlayerEntry(int index) {
    return (GolfPlayerEntry*)GolfPlayer_GetEntryByIndex(GolfDoc(), index);
}

// Invoke the embedded texture resource's draw slot.
static inline void DrawMarkerResource(void* resource) {
    ((MarkerObject*)resource)->draw();
}

// Absolute value taken by testing the sign bit, as the tolerance comparisons do.
static inline float MarkerAbsoluteValue(float value) {
    return (*(unsigned int*)&value & 0x80000000) ? -value : value;
}

// Whether the value is within the marker tolerance of zero.
static inline int MarkerNear(float value) {
    return MarkerAbsoluteValue(value) < kSpotComparisonEpsilon;
}

// Write one projected strip corner; the packed vertex color comes from the
// template the quad is copied from.
static inline void SetMarkerCorner(MarkerVertex& vertex, float x, float y, float z, float u, float v) {
    vertex.x = x;
    vertex.y = y;
    vertex.z = z;
    vertex.u = u;
    vertex.v = v;
}

#pragma push
#pragma always_inline on
// The push and always_inline keep the readiness gate part of the caller, as the
// original has it, and visit all nine textures even once one is not ready.
static inline unsigned int AreAllSpotSpritesReady(CSpot* spot) {
    unsigned int allSpritesReady = 1;
    int spriteIndex = 0;
    SpotTextureBinding* sprite = spot->textureBindings;
    do {
        allSpritesReady &= ((MarkerObject*)sprite)->ready();
        ++spriteIndex;
        ++sprite;
    } while (spriteIndex < CSpot::kTextureCount);
    return allSpritesReady;
}
#pragma pop

// Draw the tutorial target, the other players' markers and the aim marker with
// its distance label. All nine textures must be ready before anything is drawn.
void CSpot::DrawMarkersAndLabels() {
    float uv[4];
    MarkerQuad screenQuad;
    MarkerQuad worldQuad;
    MarkerMatrix matrix;
    MarkerProjection projected;
    MarkerProjection unusedProjection;
    MarkerMatrix rotation;

    MarkerVec ballMinusMarker;
    MarkerVec blended;
    MarkerVec raised;
    MarkerVec relative;
    float textUV[4];
    float textRect[4];
    MarkerVec scaledSum;
    MarkerVec scaledOther;
    MarkerVec scaledSelf;
    MarkerVec blendedCamera;
    MarkerVec otherCamera;
    MarkerVec selfCamera;
    MarkerVec ballCamera;
    MarkerVec delta;
    MarkerVec endpoint;
    MarkerVec origin;
    float arrowUV[4];
    float arrowRect[4];
    float shadowUV[4];
    float shadowRect[4];
    float invalidUV[4];
    float invalidRect[4];
    float validUV[4];
    float center[2];
    float validRect[4];
    unsigned long long weightedSum;
    unsigned long long weightedB;
    float pointB[2];
    unsigned long long weightedA;
    float pointA[2];
    unsigned int* heightColors = s_spotHeightGradient;

    // A blocked marker or an incompletely loaded texture set suppresses every
    // rendering phase.
    if (((MarkerObject*)this)->blocked())
        return;
    unsigned int allSpritesReady = AreAllSpotSpritesReady(this);
    if (!allSpritesReady)
        return;

    // Phase 1: the tutorial target model, or the score target during the
    // tutorial's score stages.
    if (GolfDoc()->tutorialMarkerVisible) {
        if ((unsigned int)(GolfDoc()->tutorialStage - 2) <= 1) {
            DrawScoreTarget();
        } else {
            Matrix_SetAxisRotation(&rotation, markerHeading, 1);
            Matrix_CopyAffine(&matrix, &rotation);
            Matrix3x3_ScaleAssign(&matrix, kHalf);
            *(WVector*)&matrix.data[9] = *(WVector*)((TutorialMarkerPosition*)GolfDoc())->position;
            Model_SetMatrixRecursive((void*)GolfDoc()->tutorialMarkerModel, 0, &matrix);
            WPuppet_UpdateBoneWorldMatrices((void*)GolfDoc()->tutorialMarkerModel, 0, 0, kOne);
            void* pane = (void*)GolfDoc()->tutorialMarkerModel;
            ((MarkerPane*)pane)->setup(s_sceneLighting, 0, 0);
            pane = (void*)GolfDoc()->tutorialMarkerModel;
            ((MarkerPane*)pane)->render(g_view, 1, 0, 1, 0, kOne);
        }
    }

    // Phase 2: the other players, skipping the current player and inactive holes.
    if (playerMarkersVisible) {
        unsigned int flags = GolfDoc()->gameStateFlags;
        uv[1] = kZero;
        uv[0] = kZero;
        uv[2] = kFloat_0_8125_Spot;
        uv[3] = kFloat_0_6875_Spot;
        // The aiming view uses a screen-space label; the other views also keep
        // the projected depth.
        if (flags & kAimViewFlag) {
            float one;
            float zero;
            float half;
            float size;
            float height;
            float width;
            unsigned int style = g_sharedDoc->gameMode;
            size = kFloat_8_Spot;
            if (!style || kFlaggedSharedSpriteMode6 == style || style == kFlaggedPlayerSpriteMode7 ||
                kFlaggedPlayerSpriteMode1 == style) {
                MarkerQuad* defaults = &((SpotRenderTemplates*)heightColors)->screenPlayer;
                half = kHalf;
                zero = kZero;
                float offset;
                offset = kSpotEndpointSize;
                one = kOne;
                height = kFloat_11_Spot;
                width = kFloat_26_SpotWidth;
                for (unsigned char i = 0; i < GolfDoc()->playerCount; ++i) {
                    float texBottom;
                    if (i == GolfDoc()->currentPlayer)
                        continue;
                    unsigned char active = GolfDoc()->currentHoleIndex;
                    if (!PlayerEntry(i)->activeHoles[active])
                        continue;
                    if (PlayerEntry(i)->status != kMarkerNormalPlayerStatus) {
                        if ((int)g_sharedDoc->partyGameMode != kMarkerPartyMode2 &&
                            (int)g_sharedDoc->partyGameMode != kMarkerPartyMode1)
                            continue;
                        if (PlayerEntry(i)->status != kMarkerPartyPlayerStatus1)
                            continue;
                    }
                    Matrix_TransformPoint(&relative, PlayerEntry(i)->position, (char*)((WView*)g_view)->inputTransform);
                    WView_ProjectViewPoint(g_view, &projected, &relative, 0);
                    GolfPlayerEntry* record = PlayerEntry(i);
                    float textY = projected.y - half * size;
                    textRect[2] = size;
                    textRect[3] = size;
                    textRect[0] = projected.x - half * size;
                    textRect[1] = textY;
                    textUV[0] = zero;
                    textUV[1] = zero;
                    textUV[2] = one;
                    textUV[3] = one;
                    WOverlay_DrawTransformedRect(playerMarkerOverlay, g_view, textUV, zero, textRect, 0,
                                                 record->color | kPlayerMarkerAlpha, 0);
                    Engine_SetFlexibleVertexFormat(kScreenMarkerFormat);
                    if (SharedDoc_IsPlayerFlag0Set(i)) {
                        unsigned int mode = g_sharedDoc->gameMode;
                        if (!mode || kFlaggedSharedSpriteMode6 == mode) {
                            DrawMarkerResource(&textureBindings[kSharedPlayerSprite]);
                        } else {
                            DrawMarkerResource(&textureBindings[kFlaggedPlayerSpriteOffset + i]);
                        }
                    } else {
                        DrawMarkerResource(&textureBindings[i]);
                    }
                    projected.y = projected.y - offset;
                    float bottom;
                    screenQuad = *defaults;
                    float texRight;
                    float right;
                    float texTop;
                    float texLeft;
                    float top;
                    float left;
                    left = projected.x - width;
                    right = width + projected.x;
                    top = projected.y - height;
                    bottom = height + projected.y;
                    texLeft = uv[0];
                    texTop = uv[1];
                    texRight = uv[2];
                    texBottom = uv[3];
                    screenQuad.v[0].x = left;
                    screenQuad.v[0].y = top;
                    screenQuad.v[0].u = texLeft;
                    screenQuad.v[0].v = texTop;
                    screenQuad.v[1].x = right;
                    screenQuad.v[1].y = top;
                    screenQuad.v[1].u = texRight;
                    screenQuad.v[1].v = texTop;
                    screenQuad.v[2].x = left;
                    screenQuad.v[2].y = bottom;
                    screenQuad.v[2].u = texLeft;
                    screenQuad.v[2].v = texBottom;
                    screenQuad.v[3].x = right;
                    screenQuad.v[3].y = bottom;
                    screenQuad.v[3].u = texRight;
                    screenQuad.v[3].v = texBottom;
                    int primitiveCount = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
                    DrawPrimitiveUP(kTriangleStrip, primitiveCount, &screenQuad, kMarkerVertexBytes);
                }
            }
        } else if ((flags & kOtherPlayerViewFlags) || GolfDoc()->cameraMode) {
            unsigned int style = g_sharedDoc->gameMode;
            if (kFlaggedSharedSpriteMode0 == style || kFlaggedSharedSpriteMode6 == style ||
                style == kFlaggedPlayerSpriteMode7 || kFlaggedPlayerSpriteMode1 == style) {
                MarkerQuad* defaults = &((SpotRenderTemplates*)heightColors)->worldPlayer;
                float height = kFloat_11_Spot;
                float width = kFloat_26_SpotWidth;
                for (unsigned char i = 0; i < GolfDoc()->playerCount; ++i) {
                    if (i == GolfDoc()->currentPlayer)
                        continue;
                    unsigned char active = GolfDoc()->currentHoleIndex;
                    if (!PlayerEntry(i)->activeHoles[active])
                        continue;
                    if (PlayerEntry(i)->status != kMarkerNormalPlayerStatus) {
                        if ((int)g_sharedDoc->partyGameMode != kMarkerPartyMode2 &&
                            (int)g_sharedDoc->partyGameMode != kMarkerPartyMode1)
                            continue;
                        if (PlayerEntry(i)->status != kMarkerPartyPlayerStatus1)
                            continue;
                    }
                    DrawPlayerMarker(i);
                    GolfPlayerEntry* record = PlayerEntry(i);
                    GolfPlayerEntry* positionRecord = PlayerEntry(i);
                    CSpot::DrawProjectedClippedQuad(this, positionRecord->position, record->color | kPlayerMarkerAlpha);
                    Engine_SetFlexibleVertexFormat(kScreenMarkerFormat);
                    if (SharedDoc_IsPlayerFlag0Set(i)) {
                        unsigned int mode = g_sharedDoc->gameMode;
                        if (!mode || mode == kFlaggedSharedSpriteMode6) {
                            DrawMarkerResource(&textureBindings[kSharedPlayerSprite]);
                        } else {
                            DrawMarkerResource(&textureBindings[kFlaggedPlayerSpriteOffset + i]);
                        }
                    } else {
                        DrawMarkerResource(&textureBindings[i]);
                    }
                    WView_ProjectWorldPoint(g_view, &projected, PlayerEntry(i)->position);
                    if ((projected.flags & kProjectedClipMask) != 0)
                        continue;
                    float texBottom;
                    float bottom;
                    float texRight;
                    float right;
                    float texTop;
                    float texLeft;
                    float z;
                    float top;
                    float left;
                    projected.y -= height + labelHeightOffset;
                    worldQuad = *defaults;

                    left = projected.x - width;
                    right = width + projected.x;
                    top = projected.y - height;
                    bottom = height + projected.y;
                    texLeft = uv[0];
                    z = projected.z;
                    texTop = uv[1];
                    texRight = uv[2];
                    texBottom = uv[3];
                    SetMarkerCorner(worldQuad.v[0], left, top, z, texLeft, texTop);
                    SetMarkerCorner(worldQuad.v[1], right, top, z, texRight, texTop);
                    SetMarkerCorner(worldQuad.v[2], left, bottom, z, texLeft, texBottom);
                    SetMarkerCorner(worldQuad.v[3], right, bottom, z, texRight, texBottom);
                    unsigned int primitiveCount = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
                    DrawPrimitiveUP(kTriangleStrip, primitiveCount, &worldQuad, kMarkerVertexBytes);
                }
            }
        }
    }
    // Phase 3: choose the marker color from the ball-minus-target height.
    if (enabled == 0)
        return;
    if (GolfDoc()->tutorialStage >= 1) {
        __mi__FRC7WVectorRC7WVector(&ballMinusMarker, GolfBall__Fv(), position);
        if (groundHitIndex == kNoGroundHit) {
            markerColor = kInvalidTargetColor;
        } else {
            float clamped;
            float value = kFloat_0_9144_Spot * (ballMinusMarker.y / kSpotWorldUnitsPerYard);

            if (value < kFloat_neg4_Spot)
                clamped = kFloat_neg4_Spot;
            else if (value > kFloat_4_Spot)
                clamped = kFloat_4_Spot;
            else
                clamped = value;
            // Map the height to the ramp coordinate [0,4] and blend two adjacent
            // packed colors in 8-bit lanes.
            float sum = kFloat_4_Spot + clamped;
            float t = sum * kHalf;

            int index = (int)t;
            float fraction = t - (float)(int)t;
            if (MarkerNear(fraction)) {
                markerColor = heightColors[index];
            } else {
                unsigned int next = heightColors[(1 + index)];

                unsigned char weight = (unsigned char)(int)((kFloat_255_SpotColor * fraction));
                unsigned char nextWeight = weight;
                int nextRedBlue = next & kRedBlueChannels;
                unsigned int prev = heightColors[index];
                unsigned int previousRedBlue = prev & kRedBlueChannels;
                int previousWeight = kColorWeightTotal - nextWeight;

                unsigned int weightedNextChannels = nextRedBlue * weight;
                markerColor =
                    (((previousRedBlue * previousWeight + weightedNextChannels) & kAlphaGreenChannels) >> 8) |
                    ((((prev >> 8) & kRedBlueChannels) * previousWeight + ((next >> 8) & kRedBlueChannels) * weight) &
                     kAlphaGreenChannels);
            }
        }

        // Phase 4: the aiming view draws the screen-space arrow and endpoint
        // overlays; the other views draw the raised world marker and range text.
        if (GolfDoc()->gameStateFlags & kAimViewFlag) {
            // Interpolate the world positions before projecting. During the
            // transition the marker X is pinned to the viewport center.
            if (!MarkerNear(positionBlend - s_spotPositionBlendTarget)) {
                __ml__FRC7WVectorf(&scaledSelf, position, positionBlend);
                __ml__FRC7WVectorf(&scaledOther, previousPosition, kOne - positionBlend);
                __pl__FRC7WVectorRC7WVector(&scaledSum, &scaledOther, &scaledSelf);
                *(WVector*)&blended = *(WVector*)&scaledSum;
                Matrix_TransformPoint(&blendedCamera, &blended, (char*)((WView*)g_view)->inputTransform);
                WView_ProjectViewPointWithoutClipTest(g_view, markerScreen, &blendedCamera);
                markerScreen[0] = kHalf * ((WView*)g_view)->viewportWidth;
                Matrix_TransformPoint(&otherCamera, transitionReferencePosition,
                                      (char*)((WView*)g_view)->inputTransform);
                WView_ProjectViewPointWithoutClipTest(g_view, &unusedProjection, &otherCamera);
            } else {
                Matrix_TransformPoint(&selfCamera, position, (char*)((WView*)g_view)->inputTransform);
                WView_ProjectViewPointWithoutClipTest(g_view, markerScreen, &selfCamera);
            }
            Matrix_TransformPoint(&ballCamera, GolfBall__Fv(), (char*)((WView*)g_view)->inputTransform);
            WView_ProjectViewPointWithoutClipTest(g_view, ballScreen, &ballCamera);
            if (markerScreen[1] > ballScreen[1])
                markerScreen[1] *= kNegativeOne;
            float ballY = ballScreen[1];
            pointA[0] = ballScreen[0];
            pointA[1] = ballY;
            // In pixel space: center = (1-progress)*ball + progress*marker.
            // The colored arrow covers progress*distance, the shadow the rest.
            weightedA = Vec2_ScaleByFloat(pointA, kOne - arrowProgress);
            float markerY = markerScreen[1];
            pointB[0] = markerScreen[0];
            pointB[1] = markerY;
            weightedB = Vec2_ScaleByFloat(pointB, arrowProgress);
            weightedSum = Vec2_Add(&weightedB, &weightedA);
            center[0] = ((float*)&weightedSum)[0];
            center[1] = ((float*)&weightedSum)[1];
            float dx = markerScreen[0] - ballScreen[0];
            float dy = markerScreen[1] - ballScreen[1];
            float angle = (float)atan2(dy, dx);
            float rotationAngle = kHalfPi + angle;

            float originX = ballScreen[0];
            origin.x = originX;
            origin.y = ballScreen[1];
            origin.z = kZero;
            float endpointY = markerScreen[1];
            endpoint.x = markerScreen[0];
            endpoint.y = endpointY;
            endpoint.z = kZero;
            __mi__FRC7WVectorRC7WVector(&delta, &endpoint, &origin);

            float markerArrowWidth;
            float arrowLength;
            float length = WVectorLen(*(WVector*)&delta);
            arrowLength = arrowProgress * length;
            markerArrowWidth = arrowWidth;

            arrowRect[1] = center[1];
            arrowRect[2] = markerArrowWidth;
            arrowRect[0] = (center[0] - kHalf * markerArrowWidth);
            arrowRect[3] = arrowLength;
            arrowUV[0] = kZero;
            arrowUV[1] = kZero;
            arrowUV[2] = kOne;
            arrowUV[3] = kOne;

            Sprite_DrawRotatedQuadWithViewportRect(arrowOverlay, g_view, arrowUV, center[0], center[1], rotationAngle,
                                                   arrowRect, 0, markerColor, 0);
            float remaining = (kOne - arrowProgress) * length;
            float shadowWidth = arrowWidth;
            float shadowX = markerScreen[0] - kHalf * shadowWidth;
            shadowRect[1] = markerScreen[1];
            shadowRect[2] = shadowWidth;
            shadowRect[0] = shadowX;
            shadowRect[3] = remaining;
            shadowUV[0] = kZero;
            shadowUV[1] = kZero;
            shadowUV[2] = kOne;

            shadowUV[3] = kOne;
            Sprite_DrawRotatedQuadWithViewportRect(arrowOverlay, g_view, shadowUV, markerScreen[0], markerScreen[1],
                                                   rotationAngle, shadowRect, 0, kArrowShadowColor, 0);
            // An invalid target gets only the invalid overlay; a valid target
            // also gets the world marker and the pulse.
            if (groundHitIndex == kNoGroundHit) {
                float y;
                float size;
                size = endpointSize;
                y = markerScreen[1] - kHalf * size;
                float x = markerScreen[0] - kHalf * size;
                invalidRect[2] = size;
                invalidRect[0] = x;
                invalidRect[1] = y;
                invalidRect[3] = size;
                invalidUV[0] = kZero;
                invalidUV[1] = kZero;
                invalidUV[2] = kOne;
                invalidUV[3] = kOne;
                WOverlay_DrawTransformedRect(invalidTargetOverlay, g_view, invalidUV, kZero, invalidRect, 0,
                                             markerColor, 0);
            } else {
                float y;
                float size;
                size = endpointSize;
                y = markerScreen[1] - kHalf * size;
                float x = markerScreen[0] - kHalf * size;
                validRect[2] = size;
                validRect[0] = x;
                validRect[1] = y;
                validRect[3] = size;
                validUV[0] = kZero;
                validUV[1] = kZero;
                validUV[2] = kOne;
                validUV[3] = kOne;
                WOverlay_DrawTransformedRect(validTargetOverlay, g_view, validUV, kZero, validRect, 0, markerColor, 0);
                DrawTexturedMarker();
                DrawFallingMarker();
            }

            if (MarkerNear(positionBlend - s_markerCompletedPositionBlend)) {
                ((ClubRangeOverlay*)clubRangeInfo)->color = markerColor;
                ((ClubRangeOverlay*)clubRangeInfo)->position = *(WVector*)position;
                void* object = clubRangeInfo;
                ((MarkerObject*)object)->show();
            }
        } else if (groundHitIndex != kNoGroundHit) {
            // The world label: raise the marker by two world units, then label the
            // horizontal range in yards.
            DrawTexturedMarker();
            DrawFallingMarker();
            unsigned int raisedX = ((unsigned int*)position)[0];
            ((unsigned int*)&raised)[1] = ((unsigned int*)position)[1];
            ((unsigned int*)&raised)[0] = raisedX;
            ((unsigned int*)&raised)[2] = ((unsigned int*)position)[2];
            raised.y = raised.y + kFloat_2_Marker;
            CSpot::DrawProjectedClippedQuad(this, &raised, markerColor);
            DebugText_SetColor(distanceText, markerColor);
            DebugText_SetWorldPosition(distanceText, &raised);
            // The label ignores the vertical separation and converts to yards.
            ballMinusMarker.y = kZero;
            float distance = WVectorLen(*(WVector*)&ballMinusMarker);
            DebugText_DrawFormattedAtWorldPosition(distanceText, g_view, -5,
                                                   (void*)(spotIndicatorStrings + kDistanceFormat),
                                                   distance / kSpotWorldUnitsPerYard);
        }
    }
}

// Draw the fading, rotating ground quad around the landing point. The alpha is
// 255*(1-pulsePhase) and the radius grows with the phase; rotationPhase is the
// quad's angle about world Y. Sets shared render state and submits one strip.
//
// OnProcess advances the phase by half the animation step and resets it past
// one; the aim-transition message sets 0.4. The phase is wrapped before the 0.7
// angular gain, so the angle jumps back to zero every two active seconds.
void CSpot::DrawFallingMarker() {
    unsigned char* state = (unsigned char*)InstanceTable_Get();
    if (((MarkerInstanceTable*)state)->current->mode == 3)
        return;

    // The render-state descriptor, texture stage states and parameters are
    // copies of the shared default lists with one entry changed.
    unsigned int descriptor[26];
    unsigned int textureStageStates[12];
    unsigned int textureStageParameters[10];
    for (int i = 0; i < 26; ++i)
        descriptor[i] = spotRenderStateDefaults[i];
    descriptor[17] = 1;
    for (int i = 0; i < 12; ++i)
        textureStageStates[i] = s_markerTextureStageStates[i];
    for (int i = 0; i < 10; ++i)
        textureStageParameters[i] = s_spotTextureStageParameters[i];
    Engine_SetRenderStateList(descriptor, 13);
    Engine_SetTextureStageStateList(0, textureStageStates, 6);
    Engine_SetTextureStageParameterList(0, textureStageParameters, 5);

    void* device = Camera_GetRenderState(g_view);
    SetTransform(kTransformProjection, ((MarkerRenderState*)device)->projection);
    SetTransform(kTransformView, ((MarkerRenderState*)device)->view);
    Engine_SetType7Resource(0);
    SetTransform(kTransformWorld, (void*)((char*)&kIdentityMatrix4x4));
    SetTransform(kTransformTexture0, (void*)((char*)&kIdentityMatrix4x4));

    void* manager = ((WView*)g_view)->renderTarget;
    void* object = fallingBitmap;
    void* table = *(void***)manager;
    Engine_SetTextureStageResource(0, ((MarkerTextureVtable*)table)->getStageResource(manager, object));
    Engine_SetTexturePalette(0, ((MarkerTextureVtable*)table)->getPaletteObject(manager, object));
    Engine_SetFlexibleVertexFormat(kWorldMarkerFormat);

    // The pulse expands from the landing position; the lift keeps it above the
    // ground.
    float fade = kOne - pulsePhase;
    unsigned char alpha = (unsigned char)(kFloat_255_SpotColor * fade);

    float scale = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
    float radius = kSpotWorldUnitsPerYard * scale * kFloat_0_05_Spot;
    radius = radius * pulsePhase;
    float verticalLift = kFloat_2_Marker + kFloat_0_05_Spot * radius;
    struct Vertex {
        float xyz[3];
        unsigned char color[4];
        float uv[2];
    } verts[4];
    Vertex* out = verts;
    for (int i = 0; i < 4; ++i) {
        float xSign = (i & 2) ? kOne : kNegativeOne;
        float zSign = (i & 1) ? kOne : kNegativeOne;
        float rotation[16];
        // Build an XZ corner, rotate it about world Y and move it to the landing
        // position. The texture coordinates follow the strip vertex order.
        float point[3] = {xSign * radius, verticalLift, zSign * radius};
        float rotated[3];
        Matrix_SetAxisRotation(&rotation, 0.7f * rotationPhase, 1);
        Matrix_TransformPoint(rotated, point, rotation);
        *(WVector*)&point = *(WVector*)&rotated;
        *(WVector*)point += *(WVector*)position;
        Vec3_CopyToOutput(point, out->xyz);
        out->color[0] = out->color[1] = out->color[2] = 255;
        out->color[3] = alpha;
        out->uv[0] = (i & 1) ? kOne : kZero;
        out->uv[1] = (i & 2) ? kZero : kOne;
        ++out;
    }
    DrawPrimitiveUP(kTriangleStrip, 2, verts, kMarkerVertexBytes);
}

// Apply a marker message, or answer a query through the caller's storage.
void CSpot::HandleMsg(const SpotMessage* message) {
    unsigned int type = message->type;
    switch (type) {
    case kAddHeading: {
        float* value = message->value.scalar;
        heading += *value;
        targetDirty = 1;
        break;
    }
    case kSetHeading: {
        float* value = message->value.scalar;
        heading = *value;
        targetDirty = 1;
        break;
    }
    case kGetTargetAngle: {
        float ballToMarker[3];
        float ballToTarget[3];
        __mi__FRC7WVectorRC7WVector(ballToTarget, targetPosition, GolfBall__Fv());
        __mi__FRC7WVectorRC7WVector(ballToMarker, position, GolfBall__Fv());
        *message->value.scalar = Vector_SignedAngleXZ(ballToMarker, ballToTarget);
        break;
    }
    case kGetPosition:
        __as__7WVectorFRC7WVector(message->value.pointer, position);
        break;
    case kSetEnabledFromInteger:
        enabled = (unsigned int)(message->value.integer - 1) == 0;
        break;
    case kEnableMarker:
        enabled = 1;
        break;
    case kDisableMarker:
        enabled = 0;
        break;
    case kDirtyTarget:
        targetDirty = 1;
        break;
    case kSetAimTransition: {
        float* value = message->value.scalar;
        float tmp[9];
        float val = *value;
        __as__7WVectorFRC7WVector(transitionReferencePosition, message->secondPointer);
        __as__7WVectorFRC7WVector(previousPosition, position);

        Matrix_SetAxisRotation(tmp, val, 1);
        float scale = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
        scale = kSpotWorldUnitsPerYard * scale;
        float result[3];

        float scaled[3];
        __ml__FfRC7WVector(scaled, &tmp[6], scale);
        __pl__FRC7WVectorRC7WVector(result, GolfBall__Fv(), scaled);
        __as__7WVectorFRC7WVector(targetPosition, result);

        float zero = kZero;
        pulsePhase = kPulseResetPhase;
        positionBlend = zero;
        break;
    }
    case 1:
    case 8:
    case 10:
    case 0x2719:
        break;
    case kSetPositionBlend: {
        float* value = message->value.scalar;
        positionBlend = *value;
        break;
    }
    default:
        break;
    }
}
