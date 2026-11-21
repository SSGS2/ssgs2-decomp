// Translation unit 0x803E56D8-0x803E68B0 (16 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".
// Bitmap-font text batching and drawing. Mod lookup: FormatTextVertices appends
// Shift-JIS text as glyph quads; s_bitmapFontGlyphScaleXY holds horizontal/vertical scale,
// s_bitmapFontHorizontalBearing the horizontal bearing adjustment. Draw consumes the queued batch.
// Resource/font/texture initialization is in gtl_bitmap_font_and_anim_keys.cpp; glyph metrics are
// read through gtl_FontData_FindGlyph, not synthesized here. All font globals
// belong to one shared renderer: serialize format/draw/teardown operations.
// No clock or gameplay distance is used. Coordinates below are font render-space
// units; texture coordinates begin as atlas texels and WriteGlyphQuad converts
// them for the bound vertex format. No aspect/viewport conversion occurs here.
#include <types.h>
#include "sdk_protos.h"
#include <decomp/cw_bitfields.h>

struct StaticStringObj {
    u32 a;
    u32 b;
    u32 c;
};

extern "C" {


extern StaticStringObj s_bitmapFontPathString;
extern u8 s_bitmapFontStringRegistrationSlot[];
void MWString_dtor(void *, int);

extern s32 s_bitmapFontInitialized;
extern void *s_bitmapFontVertexBuffer;
extern s32 s_bitmapFontPrimitiveType;
extern s32 s_bitmapFontDefaultPackedColor;
extern s32 s_bitmapFontQueuedGlyphCount;
extern void *s_bitmapFontResourceBuffer;
extern s32 s_bitmapFontHorizontalBearing;
extern s32 s_bitmapFontLastPageRunIndex;
extern f32 s_bitmapFontGlyphScaleXY[2];
extern u8 s_bitmapFontState[];
// Global batch ledger (symbol names kept as-is):
// DD32C initialized; DD338 owned shared vertex buffer; DD330 loaded font buffer;
// DD33C queued drawable glyph count; DD328 last page-run index, initially zero.
// DC180 primitive mode (supported append mode 0x98); DC184 quad texture mode;
// DC188[2] x/y scales. B1920 is PageTab plus the header/quad lookup tables below.

void gtl_BitmapFont_Initialize(int);
void *gtl_FontData_InitView(void *self, void *font);
int gtl_FontData_FindGlyph(void *font, unsigned int c, void *glyph);
void gtl_BitmapFont_WriteGlyphQuad(void *dst, s32 flags, s32 a, void *b, void *c, void *uv, void *xy, s32 tex, f32 z);
int Text_ConvertShiftJisUsingCodeTable(void *, void *, unsigned char *text, int byteLimit, unsigned short *result, int useTable);
int ShiftJIS_IsLeadByte(void *self, unsigned int c);
void VertexBuffer_GetDescription(void *buffer, void *desc);
void Resource_GetOffsetPtr(void *buffer, int offset, int size, void *out, int flags);
void GraphicsBuffer_Flush(void *buffer);
void FontDataView_dtor(void *, int);
int vsprintf(char *, const char *, void *);

struct Glyph {
    u16 u;
    u16 v;
    u16 page;
    s16 xoff;
    s16 w;
    s16 adv;
};

struct FontTab {
    u8 unk0[0x10];
    s32 lineHeight;
};

struct PageSlot {
    s32 count;
    s32 page;
};

struct Vec3 {
    f32 x;
    f32 y;
    f32 z;
};

struct Vec3Bits {
    u32 x;
    u32 y;
    u32 z;
};

struct HeapObj {
    unsigned char data[16];
    // Borrow a font-buffer view for glyph lookup; the shared font remains owned globally.
    HeapObj(void *arg) { gtl_FontData_InitView(this, arg); }
    // Destroy only the temporary view, through the resource-view destructor ABI.
    ~HeapObj() { FontDataView_dtor(this, -1); }
};

// Append formatted Shift-JIS text at a borrowed {x,y,z} origin; return no status.
// Required origin/format and valid printf arguments. vsprintf is unbounded:
// formatted BYTES must total <=1023 plus NUL, with complete Shift-JIS pairs.
// wbuf holds 1024 u16 codes; conversion must also terminate within that capacity.
// Initialization creates 0x48000 bytes = 2048 glyph slots of 0x90 bytes each.
// Existing+new drawable glyph count must fit 2048, and page-run index <2048;
// neither capacity is checked. Every glyph.page must be <20 (texture arrays).
// Require initialized primitive mode 0x98. Unsupported modes have no safe append
// layout here. Missing glyphs use space; the font MUST contain that fallback.
// Newline resets x to origin.x and adds unscaled lineHeight to y. Other glyphs
// use x/y scale for their quad, add the bearing adjustment, and advance x by
// scaleX*(width+advance); z is copied unchanged. Input origin is never written.
// Writes shared vertices/page runs and increments DD33C, then flushes the buffer.
// Draw later clears the batch; buffer/font/texture ownership remains global.
void gtl_BitmapFont_FormatTextVertices(Vec3 *origin, const char *format, ...) {
    struct {
        unsigned int argumentState;
        void *registerSaveArea;
        void *overflowArguments;
    } ap;
    char cbuf[0x400];
    u16 wbuf[0x400];
    Vec3 pos;
    u32 desc[4];
    u8 *verts;
    u8 *dst;
    u8 *tabBase = s_bitmapFontState;
    char *p;
    s32 count;
    s32 processedCodepoints;
    s32 glyphByteOffset;
    PageSlot *pageBatches;
    FontTab *fontMetrics;
    u16 *wp;

    if (s_bitmapFontInitialized == 0) {
        gtl_BitmapFont_Initialize(0);
    }
    __va_setup(&ap);
    vsprintf(cbuf, format, &ap);
    *(Vec3Bits *)&pos = *(Vec3Bits *)origin;
    p = cbuf;
    count = 0;
    while (true) {
        if (*p == 0) {
            break;
        }
        if (ShiftJIS_IsLeadByte(0, (unsigned char)*p) != 0) {
            p += 2;
        } else {
            p += 1;
        }
        count += 1;
    }
    VertexBuffer_GetDescription(s_bitmapFontVertexBuffer, desc);
    s32 cnt = s_bitmapFontQueuedGlyphCount;
    s32 stride;
    switch (s_bitmapFontPrimitiveType) {
    case 0x98:
        stride = cnt * 6;
        break;
    default:
        stride = 0;
        break;
    }
    s32 off = stride * 0x18;
    Resource_GetOffsetPtr(s_bitmapFontVertexBuffer, off, desc[2] - off, &verts, 0);
    dst = verts;
    HeapObj fontView(s_bitmapFontResourceBuffer);
    Glyph glyph;
    f32 xy[4];
    f32 uv[4];
    // Zero all 1024 u16 codes as 256 pairs of u32 words. An earlier version anchored
    // this at &ap+12; its compiled stack placed wbuf at exactly that address.
    // Anchor to wbuf directly so moving locals cannot change the target buffer.
    // The one-word-before cursor is never dereferenced: store [1], advance two
    // words, store [0]. Keep that store/update shape for CodeWarrior's stwu.
    u32 n = 0x100;
    u32 *clr = (u32 *)wbuf - 1;
    do {
        clr[1] = 0;
        clr += 2;
        clr[0] = 0;
    } while (--n);
    // -1 auto-scans the NUL within 1024 input bytes. Last argument 0x400 is
    // merely nonzero "use conversion table", NOT an output-capacity argument
    // (gtl_heap_allocators.cpp); capacity safety follows from the formatted-byte bound.
    Text_ConvertShiftJisUsingCodeTable(0, 0, (unsigned char *)cbuf, -1, wbuf, 0x400);
    wp = wbuf;
    // B1920+0x50: 2048 {count,page} runs; +0x4050: 20 palette words;
    // +0x40A0: copied font header (lineHeight at +0x10). Quad tables at
    // +0x40C8/+0x4398 are passed through unchanged to WriteGlyphQuad.
    fontMetrics = (FontTab *)((u8 *)s_bitmapFontState + 0x40A0);
    pageBatches = (PageSlot *)(tabBase + 0x50);
    glyphByteOffset = 0;
    processedCodepoints = 0;
    while (true) {
        switch (s_bitmapFontPrimitiveType) {
        case 0x98:
            dst = verts + glyphByteOffset + 0x18;
            break;
        }
        u16 c = *wp;
        if (c == 0) {
            break;
        }
        if (c == 0xA) {
            count -= 1;
            pos.x = origin->x;
            pos.y += (f32)fontMetrics->lineHeight;
        } else {
            if (gtl_FontData_FindGlyph(&fontView, c, &glyph) == 0) {
                *wp = 0x20;
                gtl_FontData_FindGlyph(&fontView, 0x20, &glyph);
            }
            s32 line = fontMetrics->lineHeight;
            f32 u0 = (f32)glyph.u;
            f32 v0 = (f32)glyph.v;
            f32 x0 = pos.x + (((f32)glyph.xoff * s_bitmapFontGlyphScaleXY[0]) + (f32)s_bitmapFontHorizontalBearing);
            f32 u1 = (f32)(glyph.u + glyph.w);
            f32 v1 = (f32)(glyph.v + line);
            f32 wd = (f32)glyph.w * s_bitmapFontGlyphScaleXY[0];
            f32 x1 = x0 + wd;
            uv[0] = u0;
            uv[1] = v0;
            uv[2] = u1;
            uv[3] = v1;
            xy[0] = x0;
            xy[1] = pos.y;
            xy[2] = x1;
            xy[3] = pos.y + ((f32)line * s_bitmapFontGlyphScaleXY[1]);
            pos.x = x0;
// The definition reads nine arguments (z = pos.z); the original call also loads r9/r10 and f2..f8, which the
// callee ignores.
            ((void (*)(void *, s32, s32, void *, void *, void *, void *, s32, s32, s32, f32, f32, f32, f32, f32, f32, f32, f32))gtl_BitmapFont_WriteGlyphQuad)(dst, 0x11F005, s_bitmapFontPrimitiveType, tabBase + 0x40C8, tabBase + 0x4398, uv, xy, s_bitmapFontDefaultPackedColor, 0, 0, pos.z, x1, pos.y, wd, v1, u1, v0, u0);
            switch (s_bitmapFontPrimitiveType) {
            case 0x98:
                Memory_Copy(dst - 0x18, dst, 0x18);
                Memory_Copy(dst + 0x60, dst + 0x48, 0x18);
                break;
            }
            glyphByteOffset += 0x90;
            PageSlot *pageRun = pageBatches + s_bitmapFontLastPageRunIndex;
            pos.x += s_bitmapFontGlyphScaleXY[0] * (f32)(glyph.w + glyph.adv);
            u16 page = glyph.page;
            if (page != pageRun->page) {
                if (s_bitmapFontQueuedGlyphCount != 0 || processedCodepoints != 0) {
                    s_bitmapFontLastPageRunIndex += 1;
                }
                pageRun = pageBatches + s_bitmapFontLastPageRunIndex;
                pageRun->page = page;
            }
            pageRun->count += 1;
        }
        wp += 1;
        processedCodepoints += 1;
    }
    s_bitmapFontQueuedGlyphCount += count;
    GraphicsBuffer_Flush(s_bitmapFontVertexBuffer);
}

extern f32 kIdentityMatrix4x4[16];
void Engine_GetRenderState(u32 state, u32 *value);
void Engine_SetCachedRenderState(u32 state, u32 value);
void Engine_GetTextureStageState(u32 stage, u32 state, u32 *value);
void Engine_SetTextureStageState(u32 stage, u32 state, u32 value);
void Engine_GetRenderStateRowField(u32 stage, u32 state, u32 *value);
void Engine_SetTextureStageParameter(u32 stage, u32 type, u32 value);
void Engine_GetTransform(u32 type, void *matrix);
void Engine_AcquireType6Resource(void **res);
void Engine_AcquireType7Resource(void **res);
void Engine_GetTextureStageResource(u32 stage, void **res);
void Engine_SetTextureStageResource(u32 stage, void *res);
void Render_GetVertexDeclaration(void **res);
void Engine_SetFlexibleVertexFormat(u32 fvf);
void Engine_SetType6Resource(void *res);
void Engine_SetType7Resource(void *res);
void SetTransform(u32 type, const void *matrix);
void Engine_SetTexturePalette(u32 stage, void *tex);
void Engine_SetVertexStreamZeroOffset(u32 stream, void *vb, u32 stride);
void DrawPrimitiveFromBoundBuffers(u32 type, u32 start, u32 count);
void Engine_SetVertexDeclaration(void *decl);
void CubeResourceStorage_dtor(void *obj, s32 flags);
void *memset(void *, int, size_t);

struct SaveTab {
    u32 render[34];
    u32 tex[16];
    u32 param[10];
};
extern SaveTab s_bitmapFontDefaultRenderStates;

struct StateRec {
    u32 id;
    u32 val;
};

struct RenderStateSaver {
    StateRec recs[17];
    u32 saved[17];
    // Capture 17 current render-state values, then apply the font's paired defaults.
    RenderStateSaver(SaveTab *src) {
        Memory_Copy(recs, src->render, 0x88);
        for (u32 i = 0; i < 17; i++) {
            Engine_GetRenderState(recs[i].id, &saved[i]);
            Engine_SetCachedRenderState(recs[i].id, recs[i].val);
        }
    }
    // Restore exactly those 17 values on scope exit/unwind.
    ~RenderStateSaver() {
        for (u32 i = 0; i < 17; i++) {
            Engine_SetCachedRenderState(recs[i].id, saved[i]);
        }
    }
};

struct TexStateSaver {
    StateRec recs[8];
    u32 saved[8];
    // Capture/apply eight stage-zero texture-state values from the font defaults.
    TexStateSaver(SaveTab *src) {
        Memory_Copy(recs, src->tex, 0x40);
        for (u32 i = 0; i < 8; i++) {
            Engine_GetTextureStageState(0, recs[i].id, &saved[i]);
            Engine_SetTextureStageState(0, recs[i].id, recs[i].val);
        }
    }
    // Restore the eight captured stage-zero texture states.
    ~TexStateSaver() {
        for (u32 i = 0; i < 8; i++) {
            Engine_SetTextureStageState(0, recs[i].id, saved[i]);
        }
    }
};

struct ParamSaver {
    StateRec recs[5];
    u32 saved[5];
    // Capture/apply five stage-zero parameter values from the font defaults.
    ParamSaver(SaveTab *src) {
        Memory_Copy(recs, src->param, 0x28);
        for (u32 i = 0; i < 5; i++) {
            Engine_GetRenderStateRowField(0, recs[i].id, &saved[i]);
            Engine_SetTextureStageParameter(0, recs[i].id, recs[i].val);
        }
    }
    // Restore the five captured stage-zero parameters.
    ~ParamSaver() {
        for (u32 i = 0; i < 5; i++) {
            Engine_SetTextureStageParameter(0, recs[i].id, saved[i]);
        }
    }
};

struct TransformGuard {
    f32 mtx[16];
    // Borrow a value copy of transform slot 2 before font drawing sets identity.
    TransformGuard() { Engine_GetTransform(2, mtx); }
    // Restore transform slot 2 from its saved sixteen float words.
    ~TransformGuard() { SetTransform(2, mtx); }
};

struct RefObj {
    CW_BITS2(
        u32 count : 16,
        u32 rest : 16)
};

#define RELEASE_REF(p)                                                         \
    RefObj *q = p;                                                             \
    if (q != 0) {                                                              \
        if (--q->count == 0) {                                                 \
            CubeResourceStorage_dtor(q, 1);                                               \
        }                                                                      \
        p = 0;                                                                 \
    }

struct Type6Guard {
    RefObj *p;
    // Retain current type-6 binding until restoration (numeric engine API kind).
    Type6Guard() { Engine_AcquireType6Resource((void **)&p); }
    // Restore the binding, then relinquish our retained reference.
    ~Type6Guard() {
        Engine_SetType6Resource(p);
        RELEASE_REF(p)
    }
};

struct VFmtGuard {
    RefObj *p;
    // Retain current type-7 binding until restoration; not the vertex declaration.
    VFmtGuard() { Engine_AcquireType7Resource((void **)&p); }
    // Restore type 7 and release the guard's retained reference.
    ~VFmtGuard() {
        Engine_SetType7Resource(p);
        RELEASE_REF(p)
    }
};

struct VDeclGuard {
    RefObj *p;
    // Retain the current vertex declaration before selecting the font FVF.
    VDeclGuard() { Render_GetVertexDeclaration((void **)&p); }
    // Restore the declaration, then release the guard's retained reference.
    ~VDeclGuard() {
        Engine_SetVertexDeclaration(p);
        RELEASE_REF(p)
    }
};

extern "C++" {
template <int N>
struct TexResGuard {
    RefObj *p;
    // Retain stage N's texture resource; Draw instantiates N in [0,7].
    TexResGuard() { Engine_GetTextureStageResource(N, (void **)&p); }
    // Restore this stage's binding and release its retained reference.
    ~TexResGuard() {
        Engine_SetTextureStageResource(N, p);
        RELEASE_REF(p)
    }
};
}

struct PageTab {
    u32 tex0[20];
    PageSlot slots[0x800];
    u32 tex1[20];
};

#pragma use_lmw_stmw on
// Draw and consume the global font batch; no-op if queued glyph count <=0.
// Requires live initialized buffer/font/pages and a valid engine render context.
// Captures/applies SaveTab defaults, detaches stages 0..7, selects font FVF,
// clears type6/type7 bindings and sets transform slot 2 to identity. Scope guards
// restore render/texture/parameter state, transform, vertex declaration, type6/7
// and all eight texture-resource bindings, releasing retained references last.
// Stream 0 and texture palette are set per draw and have NO matching guard here;
// callers must rebind those before later rendering that relies on prior values.
// Each run selects its texture/palette, but the implementation draws from vertex
// 0 with TOTAL (6*glyphCount-2) primitives for EVERY run, ignoring PageSlot.count.
// Do not interpret this as a draw of only that run's glyph subrange or silently
// repair it during readability work. Multi-page text needs this case tested.
// On normal completion DD33C=0, all 2048 runs cleared, DD328=0. No status or
// rollback contract exists for failed engine calls. Resource ownership persists.
void gtl_BitmapFont_Draw(void) {
    s32 startVtx;
    SaveTab *src = &s_bitmapFontDefaultRenderStates;
    PageTab *tab = (PageTab *)s_bitmapFontState;
    if (s_bitmapFontQueuedGlyphCount > 0) {
        u32 i;
        s32 k;
        PageSlot *rec;
        u32 *tex0;
        startVtx = 0;
        u32 *tex1;
        switch (s_bitmapFontPrimitiveType) {
        case 0x98:
            startVtx = (s_bitmapFontQueuedGlyphCount * 6) - 2;
            break;
        }
        RenderStateSaver renderSaver(src);
        TexStateSaver texSaver(src);
        ParamSaver paramSaver(src);
        TransformGuard transformGuard;
        Type6Guard type6;
        VFmtGuard vfmt;
        TexResGuard<0> res0;
        TexResGuard<1> res1;
        TexResGuard<2> res2;
        TexResGuard<3> res3;
        TexResGuard<4> res4;
        TexResGuard<5> res5;
        TexResGuard<6> res6;
        TexResGuard<7> res7;
        i = 0;
        do {
            Engine_SetTextureStageResource(i, 0);
            i += 1;
        } while (i < 8);
        VDeclGuard vdecl;
        Engine_SetFlexibleVertexFormat(0x11F005);
        Engine_SetType6Resource(0);
        Engine_SetType7Resource(0);
        SetTransform(2, kIdentityMatrix4x4);

        rec = tab->slots;
        tex0 = tab->tex0;
        tex1 = tab->tex1;
        k = 0;
        while (k <= s_bitmapFontLastPageRunIndex) {
            Engine_SetTextureStageResource(0, (void *)tex0[rec->page]);
            Engine_SetTexturePalette(0, (void *)tex1[rec->page]);
            Engine_SetVertexStreamZeroOffset(0, s_bitmapFontVertexBuffer, 0x18);
            DrawPrimitiveFromBoundBuffers(s_bitmapFontPrimitiveType, 0, startVtx);
            rec += 1;
            k += 1;
        }

        s_bitmapFontQueuedGlyphCount = 0;
        memset(tab->slots, 0, 0x4000);
        s_bitmapFontLastPageRunIndex = 0;
    }
}
#pragma use_lmw_stmw reset

void __dl__FPv(void *);

// ABI destructor: restore type 6/release its retained reference; optionally free self.
// All ABI guard destructors below accept null; flags>0 frees, otherwise storage remains.
void *Type6Guard_dtor(Type6Guard *self, int flags) {
    if (self != 0) {
        Engine_SetType6Resource(self->p);
        RefObj *q = self->p;
        if (q != 0) {
            if (--q->count == 0) {
                CubeResourceStorage_dtor(q, 1);
            }
            self->p = 0;
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore type 7 and release its saved reference, then optional delete.
void *VFmtGuard_dtor(VFmtGuard *self, int flags) {
    if (self != 0) {
        Engine_SetType7Resource(self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore the saved vertex declaration/release, then optional delete.
void *VDeclGuard_dtor(VDeclGuard *self, int flags) {
    if (self != 0) {
        Engine_SetVertexDeclaration(self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

#pragma push
#pragma use_lmw_stmw on
// ABI destructor: restore the 17 saved render states, then optionally free the guard.
void *RenderStateSaver_dtor(RenderStateSaver *self, int flags) {
    if (self != 0) {
        for (u32 i = 0; i < 17; i++) {
            Engine_SetCachedRenderState(self->recs[i].id, self->saved[i]);
        }
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}
#pragma pop

// ABI destructor: restore transform slot 2, then optionally free the guard.
void *TransformGuard_dtor(TransformGuard *self, int flags) {
    if (self != 0) {
        SetTransform(2, self->mtx);
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 0's retained texture; optionally free self.
void *TexResGuard_stage0_dtor(TexResGuard<0> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(0, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 1's retained texture; optionally free self.
void *TexResGuard1_dtor(TexResGuard<1> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(1, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 2's retained texture; optionally free self.
void *TexResGuard2_dtor(TexResGuard<2> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(2, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 3's retained texture; optionally free self.
void *TexResGuard3_dtor(TexResGuard<3> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(3, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 4's retained texture; optionally free self.
void *TexResGuard_Stage4_dtor(TexResGuard<4> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(4, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 5's retained texture; optionally free self.
void *TexResGuard5_dtor(TexResGuard<5> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(5, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 6's retained texture; optionally free self.
void *TexResGuard6_dtor(TexResGuard<6> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(6, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// ABI destructor: restore/release stage 7's retained texture; optionally free self.
void *TexResGuard_Stage7_dtor(TexResGuard<7> *self, int flags) {
    if (self != 0) {
        Engine_SetTextureStageResource(7, self->p);
        RELEASE_REF(self->p)
        if (flags > 0) {
            __dl__FPv(self);
        }
    }
    return self;
}

// Initialize empty shared font-path string and register its process-exit destruction.
void gtl_BitmapFont_InitStaticString(void) {
    s_bitmapFontPathString.a = 0;
    s_bitmapFontPathString.b = 0;
    s_bitmapFontPathString.c = 0;
    __register_global_object(&s_bitmapFontPathString, (void *)MWString_dtor, s_bitmapFontStringRegistrationSlot);
}

} // extern "C"
