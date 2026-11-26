// Sun disc, sun rays and lens-flare effects: their course resources, phase advance, and drawing.
// The item window (CItemWindow) and letter (CLetter) actors are registered and cleaned up here too.

#include <decomp/static_init.h>
// Course sun disc / sun rays / lens-flare resources and drawing (0x80192C70),
// plus CItemWindow registration/destruction and CLetter initialization/cleanup.
// CPvsFxBox in pvsfxbox_sky_and_wind_particles.cpp owns the auxiliary effect and supplies direction + dt.
// Mod lookup (float pools: the data section; preserve exact stored precision):
// - Sun rays OFF: PvsFxAuxiliaryResources_Draw phase4 emits five rotating accent
//   disc and lens flares. A future edit must decide whether to keep RNG draws too:
//   five rand() calls here affect later users of the shared C RNG.
// - Random sizes: accent = 2*(1+(rand()%800-400)/1000), range [1.2,2.798];
//   highlights = 1+(rand()%500)/10000, range [1,1.0499]. The companion's centered
//   highlight-jitter comment is incorrect: there is NO subtraction of 250 here.
//   Jitter is rerolled PER DRAW, so additional render calls change flicker/RNG.
// - World size: bitmap dimensions times [8,1.5,1.5,1] for base/center/outer/accent
//   (kCourseEffectSpriteDimensionScaleWords, .rodata 0x8046E850). Base and outer
//   are doubled at draw. All pivots use half extents; loaded dimensions are cached.
// - Widescreen/culling: outer viewport fractions [-1.4,2.4], inner [-0.4,1.4],
//   then 0.7*outer-sprite-dimension margin. Screen positions are viewport pixels;
//   stored sprite dimensions are world sizes. Preserve this existing mixed-space
//   culling test when changing projection; there is no aspect-ratio scalar here.
// - Lens-flare extent: square pixels = (viewportWidth/640)*viewportWidth*size*jitter.
//   kCourseFlareReferenceViewportWidth is the 640-pixel reference. Width appears TWICE; a
//   widescreen mod must consider both factors and highlight placement together.
// - Phase: PvsFxAuxiliaryResources_UpdatePhase (ABI PvsFxAuxiliaryResources_AdvancePhase;
//   symbol name kept as-is) advances at pi radians/second and subtracts 120*pi once above
//   the threshold. The five divisors +/-20,+/-30,60 convert that phase to angles;
//   they are not fps constants. Preserve phase wrap, draw order and float order.
// - Depth: effect origin is camera position + 1000*direction in world units;
//   temporary near/far are 3/100000, restored before every post-override exit.
//   The same 1000 symbol also normalizes accent jitter. Separate a future tuning
//   setting before changing only depth or only jitter; safe ranges are unestablished.
// - Letter fade/blink (not sun phase): OnPreLoadInit +0x5C is .5 SECONDS per
//   opacity ramp, used as 255/duration * dt by CLetter_OnProcess (golf_letter_overlays.cpp).
//   Larger duration fades more slowly; require positive duration for that path.
//   +0xA8 is elapsed seconds, advanced even without a current letter; OnDisplay
//   toggles the replay blink when >1 and resets to .5, so after the first toggle
//   the nominal half-cycle is .5s plus draw/update quantization. HandleMsg REPLAY
//   also sets .5 (main_frame_letter_and_mouse_cursor.cpp). Keep its threshold/reset/message writer together.
//   Scripted-message holding uses a separate integer call counter at +0x6C;
//   changing the fade duration does not change scripted hold or BP script speed.
// All methods require a valid receiver except deleting destructors (null accepted).
// Draw requires positive viewport extents, a valid camera and loaded handles.
// Course-load input is borrowed, finite nonzero XYZ; its magnitude is discarded
// by WVector::Normalize. The normalized direction is stored in the receiver's
// first three floats. Draw places the center at camera + 1000*storedDirection,
// so all positive multiples of an input direction yield the same placement and
// alpha; negative multiples reverse placement. A zero/NaN input has no supported
// normalization contract. Bypassing Load requires a unit-length stored direction:
// length L would place the center 1000*L world units away and change directional
// alpha through abs(Y). No normalization is performed per draw.
// Overlays/sprites are owned
// until Dtor; temporary bitmap IDs are released after dimension queries. Rendering
// mutates sprite/GX state and consumes RNG; only camera clip distances are restored.
// CLetter's list owns payload wrappers/resources and recycles links on OnDestroy;
// dtor_80193A74 later frees allocation blocks/hash storage. Do not free borrowed
// iterator results separately. CItemWindow's resource teardown is in item_window_hud.cpp.
#include "classes/CLetter.h"
#include "classes/WView.h"
// Cast-only views preserve the original access expressions and pooled-symbol
// identities; schemas live in the shared headers, with no ownership change.
#define AsWView(p) ((WView*)(p))
#include "classes/PvsFxAuxiliaryResourcesView.h"
// Handle-array prefix, also used on four-byte cursors during teardown.
#define AsOwnedResources80192FFC(p) ((PvsFxAuxiliaryResourcesView*)(p))
#include "game/itemwindow.h"
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/types.h"
#include "classes/CBpCtrl.h"
#include "classes/W3dSpr.h"
enum CourseEffectResourceLayout {
    kCourseOverlayCount = 2,
    kCourseSpriteCount = 4,
    kOverlayHandleArrayBytes = 8,
    kSpriteHandleArrayBytes = 16,
    kCourseOverlayDrawOptions = 0x2200 // WOverlay_SetCoordMode -> WOverlay::coordMode
};
static const unsigned int kFloatSignBit = 0x80000000u;
static const unsigned int kWhiteRgbBits = 0x00FFFFFFu;

extern "C" {

// Views of course selection bytes currently covered by padding in the shared headers.
struct CourseResourceSelection {
    char reserved0[0x9C6];
    unsigned char weatherVariant;
    char reserved9C7[0x1D];
    char courseResourceName[0x20];
};
// +0x3B7 is a course-file hole number (golf_bg_hole_resources.cpp), not currentHoleIndex at +0x3B6.
// The +0x3B7 byte is the displayed hole number used in resource paths.
struct CourseHoleSelection {
    char reserved0[0x3B7];
    unsigned char holeNumber;
};

extern float s_letterAngularPeriod;
extern const float kFloat_6_2831855;
extern const float kFloat_0_5_Rival;
extern const float kFloat_0_Letter;
extern float s_itemWindowAngularPeriod;
extern void* s_itemWindowTypeDescriptor;
extern void* s_actorTypeInfo;
extern void* g_resourceManager;
extern const char s_itemWindowActorFactoryName[];
extern const char s_itemWindowTaskFactoryName[];
extern char itemWindowStrings[];
// Four native big-endian float words: base8, center1.5, outer1.5, accent1.
// Definition remains in the split original .rodata, address 0x8046E850.
extern unsigned int kCourseEffectSpriteDimensionScaleWords[];
void* IntPair_Set(void*, const char*, void*);
void* ActorFactoryRegistry_GetInstance();
void StringMap_SetByName(void*, void*, const char*);
void CItemWindow_Create();
void* TaskRegistry_GetInstance();
void TaskRegistry_AppendFactory(void*, const char*);
void Object_DeleteVirtual(void*, void*);
void ResourceManager_DeleteResource(void*, void*);
void __dl__FPv(void*);
void CBpCtrl_dtor();
void __register_global_object(void*, void*, void*);
void OverlayMapTree_FreeSubtree(void*, void*);
void* __dt__18NativeBase80201D18Fv(void*, int);
void OSLockMutex(void*);
void OSUnlockMutex(void*);
void appFreeUni__FPv(void*);
void __ct__18NativeBase80201D18Fv(void*);
void OSInitMutex(void*);
void* appMallocUni__FUl(unsigned int);
void IActor_AddSkipFlags(void*, int);
void* WOverlay_Create(void*, const char*, int);
void* W3dSpr_Create(void*, const char*, int);
int ResourceManager_LoadBitmap(void*, const char*, int, int, int);
int ResourceManager_GetBitmapWidth(void*, int);
int ResourceManager_GetBitmapHeight(void*, int);
void ResourceManager_ReleaseId(void*, int);
void __as__7WVectorFRC7WVector(void*, void*);
void Normalize__7WVectorFv(void*);
void __ml__FRC7WVectorf(void*, void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void* Matrix_TransformPoint(void*, void*, void*);
void WView_ProjectViewPoint(void*, void*, void*, int);
void Camera_SetClip(void*, float, float, int);
void Billboard_DrawOrientedQuad(void*, void*, unsigned int, int);
void Particle_DrawRotatedBillboard(void*, void*, float, unsigned int);
int rand();
void WOverlay_DrawTransformedRect(void*, void*, void*, float, void*, unsigned int, unsigned int, int);
extern const float kFloat_1000_CourseAuxiliary;
extern const float kCourseEffectViewportMargin1_4;
extern const float kCourseEffectOuterViewportMaximumFactor;
extern const float kFloat_1_ItemEffects;
extern const float kItemEffectCameraNearClip;
extern const float kFloat_100000_ItemEffects;
extern const float kFloat_2_ItemEffects;
extern const float kCourseEffectInnerViewportNegativeMargin;
extern const float kCourseEffectSpriteCullingMargin;
extern const float kFloat_1_4137167_ItemEffects;
extern const float kFloat_20_LetterSprite;
extern const float kFloat_0_7853982_LetterSprite;
extern const float kFloat_neg20_ItemWindow;
extern const float kFloat_3_926991_ItemWindow;
extern const float kFloat_30_ItemEffects;
extern const float kFloat_2_984513_ItemEffects;
extern const float kFloat_neg30_ItemEffects;
extern const float kFloat_60_ItemEffects;
extern const float kCourseFlareReferenceViewportWidth;
extern const float kCourseFlareSizeJitterDivisor;
extern void* g_sharedDoc;
struct FlareHighlight {
    unsigned int overlayIndex;
    float centerOffsetFactor;
    float widthFraction;
    unsigned int tintARGB;
};
// Seven 0x10-byte runtime rows at .data 0x804AF238, not serialized pointers:
// (overlay index, signed center-offset multiplier, viewport size fraction, ARGB).
// Baseline (index,pos,size): (0,.5,.03),(0,.4,.05),(0,.2,.032),
// (0,-.47,.03),(0,-.65,.06),(0,-1,.2),(1,-1.3,.4). Tint remains per-row.
// Negative pos reflects the flare through viewport center; size controls extent.
extern FlareHighlight s_courseFlareHighlights[7]; // ABI name; these are textured overlays.
extern void* g_resourceManager;
extern unsigned char s_bpCtrlInitialized;
extern const float kCourseEffectDirectionalAlphaBias;
extern const float kFloat_0_5_LetterSprite;
extern const float kCourseEffectAlphaByteScale;
extern const float kFloat_0_TransformedText;
extern const char* s_courseEffectOverlayNames[2];
extern const char* s_courseEffectSpriteNames[4];
extern const char __vt__7CLetter[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_ItemWindow[];

// Register item-window actor and task factories and initialize the full-turn constant.
void CItemWindow_RegisterType() {
    float tau = kFloat_6_2831855;
    s_itemWindowAngularPeriod = tau;
    char* base = itemWindowStrings + 0x116;
    IntPair_Set(&s_itemWindowTypeDescriptor, base, &s_actorTypeInfo);
    void* object = ActorFactoryRegistry_GetInstance();
    StringMap_SetByName(object, (void*)CItemWindow_Create, s_itemWindowActorFactoryName);
    object = TaskRegistry_GetInstance();
    TaskRegistry_AppendFactory(object, s_itemWindowTaskFactoryName);
}

struct OverlayTreeNode {
    OverlayTreeNode* left;
    OverlayTreeNode* right;
};
struct LetterListNode {
    void* object;
    char payload[8];
    LetterListNode* previous;
    LetterListNode* next;
};

struct Obj_80192FFC {
    char reserved00[0x14];
    void* first[2];
};

extern void* IntPair_Set(void*, const char*, void*);

#pragma push
#pragma use_lmw_stmw on
// Release the two overlays and four sprites owned by the course auxiliary effect.
// destroy > 0 frees self after contents; returned address is then dangling.
// Without a resource manager handles remain untouched, matching shutdown behavior.
Obj_80192FFC* PvsFxAuxiliaryResources_Dtor(Obj_80192FFC* self, int destroy) {
    if (self != 0) {
        int zero = 0;
        char* p = (char*)self;
        int i = 0;
        while (i < kCourseOverlayCount) {
            void* manager = g_resourceManager;
            if (manager != 0) {
                void* node = AsOwnedResources80192FFC(p)->firstResource;
                if (node != 0) {
                    Object_DeleteVirtual(manager, node);
                    AsOwnedResources80192FFC(p)->firstResource = (void*)zero;
                }
            }
            ++i;
            p += 4;
        }
        unsigned int j = 0;
        char* q = (char*)self;
        while (j < kCourseSpriteCount) {
            void* manager = g_resourceManager;
            if (manager != 0) {
                void* node = AsOwnedResources80192FFC(q)->secondResource;
                if (node != 0) {
                    Object_DeleteVirtual(manager, node);
                    AsOwnedResources80192FFC(q)->secondResource = (void*)zero;
                }
            }
            ++j;
            q += 4;
        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

struct WInstanceLock {
    void* p;
    // Acquire the borrowed name-hash mutex for this scope.
    WInstanceLock(void* pp) : p(pp) {
        OSLockMutex(pp);
    }
    // Release the mutex acquired by this scope guard.
    ~WInstanceLock() {
        OSUnlockMutex(p);
    }
};

struct Obj_80193A74 {
    char reserved00[8];
    void* head;
    char reserved0c[0x10];
    void* owned;
    char listMutex[0x20];
};

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Unlink and free mutex-protected list nodes and their backing allocation.
Obj_80193A74* dtor_80193A74(Obj_80193A74* self, int destroy) {
    if (self != 0) {
        {
            WInstanceLock lock((char*)self + (int)&((Obj_80193A74*)0)->listMutex);
            if (self->head != 0) {
                void* newHead;
                void* lockPtr = (char*)self + (int)&((Obj_80193A74*)0)->listMutex;
                void* node;
                while ((node = self->head) != 0) {
                    OSLockMutex(lockPtr);
                    ((LetterListNode*)node)->next->previous = ((LetterListNode*)node)->previous;
                    ((LetterListNode*)node)->previous->next = ((LetterListNode*)node)->next;
                    newHead = ((LetterListNode*)node)->next;
                    if (newHead == node)
                        newHead = 0;
                    OSUnlockMutex(lockPtr);
                    self->head = newHead;
                    appFreeUni__FPv(node);
                }
            }
            if (self->owned != 0) {
                appFreeUni__FPv(self->owned);
                self->owned = 0;
            }
        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma pop

// Initialize the letter module's full-turn constant.
void CLetter_InitializeAngularConstant() {
    s_letterAngularPeriod = kFloat_6_2831855;
}

struct Obj_80193B4C {
    char reserved00[0x5C];
    float messageFadeDurationSeconds;
    unsigned char messageDisplayEnabled;
    char reserved61[0x47];
    float replayBlinkTimer;
};

// Enable message display and initialize fade scale and replay-icon blink time.
// +0x5C is fade DURATION, not a dimensionless scale;
// see the lookup above. dt comes from main-frame reciprocal updates-per-second
// (main_frame_letter_and_mouse_cursor.cpp), so .5 and the blink thresholds share seconds. Resources are untouched.
void CLetter::OnPreLoadInit() {
    Obj_80193B4C* self = (Obj_80193B4C*)this;
    self->messageFadeDurationSeconds = kFloat_0_5_Rival;
    self->messageDisplayEnabled = 1;
    self->replayBlinkTimer = kFloat_0_Letter;
}

struct Obj_80193B68 {
    char reserved00[0x58];
    int messageState;
    char reserved5C[0xC];
    int scriptedMessageIndex;
    int completedScriptHoldUpdates;
    float knownMessageValue;
    int textAlignment;
};

// Reset message selection and timing, and select right-aligned text.
// Protocol shared with CLetter_HandleMsg (main_frame_letter_and_mouse_cursor.cpp) and OnProcess/OnDisplay
// (golf_letter_overlays.cpp): +58 state 0=hidden, 1=fade-in, 2=hold, 3=fade-out; +68 index 0
// selects registered overlay fading, 1..15 selects s_letterAnimationScriptTriplets
// fade-in/out BP names after SHOW matches s_letterKnownMessageNames. +6C is the
// completed-script hold UPDATE counter (compare before increment, reset after hold).
// +70 caches SHOW's optional float value for known names (zero if absent/unknown);
// no read of this cache is recovered in the letter sources. It is not the hold
// timer: the selected overlay separately receives messageValue at +0x24, consumed
// as Effect::time by OnProcess (golf_letter_overlays.cpp, Holding branch): for an un-scripted
// overlay, a positive value is remaining hold SECONDS, decremented by dt only
// while Holding. Fade-out begins only if subtraction crosses strictly below 0;
// exact zero sticks, and initial <=0 disables timed dismissal. Use finite values
// and the normal nonnegative seconds delta; no validation/clamping is supplied.
// Input dismissal remains independent. For scripted names the +70 cache has no
// recovered consumer; do not apply the overlay's seconds meaning to that cache.
// Captions instead use overlay options at +0x4C.
// Preserve the write-only cache without inventing a unit for its payload. This
// reset chooses no known script, zero value/counter and alignment 2=right. It
// does not clear currentMessage, alpha, .5s duration, elapsed blink or resources;
// resource loading/SHOW must establish them before drawing. No message is sent.
void CLetter::OnInit() {
    Obj_80193B68* self = (Obj_80193B68*)this;
    float zero = kFloat_0_Letter;
    self->messageState = 0;
    self->textAlignment = 2;
    self->scriptedMessageIndex = 0;
    self->knownMessageValue = zero;
    self->completedScriptHoldUpdates = 0;
}

struct LetterView {
    void* vtable;
    char actorHeader[4];
    int displayPriority;
    char reserved0C[0x0C];
    int currentMessage;
    int activeHead;
    int iterator;
    int nodeAllocationHead;
    int freeHead;
    int nodeBlockSize;
    int nameHashMask;
    int nameHashBucketCount;
    void* nameHashBuckets;
    char reserved3C[0x28];
    int bpEntryId;
    char reserved68[0x34];
    int titleFont;
    int fastForwardIcon;
    int replayIcon;
};

// Initialize the letter actor, its mutex-protected name hash and resource handles.
CLetter::CLetter() {
    LetterView* self = (LetterView*)this;
    __ct__18NativeBase80201D18Fv(self);
    int zero = 0;
    self->currentMessage = zero;
    self->vtable = (void*)__vt__7CLetter;
    self->nameHashBuckets = (void*)zero;
    OSInitMutex(((CLetter*)self)->nameHashMutex);
    OSLockMutex(((CLetter*)self)->nameHashMutex);
    self->activeHead = zero;
    self->freeHead = zero;
    self->nodeAllocationHead = zero;
    self->nodeBlockSize = 0x20;
    self->iterator = zero;
    self->nameHashBucketCount = 0x10;
    if (self->nameHashBucketCount > 0) {
        self->nameHashBuckets = appMallocUni__FUl(0x40);
        self->nameHashMask = self->nameHashBucketCount - 1;
        int count = 0;
        int offset = 0;
        while (count < self->nameHashBucketCount) {
            *(int*)((char*)self->nameHashBuckets + offset) = zero;
            ++count;
            offset += 4;
        }
    } else {
        self->nameHashBuckets = (void*)zero;
    }
    OSUnlockMutex(((CLetter*)self)->nameHashMutex);
    self->titleFont = zero;
    self->fastForwardIcon = zero;
    self->replayIcon = zero;
    self->bpEntryId = -1;
    self->displayPriority = 4;
    // IActor_Skipped (task_registry_actor_and_debug_text.cpp) tests actor.skipMask against *g_actorSkipMask.
    // Golf-task setup (golf_task_loading_and_pause.cpp) points that global at CGolfDoc.gameStateFlags;
    // 0x2000 is the hole-results state (CGolfRule::EnterState, golf_rule_round_flow_and_shot_sim.cpp). This
    // opts the letter actor into scheduler skipping during results; it neither
    // clears a letter nor changes its local messageState. Outside golf the
    // global may be null, in which case this bit alone never skips the actor.
    IActor_AddSkipFlags(self, 0x2000);
}

struct WOverlayView {
    virtual void Vfn0(); // +08: uncalled destructor slot
    virtual void setDrawingOptions(int);
    // +0C: WOverlay_SetCoordMode stores coordMode
};

struct Obj_801930BC {
    float directionX, directionY, directionZ;
    float phaseRadians;
    float alpha;
    void* resource[2];
    void* pane[4];
    float dimensions[8];
};

// Load course overlays/sprites, cache dimensions and derive alpha from the normalized direction.
// Controls sprite world sizes/alpha; assumes empty handles and successful resource
// creation (no null recovery). Copies/normalizes arg's three floats; preserves input.
// Alpha = .5*abs(directionY)+.2; packed high-byte truncation uses 255*alpha.
// coordMode=0x2200 leaves the anchor, position/size scaling and pixel-snap bits
// clear in overlay_sprite_drawing.cpp's rectangle path; the two set bits retain their numeric identity.
void PvsFxAuxiliaryResources_LoadCourseParameters(Obj_801930BC* self, void* arg) {
    unsigned int dimensionScaleWords[4];
    unsigned int* dimensionScaleSource = kCourseEffectSpriteDimensionScaleWords;
    dimensionScaleWords[0] = *dimensionScaleSource++;
    dimensionScaleWords[1] = *dimensionScaleSource++;
    dimensionScaleWords[2] = *dimensionScaleSource++;
    dimensionScaleWords[3] = *dimensionScaleSource++;
    for (unsigned int i = 0; i < kCourseOverlayCount; ++i) {
        self->resource[i] = WOverlay_Create(g_resourceManager, s_courseEffectOverlayNames[i], 0);
        ((WOverlayView*)self->resource[i])->setDrawingOptions(kCourseOverlayDrawOptions);
    }
    Obj_801930BC* spriteSlotCursor = self;
    const char** spriteNameCursor = s_courseEffectSpriteNames;
    float* dimensionScaleCursor = (float*)dimensionScaleWords;
    Obj_801930BC* dimensionSlotCursor = self;
    for (unsigned int i = 0; i < kCourseSpriteCount; ++i) {
        spriteSlotCursor->pane[0] = W3dSpr_Create(g_resourceManager, *spriteNameCursor, 0);
        int handle = ResourceManager_LoadBitmap(g_resourceManager, *spriteNameCursor, 0, 0, 0);
        int width = ResourceManager_GetBitmapWidth(g_resourceManager, handle);
        float w = *dimensionScaleCursor * (float)width;
        int height = ResourceManager_GetBitmapHeight(g_resourceManager, handle);
        dimensionSlotCursor->dimensions[0] = w;
        dimensionSlotCursor->dimensions[1] = *dimensionScaleCursor * (float)height;
        if (g_resourceManager != 0 && handle != 0)
            ResourceManager_ReleaseId(g_resourceManager, handle);
        ++spriteNameCursor;
        spriteSlotCursor = (Obj_801930BC*)((char*)spriteSlotCursor + 4);
        ++dimensionScaleCursor;
        dimensionSlotCursor = (Obj_801930BC*)((char*)dimensionSlotCursor + 8);
    }
    *(WVector*)self = *(WVector*)arg;
    (*(WVector*)self).Normalize();
    float absoluteDirectionY;
    if (*(unsigned int*)&self->directionY & kFloatSignBit) {
        absoluteDirectionY = -self->directionY;
    } else {
        absoluteDirectionY = self->directionY;
    }
    float alpha = kFloat_0_5_LetterSprite * absoluteDirectionY + kCourseEffectDirectionalAlphaBias;
    int outerSpriteHandle = (int)self->pane[2];
    float outerScaledAlpha = kCourseEffectAlphaByteScale * alpha;
    self->alpha = alpha;
    int outerAlphaByte = (int)outerScaledAlpha;
    unsigned int packedAlphaBits = (unsigned int)outerAlphaByte << 24;
    unsigned int color = packedAlphaBits | kWhiteRgbBits;
    ((W3dSpr*)outerSpriteHandle)->SetColor(color);
    float accentScaledAlpha = kCourseEffectAlphaByteScale * self->alpha;
    int accentSpriteHandle = (int)self->pane[3];
    int accentAlphaByte = (int)accentScaledAlpha;
    packedAlphaBits = (unsigned int)accentAlphaByte << 24;
    color = packedAlphaBits | kWhiteRgbBits;
    ((W3dSpr*)accentSpriteHandle)->SetColor(color);
    self->phaseRadians = kFloat_0_TransformedText;
}

extern const float kCourseAuxiliaryPhaseRadiansPerSecond;
extern const float kCourseAuxiliaryPhaseWrapRadians;

struct Obj_801932F8 {
    char reserved00[0xC];
    float phaseRadians;
};

// PvsFxAuxiliaryResources_UpdatePhase: advance sun-ray phase by delta SECONDS.
// The symbol below keeps its AdvancePhase name. Negative/large deltas
// are not normalized modulo a turn; preserve the strict > test and single subtraction.
void PvsFxAuxiliaryResources_AdvancePhase(Obj_801932F8* self, float delta) {
    self->phaseRadians = kCourseAuxiliaryPhaseRadiansPerSecond * delta + self->phaseRadians;
    if (self->phaseRadians > kCourseAuxiliaryPhaseWrapRadians) {
        self->phaseRadians = self->phaseRadians - kCourseAuxiliaryPhaseWrapRadians;
    }
}

extern void* memset(void*, int, unsigned int);

struct Obj_80192FB0 {
    char reserved00[0x2C];
};

// Clear both arrays of owned overlay/sprite handles.
Obj_80192FB0* PvsFxAuxiliaryResources_Ctor(Obj_80192FB0* self) {
    memset(&AsOwnedResources80192FFC(self)->firstResource, 0, kOverlayHandleArrayBytes);
    memset(&AsOwnedResources80192FFC(self)->secondResource, 0, kSpriteHandleArrayBytes);
    return self;
}

// Free both unexpanded child subtrees, then the current overlay-tree node.
static inline void FreeOverlayNodeAndSubtrees(CItemWindow* self, void* branchNode) {
    void* leftSubtree = ((OverlayTreeNode*)branchNode)->left;
    if (leftSubtree != 0)
        OverlayMapTree_FreeSubtree(&self->pad_18, leftSubtree);
    void* rightSubtree = ((OverlayTreeNode*)branchNode)->right;
    if (rightSubtree != 0)
        OverlayMapTree_FreeSubtree(&self->pad_18, rightSubtree);
    appFreeUni__FPv(branchNode);
}

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Free the overlay search tree, destroy the actor base and optionally delete the object.
// Payload owner is CItemWindow, not the tree-node destructor: OnDestroy in
// item_window_hud.cpp visits ItemOverlayTreeNode::overlay, calls Object_DeleteVirtual via
// g_resourceManager, clears it, THEN frees nodes/reset sentinels and other UI
// resources. That lifecycle hook must precede this node-only destructor when
// overlays were loaded. Direct destruction with live payloads leaks them; this
// function neither substitutes for OnDestroy nor transfers them to the manager.
// The normal release path requires the resource manager still to be available.
CItemWindow::~CItemWindow() {
    // Repeated address guards are emitted nested-member destructor tests.
    // Keep their control flow; cursors follow the unrolled postorder walk.
    void *branchCursorA, *branchCursorB, *branchCursorC, *rootNode;
    if (&pad_18 != 0) {
        if (&pad_18 != 0) {
            if (&pad_18 != 0) {
                if (&pad_18 != 0) {
                    if (&pad_18 != 0) {
                        if (&pad_18 != 0) {
                            rootNode = overlayRoot;
                            if (rootNode != 0) {
                                branchCursorC = ((OverlayTreeNode*)rootNode)->left;
                                if (branchCursorC != 0) {
                                    branchCursorB = ((OverlayTreeNode*)branchCursorC)->left;
                                    if (branchCursorB != 0) {
                                        branchCursorA = ((OverlayTreeNode*)branchCursorB)->left;
                                        if (branchCursorA != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorA);
                                        }
                                        branchCursorA = ((OverlayTreeNode*)branchCursorB)->right;
                                        if (branchCursorA != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorA);
                                        }
                                        appFreeUni__FPv(branchCursorB);
                                    }
                                    branchCursorA = ((OverlayTreeNode*)branchCursorC)->right;
                                    if (branchCursorA != 0) {
                                        branchCursorB = ((OverlayTreeNode*)branchCursorA)->left;
                                        if (branchCursorB != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorB);
                                        }
                                        branchCursorB = ((OverlayTreeNode*)branchCursorA)->right;
                                        if (branchCursorB != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorB);
                                        }
                                        appFreeUni__FPv(branchCursorA);
                                    }
                                    appFreeUni__FPv(branchCursorC);
                                }
                                branchCursorA = ((OverlayTreeNode*)rootNode)->right;
                                if (branchCursorA != 0) {
                                    branchCursorB = ((OverlayTreeNode*)branchCursorA)->left;
                                    if (branchCursorB != 0) {
                                        branchCursorC = ((OverlayTreeNode*)branchCursorB)->left;
                                        if (branchCursorC != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorC);
                                        }
                                        branchCursorC = ((OverlayTreeNode*)branchCursorB)->right;
                                        if (branchCursorC != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorC);
                                        }
                                        appFreeUni__FPv(branchCursorB);
                                    }
                                    branchCursorB = ((OverlayTreeNode*)branchCursorA)->right;
                                    if (branchCursorB != 0) {
                                        branchCursorC = ((OverlayTreeNode*)branchCursorB)->left;
                                        if (branchCursorC != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorC);
                                        }
                                        branchCursorC = ((OverlayTreeNode*)branchCursorB)->right;
                                        if (branchCursorC != 0) {
                                            FreeOverlayNodeAndSubtrees(this, branchCursorC);
                                        }
                                        appFreeUni__FPv(branchCursorB);
                                    }
                                    appFreeUni__FPv(branchCursorA);
                                }
                                appFreeUni__FPv(rootNode);
                            }
                        }
                    }
                }
            }
        }
    }
    __dt__18NativeBase80201D18Fv(this, 0);
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
#pragma opt_common_subs off
// Advance the circular message-list iterator under the name-hash mutex.
static inline void* Next_80193B8C(LetterView* self) {
    void* lp = ((CLetter*)self)->nameHashMutex;
    void* item;
    OSLockMutex(lp);
    LetterListNode* node = (LetterListNode*)self->iterator;
    if (node != 0) {
        unsigned int nx = (unsigned int)node->next;
        item = node->object;
        if (nx == (unsigned int)self->activeHead)
            nx = 0;
        self->iterator = nx;
        OSUnlockMutex(lp);
    } else {
        OSUnlockMutex(lp);
        item = 0;
    }
    return item;
}

struct Item_80193B8C {
    void* p;
    // Delete the owned virtual resource and clear its wrapper pointer.
    ~Item_80193B8C() {
        if (p != 0) {
            Object_DeleteVirtual(g_resourceManager, p);
            p = 0;
        }
    }
};

// Reset the message-list iterator and return its first object.
static inline void* First_80193B8C(LetterView* self) {
    WInstanceLock lock0(((CLetter*)self)->nameHashMutex);
    self->iterator = self->activeHead;
    return Next_80193B8C(self);
}

// Delete controller/font resources, release message objects and recycle list/hash entries.
// Ownership: delete BP entry/font, then each list payload wrapper and its virtual
// resource. Intrusive list nodes survive in freeHead; hash buckets are cleared,
// not freed. Mutex scope protects list/hash links, not resource-manager calls.
// Borrower contract: CLetter_HandleMsg/LetterOverlay_FindByName (main_frame_letter_and_mouse_cursor.cpp)
// publish currentMessage/replayIcon/fastForwardIcon as raw registry payload
// pointers; FindEffect, OnProcess and OnDisplay (golf_letter_overlays.cpp) also borrow them.
// None retains a payload after unlocking. Therefore OnDestroy requires those
// message/update/draw/registration operations to be quiescent, with no borrower
// dereferencing a cached payload during or after teardown. The per-step mutex
// does NOT admit concurrent registry borrowers across this operation: payloads
// are freed before their links/hash names are removed. It also does not reset
// all cached borrowed pointers. A later use requires the load/init lifecycle,
// not reuse of a pre-destroy iterator or currentMessage. This is a serial actor
// lifecycle precondition, not a claim that the mutex joins another thread.
void CLetter::OnDestroy() {
    LetterView* self = (LetterView*)this;
    // Phase 1: unregister the singleton-owned controller entry and release the title font.
    if (self->bpEntryId != -1) {
        void* g = Sing_805DC5F0();
        if (g == 0) {

            if (((signed char)s_bpCtrlInitialized) == 0) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRegistration_ItemWindow);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        g = Sing_805DC5F0();

        ((CBpCtrl*)g)->DeleteEntryById(self->bpEntryId);
        self->bpEntryId = -1;
    }
    if (self->titleFont != 0) {
        ResourceManager_DeleteResource(g_resourceManager, (void*)self->titleFont);
        self->titleFont = 0;
    }
    // Phase 2: destroy payload resources and wrappers while advancing the active iterator.
    void* payloadWrapper;
    {
        WInstanceLock lock0(((CLetter*)self)->nameHashMutex);
        self->iterator = self->activeHead;
        payloadWrapper = Next_80193B8C(self);
    }
    while (0 != payloadWrapper) {
        if (payloadWrapper) {
            if (payloadWrapper) {
                void* ownedResource = *(void**)payloadWrapper;
                if (ownedResource) {
                    Object_DeleteVirtual(g_resourceManager, ownedResource);
                    *(void**)payloadWrapper = 0;
                }
                __dl__FPv(payloadWrapper);
            }
        }
        payloadWrapper = Next_80193B8C(self);
    }
    // Phase 3: splice the active ring into the free ring, or recycle one node at a time.
    {
        WInstanceLock lock1(((CLetter*)self)->nameHashMutex);
        int activeRingHead = self->activeHead;

        if (activeRingHead != 0 && self->freeHead != 0) {

            void* activeHeadNode = (void*)activeRingHead;
            void* activeTailNode = ((LetterListNode*)activeHeadNode)->previous;
            void* freeHeadNode = (void*)self->freeHead;
            (*(int*)&((LetterListNode*)activeTailNode)->next) = ((*(int*)&((LetterListNode*)freeHeadNode)->next));

            void* activeHeadForTail = (void*)self->activeHead;
            void* freeHeadForTail = (void*)self->freeHead;

            (*(int*)&((LetterListNode*)freeHeadForTail)->next->previous) =
                ((*(int*)&((LetterListNode*)activeHeadForTail)->previous));

            void* freeHeadForSplice = (void*)self->freeHead;
            void* activeHeadForSplice = (void*)self->activeHead;
            (*(int*)&((LetterListNode*)freeHeadForSplice)->next) = (int)activeHeadForSplice;
            void* freeHeadForBacklink = (void*)self->freeHead;
            void* activeHeadForBacklink = (void*)self->activeHead;
            (*(int*)&((LetterListNode*)activeHeadForBacklink)->previous) = (int)freeHeadForBacklink;
            self->activeHead = 0;
        } else if (activeRingHead) {
            void* nextActiveHead;
            void* recycledNode;
            void* freeRingHead;
            void* listMutex = ((CLetter*)self)->nameHashMutex;
            while ((recycledNode = (void*)self->activeHead) != 0) {
                OSLockMutex(listMutex);
                ((LetterListNode*)recycledNode)->next->previous = ((LetterListNode*)recycledNode)->previous;
                ((LetterListNode*)recycledNode)->previous->next = ((LetterListNode*)recycledNode)->next;
                nextActiveHead = ((LetterListNode*)recycledNode)->next;
                if (nextActiveHead == recycledNode)
                    nextActiveHead = 0;
                OSUnlockMutex(listMutex);
                recycledNode = (void*)self->activeHead;
                freeRingHead = (void*)self->freeHead;
                OSLockMutex(listMutex);
                if (freeRingHead == 0) {
                    ((LetterListNode*)recycledNode)->previous = (LetterListNode*)recycledNode;
                    ((LetterListNode*)recycledNode)->next = (LetterListNode*)recycledNode;
                    OSUnlockMutex(listMutex);
                } else {
                    ((LetterListNode*)recycledNode)->next = (LetterListNode*)freeRingHead;
                    ((LetterListNode*)recycledNode)->previous = ((LetterListNode*)freeRingHead)->previous;
                    ((LetterListNode*)freeRingHead)->previous = (LetterListNode*)recycledNode;
                    ((LetterListNode*)recycledNode)->previous->next = (LetterListNode*)recycledNode;
                    OSUnlockMutex(listMutex);
                    recycledNode = freeRingHead;
                }
                self->freeHead = (int)recycledNode;
                self->activeHead = (int)nextActiveHead;
            }
        }
        // Phase 4: remove hash references to the now-destroyed payloads; retain bucket storage.
        int bucketIndex = 0;
        int bucketByteOffset = 0;

        while (self->nameHashBucketCount > bucketIndex) {
            void* bucketStorage = (void*)self->nameHashBuckets;
            *(int*)((char*)bucketStorage + bucketByteOffset) = (0);

            bucketIndex = ((1 + bucketIndex));
            bucketByteOffset = 4 + bucketByteOffset;
        }
    }
}
#pragma pop

typedef Vec3f Vec3_80193320;
// Backend flags verified in wgeneric3d_render_queue_and_soft_keyboard.cpp (quad blend switch/render-pass masks):
// 0x00800000 -> GX source-alpha / ONE additive blending (states 13=4,14=1).
// 0x00300000 -> late render pass; 0x20000000 -> replay-pass flag.
// Thus centered/accent layers share additive blending; the base selects late pass.
// Text's 0x00C00000 adds 0x00400000 (queued primitive) to additive alpha.
// Facing 0 looks toward the eye; 2 uses the camera axes (w3d_sprite_and_billboard.cpp).
enum CourseEffectDrawOptions {
    kCourseEffectAdditiveAlpha = 0x00800000,
    kCourseEffectLatePass = 0x00300000,
    kCourseEffectReplayPass = 0x20000000,
    kCourseEffectLayerFlags = kCourseEffectReplayPass | kCourseEffectAdditiveAlpha,
    kCourseEffectBaseFlags = kCourseEffectLayerFlags | kCourseEffectLatePass,
    kCourseEffectFlareFlags = 0x00C00000,
    kCourseEffectWeatherTint = 0xFF505050u,
    kCourseEffectFaceEye = 0,
    kCourseEffectUseViewAxes = 2,
    kAccentCount = 5,
    kHighlightCount = 7,
    kAccentJitterRange = 800,
    kAccentJitterCenter = 400,
    kHighlightJitterRange = 500 // positive-only jitter, no centering subtraction
};
struct TextPoint_80193320 {
    float x, y, z, w;
};
struct Proj_80193320 {
    float x, y, z;
    float pad[9];
    double d;
};
struct Cam_80193320 {
    char pad0[0x178];
    float nearClip;
    float farClip;
    char pad180[0x8];
    float w;
    float h;
};
struct Self_80193320 {
    float normalizedDirectionX, normalizedDirectionY, normalizedDirectionZ;
    float phaseRadians; // Same +0x0C word written by PvsFxAuxiliaryResources_AdvancePhase.
    char pad10[0x4];
    void* overlays[2];                         // Same owned handles as Obj_801930BC::resource, not text strings.
    void* baseSprite;                          // Base billboard; late pass, doubled dimensions, camera axes.
    void* centerSprite;                        // Center billboard; also used for the weather-tinted variant.
    void* outerSprite;                         // Outer billboard; doubled dimensions.
    void* accentSprite;                        // Rotating accent billboard; reused for five draws.
    float baseWidthWorld, baseHeightWorld;     // +0x2C: world units
    float centerWidthWorld, centerHeightWorld; // +0x34: world units
    float outerWidthWorld, outerHeightWorld;   // +0x3C: world units
    float accentWidthWorld, accentHeightWorld; // +0x44: world units
};

// Set the outer layer's doubled dimensions with a centered world-space pivot.
static inline void SetCourseEffectOuterSize(Self_80193320* self, float doubleFactor) {
    {
        float storedOuterWidth = self->outerWidthWorld;
        float outerScaledWidth = storedOuterWidth * doubleFactor;
        void* outerSprite = self->outerSprite;
        float storedOuterHeight = self->outerHeightWorld;
        float outerScaledHeight = storedOuterHeight * doubleFactor;
        ((W3dSpr*)outerSprite)
            ->SetRect(outerScaledWidth, outerScaledHeight, kFloat_0_5_LetterSprite * outerScaledWidth,
                      kFloat_0_5_LetterSprite * outerScaledHeight);
    }
}

// Pixel-space culling policy for accent rays, evaluated left/right/top/bottom.
// Rectangle = [-margin*w, viewportW+margin*w] x
//             [-margin*h, viewportH+margin*h], inclusive edges, margin=0.7.
// IMPORTANT legacy boundary: outerWidthWorld/outerHeightWorld are cached WORLD
// sprite sizes, but the
// existing renderer uses their numeric values as PIXEL margins without a
// projection. This adapter keeps that reading in one place and does not
// claim a physical world-to-pixel mapping. For a later
// mod, replace both horizontal edges together, and both vertical edges together
// here. Leave billboard W3dSpr sizes in world units. Keep the short-circuit
// calculation order: eagerly constructing a rectangle changes FP evaluation.
// The other gates are viewport fractions: outer [-1.4,2.4] inclusive; inner
// [-0.4,1.4] inclusive in clear weather, strict in the tinted weather branch.
// Expression adapter keeps short-circuit branches in the draw operation: an
// int-returning inline predicate materializes a Boolean and adds instructions.
// Arguments here must be side-effect-free local pointers/scalars.
#define IsOutsideAccentCullingRectanglePx(self, camera, screenX, screenY)                    \
    ((screenX) < kCourseEffectSpriteCullingMargin * -(self)->outerWidthWorld ||              \
     (screenX) > kCourseEffectSpriteCullingMargin * (self)->outerWidthWorld + (camera)->w || \
     (screenY) < kCourseEffectSpriteCullingMargin * -(self)->outerHeightWorld ||             \
     (screenY) > kCourseEffectSpriteCullingMargin * (self)->outerHeightWorld + (camera)->h)

// Project the course effect, cull layers, draw billboards and seven jittered flare overlays.
// Place the effect 1000 world units along its stored direction from camera position.
// The camera view matrix maps world to view; negative view Z is visible. Projected X/Y
// and camera w/h are pixels. Clip distances are restored on every exit after override.
// Controls sun-disc/ray/flare visibility and sizing. Preserve five conditional
// accent RNG calls followed by seven highlight RNG calls, the signed view-Z test,
// strict weather-branch edges and normal-branch inclusive edges. No handles escape.
void PvsFxAuxiliaryResources_Draw(Self_80193320* self, Cam_80193320* camera) {
    Vec3_80193320 viewPosition;

    // Dimension loads retain their original order around rendering calls;
    // layer extents are world units and flare rectangles are viewport pixels.
    Vec3_80193320 worldPosition;
    TextPoint_80193320 textureRect;
    TextPoint_80193320 screenRect;
    float matrix0[4];
    Vec3_80193320 scaledDirection;
    __ml__FRC7WVectorf(&scaledDirection, self, kFloat_1000_CourseAuxiliary);
    __pl__FRC7WVectorRC7WVector(matrix0, (char*)&AsWView(camera)->viewMatrix[9] /* 0x68: affine translation */,
                                &scaledDirection);
    *(WVector*)&worldPosition = *(WVector*)matrix0;
    Vec3_80193320 viewPositionCopy;
    Matrix_TransformPoint(&viewPositionCopy, &worldPosition,
                          (char*)AsWView(camera)->inputTransform /* 0x14: PvsFxAuxiliaryResources_Draw */);
    *(WVector*)&viewPosition = *(WVector*)&viewPositionCopy;
    Proj_80193320 projected;
    WView_ProjectViewPoint(camera, &projected, &viewPosition, 1);
    if (!(viewPosition.z < kFloat_0_TransformedText))
        return;
    float doubleFactor;
    float projectedY = projected.y;
    float oldFar;
    float screenY = projectedY;
    float oldNear;
    float savedX = projected.x;
    float screenX = savedX;
    if (screenX < kCourseEffectViewportMargin1_4 * -camera->w ||
        screenX > camera->w * kCourseEffectOuterViewportMaximumFactor ||
        screenY < kCourseEffectViewportMargin1_4 * -camera->h ||
        screenY > camera->h * kCourseEffectOuterViewportMaximumFactor) {
        return;
    }
    textureRect.y = kFloat_0_TransformedText;
    textureRect.x = kFloat_0_TransformedText;
    textureRect.w = kFloat_1_ItemEffects;
    textureRect.z = kFloat_1_ItemEffects;
    // Phase 1: retain clip distances; temporarily broaden projection for world-space layers.
    oldNear = camera->nearClip;
    oldFar = camera->farClip;
    Camera_SetClip(camera, kItemEffectCameraNearClip, kFloat_100000_ItemEffects, 0);
    doubleFactor = kFloat_2_ItemEffects;
    // Phase 2: weather variant draws only the dark center layer inside the tighter bounds.
    if (((CourseResourceSelection*)g_sharedDoc)->weatherVariant) {
        if (screenX > kCourseEffectInnerViewportNegativeMargin * -camera->w) {
            if (screenX < camera->w * kCourseEffectViewportMargin1_4) {
                if (screenY > kCourseEffectInnerViewportNegativeMargin * -camera->h) {
                    if (screenY < camera->h * kCourseEffectViewportMargin1_4) {
                        void* savedPane = self->centerSprite;
                        ((W3dSpr*)savedPane)->SetColor(kCourseEffectWeatherTint);
                        ((W3dSpr*)(self->centerSprite))->SetPos(&worldPosition);
                        float savedW = kFloat_0_5_LetterSprite * self->centerWidthWorld;
                        float layerHeight = self->centerHeightWorld;
                        float layerPivotY = kFloat_0_5_LetterSprite * self->centerHeightWorld;
                        float savedW2 = self->centerWidthWorld;
                        ((W3dSpr*)(self->centerSprite))->SetRect(savedW2, layerHeight, savedW, layerPivotY);
                        Billboard_DrawOrientedQuad(self->centerSprite, camera, kCourseEffectLayerFlags,
                                                   kCourseEffectFaceEye);
                    }
                }
            }
        }
        Camera_SetClip(camera, oldNear, oldFar, 0);
        return;
    }
    // Phase 3: normal variant draws a doubled base layer, a centered layer, then a
    // doubled outer layer. SizeAndPivot arguments are width,height,pivotX,pivotY;
    // all these layers use a half-size pivot. Scalar aliases retain evaluation order.
    ((W3dSpr*)(self->baseSprite))->SetPos(&worldPosition);
    {
        float doubledBaseWidth = self->baseWidthWorld * doubleFactor;
        float baseLayerHeight = self->baseHeightWorld;
        float doubledBaseHeight = baseLayerHeight * doubleFactor;
        float savedValue2 = kFloat_0_5_LetterSprite * doubledBaseWidth;
        void* savedPane1C = self->baseSprite;
        float savedValue3 = kFloat_0_5_LetterSprite * doubledBaseHeight;
        ((W3dSpr*)savedPane1C)->SetRect(doubledBaseWidth, doubledBaseHeight, savedValue2, savedValue3);
    }
    Billboard_DrawOrientedQuad(self->baseSprite, camera, kCourseEffectBaseFlags, kCourseEffectUseViewAxes);
    if (screenX < kCourseEffectInnerViewportNegativeMargin * -camera->w ||
        screenX > camera->w * kCourseEffectViewportMargin1_4 ||
        screenY < kCourseEffectInnerViewportNegativeMargin * -camera->h ||
        screenY > camera->h * kCourseEffectViewportMargin1_4) {
        Camera_SetClip(camera, oldNear, oldFar, 0);
        return;
    }
    ((W3dSpr*)(self->centerSprite))->SetPos(&worldPosition);
    void* centerLayer = self->centerSprite;
    ((W3dSpr*)centerLayer)
        ->SetRect(self->centerWidthWorld, self->centerHeightWorld, kFloat_0_5_LetterSprite * self->centerWidthWorld,
                  kFloat_0_5_LetterSprite * self->centerHeightWorld);
    Billboard_DrawOrientedQuad(self->centerSprite, camera, kCourseEffectLayerFlags, kCourseEffectUseViewAxes);
    void* outerLayer = self->outerSprite;
    ((W3dSpr*)outerLayer)->SetPos(&worldPosition);
    SetCourseEffectOuterSize(self, doubleFactor);
    Billboard_DrawOrientedQuad(self->outerSprite, camera, kCourseEffectLayerFlags, kCourseEffectUseViewAxes);
    if (IsOutsideAccentCullingRectanglePx(self, camera, screenX, screenY)) {
        Camera_SetClip(camera, oldNear, oldFar, 0);
        return;
    }
    // Phase 4: five rotating accent sprites use scale 2*(1 + jitter/1000),
    // jitter in [-400,399]. Each uses centered pivots and an independent angle.
    float savedPhase = self->phaseRadians;
    float angles[kAccentCount];
    angles[0] = kFloat_1_4137167_ItemEffects + self->phaseRadians / kFloat_20_LetterSprite;
    float savedPhase2 = self->phaseRadians;
    angles[1] = kFloat_0_7853982_LetterSprite + savedPhase / kFloat_neg20_ItemWindow;
    float savedPhase3 = self->phaseRadians;
    angles[2] = kFloat_3_926991_ItemWindow + self->phaseRadians / kFloat_30_ItemEffects;
    angles[3] = kFloat_2_984513_ItemEffects + savedPhase2 / kFloat_neg30_ItemEffects;
    angles[4] = savedPhase3 / kFloat_60_ItemEffects;
    void* savedPane2 = self->accentSprite;
    ((W3dSpr*)savedPane2)->SetPos(&worldPosition);
    float k1000 = kFloat_1000_CourseAuxiliary;
    float one = kFloat_1_ItemEffects;
    float half = kFloat_0_5_LetterSprite;
    // Equation for the scheduling aliases below:
    // accentScale = 2*(1 + jitter/1000), (accentWidth,accentHeight)=accentScale*cachedAccentWorldSize, pivot=(accentWidth/2,accentHeight/2).
    // Repeated loads/temporaries preserve the matched register schedule.
    for (int i = 0; i < kAccentCount; ++i) {
        float accentScale = doubleFactor * (one + (float)(rand() % kAccentJitterRange - kAccentJitterCenter) / k1000);
        float accentWidth = self->accentWidthWorld * accentScale;
        float accentHeight = self->accentHeightWorld * accentScale;
        float accentPivotX = half * accentWidth;
        float accentPivotY = half * accentHeight;
        void* accentSpriteForSize = self->accentSprite;
        ((W3dSpr*)accentSpriteForSize)->SetRect(accentWidth, accentHeight, accentPivotX, accentPivotY);
        void* accentSpriteForDraw = self->accentSprite;
        Particle_DrawRotatedBillboard(accentSpriteForDraw, camera, angles[i], kCourseEffectLayerFlags);
    }
    float restoreFarClip = oldFar;
    float restoreNearClip = oldNear;
    Camera_SetClip(camera, restoreNearClip, restoreFarClip, 0);
    // Phase 5: clip distances are restored; draw seven screen-space flare overlays.
    // centeredScreenX/centeredScreenY = projectedPixel/viewportExtent - 0.5; centerOffsetFactor scales their
    // offset from center. Extent = (viewportWidth/640)*viewportWidth*widthFraction*sizeJitter.
    float scale = camera->w / kCourseFlareReferenceViewportWidth;
    float half2 = kFloat_0_5_LetterSprite;
    float k10000 = kCourseFlareSizeJitterDivisor;
    float viewportHeight = camera->h;
    float viewportCenterFraction = half2;
    float centeredScreenY = screenY / viewportHeight - viewportCenterFraction;
    float viewportWidth = camera->w;
    for (int i = 0; kHighlightCount > i; ++i) {
        FlareHighlight* entry = &s_courseFlareHighlights[i];
        void* overlay = self->overlays[entry->overlayIndex];
        float sizeJitter = kFloat_1_ItemEffects + (float)(rand() % kHighlightJitterRange) / k10000;
        float centeredScreenX = screenX / viewportWidth - half2;
        float highlightSize = entry->widthFraction;
        float jitteredHighlightSize = sizeJitter * highlightSize;
        float widthForExtent = camera->w;
        float highlightWidthBeforeViewportScale = widthForExtent * jitteredHighlightSize;
        screenRect.z = scale * (highlightWidthBeforeViewportScale);
        float highlightExtent = screenRect.z;
        float highlightHalfExtent = half2 * highlightExtent;
        float highlightPositionScale = entry->centerOffsetFactor;
        screenRect.w = scale * (camera->w * jitteredHighlightSize);
        float widthForPosition = camera->w;
        screenRect.x = widthForPosition * (centeredScreenX * highlightPositionScale + half2) - highlightHalfExtent;
        float viewportHeight = camera->h;
        unsigned int highlightTint = entry->tintARGB;
        screenRect.y = viewportHeight * (entry->centerOffsetFactor * centeredScreenY + half2) - highlightHalfExtent;
        WOverlay_DrawTransformedRect(overlay, camera, &textureRect, kFloat_0_TransformedText, &screenRect,
                                     kCourseEffectFlareFlags, highlightTint, 0);
    }
}

#undef IsOutsideAccentCullingRectanglePx
} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462158", _ctors_u80192C70_0, CItemWindow_RegisterType);
STATIC_INIT_ENTRY(".CRT$XCU80462158", _ctors_u80192C70_1, CLetter_InitializeAngularConstant);
#pragma pop
