// The hole information HUD: the panels shown during a hole (drive, distance, power, wind, time
// remaining, time-out, balloon and the effect box CPvsFxBox), grouped by CHoleInfoSet.
//
// Also holds the HUD font configuration registry and the effect attachment points.

#include <decomp/static_init.h>
#include <decomp/local_class.h>
// The hole-information HUD panels, the Driving Contest distance rows, the wind
// and timeout text panels, the HUD font configuration registry, and the effect
// attachment helpers.
//
// HUD rectangles are X/Y/width/height with Y down; texture rectangles are
// normalized UV extents. Attachment points use world XYZ with Y as height.
//
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
    char tree_insert_length_error[26];                       // "tree::insert length error"
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
    kGolfHudStringPool_std_exception_std_logic_error_std_length_error = (int)&((GolfHudStringPool*)0)
        ->std_exception_std_logic_error_std_length_error,
};

// Constants retain the original integer types and encoded values.
enum {
    kTutorialShowHoleCup = 4,
    kTutorialShowHoleInfo = 5,
    kHudFadeIn = 2,
    kHudFadeOut = 3,
    kMaxAttachmentPoints = 4,
    kFontRegistryInitialCapacity = 4,
    kHudTextBufferBytes = 32,
    kTimeoutTextAlignment = 2,
    kTimeoutInitialCount = 3,
    kHudAlphaByteShift = 24,
    kHudMapNodeBytes = 20,
    kHudMapPairBytes = 8
};
static const unsigned kRgbWhite = 0x00FFFFFFU;

#include "classes/CSharedDoc.h"
#include "classes/WView.h"
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
#include "classes/CWindInfo.h"
#include "classes/CTimeOutInfo.h"
#include "classes/CRemainTimeInfo.h"
#include "classes/CPowerInfo.h"
#include "classes/CBalloonInfo.h"
#include "classes/CHoleInfoSet.h"
#include "classes/CDriveInfo.h"
#include "classes/CInfoBase.h"

#include "sdk_protos.h"
#include "game/wvector.h"
#include "classes/CFontNumDraw.h"
#include "classes/CPvsFxBox.h"
#include "classes/CDistToHoleInfo.h"

// Construct a map payload at its already allocated address.
inline void* operator new(size_t, void* p) {
    return p;
}
// Placement construction does not own the supplied node allocation.
inline void operator delete(void*, void*) {}

// Fields of the effect box and the effect records it attaches to.
struct EffectAttachmentPoints {
    float screenX, screenY;
    void* transforms[4];
    void* positions[4];
};
// Each pair is borrowed: point alone is already in world space; matrix+point
// transforms a local point into world space; matrix alone contributes translation.
// The first empty pair terminates the list, even if later slots are populated.
struct AttachmentMatrix {
    float axes[9];
    float translation[3];
};

// The HUD font configuration registry: a hash table of named font entries plus
// the allocation rings the entries come from. Nodes are 0x18 bytes. The block
// header at index zero belongs to the allocation ring and is not a font entry.
struct HudFontRegistryNode {
    void* configuration;
    const char* name;
    unsigned char flags;
    char reserved09[3];
    HudFontRegistryNode* next;
    HudFontRegistryNode* prev;
    HudFontRegistryNode* bucketNext;
};
struct HudFontRegistry {
    HudFontRegistryNode* activeNodes;     // +00
    unsigned int reserved04;              // +04
    HudFontRegistryNode* allocatedBlocks; // +08  one header per allocation
    HudFontRegistryNode* freeNodes;       // +0C
    unsigned int nodesPerBlock;           // +10
    unsigned int bucketMask;              // +14  bucketCount - 1
    unsigned int bucketCount;             // +18
    HudFontRegistryNode** buckets;        // +1C
    unsigned char mutexStorage[1];        // +20  start of the embedded recursive mutex
};
typedef char HudFontRegistry_node_size[sizeof(HudFontRegistryNode) == 0x18 ? 1 : -1];
typedef char HudFontRegistry_mutex_offset[(int)&((HudFontRegistry*)0)->mutexStorage == 0x20 ? 1 : -1];

extern "C" {

void* __nw__FUl(unsigned int size);
void __ct__9CInfoBaseFv(void* self);
void* __ct__9CHoleInfoFv(void* self);
void* __ct__13CAccuracyInfoFv(void* self);
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
extern float kEffectGroundHeightUpperLimit, kEffectGroundHeightLowerLimit, kEffectGroundToleranceUpperBound,
    kEffectGroundToleranceLowerBound;
void appFreeUni__FPv(void* self);
void OverlayMapTree_FreeSubtree(void* self, void* tree);
void Vec3_Zero(void* self);
void* Matrix_TransformPoint(void* out, void* value, void* matrix);
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
void* GolfPlayer_GetEntryByIndex(void* state, unsigned int index);
void HudFont_DrawAlignedText(void* self, void* layout, float x, float y, float z, float alpha, int, int);
void WOverlay_DrawTransformedRect(void* text, void* camera, float* sourceUvRect, float, float* destinationRect, int,
                                  unsigned int color, int);
extern void* g_view;
void __as__7WVectorFRC7WVector(void* self, void* in);
unsigned int CQuadTree_GetInstance(void);
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void* out, unsigned int type, void* in, int, int, int);

struct Vec3u {
    unsigned int x, y, z;
    // Preserve float component bits for by-value ground-query calls.
    Vec3u(unsigned int xBits, unsigned int yBits, unsigned int zBits) : x(xBits), y(yBits), z(zBits) {}
};
// The Driving Contest display distance in the player's result union at +0x178:
// hundredths of the world XZ displacement plus the contest bonus. Other party
// modes store an integer score at the same address.
struct PlayerEntryView {
    unsigned char pad00[0x178];
    float driveDistance;
};
struct HudInfoInterface {
    virtual ~HudInfoInterface();
    virtual void update(float);
    virtual void draw();
};

} // extern "C"

// The current game document, which CSharedDoc keeps as a pointer word.
static inline CGolfDoc* GolfDoc() {
    return reinterpret_cast<CGolfDoc*>(AsCSharedDoc(g_sharedDoc)->golfDoc);
}

// Draw one distance row per player and emphasize the current player during the
// HUD fade. The fade timer is in seconds.
void CDriveInfo::DrawPlayerDistanceRows() {
    unsigned int color = (opacity << kHudAlphaByteShift) | kRgbWhite /* ARGB: alpha from opacity, RGB white */;
    this->color = color;
    // Phase 1: labelUvRect selects normalized texture UVs; labelRect holds layout X/Y/width/height.
    // Distance text uses viewport pixels. WOverlay_DrawTransformedRect applies each overlay's
    // anchoring/scaling to labelRect; rowSpacing increases layout Y down the player rows.
    float labelUvRect[4];
    float labelRect[4];
    labelUvRect[0] = kFloat_0_hud;
    labelUvRect[1] = kFloat_0_hud;
    labelUvRect[2] = kHudScalar0_8125;
    labelUvRect[3] = kHudScalar0_6875;
    labelRect[0] = kFloat_0_hud;
    labelRect[1] = kFloat_0_hud;
    labelRect[2] = kFloat_0_hud;
    labelRect[3] = kFloat_0_hud;
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
    while (playerIndex < GolfDoc()->playerCount) {
        alpha = kFloat_0_6_805DE33C;
        if (playerIndex == GolfDoc()->currentPlayer) {
            if (animationState == kHudFadeIn) {
                alpha += (highlightAlphaDelta * elapsed) / half;
                if (alpha > one)
                    alpha = one;
            } else if (animationState == kHudFadeOut) {
                float next = alpha + (highlightAlphaDelta * (half - elapsed)) / half;
                alpha = next < alpha ? alpha : next;
            }
        }
        void* playerEntry = GolfPlayer_GetEntryByIndex(GolfDoc(), playerIndex);
        sprintf(static_cast<char*>(textBuffer), golfHudFontStrings + kGolfHudStringPool_text_6_2f,
                ((PlayerEntryView*)playerEntry)->driveDistance);
        HudFont_DrawAlignedText(this, textBuffer, ((WView*)g_view)->viewportWidth - distanceRightMargin,
                                labelRect[1] + one, kFloat_3_Hud, alpha, 0, 0);
        alpha += highlightAlphaDelta;
        labelRect[2] = labelWidth * alpha;
        labelRect[3] = labelHeight * alpha;
        labelRect[0] =
            labelOffsetX + (float)floor((((WView*)g_view)->viewportWidth - distanceRightMargin - labelRect[2]) -
                                        labelRightMargin * alpha);
        WOverlay_DrawTransformedRect(playerLabels[playerIndex], g_view, labelUvRect, kFloat_0_hud, labelRect, 0, color,
                                     0);
        labelRect[1] += rowSpacing;
        ++playerIndex;
    }
}

// Draw each enabled panel, using the same tutorial gates as Update.
void CHoleInfoSet::Draw() {
    if (holeCupEnabled) {
        CGolfDoc* golfDoc = GolfDoc();
        if (golfDoc->tutorialStage >= kTutorialShowHoleCup && holeCupInfo != 0)
            ((HudInfoInterface*)holeCupInfo)->draw();
    }
    CGolfDoc* golfDoc = GolfDoc();
    if (golfDoc->tutorialStage >= kTutorialShowHoleInfo && holeInfo != 0)
        ((HudInfoInterface*)holeInfo)->draw();
    if (accuracyEnabled && accuracyInfo != 0)
        ((HudInfoInterface*)accuracyInfo)->draw();
    if (timeOutEnabled && timeOutInfo != 0)
        ((HudInfoInterface*)timeOutInfo)->draw();
}

// Destroy the four owned panels. The compiler adds the null check on the object
// and the optional free through the deleting destructor.
CHoleInfoSet::~CHoleInfoSet() {
    if (holeCupInfo != 0) {
        delete (HudInfoInterface*)holeCupInfo;
        holeCupInfo = 0;
    }
    if (holeInfo != 0) {
        delete (HudInfoInterface*)holeInfo;
        holeInfo = 0;
    }
    if (accuracyInfo != 0) {
        delete (HudInfoInterface*)accuracyInfo;
        accuracyInfo = 0;
    }
    if (timeOutInfo != 0) {
        delete (HudInfoInterface*)timeOutInfo;
        timeOutInfo = 0;
    }
}

// Create the four owned panels and set the initial enable flags.
void CHoleInfoSet::CreatePanels() {
    // Unused; its presence fixes the numbering of the constructor's cleanup symbols.
    struct ConstructorNumberingAnchor {
        void* holeCupInfo;
        unsigned int reserved04;
        void* holeInfo;
        void* accuracyInfo;
        void* timeOutInfo;
        unsigned char initialEnableBytes[5];
    };
    struct Loader {
        virtual void a() DECOMP_LOCAL_VIRTUAL;
        virtual void b() DECOMP_LOCAL_VIRTUAL;
        virtual void c() DECOMP_LOCAL_VIRTUAL;
        virtual void load(const char*, int, int) DECOMP_LOCAL_VIRTUAL;
    };
    struct DistanceToHoleComponent {
        unsigned char pad[0x60];
        // Construct the owned distance-to-hole component.
        DistanceToHoleComponent() {
            ((CDistToHoleInfo*)this)->__ct();
        }
    };
    struct HoleComponent {
        unsigned char pad[0xE4];
        // Construct the owned hole-data component.
        HoleComponent() {
            __ct__9CHoleInfoFv(this);
        }
    };
    struct AccuracyComponent {
        unsigned char pad[0x2C];
        // Construct the owned accuracy component.
        AccuracyComponent() {
            __ct__13CAccuracyInfoFv(this);
        }
    };
    struct HoleCupComponent : CInfoBase {
        unsigned char pad10[0x0C];
        void* resource1;
        void* resource2;
        void* resource3;
        unsigned int enabled;
        void* child;
        // Load the cup/bar overlays and own a nested distance-to-hole component.
        HoleCupComponent() {
            vtbl = const_cast<char*>(__vt__12CHoleCupInfo);
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
        int count;
        float value;
        unsigned int pad20;
        // Load the timeout font and allocate its 32-byte text buffer.
        TimeoutComponent() {
            vtbl = const_cast<char*>(__vt__12CTimeOutInfo);
            reinterpret_cast<Loader*>(this)->load(golfHudFontStrings + kGolfHudStringPool_bigfont_gsr, 0, 0);
            textBuffer = (char*)__nwa__FUl(kHudTextBufferBytes);
            float initial = kFloat_0_hud;
            alignment = kTimeoutTextAlignment;
            count = kTimeoutInitialCount;
            value = initial;
        }
    };
    // Allocate each panel in draw/update order.
    holeCupInfo = (CHoleCupInfo*)new HoleCupComponent;
    holeInfo = (CHoleInfo*)new HoleComponent;
    accuracyInfo = (CAccuracyInfo*)new AccuracyComponent;
    timeOutInfo = (CTimeOutInfo*)new TimeoutComponent;
    // The cup starts hidden; the other three enable bytes start set.
    holeCupEnabled = 0;
    reserved15[0] = 0;
    reserved15[1] = 1;
    accuracyEnabled = 1;
    timeOutEnabled = 1;
}

extern "C" {

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
    struct Ref {
        int vt;
        int c1;
        int c2;
        char* s;
    };
    struct Holder {
        char* flag;
        char* p;
        // Hold error-text storage until its shared deleter owns it.
        Holder(char* buffer, char* doneFlag) {
            p = buffer;
            flag = doneFlag;
        }
        // Delete untransferred text on exception construction failure.
        ~Holder() {
            if (p)
                __dla__FPv(p);
        }
    };
    struct ErrBase {
        void* vt;
        char* buf;
        Ref* ref;
        // Copy the diagnostic and create its shared text owner.
        ErrBase(const char* msg0) : vt((void*)__vt__Q23std11logic_error) {
            char done __attribute__((aligned(4))) = 0;
            volatile char baseDone = 0;
            const char* msg = msg0;
            int byteCount = strlen(msg) + 1;
            char* buffer = (char*)__nwa__FUl((2 * byteCount) - byteCount);
            buf = buffer;
            {
                Holder holder(buffer, &done);
                Ref* r = (Ref*)__nw__FUl(16);
                if (r) {
                    r->c1 = 1;
                    r->c2 = 1;
                    r->vt = (int)s_sharedCharDeleterVtable;
                    r->s = buffer;
                }
                ref = r;
                holder.p = 0;
            }
            strcpy(buf, msg);
        }
        // No local cleanup; the thrown object's runtime representation handles it.
        ~ErrBase() {}
    };
    struct ErrThrow : ErrBase {
        // Finish the length_error representation and transfer it to the runtime.
        ErrThrow(const char* msg) : ErrBase(msg) {
            vt = (void*)__vt__Q23std12length_error;
            __throw(
                (const char*)(golfHudFontStrings + kGolfHudStringPool_std_exception_std_logic_error_std_length_error),
                this, (void*)__dt__Q23std12length_errorFv);
        }
        // Empty local destructor preserves the original exception scaffolding.
        ~ErrThrow() {}
    };
    // Phase 1: guard size overflow, then allocate and copy the pair before linking.
    if (*reinterpret_cast<unsigned int*>(list) == 0xFFFFFFFFu) {
        ErrThrow err(golfHudFontStrings + kGolfHudStringPool_tree_insert_length_error);
    }
    struct Pair {
        int first;
        int second;
    };
    typedef void* Parent;
    struct Node {
        int left;
        unsigned int right;
        Parent parent;
        Pair copied;
    };
    struct NodeGuard {
        void* owner;
        Node* p;
        // Protect the freshly allocated node until its pair has been constructed.
        NodeGuard(void* o, Node* n) : owner(o), p(n) {}
        // Free an unpublished node if payload construction unwinds.
        ~NodeGuard() {
            if (p)
                appFreeUni__FPv(p);
        }
    };
    struct Allocation {
        char done;
        Node* p;
        Node* saved;
        // Allocate one 20-byte map node; NodeGuard takes cleanup responsibility.
        Allocation() : done(0) {
            p = static_cast<Node*>(appMallocDummy__FUl(kHudMapNodeBytes));
            saved = p;
        }
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
    if (insertLeft)
        *reinterpret_cast<void**>(position) = node;
    else
        *reinterpret_cast<void**>(static_cast<char*>(position) + 4) = node;
    unsigned int n = *reinterpret_cast<unsigned int*>(list);
    *reinterpret_cast<unsigned int*>(list) = 1 + n;
    RBTree_RebalanceAfterInsert(node, *reinterpret_cast<unsigned int*>(static_cast<char*>(list) + 4));
    if (becomesFirst)
        *reinterpret_cast<void**>(static_cast<char*>(list) + 8) = node;
    return node;
}
#pragma use_lmw_stmw reset
// Update each enabled panel, using the same tutorial gates as Draw.
void CHoleInfoSet::Update(float delta) {
    if (holeCupEnabled) {
        CGolfDoc* golfDoc = GolfDoc();
        if (golfDoc->tutorialStage >= kTutorialShowHoleCup && holeCupInfo != 0)
            ((HudInfoInterface*)holeCupInfo)->update(delta);
    }
    CGolfDoc* golfDoc = GolfDoc();
    if (golfDoc->tutorialStage >= kTutorialShowHoleInfo && holeInfo != 0)
        ((HudInfoInterface*)holeInfo)->update(delta);
    if (accuracyEnabled && accuracyInfo != 0)
        ((HudInfoInterface*)accuracyInfo)->update(delta);
    if (timeOutEnabled && timeOutInfo != 0)
        ((HudInfoInterface*)timeOutInfo)->update(delta);
}

#pragma use_lmw_stmw on
// Free both child subtrees and this map node; the expanded recursion preserves the original code.
void OverlayMapTree_FreeSubtree(void* self, void* tree) {
    // In plain C: free every node below `tree` (children first, payloads not destroyed), then `tree` itself.
    // The code below is that recursion unrolled three levels for each child, as the compiler emitted it.
    // Phase 1: free the left subtree in postorder. Three expanded levels are
    // followed by recursion for deeper branches; the tree depth is not limited.
    struct Node {
        Node* left;
        Node* right;
    };
    Node* leftChild = static_cast<Node*>(tree)->left;
    if (leftChild != 0) {
        Node* leftLeft = leftChild->left;
        if (leftLeft != 0) {
            Node* leftLeftLeft = leftLeft->left;
            if (leftLeftLeft != 0) {
                if (leftLeftLeft->left != 0)
                    OverlayMapTree_FreeSubtree(self, leftLeftLeft->left);
                if (leftLeftLeft->right != 0)
                    OverlayMapTree_FreeSubtree(self, leftLeftLeft->right);
                appFreeUni__FPv(leftLeftLeft);
            }
            Node* leftLeftRight = leftLeft->right;
            if (leftLeftRight != 0) {
                if (leftLeftRight->left != 0)
                    OverlayMapTree_FreeSubtree(self, leftLeftRight->left);
                if (leftLeftRight->right != 0)
                    OverlayMapTree_FreeSubtree(self, leftLeftRight->right);
                appFreeUni__FPv(leftLeftRight);
            }
            appFreeUni__FPv(leftLeft);
        }
        Node* leftRight = leftChild->right;
        if (leftRight != 0) {
            Node* leftRightLeft = leftRight->left;
            if (leftRightLeft != 0) {
                if (leftRightLeft->left != 0)
                    OverlayMapTree_FreeSubtree(self, leftRightLeft->left);
                if (leftRightLeft->right != 0)
                    OverlayMapTree_FreeSubtree(self, leftRightLeft->right);
                appFreeUni__FPv(leftRightLeft);
            }
            Node* leftRightRight = leftRight->right;
            if (leftRightRight != 0) {
                if (leftRightRight->left != 0)
                    OverlayMapTree_FreeSubtree(self, leftRightRight->left);
                if (leftRightRight->right != 0)
                    OverlayMapTree_FreeSubtree(self, leftRightRight->right);
                appFreeUni__FPv(leftRightRight);
            }
            appFreeUni__FPv(leftRight);
        }
        appFreeUni__FPv(leftChild);
    }
    // Phase 2: free the right subtree with the same postorder rule.
    Node* rightChild = static_cast<Node*>(tree)->right;
    if (rightChild != 0) {
        Node* rightLeft = rightChild->left;
        if (rightLeft != 0) {
            Node* rightLeftLeft = rightLeft->left;
            if (rightLeftLeft != 0) {
                if (rightLeftLeft->left != 0)
                    OverlayMapTree_FreeSubtree(self, rightLeftLeft->left);
                if (rightLeftLeft->right != 0)
                    OverlayMapTree_FreeSubtree(self, rightLeftLeft->right);
                appFreeUni__FPv(rightLeftLeft);
            }
            Node* rightLeftRight = rightLeft->right;
            if (rightLeftRight != 0) {
                if (rightLeftRight->left != 0)
                    OverlayMapTree_FreeSubtree(self, rightLeftRight->left);
                if (rightLeftRight->right != 0)
                    OverlayMapTree_FreeSubtree(self, rightLeftRight->right);
                appFreeUni__FPv(rightLeftRight);
            }
            appFreeUni__FPv(rightLeft);
        }
        Node* rightRight = rightChild->right;
        if (rightRight != 0) {
            Node* rightRightLeft = rightRight->left;
            if (rightRightLeft != 0) {
                if (rightRightLeft->left != 0)
                    OverlayMapTree_FreeSubtree(self, rightRightLeft->left);
                if (rightRightLeft->right != 0)
                    OverlayMapTree_FreeSubtree(self, rightRightLeft->right);
                appFreeUni__FPv(rightRightLeft);
            }
            Node* rightRightRight = rightRight->right;
            if (rightRightRight != 0) {
                if (rightRightRight->left != 0)
                    OverlayMapTree_FreeSubtree(self, rightRightRight->left);
                if (rightRightRight->right != 0)
                    OverlayMapTree_FreeSubtree(self, rightRightRight->right);
                appFreeUni__FPv(rightRightRight);
            }
            appFreeUni__FPv(rightRight);
        }
        appFreeUni__FPv(rightChild);
    }
    // Phase 3: both children are gone; free this node (payloads are not destroyed here).
    appFreeUni__FPv(tree);
}
#pragma use_lmw_stmw reset
struct WInstanceLock {
    void* lock;
    // Acquire the instance's recursive OS mutex.
    WInstanceLock(void* value) : lock(value) {
        OSLockMutex((OSMutex*)value);
    }
    // Release the mutex on normal return or unwind.
    ~WInstanceLock() {
        OSUnlockMutex((OSMutex*)lock);
    }
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
            for (int i = 0; i < (int)data->bucketCount; ++i)
                data->buckets[i] = 0;
        } else
            data->buckets = 0;
    }
    __register_global_object(data, reinterpret_cast<void*>(HudFontConfigRegistry_dtor),
                             s_hudFontConfigRegistryDtorRegistration);
}

} // extern "C"

// Release the timeout panel's font and text buffer, then optionally free it.
// The object is tested twice in a row, as the original does.
void* CTimeOutInfo::Destroy(int deleting) {
    if (this != 0) {
        if (this != 0) {
            void* res = fontResource;
            vtbl = (void*)cInfoBaseVtable;
            if (res != 0) {
                ResourceManager_DeleteResource(g_resourceManager, res);
                fontResource = 0;
            }
            void* buf = textBuffer;
            if (buf != 0) {
                __dla__FPv(buf);
                textBuffer = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}

// Release the wind panel's font and text buffer, then optionally free it.
// The object is tested twice in a row, as the original does.
void* CWindInfo::Destroy(int deleting) {
    if (this != 0) {
        if (this != 0) {
            void* res = fontResource;
            vtbl = (void*)cInfoBaseVtable;
            if (res != 0) {
                ResourceManager_DeleteResource(g_resourceManager, res);
                fontResource = 0;
            }
            void* buf = textBuffer;
            if (buf != 0) {
                __dla__FPv(buf);
                textBuffer = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}

// Release the remaining-time panel's font and text buffer, then optionally free it.
// The object is tested twice in a row, as the original does.
void* CRemainTimeInfo::Destroy(int deleting) {
    if (this != 0) {
        if (this != 0) {
            void* res = fontResource;
            vtbl = (void*)cInfoBaseVtable;
            if (res != 0) {
                ResourceManager_DeleteResource(g_resourceManager, res);
                fontResource = 0;
            }
            void* buf = textBuffer;
            if (buf != 0) {
                __dla__FPv(buf);
                textBuffer = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}

// Release the power panel's font and text buffer, then optionally free it.
// The object is tested twice in a row, as the original does.
void* CPowerInfo::Destroy(int deleting) {
    if (this != 0) {
        if (this != 0) {
            void* res = fontResource;
            vtbl = (void*)cInfoBaseVtable;
            if (res != 0) {
                ResourceManager_DeleteResource(g_resourceManager, res);
                fontResource = 0;
            }
            void* buf = textBuffer;
            if (buf != 0) {
                __dla__FPv(buf);
                textBuffer = 0;
            }
        }
        if (deleting > 0)
            __dl__FPv(this);
    }
    return this;
}

#pragma use_lmw_stmw on
extern "C" {
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
} // extern "C"

// Average up to four supplied attachment points, transforming each by its optional matrix.
// Required items is EffectAttachmentPoints[1]; context needs three floats and
// must not overlap any supplied position or matrix (it is zeroed before reads).
// context is a writable world-space vector, cleared even when no pair exists.
// Returns 1 with the mean of the contiguous nonempty prefix, 0 with a zero vector.
int CPvsFxBox::ComputeMeanAttachmentPosition(void* items, void* context) {
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
            if (fallback == 0)
                break;
            *(WVector*)context += *(WVector*)(((AttachmentMatrix*)fallback)->translation);
        }
        ++count;
        entry += 4;
        if ((unsigned int)count >= kMaxAttachmentPoints)
            break;
    }
    if (count > 0) {
        float converted = static_cast<float>(count);
        Vec3_DivideAssign(context, converted);
        return 1;
    }
    return 0;
}

// The numeric-font callback intentionally performs no action.
void CFontNumDraw::vfunc_4() {}
// The balloon-information callback intentionally performs no action.
void CBalloonInfo::vfunc_6() {}
// Reserved numeric-font hook; it performs no action.
void CFontNumDraw::vfunc_2() {}
// Reserved numeric-font hook; it performs no action.
void CFontNumDraw::vfunc_1() {}
// Return success unconditionally for this balloon-information hook.
int CBalloonInfo::vfunc_7() {
    return 1;
}

// Whether the Driving Contest panel is waiting for a new stroke.
int CDriveInfo::vfunc_7() {
    return animationState == 0;
}

extern "C" {
// Initialize an attachment with unset screen coordinates and four empty transform/point pairs.
void FxAttachment_Init(void* self) {
    EffectAttachmentPoints* data = static_cast<EffectAttachmentPoints*>(self);
    float value = kFloat_Neg1_0;
    data->transforms[0] = 0;
    data->screenX = value;
    data->screenY = value;
    data->positions[0] = 0;
    data->transforms[1] = 0;
    data->positions[1] = 0;
    data->transforms[2] = 0;
    data->positions[2] = 0;
    data->transforms[3] = 0;
    data->positions[3] = 0;
}
} // extern "C"

#pragma use_lmw_stmw on
// Shared node shape for the effect sets and their attachment maps; bit zero tags parent color.
struct HudEffectTreeNode {
    HudEffectTreeNode* left;
    HudEffectTreeNode* right;
    unsigned int parent;
    void* key;
    void* value;
};
static const unsigned int kHudTreeParentColorTag = 1U;

// Advance an effect-tree cursor in key order, keeping traversal scratch in its caller.
static inline void AdvanceHudEffectNode(HudEffectTreeNode*& effectNode, HudEffectTreeNode*& node) {
    node = effectNode->right;
    if (node != 0) {
        while (node->left != 0)
            node = node->left;
        effectNode = node;
    } else {
        node = reinterpret_cast<HudEffectTreeNode*>(effectNode->parent & ~kHudTreeParentColorTag);
        while (effectNode != node->left) {
            effectNode = node;
            node = reinterpret_cast<HudEffectTreeNode*>(node->parent & ~kHudTreeParentColorTag);
        }
        effectNode = node;
    }
}

// Set parent to the borrowed attachment node for an unsigned effect-pointer key, or the sentinel on a miss.
static inline void FindHudEffectAttachment(HudEffectTreeNode*& parent, HudEffectTreeNode*& node, void*& key,
                                           HudEffectTreeNode*& attachmentSentinel) {
    while (node != 0) {
        if (reinterpret_cast<unsigned int>(node->key) >= reinterpret_cast<unsigned int>(key)) {
            parent = node;
            node = node->left;
        } else
            node = node->right;
    }
    if (parent != attachmentSentinel &&
        reinterpret_cast<unsigned int>(key) < reinterpret_cast<unsigned int>(parent->key))
        parent = attachmentSentinel;
}

// Update spray and sequence world positions from their matching attachment
// records. Both passes walk an effect tree in key order and find the attachment
// entry with the same effect pointer as key; a missing entry leaves the effect's
// position unchanged.
void CPvsFxBox::UpdateAttachedEffectPositions() {
    // Parent links carry the tree color in bit 0.
    typedef HudEffectTreeNode TreeNode;
    struct EffectTrees {
        char prefix[0x158];
        TreeNode* sequenceRoot;
        TreeNode* sequenceFirst;
        unsigned int sequenceCount;
        TreeNode* sprayRoot;
        TreeNode* sprayFirst;
        unsigned int sprayCount;
        char beforeAttachments[0x210];
        TreeNode* sprayAttachmentRoot;
        char sprayAttachmentHeader[8];
        TreeNode* sequenceAttachmentRoot;
    };
    struct AttachmentNodeView {
        TreeNode* left;
        TreeNode* right;
        unsigned int parentAndColor;
        void* key;
        EffectAttachmentPoints attachment;
    };
    struct SprayPositionView {
        char prefix[0x84];
        WVector position;
    };
    struct SequencePositionView {
        char prefix[0xB8];
        WVector position;
    };
    typedef char AttachmentPayloadOffset[((int)&((AttachmentNodeView*)0)->attachment == 0x10) ? 1 : -1];
    typedef char SprayAttachmentRootOffset[((int)&((EffectTrees*)0)->sprayAttachmentRoot == 0x380) ? 1 : -1];
    typedef char SequenceAttachmentRootOffset[((int)&((EffectTrees*)0)->sequenceAttachmentRoot == 0x38C) ? 1 : -1];
    EffectTrees* trees = (EffectTrees*)this;
    {
        // Spray effects: attachment map at 0x380, position at 0x84.
        TreeNode* effectNode;
        void* key;
        TreeNode* attachmentSentinel;
        TreeNode* effectSentinel;
        attachmentSentinel = reinterpret_cast<TreeNode*>(&trees->sprayAttachmentRoot);
        effectSentinel = reinterpret_cast<TreeNode*>(&trees->sprayRoot);
        effectNode = trees->sprayFirst;
        for (; effectNode != effectSentinel;) {
            key = effectNode->value;
            TreeNode* node;
            TreeNode* parent;
            parent = reinterpret_cast<TreeNode*>(&trees->sprayAttachmentRoot);
            node = trees->sprayAttachmentRoot;
            FindHudEffectAttachment(parent, node, key, attachmentSentinel);
            WVector result;
            if (parent != attachmentSentinel &&
                CPvsFxBox::ComputeMeanAttachmentPosition(&((AttachmentNodeView*)parent)->attachment, &result))
                ((SprayPositionView*)key)->position = result;
            AdvanceHudEffectNode(effectNode, node);
        }
    }
    {
        // Sequence effects: attachment map at 0x38C, position at 0xB8.
        TreeNode* effectNode;
        void* key;
        TreeNode* attachmentSentinel;
        TreeNode* effectSentinel;
        attachmentSentinel = reinterpret_cast<TreeNode*>(&trees->sequenceAttachmentRoot);
        effectSentinel = reinterpret_cast<TreeNode*>(&trees->sequenceRoot);
        effectNode = trees->sequenceFirst;
        for (; effectNode != effectSentinel;) {
            key = effectNode->value;
            TreeNode* node;
            TreeNode* parent;
            parent = reinterpret_cast<TreeNode*>(&trees->sequenceAttachmentRoot);
            node = trees->sequenceAttachmentRoot;
            FindHudEffectAttachment(parent, node, key, attachmentSentinel);
            WVector result;
            if (parent != attachmentSentinel &&
                CPvsFxBox::ComputeMeanAttachmentPosition(&((AttachmentNodeView*)parent)->attachment, &result))
                ((SequencePositionView*)key)->position = result;
            AdvanceHudEffectNode(effectNode, node);
        }
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
int CPvsFxBox::TestGroundCrossing(float verticalStep, float* previousPosition, float* currentPosition,
                                  float* groundHeightInOut) {
    if (groundHeightInOut[0] > kEffectGroundHeightUpperLimit || groundHeightInOut[0] < kEffectGroundHeightLowerLimit)
        return 0;

    // Phase 1: query terrain independently below the previous and current positions.
    float previousGroundPoint[3];
    Vec3u previousPointBits(reinterpret_cast<unsigned int*>(previousPosition)[0],
                            reinterpret_cast<unsigned int*>(previousPosition)[1],
                            reinterpret_cast<unsigned int*>(previousPosition)[2]);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(previousGroundPoint, CQuadTree_GetInstance(), &previousPointBits, 1, 0, 1);
    float previousGroundHeight = previousGroundPoint[1];
    float currentGroundPoint[3];
    Vec3u currentPointBits(reinterpret_cast<unsigned int*>(currentPosition)[0],
                           reinterpret_cast<unsigned int*>(currentPosition)[1],
                           reinterpret_cast<unsigned int*>(currentPosition)[2]);
    GetGroundPoint__9CQuadTreeF7WVectorbPib(currentGroundPoint, CQuadTree_GetInstance(), &currentPointBits, 1, 0, 1);
    float currentGroundHeight = currentGroundPoint[1];
    float heightDifference = currentPosition[1] - currentGroundHeight;
    int withinGroundTolerance = 0;
    groundHeightInOut[0] = currentGroundHeight;
    if (heightDifference < kEffectGroundToleranceUpperBound && heightDifference > kEffectGroundToleranceLowerBound)
        withinGroundTolerance = 1;
    // Phase 2: test current contact, previous-side crossing, then the proposed Y step.
    if (!withinGroundTolerance) {
        if (currentPosition[1] < groundHeightInOut[0]) {
            heightDifference = previousPosition[1] - previousGroundHeight;
            withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound &&
                                    heightDifference > kEffectGroundToleranceLowerBound;
            if (!withinGroundTolerance) {
                if (!(previousPosition[1] < previousGroundHeight))
                    return 1;
            }
            heightDifference = currentPosition[1] + verticalStep - groundHeightInOut[0];
            withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound &&
                                    heightDifference > kEffectGroundToleranceLowerBound;
            if (withinGroundTolerance)
                return 1;
            if (currentPosition[1] + verticalStep > groundHeightInOut[0])
                return 1;
            return 0;
        } else {
            heightDifference = previousPosition[1] - previousGroundHeight;
            withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound &&
                                    heightDifference > kEffectGroundToleranceLowerBound;
            if (!withinGroundTolerance) {
                if (!(previousPosition[1] > previousGroundHeight))
                    return 1;
            }
            heightDifference = currentPosition[1] - verticalStep - groundHeightInOut[0];
            withinGroundTolerance = heightDifference < kEffectGroundToleranceUpperBound &&
                                    heightDifference > kEffectGroundToleranceLowerBound;
            if (withinGroundTolerance)
                return 1;
            if (currentPosition[1] - verticalStep < groundHeightInOut[0])
                return 1;
            return 0;
        }
    } else {
        return 1;
    }
}

// Clear the panel slots and create the four panels.
CHoleInfoSet* CHoleInfoSet::Init() {
    holeCupInfo = 0;
    holeInfo = 0;
    accuracyInfo = 0;
    timeOutInfo = 0;
    CreatePanels();
    return this;
}

extern "C" {
// Find an insertion parent and predecessor for an unsigned HUD-map key, returning side and predecessor flags.
// Return is a borrowed node or the root-pointer sentinel. predecessorOut is the
// last node <= key; leftOut chooses the parent link, flagOut stays 1 only when no
// predecessor was encountered (insertion becomes the first element).
// All arguments are required; key/output storage must not alias mutable tree
// links. Equal keys walk RIGHT and update predecessor, so a later insertion can
// append a duplicate key. This is not an insert-or-assign search.
void* HudFontConfigMap_FindInsertionPosition(void* tree, unsigned int* key, void** predecessorOut,
                                             unsigned char* leftOut, unsigned char* flagOut) {
    struct Tree {
        unsigned int pad0;
        unsigned int* node;
    };
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
