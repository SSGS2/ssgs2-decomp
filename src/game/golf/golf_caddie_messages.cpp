// Caddie actor messages: placement/following, aim-dependent speech, scene/shadow
// visibility, and asynchronous player-body/Conte loading. Geometry and animation
// settings are in golfCaddieStrings/caddieActionDescriptors and entry metadata;
// lifecycle/update consumers are golf_caddie_animation_process.cpp and caddie_situational_commentary.cpp.
// Mod lookup: placement radius/slope -> CaddieGeometry and kCaddiePlacement*;
// follow stand-off/root-motion scaling -> kCaddieFollowBall case;
// aim updates -> kCaddieSetAimYaw/kCaddieAdvanceAimYaw; model lifetime ->
// kCaddieDeletePlayerBody and begin/poll/finish player-body cases below.
// Keep nested helpers inline when the placement/preload stages add an inline level.
#pragma always_inline on
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
#include "classes/CPetBody.h"
#include "classes/CClubRingEffect.h"

extern "C" void PlayAnimationByName__8CPetBodyFfPvii(void*, void*, int, int, float);

#include "classes/GolfDocSnapshot8013.h"
// Golf-document and caddie offsets used by CGolfCaddie::HandleMsg.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CQuadTree.h"
extern "C" {
#include "sdk_protos.h"
#include "classes/CGolfCaddie.h"
#include "classes/CPreloadPetAsync.h"
#include "classes/CFx.h"
#include "classes/CShadowManager.h"

#define AsCGolfCaddie(p) ((CGolfCaddie*)(p))

typedef Vec3f V3;

typedef struct Str {
    unsigned int w0;
    int len;
    char* ptr;
} Str;

struct CaddieLayout {
    char prefix[0x48];
    float angle;
};

// Handler payload/ownership map (argument indices are zero-based):
// 2: optional named course point char*; 3: nonnull word reuses cached placement;
// 4: target pointer + integer look-at mode; 6: writable Vec3* result;
// 8: preview model pointer + difficulty integer clamped to [0,3];
// 11: commentary action integer; 15: hierarchy context pointer;
// 16/19/30: mode integers; 17: writable entry pointer* (borrowed result);
// 18/20/0x2717: float* input (aim yaw delta/body yaw/absolute aim yaw, radians);
// 21: world position pointer + animation name pointer;
// 22: two inventory indices, checked without a resulting mutation;
// 23/24: player index; 25/31: writable int* completion result;
// 27: writable int* result + player index (true also for an absent entry);
// 0x2715/0x2716: animation name char*; 0x2718: world Vec3* input.
// Message words are not validated here. Position inputs are copied; returned
// entries are borrowed. Body/preload deletion remains with the caddie actor.
// All positions are world-space, Y up; rotation inputs are radians.
// +0x68 is AIM YAW in radians, not a commentary timer: UpdateSituationalCommentary
// (caddie_situational_commentary.cpp) feeds it to Matrix_SetAxisRotation(Y, angle). Message 18 adds an angle
// and re-arms the one-shot aim comment at +0x16D; 0x2717 replaces the angle.
// Neither converts elapsed time. The separate wind timer at +0x6C accumulates
// OnProcess's seconds; commentaryTimestampMs is the global millisecond clock.
// Message arguments share a 32-bit ABI word; callers choose pointer, integer, or float payloads.
union CaddieMessageArgument {
    void* pointer;
    int integer;
    unsigned int bits;
    float real;
};
struct CaddieMessage {
    void* sender;
    int type;
    CaddieMessageArgument arguments[6];
};
struct CaddieAnimationNode {
    void* left;
    void* right;
    unsigned int parentAndColor;
    unsigned int nameFlags;
    unsigned int nameLength;
    void* nameData;
    unsigned char minimumDifficulty;
};
struct CaddieEntryMessageView {
    void* body;
    char prefix4[0x10];
    void* playerData;
    char prefix18[0x14];
    void* animationRestrictionRoot;
    char prefix30[0xC];
    float followStartDelay;
    float followSpeed;
    unsigned char textureEnabled;
};
// Metadata producer: CGolfCaddie_ParseAnimationMetadata (golf_caddie_animation_process.cpp, 0x8012A730)
// reads run-key text tokens *start_delay and *pet_speed into +0x3C/+0x40 and scales
// pet speed by 3.2 world-distance units per authored unit. OnProcess subtracts
// elapsed seconds from the start delay. Keep that conversion with the follow
// formula; an authored speed label alone does not establish ball+0xA4's units.
struct CaddieCommentaryState {
    unsigned char currentAction;
    unsigned char actionFlags[36];
    unsigned char aimCommentIssued;
    char padding[2];
    unsigned int commentaryTimestampMs;
};
struct CaddieCommentaryView {
    char prefix[0x148];
    CaddieCommentaryState commentary;
};
struct CaddieActionDescriptor {
    int action;
    unsigned char flags;
    char padding[3];
};
// Borrow the actor layout view; callers may update the commentary subobject's flags/timestamp.
inline CaddieCommentaryView& CaddieView(void* actor) {
    return *(CaddieCommentaryView*)actor;
}
struct CaddiePlayerMessageView {
    void* rank;
    void* model;
    void* inventory;
    unsigned char difficulty;
};
typedef char
    CaddieAnimationNode_difficultyOffset[((int)&((CaddieAnimationNode*)0)->minimumDifficulty == 0x18) ? 1 : -1];
typedef char
    CaddieEntryMessageView_rootOffset[((int)&((CaddieEntryMessageView*)0)->animationRestrictionRoot == 0x2C) ? 1 : -1];
#define CADDIE_NODE(p) ((CaddieAnimationNode*)(p))
#define CADDIE_ENTRY(p) ((CaddieEntryMessageView*)(p))
#define CADDIE_FIELD_OFFSET(field) ((int)&((CGolfCaddie*)0)->field)
// Affine attachment matrix: three basis vectors followed by world translation.
// Storage belongs to the caddie actor, one stable 0x30-byte slot per player.
struct CaddieAttachmentMatrix {
    V3 basisX, basisY, basisZ, translation;
};
struct CaddieEffectBone {
    char prefix[0x78];
    CaddieAttachmentMatrix worldMatrix;
};
struct CaddieEffectBoneMetadata {
    char prefix[0x18];
    V3 point;
};
struct CaddieEffectNamedBone {
    CaddieEffectBone* bone;
    CaddieEffectBoneMetadata* metadata;
};
struct CaddieEffectDescriptor {
    float screenX, screenY;
    void* transforms[4];
    void* points[4];
};
enum CaddieGeometry {
    kCaddieYawAxis = 1,
    kCaddiePlacementCandidates = 4,
    kCaddiePlayerRecordBytes = 0x1D4,
    kCaddieAttachmentMatrixBytes = 0x30,
    kCaddieMaximumDifficulty = 3,
    kCaddieShadowEntryKind = 16
};
static const float kCaddieAttachmentYawRadians = -1.5707964f; // -pi/2 about world Y
static const float kCaddiePlacementSideRadians = 0.6981317f;  // 40 degrees
static const float kCaddieHalfTurnRadians = 3.1415927f;
static const float kCaddiePlacementRadius = 22.4f; // world units from ball
static const float kCaddiePlacementHalfExtent = 16.0f;
static const float kCaddieMaximumSlopeRadians = 0.34906584f; // 20 degrees
static const float kCaddieObstacleRadius = 1.6f;
typedef char CaddieEffectDescriptor_size[(sizeof(CaddieEffectDescriptor) == 0x28) ? 1 : -1];
typedef char CaddieAttachmentMatrix_size[(sizeof(CaddieAttachmentMatrix) == kCaddieAttachmentMatrixBytes) ? 1 : -1];

enum CaddieMessageCode {
    kCaddieReactToShot = 1,
    kCaddieFollowBall = 2,
    kCaddiePlaceCurrentPlayer = 3,
    kCaddieSetLookAt = 4,
    kCaddieClearLookAt = 5,
    kCaddieGetPosition = 6,
    kCaddiePlaySound = 7,
    kCaddiePreview = 8,
    kCaddieNoOp = 9,
    kCaddieResetCommentary = 10,
    kCaddieTriggerCommentary = 11,
    kCaddieFinishCommentary = 12,
    kCaddieShow = 13,
    kCaddieHide = 14,
    kCaddieSetHierarchy = 15,
    kCaddieSetReactionValue = 16,
    kCaddieGetActiveEntry = 17,
    kCaddieAdvanceAimYaw = 18,
    kCaddieSetReactionMode = 19,
    kCaddieSetYaw = 20,
    kCaddiePlaceAtMessagePosition = 21,
    kCaddieCheckInventoryNoOp = 22,
    kCaddieDeletePlayerBody = 23,
    kCaddieBeginPlayerBodyLoad = 24,
    kCaddieIsPlayerBodyLoadDone = 25,
    kCaddieFinishPlayerBodyLoad = 26,
    kCaddieBodyReadyOrNoEntry = 27,
    kCaddieActivateAllConte = 28,
    kCaddieClearConteModels = 29,
    kCaddieSetConteState = 30,
    kCaddiePollConteLoads = 31,
    kCaddiePlayAnimation = 0x2715,
    kCaddieQueueAnimation = 0x2716,
    kCaddieSetAimYaw = 0x2717,
    kCaddieSetPosition = 0x2718
};
// The restriction tree stores byte-string keys at +0xC and a minimum difficulty
// byte at +0x18; absent keys permit playback. Parent links pack a color bit.
// Entry/body/texture views below borrow actor-owned storage.
enum {
    kSharedPlayerSlotsOffset = 0x1B4,
    kFollowSampleBias = 90
};
// Follow target stops this many world units short of the last trajectory sample.
#define kFollowStandOffDistance 9.6f
// Legacy world-distance correction per excess sample; sample cadence is not established here.
#define kFollowDistancePerSample 0.6f
enum CaddieStringOffset {
    kCaddieSceneName = 0,
    kCaddieConteNameFormat = 0xB,
    kCaddieShadowGroup = 0x20,
    kCaddieShadowTexture = 0x27,
    kCaddieRunAnimation = 0x84,
    kCaddieIdleAnimation = 0xE5,
    kCaddieMoveQueryTarget = 0x10A,
    kCaddieEffectBone = 0x123,
    kCaddieAttachedEffect = 0x12B,
    kCaddieReactionSound = 0x136
};

void IActor_DispatchMessage(void*, void*, void*, int, void*, int, int, int);
void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void* CourseData_FindNamedPoint(void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
int CQuadTree_OverlappedWithObject(void*, void*, float);
float Math_CosApprox(float);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __ml__FfRC7WVector(void*, void*, float);
void __as__7WVectorFRC7WVector(void*, void*);
void __apl__7WVectorFRC7WVector(void*, void*);
void Vec3_Zero(void*);
float Magnitude__7WVectorCFv(void*);
void* Normalize__7WVectorFv(void*);
void Matrix_CopyAffine(void*, void*);
void Matrix_ConcatAffineAssign(void*, void*);
void Matrix_SetAxisRotation(void*, float, int);
void AABB_Copy(void*, void*);
void String_ReserveCapacity(void*, int);
void String_ReplaceRange(void*, int, int, const char*, const char*, void*);
void appFreeUni__FPv(void*);
void* Object_GetChild(void*);
void* FxInterpreter_GetInstance();
void FxInterpreter_ClearEntries(void*);
void FxInterpreter_RemoveEntriesByKey(void*, void*);
void* FxAttachments_CreateFromResourceTags(void*, void*, int);
int SharedDoc_IsCourse8Or13();
void FxAttachment_Init(void*);
void* WPuppet_FindNamedBoneEntry(void*, void*);
void WPuppet_SetHierarchyContextRecursive(void*, void*);
void DrawScene_SetNamedObjectEnabled(void*, void*, int);
void DrawScene_SetNamedObjectPuppet(void*, void*, void*);
void ShadowModel_SetNamedProjectionEnabled(void*, void*, int);
void GfxObjList_RemoveGroupByName(void*, void*);
extern "C" void ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(void*, void*, void*);
void GfxObjList_RemoveEntryFromNamedGroup(void*, void*, void*);
extern "C" void GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF(void*, void*, float, int, void*, void*);
void Sound_PlayByName(void*, void*, int, int, int, int, float, float, float);
void* GolfPlayer_GetEntryByIndex(void*, int);
int ItemInventory_FindCaddieData(void*, int);
int ItemInventory_FindCaddieItemData(void*, int);
void* String_FormatTemporary(const char*, ...);
void CourseSlotTable_InitEmpty(void*);
void* CourseSlotTable_GetExtraEntry(void*, int);
void RenderPacket_Init(void*);
void RenderPacket_Destroy();

extern char golfCaddieStrings[];
extern char caddieActionDescriptors[];
extern char s_renderPacket[];
extern char s_renderPacketDtorRegistration_804F68E8[];
extern char s_conteModelSlots[];
extern char s_golfTaskTypeInfo;
extern void* g_sharedDoc;
extern void* g_sceneManager;
extern void* g_fx;
extern void* g_shadowManager;
extern void* g_cameraWork;
extern void* g_taskManager;
extern void* g_cachedTaskChildForGolfLookup;
extern void* g_soundManager;
extern char s_renderPacketInitialized;
extern char s_courseSlotTableInitialized;
extern unsigned int g_currentTimeMilliseconds;

// Borrow an ABI pointer word at an unresolved layout offset; preserve lvalue stores.
inline void*& CaddiePointerAt(void* object, int offset) {
    return *(void**)((unsigned char*)object + offset);
}
// Borrow a signed ABI word at an unresolved layout offset.
inline int& CaddieIntAt(void* object, int offset) {
    return *(int*)((unsigned char*)object + offset);
}
// Borrow an unsigned ABI word at an unresolved layout offset.
inline unsigned int& CaddieWordAt(void* object, int offset) {
    return *(unsigned int*)((unsigned char*)object + offset);
}
// Borrow a floating-point field at an unresolved layout offset.
inline float& CaddieFloatAt(void* object, int offset) {
    return *(float*)((unsigned char*)object + offset);
}
// Borrow a byte flag at an unresolved layout offset.
inline unsigned char& CaddieByteAt(void* object, int offset) {
    return *(unsigned char*)((unsigned char*)object + offset);
}

// Reset all commentary action flags byte by byte, retaining the original store order.
inline void ResetCaddieCommentary(unsigned char* obj) {
    CaddieView(obj).commentary.currentAction = 0;
    CaddieView(obj).commentary.actionFlags[0] = 0;
    CaddieView(obj).commentary.actionFlags[1] = 0;
    CaddieView(obj).commentary.actionFlags[2] = 0;
    CaddieView(obj).commentary.actionFlags[3] = 0;
    CaddieView(obj).commentary.actionFlags[4] = 0;
    CaddieView(obj).commentary.actionFlags[5] = 0;
    CaddieView(obj).commentary.actionFlags[6] = 0;
    CaddieView(obj).commentary.actionFlags[7] = 0;
    CaddieView(obj).commentary.actionFlags[8] = 0;
    CaddieView(obj).commentary.actionFlags[9] = 0;
    CaddieView(obj).commentary.actionFlags[10] = 0;
    CaddieView(obj).commentary.actionFlags[11] = 0;
    CaddieView(obj).commentary.actionFlags[12] = 0;
    CaddieView(obj).commentary.actionFlags[13] = 0;
    CaddieView(obj).commentary.actionFlags[14] = 0;
    CaddieView(obj).commentary.actionFlags[15] = 0;
    CaddieView(obj).commentary.actionFlags[16] = 0;
    CaddieView(obj).commentary.actionFlags[17] = 0;
    CaddieView(obj).commentary.actionFlags[18] = 0;
    CaddieView(obj).commentary.actionFlags[19] = 0;
    CaddieView(obj).commentary.actionFlags[20] = 0;
    CaddieView(obj).commentary.actionFlags[21] = 0;
    CaddieView(obj).commentary.actionFlags[22] = 0;
    CaddieView(obj).commentary.actionFlags[23] = 0;
    CaddieView(obj).commentary.actionFlags[24] = 0;
    CaddieView(obj).commentary.actionFlags[25] = 0;
    CaddieView(obj).commentary.actionFlags[26] = 0;
    CaddieView(obj).commentary.actionFlags[27] = 0;
    CaddieView(obj).commentary.actionFlags[28] = 0;
    CaddieView(obj).commentary.actionFlags[29] = 0;
    CaddieView(obj).commentary.actionFlags[30] = 0;
    CaddieView(obj).commentary.actionFlags[31] = 0;
    CaddieView(obj).commentary.actionFlags[32] = 0;
    CaddieView(obj).commentary.actionFlags[33] = 0;
    CaddieView(obj).commentary.actionFlags[34] = 0;
    CaddieView(obj).commentary.actionFlags[35] = 0;
    CaddieView(obj).commentary.aimCommentIssued = 0;
}

typedef void* (*VF)(void*);
typedef void (*VF2)(void*, int);

class KindObject {
public:
    virtual void* kind();
};

class DeleteObject {
public:
    virtual ~DeleteObject();
};

// Return a caddie entry by player index, or null when the index is outside the entry array.
inline static void* getAt(unsigned char* obj, int idx) {
    if (idx < 0 || (unsigned int)idx >= AsCGolfCaddie(obj)->caddieEntryCount)
        return 0;
    return ((void**)(*(void**)&AsCGolfCaddie(obj)->caddieEntries))[idx];
}

// Return the lesser byte count by reference, retaining the original comparison shape.
inline static const unsigned& rmin(const unsigned& a, const unsigned& b) {
    return a < b ? a : b;
}

// Preserve the stored animation-key length without changing its unsigned type.
inline static unsigned keyLen(unsigned v) {
    return v;
}

// Read the animation key length from its inline/heap string representation.
inline static unsigned nodeLength(void* node) {
    return !(CaddieWordAt(node, 0xc) >> 31) ? CaddieByteAt(node, 0xc) & 0x7f : CaddieWordAt(node, 0x10);
}

#pragma opt_lifetimes off
// Compare byte ranges lexicographically, using length to break equal-prefix ties.
inline static int compareRange(const void* leftBytes, unsigned leftLength, const void* rightBytes,
                               const unsigned& rightLength) {
    unsigned length = rightLength;
    int cmp = memcmp(leftBytes, rightBytes, rmin(length, leftLength));
    if (cmp == 0) {
        if (leftLength < rightLength)
            return -1;
        if (leftLength == rightLength)
            return 0;
        return 1;
    }
    return cmp;
}
#pragma opt_lifetimes reset

// Compare two animation names without assuming null termination.
inline static int cmpKey(const char* leftBytes, unsigned leftLength, const char* rightBytes, unsigned rightLength) {
    unsigned n = rmin(rightLength, rmin(keyLen(leftLength), keyLen(leftLength)));
    int c = memcmp(leftBytes, rightBytes, n);
    if (c == 0) {
        if (leftLength < rightLength)
            return -1;
        if (leftLength == rightLength)
            return 0;
        return 1;
    }
    return c;
}

// Stateless allocator token passed by address to the string library; it carries no lookup state.
struct QueryAllocator {
    char c;
};

// Build the temporary string key used for the animation restriction lookup.
inline static void makeQuery(unsigned* query, const char* name, const QueryAllocator& alloc) {
    unsigned int length = strlen(name);
    String_ReserveCapacity(query, length);
    QueryAllocator local = alloc;
    String_ReplaceRange(query, 0, 0, name, name + length, &local);
}

// Allow an animation unless its restriction entry requires a higher player difficulty.
inline static int CaddieAnimationIsAllowed(void* owner, void* self, const char* name) {
    unsigned int query[3] = {};
    QueryAllocator alloc;
    makeQuery(query, name, alloc);
    void* cand = (void*)&CADDIE_ENTRY(owner)->animationRestrictionRoot;
    void* node = CADDIE_ENTRY(owner)->animationRestrictionRoot;
    unsigned int isLong = query[0] >> 31;
    unsigned int qbyte = *(unsigned char*)query & 0x7f;
    void* qptr = (void*)query[2];
    unsigned int qsize = query[1];
    int isShort = (isLong == 0);
    int heap = (isShort == 0);
    while (node) {
        unsigned int qlen[1];
        const void* qdata;
        if (!heap) {
            qlen[0] = qbyte;
            qdata = (char*)query + 1;
        } else {
            qdata = qptr;
            qlen[0] = qsize;
        }
        unsigned int nl = nodeLength(node);
        const void* nd;
        unsigned int nl2;
        if (!(CADDIE_NODE(node)->nameFlags >> 31)) {
            nd = (char*)&CADDIE_NODE(node)->nameFlags + 1;
            nl2 = *(unsigned char*)&CADDIE_NODE(node)->nameFlags & 0x7f;
        } else {
            nd = CADDIE_NODE(node)->nameData;
            nl2 = CADDIE_NODE(node)->nameLength;
        }
        unsigned int t = rmin(nl, keyLen(nl2));
        int cmp = compareRange(nd, t, qdata, qlen[0]);
        if (cmp >= 0) {
            cand = node;
            node = *(void**)node;
        } else
            node = CADDIE_NODE(node)->right;
    }
    if (cand != (void*)&CADDIE_ENTRY(owner)->animationRestrictionRoot) {
        unsigned int nl[1];
        const void* nd;
        if (!(CADDIE_NODE(cand)->nameFlags >> 31)) {
            nd = (char*)&CADDIE_NODE(cand)->nameFlags + 1;
            nl[0] = *(unsigned char*)&CADDIE_NODE(cand)->nameFlags & 0x7f;
        } else {
            nd = CADDIE_NODE(cand)->nameData;
            nl[0] = CADDIE_NODE(cand)->nameLength;
        }
        unsigned int q1 = !heap ? qbyte : query[1];
        unsigned int q2;
        const void* qdata;
        if (!heap) {
            qdata = (char*)query + 1;
            q2 = qbyte;
        } else {
            qdata = (void*)query[2];
            qbyte = query[1];
            q2 = qbyte;
        }
        unsigned int t = rmin(q1, keyLen(q2));
        int cmp = compareRange(qdata, t, nd, nl[0]);
        if (cmp < 0)
            cand = (void*)&CADDIE_ENTRY(owner)->animationRestrictionRoot;
    }
    if (isLong)
        appFreeUni__FPv((void*)query[2]);
    if (cand == (void*)&CADDIE_ENTRY(owner)->animationRestrictionRoot)
        return 1;
    void* sel = CADDIE_ENTRY(AsCGolfCaddie(self)->activeCaddie)->playerData;
    if (sel) {
        if (((CaddiePlayerMessageView*)sel)->difficulty < CADDIE_NODE(cand)->minimumDifficulty)
            return 0;
    } else {
        if (CADDIE_NODE(cand)->minimumDifficulty)
            return 0;
    }
    return 1;
}

// Walk runtime base-type descriptors to identify a golf task.
inline static int isKind(void* o) {
    if (o) {
        void* n = ((KindObject*)o)->kind();
        while (n) {
            if (n == &s_golfTaskTypeInfo) {
                return 1;
            }
            n = CaddiePointerAt(n, 4);
        }
        return 0;
    }
    return 0;
}

// Return the active golf-task player index, or the invalid-player sentinel.
inline static int CurrentGolfPlayerIndex() {
    void* o = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_cachedTaskChildForGolfLookup = o;
    if (isKind(o))
        return ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
    return 0xff;
}

// Return the shared player record selected by the active golf task.
inline static void* CurrentGolfPlayerData() {
    void* taskChild = g_taskManager ? Object_GetChild(g_taskManager) : 0;
    g_cachedTaskChildForGolfLookup = taskChild;
    int kind = isKind(taskChild);
    void* sharedDoc = g_sharedDoc;
    int idx = kind ? ((CGolfDoc*)AsCSharedDoc(sharedDoc)->golfDoc)->currentPlayer : 0xff;
    return (unsigned char*)sharedDoc + idx * kCaddiePlayerRecordBytes + 0x1b4;
}

// Position the caddie texture above the model when the texture and entry flag are active.
inline static void UpdateCaddieTexturePosition(unsigned char* obj, unsigned char* x) {
    if ((*(int*)&AsCGolfCaddie(obj)->texture3D) && CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->textureEnabled) {
        WVector t;

        t.x = 0.0f;
        t.y = 0.3f;
        t.z = 0.0f;
        WVector o = *(WVector*)&AsCGolfCaddie(obj)->position + t;
        ((CClubRingEffect*)(AsCGolfCaddie(obj)->texture3D))->SetTranslation(&o);
    }
}

// Update the caddie texture position and mark the texture enabled.
inline static void UpdateCaddieTextureAndEnable(unsigned char* obj, unsigned char* x) {
    if ((*(int*)&AsCGolfCaddie(obj)->texture3D) && CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->textureEnabled) {
        WVector t;

        t.x = 0.0f;
        t.y = 0.3f;
        t.z = 0.0f;
        WVector o = *(WVector*)&AsCGolfCaddie(obj)->position + t;
        ((CClubRingEffect*)(AsCGolfCaddie(obj)->texture3D))->SetTranslation(&o);
        CaddieByteAt(AsCGolfCaddie(obj)->texture3D, 8) = 1;
    }
}

// Try four grounded placements around the ball; accept the first with ground,
// slope <= 20 degrees and no nearby object-bound collision. If none qualifies,
// keep the last attempted placement. This does not test camera line of sight.
inline static void FindClearCaddiePlacement(unsigned char* obj) {
    // Candidate yaw = shot heading +/- 40 degrees, then the opposite pair.
    // The model is placed 22.4 world units from the ball and turned back by pi.
    for (int i = 0; i < kCaddiePlacementCandidates; i++) {
        char yawMatrix[0x30];
        void* golfDoc = (void*)AsCSharedDoc(g_sharedDoc)->golfDoc;
        WVector groundedWorld;
        V3 v144;
        WVector groundedCopy;
        V3 v174, groundNormal;
        WVector v18c, v1b0;
        float sideSign = (i & 1) ? -1.0f : 1.0f;
        float oppositeYaw = (i & 2) ? kCaddieHalfTurnRadians : 0.0f;
        ((CaddieLayout*)obj)->angle =
            oppositeYaw + (kCaddiePlacementSideRadians * sideSign + ((CGolfDoc*)golfDoc)->headingRadians);
        Matrix_SetAxisRotation(yawMatrix, ((CaddieLayout*)obj)->angle, kCaddieYawAxis);
        WVector ballOffsetWorld = kCaddiePlacementRadius * *(WVector*)(yawMatrix + 0x18);
        WVector candidateWorld = *(WVector*)(GolfBall__Fv()) + ballOffsetWorld;
        *(Arr3*)&v144 = *(Arr3*)&candidateWorld;
        GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundedWorld, CQuadTree_GetInstance(), &v144, 1, 0, 0);
        *(WVector*)&AsCGolfCaddie(obj)->position = groundedWorld;
        ((CaddieLayout*)obj)->angle += kCaddieHalfTurnRadians;
        *(Arr3*)&groundedCopy = *(Arr3*)&AsCGolfCaddie(obj)->position;
        // Query an XZ box with 16-unit half extents; Y remains at the grounded point.
        V3 seg[2];
        Vec3_Zero(&seg[0]);
        Vec3_Zero(&seg[1]);
        v1b0.x = 1.0f;
        v1b0.y = 0.0f;
        v1b0.z = 1.0f;
        WVector v1bc = kCaddiePlacementHalfExtent * v1b0;
        WVector v1c8 = groundedCopy - v1bc;
        *(WVector*)(&seg[0]) = v1c8;
        v18c.x = 1.0f;
        v18c.y = 0.0f;
        v18c.z = 1.0f;
        WVector v198 = kCaddiePlacementHalfExtent * v18c;
        WVector v1a4 = groundedCopy + v198;
        *(WVector*)(&seg[1]) = v1a4;
        V3 cp[2];
        *(Arr3*)&cp[0] = *(Arr3*)&seg[0];
        *(Arr3*)&cp[1] = *(Arr3*)&seg[1];
        ((CQuadTree*)CQuadTree_GetInstance())->GetObjList((float*)cp);
        // Accept ground only when its normal meets the slope bound and a 1.6-unit object test passes.
        int placementAccepted;
        if (((CQuadTree*)CQuadTree_GetInstance())->GetMeshIndex(&groundedCopy, 1) < 0) {
            placementAccepted = 0;
        } else {
            float minimumNormalY = Math_CosApprox(kCaddieMaximumSlopeRadians);
            CQuadTree::GetMeshNormalAtPoint(&groundNormal, CQuadTree_GetInstance(), &groundedCopy);
            if (groundNormal.y < minimumNormalY) {
                placementAccepted = 0;
            } else {
                *(Arr3*)&v174 = *(Arr3*)&groundedCopy;
                placementAccepted =
                    CQuadTree_OverlappedWithObject(CQuadTree_GetInstance(), &v174, kCaddieObstacleRadius) == 0;
            }
        }
        if (placementAccepted)
            break;
    }
}

// Register the placed puppet with the named scene object and shadow texture queue.
inline static void RegisterCaddiePuppet(unsigned char* obj, unsigned char* x) {
    void* sceneManager = g_sceneManager;
    void* scenePuppet = ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->GetPuppet();
    DrawScene_SetNamedObjectPuppet(sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), scenePuppet);
    void* shadowManager = g_shadowManager;
    void* shadowPuppet = ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->GetPuppet();
    ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(
        shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), shadowPuppet);
}

#pragma opt_common_subs off
#pragma opt_propagation off
#pragma opt_lifetimes off
#pragma opt_strength_reduction off
// Reset and pose the selected body before attachments and render aliases use it.
// Position is world-space; yaw is radians. Each lookup deliberately reloads activeCaddie.
static inline void ApplyCurrentCaddiePlacement(unsigned char* obj) {
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->ResetPlaybackAndLookAt();
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))
        ->SetPosition((unsigned char*)&AsCGolfCaddie(obj)->position);
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->SetYaw(AsCGolfCaddie(obj)->yaw);
    ((void (*)(void*, void*, int, int, float))PlayAnimationByName__8CPetBodyFfPvii)(
        CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body, (void*)(golfCaddieStrings + kCaddieIdleAnimation), 0, 0,
        0.0f);
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->ApplyAnimationToPuppet();
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->SetupLighting();
    ((CPetBody*)(CADDIE_ENTRY(AsCGolfCaddie(obj)->activeCaddie)->body))->SetLookAtTarget(0, 1);
}

// Materialize a retained body preload, then release it and invalidate its player index.
// ev borrows the message; the selected entry and preload remain actor-owned.
static inline void FinishCaddiePlayerBodyLoad(unsigned char* obj, const unsigned char* ev) {
    int id = AsCGolfCaddie(obj)->petPreloadPlayerIndex;
    if (id != -1) {
        void* wnd = CaddiePointerAt((unsigned char*)g_sharedDoc + id * kCaddiePlayerRecordBytes, 0x1bc);
        void* o = getAt(obj, id);
        if (o) {
            ((CGolfCaddie*)obj)->CreatePetBodyForEntry(o, wnd);
            AsCGolfCaddie(obj)->petPreloadPlayerIndex = -1;
            void* w = AsCGolfCaddie(obj)->petPreload;
            if (w) {
                delete (DeleteObject*)w;
                AsCGolfCaddie(obj)->petPreload = 0;
            }
        }
    }
    return;
}

// Dispatch caddie placement, movement, commentary, visibility, and asynchronous body-loading messages.
void CGolfCaddie::HandleMsg(const CaddieMessage* message) {
    switch (message->type) {
    // Speech policy and aim messages borrow scalar payloads; action IDs index 36 flag bytes.
    case kCaddieSetReactionValue:
        reactionValue = message->arguments[0].integer;
        break;
    case kCaddieResetCommentary:
        ResetCaddieCommentary((unsigned char*)this);
        commentaryTimestampMs = g_currentTimeMilliseconds;
        break;
    case kCaddieTriggerCommentary:
        TriggerCommentary((unsigned char)message->arguments[0].integer, 1, 0);
        break;
    case kCaddieFinishCommentary: {
        int idx = currentAction;
        if (((CaddieActionDescriptor*)caddieActionDescriptors)[idx].flags == 0)
            actionFlags[idx] = 0;
        currentAction = 0;
        break;
    }
    case kCaddieSetAimYaw:
        aimYaw = CaddieFloatAt(message->arguments[0].pointer, 0);
        break;
    case kCaddieAdvanceAimYaw: {
        aimCommentIssued = (0);
        float currentAimYawRadians = aimYaw;
        commentaryTimestampMs = g_currentTimeMilliseconds;
        aimYaw = currentAimYawRadians + CaddieFloatAt(message->arguments[0].pointer, 0);
        break;
    }
    case kCaddieReactToShot:
        PlayShotReaction();
        break;
    case kCaddieFollowBall: {
        char* strings;
        // Ask the named actor whether following is allowed before changing movement state.
        int followAllowed = 1;
        V3 namedPointWorld, groundedNamedPointWorld, namedPointFromBall;
        strings = golfCaddieStrings;
        IActor_DispatchMessage(this, this, strings + kCaddieMoveQueryTarget, 2, &followAllowed, 0, 0, 0);
        unsigned char* activeEntry = (unsigned char*)activeCaddie;
        // All three gates must pass: active model, no movement block, and actor permission.
        // Failure falls through to clear the camera flag; success breaks out of the dispatcher.
        if (activeEntry && !movementBlocked && followAllowed) {
            if (message->arguments[0].pointer) {
                movementDelay = -1.0f;
                unsigned char* namedCoursePoint = (unsigned char*)CourseData_FindNamedPoint(
                    CaddiePointerAt((void*)AsCSharedDoc(g_sharedDoc)->golfDoc, 8), message->arguments[0].pointer);
                *(Arr3*)&namedPointWorld = *(Arr3*)(namedCoursePoint + 0x24);
                void* ballOrigin = GolfBall__Fv();
                GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundedNamedPointWorld, CQuadTree_GetInstance(),
                                                        &namedPointWorld, 0, 0, 0);
                __mi__FRC7WVectorRC7WVector(&namedPointFromBall, &groundedNamedPointWorld, ballOrigin);
                __as__7WVectorFRC7WVector((unsigned char*)&movementTarget, &namedPointFromBall);
                movementTarget[1] = 0.0f; // Adding the ball origin below restores the ball's Y.
                ((CPetBody*)CADDIE_ENTRY(activeCaddie)->body)->rootMotionScale = 6.0f;
                __apl__7WVectorFRC7WVector((unsigned char*)&movementTarget, GolfBall__Fv());
                CaddieByteAt(g_cameraWork, 0x1a2) = 1;
                break;
            }
            strings += kCaddieRunAnimation;
            if (CaddieAnimationIsAllowed(activeEntry, (unsigned char*)this, strings)) {
                V3 trajectoryDisplacement;
                V3 standOffVector;
                movementDelay = CADDIE_ENTRY(activeCaddie)->followStartDelay;
                void* firstBallSample = PreData__9CGolfBallFi(GolfBall__Fv(), 0);
                unsigned char* ball = (unsigned char*)GolfBall__Fv();
                void* lastBallSample = PreData__9CGolfBallFi(GolfBall__Fv(), CaddieIntAt(ball, 0x64));
                __mi__FRC7WVectorRC7WVector(&trajectoryDisplacement, lastBallSample, firstBallSample);
                __as__7WVectorFRC7WVector((unsigned char*)&movementTarget, &trajectoryDisplacement);
                movementTarget[1] = 0.0f; // Horizontalize before measuring/normalizing.
                // Scaling contract (no resampling or guards): D = horizontal
                // distance(first,last); N = last index (+0x64); E = event index
                // at +0x6C. estimate = (D + 0.6*(N-E-90))/entry.followSpeed;
                // rootMotionScale = estimate/(float@ball+0xA4 - 1).
                // 0.6 is world distance per trajectory sample; 90 is a sample
                // offset, not a render-frame count. Keep the formula in samples:
                // it uses neither elapsed seconds nor the presentation rate.
                // Requires nonzero followSpeed and float@A4 != 1; negative
                // estimates are passed through unchanged. Target geometry below
                // is independent of the estimate and uses a 9.6-world-unit gap.
                float followDurationEstimate = Magnitude__7WVectorCFv((unsigned char*)&movementTarget);
                int lastSampleIndex = CaddieIntAt(GolfBall__Fv(), 0x64);

                followDurationEstimate =
                    (kFollowDistancePerSample *
                         (float)(lastSampleIndex - (CaddieIntAt(GolfBall__Fv(), 0x6c)) - kFollowSampleBias) +
                     followDurationEstimate) /
                    CADDIE_ENTRY(activeCaddie)->followSpeed;
                // The body's root motion scale gets the estimate divided by the
                // ball's float at +0xA4 minus 1. This path has no zero-denominator guard.
                float ballScalarAtA4MinusOne = CaddieFloatAt(GolfBall__Fv(), 0xa4) - 1.0f;
                ((CPetBody*)CADDIE_ENTRY(activeCaddie)->body)->rootMotionScale =
                    followDurationEstimate / ballScalarAtA4MinusOne;
                void* followDirection = Normalize__7WVectorFv((unsigned char*)&movementTarget);
                __ml__FfRC7WVector(&standOffVector, followDirection, kFollowStandOffDistance);
                ball = (unsigned char*)GolfBall__Fv();
                lastBallSample = PreData__9CGolfBallFi(GolfBall__Fv(), CaddieIntAt(ball, 0x64));
                V3 followTargetWorld;
                __mi__FRC7WVectorRC7WVector(&followTargetWorld, lastBallSample, &standOffVector);
                __as__7WVectorFRC7WVector((unsigned char*)&movementTarget, &followTargetWorld);
                void* ballLookAtTarget = GolfBall__Fv();
                if (activeCaddie)
                    ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetLookAtTarget(ballLookAtTarget, 0);
                CaddieByteAt(g_cameraWork, 0x1a2) = 1;
                break;
            }
        }
        CaddieByteAt(g_cameraWork, 0x1a2) = 0;
        break;
    }
    // Explicit placement resets commentary/playback, then publishes scene and shadow aliases.
    case kCaddiePlaceAtMessagePosition: {
        caddieVisible = 1;
        ShadowModel_SetNamedProjectionEnabled(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), 1);
        animationState = 0;
        idleAnimationTimer = 20.0f;
        movementDelay = 0.0f;
        movementBlocked = 0;
        ResetCaddieCommentary((unsigned char*)this);
        commentaryTimestampMs = g_currentTimeMilliseconds;
        int idx = CurrentGolfPlayerIndex();
        unsigned char* x = (unsigned char*)getAt((unsigned char*)this, (unsigned char)idx);
        activeCaddie = x;
        if (x) {
            CADDIE_ENTRY(activeCaddie)->playerData = CurrentGolfPlayerData();
            void* w = CADDIE_ENTRY(activeCaddie)->body;
            ((CPetBody*)w)->ResetPlaybackAndLookAt();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetPosition(message->arguments[0].pointer);
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetYaw(5.4977875f);
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))
                ->PlayAnimationByName(0.0f, (void*)(message->arguments[1].pointer), 0, 0);
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->ApplyAnimationToPuppet();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetupLighting();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetLookAtTarget(0, 1);
            UpdateCaddieTextureAndEnable((unsigned char*)this, (unsigned char*)activeCaddie);
            DrawScene_SetNamedObjectEnabled(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 1);
            RegisterCaddiePuppet((unsigned char*)this, (unsigned char*)activeCaddie);
            reactionModeOneEnabled = 0;
        } else {
            DrawScene_SetNamedObjectPuppet(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 0);
            GfxObjList_RemoveGroupByName(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup));
        }
        break;
    }
    // Placement pipeline: select entry -> restore/find ground -> pose body ->
    // replace tag effects/course attachment -> publish scene/shadow -> cache bounds/pose.
    case kCaddiePlaceCurrentPlayer: {
        caddieVisible = 1;
        ShadowModel_SetNamedProjectionEnabled(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), 1);
        animationState = 0;
        idleAnimationTimer = 20.0f;
        movementDelay = 0.0f;
        movementBlocked = 0;
        ResetCaddieCommentary((unsigned char*)this);
        commentaryTimestampMs = g_currentTimeMilliseconds;
        int sel = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer;
        unsigned char* x = (unsigned char*)getAt((unsigned char*)this, sel);
        activeCaddie = x;
        if (x != 0) {
            // Phase 1: associate the player record and choose cached or sampled ground.
            void* useCachedPlacement = message->arguments[0].pointer;

            CADDIE_ENTRY(x)->playerData =
                (unsigned char*)g_sharedDoc +
                (((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer) * kCaddiePlayerRecordBytes +
                0x1b4;
            if (useCachedPlacement) {
                __as__7WVectorFRC7WVector((unsigned char*)&position, (unsigned char*)cachedPlacementPosition);
                yaw = cachedPlacementYaw;
            } else {
                FindClearCaddiePlacement((unsigned char*)this);
            }
            // Phase 2: reset playback and publish the world pose before resolving bones.
            ApplyCurrentCaddiePlacement((unsigned char*)this);
            // Phase 3: replace resource-tag effects, then add the course-specific bone effect.
            FxInterpreter_RemoveEntriesByKey(FxInterpreter_GetInstance(), (void*)effectEntryKey);
            void* nm = ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppet();
            effectEntryKey = (int)FxAttachments_CreateFromResourceTags(FxInterpreter_GetInstance(), nm, 0);
            // Attachment contract (courses 8/13 only): resource-tag effects above
            // are removed by effectEntryKey before rebuilding. This additional
            // effect is created only if the named bone exists. Its matrix pointer
            // borrows the actor's per-player slot; its point pointer borrows bone
            // metadata. Keep both owners alive while the effect uses those pointers.
            // g_fx's box owns the created effect; this branch has no removal call
            // for prior course effects. It is separate from effectEntryKey replacement.
            // CFx::CreateAttachedEffect retains &attachmentDescriptor as sourcePosition.
            // That descriptor is stack scratch and expires here: this is an existing
            // lifetime hazard, preserved for identity. Any new retained-pointer user
            // needs actor-owned descriptor storage as well as stable matrix/point owners.
            // Rotation: copy the bone's affine matrix, save/zero translation,
            // concatenate -pi/2 yaw, then restore translation. Thus orientation
            // changes around the bone origin without moving that origin.
            if (SharedDoc_IsCourse8Or13()) {
                CaddieEffectDescriptor attachmentDescriptor;
                FxAttachment_Init(&attachmentDescriptor);
                void* p;
                {
                    void* tag = (void*)(golfCaddieStrings + kCaddieEffectBone);
                    p = WPuppet_FindNamedBoneEntry(((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppet(), tag);
                }
                if (p) {
                    char attachmentYawMatrix[0x30];
                    V3 v2dc;
                    ((void (*)(void*, int, float))Matrix_SetAxisRotation)(attachmentYawMatrix, kCaddieYawAxis,
                                                                          kCaddieAttachmentYawRadians);
                    int slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                               kCaddieAttachmentMatrixBytes;
                    int savedSlot = slot;
                    unsigned char* savedValue = (unsigned char*)this + savedSlot;
                    Matrix_CopyAffine(((CGolfCaddie*)savedValue)->affineMatrices,
                                      &((CaddieEffectNamedBone*)p)->bone->worldMatrix);
                    slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                           kCaddieAttachmentMatrixBytes;
                    unsigned char* savedSlot2 = (unsigned char*)this + slot;
                    *(Arr3*)&v2dc =
                        *(Arr3*)&((CaddieAttachmentMatrix*)((CGolfCaddie*)savedSlot2)->affineMatrices)->translation;
                    V3 v270;
                    v270.x = 0.0f;
                    v270.y = 0.0f;
                    v270.z = 0.0f;
                    slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                           kCaddieAttachmentMatrixBytes;
                    __as__7WVectorFRC7WVector(
                        &((CaddieAttachmentMatrix*)((CGolfCaddie*)((unsigned char*)this + slot))->affineMatrices)
                             ->translation,
                        &v270);
                    slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                           kCaddieAttachmentMatrixBytes;
                    unsigned char* savedObj = slot + (unsigned char*)this;
                    Matrix_ConcatAffineAssign(((CGolfCaddie*)savedObj)->affineMatrices, attachmentYawMatrix);
                    slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                           kCaddieAttachmentMatrixBytes;
                    __as__7WVectorFRC7WVector(
                        &((CaddieAttachmentMatrix*)((CGolfCaddie*)((unsigned char*)this + slot))->affineMatrices)
                             ->translation,
                        &v2dc);
                    slot = ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer *
                           kCaddieAttachmentMatrixBytes;
                    int savedSlot3 = slot;
                    // Direct descriptor stores preserve the original register lifetimes.
                    attachmentDescriptor.transforms[0] =
                        ((CGolfCaddie*)(savedSlot3 + (unsigned char*)this))->affineMatrices;
                    attachmentDescriptor.points[0] = &((CaddieEffectNamedBone*)p)->metadata->point;
                    ((CFx*)g_fx)
                        ->CreateAttachedEffect((Vec2_8010*)(&attachmentDescriptor),
                                               (const char*)(golfCaddieStrings + kCaddieAttachedEffect), 1, 0, 0);
                }
            }
            DrawScene_SetNamedObjectEnabled(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 1);
            // Phase 4: bind scene/shadow users and publish translated puppet bounds.
            void* mgr3 = g_sceneManager;
            void* nm3 = ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppet();
            DrawScene_SetNamedObjectPuppet(mgr3, (void*)(golfCaddieStrings + kCaddieSceneName), nm3);
            if (Sing_805DC7B0() == 0) {
                if (!s_renderPacketInitialized) {
                    RenderPacket_Init(s_renderPacket);
                    __register_global_object(s_renderPacket, (void*)RenderPacket_Destroy,
                                             s_renderPacketDtorRegistration_804F68E8);
                    s_renderPacketInitialized = 1;
                }
                Sing_805DC7B0() = s_renderPacket;
            }
            ((void (*)(void*, void*, int, void*, void*, float))GetOrCreateEntry__14CShadowManagerFPCcfUiPCcPC8Src801EF)(
                g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), 16,
                (void*)(golfCaddieStrings + kCaddieShadowTexture), (unsigned char*)Sing_805DC7B0() + 0x4c, 0.09f);
            void* mgr4 = g_shadowManager;
            void* nm4 = ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppet();
            ReplaceNamedTextureQueueFront__14CShadowManagerFPCcPv(mgr4, (void*)(golfCaddieStrings + kCaddieShadowGroup),
                                                                  nm4);
            V3* seg = (V3*)((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppetBoundsMin();
            V3 sa = seg[0];
            V3 sb = seg[1];
            // The course box min/max is one 24-byte block that AABB_Copy copies to world +0x214C.
            struct {
                V3 lo, hi;
            } worldBounds;
            V3 v258;
            V3 v264;
            __pl__FRC7WVectorRC7WVector(&v258, &sa, (unsigned char*)&position);
            __pl__FRC7WVectorRC7WVector(&v264, &sb, (unsigned char*)&position);
            __as__7WVectorFRC7WVector(&worldBounds.lo, &v264);
            __as__7WVectorFRC7WVector(&worldBounds.hi, &v258);
            AABB_Copy((unsigned char*)CQuadTree_GetInstance() + 0x214c, &worldBounds);
            UpdateCaddieTextureAndEnable((unsigned char*)this, (unsigned char*)activeCaddie);
            // Phase 5: cache this grounded placement for the next restore message.
            __as__7WVectorFRC7WVector((unsigned char*)cachedPlacementPosition, (unsigned char*)&position);
            reactionModeOneEnabled = 0;
            cachedPlacementYaw = yaw;
            break;
        }
        FxInterpreter_ClearEntries(FxInterpreter_GetInstance());
        DrawScene_SetNamedObjectPuppet(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 0);
        GfxObjList_RemoveGroupByName(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup));
        break;
    }
    case kCaddieSetPosition: {
        __as__7WVectorFRC7WVector((unsigned char*)&position, message->arguments[0].pointer);
        unsigned char* x = (unsigned char*)activeCaddie;
        if (x) {
            ((CPetBody*)(CADDIE_ENTRY(x)->body))->SetPosition((unsigned char*)&position);
            UpdateCaddieTexturePosition((unsigned char*)this, x);
        }
        break;
    }
    case kCaddieSetYaw: {
        void* d = message->arguments[0].pointer;
        float newYaw = CaddieFloatAt(d, 0);
        void* x = activeCaddie;
        yaw = newYaw;
        if (x)
            ((CPetBody*)(CADDIE_ENTRY(x)->body))->SetYaw(newYaw);
        break;
    }
    case kCaddieSetLookAt: {
        int flag = message->arguments[1].integer != 0;
        void* x = activeCaddie;
        void* a = message->arguments[0].pointer;
        if (x)
            ((CPetBody*)(CADDIE_ENTRY(x)->body))->SetLookAtTarget(a, flag);
        break;
    }
    case kCaddieClearLookAt:
        if (activeCaddie)
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetLookAtTarget(0, 1);
        break;
    // arguments[0] must point to writable Vec3 storage; the position is copied, not stored.
    case kCaddieGetPosition:
        __as__7WVectorFRC7WVector(message->arguments[0].pointer, (unsigned char*)&position);
        break;
    case kCaddiePlayAnimation: {
        unsigned char* x = (unsigned char*)activeCaddie;
        if (x) {
            const char* p = (const char*)message->arguments[0].pointer;
            if (CaddieAnimationIsAllowed(x, (unsigned char*)this, p))
                ((void (*)(void*, void*, int, int, float))PlayAnimationByName__8CPetBodyFfPvii)(
                    CADDIE_ENTRY(activeCaddie)->body, message->arguments[0].pointer, 0, 0, 0.0f);
        }
        break;
    }
    case kCaddieQueueAnimation: {
        unsigned char* x = (unsigned char*)activeCaddie;
        if (x) {
            const char* p = (const char*)message->arguments[0].pointer;
            if (CaddieAnimationIsAllowed(x, (unsigned char*)this, p))
                ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))
                    ->SetNextAnimation((const char*)message->arguments[0].pointer, 0);
        }
        break;
    }
    // Sound playback also clears the current player snapshot's power-shot meter.
    // Both rule branches deliberately perform their own lookup and store.
    case kCaddiePlaySound:
        Sound_PlayByName(g_soundManager, (void*)(golfCaddieStrings + kCaddieReactionSound), 0, 0, 0, 0, 0.5f, 200.0f,
                         1.0f);
        if (AsCSharedDoc(g_sharedDoc)->golfRuleType == 1) {
            void* r = GolfPlayer_GetEntryByIndex((void*)AsCSharedDoc(g_sharedDoc)->golfDoc,
                                                 ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
            ((GolfDocSnapshot8013*)r)->powerShotMeter = 0.0f;
        } else {
            void* r = GolfPlayer_GetEntryByIndex((void*)AsCSharedDoc(g_sharedDoc)->golfDoc,
                                                 ((CGolfDoc*)(void*)AsCSharedDoc(g_sharedDoc)->golfDoc)->currentPlayer);
            ((GolfDocSnapshot8013*)r)->powerShotMeter = 0.0f;
        }
        break;
    // Borrow preview model data; clamp difficulty to 0..3 and cache the resulting placement.
    case kCaddiePreview: {
        int c = 0;
        void* data = message->arguments[0].pointer;
        ResetCaddieCommentary((unsigned char*)this);
        commentaryTimestampMs = g_currentTimeMilliseconds;
        unsigned char* x = (unsigned char*)CreateCaddieEntry((unsigned int)data,
                                                             (unsigned int)CaddiePointerAt(g_sharedDoc, 0x1bc), 1, 1);
        activeCaddie = x;
        if (x != 0) {
            int cnt = message->arguments[1].integer;
            CADDIE_ENTRY(x)->playerData = (unsigned char*)g_sharedDoc + kSharedPlayerSlotsOffset;
            ((CaddiePlayerMessageView*)CADDIE_ENTRY(activeCaddie)->playerData)->model = data;
            c = cnt < 0 ? c : (cnt > kCaddieMaximumDifficulty ? kCaddieMaximumDifficulty : cnt);
            ((CaddiePlayerMessageView*)CADDIE_ENTRY(activeCaddie)->playerData)->difficulty = c;
            FindClearCaddiePlacement((unsigned char*)this);
            activeCaddie = x;
            ((CPetBody*)(CADDIE_ENTRY(x)->body))->ResetPlaybackAndLookAt();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetPosition((unsigned char*)&position);
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetYaw(yaw);
            ((void (*)(void*, void*, int, int, float))PlayAnimationByName__8CPetBodyFfPvii)(
                CADDIE_ENTRY(activeCaddie)->body, (void*)(golfCaddieStrings + kCaddieIdleAnimation), 0, 0, 0.0f);
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->ApplyAnimationToPuppet();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetupLighting();
            ((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->SetLookAtTarget(0, 1);
            V3* seg = (V3*)((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppetBoundsMin();
            V3 sa = seg[0];
            V3 sb = seg[1];
            // The course box min/max is one 24-byte block that AABB_Copy copies to world +0x214C.
            struct {
                V3 lo, hi;
            } previewWorldBounds;
            V3 v210;
            __pl__FRC7WVectorRC7WVector(&v210, &sa, (unsigned char*)&position);
            V3 v21c;
            __pl__FRC7WVectorRC7WVector(&v21c, &sb, (unsigned char*)&position);
            __as__7WVectorFRC7WVector(&previewWorldBounds.lo, &v21c);
            __as__7WVectorFRC7WVector(&previewWorldBounds.hi, &v210);
            AABB_Copy((unsigned char*)CQuadTree_GetInstance() + 0x214c, &previewWorldBounds);
            RegisterCaddiePuppet((unsigned char*)this, (unsigned char*)activeCaddie);
            UpdateCaddieTextureAndEnable((unsigned char*)this, (unsigned char*)activeCaddie);
            // Phase 5: cache this grounded placement for the next restore message.
            __as__7WVectorFRC7WVector((unsigned char*)cachedPlacementPosition, (unsigned char*)&position);
            cachedPlacementYaw = yaw;
            break;
        }
        CaddiePointerAt(g_sharedDoc, 0x1b8) = (void*)c;
        DrawScene_SetNamedObjectPuppet(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 0);
        GfxObjList_RemoveGroupByName(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup));
        break;
    }
    case kCaddieShow:
        caddieVisible = 1;
        ShadowModel_SetNamedProjectionEnabled(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), 1);
        break;
    case kCaddieHide:
        caddieVisible = 0;
        ShadowModel_SetNamedProjectionEnabled(g_shadowManager, (void*)(golfCaddieStrings + kCaddieShadowGroup), 0);
        break;
    case kCaddieSetHierarchy:
        if (activeCaddie)
            WPuppet_SetHierarchyContextRecursive(((CPetBody*)(CADDIE_ENTRY(activeCaddie)->body))->GetPuppet(),
                                                 message->arguments[0].pointer);
        break;
    // arguments[0] is a writable entry-pointer slot; the returned entry remains actor-owned.
    case kCaddieGetActiveEntry:
        // Keep this direct lvalue: a reference helper changes load scheduling.
        *(void**)message->arguments[0].pointer = activeCaddie;
        break;
    case kCaddieSetReactionMode:
        reactionModeOneEnabled = message->arguments[0].integer - 1 == 0;
        break;
    // Existing no-op after successful lookups: walking to the end changes no record.
    case kCaddieCheckInventoryNoOp: {
        int arg = message->arguments[1].integer;
        if (ItemInventory_FindCaddieData((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory,
                                         message->arguments[0].integer)) {
            if (ItemInventory_FindCaddieItemData((unsigned char*)&AsCSharedDoc(g_sharedDoc)->itemInventory, arg)) {
                void** p = (void**)(*(void**)&caddieEntries);
                void** end = p + caddieEntryCount;
                while (end != p)
                    p++;
            }
        }
        break;
    }
    case kCaddieActivateAllConte: {
        caddieVisible = 0;
        for (int i = 0; i < (int)CaddieByteAt((void*)AsCSharedDoc(g_sharedDoc)->golfDoc, 0x2b9); i++) {
            V3 z;
            Arr3 z2;
            z.x = 0.0f;
            z.y = 0.0f;
            z.z = 0.0f;
            *(Arr3*)&z2 = *(Arr3*)&z;
            int savedI = i;
            ((CGolfCaddie*)this)->ActivatePlayerModel(savedI, 0.0f, &z2);
        }
        break;
    }
    case kCaddieClearConteModels: {
        void* name;
        int off;
        int i;
        off = 0;
        for (i = 0; i < (int)CaddieByteAt((void*)AsCSharedDoc(g_sharedDoc)->golfDoc, 0x2b9); i++) {
            name = String_FormatTemporary(golfCaddieStrings + kCaddieConteNameFormat, i);
            FxInterpreter_ClearEntries(FxInterpreter_GetInstance());
            DrawScene_SetNamedObjectPuppet(g_sceneManager, name, 0);
            GfxObjList_RemoveGroupByName(g_shadowManager, name);
            if (!GetConteModelSlotsSlot()) {
                if (!s_courseSlotTableInitialized) {
                    CourseSlotTable_InitEmpty(s_conteModelSlots);
                    s_courseSlotTableInitialized = 1;
                }
                GetConteModelSlotsSlot() = s_conteModelSlots;
            }
            void* q = CourseSlotTable_GetExtraEntry(GetConteModelSlotsSlot(), i);
            CaddiePointerAt(q, 4) = 0;
            CaddiePointerAt(q, 8) = 0;
            void* rec = (unsigned char*)conteCaddies + off;
            off += 0xc;
            CaddiePointerAt(rec, 4) = 0;
            CaddiePointerAt(rec, 8) = 0;
        }
        caddieVisible = 1;
        break;
    }
    case kCaddieSetConteState:
        storyConteCaddieOverride = message->arguments[0].pointer;
        RebuildModelLoadQueue();
        break;
    case kCaddiePollConteLoads: {
        int v = PollPendingModelLoads();
        CaddieIntAt(message->arguments[0].pointer, 0) = v;
        break;
    }
    // Body lifecycle, in actual call order: remove the shadow entry, delete the
    // body and clear its pointer, THEN clear the scene puppet alias. The scene
    // alias still exists during body destruction; do not claim it was detached
    // first or move either call as a readability change. Absent body is a no-op.
    case kCaddieDeletePlayerBody: {
        void* r = getAt((unsigned char*)this, message->arguments[0].integer);
        if (r) {
            if (CADDIE_ENTRY(r)->body) {
                void* mgr = g_shadowManager;
                GfxObjList_RemoveEntryFromNamedGroup(mgr, (void*)(golfCaddieStrings + kCaddieShadowGroup),
                                                     ((CPetBody*)(CADDIE_ENTRY(r)->body))->GetPuppet());
                if (r) {
                    if (CADDIE_ENTRY(r)->body) {
                        void* o = CADDIE_ENTRY(r)->body;
                        delete (DeleteObject*)o;
                        CADDIE_ENTRY(r)->body = 0;
                    }
                }
                DrawScene_SetNamedObjectPuppet(g_sceneManager, (void*)(golfCaddieStrings + kCaddieSceneName), 0);
            }
        }
        break;
    }
    // Begin at most the caller-selected preload; completion is driven outside this handler.
    // Begin/poll/finish are distinct async phases; the actor owns the preload object.
    case kCaddieBeginPlayerBodyLoad: {
        int n;
        void* sharedDoc;
        n = message->arguments[0].integer;
        sharedDoc = g_sharedDoc;
        void* wnd = CaddiePointerAt((unsigned char*)sharedDoc + n * kCaddiePlayerRecordBytes, 0x1b8);
        void* r = getAt((unsigned char*)this, n);
        if (r) {
            if (!CADDIE_ENTRY(r)->body) {
                unsigned cnt = CaddieByteAt((void*)AsCSharedDoc(sharedDoc)->golfDoc, 0x2b9);
                void* result = CPreloadPetAsync::Create(wnd, (((cnt <= 2))));
                int index = message->arguments[0].integer;
                petPreload = result;
                petPreloadPlayerIndex = index;
                petPreloadActive = 1;
            }
        }
        break;
    }
    // arguments[0] is int* output; completion is true when the preload-active latch is clear.
    case kCaddieIsPlayerBodyLoadDone:
        CaddieIntAt(message->arguments[0].pointer, 0) = petPreloadActive == 0;
        break;
    // Materialize the loaded body, then delete the preload object and invalidate its player index.
    case kCaddieFinishPlayerBodyLoad:
        FinishCaddiePlayerBodyLoad((unsigned char*)this, (const unsigned char*)message);
        break;
    // A missing entry is treated as ready, so callers can skip nonexistent player bodies.
    case kCaddieBodyReadyOrNoEntry: {
        int arg = message->arguments[1].integer;
        void* out = message->arguments[0].pointer;
        void* o = getAt((unsigned char*)this, arg);
        if (0 == o) {
            CaddieIntAt(out, 0) = 1;
        } else {
            CaddieIntAt(out, 0) = CADDIE_ENTRY(o)->body != 0;
        }
        break;
    }
    case kCaddieNoOp:
        break;
    default:
        break;
    }
}
#pragma opt_common_subs reset
#pragma opt_propagation reset
#pragma opt_lifetimes reset
#pragma opt_strength_reduction reset
}
