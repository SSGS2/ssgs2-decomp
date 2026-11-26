// Caddie models and the reactions that drive them: binding a player's caddie to
// the scene, shadow and effect system, loading its model asynchronously, stopping
// it at the predicted landing point, and choosing dodge animations from the
// incoming shot.
#include <decomp/offset_of.h>
#include "game/golfball.h"
#include "classes/WPuppet.h"
#include "iff/IffStruct.h"
#include "classes/CPetBody.h"
#include "classes/IntStringTreeNode8013.h"
#define AsIntStringTreeNode8013(p) ((IntStringTreeNode8013*)(p))
#include "classes/CSharedDoc.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/Context8012F978.h"
#define AsContext8012F978(p) ((Context8012F978*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))

#include "game/singletons.h"
#include "game/wvector.h"
#include "sdk_protos.h"
#include "classes/CGolfCaddie.h"
#include "classes/CFx.h"
#include "classes/CShadowManager.h"

extern "C" {

// Model-holder records are distinct from CPetBody, whose pointer is their first word.
enum {
    kLandingStopWindow = 30,
    kTutorialFinishedStage = 6
};
struct CaddieBoneEntry {
    void* bone;
    void* metadata;
};
struct CaddieStringNodeView {
    char treeAndKey[0x10];
    unsigned char flags;
    char inlineText[11];
};
enum {
    kLoadTreeOffset = (int)DECOMP_OFFSETOF(Context8012F978, pendingTextCount),
    kTreeValueOffset = 12,
    kTreeStringOffset = 16,
    kTreeInlineTextOffset = (int)DECOMP_OFFSETOF(CaddieStringNodeView, inlineText)
};

struct CaddieHolderView {
    void* body;
    char padding004[0xC];
    IFF_STRUCT::sCaddie* caddieData;
    void* equippedItem;
    char padding018[0x20];
    float stoppingDistance;
};
struct CaddieBindingView {
    unsigned int type;
    void* courseSlot;
    void* holder;
};
struct CourseCaddieSlotView {
    unsigned char type;
    char padding001[3];
    void* puppet;
    void* shadowPuppet;
};
struct CaddieEquipmentView {
    unsigned int caddieId, type, itemId;
};
struct QuadTreeReactionView {
    char padding000[0x2164];
    unsigned char shotReaction;
};
struct BallSampleView {
    unsigned int position[3];
    unsigned int velocity[3];
};
// A vector copied as three raw words, as the original does when the values pass through unchanged.
struct VectorWords {
    unsigned int x, y, z;
};
// Byte layout of the existing string pool; encoded lengths include the NUL.
struct CaddieStrings {
    char padding000[0xB];
    char playerNameFormat[10]; // CADDIE_%d
    char padding015[0xB];
    char caddieShadowName[7]; // caddie
    char sunLight[4];         // sun
    char padding02B[0xF8];
    char breathBone[8];    // Breathe
    char breathEffect[11]; // breath.seq
    char padding136[0xB];
    char spineBone[12];             // Bip01_Spine
    char fallbackBone[7];           // Bone03
    char dodgeLeft[11];             // 左に皮下期
    char dodgeRight[11];            // 右に皮下期
    char dodgeHighBall[17];         // 高いボール皮下期
    char stopAnimation[7];          // 急停車
    char tutorialStopAnimation[21]; // チュートリアル急停車
    char bunkerAnimation[9];        // バンカー
    char storyStand[15];            // MT_STORY_STAND
    char storyModelSuffix[11];      // .story.pet
    char modelSuffix[5];            // .pet
};
enum {
    kCaddieStrings_playerNameFormat = (int)DECOMP_OFFSETOF(CaddieStrings, playerNameFormat),
    kCaddieStrings_caddieShadowName = (int)DECOMP_OFFSETOF(CaddieStrings, caddieShadowName),
    kCaddieStrings_sunLight = (int)DECOMP_OFFSETOF(CaddieStrings, sunLight),
    kCaddieStrings_breathBone = (int)DECOMP_OFFSETOF(CaddieStrings, breathBone),
    kCaddieStrings_breathEffect = (int)DECOMP_OFFSETOF(CaddieStrings, breathEffect),
    kCaddieStrings_spineBone = (int)DECOMP_OFFSETOF(CaddieStrings, spineBone),
    kCaddieStrings_fallbackBone = (int)DECOMP_OFFSETOF(CaddieStrings, fallbackBone),
    kCaddieStrings_dodgeLeft = (int)DECOMP_OFFSETOF(CaddieStrings, dodgeLeft),
    kCaddieStrings_dodgeRight = (int)DECOMP_OFFSETOF(CaddieStrings, dodgeRight),
    kCaddieStrings_dodgeHighBall = (int)DECOMP_OFFSETOF(CaddieStrings, dodgeHighBall),
    kCaddieStrings_stopAnimation = (int)DECOMP_OFFSETOF(CaddieStrings, stopAnimation),
    kCaddieStrings_tutorialStopAnimation = (int)DECOMP_OFFSETOF(CaddieStrings, tutorialStopAnimation),
    kCaddieStrings_bunkerAnimation = (int)DECOMP_OFFSETOF(CaddieStrings, bunkerAnimation),
    kCaddieStrings_storyStand = (int)DECOMP_OFFSETOF(CaddieStrings, storyStand),
    kCaddieStrings_storyModelSuffix = (int)DECOMP_OFFSETOF(CaddieStrings, storyModelSuffix),
    kCaddieStrings_modelSuffix = (int)DECOMP_OFFSETOF(CaddieStrings, modelSuffix),
};

void* String_FormatTemporary(const char*, ...);

// Read the course collision manager reaction code used by caddie dodge animations.
unsigned char CQuadTree_GetCaddieShotReaction(void* self) {
    return ((QuadTreeReactionView*)self)->shotReaction;
}

// Forward the look-at target to the active caddie body, if loaded.
void CGolfCaddie::SetLookAtTarget(void* target, unsigned char enabled) {
    void* p = AsContext8012F978(this)->animationHolder;
    if (p == 0)
        return;
    ((CPetBody*)(*(void**)p))->SetLookAtTarget(target, enabled);
}

void appFreeUni__FPv(void*);
void __dl__FPv(void*);
extern char golfCaddieStrings[];
void* g_sharedDoc;
void* WPuppet_FindNamedBoneEntry(void*, const char*);
void Matrix_ExpandAffineTo4x4(void*, void*);
void* ModelRenderParams_GetShared();
void ModelRenderParams_Copy(void*, void*);
void Mtx44_TransformDirection(void*, void*, void*);
void Mtx44_SetRotationY(void*, float);
void Vec3_Normalize(void*, void*);
void WPuppet_SetRenderParams(void*, void*);
void WPuppet_SetRenderParamsEnabled(void*, int);
void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
float Vector_SignedAngleXZ(void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void* CQuadTree_GetInstance();
void WPuppet_RebuildBoneController(void*);
void WPuppet_InitializeTextureCoordinateAnimation(void*);
void RBTree_RebalanceAfterErase(void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
float __ml__FRC7WVectorRC7WVector(void*, void*);
float Magnitude__7WVectorCFv(void*);
void Normalize__7WVectorFv(void*);
void __ml__FRC7WVectorf(void*, void*, float);
int GetGroundType__9CQuadTreeF7WVector(void*, void*);
WVector GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, WVector, int, int, int);
void IntStringTree_DestroySubtree(void*, void*);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void* ItemInventory_FindCaddieItemData(void*, void*);
void String_ReserveCapacity(void*, int);
void* IntStringTree_InsertAtPosition(void*, void*, unsigned char, unsigned char, void*);
void RenderPacket_Init(void*);
extern "C" void GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(void*, const char*, float, int, const char*,
                                                                        const char*);
void RenderPacket_Destroy();
struct Node_80130C2C;
struct IntStringTreeHeader;
Node_80130C2C* IntStringTree_FindInsertPosition(IntStringTreeHeader*, int*, Node_80130C2C**, unsigned char*,
                                                unsigned char*);
extern signed char s_renderPacketInitialized;
extern void* g_shadowManager;
extern char s_renderPacket[];
extern char s_renderPacketDtorRegistration_804F68E8[];
extern const float kFloat_0_09_CaddieShadow;
extern float kFloat_neg1_5707964_CaddieRotation;
extern const float kFloat_0_GolfCaddie;
extern float kFloat_0_00001_Caddie;
extern float kFloat_2_GolfPlayer;
extern float kFloat_1_GolfCaddie;
extern float kFloat_5_Caddie;
// g_sharedDoc is declared above as a pointer.
extern int g_fx;

// Stop or reposition the moving caddie near the predicted landing point, then update lighting.
// activeCaddie is the caddie entry; its first word is the body that owns the motion.
void CGolfCaddie::UpdateMovement() {
    if (activeCaddie) {
        float stoppingDistance;
        float shotCurveMagnitude;
        float movementVectors[6]; // XYZ displacement, then XYZ body direction.
        WVector repositionVector;
        // Phase 1: snapshot body position/direction in world space. Flatten both
        // vectors to XZ; their dot product tests whether movement passed the target.
        __as__7WVectorFRC7WVector((char*)position, (char*)&((CPetBody*)(*(void**)activeCaddie))->positionX);

        void* body = *(void**)activeCaddie;

        *(VectorWords*)(movementVectors + 3) = *(VectorWords*)((CPetBody*)body)->movementDirection;
        __mi__FRC7WVectorRC7WVector(movementVectors, movementTarget, position);
        movementVectors[4] = kFloat_0_GolfCaddie;
        movementVectors[1] = kFloat_0_GolfCaddie;
        float movementAlignment = __ml__FRC7WVectorRC7WVector(movementVectors + 3, movementVectors);
        void* status = GolfBall__Fv();
        // Phase 2: use twice the configured stopping distance for a non-negligible
        // shot curve setting. The sign-bit test is the original float absolute-value
        // idiom (including -0); keep it rather than changing NaN/sign behavior.
        if (*(unsigned int*)&((CGolfBall*)status)->m_initCurve & 0x80000000)
            shotCurveMagnitude = -((CGolfBall*)status)->m_initCurve;
        else
            shotCurveMagnitude = ((CGolfBall*)status)->m_initCurve;

        float savedCurveMagnitude = shotCurveMagnitude;
        bool shotCurveIsNegligible = savedCurveMagnitude < kFloat_0_00001_Caddie;
        if (shotCurveIsNegligible)
            stoppingDistance = ((CaddieHolderView*)activeCaddie)->stoppingDistance;
        else
            stoppingDistance = kFloat_2_GolfPlayer * ((CaddieHolderView*)activeCaddie)->stoppingDistance;
        // The original samples this magnitude and never reads it.
        float directionLength = Magnitude__7WVectorCFv(movementVectors + 3);

        // Stop if close enough, OR if a positive event sample exists AND either
        // fewer than 30 prediction samples remain or the target lies behind us.
        // No time unit is implied by 30: it is a sample-index difference. The
        // prediction/event indices and loaded body are trusted by this routine.
        if ((Magnitude__7WVectorCFv(movementVectors)) < stoppingDistance ||
            ((((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] > 0) &&
             ((((CGolfBall*)GolfBall__Fv())->m_preDataCount - ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0] <
               kLandingStopWindow) ||
              movementAlignment < kFloat_0_GolfCaddie))) {
            // Phase 3: restore root-motion scale, choose stop animation and remove trail FX.
            // Stage 6 is the finished/non-lesson state; the tutorial animation
            // resource is selected for every other stage.
            ((CPetBody*)(*(void**)activeCaddie))->rootMotionScale = kFloat_1_GolfCaddie;
            void* manager = (*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
            if (AsCGolfDoc(manager)->tutorialStage == kTutorialFinishedStage)
                ((CPetBody*)(*(void**)activeCaddie))
                    ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                          (void*)(golfCaddieStrings + kCaddieStrings_stopAnimation), 1, 0);
            else
                ((CPetBody*)(*(void**)activeCaddie))
                    ->PlayAnimationByName(kFloat_0_GolfCaddie,
                                          (void*)(golfCaddieStrings + kCaddieStrings_tutorialStopAnimation), 1, 0);
            animationState = 0;
            ((CFx*)g_fx)->RemoveControl(runDustEffect);
            if (((CGolfBall*)GolfBall__Fv())->m_preDataCount > 0) {
                // Inspect the terminal prediction sample for follow-up animation.
                // PreData(count) is deliberately used as an addressable sample.

                void* ball = GolfBall__Fv();
                void* landingSample = PreData__9CGolfBallFi(ball, ((CGolfBall*)ball)->m_preDataCount);
                float landingPosition[3];
                *(VectorWords*)landingPosition = *(VectorWords*)landingSample;
                unsigned int groundType = GetGroundType__9CQuadTreeF7WVector(CQuadTree_GetInstance(), landingPosition);
                if (((CGolfBall*)ball)->m_recoveryRequired || (unsigned char)groundType >= 3)
                    ((CPetBody*)(*(void**)activeCaddie))
                        ->SetNextAnimation(golfCaddieStrings + kCaddieStrings_bunkerAnimation, 0);
                float horizontalDirectionLength = Magnitude__7WVectorCFv(movementVectors + 3);
                if (*(unsigned int*)&horizontalDirectionLength & 0x80000000)
                    horizontalDirectionLength = -horizontalDirectionLength;
                int directionIsNegligible = horizontalDirectionLength < kFloat_0_00001_Caddie;
                if (directionIsNegligible) {
                    // Phase 4: for a stationary direction, place the caddie five
                    // world-distance units before the last sample along the shot,
                    // then project onto ground. Assumes a normalizable shot span.
                    float groundScratch[3];
                    void* current = GolfBall__Fv();
                    void* firstBallSample = PreData__9CGolfBallFi(current, 0);
                    repositionVector =
                        *(WVector*)PreData__9CGolfBallFi(current, ((CGolfBall*)current)->m_preDataCount) -
                        *(WVector*)firstBallSample;
                    repositionVector.Normalize();
                    repositionVector =
                        *(WVector*)PreData__9CGolfBallFi(current, ((CGolfBall*)current)->m_preDataCount) -
                        repositionVector * kFloat_5_Caddie;
                    repositionVector =
                        GetGroundPoint__9CQuadTreeF7WVectorbPib(CQuadTree_GetInstance(), repositionVector, 0, 0, 0);
                    ((CPetBody*)(*(void**)activeCaddie))->SetPosition(&repositionVector);
                    *(WVector*)position = repositionVector;
                }
            }
        }
        // Refresh lighting even when no stop/reposition transition occurred.
        ((CPetBody*)(*(void**)activeCaddie))->SetupLighting();
    }
}

extern void* g_sceneManager;
extern char s_conteModelSlots[];
extern signed char s_courseSlotTableInitialized;
void* FxInterpreter_GetInstance();
void FxInterpreter_ClearEntries(void*);
void DrawScene_SetNamedObjectPuppet(void*, void*, int);
void GfxObjList_RemoveGroupByName(void*, void*);
void CourseSlotTable_InitEmpty(void*);
void* CourseSlotTable_GetExtraEntry(void*, int);
void FxInterpreter_RemoveEntriesByKey(void*, void*);
void* FxAttachments_CreateFromResourceTags(void*, void*, int);
void* SharedDoc_IsCourse8Or13();
void* Matrix_SetAxisRotation(void*, float, int);
void Matrix_CopyAffine(void*, void*);
void Matrix_ConcatAffineAssign(void*, void*);
void FxAttachment_Init(void*);
extern "C" void CreateAttachedEffect__3CFxFP9Vec2_8010PCcUcii(void*, void*, const char*, int, int, int, int);
void DrawScene_SetNamedObjectEnabled(void*, void*, int);
extern "C" void ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(void*, void*, void*);
void ShadowModel_SetNamedProjectionEnabled(void*, void*, int);
void __as__7WVectorFRC7WVector(void*, void*);

// Bind a player caddie model to scene, shadow and effects, or clear the absent slot.
void CGolfCaddie::ActivatePlayerModel(int playerIndex, float yawRadians, void* positionOrBone) {
    // playerIndex selects the holder/binding; yawRadians is model yaw in radians.
    // positionOrBone initially borrows a world position; the local is reused
    // later for puppet/bone lookup. Slot/puppet/equipment pointers stay borrowed.
    // Phase 1: resolve player equipment/holder; absent holders detach scene/shadow.
    void* courseSlot;
    int attachmentAddress;
    void* equipmentOrTransform;
    // Reused scratch: player-name string, string pool, then scene/shadow manager.
    void* resourceContext;
    void* modelHolder;
    void* playerName;
    int selectedPlayer;
    CGolfCaddie* caddie;
    caddie = this;
    selectedPlayer = playerIndex;
    float savedYaw = yawRadians;

    playerName =
        (void*)String_FormatTemporary((char*)golfCaddieStrings + kCaddieStrings_playerNameFormat, selectedPlayer);
    struct Record {
        char pad[0x1b4];
        char data[0x20];
        // Borrow this player record view from the shared document.
        static void* get(int i) {
            return ((Record*)g_sharedDoc)[i].data;
        }
    };
    equipmentOrTransform = Record::get(selectedPlayer);

    struct Items {
        // Return a loaded holder by index, or null for an invalid index.
        static void* get(CGolfCaddie* caddie, int playerIndex) {
            if (playerIndex < 0 || (unsigned int)playerIndex >= caddie->caddieEntryCount)
                return 0;
            return caddie->caddieEntries[playerIndex];
        }
    };
    modelHolder = Items::get(caddie, selectedPlayer);

    if (modelHolder == 0) {
        resourceContext = String_FormatTemporary(golfCaddieStrings + kCaddieStrings_playerNameFormat, selectedPlayer);
        FxInterpreter_ClearEntries(FxInterpreter_GetInstance());
        DrawScene_SetNamedObjectPuppet((void*)g_sceneManager, resourceContext, 0);
        GfxObjList_RemoveGroupByName((void*)g_shadowManager, resourceContext);
        if ((void*)GetConteModelSlotsSlot() == 0) {
            if (!s_courseSlotTableInitialized) {
                CourseSlotTable_InitEmpty((char*)s_conteModelSlots);
                s_courseSlotTableInitialized = 1;
            }
            GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
        }
        void* r3 = CourseSlotTable_GetExtraEntry((void*)GetConteModelSlotsSlot(), selectedPlayer);
        *(int*)&((CourseCaddieSlotView*)r3)->puppet = 0;
        *(int*)&((CourseCaddieSlotView*)r3)->shadowPuppet = 0;
        void* base = (char*)caddie->conteCaddies;
        void* p = (char*)base + selectedPlayer * 0xc;
        *(int*)&((CaddieBindingView*)p)->courseSlot = 0;
        *(int*)&((CaddieBindingView*)p)->holder = 0;
    } else {

        if ((void*)GetConteModelSlotsSlot() == 0) {
            if (!s_courseSlotTableInitialized) {
                CourseSlotTable_InitEmpty((char*)s_conteModelSlots);
                s_courseSlotTableInitialized = 1;
            }
            GetConteModelSlotsSlot() = (void*)s_conteModelSlots;
        }
        courseSlot = CourseSlotTable_GetExtraEntry((void*)GetConteModelSlotsSlot(), selectedPlayer);
        *(unsigned char*)courseSlot = ((CaddieEquipmentView*)equipmentOrTransform)->type;
        void* t = ((CPetBody*)(*(void**)modelHolder))->GetPuppet();
        ((CourseCaddieSlotView*)courseSlot)->puppet = t;
        t = ((CPetBody*)(*(void**)modelHolder))->GetPuppet();
        ((CourseCaddieSlotView*)courseSlot)->shadowPuppet = t;
        ((CaddieHolderView*)modelHolder)->equippedItem = equipmentOrTransform;
        ((CPetBody*)(*(void**)modelHolder))->ResetPlaybackAndLookAt();
        ((CPetBody*)(*(void**)modelHolder))->SetPosition(positionOrBone);
        ((CPetBody*)(*(void**)modelHolder))->SetYaw(savedYaw);
        resourceContext = golfCaddieStrings;
        ((CPetBody*)(*(void**)modelHolder))
            ->PlayAnimationByName(kFloat_0_GolfCaddie, (void*)((char*)resourceContext + kCaddieStrings_storyStand), 0,
                                  0);
        ((CPetBody*)(*(void**)modelHolder))->ApplyAnimationToPuppet();
        ((CPetBody*)(*(void**)modelHolder))->SetupLighting();
        ((CPetBody*)(*(void**)modelHolder))->SetLookAtTarget(0, 1);
        void* t1 = FxInterpreter_GetInstance();
        FxInterpreter_RemoveEntriesByKey(t1, (void*)caddie->effectEntryKey);
        positionOrBone = (void*)((CPetBody*)(*(void**)modelHolder))->GetPuppet();
        void* t2 = FxInterpreter_GetInstance();
        caddie->effectEntryKey = (int)FxAttachments_CreateFromResourceTags(t2, positionOrBone, 0);

        if (SharedDoc_IsCourse8Or13() != 0) {
            positionOrBone = (char*)resourceContext + kCaddieStrings_breathBone;
            void* t3 = ((CPetBody*)(*(void**)modelHolder))->GetPuppet();
            positionOrBone = WPuppet_FindNamedBoneEntry(t3, (const char*)positionOrBone);
            if (positionOrBone != 0) {
                char buf48[0x30];
                char buf20[0x28];
                struct Vector {
                    unsigned int data[3];
                };
                Vector saved;
                float zero[3];
                Matrix_SetAxisRotation(buf48, kFloat_neg1_5707964_CaddieRotation, 1);
                attachmentAddress = selectedPlayer * 0x30;
                void* src = (char*)*(void**)positionOrBone + 0x78;
                equipmentOrTransform = (char*)caddie->affineMatrices[0] + attachmentAddress;
                Matrix_CopyAffine(equipmentOrTransform, src);
                attachmentAddress = (int)((char*)(caddie->affineMatrices[0] + 9) + attachmentAddress);
                saved = *(Vector*)attachmentAddress;
                float f0 = kFloat_0_GolfCaddie;
                zero[0] = f0;
                zero[1] = f0;
                zero[2] = f0;
                *(WVector*)((void*)attachmentAddress) = *(WVector*)zero;
                Matrix_ConcatAffineAssign(equipmentOrTransform, buf48);
                *(WVector*)((void*)attachmentAddress) = *(WVector*)&saved;
                FxAttachment_Init(buf20);
                *(void**)(buf20 + 0x08) = equipmentOrTransform;
                void* r9 = ((CaddieBoneEntry*)positionOrBone)->metadata;
                int r0v = (int)r9 + 0x18;
                *(int*)(buf20 + 0x18) = r0v;
                void* r3call = (void*)g_fx;
                CreateAttachedEffect__3CFxFP9Vec2_8010PCcUcii(
                    r3call, buf20, (char*)resourceContext + kCaddieStrings_breathEffect, 1, 0, 0, (int)r9);
            }
        }

        // Phase 2: publish the loaded puppet to the named scene object and shadow.
        // The local attachment path above rotates the bone frame by -pi/2 about Y
        // while restoring its world translation after the rotation concatenation.
        resourceContext = g_sceneManager;
        t = ((CPetBody*)(*(void**)modelHolder))->GetPuppet();
        DrawScene_SetNamedObjectPuppet(resourceContext, playerName, (int)(long)t);
        DrawScene_SetNamedObjectEnabled((void*)g_sceneManager, playerName, 1);

        if ((void*)((int&)Sing_805DC7B0()) == 0) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init((char*)s_renderPacket);
                __register_global_object((void*)s_renderPacket, (void*)RenderPacket_Destroy,
                                         (void*)s_renderPacketDtorRegistration_804F68E8);
                s_renderPacketInitialized = 1;
            }
            ((int&)Sing_805DC7B0()) = (int)(long)s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, (const char*)playerName, kFloat_0_09_CaddieShadow, 0x10,
            golfCaddieStrings + kCaddieStrings_sunLight, (char*)((int&)Sing_805DC7B0()) + 0x4c);

        resourceContext = g_shadowManager;
        t = ((CPetBody*)(*(void**)modelHolder))->GetPuppet();
        ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(resourceContext, playerName, t);
        ShadowModel_SetNamedProjectionEnabled((void*)g_shadowManager, playerName, 1);

        void* base2 = (char*)caddie->conteCaddies;
        void* p2 = (char*)base2 + selectedPlayer * 0xc;
        ((CaddieBindingView*)p2)->courseSlot = courseSlot;
        ((CaddieBindingView*)p2)->holder = (void*)modelHolder;
    }
}

void* String_FormatTemporary(const char*, ...);

// Choose a left, right or high-ball dodge animation from ball direction and reaction state.
void CGolfCaddie::PlayShotReaction() {
    if (AsContext8012F978(this)->animationHolder == 0)
        return;
    void* cur = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
    char v[0x10];
    __mi__FRC7WVectorRC7WVector(v, (char*)AsContext8012F978(this)->position, cur);
    cur = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
    extern float kFloat_neg1_GolfPlayerDirection;
    float _angle = Vector_SignedAngleXZ(v, ((BallSampleView*)cur)->velocity);
    float angle = _angle * kFloat_neg1_GolfPlayerDirection;
    int state = ((unsigned char (*)(void*))CQuadTree_GetCaddieShotReaction)(CQuadTree_GetInstance());
    switch (state) {
    case 2:
    case 3:
        if (angle > kFloat_0_GolfCaddie) {
            char* base = golfCaddieStrings;
            if (CanPlayAnimation(AsContext8012F978(this)->animationHolder, base + kCaddieStrings_dodgeLeft)) {
                AsContext8012F978(this)->animationActive = 1;
                void** p = (*(void***)&AsContext8012F978(this)->animationHolder);
                ((CPetBody*)(*p))->PlayAnimationByName(kFloat_0_GolfCaddie, base + kCaddieStrings_dodgeLeft, 0, 0);
            }
        } else {
            char* base = golfCaddieStrings;
            if (CanPlayAnimation(AsContext8012F978(this)->animationHolder, base + kCaddieStrings_dodgeRight)) {
                AsContext8012F978(this)->animationActive = 1;
                void** p = (*(void***)&AsContext8012F978(this)->animationHolder);
                ((CPetBody*)(*p))->PlayAnimationByName(kFloat_0_GolfCaddie, base + kCaddieStrings_dodgeRight, 0, 0);
            }
        }
        break;
    case 1: {
        char* base = golfCaddieStrings;
        if (CanPlayAnimation(AsContext8012F978(this)->animationHolder, base + kCaddieStrings_dodgeHighBall)) {
            AsContext8012F978(this)->animationActive = 1;
            void** p = (*(void***)&AsContext8012F978(this)->animationHolder);
            ((CPetBody*)(*p))->PlayAnimationByName(kFloat_0_GolfCaddie, base + kCaddieStrings_dodgeHighBall, 0, 0);
        }
        break;
    }
    case 0:
        AsContext8012F978(this)->animationActive = 0;
        SetLookAtTarget(GolfBall__Fv(), 1);
        return;
    }
}

struct CF8Link {
    CF8Link* left;
    CF8Link* right;
    unsigned int parent;
};
struct CF8Iterator {
    char* ptr;
    // Copy a borrowed tree cursor without transferring node ownership.
    CF8Iterator(const CF8Iterator& other) : ptr(other.ptr) {}
    // Replace the cursor with another borrowed node address.
    CF8Iterator& operator=(const CF8Iterator& other) {
        ptr = other.ptr;
        return *this;
    }
    // Compare tree positions by node address.
    bool operator==(CF8Iterator other) const {
        return ptr == other.ptr;
    }
    // Advance in order, masking the parent color bit while ascending.
    CF8Iterator& operator++() {
        CF8Link* child = ((CF8Link*)ptr)->right;
        if (child) {
            while (child->left)
                child = child->left;
            ptr = (char*)child;
            return *this;
        } else {
            CF8Link* parent;
            while ((CF8Link*)ptr != (parent = (CF8Link*)(((CF8Link*)ptr)->parent & ~1u))->left)
                ptr = (char*)parent;
            ptr = (char*)parent;
        }
        return *this;
    }
};

struct CF8Node {
    char padding000[0x10];
    unsigned int flags;
    unsigned int length;
    void* buffer;
};

// Poll one queued model load, finalize its puppet, and start the next request.
int CGolfCaddie::PollPendingModelLoads() {
    // 1 means the queue is empty/drained, 0 means work remains. A completed load
    // may have a null puppet: completion here does not guarantee model success.
    // Tree keys index modelHolders; node strings own filenames until node erase.
    if (AsContext8012F978(this)->pendingTextCount == 0)
        return 1;
    void* current = AsContext8012F978(this)->pendingTextIterator;
    unsigned int index = *(unsigned int*)&AsIntStringTreeNode8013(current)->key;
    void* node = *(void**)((char*)(*(void**)&AsContext8012F978(this)->modelHolders) + index * 4);
    if (!((CPetBody*)(*(void**)node))->PollModelLoad())
        return 0;
    if (((CPetBody*)(*(void**)node))->GetPuppet()) {
        WPuppet_RebuildBoneController(((CPetBody*)(*(void**)node))->GetPuppet());
        WPuppet_InitializeTextureCoordinateAnimation(((CPetBody*)(*(void**)node))->GetPuppet());
    }
    if (AsContext8012F978(this)->storyModelMode == 0)
        ParseAnimationMetadata(node);
    // Empty the filename without releasing capacity, then advance begin BEFORE
    // erase/rebalance. Free heap text (bit 31) and finally the node itself.
    if (((AsIntStringTreeNode8013(current)->stringFlags >> 31) == 0)) {
        ((CaddieStringNodeView*)current)->inlineText[0] = 0;
        *(signed char*)&AsIntStringTreeNode8013(current)->stringFlags &= (signed char)~0x7F;
    } else {
        *(unsigned char*)AsIntStringTreeNode8013(current)->stringBuffer = 0;
        *(int*)&AsIntStringTreeNode8013(current)->stringLength = 0;
    }
    CF8Iterator& begin = *(CF8Iterator*)&AsContext8012F978(this)->pendingTextIterator;
    CF8Iterator it = begin;
    current = it.ptr;
    if (it == begin) {
        begin = ++it;
    }
    RBTree_RebalanceAfterErase(current, AsContext8012F978(this)->pendingTextTreeRoot);
    CF8Node* dead = (CF8Node*)current;
    if ((char*)dead + kTreeValueOffset && (char*)dead + kTreeStringOffset && (dead->flags >> 31))
        appFreeUni__FPv(dead->buffer);
    appFreeUni__FPv(current);
    int count = --AsContext8012F978(this)->pendingTextCount;
    if (count != 0) {
        void* next = AsContext8012F978(this)->pendingTextIterator;
        void* item = *(void**)((char*)(*(void**)&AsContext8012F978(this)->modelHolders) +
                               (*(unsigned int*)&AsIntStringTreeNode8013(next)->key * 4));
        void* resource = *(void**)item;
        void* callback;
        if ((AsIntStringTreeNode8013(next)->stringFlags >> 31) == 0)
            callback = ((CaddieStringNodeView*)next)->inlineText;
        else
            callback = AsIntStringTreeNode8013(next)->stringBuffer;
        ((CPetBody*)resource)->RequestModelLoad(callback);
        return 0;
    }
    return 1;
}

struct EC8VirtualObject {
    virtual void slot0();
    virtual void update();
    virtual int complete();
};

// Free a heap-backed string payload and optionally its containing object.
void* HeapString_dtor(char* self, int deallocate) {
    if (self != 0) {
        if (self + 4 != 0) {
            if ((*(unsigned int*)(self + 4) >> 31) != 0)
                appFreeUni__FPv(*(void**)(self + 0xC));
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}

// Advance the pending operation and report whether its player is still current.
int CGolfCaddie::UpdatePendingOperation() {
    if (petPreloadActive == 0)
        return 0;
    EC8VirtualObject* operation = (EC8VirtualObject*)petPreload;
    operation->update();
    operation = (EC8VirtualObject*)petPreload;
    int result = operation->complete();
    if (result != 0)
        petPreloadActive = 0;
    int expected = petPreloadPlayerIndex;
    operation = (EC8VirtualObject*)(*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    int actual = AsCGolfDoc(operation)->currentPlayer;
    return expected == actual;
}

// Rotate shared lighting into the caddie bone frame and enable the resulting render parameters.
void CGolfCaddie::UpdateBoneRelativeRenderParams(void* puppet) {
    if (puppet == 0)
        return;
    char* base = golfCaddieStrings;
    void* entry = WPuppet_FindNamedBoneEntry(puppet, base + kCaddieStrings_spineBone);
    if (entry == 0)
        entry = WPuppet_FindNamedBoneEntry(puppet, base + kCaddieStrings_fallbackBone);
    char expandedMatrix[0x40];
    Matrix_ExpandAffineTo4x4((char*)*(void**)entry + 0x78, expandedMatrix);
    struct Transform {
        char data[0x68];
    };
    Transform copied;
    void* vector = ModelRenderParams_GetShared();
    Transform first;
    ModelRenderParams_Copy(&first, vector);
    copied = first;
    Mtx44_TransformDirection(copied.data + 0x40, copied.data + 0x40, expandedMatrix);
    char rotation[0x40];
    Mtx44_SetRotationY(rotation, kFloat_neg1_5707964_CaddieRotation);
    Mtx44_TransformDirection(copied.data + 0x40, copied.data + 0x40, rotation);
    Vec3_Normalize(copied.data + 0x40, copied.data + 0x40);
    WPuppet_SetRenderParams((void*)puppet, &copied);
    WPuppet_SetRenderParamsEnabled((void*)puppet, 1);
}

struct Node_80130C2C {
    Node_80130C2C* left;
    Node_80130C2C* right;
    char parentAndKeyBytes[4];
    int key;
};

struct IntStringTreeHeader {
    char padding000[4];
    Node_80130C2C* root;
};

struct WalkNode {
    WalkNode* child[2];
    char parentAndKeyBytes[8];
    unsigned int flags;
    char lengthBytes[4];
    void* owned;
};
// Queue node ABI: children +0/+4, tagged parent +8, player index +0xC,
// 12-byte small string +0x10 (heap pointer +0x18). The header is embedded:
// count/root/cached-begin; empty begin points at the root field, not a node.
// Tree owns nodes/heap filenames; modelHolders are borrowed, never freed here.

// Destroy one detached queue node after its children have been released.
inline void DestroyCaddieFilenameNode(WalkNode* node) {
    if (((char*)node + 0xC) && ((char*)node + 0x10) && (node->flags >> 31))
        appFreeUni__FPv(node->owned);
    appFreeUni__FPv(node);
}

// Carry the allocator scratch byte; it owns no storage.
struct StringAllocator {
    char byte;
    StringAllocator() {}
};

struct LocalString {
    struct Representation {
        unsigned int flags, length;
        char* buffer;
    };
    union {
        struct {
            unsigned int flags, length;
            char* buffer;
        };
        Representation rep;
    };
    // Create an empty inline string.
    LocalString() : flags(0), length(0), buffer(0) {}
    // Free only the owned heap representation.
    ~LocalString() {
        if (flags >> 31)
            appFreeUni__FPv(buffer);
    }
    // Test the heap tag in the first word.
    unsigned int heap() const {
        return flags >> 31;
    }
    // Return text length in bytes, decoding inline or heap storage.
    unsigned int size() const {
        if ((flags >> 31) == 0)
            return *(const unsigned char*)this & 0x7F;
        return length;
    }
    // Borrow text bytes until the next mutation or destruction.
    const char* data() const {
        if ((flags >> 31) == 0)
            return (const char*)this + 1;
        return buffer;
    }
    // Replace a byte range, copying the supplied half-open source range.
    void replace(int pos, int count, const char* first, const char* last, StringAllocator alloc) {
        String_ReplaceRange(this, pos, count, first, last, &alloc.byte);
    }
    // Replace a byte range using a borrowed NUL-terminated string.
    void replace_text(int pos, int count, const char* const& text) {
        replace(pos, count, text, text + strlen(text), StringAllocator());
    }
    // Replace all text with a copy of the supplied string.
    void assign(const char* text) {
        replace_text(0, size(), text);
    }
    // Append a copy of the supplied string.
    void append(const char* text) {
        replace_text(size(), 0, text);
    }
    // Copy inline bytes directly, or deep-copy the owned heap text.
    LocalString(const LocalString& other) {
        if (!other.heap()) {
            rep = other.rep;
        } else {
            flags = 0;
            length = 0;
            buffer = 0;
            String_ReserveCapacity(this, 0);
            replace(0, 0, other.buffer, other.buffer + other.length, StringAllocator());
        }
    }
    // Assign without sharing the source heap buffer.
    LocalString& operator=(const LocalString& other) {
        if (!heap() && !other.heap()) {
            rep = other.rep;
        } else {
            unsigned int n;
            if (heap())
                n = length;
            else
                n = *(const unsigned char*)this & 0x7F;
            const char* p;
            unsigned int len;
            if (!other.heap()) {
                p = (const char*)&other + 1;
                len = *(const unsigned char*)&other & 0x7F;
            } else {
                p = other.buffer;
                len = other.length;
            }
            replace(0, n, p, p + len, StringAllocator());
        }
        return *this;
    }
};

struct LocalPair {
    int key;
    LocalString value;
    // Copy an indexed filename into an insertion value.
    LocalPair(int k, const LocalString& s) : key(k), value(s) {}
};

struct D8Context {
    char padding000[0x20];
    void** items;
    unsigned int count;
    char padding028[0x15C];
    int state;
    int treeCount;
    WalkNode* root;
    int sentinelWord;
    WalkNode* begin;
};

// Destroy queued string nodes in postorder; deeper subtrees use the shared tree destructor.
inline void clear_level1(void* self, WalkNode* node) {
    if (node->child[0])
        IntStringTree_DestroySubtree((char*)self + kLoadTreeOffset, node->child[0]);
    if (node->child[1])
        IntStringTree_DestroySubtree((char*)self + kLoadTreeOffset, node->child[1]);
    DestroyCaddieFilenameNode(node);
}
// Clear two expanded tree levels, freeing children before their parent.
inline void clear_level2(void* self, WalkNode* node) {
    WalkNode* child = node->child[0];
    if (child)
        clear_level1(self, child);
    child = node->child[1];
    if (child)
        clear_level1(self, child);
    DestroyCaddieFilenameNode(node);
}
// Clear three expanded tree levels, freeing children before their parent.
inline void clear_level3(void* self, WalkNode* node) {
    WalkNode* child = node->child[0];
    if (child)
        clear_level2(self, child);
    child = node->child[1];
    if (child)
        clear_level2(self, child);
    DestroyCaddieFilenameNode(node);
}

// Return an existing indexed filename or insert an empty string for the new key.
inline LocalString& find_string(IntStringTreeHeader* tree, int& index) {
    Node_80130C2C* key;
    unsigned char less, found;
    void* at = IntStringTree_FindInsertPosition(tree, &index, &key, &less, &found);
    LocalPair* result;
    if (key)
        result = (LocalPair*)((char*)key + kTreeValueOffset);
    if (!key || result->key < index) {
        const LocalString& empty = LocalString();
        LocalPair value(index, empty);
        result = (LocalPair*)((char*)IntStringTree_InsertAtPosition(tree, at, less, found, &value) + 0xC);
    }
    return result->value;
}

// Rebuild indexed caddie model filenames from story mode and equipped caddie items.
void CGolfCaddie::RebuildModelLoadQueue() {
    D8Context* self = (D8Context*)this;
    // Phase 1: discard every old filename/node and restore the empty sentinel.
    WalkNode* root = self->root;
    if (root) {
        WalkNode* first = root->child[0];
        if (first) {
            clear_level3(self, first);
        }
        first = root->child[1];
        if (first) {
            clear_level3(self, first);
        }
        if (((char*)root + kTreeValueOffset) && ((char*)root + kTreeStringOffset) && (root->flags >> 31))
            appFreeUni__FPv(root->owned);
        appFreeUni__FPv(root);
        self->treeCount = 0;
        self->root = 0;
        self->begin = (WalkNode*)&self->root;
    }
    if (self->count == 0)
        return;
    // Phase 2: choose each model name. Story mode appends .story.pet; otherwise
    // an equipped item's model overrides the base name when inventory lookup
    // succeeds, with .pet fallback. Copy into an index-keyed owned tree string.
    LocalString local;
    char* global = golfCaddieStrings;
    unsigned int index = 0;
    for (; index < self->count; ++index) {
        void* item = self->items[index];
        if (item) {
            const char* label = ((CaddieHolderView*)item)->caddieData->model;
            local.assign(label);
            if (self->state) {
                local.append(global + kCaddieStrings_storyModelSuffix);
            } else if (!((CaddieHolderView*)item)->equippedItem)
                local.append(global + kCaddieStrings_modelSuffix);
            else {
                IFF_STRUCT::sCadItem* resource = (IFF_STRUCT::sCadItem*)ItemInventory_FindCaddieItemData(
                    (char*)AsCSharedDoc(g_sharedDoc)->itemInventory,
                    *(void**)((char*)((CaddieHolderView*)item)->equippedItem + 8));
                if (resource)
                    local.assign(resource->model);
                else
                    local.append(global + kCaddieStrings_modelSuffix);
            }
            LocalString* destination = &find_string((IntStringTreeHeader*)((char*)self + kLoadTreeOffset), (int&)index);
            *destination = local;
        }
    }
    // Phase 3: start only the first indexed load; polling starts each successor.
    // Filename bytes stay tree-owned through completion. Whether the request
    // copies or retains that pointer internally is not established in this unit.
    if (!self->treeCount)
        return;
    {
        void* cur = self->begin;
        void* node = self->items[*(unsigned int*)&AsIntStringTreeNode8013(cur)->key];
        ((CPetBody*)(*(void**)node))->RequestModelLoad((void*)((LocalString*)((char*)cur + kTreeStringOffset))->data());
        // Ensure the shared shadow render packet, then register the caddie shadow.
        if (!((int&)Sing_805DC7B0())) {
            if (!s_renderPacketInitialized) {
                RenderPacket_Init(s_renderPacket);
                __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                         s_renderPacketDtorRegistration_804F68E8);
                s_renderPacketInitialized = 1;
            }
            ((int&)Sing_805DC7B0()) = (int)s_renderPacket;
        }
        GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(
            g_shadowManager, golfCaddieStrings + kCaddieStrings_caddieShadowName, kFloat_0_09_CaddieShadow, 0x10,
            golfCaddieStrings + kCaddieStrings_sunLight, (char*)((int&)Sing_805DC7B0()) + 0x4C);
    }
}

// Find the insertion parent and predecessor for an integer key in the model-load tree.
Node_80130C2C* IntStringTree_FindInsertPosition(IntStringTreeHeader* root, int* key, Node_80130C2C** out,
                                                unsigned char* less, unsigned char* found) {
    unsigned char zero = 0;
    unsigned char one = 1;
    *out = 0;
    Node_80130C2C* parent = (Node_80130C2C*)((char*)root + 4);
    Node_80130C2C* node = root->root;
    *less = one;
    *found = one;
    while (node != 0) {
        int value = *key;
        parent = node;
        if (value < node->key) {
            node = node->left;
            *less = one;
        } else {
            *out = node;
            node = node->right;
            *less = zero;
            *found = zero;
        }
    }
    return parent;
}

} // extern "C"
