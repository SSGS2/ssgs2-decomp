// Overlay sprite drawing: draws colored rectangles and textured sprites into the current view,
// with flip, pixel snap and anchoring options. Also gives an overlay's texture width and height
// and loads and releases its texture.

#include "classes/WView.h"
#include "classes/WOverlay.h"

extern "C" {

enum SpriteFlags {
    kSpriteTextureMask = 0x7FF,
    kSpritePixelSnap = 0x400,
    kSpriteSubmitFlags = 0x30300000,
    kSpriteVertexFormat = 0x20,
    kSpriteQuadPrimitive = 0x10,
    kSpriteFlipU = 1,
    kSpriteFlipV = 2,
    kTextHorizontalAnchorMask = 6,
    kTextCenterX = 2,
    kTextVerticalAnchorMask = 0x60,
    kTextCenterY = 0x20,
    kTextScalePosition = 0x100,
    kTextScaleSize = 0x1000
};
enum SpriteClipOutcode {
    kOutsideLeft = 1,
    kOutsideRight = 2,
    kOutsideTop = 4,
    kOutsideBottom = 8
};
// Rectangles are [x,y,width,height], not opposite corners. Destination positions
// are viewport pixels with screen Y down; source rectangles are texture UVs.
// Rendering uses shared scratch vertices, so these routines are not reentrant.
// Legacy void* flags/colors carry 32-bit values, not addresses to dereference.
// Polygon clippers borrow disjoint arrays with room for added intersections;
// they write x/y/u/v only. The caller restores z/w/color before submission.
// Boundary tests are strict: points exactly on an edge are not copied by the
// inside test, and intersections are emitted only on a strict sign change.
#include "sdk_protos.h"
#include "classes/CHashPath.h"

struct OverlayTextureView {
    char baseObject[0x10];
    void* resourcePaths;
    float clipRect[4];
    unsigned char clipEnabled;
    char reserved25[3];
    int coordMode;
    void* texture;
};

// WView supplies the viewport width/height used by polygon clipping.

struct OverlayDrawDispatch {
    virtual void fn0();
    virtual void release();
    virtual void fn10();
    virtual void fn14();
    virtual void drawQuad(const char*, int, unsigned int, int);
};

struct SpriteQuadVertex {
    float x;
    float y;
    union {
        float scale;
        float z;
    };
    union {
        float one;
        float w;
    };
    unsigned int color;
    char reserved14[4];
    float u;
    float v;
    char reserved20[0x14];
    unsigned char clipFlags;
    char reserved35[3];
};

struct ClippedSpriteVertex {
    float x;
    float y;
    char reserved08[0x10];
    float u; // same UV offsets as SpriteQuadVertex
    float v;
    char reserved20[0x18];
};

struct SpriteRenderDispatch {
    virtual void fn0();
    virtual void fn4();
    virtual void fn8();
    virtual void release();
    virtual void render(void*, int, unsigned int, int);
};
struct TextureLookupResult {
    int width;
    int height;
};

struct OverlayPathString {
    union {
        unsigned int flags;
        char firstWord[4];
    } header;
    unsigned int secondWord;
    char* heapText;
};

void* ISharedResource_GetTextureDescription(void*);
void String_ReserveCapacity(void*, unsigned int);
void String_ReplaceRange(void*, int, int, const char*, const char*, const char*);
void ResourcePath_NormalizeImageExtension(void*);
void* CreateSharedTextureResource__FPCci(const char*, int);
int Get__15ISharedResourceFv(void*);
void appFreeUni__FPv(void*);

void BaseObject_dtor(void*, int);
void __dl__FPv(void*);
void DrawText_TransformRectToViewport(float*, WView*, const float*, int);
void Sprite_DrawClippedTexturedQuad(void*, void*, void*, void*, float*, float, void*, void*, void*, bool);
float Math_CosApprox(float);
float Math_SinApprox(float);
void WView_SetOptionalTextureOverride(void*, void*);
void* memcpy(void*, const void*, unsigned int);
extern SpriteQuadVertex* s_clippedSpriteVertexPointers[16];
extern const float kFloat_0_00001_BitmapDraw;
void WOverlay_DrawTransformedRect(void*, void*, void*, float, float*, void*, void*, void*);
extern const float kFloat_0_B2d;
extern const float kFloat_0_5_BitmapDraw;
extern const float kFloat_1_BitmapDraw;
extern const float kFloat_640_TextViewport;
extern const float kFloat_480_Viewport;
extern char __vt__8WOverlay[];
extern char s_spriteQuadVertexPointers[];
// One shared four-corner sprite quad (0xE0 bytes); the original
// symbol map gives each 0x38-byte vertex its own label.
typedef char SpriteQuadVertex_size[(sizeof(SpriteQuadVertex) == 0x38) ? 1 : -1];
struct SpriteQuad {
    SpriteQuadVertex vertices[4];
};
extern SpriteQuad s_spriteQuadVertex0;

// Fill the shared quad with rectangle corners and submit a colored sprite.
void Sprite_DrawColoredRect(void* self, float* rect, int flags, unsigned int color, float depth) {
    SpriteQuadVertex* dst = s_spriteQuadVertex0.vertices;
    float half = kFloat_0_5_BitmapDraw;
    for (int corner = 0; corner < 4; ++corner) {
        float x;
        if ((corner & 1) != 0) {
            x = rect[0] + rect[2];
        } else {
            x = rect[0];
        }
        dst->x = x - half;
        float y;
        if ((corner & 2) != 0) {
            y = rect[1] + rect[3];
        } else {
            y = rect[1];
        }
        dst->y = y - half;
        dst->scale = depth;
        dst->one = kFloat_1_BitmapDraw;
        dst->color = color;
        ++dst;
    }
    if ((flags & kSpriteTextureMask) != 0) {
        s_spriteQuadVertex0.vertices[0].u = kFloat_1_BitmapDraw; // original table +0x18/+0x1C, stride 0x38
        s_spriteQuadVertex0.vertices[0].v = kFloat_1_BitmapDraw;
        s_spriteQuadVertex0.vertices[1].u = kFloat_0_B2d;
        s_spriteQuadVertex0.vertices[1].v = kFloat_1_BitmapDraw;
        s_spriteQuadVertex0.vertices[2].u = kFloat_1_BitmapDraw;
        s_spriteQuadVertex0.vertices[2].v = kFloat_0_B2d;
        s_spriteQuadVertex0.vertices[3].u = kFloat_0_B2d;
        s_spriteQuadVertex0.vertices[3].v = kFloat_0_B2d;
    }
    ((OverlayDrawDispatch*)self)
        ->drawQuad(s_spriteQuadVertexPointers, 0x20, (unsigned int)flags | kSpriteSubmitFlags, 0);
}

// Clip a polygon against x > 0, interpolating its texture coordinates.
void Sprite_ClipPolygonToLeftEdge(void* unused0, void* unused1, ClippedSpriteVertex* out, ClippedSpriteVertex* in,
                                  int* outCount, int count) {
    *outCount = 0;
    float zero = kFloat_0_B2d;
    for (int i = 0; i < count; ++i) {
        ClippedSpriteVertex* current = &in[i];
        if (current->x > zero) {
            out[*outCount].x = current->x;
            out[*outCount].y = current->y;
            out[*outCount].u = current->u;
            out[*outCount].v = current->v;
            ++*outCount;
        }
        int nextIndex = (i + 1) % count;
        ClippedSpriteVertex* next = &in[nextIndex];
        float nextX = next->x;
        if (current->x * nextX < zero) {
            float absNextX;
            if (nextX > zero) {
                absNextX = nextX;
            } else {
                absNextX = -nextX;
            }
            float absX;
            if (current->x > zero) {
                absX = current->x;
            } else {
                absX = -current->x;
            }
            float numerator;
            if (current->x > zero) {
                numerator = current->x;
            } else {
                numerator = -current->x;
            }
            float ratio = numerator / (absNextX + absX);
            out[*outCount].x = zero;
            out[*outCount].y = current->y + ratio * (next->y - current->y);
            out[*outCount].u = current->u + ratio * (next->u - current->u);
            out[*outCount].v = current->v + ratio * (next->v - current->v);
            ++*outCount;
        }
    }
}

// Clip a polygon against y > 0, interpolating its texture coordinates.
void Sprite_ClipPolygonToTopEdge(void* unused0, void* unused1, ClippedSpriteVertex* out, ClippedSpriteVertex* in,
                                 int* outCount, int count) {
    *outCount = 0;
    float zero = kFloat_0_B2d;
    for (int i = 0; i < count; ++i) {
        ClippedSpriteVertex* current = &in[i];
        if (current->y > zero) {
            out[*outCount].x = current->x;
            out[*outCount].y = current->y;
            out[*outCount].u = current->u;
            out[*outCount].v = current->v;
            ++*outCount;
        }
        int nextIndex = (i + 1) % count;
        ClippedSpriteVertex* next = &in[nextIndex];
        float nextY = next->y;
        if (current->y * nextY < zero) {
            float absNextY;
            if (nextY > zero) {
                absNextY = nextY;
            } else {
                absNextY = -nextY;
            }
            float absY;
            if (current->y > zero) {
                absY = current->y;
            } else {
                absY = -current->y;
            }
            float numerator;
            if (current->y > zero) {
                numerator = current->y;
            } else {
                numerator = -current->y;
            }
            float ratio = numerator / (absNextY + absY);
            out[*outCount].x = current->x + ratio * (next->x - current->x);
            out[*outCount].y = zero;
            out[*outCount].u = current->u + ratio * (next->u - current->u);
            out[*outCount].v = current->v + ratio * (next->v - current->v);
            ++*outCount;
        }
    }
}

// Clip a polygon against the viewport right edge.
void Sprite_ClipPolygonToRightEdge(void* unused0, WView* camera, ClippedSpriteVertex* out, ClippedSpriteVertex* in,
                                   int* outCount, int count) {
    *outCount = 0;
    float zero = kFloat_0_B2d;
    for (int i = 0; i < count; ++i) {
        ClippedSpriteVertex* current = &in[i];
        float limit = camera->viewportWidth;
        if (current->x < limit) {
            out[*outCount].x = current->x;
            out[*outCount].y = current->y;
            out[*outCount].u = current->u;
            out[*outCount].v = current->v;
            ++*outCount;
        }
        int nextIndex = (i + 1) % count;
        ClippedSpriteVertex* next = &in[nextIndex];
        limit = camera->viewportWidth;
        float diff = current->x - limit;
        if (diff * (next->x - limit) < zero) {
            float absDiff = (diff > zero) ? diff : -diff;
            float nextLimit = camera->viewportWidth;
            float nextDistance = next->x - nextLimit;
            nextDistance = (nextDistance > zero) ? nextDistance : -nextDistance;
            float curLimit = camera->viewportWidth;
            float numerator = current->x - curLimit;
            numerator = (numerator > zero) ? numerator : -numerator;
            out[*outCount].x = camera->viewportWidth;
            float ratio = numerator / (nextDistance + absDiff);
            out[*outCount].y = current->y + ratio * (next->y - current->y);
            out[*outCount].u = current->u + ratio * (next->u - current->u);
            out[*outCount].v = current->v + ratio * (next->v - current->v);
            ++*outCount;
        }
    }
}

// Clip a polygon against the viewport bottom edge.
void Sprite_ClipPolygonToBottomEdge(void* unused0, WView* camera, ClippedSpriteVertex* out, ClippedSpriteVertex* in,
                                    int* outCount, int count) {
    *outCount = 0;
    float zero = kFloat_0_B2d;
    for (int i = 0; i < count; ++i) {
        ClippedSpriteVertex* current = &in[i];
        float limit = camera->viewportHeight;
        if (current->y < limit) {
            out[*outCount].x = current->x;
            out[*outCount].y = current->y;
            out[*outCount].u = current->u;
            out[*outCount].v = current->v;
            ++*outCount;
        }
        int nextIndex = (i + 1) % count;
        ClippedSpriteVertex* next = &in[nextIndex];
        limit = camera->viewportHeight;
        float diff = current->y - limit;
        if (diff * (next->y - limit) < zero) {
            float absDiff = (diff > zero) ? diff : -diff;
            float nextLimit = camera->viewportHeight;
            float nextDistance = next->y - nextLimit;
            nextDistance = (nextDistance > zero) ? nextDistance : -nextDistance;
            float curLimit = camera->viewportHeight;
            float numerator = current->y - curLimit;
            numerator = (numerator > zero) ? numerator : -numerator;
            float ratio = numerator / (nextDistance + absDiff);
            out[*outCount].x = current->x + ratio * (next->x - current->x);
            out[*outCount].y = camera->viewportHeight;
            out[*outCount].u = current->u + ratio * (next->u - current->u);
            out[*outCount].v = current->v + ratio * (next->v - current->v);
            ++*outCount;
        }
    }
}

// Read texture width in pixels, or zero when the overlay has no texture.
int WOverlay_GetWidth(OverlayTextureView* self) {
    if (self->texture != 0) {
        return ((TextureLookupResult*)ISharedResource_GetTextureDescription(self->texture))->width;
    }
    return 0;
}

// Read texture height in pixels, or zero when the overlay has no texture.
int WOverlay_GetHeight(OverlayTextureView* self) {
    if (self->texture != 0) {
        return ((TextureLookupResult*)ISharedResource_GetTextureDescription(self->texture))->height;
    }
    return 0;
}

// Empty sprite-unit hook retained for binary compatibility.
void Sprite_NoOpHook() {}

// Clip destination and UV rectangles, optionally rotate, then submit the sprite.
void Sprite_DrawClippedTexturedQuad(void* self, void* camera, void* texture, void* sourceUvRect, float* destinationRect,
                                    float rotationAngle, void* renderFlags, void* packedColor, void* uvFlipFlags,
                                    bool clipToBounds) {
    // Nonzero rotation uses Math_Sin/CosApprox's angular convention. Clipping
    // here intersects the unrotated rectangle; the pivot is its pixel center.
    float* texRect = (float*)sourceUvRect;
    float dst[4];
    float mapped[4];
    float rectCopy[4];
    float clippedRect[4];
    float angleBits = rotationAngle;
    double savedAngle = rotationAngle;
    // Phase 1: intersect destination bounds, trim UVs in the same proportions,
    // and recurse once with clipping disabled when the size changed.
    if (clipToBounds == 1) {
        float* clip;
        if (((OverlayTextureView*)self)->clipEnabled != 0) {
            clip = ((OverlayTextureView*)self)->clipRect;
        } else {
            struct ClipSetup {
                // Build [inset,inset,width-inset,height-inset] viewport bounds.
                static void set(float* out, float x, float w, float h) {
                    float bottom = h - x;
                    float right = w - x;
                    out[0] = x;
                    out[1] = x;
                    out[2] = right;
                    out[3] = bottom;
                }
            };
            ClipSetup::set(clippedRect, kFloat_0_5_BitmapDraw, ((WView*)camera)->viewportWidth,
                           ((WView*)camera)->viewportHeight);
            clip = clippedRect;
        }
        rectCopy[0] = destinationRect[0];
        rectCopy[1] = destinationRect[1];
        rectCopy[2] = destinationRect[2];
        rectCopy[3] = destinationRect[3];
        dst[0] = rectCopy[0] < clip[0] ? clip[0] : rectCopy[0];
        dst[1] = rectCopy[1] < clip[1] ? clip[1] : rectCopy[1];
        float clipRight = clip[0] + clip[2];
        float right = rectCopy[0] + rectCopy[2];
        clipRight = right > clipRight ? clipRight : right;
        dst[2] = clipRight - dst[0];
        float bottom = rectCopy[1] + rectCopy[3];
        float clipBottom = clip[1] + clip[3];
        clipBottom = bottom > clipBottom ? clipBottom : bottom;
        dst[3] = clipBottom - dst[1];
        int empty = (dst[2] < kFloat_0_B2d) || (dst[3] < kFloat_0_B2d);
        if (empty) {
            return;
        }
        if (destinationRect[2] != dst[2] || destinationRect[3] != dst[3]) {
            if (destinationRect[2] != dst[2]) {
                float xoff = dst[0] - destinationRect[0];
                float sumRect = destinationRect[0] + destinationRect[2];
                float sumDst = dst[0] + dst[2];
                float tw = texRect[2];
                float ratio = tw / destinationRect[2];
                mapped[0] = ratio * xoff + texRect[0];
                mapped[2] = ratio * (sumDst - sumRect - xoff) + tw;
            } else {
                float b, a;
                a = texRect[0];
                b = texRect[2];
                mapped[0] = a;
                mapped[2] = b;
            }
            if (destinationRect[3] != dst[3]) {
                float th = texRect[3];
                float ratio = th / destinationRect[3];
                float yoff = dst[1] - destinationRect[1];
                float sumRect = destinationRect[1] + destinationRect[3];
                float sumDst = dst[1] + dst[3];
                mapped[1] = ratio * yoff + texRect[1];
                mapped[3] = ratio * (sumDst - sumRect - yoff) + th;
            } else {
                float b, a;
                a = texRect[1];
                b = texRect[3];
                mapped[1] = a;
                mapped[3] = b;
            }
            Sprite_DrawClippedTexturedQuad(self, camera, texture, mapped, dst, (float)savedAngle, renderFlags,
                                           packedColor, uvFlipFlags, false);
            return;
        }
    }

    // Phase 2: build pixel-aligned corners and choose each corner's UV endpoint.
    float xLeft;
    float xRight;
    float yTop;
    float yBottom;
    if ((AsWOverlay(self)->coordMode & kSpritePixelSnap) != 0) {
        float fx = (float)floor((double)destinationRect[0]);
        if (fx < kFloat_1_BitmapDraw) {
            xLeft = kFloat_0_5_BitmapDraw;
            xRight = xLeft + destinationRect[2];
        } else {
            xLeft = (float)floor((double)destinationRect[0]) - kFloat_0_5_BitmapDraw;
            xRight = xLeft + destinationRect[2];
        }
        float fy = (float)floor((double)destinationRect[1]);
        if (fy < kFloat_1_BitmapDraw) {
            yTop = kFloat_0_5_BitmapDraw;
            yBottom = yTop + destinationRect[3];
        } else {
            yTop = (float)floor((double)destinationRect[1]) - kFloat_0_5_BitmapDraw;
            yBottom = yTop + destinationRect[3];
        }
    }
    float half = kFloat_0_5_BitmapDraw;
    unsigned int flipX = !((unsigned int)uvFlipFlags & 1);
    unsigned int flipY = !((unsigned int)uvFlipFlags & 2);
    for (int i = 0; i < 4; ++i) {
        if ((AsWOverlay(self)->coordMode & kSpritePixelSnap) != 0) {
            s_spriteQuadVertex0.vertices[i].x = (i & 1) ? xRight : xLeft;
            s_spriteQuadVertex0.vertices[i].y = (i & 2) ? yBottom : yTop;
        } else {
            s_spriteQuadVertex0.vertices[i].x =
                ((i & 1) ? destinationRect[0] + destinationRect[2] : destinationRect[0]) - half;
            s_spriteQuadVertex0.vertices[i].y =
                ((i & 2) ? destinationRect[1] + destinationRect[3] : destinationRect[1]) - half;
        }
        s_spriteQuadVertex0.vertices[i].scale = kFloat_0_B2d;
        s_spriteQuadVertex0.vertices[i].one = kFloat_1_BitmapDraw;
        s_spriteQuadVertex0.vertices[i].color = ((unsigned int)packedColor);
        s_spriteQuadVertex0.vertices[i].u = (((i & 1) != 0) == flipX) ? texRect[0] + texRect[2] : texRect[0];
        s_spriteQuadVertex0.vertices[i].v = (((i & 2) != 0) == flipY) ? texRect[1] + texRect[3] : texRect[1];
    }
    // Phase 3: rotate around the destination center, bind texture and submit.
    float absAngle;
    if ((*(unsigned int*)&angleBits & 0x80000000) != 0) {
        absAngle = -(float)savedAngle;
    } else {
        absAngle = (float)savedAngle;
    }
    int small = absAngle < kFloat_0_00001_BitmapDraw;
    if (!small) {
        float cosAngle = Math_CosApprox((float)savedAngle);
        float sinAngle = Math_SinApprox((float)savedAngle);
        float cy = kFloat_0_5_BitmapDraw * destinationRect[3] + destinationRect[1];
        float cx = kFloat_0_5_BitmapDraw * destinationRect[2] + destinationRect[0];
        for (int i = 0; i < 4; ++i) {
            float dy = s_spriteQuadVertex0.vertices[i].y - cy;
            float dx = s_spriteQuadVertex0.vertices[i].x - cx;
            float newX = cx + (dx * cosAngle - dy * sinAngle);
            float newY = cy + (dx * sinAngle + dy * cosAngle);
            s_spriteQuadVertex0.vertices[i].x = newX;
            s_spriteQuadVertex0.vertices[i].y = newY;
        }
    }
    WView_SetOptionalTextureOverride(camera, texture);
    ((SpriteRenderDispatch*)camera)
        ->render(s_spriteQuadVertexPointers, 0x20, (unsigned int)renderFlags | kSpriteSubmitFlags, 0);
    WView_SetOptionalTextureOverride(camera, 0);
}

// Apply horizontal/vertical anchoring and optional 640x480 coordinate scaling.
void DrawText_TransformRectToViewport(float* out, WView* camera, const float* input, int flags) {
    // X anchor: 0 = left, bit 2 = center, otherwise mask 6 = right.
    // Y anchor: 0 = top, bit 0x20 = center, otherwise mask 0x60 = bottom.
    // Position and size scale independently from the 640x480 reference canvas.
    float xOffset;
    if ((flags & kTextHorizontalAnchorMask) == 0) {
        xOffset = kFloat_0_B2d;
    } else {
        float xScale;
        if ((flags & kTextCenterX) != 0) {
            xScale = kFloat_0_5_BitmapDraw;
        } else {
            xScale = kFloat_1_BitmapDraw;
        }
        float w = camera->viewportWidth;
        xOffset = xScale * w;
    }
    float yOffset;
    if ((flags & kTextVerticalAnchorMask) == 0) {
        yOffset = kFloat_0_B2d;
    } else {
        float yScale;
        if ((flags & kTextCenterY) != 0) {
            yScale = kFloat_0_5_BitmapDraw;
        } else {
            yScale = kFloat_1_BitmapDraw;
        }
        float h = camera->viewportHeight;
        yOffset = yScale * h;
    }
    if ((flags & kTextScalePosition) != 0) {
        float width = camera->viewportWidth;
        out[0] = xOffset + input[0] * width / kFloat_640_TextViewport;
        float height = camera->viewportHeight;
        out[1] = yOffset + input[1] * height / kFloat_480_Viewport;
    } else {
        out[0] = xOffset + input[0];
        out[1] = yOffset + input[1];
    }
    if ((flags & kTextScaleSize) != 0) {
        float width = camera->viewportWidth;
        out[2] = input[2] * width / kFloat_640_TextViewport;
        float height = camera->viewportHeight;
        out[3] = input[3] * height / kFloat_480_Viewport;
    } else {
        out[2] = input[2];
        out[3] = input[3];
    }
}

// Rotate about the supplied pivot, clip crossed viewport edges, and render the polygon.
void Sprite_DrawRotatedClippedQuad(void* self, WView* camera, void* texture, float pivotX, float pivotY, float angle,
                                   float* sourceUvRect, float* destinationRect, void* renderFlags, void* packedColor,
                                   int flags) {
    // pivot is already in viewport pixels. A nonzero angle clips the rotated
    // polygon to the VIEWPORT, using up to eight vertices; the zero-angle path
    // delegates to the rectangle clipper, which also honors overlay clipRect.
    double savedAngle = angle;
    ClippedSpriteVertex clipped[8];
    float absAngle;

    if (!((*(unsigned int*)&angle & 0x80000000u) != 0)) {
        absAngle = (float)savedAngle;
    } else {
        absAngle = -(float)savedAngle;
    }
    bool angleIsZero = absAngle < kFloat_0_00001_BitmapDraw;
    if ((angleIsZero)) {
        Sprite_DrawClippedTexturedQuad(self, camera, texture, sourceUvRect, destinationRect, kFloat_0_B2d, renderFlags,
                                       packedColor, (void*)flags, 1);
        return;
    }

    // Phase 1: construct the four corners, retaining their UV correspondence.
    float half = kFloat_0_5_BitmapDraw;
    float zero = kFloat_0_B2d;
    int outcode = 0;
    float one = kFloat_1_BitmapDraw;

    SpriteQuadVertex* vertex = s_spriteQuadVertex0.vertices;
    for (int i = 0; 4 > i; ++i) {
        if ((AsWOverlay(self)->coordMode & kSpritePixelSnap) != 0) {
            int rightCorner = 1 & i;
            float px = floor((rightCorner) != 0 ? destinationRect[0] + destinationRect[2] : destinationRect[0]);
            vertex->x = px - half;
            int bottomCorner = i & 2;
            float py = floor((bottomCorner) != 0 ? destinationRect[1] + destinationRect[3] : destinationRect[1]);
            vertex->y = py - half;
        } else {
            vertex->x = ((i & 1) != 0 ? destinationRect[0] + destinationRect[2] : destinationRect[0]) - half;
            vertex->y = ((2 & i) != 0 ? destinationRect[1] + destinationRect[3] : destinationRect[1]) - half;
        }
        vertex->z = zero;
        vertex->w = one;
        vertex->color = ((unsigned int)packedColor);
        int flipX = !(flags & 1);
        int rightUvCorner = i & 1;
        vertex->u = ((rightUvCorner) != 0) == flipX ? sourceUvRect[0] + sourceUvRect[2] : sourceUvRect[0];
        vertex->v = ((i & 2) != 0) == (!((flags & 2))) ? sourceUvRect[1] + sourceUvRect[3] : sourceUvRect[1];
        SpriteQuadVertex* nextVertexAlias0 = vertex + 1;
        SpriteQuadVertex* nextVertexAlias1 = vertex + 1;
        SpriteQuadVertex* sum = vertex + 1;
        vertex = sum;
    }

    // Phase 2: rotate pixel coordinates about the supplied pivot and OR edge codes.
    float cosAngle = Math_CosApprox(savedAngle);
    float sinAngle = Math_SinApprox(savedAngle);
    vertex = s_spriteQuadVertex0.vertices;
    for (int i = 0; 4 > i; ++i) {
        float vertexXAlias0 = vertex->x;
        float vertexXAlias1 = vertex->x;
        float vertexXAlias2 = vertexXAlias1;
        float vertexXAlias3 = vertex->x;
        float vertexXAlias4 = vertexXAlias2;
        float vertexXAlias5 = vertex->x;
        float vertexXAlias6 = vertexXAlias4;
        float vertexXAlias7 = vertex->x;
        float vertexXAlias8 = vertexXAlias5;
        float vertexXAlias9 = vertexXAlias7;
        float vertexXAlias10 = vertex->x;
        float vertexXAlias11 = vertexXAlias8;
        float vertexXAlias12 = vertexXAlias10;
        float vertexXAlias13 = vertex->x;
        float vertexXAlias14 = vertexXAlias11;
        float vertexXAlias15 = vertex->x;
        float vertexXAlias16 = vertexXAlias13;
        float vertexYAlias0 = vertex->y;
        float vertexYAlias1 = vertexYAlias0;
        float vertexXAlias17 = vertexXAlias16;
        float vertexYAlias2 = vertexYAlias1;
        // The aliases preserve local symbol numbering for a later cleanup destructor.
        // Effective rotation: x = pivotX + dx*cos - dy*sin; y = pivotY + dx*sin + dy*cos.
        float savedX = pivotX;
        float vertexYAlias3 = vertex->y;
        float vertexYAlias4 = vertex->y;
        float vertexYAlias5 = vertexYAlias4;
        float vertexYAlias6 = vertex->y;
        float vertexYAlias7 = vertexYAlias5;
        float cosAngleAlias0 = cosAngle;
        float vertexYAlias8 = vertexYAlias7;
        float cosAngleAlias1 = cosAngleAlias0;
        float savedY = pivotY;
        float dy = vertexYAlias8 - savedY;
        float ySinAlias0 = dy * sinAngle;
        float product = dy * sinAngle;
        float dx = vertexXAlias17 - savedX;
        float newX = savedX + (dx * cosAngleAlias1 - product);
        float newXValue = newX;
        vertex->x = newXValue;
        float newY = savedY + (dx * sinAngle + dy * cosAngle);
        float newYValue = newY;
        vertex->y = newYValue;
        if (zero > newX)
            outcode |= kOutsideLeft;
        if (camera->viewportWidth < vertex->x)
            outcode |= kOutsideRight;
        if (vertex->y < zero)
            outcode |= kOutsideTop;
        if (camera->viewportHeight < vertex->y)
            outcode |= kOutsideBottom;
        vertex += 1;
    }

    // Phase 3: reorder grid corners 0,1,3,2 into polygon winding, then clip
    // left/right/top/bottom. Each pass consumes the previous pass's polygon.
    SpriteQuadVertex local[8];
    int count = 4;
    memcpy(&local[0], &s_spriteQuadVertex0.vertices[0], sizeof(SpriteQuadVertex));
    memcpy(&local[1], &s_spriteQuadVertex0.vertices[1], sizeof(SpriteQuadVertex));
    memcpy(&local[2], &s_spriteQuadVertex0.vertices[3], sizeof(SpriteQuadVertex));
    memcpy(&local[3], &s_spriteQuadVertex0.vertices[2], sizeof(SpriteQuadVertex));
    if ((kOutsideLeft & outcode) != 0) {
        Sprite_ClipPolygonToLeftEdge(self, camera, clipped, (ClippedSpriteVertex*)local, &count, count);
        memcpy(local, clipped, count * sizeof(SpriteQuadVertex));
    }
    if ((outcode & kOutsideRight) != 0) {
        Sprite_ClipPolygonToRightEdge(self, camera, clipped, (ClippedSpriteVertex*)local, &count, count);
        memcpy(local, clipped, count * sizeof(SpriteQuadVertex));
    }
    if ((outcode & kOutsideTop) != 0) {
        Sprite_ClipPolygonToTopEdge(self, camera, clipped, (ClippedSpriteVertex*)local, &count, count);
        memcpy(local, clipped, count * sizeof(SpriteQuadVertex));
    }
    if ((outcode & kOutsideBottom) != 0) {
        int countValue = count;
        Sprite_ClipPolygonToBottomEdge(self, camera, clipped, (ClippedSpriteVertex*)local, &count, countValue);
        memcpy(local, clipped, count * sizeof(SpriteQuadVertex));
    }
    // Phase 4: restore fields not written by clippers and submit a terminated
    // pointer list. These stack-backed pointers are valid only during render().
    if (outcode) {
        if (count != 0) {
            for (int i = 0; i < count; ++i) {
                s_clippedSpriteVertexPointers[i] = &local[i];
                s_clippedSpriteVertexPointers[i]->w = one;
                s_clippedSpriteVertexPointers[i]->color = ((unsigned int)packedColor);
                s_clippedSpriteVertexPointers[i]->z = zero;
                s_clippedSpriteVertexPointers[i]->clipFlags = 0;
            }
            int countValue2 = count;
            s_clippedSpriteVertexPointers[countValue2] = 0;
            WView_SetOptionalTextureOverride(camera, texture);
            ((SpriteRenderDispatch*)camera)
                ->render(s_clippedSpriteVertexPointers, 0x20, ((unsigned int)renderFlags | kSpriteSubmitFlags), 0);
            WView_SetOptionalTextureOverride(camera, 0);
        }
    } else {
        WView_SetOptionalTextureOverride(camera, texture);
        ((SpriteRenderDispatch*)camera)
            ->render(s_spriteQuadVertexPointers, 0x10, ((unsigned int)renderFlags | kSpriteSubmitFlags), 0);
        WView_SetOptionalTextureOverride(camera, 0);
    }
}

// Convert the destination rectangle to viewport coordinates before rotating its quad.
void Sprite_DrawRotatedQuadWithViewportRect(void* self, void* camera, void* sourceUvRect, float pivotX, float pivotY,
                                            float rotationAngle, const float* transform, void* renderFlags,
                                            void* packedColor, void* uvFlipFlags) {
    float destinationRect[4];
    struct Rect {
        float x, y, z, w;
        // Reorder the reverse ABI arguments into [left,top,width,height].
        Rect(float d, float c, float b, float a) : x(a), y(b), z(c), w(d) {}
    };
    Rect input(transform[3], transform[2], transform[1], transform[0]);
    DrawText_TransformRectToViewport(destinationRect, (WView*)camera, (float*)&input, AsWOverlay(self)->coordMode);
    Sprite_DrawRotatedClippedQuad(self, (WView*)camera, ((OverlayTextureView*)self)->texture, pivotX, pivotY,
                                  rotationAngle, (float*)sourceUvRect, destinationRect, renderFlags, packedColor,
                                  (int)(long)uvFlipFlags);
}

// Destroy the resource base and optionally free its allocation.
void* __dt__9WResourceFv(void* self, int deleting) {
    if (self != 0) {
        BaseObject_dtor(self, 0);
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Normalize an image path and wait for its shared texture; return nonzero on failure.
int WOverlay_Load(OverlayTextureView* self, const char* text) {
    // Releases the previous retained texture before trying text. The replacement
    // is retained by self, including failure as null. Busy-waits with no timeout;
    // return 0 is success and return 1 means no resource was obtained.
    struct CharBox {
        char c;
    };
    CharBox terminatorBox;
    CharBox sourceBox;
    void* result;
    if (self->texture != 0) {
        ((OverlayDrawDispatch*)self->texture)->release();
        self->texture = 0;
    }
    {
        OverlayPathString local;
        local.header.flags = 0;
        local.secondWord = 0;
        local.heapText = 0;
        unsigned int length = strlen(text);
        String_ReserveCapacity(&local, length);
        terminatorBox.c = *(volatile char*)&sourceBox.c;
        String_ReplaceRange(&local, 0, 0, text, text + length, &terminatorBox.c);
        struct StringCleanup {
            void* object;
            // Free normalized-path text only when the small string owns a heap block.
            ~StringCleanup() {
                unsigned int* words = (unsigned int*)object;
                if (words[0] >> 31)
                    appFreeUni__FPv(*(char**)(words + 2));
            }
        } cleanup = {&local};
        ResourcePath_NormalizeImageExtension(&local);
        if (((CHashPath*)(self->resourcePaths))
                ->Exists((local.header.flags >> 31) == 0 ? (char*)&local + 1 : local.heapText) == 0) {
            result = 0;
        } else {
            result = CreateSharedTextureResource__FPCci(
                (local.header.flags >> 31) == 0 ? (char*)&local + 1 : local.heapText, 0);
            if (result != 0 && Get__15ISharedResourceFv(result) == 0) {
                while (Get__15ISharedResourceFv(result) == 0) {
                }
            }
        }
    }
    self->texture = result;
    return result == 0;
}

// Release the overlay texture, destroy the base, and optionally free the overlay.
void* WOverlay_dtor(OverlayTextureView* self, int deleting) {
    if (self != 0) {
        void* subObject = self->texture;
        *(void**)self = &__vt__8WOverlay;
        if (subObject != 0) {
            ((OverlayDrawDispatch*)subObject)->release();
            self->texture = 0;
        }
        if (self != 0) {
            BaseObject_dtor(self, 0);
        }
        if (deleting > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Apply overlay anchoring and draw its texture in the transformed rectangle.
void WOverlay_DrawTransformedRect(void* self, void* camera, void* sourceUvRect, float rotationAngle, float* transform,
                                  void* renderFlags, void* packedColor, void* uvFlipFlags) {
    float width = transform[2];
    float left = transform[0];
    float height = transform[3];
    float destinationRect[4];

    float input[4];
    input[1] = (transform[1]);
    input[0] = left;

    input[2] = width;
    input[3] = height;
    DrawText_TransformRectToViewport(destinationRect, (WView*)camera, input, AsWOverlay(self)->coordMode);
    Sprite_DrawClippedTexturedQuad(self, camera, ((OverlayTextureView*)self)->texture, sourceUvRect, destinationRect,
                                   rotationAngle, renderFlags, packedColor, uvFlipFlags, 1);
}

} // extern "C"
