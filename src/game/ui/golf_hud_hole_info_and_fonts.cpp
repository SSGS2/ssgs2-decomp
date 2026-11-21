#include <decomp/static_init.h>
#include <decomp/local_class.h>
// Golf HUD hole/cup/accuracy/timeout consumers, font registry and effect attachments.
// Widescreen lookup: CHoleInfoSet_Draw is a dispatch boundary, not a projection
// USE_B uses the current pin pivot) and 0x80100DC0 (hole/course/par header: LEFT).
// applies tutorial admission stages 4/5 and child enable flags. Preserve the
// separate child calls/order so pin text and the header can anchor differently.
// CDriveInfo_DrawPlayerDistanceRows positions distance rows from WView::viewportWidth and the
// kDriveHud* layout/UV constants. Its distance buffer uses the stored player
// Driving Contest result directly (%6.2f): display-distance units including the
// contest bonus, not raw world distance. CDriveSystem_HandleMsg request 0
// (drive_contest_and_darts.cpp) produces horizontal current-minus-launch displacement divided
// by binary32 3.200000047683716, truncated to hundredths, and a separate 100-unit
// hole-in bonus. CGolfRule_ProcessBallFlight (golf_rule_round_flow_and_shot_sim.cpp) accumulates that bonus
// at player+170 and publishes distance+bonus at +178+routeSlot*4. This defines the
// numerical unit; a physical SI/metre interpretation is not established by those
// producers. Do not add another /3.2 or label the bonus as extra travelled length.
// This row reads slot zero; no yard/metre conversion occurs in this unit.
// Coordinate map: HUD rectangles are X/Y/width/height (X right, Y down), source
// atlas rectangles are normalized UV extents. Attachment points instead use
// world XYZ with Y height; optional affine matrices convert local to world.
// CPvsFxBox_TestGroundCrossing exposes terrain-contact limits/tolerances in
// u801005E4_data.cpp (+/-99 and strict +/-1e-5 world Y), unrelated to viewport size.
// Ownership: CHoleInfoSet_Ctor/Init own four children; Dtor releases them.
// Font/text destructors release resource-manager handles and array-new buffers.
// Tree map values and attachment matrix/point pointers remain borrowed. Registry
// operations lock a recursive mutex; HUD draw/update assume serialized scene calls.
// Required receivers/outputs unless an entry explicitly allows null; deleting>0
// frees wrapper storage and returned self then becomes a stale address. Scalar
// data also lives in phase1_shared_data.cpp; resource paths use golfHudFontStrings.
// No local output resolution, aspect, scissor or presentation-rate constant exists.
// Byte layout of the existing string pool; no new string storage.
struct GolfHudStringPool {
    char font_hole_gsr[15]; // "[font_hole.gsr"
    char padding_00F[19];
    char font_bar_gsr[14]; // "[font_bar.gsr"
    char padding_030[85];
    char bigfont_gsr[13]; // "[bigfont.gsr"
    char padding_092[452];
    char top_hole_l_gsr[16]; // "[top_hole_l.gsr"
    char top_hole_m_gsr[16]; // "[top_hole_m.gsr"
    char top_hole_h_gsr[16]; // "[top_hole_h.gsr"
    char padding_286[179];
    char text_6_2f[6]; // "%6.2f"
    char padding_33F[602];
    char tree_insert_length_error[26]; // "tree::insert length error"
    char std_exception_std_logic_error_std_length_error[55]; // "!std::exception!!std::logic_error!!std::length_error!!"
};
enum {
    kGolfHudStringPool_font_hole_gsr = (int)&((GolfHudStringPool*)0)->font_hole_gsr,
    kGolfHudStringPool_font_bar_gsr = (int)&((GolfHudStringPool*)0)->font_bar_gsr,
    kGolfHudStringPool_bigfont_gsr = (int)&((GolfHudStringPool*)0)->bigfont_gsr,
    kGolfHudStringPool_top_hole_l_gsr = (int)&((GolfHudStringPool*)0)->top_hole_l_gsr,
    kGolfHudStringPool_top_hole_m_gsr = (int)&((GolfHudStringPool*)0)->top_hole_m_gsr,
    kGolfHudStringPool_top_hole_h_gsr = (int)&((GolfHudStringPool*)0)->top_hole_h_gsr,
    kGolfHudStringPool_text_6_2f = (int)&((GolfHudStringPool*)0)->text_6_2f,
    kGolfHudStringPool_tree_insert_length_error = (int)&((GolfHudStringPool*)0)->tree_insert_length_error,
    kGolfHudStringPool_std_exception_std_logic_error_std_length_error = (int)&((GolfHudStringPool*)0)->std_exception_std_logic_error_std_length_error,
};

// Constants retain the original integer types and encoded values.
enum { kTutorialShowHoleCup = 4, kTutorialShowHoleInfo = 5,
       kHudFadeIn = 2, kHudFadeOut = 3, kMaxAttachmentPoints = 4,
       kFontRegistryInitialCapacity = 4,
       kHudTextBufferBytes = 32, kTimeoutInitialKind = 2, kTimeoutInitialCount = 3,
       kHudAlphaByteShift = 24, kHudMapNodeBytes = 20, kHudMapPairBytes = 8 };
static const unsigned kRgbWhite = 0x00FFFFFFU;

#include "classes/CSharedDoc.h"
#include "classes/WView.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CWindInfo.h"
#define AsCWindInfo(p) ((CWindInfo*)(p))
// Translation unit 0x801005E4-0x80101BD0 (25 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"
#include "game/wvector.h"

// Construct a map payload at its already allocated address.
inline void* operator new(size_t, void* p) { return p; }
// Placement construction does not own the supplied node allocation.
inline void operator delete(void*, void*) {}

// Layout-only views for HUD types that have no class header in this unit.
struct HoleHudSet {
    void* holeCupInfo;
    unsigned int reserved04;
    void* holeInfo;
    void* accuracyInfo;
    void* timeOutInfo;
    unsigned char holeCupEnabled;
    unsigned char reserved15[2];
    unsigned char accuracyEnabled;
    unsigned char timeOutEnabled;
};
struct HudTextResources {
    const unsigned int* vtable;
    void* fontResource;
    unsigned char baseState[0x0C];
    void* textBuffer;
};
struct EffectAttachmentPoints {
    float screenX, screenY;
    void* transforms[4];
    void* positions[4];
};
// Each pair is borrowed: point alone is already in world space; matrix+point
// transforms a local point into world space; matrix alone contributes translation.
// The first empty pair terminates the list, even if later slots are populated.
struct AttachmentMatrix { float axes[9]; float translation[3]; };

// One schema for initialization, registration/lookup and destruction. These are
// native 32-bit Wii pointers/words, not serialized font data. hud_distance_to_pin_and_cup_info.cpp's
// FontRegistryTable has the SAME offsets; its lookup overlay reads only
// bucketMask/buckets/mutex (+14/+1C/+20). Node+00 points to RegisteredFontInfo
// (name[32] at info+24, halfScale at +44); node+04 borrows that name; +14 is
// the hash-bucket link. next/prev (+0C/+10) form circular allocation/free/active
// rings. nodesPerBlock=4 includes the allocation-header node at index zero;
// registration uses indices 1..3 as free nodes. Destruction frees allocation
// blocks via +08, NOT each active/free node inside those blocks. It does not
// delete node->configuration; do not infer that payload cleanup from block free.
struct HudFontRegistryNode {
    void* configuration; const char* name;
    unsigned char flags; char reserved09[3];
    HudFontRegistryNode* next; HudFontRegistryNode* prev;
    HudFontRegistryNode* bucketNext;
};
struct HudFontRegistry {
    HudFontRegistryNode* activeNodes;    // +00, registration inserts used nodes
    unsigned int reserved04;           // +04, initialized to zero
    HudFontRegistryNode* allocatedBlocks; // +08, one header per allocation
    HudFontRegistryNode* freeNodes;     // +0C, registration detaches a free slot
    unsigned int nodesPerBlock;        // +10, allocation stride is node size 0x18
    unsigned int bucketMask;           // +14, bucketCount-1
    unsigned int bucketCount;          // +18, initialized to four
    HudFontRegistryNode** buckets;     // +1C, owned pointer array, appFreeUni
    unsigned char mutexStorage[1];     // +20, start of embedded recursive OSMutex
};
typedef char HudFontRegistry_node_size[sizeof(HudFontRegistryNode) == 0x18 ? 1 : -1];
typedef char HudFontRegistry_mutex_offset[(int)&((HudFontRegistry*)0)->mutexStorage == 0x20 ? 1 : -1];

extern "C" {

void CHoleInfoSet_Ctor(void* self);
void* __nw__FUl(unsigned int size);
void __ct__9CInfoBaseFv(void* self);
void CDistToHoleInfo_Ctor(void* self);
void CHoleInfo_Ctor(void* self);
void CAccuracyInfo_Ctor(void* self);
void __dt__9CInfoBaseFv(void* self, int deleting);
void* WOverlay_Create(void* self, const char* name, int flag);
void* __nwa__FUl(unsigned int size);
void* appMallocDummy__FUl(unsigned int size);
void RBTree_RebalanceAfterInsert(void* self, unsigned int value);
void __dt__Q23std12length_errorFv(void*);
extern const char __vt__12CHoleCupInfo[];
extern const char __vt__12CTimeOutInfo[];
extern const char golfHudFontStrings[];
extern const char __vt__Q23std11logic_error[];
extern const char s_sharedCharDeleterVtable[];
extern const char __vt__Q23std12length_error[];
extern void* g_resourceManager;
extern const float kFloat_0_hud;
// Native row placement: vertical scale/origin, cropped atlas UV width/height;
// changing the crop requires matching the player-label overlay image.
extern const float kDriveHudVerticalLayoutScale, kDriveHudRowOriginAndLabelMargin, kHudScalar0_8125, kHudScalar0_6875;
// .8 base label size; width/height are multiplied by highlight alpha as well.
// Alpha delta therefore changes both brightness and visible label dimensions.
extern const float kDriveHudLabelBaseScale, kDriveHudLabelWidth, kDriveHudLabelHeight, kDriveHudHighlightAlphaDelta;
extern const float kFloat_0_5_hud, kFloat_1_805DE314, kFloat_28_GolfHud, kFloat_50_HudLayout;
extern const float kFloat_25_805DE368, kFloat_0_6_805DE33C, kFloat_3_Hud;
void ResourceManager_DeleteResource(void* self, void* resource);
void __dla__FPv(void* self);
void __dl__FPv(void* self);
extern void* g_resourceManager;
extern const unsigned int cInfoBaseVtable[];
extern const float kFloat_Neg1_0;
extern float kEffectGroundHeightUpperLimit, kEffectGroundHeightLowerLimit, kEffectGroundToleranceUpperBound, kEffectGroundToleranceLowerBound;
void appFreeUni__FPv(void* self);
void OverlayMapTree_FreeSubtree(void* self, void* tree);
void Vec3_Zero(void* self);
void Matrix_TransformPoint(void* out, void* value, void* matrix);
void __apl__7WVectorFRC7WVector(void* self, void* in);
void Vec3_DivideAssign(void* self, float value);
void __dt__13WInstanceLockFv(void* self);
extern CSharedDoc* g_sharedDoc;
extern const float kFloat_6_2831855;
extern float s_hudInfoFullCircleAngle;
extern char s_hudFontConfigRegistryDtorRegistration[];
extern char g_hudFontConfigurationRegistry[];
void* appMallocUni__FUl(unsigned int size);
void* HudFontConfigRegistry_dtor(void* self, int deleting);
void CHoleInfoSet_Draw(void* self);
void CHoleInfoSet_Update(void* self, float dt);
void* GolfPlayer_GetEntryByIndex(void* state, unsigned int index);
void HudFont_DrawAlignedText(void* self, void* layout, float x, float y, float z, float alpha, int, int);
void WOverlay_DrawTransformedRect(void* text, void* camera, float* sourceUvRect, float, float* destinationRect, int, unsigned int color, int);
extern void* g_view;
int CPvsFxBox_ComputeMeanAttachmentPosition(void* items, void* context);
void __as__7WVectorFRC7WVector(void* self, void* in);
unsigned int CQuadTree_GetInstance(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void* out, unsigned int type, void* in, int, int, int);

struct Vec3u {
    unsigned int x, y, z;
    // Preserve float component bits for by-value ground-query calls.
    Vec3u(unsigned int a, unsigned int b, unsigned int c) : x(a), y(b), z(c) {}
};
// Driving Contest lane of the player's result union (+0x178); binary32 display
// distance (world XZ magnitude/3.2, hundredths) including bonus; see producer above.
// Other party modes use integer score at this same address;
// CDriveInfo is only valid for the Driving Contest interpretation.
struct PlayerEntryView { unsigned char pad00[0x178]; float driveDistance; };
struct HudInfoInterface { 
virtual void unused0(); virtual void update(float); virtual void draw();
};
struct HudOwnedObject { 
virtual void destroy(int);
};

// Draw each player's drive-distance row and emphasize the current player during the HUD fade.
// Require initialized font/buffer and playerLabels[0..playerCount-1], with
// playerCount<=4. alphaByte supplies packed alpha; fadeState 2/3 uses elapsed
// SECONDS at +2C, written by CDriveInfo_UpdateDistanceAnimation (golf_hud_flight_info_and_pang_rewards.cpp).
// CScreen_OnProcess -> UpdateHudTimedChildren -> modeInfo virtual update
// (ingame_hud_screen.cpp; CDriveInfo installed for Driving Contest) forwards task dt seconds.
// The updater adds delta once, resets elapsed at each phase transition, and
// uses shared .5 as the count-phase duration in seconds. Here the same .5
// scales highlight fade-in/out; elsewhere it also scales font/text geometry
// (hud_distance_to_pin_and_cup_info.cpp) and audio volume (golf_hud_flight_info_and_pang_rewards.cpp). A fade-duration mod must separate
// storage or coordinate those roles rather than globally changing .5.
// Preserve integer alpha packing,
// floor-based label placement and both text/overlay draws for each player.
void CDriveInfo_DrawPlayerDistanceRows(void* self) {
    struct Data { unsigned char pad00[0x0C]; unsigned int color; unsigned char pad10[4]; void* distanceText; unsigned char pad18[0xC]; unsigned int alphaByte; int fadeState; float fadeTime; unsigned char pad30[4]; void** playerLabels; };
    Data* data = static_cast<Data*>(self);
    unsigned int color = (data->alphaByte << kHudAlphaByteShift) | kRgbWhite /* ARGB: alpha from the byte at +0x24, RGB white */;
    data->color = color;
    // Phase 1: labelUvRect selects normalized texture UVs; labelRect holds layout X/Y/width/height.
    // Distance text uses viewport pixels. WOverlay_DrawTransformedRect applies each overlay's
    // anchoring/scaling to labelRect; rowSpacing increases layout Y down the player rows.
    float labelUvRect[4];
    float labelRect[4];
    labelUvRect[0] = 0.0f;
    labelUvRect[1] = 0.0f;
    labelUvRect[2] = kHudScalar0_8125;
    labelUvRect[3] = kHudScalar0_6875;
    labelRect[0] = 0.0f;
    labelRect[1] = 0.0f;
    labelRect[2] = 0.0f;
    labelRect[3] = 0.0f;
    labelRect[1] = (float)floor(kDriveHudRowOriginAndLabelMargin - kDriveHudVerticalLayoutScale * labelRect[0]);
    float labelBaseScale = kDriveHudLabelBaseScale;
    float one;
    float half;
    float highlightAlphaDelta;
    float alpha;
    float rowSpacing;
    float labelOffsetX;
    float labelRightMargin;
    float labelHeight;
    float labelWidth;
    float distanceRightMargin;
    labelWidth = kDriveHudLabelWidth * labelBaseScale;
    labelHeight = kDriveHudLabelHeight * labelBaseScale;
    highlightAlphaDelta = kDriveHudHighlightAlphaDelta;
    half = kFloat_0_5_hud;
    one = kFloat_1_805DE314;
    distanceRightMargin = kFloat_28_GolfHud;
    labelRightMargin = kDriveHudRowOriginAndLabelMargin;
    labelOffsetX = kFloat_50_HudLayout; // Shared 50.0 constant; horizontal label-layout units here, not atlas Y.
    rowSpacing = kFloat_25_805DE368;
    // Phase 2: each row combines formatted distance with a player-label overlay; alpha
    // changes only for the current player, then also scales that row's label size.
    unsigned char playerIndex = 0;
    while (playerIndex < AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->playerCount) {
        alpha = kFloat_0_6_805DE33C;
        if (playerIndex == AsCGolfDoc((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc))->currentPlayer) {
            if (data->fadeState == kHudFadeIn) {
                alpha += (highlightAlphaDelta * data->fadeTime) / half;
                if (alpha > one) alpha = one;
            } else if (data->fadeState == kHudFadeOut) {
                float next = alpha + (highlightAlphaDelta * (half - data->fadeTime)) / half;
                alpha = next < alpha ? alpha : next;
            }
        }
        void* playerEntry = GolfPlayer_GetEntryByIndex((*(void**)&AsCSharedDoc(g_sharedDoc)->golfDoc), playerIndex);
        sprintf(static_cast<char*>(data->distanceText), golfHudFontStrings + kGolfHudStringPool_text_6_2f, ((PlayerEntryView*)playerEntry)->driveDistance);
        HudFont_DrawAlignedText(self, data->distanceText, ((WView*)g_view)->viewportWidth - distanceRightMargin, labelRect[1] + one, kFloat_3_Hud, alpha, 0, 0);
        alpha += highlightAlphaDelta;
        labelRect[2] = labelWidth * alpha;
        labelRect[3] = labelHeight * alpha;
        labelRect[0] = labelOffsetX + (float)floor((((WView*)g_view)->viewportWidth - distanceRightMargin - labelRect[2]) - labelRightMargin * alpha);
        WOverlay_DrawTransformedRect(data->playerLabels[playerIndex], g_view, labelUvRect, 0.0f, labelRect, 0, color, 0);
        labelRect[1] += rowSpacing;
        ++playerIndex;
    }
}

// Draw enabled hole/cup, hole, accuracy, and timeout HUD components, respecting tutorial gates.
// Borrows self, current document and initialized children. Null children are
// skipped. Cup requires its enable byte AND tutorialStage>=4; hole info has no
// enable byte and requires stage>=5. Accuracy/timeout use only their own flags.
// Both document lookups remain separate across child callbacks. No data is returned.
void CHoleInfoSet_Draw(void* self) {
    HoleHudSet* data = static_cast<HoleHudSet*>(self);
    if (data->holeCupEnabled) {
        CGolfDoc* golfDoc = (*(CGolfDoc**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (golfDoc->tutorialStage >= kTutorialShowHoleCup && data->holeCupInfo != 0)
            static_cast<HudInfoInterface*>(data->holeCupInfo)->draw();
    }
    CGolfDoc* golfDoc = (*(CGolfDoc**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (golfDoc->tutorialStage >= kTutorialShowHoleInfo && data->holeInfo != 0)
        static_cast<HudInfoInterface*>(data->holeInfo)->draw();
    if (data->accuracyEnabled && data->accuracyInfo != 0)
        static_cast<HudInfoInterface*>(data->accuracyInfo)->draw();
    if (data->timeOutEnabled && data->timeOutInfo != 0)
        static_cast<HudInfoInterface*>(data->timeOutInfo)->draw();
}

// Destroy the four owned hole-information HUD components and optionally delete their owner.
void* CHoleInfoSet_Dtor(void* self, int deleting) {
    if (self != 0) {
        if (((HoleHudSet*)self)->holeCupInfo != 0) {
            if (((HoleHudSet*)self)->holeCupInfo != 0) {
                static_cast<HudOwnedObject*>(((HoleHudSet*)self)->holeCupInfo)->destroy(1);
            }
            ((HoleHudSet*)self)->holeCupInfo = 0;
        }
        void* p1 = ((HoleHudSet*)self)->holeInfo;
        if (p1 != 0) {
            if (p1 != 0) {
                static_cast<HudOwnedObject*>(p1)->destroy(1);
            }
            ((HoleHudSet*)self)->holeInfo = 0;
        }
        void* p2 = ((HoleHudSet*)self)->accuracyInfo;
        if (p2 != 0) {
            if (p2 != 0) {
                static_cast<HudOwnedObject*>(p2)->destroy(1);
            }
            ((HoleHudSet*)self)->accuracyInfo = 0;
        }
        void* p3 = ((HoleHudSet*)self)->timeOutInfo;
        if (p3 != 0) {
            if (p3 != 0) {
                static_cast<HudOwnedObject*>(p3)->destroy(1);
            }
            ((HoleHudSet*)self)->timeOutInfo = 0;
        }
        if (deleting > 0) __dl__FPv(self);
    }
    return self;
}

} // extern "C"

struct CInfoBase {
    void* vtable; unsigned char pad[0x0C];
    // Initialize the common HUD font/resource owner before constructing a child.
    CInfoBase() { __ct__9CInfoBaseFv(this); }
    ~CInfoBase();
};

extern "C" {

// Allocate the cup/distance, hole, accuracy, and timeout HUD components and load their fonts/resources.
// Failure/publication contract: CHoleInfoSet_Init zeroes all four owner slots
// first. Each assignment below publishes only after that child's constructor
// returns; prior completed children remain installed if a later construction
// throws. This raw-pointer set has no automatic rollback. An owner that catches
// must call CHoleInfoSet_Dtor(self,0) before discarding/retrying initialized
// storage; calling Ctor directly requires the same zero-slot precondition.
// Cup construction: CInfoBase owns its font, resource1/2/3 are overlays, child
// owns CDistToHoleInfo. On a cup-body throw CInfoBase's destructor runs, but
// raw overlay members already acquired are not individually guarded. Hole and
// accuracy constructors control their own partial cleanup. Timeout construction
// similarly unwinds CInfoBase if its text-buffer allocation throws. The complete
// children's deleting virtual destructors release their fonts/overlays/buffers
// and nested children (golf_hud_flight_info_and_pang_rewards.cpp), before CHoleInfoSet_Dtor clears each slot.
// Null resource/allocation results are not converted into a constructor status;
// load callbacks and later draw/update require successfully initialized children.
void CHoleInfoSet_Ctor(void* self) {
    // Retain this unused local declaration to preserve CodeWarrior's cleanup-symbol numbering.
    struct ConstructorNumberingAnchor { void* holeCupInfo; unsigned int reserved04; void* holeInfo; void* accuracyInfo; void* timeOutInfo; unsigned char initialEnableBytes[5]; };
    struct Loader { 
    virtual void a() DECOMP_LOCAL_VIRTUAL; virtual void b() DECOMP_LOCAL_VIRTUAL; virtual void c() DECOMP_LOCAL_VIRTUAL; virtual void load(const char*, int, int) DECOMP_LOCAL_VIRTUAL;
    };
    struct DistanceToHoleComponent {
        unsigned char pad[0x60];
        // Construct the owned distance-to-hole component.
        DistanceToHoleComponent() { CDistToHoleInfo_Ctor(this); }
    };
    struct HoleComponent {
        unsigned char pad[0xE4];
        // Construct the owned hole-data component.
        HoleComponent() { CHoleInfo_Ctor(this); }
    };
    struct AccuracyComponent {
        unsigned char pad[0x2C];
        // Construct the owned accuracy component.
        AccuracyComponent() { CAccuracyInfo_Ctor(this); }
    };
    struct HoleCupComponent : CInfoBase {
        unsigned char pad10[0x14]; void* resource1; void* resource2; void* resource3; unsigned int enabled; void* child;
        // Load the cup/bar overlays and own a nested distance-to-hole component.
        HoleCupComponent() {
            vtable = const_cast<char*>(__vt__12CHoleCupInfo);
            child = 0;
            reinterpret_cast<Loader*>(this)->load(golfHudFontStrings + kGolfHudStringPool_font_bar_gsr, 0, 0);
            enabled = 1;
            resource1 = WOverlay_Create(g_resourceManager, golfHudFontStrings + kGolfHudStringPool_top_hole_l_gsr, 0);
            resource2 = WOverlay_Create(g_resourceManager, golfHudFontStrings + kGolfHudStringPool_top_hole_m_gsr, 0);
            resource3 = WOverlay_Create(g_resourceManager, golfHudFontStrings + kGolfHudStringPool_top_hole_h_gsr, 0);
            child = new DistanceToHoleComponent;
        }
    };
    struct TimeoutComponent : CInfoBase {
        int kind; void* allocation; int count; float value; unsigned int pad20;
        // Load the timeout font and allocate its 32-byte text buffer.
        TimeoutComponent() {
            vtable=const_cast<char*>(__vt__12CTimeOutInfo);
            reinterpret_cast<Loader*>(this)->load(golfHudFontStrings + kGolfHudStringPool_bigfont_gsr,0,0);
            allocation=__nwa__FUl(kHudTextBufferBytes);
            float initial = kFloat_0_hud;
            kind=kTimeoutInitialKind; count=kTimeoutInitialCount; value=initial;
        }
    };
    // Phase 1: allocate each owned component in draw/update order.
    HoleHudSet* components = static_cast<HoleHudSet*>(self);
    components->holeCupInfo = new HoleCupComponent;
    components->holeInfo = new HoleComponent;
    components->accuracyInfo = new AccuracyComponent;
    components->timeOutInfo = new TimeoutComponent;
    // Phase 2: cup starts hidden; preserve both reserved-byte initial values.
    components->holeCupEnabled=0; components->reserved15[0]=0; components->reserved15[1]=1;
    components->accuracyEnabled=1; components->timeOutEnabled=1;
}

#pragma use_lmw_stmw on
// Insert a copied key/value pair into the HUD map and rebalance the red-black tree.
void* HudFontConfigMap_InsertAndRebalance(void* list, void* position, int insertLeft, int becomesFirst, void* source) {
    // Tree header = {count, root, first}; node = {left, right, parent/color, key, value}.
    // position is the insertion parent (or root sentinel); source is a borrowed
    // two-word pair. This copies the words, not the objects they might reference.
    // Require nonnull tree/position/source, a valid insertion side and no payload
    // alias into the node being allocated. Returns the borrowed inserted node;
    // the map owns it until OverlayMapTree_FreeSubtree. Size overflow throws a
    // length_error, allocation/construction unwind frees the unpublished node.
    struct Ref { int vt; int c1; int c2; char* s; };
    struct Holder {
        char* flag; char* p;
        // Hold error-text storage until its shared deleter owns it.
        Holder(char* b, char* f) { p = b; flag = f; }
        // Delete untransferred text on exception construction failure.
        ~Holder() { if (p) __dla__FPv(p); }
    };
    struct ErrBase {
        void* vt; char* buf; Ref* ref;
        // Copy the diagnostic and create its shared text owner.
        ErrBase(const char* msg0) : vt((void*)__vt__Q23std11logic_error) {
            char done __attribute__((aligned(4))) = 0;
            volatile char baseDone = 0;
            const char* msg = msg0;
            int n = strlen(msg) + 1;
            char* b = (char*)__nwa__FUl((2 * n) - n);
            buf = b;
            { Holder h(b, &done); Ref* r = (Ref*)__nw__FUl(16); if (r) { r->c1 = 1; r->c2 = 1; r->vt = (int)s_sharedCharDeleterVtable; r->s = b; } ref = r; h.p = 0; }
            strcpy(buf, msg);
        }
        // No local cleanup; the thrown object's runtime representation handles it.
        ~ErrBase() {}
    };
    struct ErrThrow : ErrBase {
        // Finish the length_error representation and transfer it to the runtime.
        ErrThrow(const char* msg) : ErrBase(msg) { vt = (void*)__vt__Q23std12length_error; __throw((const char*)(golfHudFontStrings + kGolfHudStringPool_std_exception_std_logic_error_std_length_error), this, (void*)__dt__Q23std12length_errorFv); }
        // Empty local destructor preserves the original exception scaffolding.
        ~ErrThrow() {}
    };
    // Phase 1: guard size overflow, then allocate and copy the pair before linking.
    if (*reinterpret_cast<unsigned int*>(list) == 0xFFFFFFFFu) {
        ErrThrow err(golfHudFontStrings + kGolfHudStringPool_tree_insert_length_error);
    }
    struct Pair { int first; int second; };
    typedef void* Parent;
    struct Node { int left; unsigned int right; Parent parent; Pair copied; };
    struct NodeGuard {
        void* owner; Node* p;
        // Protect the freshly allocated node until its pair has been constructed.
        NodeGuard(void* o, Node* n) : owner(o), p(n) {}
        // Free an unpublished node if payload construction unwinds.
        ~NodeGuard() { if (p) appFreeUni__FPv(p); }
    };
    struct Allocation { char done;
        Node* p;  Node* saved;
        // Allocate one 20-byte map node; NodeGuard takes cleanup responsibility.
        Allocation() : done(0) { p = static_cast<Node*>(appMallocDummy__FUl(kHudMapNodeBytes)); saved = p; }
        // Allocation wrapper itself owns no cleanup after handing off the node.
        ~Allocation() {}
    } allocation;
    Node* node = allocation.p;
    NodeGuard guard(static_cast<char*>(list) + 4, node);
    new (&node->copied) Pair(*static_cast<Pair*>(source));
    node->right = 0;
    guard.p = 0;
    node->left = 0;
    new (&node->parent) Parent(position);
    // Phase 2: publish the node, rebalance, and update the cached first node.
    if (insertLeft) *reinterpret_cast<void**>(position) = node;
    else *reinterpret_cast<void**>(static_cast<char*>(position) + 4) = node;
    unsigned int n = *reinterpret_cast<unsigned int*>(list);
    *reinterpret_cast<unsigned int*>(list) = 1 + n;
    RBTree_RebalanceAfterInsert(node, *reinterpret_cast<unsigned int*>(static_cast<char*>(list) + 4));
    if (becomesFirst) *reinterpret_cast<void**>(static_cast<char*>(list) + 8) = node;
    return node;
}
#pragma use_lmw_stmw reset
// Update the enabled hole-information HUD components with the same tutorial gates used for drawing.
// dt is forwarded unchanged to each admitted child's update; this function has
// no frame counter, catch-up loop or completion result. Keep draw/update gates aligned.
void CHoleInfoSet_Update(void* self, float dt) {
    HoleHudSet* data = static_cast<HoleHudSet*>(self);
    if (data->holeCupEnabled) {
        CGolfDoc* golfDoc = (*(CGolfDoc**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
        if (golfDoc->tutorialStage >= kTutorialShowHoleCup && data->holeCupInfo != 0)
            static_cast<HudInfoInterface*>(data->holeCupInfo)->update(dt);
    }
    CGolfDoc* golfDoc = (*(CGolfDoc**)&AsCSharedDoc(g_sharedDoc)->golfDoc);
    if (golfDoc->tutorialStage >= kTutorialShowHoleInfo && data->holeInfo != 0)
        static_cast<HudInfoInterface*>(data->holeInfo)->update(dt);
    if (data->accuracyEnabled && data->accuracyInfo != 0)
        static_cast<HudInfoInterface*>(data->accuracyInfo)->update(dt);
    if (data->timeOutEnabled && data->timeOutInfo != 0)
        static_cast<HudInfoInterface*>(data->timeOutInfo)->update(dt);
}

#pragma use_lmw_stmw on
// Free both child subtrees and this map node; the expanded recursion preserves the original code.
void OverlayMapTree_FreeSubtree(void* self, void* tree) {
    // In plain C: free every node below `tree` (children first, payloads not destroyed), then `tree` itself.
    // The code below is that recursion unrolled three levels for each child, as the compiler emitted it.
    // Phase 1: free the left subtree in postorder. Three expanded levels are
    // followed by recursion for deeper branches; the tree depth is not limited.
    struct Node { Node* left; Node* right; };
    Node* a = static_cast<Node*>(tree)->left;
    if (a != 0) {
        Node* b = a->left;
        if (b != 0) {
            Node* c = b->left;
            if (c != 0) {
                if (c->left != 0) OverlayMapTree_FreeSubtree(self, c->left);
                if (c->right != 0) OverlayMapTree_FreeSubtree(self, c->right);
                appFreeUni__FPv(c);
            }
            Node* d = b->right;
            if (d != 0) {
                if (d->left != 0) OverlayMapTree_FreeSubtree(self, d->left);
                if (d->right != 0) OverlayMapTree_FreeSubtree(self, d->right);
                appFreeUni__FPv(d);
            }
            appFreeUni__FPv(b);
        }
        Node* e = a->right;
        if (e != 0) {
            Node* f = e->left;
            if (f != 0) {
                if (f->left != 0) OverlayMapTree_FreeSubtree(self, f->left);
                if (f->right != 0) OverlayMapTree_FreeSubtree(self, f->right);
                appFreeUni__FPv(f);
            }
            Node* g = e->right;
            if (g != 0) {
                if (g->left != 0) OverlayMapTree_FreeSubtree(self, g->left);
                if (g->right != 0) OverlayMapTree_FreeSubtree(self, g->right);
                appFreeUni__FPv(g);
            }
            appFreeUni__FPv(e);
        }
        appFreeUni__FPv(a);
    }
    // Phase 2: free the right subtree with the same postorder rule.
    Node* h = static_cast<Node*>(tree)->right;
    if (h != 0) {
        Node* i = h->left;
        if (i != 0) {
            Node* j = i->left;
            if (j != 0) {
                if (j->left != 0) OverlayMapTree_FreeSubtree(self, j->left);
                if (j->right != 0) OverlayMapTree_FreeSubtree(self, j->right);
                appFreeUni__FPv(j);
            }
            Node* k = i->right;
            if (k != 0) {
                if (k->left != 0) OverlayMapTree_FreeSubtree(self, k->left);
                if (k->right != 0) OverlayMapTree_FreeSubtree(self, k->right);
                appFreeUni__FPv(k);
            }
            appFreeUni__FPv(i);
        }
        Node* l = h->right;
        if (l != 0) {
            Node* m = l->left;
            if (m != 0) {
                if (m->left != 0) OverlayMapTree_FreeSubtree(self, m->left);
                if (m->right != 0) OverlayMapTree_FreeSubtree(self, m->right);
                appFreeUni__FPv(m);
            }
            Node* n = l->right;
            if (n != 0) {
                if (n->left != 0) OverlayMapTree_FreeSubtree(self, n->left);
                if (n->right != 0) OverlayMapTree_FreeSubtree(self, n->right);
                appFreeUni__FPv(n);
            }
            appFreeUni__FPv(l);
        }
        appFreeUni__FPv(h);
    }
    // Phase 3: both children are gone; free this node (payloads are not destroyed here).
    appFreeUni__FPv(tree);
}
#pragma use_lmw_stmw reset
struct WInstanceLock {
    void* lock;
    // Acquire the instance's recursive OS mutex.
    WInstanceLock(void* value) : lock(value) { OSLockMutex((OSMutex*)value); }
    // Release the mutex on normal return or unwind.
    ~WInstanceLock() { OSUnlockMutex((OSMutex*)lock); }
};
// Initialize the locked HUD-font configuration registry and register its global destructor.
void HudFontConfigRegistry_Initialize() {
    int zero = 0;
    HudFontRegistry* data = reinterpret_cast<HudFontRegistry*>(((char*)&g_hudFontConfigurationRegistry));
    float angle = kFloat_6_2831855;
    s_hudInfoFullCircleAngle = angle;
    void* head = reinterpret_cast<char*>(data) + 0x20;
    OSInitMutex((OSMutex*)head);
    {
        WInstanceLock guard(head);
        data->activeNodes = (HudFontRegistryNode*)zero;
        data->freeNodes = (HudFontRegistryNode*)zero;
        data->allocatedBlocks = (HudFontRegistryNode*)zero;
        data->nodesPerBlock = kFontRegistryInitialCapacity;
        data->reserved04 = zero;
        data->bucketCount = kFontRegistryInitialCapacity;
        if (data->bucketCount > 0) {
            data->buckets = static_cast<HudFontRegistryNode**>(appMallocUni__FUl(0x10));
            data->bucketMask = data->bucketCount - 1;
            for (int i = 0; i < (int)data->bucketCount; ++i) data->buckets[i] = 0;
        } else data->buckets = 0;
    }
    __register_global_object(data, reinterpret_cast<void*>(HudFontConfigRegistry_dtor), s_hudFontConfigRegistryDtorRegistration);
}

// Reset the vtable to the CInfoBase one, release the font resource and free the text buffer.
static inline void ReleaseHudTextResources(void* self) {
    void* res = ((HudTextResources*)self)->fontResource;
    ((HudTextResources*)self)->vtable = cInfoBaseVtable;
    if (res != 0) {
        ResourceManager_DeleteResource(g_resourceManager, res);
        ((HudTextResources*)self)->fontResource = 0;
    }
    void* buf = ((HudTextResources*)self)->textBuffer;
    if (buf != 0) {
        __dla__FPv(buf);
        ((HudTextResources*)self)->textBuffer = 0;
    }
}

// Release the timeout HUD font resource and text allocation, then optionally delete the object.
void* CTimeOutInfo_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            ReleaseHudTextResources(self);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the wind HUD font resource and text buffer, then optionally delete the object.
void* CWindInfo_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            void* res = ((HudTextResources*)self)->fontResource;
            ((HudTextResources*)self)->vtable = cInfoBaseVtable;
            if (res != 0) {
                ResourceManager_DeleteResource(g_resourceManager, res);
                ((HudTextResources*)self)->fontResource = 0;
            }
            void* buf = AsCWindInfo(self)->textBuffer;
            if (buf != 0) {
                __dla__FPv(buf);
                AsCWindInfo(self)->textBuffer = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the remaining-time HUD font resource and text allocation, then optionally delete the object.
void* CRemainTimeInfo_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            ReleaseHudTextResources(self);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

// Release the power HUD font resource and text allocation, then optionally delete the object.
void* CPowerInfo_dtor(void* self, int deleting) {
    if (self != 0) {
        if (self != 0) {
            ReleaseHudTextResources(self);
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}

#pragma use_lmw_stmw on
// Unlink and free registry nodes under its mutex, release the block array, and optionally delete the registry.
void* HudFontConfigRegistry_dtor(void* self, int deleting) { // destructor of the HUD font configuration registry
    HudFontRegistry* data = static_cast<HudFontRegistry*>(self);
    if (self != 0) {
        {
            WInstanceLock guard(static_cast<char*>(self) + 0x20);
            // Free whole node allocations through their index-zero ring, then buckets.
            if (data->allocatedBlocks != 0) {
                OSMutex* registryMutex = reinterpret_cast<OSMutex*>(data->mutexStorage);
                HudFontRegistryNode* node;
                while ((node = data->allocatedBlocks) != 0) {
                OSLockMutex(registryMutex);
                    node->prev->next = node->next;
                    node->next->prev = node->prev;
                    HudFontRegistryNode* next = node->prev == node ? 0 : node->prev;
                    OSUnlockMutex(registryMutex);
                    data->allocatedBlocks = next;
                    appFreeUni__FPv(node);
                }
            }
            if (data->buckets != 0) {
                appFreeUni__FPv(data->buckets);
                data->buckets = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset

// Average up to four supplied attachment points, transforming each by its optional matrix.
// Required items is EffectAttachmentPoints[1]; context needs three floats and
// must not overlap any supplied position or matrix (it is zeroed before reads).
// context is a writable world-space vector, cleared even when no pair exists.
// Returns 1 with the mean of the contiguous nonempty prefix, 0 with a zero vector.
int CPvsFxBox_ComputeMeanAttachmentPosition(void* items, void* context) {
    Vec3_Zero(context);
    int count = 0;
    char* entry = static_cast<char*>(items);
    void* value;
    void* fallback;
    void* selected;
    char temp[0x10];
    while (1) {
        value = ((EffectAttachmentPoints*)entry)->positions[0];
        if (value != 0) {
            fallback = ((EffectAttachmentPoints*)entry)->transforms[0];
            selected = value;
            if (fallback != 0) {
                Matrix_TransformPoint(temp, value, fallback);
                selected = temp;
            }
            *(WVector*)context += *(WVector*)selected;
        } else {
            fallback = ((EffectAttachmentPoints*)entry)->transforms[0];
            if (fallback == 0) break;
            *(WVector*)context += *(WVector*)(((AttachmentMatrix*)fallback)->translation);
        }
        ++count;
        entry += 4;
        if ((unsigned int)count >= kMaxAttachmentPoints) break;
    }
    if (count > 0) {
        float converted = static_cast<float>(count);
        Vec3_DivideAssign(context, converted);
        return 1;
    }
    return 0;
}

// The numeric-font callback intentionally performs no action.
void CFontNumDraw_vfunc_4() {}
// The balloon-information callback intentionally performs no action.
void CBalloonInfo_vfunc_6() {}
// Reserved numeric-font hook; it performs no action.
void CFontNumDraw_vfunc_2() {}
// Reserved numeric-font hook; it performs no action.
void CFontNumDraw_vfunc_1() {}
// Return success unconditionally for this balloon-information hook.
int CBalloonInfo_vfunc_7() { return 1; }

// Test whether the drive-distance HUD transition-state word at +28 is zero.
// This reads the same word CDriveInfo_DrawPlayerDistanceRows compares with fade states 2/3;
// it returns a Boolean, advances no timer and does not poll resource readiness.
int CDriveInfo_vfunc_7(void* self) {
    return *reinterpret_cast<unsigned int*>(static_cast<char*>(self) + 0x28) == 0;
}

// Initialize an attachment with unset screen coordinates and four empty transform/point pairs.
void FxAttachment_Init(void* self) {
    struct Data {
        float x;
        float y;
        unsigned int transform0;
        unsigned int transform1;
        unsigned int transform2;
        unsigned int transform3;
        unsigned int point0;
        unsigned int point1;
        unsigned int point2;
        unsigned int point3;
    };
    Data* data = static_cast<Data*>(self);
    float value = kFloat_Neg1_0;
    data->transform0 = 0;
    data->x = value;
    data->y = value;
    data->point0 = 0;
    data->transform1 = 0;
    data->point1 = 0;
    data->transform2 = 0;
    data->point2 = 0;
    data->transform3 = 0;
    data->point3 = 0;
}

#pragma use_lmw_stmw on
// Shared node shape for the effect sets and their attachment maps; bit zero tags parent color.
struct HudEffectTreeNode {
    HudEffectTreeNode* left; HudEffectTreeNode* right;
    unsigned int parent; void* key; void* value;
};
static const unsigned int kHudTreeParentColorTag = 1U;

// Advance an effect-tree cursor in key order, keeping traversal scratch in its caller.
static inline void AdvanceHudEffectNode(HudEffectTreeNode*& effectNode, HudEffectTreeNode*& node) {
    node = effectNode->right;
    if (node != 0) { while (node->left != 0) node = node->left; effectNode = node; }
    else {
        node = reinterpret_cast<HudEffectTreeNode*>(effectNode->parent & ~kHudTreeParentColorTag);
        while (effectNode != node->left) {
            effectNode = node;
            node = reinterpret_cast<HudEffectTreeNode*>(node->parent & ~kHudTreeParentColorTag);
        }
        effectNode = node;
    }
}

// Set parent to the borrowed attachment node for an unsigned effect-pointer key, or the sentinel on a miss.
static inline void FindHudEffectAttachment(HudEffectTreeNode*& parent, HudEffectTreeNode*& node, void*& key, HudEffectTreeNode*& attachmentSentinel) {
    while (node != 0) {
        if (reinterpret_cast<unsigned int>(node->key) >= reinterpret_cast<unsigned int>(key)) { parent = node; node = node->left; }
        else node = node->right;
    }
    if (parent != attachmentSentinel && reinterpret_cast<unsigned int>(key) < reinterpret_cast<unsigned int>(parent->key)) parent = attachmentSentinel;
}

// Update spray and sequence world positions from their matching attachment records.
// Required initialized effect/attachment trees; effects and attachment pointers
// are borrowed and must survive traversal. No tree mutation or allocation occurs.
// An output aliases the effect's position member, not an attachment point.
void CPvsFxBox_UpdateAttachedEffectPositions(void* self) {
    // Both passes iterate effect trees in key order and lower_bound the attachment
    // map using the effect pointer as an unsigned key. Missing/empty attachments
    // leave the previous effect position unchanged. Parent links tag color in bit 0.
    // Phase 1: effect tree +0x164, attachment map +0x380, output position +0x84.
    typedef HudEffectTreeNode TreeNode;
    struct EffectTrees {
        char prefix[0x158];
        TreeNode* sequenceRoot; TreeNode* sequenceFirst; unsigned int sequenceCount;
        TreeNode* sprayRoot; TreeNode* sprayFirst; unsigned int sprayCount;
        char beforeAttachments[0x210];
        TreeNode* sprayAttachmentRoot; char sprayAttachmentHeader[8];
        TreeNode* sequenceAttachmentRoot;
    };
    struct AttachmentNodeView {
        TreeNode* left; TreeNode* right; unsigned int parentAndColor; void* key;
        EffectAttachmentPoints attachment;
    };
    struct SprayPositionView { char prefix[0x84]; WVector position; };
    struct SequencePositionView { char prefix[0xB8]; WVector position; };
    typedef char AttachmentPayloadOffset[((int)&((AttachmentNodeView*)0)->attachment == 0x10) ? 1 : -1];
    typedef char SprayAttachmentRootOffset[((int)&((EffectTrees*)0)->sprayAttachmentRoot == 0x380) ? 1 : -1];
    typedef char SequenceAttachmentRootOffset[((int)&((EffectTrees*)0)->sequenceAttachmentRoot == 0x38C) ? 1 : -1];
    EffectTrees* trees = static_cast<EffectTrees*>(self);
    {
    TreeNode* effectNode;
    void* key;
    TreeNode* attachmentSentinel;
    TreeNode* effectSentinel;
    attachmentSentinel = reinterpret_cast<TreeNode*>(&trees->sprayAttachmentRoot);
    effectSentinel = reinterpret_cast<TreeNode*>(&trees->sprayRoot);
    effectNode = trees->sprayFirst;
    for (; effectNode != effectSentinel; ) {
        key = effectNode->value;
        TreeNode* node;
        TreeNode* parent;
        parent = reinterpret_cast<TreeNode*>(&trees->sprayAttachmentRoot);
        node = trees->sprayAttachmentRoot;
        FindHudEffectAttachment(parent, node, key, attachmentSentinel);
        WVector result;
        if (parent != attachmentSentinel && CPvsFxBox_ComputeMeanAttachmentPosition(&((AttachmentNodeView*)parent)->attachment, &result))
            ((SprayPositionView*)key)->position = result;
        AdvanceHudEffectNode(effectNode, node);
    }
    }
    // Phase 2: effect tree +0x158, attachment map +0x38C, output position +0xB8.
    void* key;
    TreeNode* effectNode;
    TreeNode* effectSentinel;
    TreeNode* attachmentSentinel;
    effectNode = trees->sequenceFirst;
    attachmentSentinel = reinterpret_cast<TreeNode*>(&trees->sequenceAttachmentRoot);
    effectSentinel = reinterpret_cast<TreeNode*>(&trees->sequenceRoot);
    for (; effectSentinel != effectNode; ) {
        key = effectNode->value;
        TreeNode* parent;
        TreeNode* node;
        parent = reinterpret_cast<TreeNode*>(&trees->sequenceAttachmentRoot);
        node = trees->sequenceAttachmentRoot;
        FindHudEffectAttachment(parent, node, key, attachmentSentinel);
        WVector result;
        if (parent != attachmentSentinel && CPvsFxBox_ComputeMeanAttachmentPosition(&((AttachmentNodeView*)parent)->attachment, &result))
            ((SequencePositionView*)key)->position = result;
        AdvanceHudEffectNode(effectNode, node);
    }
}

#pragma use_lmw_stmw reset
// Test whether a vertical step between two positions touches or crosses terrain, returning the current ground height.
// Positions and verticalStep use world units (Y is height). groundHeightInOut
// must initially lie in the configured valid range; otherwise return 0 unchanged.
// After valid queries it always receives current ground Y. Return 1 means within
// tolerance, a crossing since previousPosition, or a crossing by the signed step.
// Query contract: GetGroundPoint (course_collision_and_ball_physics.cpp) is a by-value Vec3 return through
// the hidden output argument used here. Arguments (1,0,1) mean useFloorOnMiss,
// no triangle-ID output, and clampRaisedRegions. A miss writes surface+0x68's
// floor Y, then raised-region/minimum-Y clamps still apply; both output Y lanes
// are valid even without a triangle hit. This is not a success-status API.
// Require a live CQuadTree with loaded groundSurface, valid candidate/index and
// groundRegions storage, finite positions and finite floor/plane data. See
// CQuadTree_GetMeshList/FindHighestGroundPointInCandidates there.
// No null/unloaded-tree recovery exists; the initial +/-99 gate only validates
// the incoming cached height. It does not establish terrain-query readiness.
int CPvsFxBox_TestGroundCrossing(void*, float verticalStep, float* previousPosition, float* currentPosition, float* groundHeightInOut) {
    if (groundHeightInOut[0] > kEffectGroundHeightUpperLimit || groundHeightInOut[0] < kEffectGroundHeightLowerLimit) return 0;

    // Phase 1: query terrain independently below the previous and current positions.
    float previousGroundPoint[3];
    Vec3u previousPointBits(reinterpret_cast<unsigned int*>(previousPosition)[0], reinterpret_cast<unsigned int*>(previousPosition)[1], reinterpret_cast<unsigned int*>(previousPosition)[2]);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(previousGroundPoint, CQuadTree_GetInstance(), &previousPointBits, 1, 0, 1);
    float previousGroundHeight = previousGroundPoint[1];
    float currentGroundPoint[3];
    Vec3u currentPointBits(reinterpret_cast<unsigned int*>(currentPosition)[0], reinterpret_cast<unsigned int*>(currentPosition)[1], reinterpret_cast<unsigned int*>(currentPosition)[2]);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(currentGroundPoint, CQuadTree_GetInstance(), &currentPointBits, 1, 0, 1);
    float currentGroundHeight = currentGroundPoint[1];
    float heightDifference = currentPosition[1] - currentGroundHeight;
    int withinGroundTolerance = 0;
    groundHeightInOut[0] = currentGroundHeight;
    if (heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound) withinGroundTolerance = 1;
    // Phase 2: test current contact, previous-side crossing, then the proposed Y step.
    if (!withinGroundTolerance) {
        if (currentPosition[1] < groundHeightInOut[0]) {
        heightDifference = previousPosition[1] - previousGroundHeight;
        withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound;
        if (!withinGroundTolerance) {
            if (!(previousPosition[1] < previousGroundHeight)) return 1;
        }
        heightDifference = currentPosition[1] + verticalStep - groundHeightInOut[0];
        withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound;
        if (withinGroundTolerance) return 1;
        if (currentPosition[1] + verticalStep > groundHeightInOut[0]) return 1;
        return 0;
    } else {
        heightDifference = previousPosition[1] - previousGroundHeight;
        withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound;
        if (!withinGroundTolerance) {
            if (!(previousPosition[1] > previousGroundHeight)) return 1;
        }
        heightDifference = currentPosition[1] - verticalStep - groundHeightInOut[0];
        withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound;
        if (withinGroundTolerance) return 1;
        if (currentPosition[1] - verticalStep < groundHeightInOut[0]) return 1;
        return 0;
        }
    } else {
        return 1;
    }
}

// Clear owned child pointers before constructing the hole-information HUD set.
void* CHoleInfoSet_Init(void* self) {
    unsigned int* data = static_cast<unsigned int*>(self);
    data[0] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    CHoleInfoSet_Ctor(self);
    return self;
}

// Find an insertion parent and predecessor for an unsigned HUD-map key, returning side and predecessor flags.
// Return is a borrowed node or the root-pointer sentinel. predecessorOut is the
// last node <= key; leftOut chooses the parent link, flagOut stays 1 only when no
// predecessor was encountered (insertion becomes the first element).
// All arguments are required; key/output storage must not alias mutable tree
// links. Equal keys walk RIGHT and update predecessor, so a later insertion can
// append a duplicate key. This is not an insert-or-assign search.
void* HudFontConfigMap_FindInsertionPosition(void* tree, unsigned int* key, void** predecessorOut, unsigned char* leftOut, unsigned char* flagOut) {
    struct Tree { unsigned int pad0; unsigned int* node; };
    Tree* t = static_cast<Tree*>(tree);
    *predecessorOut = 0;
    void* parent = &t->node;
    unsigned int* node = t->node;
    *leftOut = 1;
    *flagOut = 1;
    while (node != 0) {
        unsigned int value = *key;
        parent = node;
        if (value < node[3]) {
            node = reinterpret_cast<unsigned int*>(node[0]);
            *leftOut = 1;
        } else {
            *predecessorOut = node;
            node = reinterpret_cast<unsigned int*>(node[1]);
            *leftOut = 0;
            *flagOut = 0;
        }
    }
    return parent;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462100", _ctors_u801005E4_0, HudFontConfigRegistry_Initialize);
#pragma pop
