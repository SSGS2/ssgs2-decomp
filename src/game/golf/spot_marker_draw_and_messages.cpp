// Golf aiming/landing marker rendering and synchronous marker messages.
// Mod lookup:
// - Other-player visibility: SpotVisibilityCode and CSpot_DrawMarkersAndLabels phase 2;
//   SharedDoc gameMode, partyGameMode, active-hole/status and HIDE all gate it.
// - Pulse size/fade/rotation: CSpot_DrawFallingMarker; CSpot_OnProcess in
//   spot_marker_actor.cpp produces rotationPhase/pulsePhase from deltaTime seconds.
// - Yard labels: kSpotWorldUnitsPerYard (3.2 world units/yard); geometry is Y-up
//   world space, projected quads/text use WView pixel coordinates.
// - Heading/position edits: CSpot_HandleMsg and SpotMessageType below; payload
//   pointers borrow caller storage synchronously; output messages write there.
// Data companions: u800F0998_data.cpp (draw defaults/texture-stage lists) and
// u800EFEAC_data.cpp (marker scalars/string pool/height gradient/templates).
// CSpot.h owns the ABI field map. Sprite resources are owned by CSpot's loading
// unit; draw borrows them and requires all nine ready. Draw changes engine state
// without restoring it; callers serialize submission and resource destruction.
#include <decomp/record_sizes.h>
extern "C" {
extern unsigned char s_spotPlayerMarkerBitmapNames[];
}
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "game/types.h"
#include "game/wvector.h"
#include "classes/CSpot.h"
#include "iff/IffStruct.h"
extern "C" char kIdentityMatrix4x4[];

#define AsCSpot(p) ((CSpot*)(p))
// Translation unit 0x800F0998-0x800F1D9C (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// Unused six-float legacy overlay; the three-component working vectors use MarkerVec.
struct SpotSixFloatRecord { float x, y, z, w, u, v; };
void __as__7WVectorFRC7WVector(void*, void*);

void* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, int);
void* Camera_GetRenderState(void*);
extern void* g_view;
extern unsigned int spotRenderStateDefaults[];
extern unsigned int s_markerTextureStageStates[];
extern unsigned int s_spotTextureStageParameters[];
extern float kFloat_0_05_Spot;
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(int, void*, int);
void Engine_SetTextureStageParameterList(int, void*, int);
void SetTransform(int, void*);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, void*);
void Engine_SetTexturePalette(int, void*);
void Engine_SetFlexibleVertexFormat(int);
void* Matrix_SetAxisRotation(void*, float, int);
void Matrix_TransformPoint(void*, void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void Vec3_CopyToOutput(void*, void*);
void DrawPrimitiveUP(int, int, void*, int);

void* fn_800F0998_game_state;
void CSpot_DrawScoreTarget(void*);
void CSpot_DrawFallingMarker(void*);
void Matrix_CopyAffine(void*, void*);
void Matrix3x3_ScaleAssign(void*, float);
void Model_SetMatrixRecursive(void*, int, void*);
void WPuppet_UpdateBoneWorldMatrices(void*, int, int, float);
void* GolfPlayer_GetEntryByIndex(void*, int);
void WView_ProjectViewPoint(void*, void*, void*, int);
void WView_ProjectWorldPoint(void*, void*, void*);
void CSpot_DrawPlayerMarker(void*, int);
void CSpot_DrawProjectedClippedQuad(void*, void*, unsigned int);
void CSpot_DrawTexturedMarker(void*);
int GX_GetPrimitiveCount(int, int);
void* GolfBall__Fv();
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FRC7WVectorf(void*, void*, float);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void Matrix_TransformPoint(void*, void*, void*);
void WView_ProjectViewPointWithoutClipTest(void*, void*, void*);
float WVectorLen__FRC7WVector(void*);
unsigned long long Vec2_ScaleByFloat(void*, float);
unsigned long long Vec2_Add(void*, void*);
double atan2(float, float);
void Sprite_DrawRotatedQuadWithViewportRect(void*, void*, void*, float, float, float, void*, int, unsigned int, int);
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, int, unsigned int, int);
void DebugText_SetColor(void*, unsigned int);
void DebugText_SetWorldPosition(void*, void*);
void DebugText_DrawFormattedAtWorldPosition(void*, void*, int, void*, ...);
int SharedDoc_IsPlayerFlag0Set(int);
extern void* g_sharedDoc;
extern unsigned int s_spotHeightGradient[];
extern char s_sceneLighting[];
extern const char spotIndicatorStrings[];
extern float kFloat_0_5_Marker, kFloat_1_spot, kFloat_0_spot, kFloat_0_8125_Spot;
extern float kFloat_0_6875_Spot, kFloat_8_Spot, kSpotEndpointSize, kFloat_26_SpotWidth;
extern float kFloat_11_Spot, kSpotWorldUnitsPerYard, kFloat_0_9144_Spot, kFloat_neg4_Spot;
extern float kFloat_4_Spot, kSpotComparisonEpsilon, kFloat_255_SpotColor, kFloat_neg1_U800EFEAC;
extern float kFloat_1_5707964_Marker, kFloat_2_Marker, s_spotPositionBlendTarget, s_markerCompletedPositionBlend;

// All projection records and temporary vectors below are stack objects.
typedef Vec3f MarkerVec;
struct MarkerVertex { float x, y, z; unsigned int color; float u, v; };
struct MarkerQuad { MarkerVertex v[4]; };
struct MarkerProjection { float x, y, z; char rest[0x28]; unsigned char flags; char pad[3]; };
struct MarkerMatrix { float data[12]; };
struct SpotPlayerEntry {
    char prefix[8];
    int status;
    char beforePosition[0x1C - 0xC];
    float position[3];
    unsigned char activeHoles[18];
    char beforeColor[0x94 - 0x3A];
    unsigned int color;
};
struct SpotRenderTemplates {
    // Only the five colors belong to the named symbol; quads are adjacent rodata views.
    unsigned int heightColors[5];
    char beforeQuads[0xA8 - 5 * sizeof(unsigned int)];
    MarkerQuad screenPlayer;
    MarkerQuad worldPlayer;
};
struct TutorialMarkerPosition { char prefix[0xAC]; float position[3]; };
struct SpotRangeOverlay { char prefix[0xC]; unsigned int color; char beforePosition[8]; WVector position; };
struct MarkerViewTransforms { char prefix[0x20]; float view[16]; float projection[16]; };
struct MarkerTextureVtable {
    void* earlierSlots[28];
    void* (*getStageResource)(void*, void*);
    void* (*getPaletteObject)(void*, void*); // +0x74: borrowed palette; this vtable view is unit-local.
};
struct MarkerInstanceMode { char prefix[0xA]; unsigned char mode; };
struct MarkerInstanceTable { unsigned int prefix; MarkerInstanceMode* current; };
// Message ABI: `value` is one 32-bit word, not a tagged runtime variant.
// Heading messages 0/0x2717 and blend 7 read float*; query 2 writes three floats to
// value.pointer; query 11 writes float to value.scalar. Message 6 reads a
// heading float* plus secondPointer to a world position and resets blending.
// Message 9 enables only when integer == 1. Payload memory is caller-owned;
// copied vectors are not retained, and query destinations must be writable.
struct SpotMessage {
    unsigned int header;
    int type;
    union { int integer; void* pointer; float* scalar; } value;
    void* secondPointer;
};
struct SpotSprite { char storage[0x28]; };
struct SpotSprites { char prefix[0xD0]; SpotSprite players[9]; };
struct SpotStringPool { char beforeDistance[0xED]; char distanceYards[6]; };
enum SpotDrawConstants {
    kMarkerSpriteCount = 9, kSharedPlayerSprite = 8, kFlaggedPlayerSpriteOffset = 4,
    kTriangleStrip = 0x98, kQuadVertexCount = 4, kMarkerVertexBytes = sizeof(MarkerVertex),
    kScreenMarkerFormat = 0x11F005, kWorldMarkerFormat = 0x11F001,
    kTransformView = 0, kTransformProjection = 1, kTransformWorld = 2, kTransformTexture0 = 11,
    kProjectedClipMask = 3, kAimViewFlag = 0x200, kOtherPlayerViewFlags = 0x8CC2,
    kNoGroundHit = -1, kColorWeightTotal = 255
};
static const unsigned int kInvalidTargetColor = 0xFFFF0000;
static const unsigned int kPlayerMarkerAlpha = 0x80000000;
static const unsigned int kArrowShadowColor = 0xA0000000;
enum ColorChannelMask { kRedBlueChannels = 0x00FF00FF, kAlphaGreenChannels = 0xFF00FF00 };
// SharedDoc gameMode is the outer menu/task mode, separate from golfRuleType and
// partyGameMode: 0/1 are single/multiplayer round paths; 6 is Story (course-load
// dispatch in golf_background_load.cpp); 7 is Party (CPartySelect menu in party_main_task_and_phf_helpers.cpp writes it).
// CSingleMainRev_HandleMsg (training_and_single_main_tasks.cpp) writes 0; CFightMain_HandleMsg
// (pad_swing_ctrl_and_com_think.cpp) writes 1 for versus setup. Do not call 0/1 stroke/match, which is
// a separate rule supported by the versus setup menus as well.
// Modes 0/6 use the shared flagged-player sprite; 1/7 use each player's flagged sprite.
// Only these four gameMode values enter either other-player drawing path.
// Nonzero player status is visible only for status 1 in party modes 1 or 2.
enum SpotVisibilityCode {
    kFlaggedSharedSpriteMode0 = 0, kFlaggedPlayerSpriteMode1 = 1, kFlaggedSharedSpriteMode6 = 6, kFlaggedPlayerSpriteMode7 = 7,
    kMarkerPartyMode1 = 1, kMarkerPartyMode2 = 2,
    kMarkerNormalPlayerStatus = 0, kMarkerPartyPlayerStatus1 = 1
};

// Byte offsets preserve the existing alias behavior; screen arrays begin
// full projection records, not standalone float[2] destinations. XY are pixels.
enum SpotFieldOffset {
    kSpotPackedMarkerColorOffset = IFF_OFFSET(CSpot, markerColor),
    kSpotMarkerProjectionOffset = IFF_OFFSET(CSpot, markerScreen),
    kSpotBallProjectionOffset = IFF_OFFSET(CSpot, ballScreen),
    kSpotWorldPositionOffset = IFF_OFFSET(CSpot, position),
    kSpotPreviousWorldPositionOffset = IFF_OFFSET(CSpot, previousPosition),
    kSpotTransitionReferenceOffset = IFF_OFFSET(CSpot, transitionReferencePosition),
};

enum SpotMessageType {
    kAddHeading = 0, kGetPosition = 2, kEnableMarker = 3, kDisableMarker = 4,
    kDirtyTarget = 5, kSetAimTransition = 6, kSetPositionBlend = 7,
    kSetEnabledFromInteger = 9, kGetTargetAngle = 11, kSetHeading = 0x2717
};
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
struct MarkerPane {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void setup(void*, int, int);
    virtual void render(void*, int, int, int, int, float);
};

// Borrow the active golf document; drawing requires the shared document to exist.
inline unsigned char* SpotGolfDocument() {
    return (unsigned char*)AsCSharedDoc(g_sharedDoc)->golfDoc;
}
// Borrow one player entry; caller supplies an index below document playerCount.
inline unsigned char* SpotPlayerForIndex(int index) {
    return (unsigned char*)GolfPlayer_GetEntryByIndex(SpotGolfDocument(), index);
}
// Invoke the embedded sprite resource's draw slot.
inline void DrawMarkerResource(void* resource) { ((MarkerObject*)resource)->draw(); }
// Preserve the float sign-bit test used by the original tolerance comparisons.
inline float MarkerAbsoluteValue(float value) {
    return (*(unsigned int*)&value & 0x80000000) ? -value : value;
}
// Compare a signed float against the marker tolerance without changing the sign-bit test.
inline int MarkerNear(float value) {
    return MarkerAbsoluteValue(value) < kSpotComparisonEpsilon;
}



// Write one projected strip corner; preserve the template's packed vertex color.
static inline void SetMarkerCorner(MarkerVertex& vertex, float x, float y, float z, float u, float v) {
    vertex.x = x;
    vertex.y = y;
    vertex.z = z;
    vertex.u = u;
    vertex.v = v;
}

// Coordinate convention: player/ball/landing positions are in world space,
// Y up. inputTransform maps world -> view; ProjectViewPoint consumes that result,
// while ProjectWorldPoint does both steps. Projected XY/label sizes are pixels;
// projected Z is reused for all corners of a screen-aligned player quad.
// All nine sprite resources must be ready before any part of this draw proceeds.
// Draw tutorial targets, other players, and the colored aiming marker with its distance label.
#pragma push
#pragma always_inline on
// Combine sprite readiness with the original bitwise gate; always visit all nine resources.
static inline unsigned int AreAllSpotSpritesReady(unsigned char* markerBytes) {
    unsigned int allSpritesReady = 1;
    int spriteIndex = 0;
    unsigned char* spriteBytes = markerBytes + IFF_OFFSET(SpotSprites, players);
    do {
        allSpritesReady &= ((MarkerObject*)spriteBytes)->ready();
        ++spriteIndex;
        spriteBytes += sizeof(SpotSprite);
    } while (spriteIndex < kMarkerSpriteCount);
    return allSpritesReady;
}
#pragma pop

// Draw the tutorial/player/aim overlays after resource gating, then draw the range label.


// Draw tutorial/player/aim overlays and a range label using the retained height color.
void CSpot_DrawMarkersAndLabels(void* self) {
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
    float arrowUV[4]; float arrowRect[4];
    float shadowUV[4]; float shadowRect[4];
    float invalidUV[4]; float invalidRect[4];
    float validUV[4]; float center[2];
    float validRect[4];
    unsigned long long weightedSum;
    unsigned long long weightedB;
    float pointB[2];
    unsigned long long weightedA;
    float pointA[2];
    unsigned char* markerBytes = (unsigned char*)self;
    unsigned int* heightColors = s_spotHeightGradient;

    // Gate: blocked or incompletely loaded sprites suppress every rendering phase.
    if (((MarkerObject*)self)->blocked()) return;
    unsigned int allSpritesReady = AreAllSpotSpritesReady(markerBytes);
    if (!allSpritesReady) return;

    // Phase 1: draw the tutorial target model or score target.
    if (AsCGolfDoc(SpotGolfDocument())->tutorialMarkerVisible) {
        if ((unsigned int)(AsCGolfDoc(SpotGolfDocument())->tutorialStage - 2) <= 1) {
            CSpot_DrawScoreTarget(self);
        } else {
            Matrix_SetAxisRotation(&rotation, AsCSpot(markerBytes)->markerHeading, 1);
            Matrix_CopyAffine(&matrix, &rotation);
            Matrix3x3_ScaleAssign(&matrix, kFloat_0_5_Marker);
            *(WVector*)&matrix.data[9] = *(WVector*)((TutorialMarkerPosition*)SpotGolfDocument())->position;
            Model_SetMatrixRecursive((void*)AsCGolfDoc(SpotGolfDocument())->tutorialMarkerModel, 0, &matrix);
            WPuppet_UpdateBoneWorldMatrices((void*)AsCGolfDoc(SpotGolfDocument())->tutorialMarkerModel, 0, 0, kFloat_1_spot);
            void* pane = (void*)AsCGolfDoc(SpotGolfDocument())->tutorialMarkerModel;
            ((MarkerPane*)pane)->setup(s_sceneLighting, 0, 0);
            pane = (void*)AsCGolfDoc(SpotGolfDocument())->tutorialMarkerModel;
            ((MarkerPane*)pane)->render(g_view, 1, 0, 1, 0, kFloat_1_spot);
        }
    }

    // Phase 2: draw other active players; exclude the current player and inactive holes.
    if (AsCSpot(markerBytes)->playerMarkersVisible) {
        unsigned int flags = AsCGolfDoc(SpotGolfDocument())->gameStateFlags;
        uv[1] = kFloat_0_spot;

        uv[0] = kFloat_0_spot;
        uv[2] = kFloat_0_8125_Spot;
        uv[3] = kFloat_0_6875_Spot;
        // Aiming view uses a screen-space label; other views also retain projected depth.
        if (flags & kAimViewFlag) {
            float one; float zero; float half; float size; float height; float width;
            unsigned int style = AsCSharedDoc(g_sharedDoc)->gameMode;
            size = kFloat_8_Spot;
            if (!style || kFlaggedSharedSpriteMode6 == style || style == kFlaggedPlayerSpriteMode7 || kFlaggedPlayerSpriteMode1 == style) {
                MarkerQuad* defaults = (MarkerQuad*)((char*)(&((SpotRenderTemplates*)heightColors)->screenPlayer));
                half = kFloat_0_5_Marker;
                zero = kFloat_0_spot;
                float offset; offset = kSpotEndpointSize;
                one = kFloat_1_spot;
                height = kFloat_11_Spot;
                width = kFloat_26_SpotWidth;
                for (unsigned char i = 0; i < AsCGolfDoc(SpotGolfDocument())->playerCount; ++i) {
                    float texBottom;
                    if (i == AsCGolfDoc(SpotGolfDocument())->currentPlayer) continue;
                    unsigned char active = AsCGolfDoc(SpotGolfDocument())->currentHoleIndex;
                    if (!((SpotPlayerEntry*)SpotPlayerForIndex(i))->activeHoles[active]) continue;
                    if (((SpotPlayerEntry*)SpotPlayerForIndex(i))->status != kMarkerNormalPlayerStatus) {
                        if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode != kMarkerPartyMode2 && (int)AsCSharedDoc(g_sharedDoc)->partyGameMode != kMarkerPartyMode1) continue;
                        if (((SpotPlayerEntry*)SpotPlayerForIndex(i))->status != kMarkerPartyPlayerStatus1) continue;
                    }
                    Matrix_TransformPoint(&relative, ((SpotPlayerEntry*)SpotPlayerForIndex(i))->position, (char*)AsWView(g_view)->inputTransform);
                    WView_ProjectViewPoint(g_view, &projected, &relative, 0);
                    unsigned char* record = SpotPlayerForIndex(i);
                    float textY = projected.y - half * size;
                    textRect[2] = size;
                    textRect[3] = size;
                    textRect[0] = projected.x - half * size;
                    textRect[1] = textY;
                    textUV[0] = zero;
                    textUV[1] = zero;
                    textUV[2] = one;
                    textUV[3] = one;
                    WOverlay_DrawTransformedRect(AsCSpot(markerBytes)->playerMarkerOverlay, g_view, textUV, zero,
                    textRect, 0, ((SpotPlayerEntry*)record)->color | kPlayerMarkerAlpha, 0);
                    Engine_SetFlexibleVertexFormat(kScreenMarkerFormat);
                    if (SharedDoc_IsPlayerFlag0Set(i)) {
                        unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
                        if (!mode || kFlaggedSharedSpriteMode6 == mode) {
                            DrawMarkerResource(markerBytes + IFF_OFFSET(SpotSprites, players[kSharedPlayerSprite]));
                        } else {
                            DrawMarkerResource(markerBytes + (kFlaggedPlayerSpriteOffset + i) * sizeof(SpotSprite) + IFF_OFFSET(SpotSprites, players));
                        }
                    } else {
                        DrawMarkerResource(markerBytes + i * sizeof(SpotSprite) + IFF_OFFSET(SpotSprites, players));
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
        } else if ((flags & kOtherPlayerViewFlags) || AsCGolfDoc(SpotGolfDocument())->cameraMode) {
            unsigned int style = AsCSharedDoc(g_sharedDoc)->gameMode;
            if (kFlaggedSharedSpriteMode0 == style || kFlaggedSharedSpriteMode6 == style || style == kFlaggedPlayerSpriteMode7 || kFlaggedPlayerSpriteMode1 == style) {
                MarkerQuad* defaults = (MarkerQuad*)((char*)(&((SpotRenderTemplates*)heightColors)->worldPlayer));
                float height = kFloat_11_Spot;
                float width = kFloat_26_SpotWidth;
                for (unsigned char i = 0; i < AsCGolfDoc(SpotGolfDocument())->playerCount; ++i) {
                    if (i == AsCGolfDoc(SpotGolfDocument())->currentPlayer) continue;
                    unsigned char active = AsCGolfDoc(SpotGolfDocument())->currentHoleIndex;
                    if (!((SpotPlayerEntry*)SpotPlayerForIndex(i))->activeHoles[active]) continue;
                    if (((SpotPlayerEntry*)SpotPlayerForIndex(i))->status != kMarkerNormalPlayerStatus) {
                        if ((int)AsCSharedDoc(g_sharedDoc)->partyGameMode != kMarkerPartyMode2 && (int)AsCSharedDoc(g_sharedDoc)->partyGameMode != kMarkerPartyMode1) continue;
                        if (((SpotPlayerEntry*)SpotPlayerForIndex(i))->status != kMarkerPartyPlayerStatus1) continue;
                    }
                    CSpot_DrawPlayerMarker(self, i);
                    unsigned char* record = SpotPlayerForIndex(i);
                    unsigned char* positionRecord = SpotPlayerForIndex(i);
                    CSpot_DrawProjectedClippedQuad(self, ((SpotPlayerEntry*)positionRecord)->position, ((SpotPlayerEntry*)record)->color | kPlayerMarkerAlpha);
                    Engine_SetFlexibleVertexFormat(kScreenMarkerFormat);
                    if (SharedDoc_IsPlayerFlag0Set(i)) {
                        unsigned int mode = AsCSharedDoc(g_sharedDoc)->gameMode;
                        if (!mode || mode == kFlaggedSharedSpriteMode6) {
                            DrawMarkerResource(markerBytes + IFF_OFFSET(SpotSprites, players[kSharedPlayerSprite]));
                        } else {
                            DrawMarkerResource(markerBytes + (kFlaggedPlayerSpriteOffset + i) * sizeof(SpotSprite) + IFF_OFFSET(SpotSprites, players));
                        }
                    } else {
                        DrawMarkerResource(markerBytes + i * sizeof(SpotSprite) + IFF_OFFSET(SpotSprites, players));
                    }
                    WView_ProjectWorldPoint(g_view, &projected, ((SpotPlayerEntry*)SpotPlayerForIndex(i))->position);
                    if ((projected.flags & kProjectedClipMask) != 0) continue;
                    float texBottom;
                    float bottom;
                    float texRight;
                    float right;
                    float texTop;
                    float texLeft;
                    float z;
                    float top;
                    float left;
                    projected.y -= height + AsCSpot(markerBytes)->labelHeightOffset;
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
    // Phase 3 (color only): no camera/projection writes occur in this block.
    // Use ball-minus-target world height to select/blend the five-entry color ramp.
    // The branch remains here because extracting it changes register/stack allocation.
    if (AsCSpot(markerBytes)->enabled == 0) return;
    if (AsCGolfDoc(SpotGolfDocument())->tutorialStage >= 1) {
    __mi__FRC7WVectorRC7WVector(&ballMinusMarker, GolfBall__Fv(), (markerBytes + kSpotWorldPositionOffset));
    if (AsCSpot(markerBytes)->groundHitIndex == kNoGroundHit) {
        *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset) = kInvalidTargetColor;
        } else { float clamped;  float value = kFloat_0_9144_Spot * (ballMinusMarker.y / kSpotWorldUnitsPerYard);

        if (value < kFloat_neg4_Spot) clamped = kFloat_neg4_Spot;
        else if (value > kFloat_4_Spot) clamped = kFloat_4_Spot;
        else clamped = value;
            // Clamp the converted height difference to [-4,4], map to ramp
            // coordinate [0,4], then blend adjacent packed colors in 8-bit lanes.
            float sum = kFloat_4_Spot + clamped;
            float t = (sum) * kFloat_0_5_Marker;

            int index = (int)t;
            float fraction = t - (float)(int)t;
        if (MarkerNear(fraction)) {
                *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset) = heightColors[index];
            } else {
                unsigned int next = heightColors[(1 + index)];

                unsigned char weight = (unsigned char)(int)((kFloat_255_SpotColor * fraction));
                unsigned char nextWeight = weight;
                int nextRedBlue = next & kRedBlueChannels;
                unsigned int prev = heightColors[index];
                unsigned int previousRedBlue = prev & kRedBlueChannels;
                int previousWeight = kColorWeightTotal - nextWeight;

                unsigned int weightedNextChannels = nextRedBlue * weight;
                *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset) = (((previousRedBlue * previousWeight + weightedNextChannels) & kAlphaGreenChannels) >> 8) | ((((prev >> 8) & kRedBlueChannels) * previousWeight + ((next >> 8) & kRedBlueChannels) * weight) & kAlphaGreenChannels);
        }
    }

    // Phase 4: aim view draws a screen-space arrow and endpoint overlays. The alternate
    // branch below draws the raised world marker and horizontal-distance text.
    if (AsCGolfDoc(SpotGolfDocument())->gameStateFlags & kAimViewFlag) {

        // Interpolate world positions before projection. During transition the
        // marker X is pinned to viewport center; otherwise use its projected X.
        if (!MarkerNear((AsCSpot(markerBytes)->positionBlend - s_spotPositionBlendTarget))) {
            __ml__FRC7WVectorf(&scaledSelf, (markerBytes + kSpotWorldPositionOffset), AsCSpot(markerBytes)->positionBlend);
            __ml__FRC7WVectorf(&scaledOther, (markerBytes + kSpotPreviousWorldPositionOffset), kFloat_1_spot - AsCSpot(markerBytes)->positionBlend);
            __pl__FRC7WVectorRC7WVector(&scaledSum, &scaledOther, &scaledSelf);
            *(WVector*)&blended = *(WVector*)&scaledSum;
            Matrix_TransformPoint(&blendedCamera, &blended, (char*)AsWView(g_view)->inputTransform);
            WView_ProjectViewPointWithoutClipTest(g_view, (markerBytes + kSpotMarkerProjectionOffset), &blendedCamera);
            AsCSpot(markerBytes)->markerScreen[0] = kFloat_0_5_Marker * AsWView(g_view)->viewportWidth;
            Matrix_TransformPoint(&otherCamera, (markerBytes + kSpotTransitionReferenceOffset), (char*)AsWView(g_view)->inputTransform);
            WView_ProjectViewPointWithoutClipTest(g_view, &unusedProjection, &otherCamera);
        } else {
            Matrix_TransformPoint(&selfCamera, (markerBytes + kSpotWorldPositionOffset), (char*)AsWView(g_view)->inputTransform);
            WView_ProjectViewPointWithoutClipTest(g_view, (markerBytes + kSpotMarkerProjectionOffset), &selfCamera);
        }
        Matrix_TransformPoint(&ballCamera, GolfBall__Fv(), (char*)AsWView(g_view)->inputTransform);
        WView_ProjectViewPointWithoutClipTest(g_view, (markerBytes + kSpotBallProjectionOffset), &ballCamera);
        if (AsCSpot(markerBytes)->markerScreen[1] > AsCSpot(markerBytes)->ballScreen[1]) AsCSpot(markerBytes)->markerScreen[1] *= kFloat_neg1_U800EFEAC;
        float ballY = AsCSpot(markerBytes)->ballScreen[1];
        pointA[0] = AsCSpot(markerBytes)->ballScreen[0];
            pointA[1] = ballY;
        // In pixel space: center = (1-progress)*ball + progress*marker.
        // Draw progress*distance as the colored arrow and the remainder as shadow.
        weightedA = Vec2_ScaleByFloat(pointA, kFloat_1_spot - AsCSpot(markerBytes)->arrowProgress);
        float markerY = AsCSpot(markerBytes)->markerScreen[1];
        pointB[0] = AsCSpot(markerBytes)->markerScreen[0];
            pointB[1] = markerY;
        weightedB = Vec2_ScaleByFloat(pointB, AsCSpot(markerBytes)->arrowProgress);
        weightedSum = Vec2_Add(&weightedB, &weightedA);
        center[0] = ((float*)&weightedSum)[0];
        center[1] = ((float*)&weightedSum)[1];
        float dx = AsCSpot(markerBytes)->markerScreen[0] - AsCSpot(markerBytes)->ballScreen[0];
        float dy = AsCSpot(markerBytes)->markerScreen[1] - AsCSpot(markerBytes)->ballScreen[1];
            float angle = (float)atan2(dy, dx);
            float rotationAngle = kFloat_1_5707964_Marker + angle;

        float originX = AsCSpot(markerBytes)->ballScreen[0];
        origin.x = originX;
        origin.y = AsCSpot(markerBytes)->ballScreen[1];
        origin.z = kFloat_0_spot;
        float endpointY = AsCSpot(markerBytes)->markerScreen[1];
        endpoint.x = AsCSpot(markerBytes)->markerScreen[0];
            endpoint.y = endpointY;
        endpoint.z = kFloat_0_spot;
        __mi__FRC7WVectorRC7WVector(&delta, &endpoint, &origin);

        float arrowWidth; float arrowLength;
        float length = WVectorLen(*(WVector*)&delta); arrowLength = AsCSpot(markerBytes)->arrowProgress * length;
        arrowWidth = AsCSpot(markerBytes)->arrowWidth;

            arrowRect[1] = center[1];
            arrowRect[2] = arrowWidth;
            arrowRect[0] = (center[0] - kFloat_0_5_Marker * arrowWidth);
        arrowRect[3] = arrowLength;
        arrowUV[0] = kFloat_0_spot;
        arrowUV[1] = kFloat_0_spot;
        arrowUV[2] = kFloat_1_spot;
        arrowUV[3] = kFloat_1_spot;

        Sprite_DrawRotatedQuadWithViewportRect(AsCSpot(markerBytes)->arrowOverlay, g_view, arrowUV, center[0], center[1],
            rotationAngle, arrowRect, 0, *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset), 0); float remaining = (kFloat_1_spot - AsCSpot(markerBytes)->arrowProgress) * length;
        float shadowWidth = AsCSpot(markerBytes)->arrowWidth;
            float shadowX = AsCSpot(markerBytes)->markerScreen[0] - kFloat_0_5_Marker * shadowWidth;
        shadowRect[1] = AsCSpot(markerBytes)->markerScreen[1];
            shadowRect[2] = shadowWidth;
            shadowRect[0] = shadowX;
            shadowRect[3] = remaining;
        shadowUV[0] = kFloat_0_spot;
        shadowUV[1] = kFloat_0_spot;
        shadowUV[2] = kFloat_1_spot;

            shadowUV[3] = kFloat_1_spot;
            Sprite_DrawRotatedQuadWithViewportRect(AsCSpot(markerBytes)->arrowOverlay, g_view, shadowUV, AsCSpot(markerBytes)->markerScreen[0], AsCSpot(markerBytes)->markerScreen[1],
            rotationAngle, shadowRect, 0, kArrowShadowColor, 0);
        // Invalid targets get only the invalid overlay; valid targets add world markers.
        if (AsCSpot(markerBytes)->groundHitIndex == kNoGroundHit) {
            float y; float size;
            size = AsCSpot(markerBytes)->endpointSize;
            y = AsCSpot(markerBytes)->markerScreen[1] - kFloat_0_5_Marker * size;
                float x = AsCSpot(markerBytes)->markerScreen[0] - kFloat_0_5_Marker * size;
            invalidRect[2] = size;
            invalidRect[0] = x;
            invalidRect[1] = y;
            invalidRect[3] = size;
            invalidUV[0] = kFloat_0_spot;
            invalidUV[1] = kFloat_0_spot;
            invalidUV[2] = kFloat_1_spot;
            invalidUV[3] = kFloat_1_spot;
            WOverlay_DrawTransformedRect(AsCSpot(markerBytes)->invalidTargetOverlay, g_view, invalidUV, kFloat_0_spot,
                invalidRect, 0, *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset), 0);
        } else {
            float y; float size;
            size = AsCSpot(markerBytes)->endpointSize;
            y = AsCSpot(markerBytes)->markerScreen[1] - kFloat_0_5_Marker * size;
                float x = AsCSpot(markerBytes)->markerScreen[0] - kFloat_0_5_Marker * size;
            validRect[2] = size;
                validRect[0] = x;
            validRect[1] = y;
            validRect[3] = size;
            validUV[0] = kFloat_0_spot;
            validUV[1] = kFloat_0_spot;
            validUV[2] = kFloat_1_spot;
            validUV[3] = kFloat_1_spot;
            WOverlay_DrawTransformedRect(AsCSpot(markerBytes)->validTargetOverlay, g_view, validUV, kFloat_0_spot,
                validRect, 0, *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset), 0);
            CSpot_DrawTexturedMarker(self);
            CSpot_DrawFallingMarker(self);
        }

        if (MarkerNear((AsCSpot(markerBytes)->positionBlend - s_markerCompletedPositionBlend))) {
            ((SpotRangeOverlay*)AsCSpot(markerBytes)->clubRangeInfo)->color = *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset);
            ((SpotRangeOverlay*)AsCSpot(markerBytes)->clubRangeInfo)->position = *(WVector*)((markerBytes + kSpotWorldPositionOffset));
            void* object = AsCSpot(markerBytes)->clubRangeInfo;
            ((MarkerObject*)object)->show();
        }
    } else if (AsCSpot(markerBytes)->groundHitIndex != kNoGroundHit) {
        // World-label path: raise by two world units, then label horizontal range in yards.
        CSpot_DrawTexturedMarker(self);
        CSpot_DrawFallingMarker(self);
        unsigned int raisedX = *(unsigned int*)(markerBytes + kSpotWorldPositionOffset);
        ((unsigned int*)&raised)[1] = *(unsigned int*)(markerBytes + kSpotWorldPositionOffset + 4);
        ((unsigned int*)&raised)[0] = raisedX;

            ((unsigned int*)&raised)[2] = *(unsigned int*)(markerBytes + kSpotWorldPositionOffset + 8);
            raised.y = raised.y + kFloat_2_Marker;
        CSpot_DrawProjectedClippedQuad(self, &raised, *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset));
        DebugText_SetColor(AsCSpot(markerBytes)->distanceText, *(unsigned int*)(markerBytes + kSpotPackedMarkerColorOffset));
        DebugText_SetWorldPosition(AsCSpot(markerBytes)->distanceText, &raised);
        // Distance label ignores vertical separation and divides world distance by 3.2 for yards.
        ballMinusMarker.y = kFloat_0_spot;
        float distance = WVectorLen(*(WVector*)&ballMinusMarker);
            DebugText_DrawFormattedAtWorldPosition(AsCSpot(markerBytes)->distanceText, g_view, -5, ((SpotStringPool*)&spotIndicatorStrings)->distanceYards,
            distance / kSpotWorldUnitsPerYard);
    }
    }
}

// Draw a fading, rotating ground quad around the current landing position.
// Required: live marker/view and loaded fallingBitmap. pulsePhase must be finite
// and in [0,1] at draw entry: 255*(1-phase) then fits unsigned byte [0,255],
// truncated toward zero. This conversion does not clamp invalid input.
// CSpot_OnProcess (spot_marker_actor.cpp) adds 0.5*animationStep, resets values >1 to 0;
// construction/target refresh set 0, message 6 sets 0.4. Nonnegative finite
// animation steps maintain this invariant; direct field edits must do so too.
// Sets shared render/texture state and submits four stack vertices synchronously.
void CSpot_DrawFallingMarker(void* self) {
    unsigned char* state = (unsigned char*)InstanceTable_Get();
    if (((MarkerInstanceTable*)state)->current->mode == 3)
        return;

    // The first descriptor is copied from the render-state defaults.
    unsigned int descriptor[26];
    unsigned int textureStageStates[12];
    unsigned int textureStageParameters[10];
    for (int i = 0; i < 26; ++i) descriptor[i] = spotRenderStateDefaults[i];
    descriptor[17] = 1;
    for (int i = 0; i < 12; ++i) textureStageStates[i] = s_markerTextureStageStates[i];
    for (int i = 0; i < 10; ++i) textureStageParameters[i] = s_spotTextureStageParameters[i];
    Engine_SetRenderStateList(descriptor, 13);
    Engine_SetTextureStageStateList(0, textureStageStates, 6);
    Engine_SetTextureStageParameterList(0, textureStageParameters, 5);

    void* device = Camera_GetRenderState(g_view);
    SetTransform(kTransformProjection, ((MarkerViewTransforms*)device)->projection);
    SetTransform(kTransformView, ((MarkerViewTransforms*)device)->view);
    Engine_SetType7Resource(0);
    SetTransform(kTransformWorld, (void*)((char*)&kIdentityMatrix4x4));
    SetTransform(kTransformTexture0, (void*)((char*)&kIdentityMatrix4x4));

    void* manager = AsWView(g_view)->renderTarget;
    void* object = AsCSpot(self)->fallingBitmap;
    void* table = *(void***)manager;
    /* original 0x800F1994-0x800F19C4: lwz r12,0(r3); lwz r12,0x70/0x74(r12); mtctr; bctrl (ours called table+slot without loading it) */
    Engine_SetTextureStageResource(0, ((MarkerTextureVtable*)table)->getStageResource(manager, object));
    Engine_SetTexturePalette(0, ((MarkerTextureVtable*)table)->getPaletteObject(manager, object));
    Engine_SetFlexibleVertexFormat(kWorldMarkerFormat);

    // World-space expanding pulse: alpha = 255*(1-phase), radius grows with
    // phase and the configured golf scale. Lift above ground avoids z fighting.
    float fade = kFloat_1_spot - AsCSpot(self)->pulsePhase;
    unsigned char alpha = (unsigned char)(kFloat_255_SpotColor * fade);

    float scale = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
    float radius = kSpotWorldUnitsPerYard * scale * kFloat_0_05_Spot;
    radius = radius * AsCSpot(self)->pulsePhase;
    float verticalLift = kFloat_2_Marker + kFloat_0_05_Spot * radius;
    struct Vertex { float xyz[3]; unsigned char color[4]; float uv[2]; } verts[4];
    Vertex* out = verts;
    for (int i = 0; i < 4; ++i) {
        float xSign = (i & 2) ? kFloat_1_spot : kFloat_neg1_U800EFEAC;
        float zSign = (i & 1) ? kFloat_1_spot : kFloat_neg1_U800EFEAC;
        float rotation[16];
        // Build an XZ corner, rotate about world Y in radians, then translate
        // by the landing position. UV orientation follows strip vertex order.
        float point[3] = {xSign * radius, verticalLift, zSign * radius};
        float rotated[3];
        // rotationPhase is radians: OnProcess adds pi*deltaSeconds while enabled
        // and sprites are ready, then subtracts 2*pi ONCE when >2*pi. It pauses
        // with that update gate; it is not wall time or a rendered-frame count.
        // 0.7 is a dimensionless angular gain, giving 0.7*pi rad/s between wraps.
        // The source phase wraps before this gain: the displayed angle therefore
        // jumps from about 1.4*pi to zero every two active seconds. Keep this
        // sawtooth/reset behavior; do not wrap the scaled angle as a cleanup.
        Matrix_SetAxisRotation(&rotation, 0.7f * AsCSpot(self)->rotationPhase, 1);
        Matrix_TransformPoint(rotated, point, rotation);
        *(WVector*)&point = *(WVector*)&rotated;
        *(WVector*)point += *(WVector*)(AsCSpot(self)->position);
        Vec3_CopyToOutput(point, out->xyz);
        out->color[0] = out->color[1] = out->color[2] = 255;
        out->color[3] = alpha;
        out->uv[0] = (i & 1) ? kFloat_1_spot : kFloat_0_spot;
        out->uv[1] = (i & 2) ? kFloat_0_spot : kFloat_1_spot;
        ++out;
    }
    DrawPrimitiveUP(kTriangleStrip, 2, verts, kMarkerVertexBytes);
}

void* GolfBall__Fv();
void* __mi__FRC7WVectorRC7WVector(void*, void*, void*);
float Vector_SignedAngleXZ(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void* Matrix_SetAxisRotation(void*, float, int);
void* InstanceTable_Get();
float GolfParameter_ResolveFloat(void*, int);
void* __ml__FfRC7WVector(void*, void*, float);
void* __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern float kSpotWorldUnitsPerYard;
extern float kFloat_0_spot;
extern const float kFloat_0_4_Spot;

// Apply marker enable/heading/transition messages or return the requested position and angle.
void CSpot_HandleMsg(void* self, const void* arg) {
    unsigned int type = ((const SpotMessage*)arg)->type;
    switch (type) {
    case kAddHeading: {
        float* value = ((const SpotMessage*)arg)->value.scalar;
        AsCSpot(self)->heading += *value;
        AsCSpot(self)->targetDirty = 1;
        break;
    }
    case kSetHeading: {
        float* value = ((const SpotMessage*)arg)->value.scalar;
        AsCSpot(self)->heading = *value;
        AsCSpot(self)->targetDirty = 1;
        break;
    }
    case kGetTargetAngle: {
        float b[3]; float a[3];
        __mi__FRC7WVectorRC7WVector(a, AsCSpot(self)->targetPosition, GolfBall__Fv());
        __mi__FRC7WVectorRC7WVector(b, AsCSpot(self)->position, GolfBall__Fv());
        *((const SpotMessage*)arg)->value.scalar = Vector_SignedAngleXZ(b, a);
        break;
    }
    case kGetPosition:
        __as__7WVectorFRC7WVector(((const SpotMessage*)arg)->value.pointer, AsCSpot(self)->position);
        break;
    case kSetEnabledFromInteger: AsCSpot(self)->enabled = (unsigned int)(((const SpotMessage*)arg)->value.integer - 1) == 0; break;
    case kEnableMarker:
        AsCSpot(self)->enabled = 1;
        break;
    case kDisableMarker:
        AsCSpot(self)->enabled = 0;
        break;
    case kDirtyTarget:
        AsCSpot(self)->targetDirty = 1;
        break;
    case kSetAimTransition: { float* value = ((const SpotMessage*)arg)->value.scalar;
        float tmp[9];
        float val = *value;
        __as__7WVectorFRC7WVector(AsCSpot(self)->transitionReferencePosition, ((const SpotMessage*)arg)->secondPointer);
        __as__7WVectorFRC7WVector(AsCSpot(self)->previousPosition, AsCSpot(self)->position);

        Matrix_SetAxisRotation(tmp, val, 1);
        float scale = GolfParameter_ResolveFloat(InstanceTable_Get(), 0);
            scale = (kSpotWorldUnitsPerYard * scale);
        float result[3];

            float scaled[3];
            __ml__FfRC7WVector(scaled, &tmp[6], scale);
        __pl__FRC7WVectorRC7WVector(result, GolfBall__Fv(), scaled);
        __as__7WVectorFRC7WVector(AsCSpot(self)->targetPosition, result);

            float _a = kFloat_0_spot;
            AsCSpot(self)->pulsePhase = (kFloat_0_4_Spot);
            AsCSpot(self)->positionBlend = _a;
        break;
    }
    case 1:
    case 8:
    case 10:
    case 0x2719:
        break;
    case kSetPositionBlend: {
        float* value = ((const SpotMessage*)arg)->value.scalar;
        AsCSpot(self)->positionBlend = *value;
        break;
    }
    default:
        break;
    }
}

} // extern "C"
