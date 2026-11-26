// Skid marks and ball trails: the Cobra skid-mark strip that follows the ball on the ground,
// the putt line display, the trajectory polygon, and the tutorial dialog and its text tree.

#include <decomp/static_init.h>
typedef unsigned long new_size_t;
#include "classes/ntSkidMarkCobra.h" // ntSkidMark_Cobra_AllocateFrameRange/2: active flag at +0x1C
#include "classes/WView.h"           // Layout offsets checked in the header.
#include "game/singletons.h"
#include "game/wvector.h"
#include "game/golfball.h"
#include "classes/GameInputManager.h"
#include "classes/CMouseCursor.h"
#include "game/types.h"
#include "classes/CTutorial.h"
#include "classes/CPuttLine.h"
#include "classes/CGenePoly3D.h"
#include "classes/CBallTrace.h"
#include "classes/CInGameTutorialDialog.h"
#include "classes/CTrajectory.h"

#define AsCPuttLine(p) ((CPuttLine*)(p))

extern "C" {

extern char s_trajectoryTypeInfo;
extern float kFloat_6_2831855;
extern float s_ballTrailFullTurn;
void Engine_SetFlexibleVertexFormat(unsigned int);
void Vec3_CopyToOutput(void*, void*);
void DrawPrimitiveUP(unsigned int, unsigned int, void*, unsigned int);
void __dt__18NativeBase80201D18Fv(void*, int);
void __dl__FPv(void*);
void __dla__FPv(void*);
extern char __vt__11CGenePoly3D[0x100];
extern char __vt__16ntSkidMark_Cobra[0x100];
extern char s_ballTraceBaseVtable[0x100];
extern void* g_resourceManager;
extern float kFloat_1_BallTrack;
extern float kFloat_0_0_BallTrack;
extern float kChipLineRibbonRadiusMultiplier;
// Preset layout is three lists of (state ID, raw value) pairs, NOT matrices:
// +00: 15 render states, +78: 6 stage states, +A8: 5 stage parameters.
// locals.renderStatePairs below holds the first state-pair list.
// These are engine IDs (gx_renderer_state_and_efb_copy.cpp); their GX consumers are in gx_render_state_and_texture_upload.cpp.
struct CobraStates {
    unsigned int data[30];
};
struct CobraTev {
    unsigned int data[12];
};
struct CobraTex {
    unsigned int data[10];
};
extern unsigned int cometRenderStatePreset[52];
extern void* g_view;
void* __nwa__FUl(unsigned int);
void* GolfBall__Fv();
void* PreData__9CGolfBallFi(void*, int);
void Vector_CrossProduct(void*, void*, void*);
void Normalize__7WVectorFv(void*);
void __amu__7WVectorFf(void*, float);
void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
void __as__7WVectorFRC7WVector(void*, void*);
void* CQuadTree_GetInstance();
void GetGroundPoint__9CQuadTreeF7WVectorbPib(void*, void*, void*, int, int, int);
void ResourceManager_ReleaseId(void*, void*);
void memset(void*, int, unsigned int);
void* __construct_new_array(void*, void*, unsigned int, unsigned int, unsigned int);
void CobraTrailSegment_DefaultConstruct();
void Engine_SetRenderStateList(void*, int = 15);
void Engine_SetTextureStageStateList(unsigned int, void*, unsigned int);
void Engine_SetTextureStageParameterList(unsigned int, void*, unsigned int);
void* Camera_GetRenderState(void*);
void SetTransform(unsigned int, void*);
void Engine_SetType7Resource(unsigned int);
void Engine_SetTextureStageResource(unsigned int, void*);
void Engine_SetTexturePalette(unsigned int, void*);
void Engine_SetCachedRenderState(unsigned int, unsigned int);
unsigned int GX_GetPrimitiveCount(unsigned int, unsigned int);
void Engine_SetRenderStateList(void*, int);
void Engine_SetTextureStageStateList(unsigned int, void*, unsigned int);
void Engine_SetTextureStageParameterList(unsigned int, void*, unsigned int);
void Engine_SetType7Resource(unsigned int);
void Engine_SetTextureStageResource(unsigned int, void*);
void Engine_SetTexturePalette(unsigned int, void*);
void Engine_SetCachedRenderState(unsigned int, unsigned int);

struct TutorialDialogValueView {
    char reservedPrefix[0x50];
    unsigned int dialogValue;
};

struct PolygonPositionView {
    char reservedPrefix[8];
    void* data;
};

struct TutorialDialogWordsView {
    char reservedPrefix[0x54];
    unsigned int dialogWord0;
    unsigned int dialogWord1;
    unsigned int dialogWord2;
    unsigned int dialogFinalWord;
};

struct PolygonAllocationView {
    char reservedPrefix[8];
    void* data;
};
struct cleanup_80206354 {
    unsigned char value;
    virtual ~cleanup_80206354();
};
struct CobraDestructionView {
    void* vtable;
    char reservedBaseFields[4];
    void* textureId;
    char reservedObjectBody[0x108];
    cleanup_80206354* polygon;
    void* segments;
    unsigned int segmentCount;
    unsigned int writeIndex;
};
// These private structs are overlapping receiver/alias views, not
// serialized function arguments. Cobra storage is a 16-byte new[] header then
// segmentCount 24-byte records, each two world-space Vec3 edge positions.
// segmentCount = allocated capacity (lastFrame-firstFrame+1, inclusive);
// cursor/writeIndex = number of committed sampled records, NOT a frame number;
// firstFrame/lastFrame = absolute ball simulation indices;
// lastSampledFrame = absolute index consumed so far (starts at firstFrame-1).
// A live endpoint can occupy entries[cursor] without incrementing cursor; draw
// includes that provisional record while lastSampledFrame < lastFrame, capped
// at allocated capacity. Destruction frees segments-16, not the interior pointer.
struct CobraAllocationView {
    void* vtable;
    char reservedBaseFields[0x110];
    void* polygon;
    void* segments;
    unsigned int segmentCount;
    unsigned int cursor;
    unsigned int firstFrame;
    unsigned int lastFrame;
    unsigned int lastSampledFrame;
};

// Replace polygon vertex storage with a zeroed array using the original byte-sized allocation count.
// The allocation uses vertexCount's low byte; success returns the full vertexCount, failure returns 0.
// Existing storage is released first, so allocation failure leaves no old vertices.
unsigned int CGenePoly3D::ReplaceVertexStorage(PolygonAllocationView* p, unsigned int vertexCount) {
    if (p->data) {
        __dla__FPv(p->data);
        p->data = 0;
    }
    if (vertexCount) {
        p->data = __nwa__FUl((unsigned char)vertexCount * 16);
        memset(p->data, 0, (vertexCount << 4) & 0xFF0);
    }
    return p->data ? vertexCount : 0;
}

// Release skid-mark segment storage, texture handle, and polygon before optional deletion.
void* ntSkidMark_Cobra_dtor(CobraDestructionView* p, int shouldDelete) {
    if (p) {
        p->vtable = __vt__16ntSkidMark_Cobra;
        if (p->segments) {
            if (p->segments)
                __dla__FPv((char*)p->segments - 16);
            p->segments = 0;
        }
        if (p->textureId) {
            ResourceManager_ReleaseId(g_resourceManager, p->textureId);
            p->textureId = 0;
        }
        p->segmentCount = 0;
        p->writeIndex = 0;
        if (p) {
            p->vtable = s_ballTraceBaseVtable;
            if (p->polygon) {
                delete p->polygon;
                p->polygon = 0;
            }
        }
        if (shouldDelete > 0)
            __dl__FPv(p);
    }
    return p;
}

#pragma push
#pragma use_lmw_stmw on
struct CobraTrailSegment {
    // Allocate the array through the matching new[] allocator (compiler adds its header).
    static void* operator new[](new_size_t size) {
        return __nwa__FUl(size);
    }
    char data[0x18];
    CobraTrailSegment();
};
// Preserve the original no-op element-construction call for every segment.
CobraTrailSegment::CobraTrailSegment() {
    CobraTrailSegment_DefaultConstruct();
}

struct ntSkidMark_Cobra_iface_base {
    unsigned char count;
    char pad[3];
};
struct ntSkidMark_Cobra_iface : ntSkidMark_Cobra_iface_base {
    virtual void vf0();
    virtual void vf1();
    virtual void vf2();
    virtual void vf3();
    virtual void vf4();
    virtual void call(unsigned char, float, float);
};

// Allocate trail segments over the selected simulation range and initialize quad texture coordinates.
void ntSkidMark_Cobra_AllocateFrameRange(CobraAllocationView* p) {
    // Phase 1: clamp the requested absolute frame range to simulation bounds.
    // Valid input must leave lastFrame >= firstFrame; the unsigned subtraction
    // below performs no empty-range validation.
    unsigned int firstSimulationFrame = (*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]);
    if ((int)p->firstFrame < (int)firstSimulationFrame)
        p->firstFrame = (*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]);
    if ((int)p->lastFrame < 0 || (int)p->lastFrame > ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[10])
        p->lastFrame = (*(unsigned int*)&((CGolfBall*)GolfBall__Fv())->m_preDataCount);
    p->segmentCount = p->lastFrame - p->firstFrame + 1;
    // Phase 2: replace owned segment storage; cursor resets independently of frame indices.
    if (p->segments) {
        if (p->segments)
            __dla__FPv((char*)p->segments - 16);
        p->segments = 0;
    }
    int segmentCount = p->segmentCount;
    if (segmentCount > 0) {
        p->segments = new CobraTrailSegment[segmentCount];
    }
    p->cursor = 0;
    int cornerIndex = 0;
    p->lastSampledFrame = p->firstFrame - 1;
    // Phase 3: quad corner order is previous+/previous-/current+/current-.
    // The same bit pattern below selects (U,V): (0,1),(0,0),(1,1),(1,0).
    for (; cornerIndex < 4; ++cornerIndex) {
        ntSkidMark_Cobra_iface* iface = (ntSkidMark_Cobra_iface*)p->polygon;
        if (iface->count <= (unsigned char)cornerIndex)
            continue;
        float textureV = (cornerIndex & 1) ? kFloat_0_0_BallTrack : kFloat_1_BallTrack;
        float textureU = (cornerIndex & 2) ? kFloat_1_BallTrack : kFloat_0_0_BallTrack;
        iface->call((unsigned char)cornerIndex, textureU, textureV);
    }
    AsntSkidMarkCobra(p)->active = 1;
}
#pragma pop

// Default segment construction requires no scalar initialization.
void CobraTrailSegment_DefaultConstruct() {}
// Camera render state stores two consecutive 4x4 transform matrices.
struct TrailCameraMatrices {
    char prefix[0x20];
    float view[16], projection[16];
};
enum TrailRenderContract {
    kTrailRenderPairCount = 15,
    kTrailStageStatePairCount = 6,
    kTrailStageParameterPairCount = 5,
    kTrailTextureStage = 0,
    kTrailViewTransform = 0,
    kTrailProjectionTransform = 1,
    kTrailDepthCompareState = 3,
    kTrailDepthWriteState = 5,
    kTrailSourceBlendState = 13,
    kTrailDestinationBlendState = 14,
    kTrailCompareAlways = 7,
    kTrailCompareGreaterEqual = 6,
    kTrailSourceAlpha = 4,
    kTrailInverseSourceAlpha = 5,
    kTrailDepthWriteOff = 0
};
enum TrailDrawConstant {
    kTrailTriangleStrip = 0x98,
    kPositionDiffuseFormat = 0xF001,
    kSpecialPuttSegmentColor = 0x100,
    kTutorialCancelMessage = 0x4D,
    kTutorialCancelModeMessage = 0x4E,
    kTutorialReleaseInputMessage = 0x4F,
    kCobraSegmentBytes = 24,
    kPuttSegmentBytes = 0x1c,
    kPuttFirstEdgeOffset = 4,
    kPuttSecondEdgeOffset = 0x10,
    kCornerCurrentFrameBit = 2,
    kCornerSecondEdgeBit = 1
};
typedef Vec3f vec3;
// A bit-preserving XYZ transport view for vector ABI calls, not integer coordinates.
// Word loads/stores must not become numeric float/unsigned conversions.
struct uvec3 {
    unsigned int x, y, z;
};
// Each simulation sample maps to one pair of world-space edges. The live pair
// occupies cursor without consuming it; drawing connects pair i-1 to pair i.
struct CobraTrailEdges {
    WVector positiveEdge, negativeEdge;
};
typedef char CobraTrailEdges_size[(sizeof(CobraTrailEdges) == 24) ? 1 : -1];
struct CobraSamplingView {
    char reservedPrefix[0x1C];
    unsigned char active;
    char reserved1D[0x118 - 0x1D];
    CobraTrailEdges* entries;
    int segmentCount;
    int cursor;
    unsigned int firstFrame;
    unsigned int lastFrame;
    unsigned int lastSampledFrame;
};

// Return the earlier simulation boundary by reference, preserving the original min operation.
inline const int& min_802065C4(const int& a, const int& b) {
    return a < b ? a : b;
}

// Project sampled ball positions onto the ground and build left/right trail edges from velocity and radius.
void ntSkidMark_Cobra_SampleGroundTrail(CobraSamplingView* p) {
    // Phase 1: select a simulation sample, then compute one horizontal half-width
    // for this update: normalize(worldUp cross velocity) * (10 * ball radius).
    // Coordinates and radius share the game's world units; no meter scale is assumed.
    if (!p->active)
        return;
    int* end = &((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[10];
    int* start = &((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0];
    int sampleFrame = min_802065C4(*end, *start);
    if (sampleFrame < (int)p->firstFrame)
        return;
    WVector trailHalfWidth;
    WVector samplePosition;
    WVector livePosition;
    vec3 worldUp = {kFloat_0_0_BallTrack, kFloat_1_BallTrack, kFloat_0_0_BallTrack};
    Vector_CrossProduct(&trailHalfWidth, &worldUp, (char*)PreData__9CGolfBallFi(GolfBall__Fv(), sampleFrame) + 12);
    trailHalfWidth.Normalize();
    trailHalfWidth *= kChipLineRibbonRadiusMultiplier * ((CGolfBall*)GolfBall__Fv())->m_radius;
    // Phase 2: append each not-yet-consumed absolute sample within capacity.
    // Project only its center Y to terrain; the side vector supplies the two edges.
    int absoluteSampleFrame = (p->lastSampledFrame + 1);
    for (; absoluteSampleFrame <= sampleFrame; ++absoluteSampleFrame) {
        if (p->cursor < p->segmentCount) {
            vec3 groundPoint;
            vec3 groundQueryPosition;
            vec3 positiveEdge;

            vec3 negativeEdge;
            void* simulationSample = PreData__9CGolfBallFi(GolfBall__Fv(), absoluteSampleFrame);
            unsigned int positionYBits = ((uvec3*)simulationSample)->y;
            unsigned int positionXBits = ((uvec3*)simulationSample)->x;
            ((uvec3*)&samplePosition)->y = positionYBits;
            ((uvec3*)&samplePosition)->x = positionXBits;
            ((uvec3*)&samplePosition)->z = ((uvec3*)simulationSample)->z;
            *(uvec3*)&groundQueryPosition = *(uvec3*)&samplePosition;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &groundQueryPosition, 1, 0,
                                                    1);
            samplePosition.y = groundPoint.y;

            __pl__FRC7WVectorRC7WVector(&positiveEdge, &samplePosition, &trailHalfWidth);
            p->entries[p->cursor].positiveEdge = *(WVector*)&positiveEdge;
            __mi__FRC7WVectorRC7WVector(&negativeEdge, &samplePosition, &trailHalfWidth);
            p->entries[p->cursor].negativeEdge = *(WVector*)&negativeEdge;
            p->cursor += 1;
        }
    }
    p->lastSampledFrame = sampleFrame;
    // Phase 3: write the provisional live endpoint WITHOUT advancing cursor.
    // The next committed sample will overwrite it. Draw counts it separately.
    if (sampleFrame < (int)p->lastFrame) {
        if (p->cursor < p->segmentCount) {
            uvec3* liveBallPositionBits = (uvec3*)GolfBall__Fv();
            unsigned int positionXBits = liveBallPositionBits->x;
            unsigned int positionYBits = liveBallPositionBits->y;

            ((uvec3*)&livePosition)->y = positionYBits;
            ((uvec3*)&livePosition)->x = positionXBits;
            ((uvec3*)&livePosition)->z = liveBallPositionBits->z;
            vec3 groundPoint;
            vec3 groundQueryPosition;
            *(uvec3*)&groundQueryPosition = *(uvec3*)&livePosition;
            GetGroundPoint__9CQuadTreeF7WVectorbPib(&groundPoint, CQuadTree_GetInstance(), &groundQueryPosition, 1, 0,
                                                    1);
            livePosition.y = groundPoint.y;
            vec3 positiveEdge;
            __pl__FRC7WVectorRC7WVector(&positiveEdge, &livePosition, &trailHalfWidth);
            vec3 negativeEdge;
            p->entries[p->cursor].positiveEdge = *(WVector*)&positiveEdge;
            __mi__FRC7WVectorRC7WVector(&negativeEdge, &livePosition, &trailHalfWidth);
            p->entries[p->cursor].negativeEdge = *(WVector*)&negativeEdge;
        }
    }
}

struct TrailEnabledView {
    char reservedPrefix[0x1c];
    unsigned char enabled;
};
struct PuttManager {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void* texture(void*);
    virtual void* source(void*);
};
struct PuttPoly {
    unsigned char count;
    virtual void vf0();
    virtual void begin();
    virtual void vf3();
    virtual void color(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
    virtual void position(unsigned char, const uvec3&);
    virtual void vf6();
    virtual void draw(unsigned int, unsigned int);
    // Set one borrowed position only if that polygon corner exists.
    void setPosition(unsigned char index, const uvec3& value) {
        if (count > index) {
            uvec3 copy = value;
            position(index, copy);
        }
    }
};
#pragma push
#pragma use_lmw_stmw on
// Draw putt-line quads with per-segment colors and the active camera transforms.
// Putt records differ from Cobra records: +0 is a color selector/alpha word,
// +4 first world-space edge, +0x10 second edge (28 bytes total). 0x100 is a
// special palette selector, NOT a packed RGBA value; otherwise low byte is alpha.
// color virtual arguments are index,A,R,G,B; vertex memory stores R,G,B,A.
void CPuttLine::Display() {
    const TrailEnabledView* p = (const TrailEnabledView*)this;
    // Phase 1: install the render preset/camera and bind the trail texture and polygon.
    struct Tables {
        CobraStates renderStatePairs;
        CobraTev arr1;
        CobraTex arr2;
    };
    Tables* base = (Tables*)cometRenderStatePreset;
    if (!p->enabled || !AsCPuttLine(p)->segments)
        return;
    struct {
        CobraTev arr1;
        CobraTex arr2;
        CobraStates renderStatePairs;
    } locals;
    locals.renderStatePairs = base->renderStatePairs;
    locals.arr1 = base->arr1;
    locals.arr2 = base->arr2;
    Engine_SetRenderStateList(&locals.renderStatePairs, kTrailRenderPairCount);
    Engine_SetTextureStageStateList(kTrailTextureStage, &locals.arr1, kTrailStageStatePairCount);
    Engine_SetTextureStageParameterList(kTrailTextureStage, &locals.arr2, kTrailStageParameterPairCount);
    void* device = Camera_GetRenderState(g_view);
    SetTransform(kTrailViewTransform, ((TrailCameraMatrices*)device)->view);
    SetTransform(kTrailProjectionTransform, ((TrailCameraMatrices*)device)->projection);
    PuttPoly* iface = (*(PuttPoly**)&AsCPuttLine(p)->polygon);
    Engine_SetType7Resource(0);
    iface->begin();
    PuttManager* mgr = (PuttManager*)((WView*)g_view)->renderTarget /* 0xA4 */;
    void* item = AsCPuttLine(p)->textureId;
    void* state = mgr->texture(item);
    Engine_SetTextureStageResource(0, state);
    mgr = (PuttManager*)((WView*)g_view)->renderTarget /* 0xA4 */;
    state = mgr->source(item);
    Engine_SetTexturePalette(0, state);
    // Putt lines ignore depth comparison and never write depth.
    Engine_SetCachedRenderState(kTrailDepthWriteState, kTrailDepthWriteOff);
    Engine_SetCachedRenderState(kTrailDepthCompareState, kTrailCompareAlways);
    // Phase 2: bridge successive records as triangle strips. Corner table:
    // pass 0 = previous first edge, 1 = previous second, 2 = current first,
    // 3 = current second. Color is selected from that same record.
    for (int frame = 1; frame <= ((CGolfBall*)GolfBall__Fv())->m_eventPreDataIndex[0]; ++frame) {
        for (int pass = 0; pass < 4; ++pass) {
            int index = (pass & kCornerCurrentFrameBit) ? frame : frame - 1;
            uvec3 vec =
                (pass & kCornerSecondEdgeBit)
                    ? *(uvec3*)((char*)AsCPuttLine(p)->segments + index * kPuttSegmentBytes + kPuttSecondEdgeOffset)
                    : *(uvec3*)((char*)AsCPuttLine(p)->segments + index * kPuttSegmentBytes + kPuttFirstEdgeOffset);
            iface = (*(PuttPoly**)&AsCPuttLine(p)->polygon);
            iface->setPosition((unsigned char)pass, vec);
            int color = *(int*)((char*)AsCPuttLine(p)->segments + index * kPuttSegmentBytes);
            if (color == kSpecialPuttSegmentColor) {
                iface = (*(PuttPoly**)&AsCPuttLine(p)->polygon);
                if (iface->count > (unsigned char)pass)
                    iface->color((unsigned char)pass, 0x80, 0xE6, 0xC8, 0x82);
            } else {
                iface = (*(PuttPoly**)&AsCPuttLine(p)->polygon);
                unsigned char alpha = (unsigned char)color;
                if (iface->count > (unsigned char)pass)
                    iface->color((unsigned char)pass, alpha, 0xff, 0xff, 0xff);
            }
        }
        iface = (*(PuttPoly**)&AsCPuttLine(p)->polygon);
        if (iface->count) {
            unsigned int v = GX_GetPrimitiveCount(iface->count, kTrailTriangleStrip);
            iface->draw(kTrailTriangleStrip, v);
        }
    }
}
#pragma pop

#pragma push
#pragma use_lmw_stmw on
struct CobraDrawingView {
    char reservedPrefix[8];
    void* item;
    char reservedObjectBody[0x108];
    void* iface;
    void* entries;
    unsigned int segmentCount;
    unsigned int cursor;
    unsigned int firstFrame;
    unsigned int lastFrame;
    unsigned int lastSampledFrame;
    char reserved130[4];
};
struct CobraManager {
    virtual void slot0();
    virtual void slot1();
    virtual void slot2();
    virtual void slot3();
    virtual void slot4();
    virtual void slot5();
    virtual void slot6();
    virtual void slot7();
    virtual void slot8();
    virtual void slot9();
    virtual void slot10();
    virtual void slot11();
    virtual void slot12();
    virtual void slot13();
    virtual void slot14();
    virtual void slot15();
    virtual void slot16();
    virtual void slot17();
    virtual void slot18();
    virtual void slot19();
    virtual void slot20();
    virtual void slot21();
    virtual void slot22();
    virtual void slot23();
    virtual void slot24();
    virtual void slot25();
    virtual void* texture(void*);
    virtual void* source(void*);
};
struct CobraPoly {
    unsigned char count;
    virtual void vf0();
    virtual void begin();
    virtual void vf3();
    virtual void color(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
    virtual void position(unsigned char, uvec3);
    virtual void vf6();
    virtual void draw(unsigned int, unsigned int);
    // Copy one position to the virtual setter only if that corner exists.
    void setPosition(unsigned char index, const uvec3& value) {
        uvec3 copy = value;
        if (count > index)
            position(index, copy);
    }
};

// Draw available Cobra trail quads, including a live endpoint before the simulation range finishes.
void ntSkidMark_Cobra_DrawTrail(CobraDrawingView* p) {
    // Phase 1: count committed records plus the optional live endpoint, capped
    // at segmentCount. A quad requires two records, so draw starts at frame 1.
    char* base = (char*)&cometRenderStatePreset;
    if (!AsntSkidMarkCobra(p)->active)
        return;
    int count;
    if ((int)p->lastSampledFrame < (int)p->lastFrame)
        count = p->cursor + 1;
    else
        count = p->cursor;
    int end = (*(int*)&p->segmentCount);
    if (end > count)
        end = count;
    struct {
        CobraTev arr1;
        CobraTex arr2;
        CobraStates renderStatePairs;
    } locals;
    locals.renderStatePairs = *(CobraStates*)(base + 0);
    locals.arr1 = *(CobraTev*)(base + 0x78);
    locals.arr2 = *(CobraTex*)(base + 0xA8);
    Engine_SetRenderStateList(&locals.renderStatePairs, kTrailRenderPairCount);
    Engine_SetTextureStageStateList(kTrailTextureStage, &locals.arr1, kTrailStageStatePairCount);
    Engine_SetTextureStageParameterList(kTrailTextureStage, &locals.arr2, kTrailStageParameterPairCount);
    // Cobra uses source-alpha/inverse-source-alpha blending and GEQUAL depth
    // comparison, with depth writes disabled below. The preset supplies other states.
    Engine_SetCachedRenderState(kTrailSourceBlendState, kTrailSourceAlpha);
    Engine_SetCachedRenderState(kTrailDestinationBlendState, kTrailInverseSourceAlpha);
    void* device = Camera_GetRenderState(g_view);
    SetTransform(kTrailViewTransform, ((TrailCameraMatrices*)device)->view);
    SetTransform(kTrailProjectionTransform, ((TrailCameraMatrices*)device)->projection);
    CobraPoly* iface = (CobraPoly*)p->iface;
    Engine_SetType7Resource(0);
    iface->begin();
    CobraManager* mgr = (CobraManager*)((WView*)g_view)->renderTarget /* 0xA4 */;
    void* item = p->item;
    void* value = mgr->texture(item);
    Engine_SetTextureStageResource(0, value);
    mgr = (CobraManager*)((WView*)g_view)->renderTarget /* 0xA4 */;
    value = mgr->source(item);
    Engine_SetTexturePalette(0, value);
    Engine_SetCachedRenderState(kTrailDepthWriteState, kTrailDepthWriteOff);
    Engine_SetCachedRenderState(kTrailDepthCompareState, kTrailCompareGreaterEqual);
    // Phase 2: initialize white/opaque corners, then submit adjacent record pairs.
    for (int i = 0; i < 4; ++i) {
        iface = (CobraPoly*)p->iface;
        if (iface->count > (unsigned char)i)
            iface->color((unsigned char)i, 0xFF, 0xFF, 0xFF, 0xFF);
    }
    int frame = 1;
    for (; frame < end; ++frame) {
        for (int pass = 0; pass < 4; ++pass) {
            int index = (pass & kCornerCurrentFrameBit) ? frame : frame - 1;
            uvec3 vec = (pass & kCornerSecondEdgeBit)
                            ? *(uvec3*)((char*)p->entries + index * kCobraSegmentBytes + sizeof(vec3))
                            : *(uvec3*)((char*)p->entries + index * kCobraSegmentBytes);
            iface = (CobraPoly*)p->iface;
            iface->setPosition((unsigned char)pass, vec);
        }
        iface = (CobraPoly*)p->iface;
        if (iface->count) {
            unsigned int v = GX_GetPrimitiveCount(iface->count, kTrailTriangleStrip);
            iface->draw(kTrailTriangleStrip, v);
        }
    }
}
#pragma pop
// The base interface leaves this operation to derived implementations.
void CBallTrace::BlowAway() {}
// The base interface leaves this operation to derived implementations.
void CBallTrace::Generate() {}
// The base interface leaves this operation to derived implementations.
void CGenePoly3D::vfunc_5() {}

struct PolygonColorView {
    char reservedPrefix[8];
    unsigned char* volatile data;
};

// Set vertex color bytes in alpha/red/green/blue argument order.
void CGenePoly3D::SetVertexColorARGB(PolygonColorView* p, unsigned char index, unsigned char alpha, unsigned char red,
                                     unsigned char green, unsigned char blue) {
    unsigned int offset = index * 16;
    (p->data + offset)[0xF] = alpha;
    (p->data + offset)[0xC] = red;
    (p->data + offset)[0xD] = green;
    (p->data + offset)[0xE] = blue;
}

// Destroy the trajectory base and optionally free the allocation.
CTrajectory::~CTrajectory() {
    __dt__18NativeBase80201D18Fv(this, 0);
}

struct PolygonDestructionView {
    char reservedPrefix[4];
    void* vtable;
    void* data;
};

// Release polygon vertex storage and optionally delete the polygon.
CGenePoly3D::~CGenePoly3D() {
    PolygonDestructionView* p = (PolygonDestructionView*)this;
    p->vtable = &__vt__11CGenePoly3D;
    if (p->data != 0) {
        __dla__FPv(p->data);
        p->data = 0;
    }
}

// Copy the selected vertex position to the supplied output vector.
void CGenePoly3D::CopyVertexPosition(PolygonPositionView* p, unsigned char index, void* output) {
    Vec3_CopyToOutput(output, (char*)p->data + index * 16);
}

// Draw the stored 16-byte vertices with the requested primitive and count.
void CGenePoly3D::DrawStoredVertices(unsigned int primitive, unsigned int count, void* vertices) {
    DrawPrimitiveUP(primitive, count, this->vertices, 16);
}

// Select the position/color vertex format used by these immediate polygons.
void CGenePoly3D::SelectVertexFormat() {
    Engine_SetFlexibleVertexFormat(kPositionDiffuseFormat);
}

// Initialize the trail full-turn angle cache.
void BallTrail_InitFullTurnConstant() {
    s_ballTrailFullTurn = kFloat_6_2831855;
}

// Return the trajectory runtime type descriptor.
void* CTrajectory::GetRTTI() {
    return &s_trajectoryTypeInfo;
}

// Adjust the secondary-interface receiver to the trajectory object before drawing.
// The draw itself returns nothing, but the original thunk passes the call's result through.
extern "C" void* Draw__11CTrajectoryFv(void* self);

void* CTrajectory::DrawThunk18(void* p) {
    return Draw__11CTrajectoryFv((char*)p - 0x18);
}

// Store the dialog value consumed by the tutorial interface.
void CInGameTutorialDialog::vfunc_16(TutorialDialogValueView* p, unsigned int value) {
    p->dialogValue = value;
}

// Copy the three supplied dialog words and the final dialog parameter.
// values borrows at least three readable words; the receiver keeps copies only.
void CInGameTutorialDialog::SetCallback(TutorialDialogWordsView* p, unsigned int* values, unsigned int last) {
    unsigned int word1 = values[1];
    unsigned int word2 = values[2];
    unsigned int word0 = values[0];
    p->dialogWord0 = word0;
    p->dialogWord1 = word1;
    p->dialogWord2 = word2;
    p->dialogFinalWord = last;
}

extern char __vt__9CTutorial[];
extern char tutorialCommandStrings[];
extern void* g_mouseCursor;
extern void* g_inputPointer;
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_tutorial[];
extern signed char s_refObjectListInitialized;
extern char s_refObjectList[];
extern char s_tutorialRefObjectListDtorRegistration[];
void IActor_DispatchMessage(void*, void*, void*, int, int, int, int, int);
void Pointer_ClearInputSuppression(void*);
void InputManager_Init(void*);
void InputManager_Dtor();
void __register_global_object(void*, void*, void*);
void RefObjectList_Init(void*);
void RefObjectList_dtor();
void RefObjectList_ClearAndRelease(void*);
void IntStringTree_DestroySubtree(void*, void*);
void appFreeUni__FPv(void*);
void TutorialBlockRing_dtor(void*, int);

struct V3 {
    int a, b, c;
};
struct RefPad {
    int refcount;
};
struct RefObj : RefPad {
    virtual void vf0(int);
    virtual void vf1();
    virtual void vf2(int);
    virtual void vf3();
    virtual void vf4();
    virtual void vf5();
    virtual void vf6();
    virtual void vf7();
    virtual void vf8();
    virtual void vf9();
    virtual void vf10();
    virtual void vf11();
    virtual void vf12();
    virtual void vf13();
    virtual void vf14();
    virtual void vf15();
    virtual void vf16(int);
    virtual void vf17(const V3&, void*);
};
extern V3 zeroInitializationWords;

// Nodes hold an integer key followed by an inline-or-heap narrow string.
struct TutorialTextNode {
    void* left;
    void* right;
    void* parent;
    int key;
    unsigned int stringFlags, stringLength;
    void* stringData;
};
// Free a node's heap-backed text (tag bit 31), then the node itself; children must already be gone.
inline void DestroyTutorialTextNode(void* node) {
    if ((int)node + 0xC != 0) {
        if ((int)node + 0x10 != 0) {
            if (((TutorialTextNode*)node)->stringFlags >> 31)
                appFreeUni__FPv(((TutorialTextNode*)node)->stringData);
        }
    }
    appFreeUni__FPv(node);
}
// Destroy both child subtrees before releasing this node and its heap-backed text.
inline void DestroyTutorialTextLeaf(void* owner, void* node) {
    if (((TutorialTextNode*)node)->left)
        IntStringTree_DestroySubtree((char*)owner + (int)&((CTutorial*)0)->textMapCount,
                                     ((TutorialTextNode*)node)->left);
    if (((TutorialTextNode*)node)->right)
        IntStringTree_DestroySubtree((char*)owner + (int)&((CTutorial*)0)->textMapCount,
                                     ((TutorialTextNode*)node)->right);
    DestroyTutorialTextNode(node);
}

// Release a text grandchild in postorder; descendant is caller scratch so the
// expanded destructor retains its original register lifetimes across dialog cleanup.
static inline void DestroyTutorialTextGrandchild(void* owner, void* grandchild, void*& descendant) {
    if (grandchild) {
        descendant = ((TutorialTextNode*)grandchild)->left;
        if (descendant) {
            DestroyTutorialTextLeaf(owner, descendant);
        }
        descendant = ((TutorialTextNode*)grandchild)->right;
        if (descendant) {
            DestroyTutorialTextLeaf(owner, descendant);
        }
        DestroyTutorialTextNode(grandchild);
    }
}

// Cancel tutorial input modes and reopen the shared input manager action gates.
static inline void RestoreTutorialInput(void* p) {
    ((CMouseCursor*)g_mouseCursor)->moveToActive = 0;
    IActor_DispatchMessage(p, p, tutorialCommandStrings, kTutorialCancelMessage, 0, 0, 0, 0);
    IActor_DispatchMessage(p, p, tutorialCommandStrings, kTutorialCancelModeMessage, 0, 0, 0, 0);
    IActor_DispatchMessage(p, p, tutorialCommandStrings, kTutorialReleaseInputMessage, 0, 0, 0, 0);
    Pointer_ClearInputSuppression(g_inputPointer);
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_tutorial);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    char* s = (char*)GetInputManagerSlot();
    ((GameInputManager*)s)->actionBlocked[0] = 0;
    ((GameInputManager*)s)->actionBlocked[1] = 0;
    ((GameInputManager*)s)->actionBlocked[2] = 0;
    ((GameInputManager*)s)->actionBlocked[3] = 0;
    ((GameInputManager*)s)->actionBlocked[4] = 0;
    ((GameInputManager*)s)->actionBlocked[5] = 0;
    ((GameInputManager*)s)->actionBlocked[6] = 0;
    ((GameInputManager*)s)->actionBlocked[7] = 0;
    ((GameInputManager*)s)->actionBlocked[8] = 0;
    ((GameInputManager*)s)->actionBlocked[9] = 0;
    ((GameInputManager*)s)->actionBlocked[10] = 0;
    ((GameInputManager*)s)->actionBlocked[11] = 0;
    ((GameInputManager*)s)->actionBlocked[12] = 0;
    ((GameInputManager*)s)->actionBlocked[13] = 0;
    ((GameInputManager*)s)->actionBlocked[14] = 0;
    ((GameInputManager*)s)->actionBlocked[15] = 0;
    ((GameInputManager*)s)->actionBlocked[16] = 0;
    ((GameInputManager*)s)->actionBlocked[17] = 0;
    ((GameInputManager*)s)->actionBlocked[18] = 0;
    ((GameInputManager*)s)->actionBlocked[19] = 0;
    ((GameInputManager*)s)->actionBlocked[20] = 0;
    ((GameInputManager*)s)->actionBlocked[21] = 0;
    ((GameInputManager*)s)->actionBlocked[22] = 0;
    ((GameInputManager*)s)->actionBlocked[23] = 0;
    ((GameInputManager*)s)->actionBlocked[24] = 0;
    ((GameInputManager*)s)->actionBlocked[25] = 0;
    ((GameInputManager*)s)->actionBlocked[26] = 0;
    ((GameInputManager*)s)->actionBlocked[27] = 0;
    ((GameInputManager*)s)->actionBlocked[28] = 0;
    ((GameInputManager*)s)->actionBlocked[29] = 0;
    ((GameInputManager*)s)->actionBlocked[30] = 0;
    ((GameInputManager*)s)->actionBlocked[31] = 0;
    ((GameInputManager*)s)->actionBlocked[32] = 0;
    ((GameInputManager*)s)->actionBlocked[33] = 0;
    ((GameInputManager*)s)->actionBlocked[34] = 0;
    ((GameInputManager*)s)->actionBlocked[35] = 0;
    ((GameInputManager*)s)->actionBlocked[36] = 0;
    ((GameInputManager*)s)->actionBlocked[37] = 0;
    ((GameInputManager*)s)->actionBlocked[38] = 0;
    if (!s) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_tutorial);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    ((GameInputManager*)GetInputManagerSlot())->selectedPadLookupDisabled = 0;
}

// Reset one retained dialog, release its reference, and clear its owner slot.
// Reload the slot after every virtual call: callbacks may replace the dialog.
static inline void ReleaseTutorialDialog(void* p, void*& dialog, void*& object) {
    if (dialog) {
        int receiver = (int)p;
        if (p != 0)
            receiver = (int)p + 0x18;
        V3 dialogArguments = zeroInitializationWords;
        object = dialog;
        ((RefObj*)object)->vf17(dialogArguments, (void*)receiver);
        object = dialog;
        ((RefObj*)object)->vf16(2);
        object = dialog;
        ((RefObj*)object)->vf2(0);
        object = dialog;
        if (object != 0) {
            if (--((RefObj*)object)->refcount == 0) {
                if (object != 0)
                    ((RefObj*)object)->vf0(1);
            }
            dialog = 0;
        }
    }
}

#pragma push
#pragma use_lmw_stmw on
// Cancel tutorial messages, restore input, release dialogs, and destroy text-map and command storage.
// shouldDelete > 0 also frees p; return p's original address. Dialogs use shared
// reference release; tree nodes and heap text are exclusively owned here.
CTutorial::~CTutorial() {
    void* root;
    void* child;
    // Also used as the dialog helper's scratch to retain register lifetimes.
    void* grandchildScratch;
    void* descendant;
    void* reservedScratch;
    *(void**)this = __vt__9CTutorial;
    secondaryVtable = __vt__9CTutorial + 0x3C;
    // Phase 1: cancel cursor motion and tutorial message/input modes before
    // restoring all 39 action gates and selected-pad lookup. Dispatches may
    // invoke callbacks; preserve their order before releasing dialog references.
    RestoreTutorialInput(this);
    // Phase 2: reset both dialogs using the zero argument triple and secondary
    // receiver p+0x18, then virtual slots 16(2), 2(0), and shared release.
    // Slot semantics beyond those arguments are unresolved; slot 0 deletes
    // only after the reference count reaches zero. Each field is then nulled.
    ReleaseTutorialDialog(this, messageDialog, grandchildScratch);
    ReleaseTutorialDialog(this, modeMessageDialog, grandchildScratch);
    // Phase 3: release shared tutorial reference-list contents before owned text.
    if (!Sing_805DC930()) {
        if (!s_refObjectListInitialized) {
            RefObjectList_Init(s_refObjectList);
            __register_global_object(s_refObjectList, (void*)RefObjectList_dtor,
                                     s_tutorialRefObjectListDtorRegistration);
            s_refObjectListInitialized = 1;
        }
        Sing_805DC930() = s_refObjectList;
    }
    RefObjectList_ClearAndRelease(Sing_805DC930());
    // Phase 4: destroy the text tree in postorder. Repeated address guards
    // are destructor continuations for ONE embedded container, not six maps.
    // They emit real repeated branches and must remain for byte identity.
    // The outer levels remain explicit; the grandchild helper delegates
    // deeper descendants to the recursive tree routine. Free each text before
    // its node, and both children before their parent.
    if ((int)this + 0x40 != 0)
        if ((int)this + 0x40 != 0)
            if ((int)this + 0x40 != 0)
                if ((int)this + 0x40 != 0)
                    if ((int)this + 0x40 != 0)
                        if ((int)this + 0x40 != 0) {
                            root = textMapRoot;
                            if (root) {
                                child = ((TutorialTextNode*)root)->left;
                                if (child) {
                                    grandchildScratch = ((TutorialTextNode*)child)->left;
                                    DestroyTutorialTextGrandchild(this, grandchildScratch, descendant);
                                    grandchildScratch = ((TutorialTextNode*)child)->right;
                                    DestroyTutorialTextGrandchild(this, grandchildScratch, descendant);
                                    DestroyTutorialTextNode(child);
                                }
                                child = ((TutorialTextNode*)root)->right;
                                if (child) {
                                    grandchildScratch = ((TutorialTextNode*)child)->left;
                                    DestroyTutorialTextGrandchild(this, grandchildScratch, descendant);
                                    grandchildScratch = ((TutorialTextNode*)child)->right;
                                    DestroyTutorialTextGrandchild(this, grandchildScratch, descendant);
                                    DestroyTutorialTextNode(child);
                                }
                                DestroyTutorialTextNode(root);
                            }
                        }
    // Phase 5: destroy the owned command buffer, base, and optional receiver allocation.
    if ((int)this + 0x1C != 0)
        if ((int)this + 0x1C != 0)
            if ((int)this + 0x1C != 0)
                if ((int)this + 0x1C != 0)
                    if ((int)this + 0x1C != 0) {
                        TutorialBlockRing_dtor(((char*)&commandReadOffset), 0);
                    }
    if (this != 0)
        __dt__18NativeBase80201D18Fv(this, 0);
}
#pragma pop

// Compile-time guards for private ABI views used above.
#define LOCAL_FIELD(t, f, o) typedef char local_##t##_##f[((int)&((t*)0)->f == (o)) ? 1 : -1]
LOCAL_FIELD(TrailCameraMatrices, view, 0x20);
LOCAL_FIELD(TrailCameraMatrices, projection, 0x60);
LOCAL_FIELD(TutorialTextNode, stringFlags, 0x10);
LOCAL_FIELD(TutorialTextNode, stringData, 0x18);
#undef LOCAL_FIELD

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621E4", _ctors_u80205FCC_0, BallTrail_InitFullTurnConstant);
#pragma pop
