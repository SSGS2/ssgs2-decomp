// Trail ribbon drawing: builds the vertices of a ribbon from its recent history samples and draws
// them now, using one of six line or triangle-strip topologies selected by the ribbon's mode.

extern "C" {

// One history sample contains three screen-plane points: 0=(x,y), 1=(x1,y1),
// 2=(x2,y2). Point 1 uses the second RGB color; points 0/2 use the first.
// Each opacity is converted float -> int -> byte, without saturation.
struct RibbonPoint {
    float x, y, opacity0;
    char pad0[4];
    float x1, y1, opacity1;
    char pad1[4];
    float x2, y2, opacity2;
};

// history is a borrowed ring of point pointers, head is its newest sample.
// Require 0 <= head < capacity and 2 <= count <= capacity when rendering.
// Modes below describe emitted topology; artistic names are not recovered.
enum RibbonTopology {
    kAlternatingTriangleStrip = 0,
    kThreePointSegmentLineStrip = 1,
    kThreePointSampleLineStrip = 2,
    kAlternatingEdgeLineStrip = 3,
    kMiddlePointLineStrip = 4,
    kOuterReturnLineStrip = 5
};
enum {
    kRibbonLocalVertices = 1024,
    kRibbonColoredFormat = 0xF005,
    kRibbonTriangleStripPrimitive = 0x98,
    kRibbonLineStripPrimitive = 0xB0
};
struct Ribbon {
    char pad0[4];
    int head;  // 0x4
    int count; // 0x8
    char padc[0x14];
    RibbonPoint** history; // 0x20
    char pad24[4];
    int mode;                     // 0x28
    unsigned char r, g, b, a;     // 0x2c
    unsigned char r1, g1, b1, a1; // 0x30
    char pad34[0x14];
    int capacity; // 0x48
};

struct Vertex {
    float x, y, z;
    unsigned char r, g, b, a;
};
// Three XYZ floats and four color bytes; sample topology 2 emits three vertices.
enum {
    kRibbonVertexBytes = 16,
    kRibbonSampleVertexBytes = 3 * kRibbonVertexBytes
};

void Render_SetColoredQuadState();
void Engine_SetFlexibleVertexFormat(int);
void Engine_SetType7Resource(int);
void Engine_SetTextureStageResource(int, int);
void* appMallocUni__FUl(int);
void appFreeUni__FPv(void*);
int GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);
extern const float kFloat_0_AngleVector;

// Set untextured colored render state, emit the selected topology and draw it now.
// No state restoration; call only within the caller's render-state scope.
// Mode 0 needs 1+6*(count-1) vertices: the fixed stack buffer requires count <=171.
// Modes 1..5 allocate transient vertices and free them after DrawPrimitiveUP.
// Allocations are assumed to succeed; no count/capacity validation is performed.
void TrailRibbon_Build(Ribbon* ribbon) {
    Vertex* vertices;
    RibbonPoint* current;
    RibbonPoint* previous;
    int used;
    int i;
    Vertex local[kRibbonLocalVertices];

    if (ribbon->count < 2)
        return;

// Explicit output/cursor/color-owner arguments expose the former captures.
// cr/cg/cb select members (r/g/b or r1/g1/b1); red/green/blue are locals.
// Keep color loads before position/alpha reads; arguments must have no side effects.
// An inline-function replacement was checked and changed this unit's .text.
#define EMIT_RIBBON_VERTEX(ribbon, vertices, used, px, py, cr, cg, cb, ca) \
    {                                                                      \
        unsigned char red, green, blue;                                    \
        blue = ribbon->cb;                                                 \
        green = ribbon->cg;                                                \
        red = ribbon->cr;                                                  \
        Vertex* output = &vertices[used++];                                \
        int alpha = (int)(ca);                                             \
        output->x = (px);                                                  \
        output->y = (py);                                                  \
        output->z = kFloat_0_AngleVector;                                  \
        output->r = red;                                                   \
        output->g = green;                                                 \
        output->b = blue;                                                  \
        output->a = alpha;                                                 \
    }

    switch (ribbon->mode) {
    case kAlternatingTriangleStrip: {
        // Six vertices per adjacent pair after the first seed; head parity swaps edges 0/2.
        // Repeated final previous vertices make zero-area strip joins, not extra visible faces.
        vertices = local;
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count - 1; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            previous = ribbon->history[(ribbon->head - i + ribbon->capacity - 1) % ribbon->capacity];
            if (i == 0) {
                if (ribbon->head & 1) {
                    EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
                } else {
                    EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
                }
            }
            if (((ribbon->head & 1) != 0 && (i & 1) != 0) || ((ribbon->head & 1) == 0 && (i & 1) == 0)) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x2, previous->y2, r, g, b, previous->opacity2);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x1, previous->y1, r1, g1, b1, previous->opacity1);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x, previous->y, r, g, b, previous->opacity0);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x, previous->y, r, g, b, previous->opacity0);
            } else {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x, previous->y, r, g, b, previous->opacity0);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x1, previous->y1, r1, g1, b1, previous->opacity1);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x2, previous->y2, r, g, b, previous->opacity2);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x2, previous->y2, r, g, b, previous->opacity2);
            }
        }
        DrawPrimitiveUP(kRibbonTriangleStripPrimitive, GX_GetPrimitiveCount(used, kRibbonTriangleStripPrimitive),
                        vertices, kRibbonVertexBytes);
        break;
    }

    case kThreePointSegmentLineStrip: {
        // Trace p2,p1,p0, then q0,p1,q1,p2,q2,q1,q0 for each adjacent pair.
        // This is alpha continuous line strip, so repeated points retrace edges.
        vertices = (Vertex*)appMallocUni__FUl((7 * (ribbon->count - 1) + 3) * kRibbonVertexBytes);
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count - 1; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            previous = ribbon->history[(ribbon->head - i + ribbon->capacity - 1) % ribbon->capacity];
            if (i == 0) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
            }
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x, previous->y, r, g, b, previous->opacity0);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x1, previous->y1, r1, g1, b1, previous->opacity1);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x2, previous->y2, r, g, b, previous->opacity2);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x1, previous->y1, r1, g1, b1, previous->opacity1);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, previous->x, previous->y, r, g, b, previous->opacity0);
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        if (vertices)
            appFreeUni__FPv(vertices);
        break;
    }

    case kThreePointSampleLineStrip: {
        // Connect 0 -> 1 -> 2 at each sample, then connect to the next sample 0.
        vertices = (Vertex*)appMallocUni__FUl(ribbon->count * kRibbonSampleVertexBytes);
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        if (vertices)
            appFreeUni__FPv(vertices);
        break;
    }

    case kAlternatingEdgeLineStrip: {
        // Alternate edges by traversal offset from newest (i), independent of head parity.
        // First pass starts at edge 0; the second traces the complementary edge.
        vertices = (Vertex*)appMallocUni__FUl(ribbon->count * kRibbonVertexBytes);
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            if (i & 1) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
            } else {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
            }
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        used = 0;
        for (i = 0; i < ribbon->count; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            if (i & 1) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
            } else {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
            }
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        if (vertices)
            appFreeUni__FPv(vertices);
        break;
    }

    case kMiddlePointLineStrip: {
        // Follow only point 1, using the second color.
        vertices = (Vertex*)appMallocUni__FUl(ribbon->count * kRibbonVertexBytes);
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        if (vertices)
            appFreeUni__FPv(vertices);
        break;
    }

    case kOuterReturnLineStrip: {
        // Seed with newest 2,1; follow point 0 newest-to-oldest, cross via oldest 1,
        // then return along point 2 oldest-to-newest (2*count+3 vertices).
        vertices = (Vertex*)appMallocUni__FUl((ribbon->count * 2 + 3) * kRibbonVertexBytes);
        Render_SetColoredQuadState();
        Engine_SetFlexibleVertexFormat(kRibbonColoredFormat);
        Engine_SetType7Resource(0);
        Engine_SetTextureStageResource(0, 0);
        used = 0;
        for (i = 0; i < ribbon->count; i++) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            if (i == 0) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
            }
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x, current->y, r, g, b, current->opacity0);
        }
        for (i = ribbon->count - 1; i >= 0; i--) {
            current = ribbon->history[(ribbon->head - i + ribbon->capacity) % ribbon->capacity];
            if (i == ribbon->count - 1) {
                EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x1, current->y1, r1, g1, b1, current->opacity1);
            }
            EMIT_RIBBON_VERTEX(ribbon, vertices, used, current->x2, current->y2, r, g, b, current->opacity2);
        }
        DrawPrimitiveUP(kRibbonLineStripPrimitive, GX_GetPrimitiveCount(used, kRibbonLineStripPrimitive), vertices,
                        kRibbonVertexBytes);
        if (vertices)
            appFreeUni__FPv(vertices);
        break;
    }
    }

#undef EMIT_RIBBON_VERTEX
}

} // extern "C"
