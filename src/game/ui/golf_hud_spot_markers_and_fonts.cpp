#include <decomp/static_init.h>
struct GolfHudFontStrings {
    char holeFont[15]; // +0x0: "[font_hole.gsr"
    char holeGlyphs[19]; // +0xF: "1234567890ym%-.,+>"
    char barFont[14]; // +0x22: "[font_bar.gsr"
    char barGlyphs[17]; // +0x30: "1234567890ym%-. "
    char windFont[15]; // +0x41: "[font_wind.gsr"
    char windGlyphs[21]; // +0x50: "1234567890ym%-.?/:, "
    char clubFont[15]; // +0x65: "[font_club.gsr"
    char hexGlyphs[17]; // +0x74: "0123456789abcdef"
    char largeFont[13]; // +0x85: "[bigfont.gsr"
    char largeGlyphs[21]; // +0x92: "1234567890ym+-./x?p "
    char shopNumberFont[16]; // +0xA7: "num_shop_00.gsr"
    char numberCommaGlyphs[12]; // +0xB7: "1234567890,"
    char statusNumberFont[12]; // +0xC3: "num_sts.gsr"
    char digitGlyphs[11]; // +0xCF: "1234567890"
    char galleryNumberFont[13]; // +0xDA: "gall_num.gsr"
    char commonNumberFont[17]; // +0xE7: "common_num_0.gsr"
    char scoreNumberFont[19]; // +0xF8: "score_num11_20.gsr"
    char scoreGlyphs[14]; // +0x10B: "1723456890-+/"
    char customNumberFont0[18]; // +0x119: "custom_num_00.gsr"
    char customNumberFont1[18]; // +0x12B: "custom_num_01.gsr"
    char customNumberFont2[18]; // +0x13D: "custom_num_02.gsr"
    char customNumberFont3[18]; // +0x14F: "custom_num_03.gsr"
    char staminaNumberFont[15]; // +0x161: "num_stm_00.gsr"
    char systemNumberFont[20]; // +0x170: "num_system15_19.gsr"
    char signedNumberGlyphs[13]; // +0x184: "1234567890-+"
    char resultNumberFont[18]; // +0x191: "num_result_19.gsr"
    char countdownFont[19]; // +0x1A3: "font_countdown.gsr"
    char statusFont0[18]; // +0x1B6: "num_status_00.gsr"
    char statusGlyphs0[13]; // +0x1C8: "1234567890,/"
    char statusFont1[18]; // +0x1D5: "num_status_01.gsr"
    char statusGlyphs1[15]; // +0x1E7: "1234567890,.-+"
    char tipsNumberFont[13]; // +0x1F6: "tips_num.gsr"
    char tipsGlyphs[20]; // +0x203: "1234567890ym+-./xp "
    char partyPointFont[16]; // +0x217: "party_point.gsr"
    char pointGlyph[2]; // +0x227: "p"
    char tipsPointFont[17]; // +0x229: "tips_num_pts.gsr"
};
// Named byte offsets retain the registry constructor's original address scheduling.
enum HudFontStringOffset {
    kHud_holeFont = (int)&((GolfHudFontStrings*)0)->holeFont,
    kHud_holeGlyphs = (int)&((GolfHudFontStrings*)0)->holeGlyphs,
    kHud_barFont = (int)&((GolfHudFontStrings*)0)->barFont,
    kHud_barGlyphs = (int)&((GolfHudFontStrings*)0)->barGlyphs,
    kHud_windFont = (int)&((GolfHudFontStrings*)0)->windFont,
    kHud_windGlyphs = (int)&((GolfHudFontStrings*)0)->windGlyphs,
    kHud_clubFont = (int)&((GolfHudFontStrings*)0)->clubFont,
    kHud_hexGlyphs = (int)&((GolfHudFontStrings*)0)->hexGlyphs,
    kHud_largeFont = (int)&((GolfHudFontStrings*)0)->largeFont,
    kHud_largeGlyphs = (int)&((GolfHudFontStrings*)0)->largeGlyphs,
    kHud_shopNumberFont = (int)&((GolfHudFontStrings*)0)->shopNumberFont,
    kHud_numberCommaGlyphs = (int)&((GolfHudFontStrings*)0)->numberCommaGlyphs,
    kHud_statusNumberFont = (int)&((GolfHudFontStrings*)0)->statusNumberFont,
    kHud_digitGlyphs = (int)&((GolfHudFontStrings*)0)->digitGlyphs,
    kHud_galleryNumberFont = (int)&((GolfHudFontStrings*)0)->galleryNumberFont,
    kHud_commonNumberFont = (int)&((GolfHudFontStrings*)0)->commonNumberFont,
    kHud_scoreNumberFont = (int)&((GolfHudFontStrings*)0)->scoreNumberFont,
    kHud_scoreGlyphs = (int)&((GolfHudFontStrings*)0)->scoreGlyphs,
    kHud_customNumberFont0 = (int)&((GolfHudFontStrings*)0)->customNumberFont0,
    kHud_customNumberFont1 = (int)&((GolfHudFontStrings*)0)->customNumberFont1,
    kHud_customNumberFont2 = (int)&((GolfHudFontStrings*)0)->customNumberFont2,
    kHud_customNumberFont3 = (int)&((GolfHudFontStrings*)0)->customNumberFont3,
    kHud_staminaNumberFont = (int)&((GolfHudFontStrings*)0)->staminaNumberFont,
    kHud_systemNumberFont = (int)&((GolfHudFontStrings*)0)->systemNumberFont,
    kHud_signedNumberGlyphs = (int)&((GolfHudFontStrings*)0)->signedNumberGlyphs,
    kHud_resultNumberFont = (int)&((GolfHudFontStrings*)0)->resultNumberFont,
    kHud_countdownFont = (int)&((GolfHudFontStrings*)0)->countdownFont,
    kHud_statusFont0 = (int)&((GolfHudFontStrings*)0)->statusFont0,
    kHud_statusGlyphs0 = (int)&((GolfHudFontStrings*)0)->statusGlyphs0,
    kHud_statusFont1 = (int)&((GolfHudFontStrings*)0)->statusFont1,
    kHud_statusGlyphs1 = (int)&((GolfHudFontStrings*)0)->statusGlyphs1,
    kHud_tipsNumberFont = (int)&((GolfHudFontStrings*)0)->tipsNumberFont,
    kHud_tipsGlyphs = (int)&((GolfHudFontStrings*)0)->tipsGlyphs,
    kHud_partyPointFont = (int)&((GolfHudFontStrings*)0)->partyPointFont,
    kHud_pointGlyph = (int)&((GolfHudFontStrings*)0)->pointGlyph,
    kHud_tipsPointFont = (int)&((GolfHudFontStrings*)0)->tipsPointFont
};

#include "classes/CameraTransform800D.h"
// Prefix returned by Camera_GetRenderState; matrices are passed directly to SetTransform.
struct MarkerCameraMatrices {
    char cameraParameters[0x20];
    float view[16];
    float projection[16];
};
#include "classes/CSpot.h" // Layout offsets checked in the header.
#include "classes/CSharedDoc.h" // Layout offsets checked in the header.
#include "classes/WView.h"
#define AsWView(p) ((WView*)(p))
#include "classes/CGolfDoc.h"
#define AsCGolfDoc(p) ((CGolfDoc*)(p))
// Translation unit 0x800F1D9C-0x800F30AC (10 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/types.h"

extern "C" {

extern char s_spotTypeInfo;
extern float kFloat_6_2831855;
extern float s_twoPi_SpotRendering;
extern void* g_view;
extern void* g_resourceManager;
extern float kFloat_0_spot, kFloat_1_spot, kFloat_0_5_Marker;
extern float kFloat_2_Marker, kFloat_44_8_Marker;
extern float kFloat_neg1_U800EFEAC;
extern float kFloat_32_GolfView, kFloat_0_015625_GolfView, kFloat_229_5_GolfView, kFloat_640_Spot, kFloat_10_GolfView;
extern void* g_sharedDoc;
extern unsigned int spotRenderStateDefaults[], s_markerTextureStageStates[], s_spotTextureStageParameters[];
extern void Engine_SetTextureStageResource(int, int);
extern void Engine_SetTexturePalette(int, int);
extern void Engine_SetRenderStateList(void*, int);
extern void Engine_SetTextureStageStateList(int, void*, int);
extern void Engine_SetTextureStageParameterList(int, void*, int);
extern void SetTransform(int, void*);
extern void Engine_SetType7Resource(int);
extern void Engine_SetFlexibleVertexFormat(int);
extern void DrawPrimitiveUP(int, int, void*, int);
extern void __as__7WVectorFRC7WVector(void*, void*);
extern void __mi__FRC7WVectorRC7WVector(void*, void*, void*);
extern float Magnitude__7WVectorCFv(void*);
extern void Normalize__7WVectorFv(void*);
extern void __ml__FfRC7WVector(void*, void*, float);
extern void __pl__FRC7WVectorRC7WVector(void*, void*, void*);
extern void __apl__7WVectorFRC7WVector(void*, void*);
extern void Vec3_CopyToOutput(void*, void*);
extern void* GolfPlayer_GetEntryByIndex(void*, unsigned char);
extern void* Camera_GetRenderState(void*);
extern void __destroy_arr(void*, void*, int, int);
extern void CBpTex_dtor(void*);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void __dl__FPv(void*);
extern void __dla__FPv(void*);
extern void ResourceManager_DeleteResource(void*, void*);
extern void WView_ProjectWorldPoint(void*, void*, void*);
extern char cInfoBaseVtable[];
extern char kIdentityMatrix4x4[];
extern char s_cameraTransform[];
extern void __as__7WVectorFRC7WVector(void*, void*);

extern void TableLock_ctor(void*, int, int);
extern void* memset(void*, int, unsigned int) throw();
extern void* __dt__9TableLockFv(void*, int);
extern void CInfoBase_RegisterNamedInfo(void*, const char*, int, int, int, int, int, const char*, int);
extern unsigned char s_infoBaseFontRegistryInitialized;
extern char golfHudFontStrings[];

enum SpotDrawConstants {
    kSpotTexturedVertexFormat = 0x11F001, kSpotTriangleStrip = 0x98,
    kSpotVertexBytes = 0x18, kSpotDestinationBlendValueWord = 17,
    kSpotDestinationBlendFactor = 5, kSpotMaterialIndexMask = 0x7FF,
    kSpotProjectedDrawFlags = 0x02400000,
    // Element counts are (state, value) word pairs: block word count / 2.
    kSpotRenderStatePairs = 13, kSpotTextureStageStatePairs = 6, kSpotTextureStageParamPairs = 5,
    // SetTransform slots: view and projection get the camera matrices; the other two are reset to identity.
    kXformView = 0, kXformProjection = 1, kXformIdentitySlotA = 2, kXformIdentitySlotB = 11
};
// spotRenderStateDefaults is a sequence of (state,value) words. Word 17 is the
// value of pair 8, whose state ID (word 16) is 14: destination blend factor.
// This overrides its default zero with factor 5; word 17 is not engine state ID 17.
struct Vtx { float x, y, z; unsigned char c[4]; float u, v; };
struct UV3 { unsigned int w[3]; };
struct SpotTextureStageStateBlock { unsigned int w[12]; };
struct SpotTextureStageParamBlock { unsigned int w[10]; };
struct SpotRenderStateBlock { unsigned int w[26]; };

// Render-target virtual ABI: s0(item) supplies a texture-resource handle for
// Engine_SetTextureStageResource; s1(item) supplies its GX texture object for
// Engine_SetTexturePalette. item is borrowed; neither call releases or transfers it here.
struct GfxVt {
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03(); virtual void v04();
    virtual void v05(); virtual void v06(); virtual void v07(); virtual void v08(); virtual void v09();
    virtual void v10(); virtual void v11(); virtual void v12(); virtual void v13(); virtual void v14();
    virtual void v15(); virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23(); virtual void v24();
    virtual void v25();
    virtual int s0(void*);
    virtual int s1(void*);
};

struct HoleMarkerRecord {
    float teePosition[3];
    Vec3f pinPosition;
    char holeMetadata[8];
};
struct TutorialMarkerPosition {
    char golfDocPrefix[0xAC];
    float position[3];
};
struct SpotOverlayStorage {
    char spotPrefix[0xD0];
    char markerOverlays[9][0x28];
};

struct Obj1D9C {
    char pad0[0x24];
    Vec3f pos;
    char pad1[0x90 - 0x30];
    unsigned int color;
    char pad2[0xA4 - 0x94];
    void* playerMarkerBitmap;
    void* item;
};

// Draw a ground marker whose opacity fades with hole distance and camera distance.
// The ground basis is XZ toward the pin. Opacity uses the lesser of hole-distance
// and camera-distance fades. Positions and the 10-unit half-size use game world units.
void CSpot_DrawTexturedMarker(Obj1D9C* spot) {
    Vtx verts[4];
    SpotRenderStateBlock renderStates;
    SpotTextureStageParamBlock textureParameters;
    SpotTextureStageStateBlock textureStates;
    Vec3f cur;
    Vec3f pos;
    Vec3f perp;
    Vec3f origin;
    Vec3f cameraDisplacement;
    Vec3f scaled;
    Vec3f sum;
    Vec3f rotB;
    Vec3f rotA;
    float distanceOpacity[2];
    void* scene = (void*)((CSharedDoc*)g_sharedDoc)->golfDoc /* 0xB90 */;
    unsigned char index = *((unsigned char*)&AsCGolfDoc(scene)->currentHoleIndex);
    HoleMarkerRecord* selected = (HoleMarkerRecord*)AsCGolfDoc(scene)->holeRecords + index;
    *(UV3*)&origin = *(UV3*)&selected->pinPosition;
    __mi__FRC7WVectorRC7WVector(&pos, &origin, &spot->pos);
    pos.y = kFloat_0_spot;
    float nx = kFloat_neg1_U800EFEAC * pos.x;
    perp.x = pos.z;
    perp.y = kFloat_0_spot;
    perp.z = nx;
    float holeDistance = Magnitude__7WVectorCFv(&pos);
    holeDistance -= kFloat_32_GolfView;
    float holeFade = holeDistance * kFloat_0_015625_GolfView;
    float clampedHoleFade;
    if (holeFade < kFloat_0_spot) clampedHoleFade = kFloat_0_spot;
    else if (holeFade > kFloat_1_spot) clampedHoleFade = kFloat_1_spot;
    else clampedHoleFade = holeFade;
    distanceOpacity[0] = kFloat_229_5_GolfView * clampedHoleFade;
    __mi__FRC7WVectorRC7WVector(&cameraDisplacement, AsCameraTransform800D(s_cameraTransform)->position, &spot->pos);
    float cameraFade = kFloat_1_spot - Magnitude__7WVectorCFv(&cameraDisplacement) / kFloat_640_Spot;
    float clampedCameraFade = cameraFade < kFloat_0_spot ? kFloat_0_spot : (cameraFade > kFloat_1_spot ? kFloat_1_spot : cameraFade);
    distanceOpacity[1] = kFloat_229_5_GolfView * clampedCameraFade;
    Normalize__7WVectorFv(&pos);
    Normalize__7WVectorFv(&perp);
    float two;
    float s2;
    float ten;
    ten = kFloat_10_GolfView;
    renderStates = *(SpotRenderStateBlock*)spotRenderStateDefaults;
    unsigned int markerPolicy = kSpotDestinationBlendFactor;
    renderStates.w[kSpotDestinationBlendValueWord] = markerPolicy;
    textureStates = *(SpotTextureStageStateBlock*)s_markerTextureStageStates;
    textureParameters = *(SpotTextureStageParamBlock*)s_spotTextureStageParameters;
    Engine_SetRenderStateList(&renderStates, kSpotRenderStatePairs);
    Engine_SetTextureStageStateList(0, &textureStates, kSpotTextureStageStatePairs);
    Engine_SetTextureStageParameterList(0, &textureParameters, kSpotTextureStageParamPairs);
    void* transform = Camera_GetRenderState(g_view);
    SetTransform(kXformProjection, ((MarkerCameraMatrices*)transform)->projection);
    SetTransform(kXformView, ((MarkerCameraMatrices*)transform)->view);
    Engine_SetType7Resource(0);
    SetTransform(kXformIdentitySlotA, kIdentityMatrix4x4);
    SetTransform(kXformIdentitySlotB, kIdentityMatrix4x4);
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTextureStageResource(0, g->s0(spot->item));
    }
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTexturePalette(0, g->s1(spot->item));
    }
    Engine_SetFlexibleVertexFormat(kSpotTexturedVertexFormat);
    verts[0].c[0] = (unsigned char)(spot->color >> 16);
    verts[0].c[1] = (unsigned char)(spot->color >> 8);
    verts[0].c[2] = (unsigned char)spot->color;
    float alpha = distanceOpacity[1];
    alpha = (distanceOpacity[0] > alpha) ? alpha : distanceOpacity[0];
    verts[0].c[3] = (unsigned char)(int)alpha;
    verts[1].c[0] = verts[0].c[0]; verts[1].c[1] = verts[0].c[1]; verts[1].c[2] = verts[0].c[2]; verts[1].c[3] = verts[0].c[3];
    verts[2].c[0] = verts[0].c[0]; verts[2].c[1] = verts[0].c[1]; verts[2].c[2] = verts[0].c[2]; verts[2].c[3] = verts[0].c[3];
    verts[3].c[0] = verts[0].c[0]; verts[3].c[1] = verts[0].c[1]; verts[3].c[2] = verts[0].c[2]; verts[3].c[3] = verts[0].c[3];
    int row, col, i;
    two = kFloat_2_Marker;
    for (i = 0; i < 4; i++) {
        __as__7WVectorFRC7WVector(&cur, &spot->pos);
        row = i / 2;
        cur.y += two;
        float s1 = row ? kFloat_1_spot : kFloat_neg1_U800EFEAC;
        col = i % 2;
        if (col) s2 = kFloat_1_spot;
        else s2 = kFloat_neg1_U800EFEAC;
        __ml__FfRC7WVector(&rotA, &perp, s1);
        __ml__FfRC7WVector(&rotB, &pos, s2);
        __pl__FRC7WVectorRC7WVector(&sum, &rotB, &rotA);
        __ml__FfRC7WVector(&scaled, &sum, ten);
        __apl__7WVectorFRC7WVector(&cur, &scaled);
        Vec3_CopyToOutput(&cur, &verts[i]);
        verts[i].u = row ? kFloat_0_spot : kFloat_1_spot;
        verts[i].v = col ? kFloat_0_spot : kFloat_1_spot;
    }
    DrawPrimitiveUP(kSpotTriangleStrip, 2, verts, kSpotVertexBytes);
}

struct TableLock {
    char data[8];
    // Acquire the indexed table lock for this scope.
    TableLock(int index, int active) { TableLock_ctor(this, index, active); }
    ~TableLock() throw();
};

struct InfoBaseResources {
    void* vtable;
    void* resource;
    unsigned int resourceCount;
    char reserved[8];
    void* ownedInfo;
};

// Initialize this instance, then register the shared HUD font atlases on the first construction.
void* __ct__9CInfoBaseFv(void* self) {
    *(const char**)self = cInfoBaseVtable;
    ((InfoBaseResources*)self)->ownedInfo = 0;
    TableLock lock(2, 1);
    memset(&((InfoBaseResources*)self)->resource, 0, 8);
    if (s_infoBaseFontRegistryInitialized) { return self; }
    // One registration per HUD font: (name, atlas parameters 0 and 1, resource-name flag 1, atlas
    // parameters 2 and 3, glyph order string, halfScale 0). See CInfoBase_RegisterNamedInfo.
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_holeFont, 16, 16, 1, 128, 32, golfHudFontStrings + kHud_holeGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_barFont, 15, 16, 1, 128, 32, golfHudFontStrings + kHud_barGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_windFont, 16, 16, 1, 128, 64, golfHudFontStrings + kHud_windGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_clubFont, 32, 32, 1, 128, 64, golfHudFontStrings + kHud_hexGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_largeFont, 20, 32, 1, 256, 64, golfHudFontStrings + kHud_largeGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_shopNumberFont, 13, 32, 1, 256, 32, golfHudFontStrings + kHud_numberCommaGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_statusNumberFont, 8, 16, 1, 128, 16, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_galleryNumberFont, 16, 32, 1, 256, 32, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_commonNumberFont, 16, 32, 1, 256, 32, golfHudFontStrings + kHud_numberCommaGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_scoreNumberFont, 11, 20, 1, 256, 32, golfHudFontStrings + kHud_scoreGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_customNumberFont0, 12, 32, 1, 256, 32, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_customNumberFont1, 13, 32, 1, 256, 32, golfHudFontStrings + kHud_numberCommaGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_customNumberFont2, 11, 32, 1, 128, 32, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_customNumberFont3, 10, 32, 1, 128, 32, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_staminaNumberFont, 11, 32, 1, 256, 32, golfHudFontStrings + kHud_numberCommaGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_systemNumberFont, 15, 19, 1, 256, 32, golfHudFontStrings + kHud_signedNumberGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_resultNumberFont, 19, 32, 1, 256, 32, golfHudFontStrings + kHud_signedNumberGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_countdownFont, 30, 30, 1, 256, 64, golfHudFontStrings + kHud_digitGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_statusFont0, 14, 32, 1, 256, 32, golfHudFontStrings + kHud_statusGlyphs0, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_statusFont1, 13, 32, 1, 256, 32, golfHudFontStrings + kHud_statusGlyphs1, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_tipsNumberFont, 22, 32, 1, 256, 64, golfHudFontStrings + kHud_tipsGlyphs, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_partyPointFont, 48, 32, 1, 64, 32, golfHudFontStrings + kHud_pointGlyph, 0);
    CInfoBase_RegisterNamedInfo(self, golfHudFontStrings + kHud_tipsPointFont, 48, 32, 1, 64, 32, golfHudFontStrings + kHud_pointGlyph, 0);
    s_infoBaseFontRegistryInitialized = 1;
    return self;
}

struct GolfPlayerEntry {
    char playerHeader[0x1C];
    Vec3f position;
    char pad[0x94 - 0x28];
    unsigned int color;
};

// Look up the golf player record (position and marker color) for playerIndex in the shared game document.
static inline GolfPlayerEntry* SelectedPlayerEntry(int playerIndex) {
    return (GolfPlayerEntry*)GolfPlayer_GetEntryByIndex((void*)((CSharedDoc*)g_sharedDoc)->golfDoc, (unsigned char)playerIndex);
}

// Draw a textured quad at the selected player's position using that player's marker color.
void CSpot_DrawPlayerMarker(Obj1D9C* spot, int playerIndex) {
    Vtx verts[4];
    SpotRenderStateBlock renderStates;
    SpotTextureStageParamBlock textureParameters;
    SpotTextureStageStateBlock textureStates;
    Vec3f position;
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTextureStageResource(0, g->s0(spot->playerMarkerBitmap));
    }
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTexturePalette(0, g->s1(spot->playerMarkerBitmap));
    }
    float half = kFloat_0_5_Marker;
    int rightCorner, farCorner, i;
    for (i = 0; i < 4; i++) {
        __as__7WVectorFRC7WVector(&position, &SelectedPlayerEntry(playerIndex)->position);
        rightCorner = i & 1;
        position.x += half * (rightCorner ? kFloat_1_spot : kFloat_neg1_U800EFEAC);
        farCorner = i & 2;
        position.z += half * (farCorner ? kFloat_1_spot : kFloat_neg1_U800EFEAC);
        Vec3_CopyToOutput(&position, &verts[i]);
        verts[i].c[0] = (unsigned char)(SelectedPlayerEntry(playerIndex)->color >> 16);
        verts[i].c[1] = (unsigned char)(SelectedPlayerEntry(playerIndex)->color >> 8);
        verts[i].c[2] = (unsigned char)SelectedPlayerEntry(playerIndex)->color;
        verts[i].c[3] = 0x80;
        verts[i].u = rightCorner ? kFloat_1_spot : kFloat_0_spot;
        verts[i].v = farCorner ? kFloat_0_spot : kFloat_1_spot;
    }
    renderStates = *(SpotRenderStateBlock*)spotRenderStateDefaults;
    unsigned int markerPolicy = kSpotDestinationBlendFactor;
    renderStates.w[kSpotDestinationBlendValueWord] = markerPolicy;
    textureStates = *(SpotTextureStageStateBlock*)s_markerTextureStageStates;
    textureParameters = *(SpotTextureStageParamBlock*)s_spotTextureStageParameters;
    Engine_SetRenderStateList(&renderStates, kSpotRenderStatePairs);
    Engine_SetTextureStageStateList(0, &textureStates, kSpotTextureStageStatePairs);
    Engine_SetTextureStageParameterList(0, &textureParameters, kSpotTextureStageParamPairs);
    void* transform = Camera_GetRenderState(g_view);
    SetTransform(kXformProjection, ((MarkerCameraMatrices*)transform)->projection);
    SetTransform(kXformView, ((MarkerCameraMatrices*)transform)->view);
    Engine_SetType7Resource(0);
    SetTransform(kXformIdentitySlotA, kIdentityMatrix4x4);
    SetTransform(kXformIdentitySlotB, kIdentityMatrix4x4);
    Engine_SetFlexibleVertexFormat(kSpotTexturedVertexFormat);
    DrawPrimitiveUP(kSpotTriangleStrip, 2, verts, kSpotVertexBytes);
}

// Draw the tutorial target quad, doubling its size in tutorial stage two.
void CSpot_DrawScoreTarget(void* self) {
    char* base = (char*)spotRenderStateDefaults;
    Vtx verts[4];
    SpotRenderStateBlock renderStates;
    SpotTextureStageParamBlock textureParameters;
    SpotTextureStageStateBlock textureStates;
    float offset;
    float step;
    unsigned int markerPolicy = kSpotDestinationBlendFactor;
    void* scene = (void*)((CSharedDoc*)g_sharedDoc)->golfDoc /* 0xB90 */;
    float scale;
    if (AsCGolfDoc(scene)->tutorialStage == 2) {
        scale = kFloat_2_Marker;
    } else {
        scale = kFloat_1_spot;
    }
    renderStates = *(SpotRenderStateBlock*)spotRenderStateDefaults;
    renderStates.w[kSpotDestinationBlendValueWord] = markerPolicy;
    textureStates = *(SpotTextureStageStateBlock*)(((char*)(base + 0x68)));
    textureParameters = *(SpotTextureStageParamBlock*)(((char*)(base + 0x98)));
    Engine_SetRenderStateList(&renderStates, kSpotRenderStatePairs);
    Engine_SetTextureStageStateList(0, &textureStates, kSpotTextureStageStatePairs);
    Engine_SetTextureStageParameterList(0, &textureParameters, kSpotTextureStageParamPairs);
    void* transform = Camera_GetRenderState(g_view);
    SetTransform(kXformProjection, ((MarkerCameraMatrices*)transform)->projection);
    SetTransform(kXformView, ((MarkerCameraMatrices*)transform)->view);
    Engine_SetType7Resource(0);
    SetTransform(kXformIdentitySlotA, kIdentityMatrix4x4);
    SetTransform(kXformIdentitySlotB, kIdentityMatrix4x4);
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTextureStageResource(0, g->s0(((CSpot*)self)->scoreTargetBitmap /* 0x23C */));
    }
    {
        GfxVt* g = (GfxVt*)AsWView(g_view)->renderTarget /* 0xA4 */;
        Engine_SetTexturePalette(0, g->s1(((CSpot*)self)->scoreTargetBitmap /* 0x23C */));
    }
    Engine_SetFlexibleVertexFormat(kSpotTexturedVertexFormat);
    step = kFloat_2_Marker;
    offset = kFloat_44_8_Marker;
    for (int i = 0; i < 4; ++i) {
        float point[3];
        void* golfDoc = (void*)((CSharedDoc*)g_sharedDoc)->golfDoc /* 0xB90 */;
        __as__7WVectorFRC7WVector(point, ((TutorialMarkerPosition*)golfDoc)->position);
        point[1] += step;
        float xSign = (i & 1) ? kFloat_1_spot : kFloat_neg1_U800EFEAC;
        float xScale = scale * xSign;
        point[0] += offset * xScale;
        float zSign = (i & 2) ? kFloat_neg1_U800EFEAC : kFloat_1_spot;
        float zScale = scale * zSign;
        point[2] += offset * zScale;
        Vec3_CopyToOutput(point, &verts[i]);
        verts[i].u = (i & 1) ? kFloat_1_spot : kFloat_0_spot;
        verts[i].v = (i & 2) ? kFloat_0_spot : kFloat_1_spot;
        verts[i].c[3] = 0xFF;
        verts[i].c[2] = 0xFF;
        verts[i].c[1] = 0xFF;
        verts[i].c[0] = 0xFF;
    }
    DrawPrimitiveUP(kSpotTriangleStrip, 2, verts, kSpotVertexBytes);
}

// Destroy all nine embedded marker overlays and the native base, then optionally free the spot.
void* CSpot_dtor(void* self, int del) {
    if (self) {
        __destroy_arr(((SpotOverlayStorage*)self)->markerOverlays, CBpTex_dtor, 0x28, 9);
        __dt__18NativeBase80201D18Fv(self, 0);
        if (del > 0) __dl__FPv(self);
    }
    return self;
}

// Release the HUD font resource and owned info buffer, then optionally delete the registry.
void* __dt__9CInfoBaseFv(void* self, int del) {
    if (self) {
        void** p = (void**)self;
        void* x = p[1];
        p[0] = &cInfoBaseVtable;
        if (x) { ResourceManager_DeleteResource(g_resourceManager, x); p[1] = 0; }
        if (p[5]) { __dla__FPv(p[5]); p[5] = 0; }
        if (del > 0) __dl__FPv(self);
    }
    return self;
}

// A projected vertex occupies 0x38 bytes, including draw-state words left untouched here.
struct SpotScreenVertex {
    float x, y, z, rhw;
    unsigned int color;
    unsigned int vertexFlags;
    float u, v;
    char drawState[0x18];
};
struct SpotProjectedQuad {
    char spotPrefix[0x94];
    float width, height;
    unsigned int material;
    char spotState[0x240 - 0xA0];
    char drawHeader[0x14];
    SpotScreenVertex vertices[4];
};
struct SpotProjection {
    float x, y, z, rhw;
    char projectionState[0x24];
    unsigned char clipFlags;
    char padding[3];
};
typedef char SpotScreenVertex_size[(sizeof(SpotScreenVertex) == 0x38) ? 1 : -1];
typedef char SpotProjectedQuad_offset[((int)&((SpotProjectedQuad*)0)->vertices == 0x254) ? 1 : -1];

// View virtual ABI: m6 submits a borrowed draw-header/vertex packet with format
// and packed material flags; m4(0) follows submission to restore/reset view draw
// state. The exact state reset by m4 is unresolved; keep slot names until confirmed.
struct Obj884 {
    virtual void a(); virtual void b();
    virtual void m4(int);
    virtual void c();
    virtual void m6(void*, int, unsigned int, int);
};

// Project a world point, build a screen quad, and clip its positions/UVs to the viewport.
void CSpot_DrawProjectedClippedQuad(void* obj, void* worldPosition, unsigned int color) {
    SpotProjection projected;
    WView_ProjectWorldPoint(g_view, &projected, worldPosition);
    if ((projected.clipFlags & 3) != 0) return;
    volatile SpotProjectedQuad* quad = (volatile SpotProjectedQuad*)obj;
    float rightX, topY, height, bottomY, leftX, width, one, zero, half, projectedDepth, projectedRhw;
    projectedDepth = projected.z;
    zero = kFloat_0_spot;
    quad->vertices[0].z = projectedDepth;
    one = kFloat_1_spot;
    projectedRhw = projected.rhw;
    half = kFloat_0_5_Marker;
    quad->vertices[0].rhw = projectedRhw;
    width = quad->width;
    quad->vertices[0].color = (int)color;
    height = quad->height;
    quad->vertices[1].z = projected.z;
    quad->vertices[1].rhw = projected.rhw;
    quad->vertices[1].color = (int)color;
    quad->vertices[2].z = projected.z;
    quad->vertices[2].rhw = projected.rhw;
    quad->vertices[2].color = (int)color;
    quad->vertices[3].z = projected.z;
    quad->vertices[3].rhw = projected.rhw;
    quad->vertices[3].color = (int)color;
    quad->vertices[0].u = zero;
    quad->vertices[0].v = one;
    leftX = projected.x - half * width;
    quad->vertices[0].x = leftX;
    bottomY = projected.y;
    quad->vertices[0].y = bottomY;
    quad->vertices[1].u = zero;
    quad->vertices[1].v = zero;
    quad->vertices[1].x = projected.x - half * width;
    topY = projected.y - height;
    quad->vertices[2].u = one;
    quad->vertices[2].v = one;
    quad->vertices[1].y = topY;
    rightX = projected.x + half * width;
    quad->vertices[2].x = rightX;
    quad->vertices[2].y = projected.y;
    quad->vertices[3].u = one;
    quad->vertices[3].v = zero;
    quad->vertices[3].x = projected.x + half * width;
    quad->vertices[3].y = projected.y - height;
    if (leftX > AsWView(g_view)->viewportWidth - half) return;
    if (rightX < half) return;
    if (bottomY < half) return;
    if (topY > AsWView(g_view)->viewportHeight - half) return;
    // Vertex i lives at obj+0x254+i*0x38. p is a biased byte cursor: its apparent
    // SpotProjectedQuad type preserves the +0x254 load displacement, but after the
    // first step it does NOT point to a whole quad. Only vertices[0] may be accessed
    // through p; width/height must use quad, whose address stays fixed.
    // Clamp to half-pixel viewport edges and adjust UV by clipped pixels / sprite size.
    SpotProjectedQuad* vertexCursor = (SpotProjectedQuad*)obj;
    for (int i = 0; i < 4; i++) {
        float xx = vertexCursor->vertices[0].x;
        if (xx < half) {
            float diff = half - xx;
            vertexCursor->vertices[0].u += diff / quad->width;
            vertexCursor->vertices[0].x = half;
        } else if (xx > AsWView(g_view)->viewportWidth - half) {
            float diff = xx - AsWView(g_view)->viewportWidth;
            vertexCursor->vertices[0].u -= (half + diff) / quad->width;
            vertexCursor->vertices[0].x = AsWView(g_view)->viewportWidth - half;
        }
        float yy = vertexCursor->vertices[0].y;
        if (yy < half) {
            vertexCursor->vertices[0].v += (half - yy) / quad->height;
            vertexCursor->vertices[0].y = half;
        } else if (yy > AsWView(g_view)->viewportHeight - half) {
            float diff = yy - AsWView(g_view)->viewportHeight;
            vertexCursor->vertices[0].v -= (half + diff) / quad->height;
            vertexCursor->vertices[0].y = AsWView(g_view)->viewportHeight - half;
        }
        vertexCursor = (SpotProjectedQuad*)((char*)vertexCursor + sizeof(SpotScreenVertex));
    }
    unsigned int material = ((SpotProjectedQuad*)obj)->material;
    unsigned int materialIndex = material & kSpotMaterialIndexMask;
    unsigned int drawFlags = materialIndex | kSpotProjectedDrawFlags;
    ((Obj884*)g_view)->m6(((SpotProjectedQuad*)obj)->drawHeader, 0x20, drawFlags, 0);
    ((Obj884*)g_view)->m4(0);
}


// Return the spot object's original runtime type-information address.
void* CSpot_GetRTTI() {
    return &s_spotTypeInfo;
}

void* CSpot_DrawMarkersAndLabels(void*);

// Adjust the secondary-interface pointer back to its owner before drawing the spot.
void* CSpot_vfunc_1Thunk18(void* arg) {
    return CSpot_DrawMarkersAndLabels((char*)arg - 0x18);
}

// Initialize the spot-rendering helper's two-pi cache.
void SpotRendering_InitTwoPiCache() {
    s_twoPi_SpotRendering = kFloat_6_2831855;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804620FC", _ctors_u800F1D9C_0, SpotRendering_InitTwoPiCache);
#pragma pop
