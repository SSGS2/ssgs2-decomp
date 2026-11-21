#include <decomp/ct_member.h>
// BGM Collection: resource readiness, track text/artwork, and previous/next/play/
// stop input. CBgmCollection_ctor in golf_base_2d_hud_layout.cpp creates the owned resources;
// CBgmCollection.h and the overlapping local views below preserve their ABI.
// Modding lookup (frame rate / UI timing / textures):
// - kButtonDelayUpdates=4 is the pressed-button hold in vfunc_5: four eligible
//   process calls, preincrement then equality, NOT seconds or retrace count.
//   Global fade and first-call gates pause the hold. Larger values hold longer;
//   keep its zero reset and equality test together when changing cadence.
// - kUnlockBitCount=45 bounds the saved unlock bits. Previous/next wrap across
//   table entries and stop at an unlocked entry or one full circuit. Requires
//   a nonempty table with count<=45 and a valid selected index. Keep this domain
//   synchronized with save-data bits and the BGM metadata table, not just UI.
// - Artwork geometry comes from layout element kArtworkPane (8), in layout
//   pixels with x right/y down; text positions come from Bp named panes. The
//   defaults/template are in u80318714_data.cpp and s_bgmCollectionArtworkQuadTemplate.
//   Vertex format 0x11F005 and 24-byte stride travel together; UV/color remain
//   from the template while xy is replaced. This unit sets no video resolution.
// - UpdateTrackInfo builds prefix+resourceName+suffix and requests artwork only
//   if that name changes. Texture format/dimensions come from the loaded asset,
//   not a local format knob. Sound names and title/composer keys also use table
// self is required, fully constructed and used serially. BgmTableWrapper owns
// only its wrapper; table bytes stay borrowed from sharedFile and must outlive
// it. Text/quad inputs are borrowed during draw, temporaries own their buffers.
// Release map: CBgmCollection_dtor (bgm_collection_dtor.cpp) unregisters the Bp/draw entries,
// destroys messageResource/messageDraw, table wrapper, texture binding and layout,
// then CSharedFileHandle_Destroy releases sharedFile; it stops BGM and finally
// releases texture pack and the cached name string. Do not free table bytes here.
// The local views are aliases of CBgmCollection, not separately owned objects.
// Async load requests may defer display; readiness queries preserve all resources.
namespace exception_runtime {
extern "C" {
void __throw(const char*, void*, void*) __attribute__((noreturn));
void __dt__Q23std12out_of_rangeFv(void*, int);
extern const char bgmCollectionStrings[];
}
}

// Translation unit 0x80318714-0x80319AD8 (7 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "game/wstring.h"
#include "classes/CBgmCollection.h"
#include "game/singletons.h"
#include "game/exception.h"
#include <decomp/cw_bitfields.h>
enum BgmCollectionConstants {
    // Hold duration in eligible vfunc_5 calls; do not confuse with audio time.
    kScriptIndex = 14, kButtonDelayUpdates = 4, kUnlockBitCount = 45,
    kActionPrevious = 0, kActionNext = 2, kActionPlay = 4, kActionStop = 6,
    kBackInputAction = 6, kAnyPlayer = -1,
    kPreviousBase = 0, kPreviousHighlight = 1, kNextBase = 2, kNextHighlight = 3,
    kPlayBase = 4, kPlayHighlight = 5, kStopBase = 6, kStopHighlight = 7,
    kArtworkPane = 8, kAuxiliaryPane9 = 9, kAuxiliaryPane10 = 10,
    kVertexFormatTexturedColor = 0x11F005, kTriangleStrip = 0x98,
    kQuadVertexCount = 4, kVertexBytes = 0x18, kNormalButtonState = 0, kEnabledFeedback = 1, kPressedButtonState = 2,
    kCollectionScriptName = 0x89, kTrackNamePane = 0x98, kComposerPane = 0xAA,
    kTexturePathPrefix = 0xB8, kTexturePathSuffix = 0xB9, kRangeErrorTypeName = 0x52
};

// Borrow the shared class layout without changing object ownership.
inline CBgmCollection* AsBgmCollection(void* object) { return (CBgmCollection*)object; }

extern "C" {
struct StringRep {
    union { unsigned word; unsigned char bytes[4]; struct { CW_BITS2( unsigned islong:1, unsigned capacity:31) } storageFlags; } rep;
    unsigned length;
    char* ptr;
    // Return bytes in the packed inline/heap string, excluding the terminator.
    unsigned size() const { return !(rep.word >> 31) ? rep.bytes[0]&127 : length; }
    // Borrow text bytes until the string changes or is destroyed.
    const char* data() const { return !(rep.word >> 31) ? (const char*)&rep.bytes[1] : ptr; }
    // Borrow a data/length pair using the active storage representation.
    void range(const char*& p, unsigned& n) const {
        if (!(rep.word >> 31)) { p=(const char*)&rep.bytes[1]; n=rep.bytes[0]&127; }
        else { p=ptr; n=length; }
    }
};
struct BgmCollectionView {
    char pad0[0x18];
    void* bpEntryId;
    void* layout;
    void* trackTable;
    void* messageResource; // +0x24 owned lookup provider; internal representation is opaque.
    char pad28[4];
    void* messageDraw; // +0x2C owned CMsgDraw; shared class layout uses this name too.
    int selectedTrackIndex;
    struct ArtworkDrawInterface* textureState;
    char pad38[0x10];
    StringRep current;
    char pad54[0x82c];
    void* sharedFile;
    void* textureResource;
};
void Layout_Update(void*);
void CPackTextureResource_PollLoad(void*);
unsigned CPackTextureResource_IsLoadFinished(void*);
void __dl__FPv(void*);
void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
void* IdMap_Lookup(void*, void*);
unsigned BpObject_PollLoad(void*);
unsigned Layout_PollLoadComplete(void*);
void* CSharedFilePtr_GetLoadedBuffer(void*);
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_80316848[];

// Texture binding interface: +0C loads a path (ArtworkLoadInterface below), +14
// prepares drawing, +18 tests drawable readiness. Other slots unresolved.
struct ArtworkDrawInterface {
    virtual void v8(); virtual void vc(); virtual void v10(); virtual void v14(); virtual int v18();
};
void UIRender_SetDefaultState(BgmCollectionView*);
void BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void Layout_DrawWithOpacity(void*, float);
void Layout_GetElementPosition(void*, int, void*);
void Layout_GetElementSize(void*, int, void*);
void Engine_SetFlexibleVertexFormat(int);
int GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, int, void*, int);
void Text_DrawGlyphs(void*);
void* __nw__FUl(unsigned long);
void CBpCtrlHandle_Dtor(void*);
void CBpCtrlHandle_SetScriptIndex(void*, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, int);
void Layout_ReadElementRect(void*, int, void*);
void Layout_WriteElementRect(void*, int, void*);
void BpObject_StartScriptByName(void*, const char*, int);
int CBgmCollection_UpdateTrackInfo(BgmCollectionView*, int);
void IFadeMenuBase_StartMenuFadeOut(BgmCollectionView*);
extern signed char s_bpCtrlHandleInitialized;
// Shared two-word ABI storage: controller handles here, pane X/Y below.
// These uses do not imply that the global handle contains coordinates.
struct TwoWordAbiStorage { int a,b; };
extern TwoWordAbiStorage s_bpCtrlHandle;
extern char s_bpCtrlHandleDtorRegistration_BgmCollection[];
extern char __vt__20CBgm_collection_data[];
extern char bgmCollectionStrings[];
struct Vertex { float x,y; char rest[16]; };
struct Quad { Vertex v[4]; };
extern Quad s_bgmCollectionArtworkQuadTemplate;
extern const float kBgmCollectionLayoutOpacity;
// Borrow the current BP controller, constructing static storage on first use.
inline void* Manager(void*& instance) {
    if (!instance) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        instance = s_bpCtrl;
    }
    return instance;
}
struct BgmTableWrapper {
    void* value;
    void* table;
    // Wrap borrowed loaded-table bytes; deleting the wrapper does not free them.
    BgmTableWrapper(void* v) : value(0) { if (v) value = v; table = __vt__20CBgm_collection_data; }
};
// Initialize collection panes and track data, start the collection script and display the selected track.
// Called once after readiness: installs script index 14 globally, creates the
// owned table wrapper over borrowed loaded bytes, and starts the menu fade.
// Copies hit rectangles to the paired highlight elements before input starts.
// Required allocation/resources are not checked here; no partial-success status.
void CBgmCollection_StartMenu(BgmCollectionView* self) {
    void* data = __nw__FUl(16);
    if (!Sing_805DC5F8()) {
        if (!s_bpCtrlHandleInitialized) {
            s_bpCtrlHandle.a = -1;
            s_bpCtrlHandle.b = -1;
            __register_global_object(&s_bpCtrlHandle, CBpCtrlHandle_Dtor, s_bpCtrlHandleDtorRegistration_BgmCollection);
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = &s_bpCtrlHandle;
    }
    CBpCtrlHandle_SetScriptIndex(Sing_805DC5F8(), kScriptIndex);
    Layout_SetElementPlayerFlagsAndState(AsBgmCollection(self)->layout, kArtworkPane, 0);
    Layout_SetElementPlayerFlagsAndState(AsBgmCollection(self)->layout, kAuxiliaryPane9, 0);
    Layout_SetElementPlayerFlagsAndState(AsBgmCollection(self)->layout, kAuxiliaryPane10, 0);
    Layout_ReadElementRect(AsBgmCollection(self)->layout, kPreviousBase, data);
    Layout_WriteElementRect(AsBgmCollection(self)->layout, kPreviousHighlight, data);
    Layout_ReadElementRect(AsBgmCollection(self)->layout, kNextBase, data);
    Layout_WriteElementRect(AsBgmCollection(self)->layout, kNextHighlight, data);
    Layout_ReadElementRect(AsBgmCollection(self)->layout, kPlayBase, data);
    Layout_WriteElementRect(AsBgmCollection(self)->layout, kPlayHighlight, data);
    Layout_ReadElementRect(AsBgmCollection(self)->layout, kStopBase, data);
    Layout_WriteElementRect(AsBgmCollection(self)->layout, kStopHighlight, data);
    void* value = CSharedFilePtr_GetLoadedBuffer(AsBgmCollection(self)->sharedFile);
    BgmTableWrapper* h = new BgmTableWrapper(value);
    void* manager = Sing_805DC5F0();
    self->trackTable = h;
    if (!manager) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(), self->bpEntryId), bgmCollectionStrings + kCollectionScriptName, 1);
    CBgmCollection_UpdateTrackInfo(self, AsBgmCollection(self)->selectedTrackIndex);
    if (data) __dl__FPv(data);
    IFadeMenuBase_StartMenuFadeOut(self);
}

#pragma optimize_for_size on
// Draw the collection script, layout, optional track artwork and text.
// Artwork interface +0x18 returns drawable readiness and +0x14 prepares binding;
// a pending texture skips only the quad. Draw uses borrowed stack geometry and
// changes renderer state; no texture data is allocated or captured here.
void CBgmCollection_Draw(BgmCollectionView* self) {
    UIRender_SetDefaultState(self);
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Manager(Sing_805DC5F0()), self->bpEntryId), 0);
    BpObject_Draw(IdMap_Lookup(Manager(Sing_805DC5F0()), self->bpEntryId));
    Layout_DrawWithOpacity(AsBgmCollection(self)->layout, kBgmCollectionLayoutOpacity);
    if (self->textureState->v18()) {
        self->textureState->v14();
        float pos[2];
        float size[2];
        Layout_GetElementPosition(AsBgmCollection(self)->layout, kArtworkPane, pos);
        Layout_GetElementSize(AsBgmCollection(self)->layout, kArtworkPane, size);
        Quad q = s_bgmCollectionArtworkQuadTemplate;
        q.v[0].x = pos[0];
        q.v[0].y = pos[1];
        q.v[1].x = pos[0]+size[0];
        q.v[1].y = pos[1];
        q.v[2].x = pos[0];
        q.v[2].y = pos[1]+size[1];
        q.v[3].x = pos[0]+size[0];
        q.v[3].y = pos[1]+size[1];
        Engine_SetFlexibleVertexFormat(kVertexFormatTexturedColor);
        int count = GX_GetPrimitiveCount(kQuadVertexCount, kTriangleStrip);
        DrawPrimitiveUP(kTriangleStrip, count, &q, kVertexBytes);
    }
    Text_DrawGlyphs(self->messageDraw);
}

#pragma optimize_for_size off
// Update the layout and poll the artwork resource load.
// Load-pump hook only, with no completion status; vfunc_2 supplies the readiness flag.
void CBgmCollection_UpdateLayoutAndTexture(BgmCollectionView* self) {
    Layout_Update(AsBgmCollection(self)->layout);
    CPackTextureResource_PollLoad(AsBgmCollection(self)->textureResource);
}

// Optionally delete the track-table wrapper; its referenced table is not freed here.
void* CBgm_collection_data_dtor(void* self, int flag) {
    if (self && flag > 0) __dl__FPv(self);
    return self;
}

// Poll the artwork, script, layout and shared table until all collection resources are ready.
// Returns their combined readiness, not an error code. All three polls execute
// before testing the borrowed shared-file buffer; no ownership changes.
unsigned CBgmCollection_AreResourcesLoaded(BgmCollectionView* self) {
    unsigned result = CPackTextureResource_IsLoadFinished(AsBgmCollection(self)->textureResource) & 1;
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    result &= BpObject_PollLoad(IdMap_Lookup(Sing_805DC5F0(), self->bpEntryId));
    result &= Layout_PollLoadComplete(AsBgmCollection(self)->layout);
    if (!result || !CSharedFilePtr_GetLoadedBuffer(AsBgmCollection(self)->sharedFile)) result = 0;
    return result;
}

struct ArtworkLoadInterface { 
virtual void v8(); virtual void vc(const char*);
};
void appFreeUni__FPv(void*);
void String_ReplaceRange(void*, unsigned, unsigned, const char*, const char*, void*);
unsigned strlen(const char*);
int memcmp(const char*, const char*, unsigned);
void BpTextureBinding_ReleaseSources(void*);
struct Alloc { char x; };
}
namespace std {
class LocalNString_u80318714 : public StringRep {
public:
    // Initialize an empty inline string.
    LocalNString_u80318714() { rep.word=0; length=0; ptr=0; }
    // Free only the string-owned heap buffer.
    ~LocalNString_u80318714() { if(rep.word >> 31) appFreeUni__FPv(ptr); }
    // Replace a byte range through the engine allocator; copy the supplied bytes.
    void replace(unsigned n, const char* p, const char* e, Alloc a) { String_ReplaceRange(this,0,n,p,e,&a); }
    // Erase all characters through the engine replacement API.
    void clear() { Alloc a; replace(-1,0,0,a); }
    // Replace the first n bytes with a copy of NUL-terminated input.
    void assignChars(const char* p, unsigned n, Alloc a) { unsigned len=strlen(p); String_ReplaceRange(this,0,n,p,p+len,&a); }
    // Copy a borrowed C string, replacing this string's existing contents.
    void assign(const char* p) { unsigned n=size(); Alloc a; assignChars(p,n,a); }
    // Copy inline words directly; deep-copy heap storage through replace.
    void assign(const StringRep& p) {
        unsigned flag=rep.word>>31;
        if(!flag) { unsigned source=p.rep.word; if (!(source>>31)) {
            rep.word=source;
            length=p.length;
            ptr=p.ptr;
            return;
        } } {
            unsigned n=flag ? length : rep.bytes[0]&127;
            const char* text;
            unsigned len;
            p.range(text,len);
            Alloc a;
            replace(n,text,text+len,a);
        }
    }
};
}
typedef std::LocalNString_u80318714 String;
extern "C" {
String String_ConcatCStringAndString(const char*, const String&);
String String_ConstructConcatCStr(const String&, const char*);

// Table header view: +8 is the entry count; the holder borrows its table.
struct BgmEntryTableView { char pad[8]; int count; };
struct BgmTableHandle { BgmEntryTableView* list; };
void Table_GetEntry(void*, int);
void* Table_ResolveDataOffset(void*, int);
const char* OffsetStringNode_GetText(void*);
int BgmCollectionTable_GetMetadata(void*, int);
int OffsetTable_GetEntryWord10(void*, int);
int BgmCollectionTable_GetResource(void*, int);
void CMsgDraw_ResetTextState(void*);
void CMsgDraw_ClearFormattingAndRevealAll(void*);
void* BpObject_GetOrCreateNamedGroup(void*, const char*);
void UIGroup_GetPosition(void*, void*);
void Text_SetPosition(void*, void*);
unsigned UIGroup_GetTintColor(void*);
void CMsgDraw_SetColor(void*, unsigned);
void* TextResource_FindNamedEntry(void*, const char*);
void Text_SetContent(void*, void*);
extern int kBgmCollectionInitialTextX;
extern int kBgmCollectionInitialTextY;
// Convert packed pane RGB byte order to renderer color with opaque alpha.
inline unsigned SwapColor(unsigned c) { return ((c<<24)&0xff000000)|((c<<8)&0xff0000)|((c>>8)&0xff00)|255; }
// Resolve a table-relative text offset; return borrowed text or null.
inline const char* GetText(BgmTableHandle* h,int n,int (*lookup)(void*,int)) {
    void* p=Table_ResolveDataOffset(h->list,lookup(h,n));
    return p ? OffsetStringNode_GetText(p) : 0;
}
// Borrow the lesser unsigned value without extending its lifetime.
inline const unsigned& Min(const unsigned& a,const unsigned& b) { return a<b?a:b; }
// Compare a bounded string prefix with a borrowed byte range, lexicographically.
inline int CompareRange(const StringRep& s, unsigned len, const char* p, const unsigned n) {
    unsigned count;
    const char* text;
    unsigned raw;
    if(!(s.rep.word>>31)) { text=(const char*)&s.rep.bytes[1]; raw=s.rep.bytes[0]&127; }
    else { text=s.ptr; raw=s.length; }
    const unsigned sz=raw;
    count=Min(len,sz);
    int result=memcmp(text,p,Min(n,count));
    if(!result) { if(count<n) result=-1; else if(count==n) return 0; else return 1; }
    return result;
}
// Compare the full packed string with a NUL-terminated borrowed string.
inline int Compare(const StringRep& s, const char* p) { const unsigned n=strlen(p); return CompareRange(s,s.size(),p,n); }
#pragma push
#pragma use_lmw_stmw on
// Update title/composer panes and replace artwork only when its name changes.
// Caller supplies a nonnull table and 0<=trackIndex<count. The signed legacy
// guard returns 0 only for index>count (null list substitutes count=0).
// index==count reaches Table_GetEntry's unchecked table+0x20+index*stride,
// which addresses one past the entries; negative indices address before entry 0.
// A null list with index<=0 also reaches that unchecked lookup. None of those
// invalid cases has a defined fallback, and the guard does not make them safe.
// Rejected index>count may initialize the BP singleton but leaves track text and
// artwork unchanged. Valid input returns 1 after updates; text/resources
// are borrowed, while self->current retains its own resource-name copy.
// Table text/resource keys must resolve to NUL-terminated strings. Named text
// panes must exist; no missing-key fallback is supplied. Releases old artwork
// sources BEFORE the new load request; failure can leave artwork pending/absent.
// The path temporary dies on return, so the binding's load interface must copy
// or consume it during the call; callers cannot retain that stack string.
int CBgmCollection_UpdateTrackInfo(BgmCollectionView* self, int trackIndex) {
    // Phase 1: resolve the BP context and selected track.
    void* manager = Sing_805DC5F0();
    if (!manager) {
        if (!s_bpCtrlInitialized) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80316848);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    void* scriptContext=IdMap_Lookup(Sing_805DC5F0(),self->bpEntryId);
    BgmTableHandle* tracks=(BgmTableHandle*)self->trackTable;
    if(trackIndex>(tracks->list?tracks->list->count:0)) return 0;
    Table_GetEntry(*(BgmEntryTableView* volatile*)tracks,trackIndex);
    // Phase 2: reset text, then place/color the title from its layout pane.
    // TwoWordAbiStorage.a/b carry the two position words here; keep them as integer bits.
    TwoWordAbiStorage textPosition;
    textPosition.a=kBgmCollectionInitialTextX;
    textPosition.b=kBgmCollectionInitialTextY;
    WString textScratch;
    CMsgDraw_ResetTextState(self->messageDraw);
    CMsgDraw_ClearFormattingAndRevealAll(self->messageDraw);
    void* textPane=BpObject_GetOrCreateNamedGroup(scriptContext,bgmCollectionStrings + kTrackNamePane);
    UIGroup_GetPosition(textPane,&textPosition);
    Text_SetPosition(self->messageDraw,&textPosition);
    unsigned color=UIGroup_GetTintColor(textPane);
    unsigned rendererColor=((color<<24)&0xff000000)|((color<<8)&0xff0000);
    rendererColor |= (unsigned short)(((color>>8)&0xff00)|255);
    CMsgDraw_SetColor(self->messageDraw,rendererColor);
    Text_SetContent(self->messageDraw,TextResource_FindNamedEntry(self->messageResource,GetText((BgmTableHandle*)self->trackTable,trackIndex,BgmCollectionTable_GetMetadata)));
    // Phase 3: apply the composer pane with the same packed RGB conversion.
    textPane=BpObject_GetOrCreateNamedGroup(scriptContext,bgmCollectionStrings + kComposerPane);
    UIGroup_GetPosition(textPane,&textPosition);
    Text_SetPosition(self->messageDraw,&textPosition);
    color=UIGroup_GetTintColor(textPane);
    rendererColor=((color<<24)&0xff000000)|((color<<8)&0xff0000);
    rendererColor |= (unsigned short)(((color>>8)&0xff00)|255);
    CMsgDraw_SetColor(self->messageDraw,rendererColor);
    // The shared accessor reads the word at entry+0x10; this caller resolves
    // that word as a table-relative offset to composer text.
    Text_SetContent(self->messageDraw,TextResource_FindNamedEntry(self->messageResource,GetText((BgmTableHandle*)self->trackTable,trackIndex,OffsetTable_GetEntryWord10)));
    // Phase 4: compare the cached name, rebuild prefix+name+suffix, then
    // release old texture sources before requesting the new path.
    String artworkPath;
    String resourceNameCopy;
    if(Compare(self->current,GetText((BgmTableHandle*)self->trackTable,trackIndex,BgmCollectionTable_GetResource))) {
        ((String&)self->current).clear();
        const char* resourceName=GetText((BgmTableHandle*)self->trackTable,trackIndex,BgmCollectionTable_GetResource);
        ((String&)self->current).assign(resourceName);
        resourceName=GetText((BgmTableHandle*)self->trackTable,trackIndex,BgmCollectionTable_GetResource);
        resourceNameCopy.assign(resourceName);
        {
            String first=String_ConcatCStringAndString(bgmCollectionStrings + kTexturePathPrefix,resourceNameCopy);
            String second=String_ConstructConcatCStr(first,bgmCollectionStrings + kTexturePathSuffix);
            artworkPath.assign(second);
        }
        BpTextureBinding_ReleaseSources(self->textureState);

        ((ArtworkLoadInterface*)self->textureState)->vc(artworkPath.data());
    }
    artworkPath.clear();
    resourceNameCopy.clear();
    return 1;
}
#pragma pop
struct BgmCollectionInputView {
    char pad0[0x1c];
    void* layout;
    BgmTableHandle* items;
    char pad24[8];
    void* text; // +0x2C, same owned renderer as BgmCollectionView::messageDraw.
    int selectedTrackIndex;
    void* other;
    int finished, buttonDelayActive, buttonDelayUpdateCount, inputInitialized;
    char pad48[12];
    int exitRequested;
    char pad58[0x1d0];
    unsigned unlockedTracks[2];
};
struct Activated {
    unsigned* data;
    unsigned count, capacity;
    // Start an empty owning vector of activated element IDs.
    Activated() : data(0), count(0), capacity(0) {}
    // Remove the known trivial elements from the constructed count.
    void erase(const unsigned& n) { count -= n; }
    // Clear the count and release the vector-owned ID allocation.
    ~Activated() { if(data) { const unsigned n=count; erase(n); appFreeUni__FPv(data); } }
};
extern signed char s_fadeOverlayInitialized;
extern char s_fadeOverlay[], s_fadeOverlayDtorRegistration_BgmCollection[];
void FadeOverlay_Init(void*);
void ResourceState_Dtor(void*);
int FadeState_GetState(void*);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[], s_inputManagerDtorRegistration_BgmCollection[];
void InputManager_Init(void*);
void InputManager_Dtor(void*);
int Input_IsActionPressed(void*, int, int);
void Layout_SetElementValueState(void*, int, int);
void Layout_SetElementFeedbackState(void*, int, int);
int Layout_CollectActivated(void*, Activated*, Activated*);
void CMsgDraw_UpdateReveal(void*);
extern void* g_soundManager;
int SoundManager_HasActiveMusicSlot(void*);
void SoundManager_RequestBgmChange(void*,const char*,int,int);
void SoundManager_RequestBgmStop(void*,int);
int BgmCollectionTable_GetSoundName(void*,int);
extern char s_bgmCollectionUnlockIndexRangeError[], __vt__Q23std11logic_error[], s_sharedCharDeleterVtable[], __vt__Q23std12out_of_range[];
void* __nwa__FUl(unsigned);
void __dla__FPv(void*);
void __dt__Q33std6detail16char_array_guardFv(void*,int);
void __throw(const char*,void*,void (*)(void*)) __attribute__((noreturn));
char* strcpy(char*, const char*);
struct ErrorAlloc {
    char value; char pad[3];
    // Initialize the exception-message allocator token; padding is ABI storage.
    ErrorAlloc() : value(0) {}
};
struct BufferGuard_u80318714 {
    char* data;
    void* allocator;
    // Own an exception-text allocation until its control block adopts it.
    BufferGuard_u80318714(char* p,void* a) : data(p),allocator(a) {}
    // Transfer the allocation out and disarm exceptional cleanup.
    char* release() { char* p=data; data=0; return p; }
    // Release unadopted exception text on scope exit.
    ~BufferGuard_u80318714() { if(data) __dla__FPv(data); }
};
struct ExceptionCount {
    void* table;
    int a,b;
    char* data;
    // Initialize strong/weak message ownership counts and adopt the character array.
    ExceptionCount(char* p) : a(1),b(1) { table=s_sharedCharDeleterVtable; data=p; }
};
struct ErrorBase_u80318714 : std::exception {
    // Install the exception type's vtable in the ABI header.
    void SetTable(const void* vt) { *(const void**)this = vt; }
char* data;
ExceptionCount* owner;
    // Copy error text and transfer its guarded storage to shared ownership.
    ErrorBase_u80318714(const char* text) {
        SetTable(__vt__Q23std11logic_error);
        ErrorAlloc allocator;
        unsigned len=strlen(text)+1;
        data=(char*)__nwa__FUl((len<<1)-len);
        {
            char* p=data;
            BufferGuard_u80318714 guard(p,&allocator);
            owner=new ExceptionCount(p);
            guard.release();
        }
        strcpy(data,text);
    }
};
namespace std {
struct out_of_range : ErrorBase_u80318714 {
    // Construct the range-error message and install the derived vtable.
    out_of_range(const char* p) : ErrorBase_u80318714(p) { SetTable(__vt__Q23std12out_of_range); }
    ~out_of_range();
    DECOMP_CT_MEMBER(out_of_range)
};
}
struct BitRef {
    const unsigned* volatile unlockedTracks;
    unsigned index;
    // Borrow the unlock-bit array and remember its zero-based track index.
    BitRef(const unsigned* p,unsigned n) : unlockedTracks(p),index(n) {}
    // Return this track's unlock bit; throw if index exceeds the 45-bit domain.
    unsigned test() const {
        unsigned n=index;
        if(n>=kUnlockBitCount) {
            {
                union { double alignment; unsigned char bytes[sizeof(std::out_of_range)]; } storage;
                std::out_of_range* error = (std::out_of_range*)storage.bytes;
                error->__ct(s_bgmCollectionUnlockIndexRangeError);
                exception_runtime::__throw(exception_runtime::bgmCollectionStrings + kRangeErrorTypeName, error,
                    (void*)exception_runtime::__dt__Q23std12out_of_rangeFv);
            }
        }
        return unlockedTracks[n/32] & (1u<<(n%32));
    }
};
#pragma push
#pragma use_lmw_stmw on
// Process previous, next, play, stop and exit actions while honoring fades, unlock bits and button delay.
// Returns 1 only after a previously requested exit is published to finished;
// otherwise 0, including a Back press (sets exitRequested for a later call).
// Phase gates: global fade -> one-call initialization -> four-call hold -> exit.
// Gated calls skip layout/text updates. Accepted actions set the hold, but the
// current collected batch still runs in full; multiple IDs can act in one call.
// Scanning previous/next mutates selectedTrackIndex before checking unlock;
// BitRef throws out_of_range at >=45 (also negative cast to unsigned). Actions/
// values own only collected arrays and release them on return or unwinding.
// Preserve sound requests (including replace-active-slot policy), feedback,
// wrap termination, and the deliberately separate volatile unlock snapshots.
int CBgmCollection_UpdateAndHandleInput(BgmCollectionInputView* self) {
    // Phase 1: gate against global fade, initial input and pressed-button hold.
    Activated actions;
    Activated values;
    if(!Sing_805DC608()) {
        if(!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay,ResourceState_Dtor,s_fadeOverlayDtorRegistration_BgmCollection);
            s_fadeOverlayInitialized=1;
        }
        Sing_805DC608()=s_fadeOverlay;
    }
    if(FadeState_GetState(Sing_805DC608())) return 0;
    if(!self->inputInitialized) { self->inputInitialized=1; return 0; }
    if(self->buttonDelayActive) {
        if(++self->buttonDelayUpdateCount==kButtonDelayUpdates) {
            self->buttonDelayActive=0;
            Layout_SetElementValueState(self->layout,kPlayBase,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kPlayHighlight,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kStopHighlight,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kStopBase,kNormalButtonState);
            Layout_SetElementFeedbackState(self->layout,kPlayBase,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kPlayHighlight,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kStopHighlight,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kStopBase,kEnabledFeedback);
            Layout_SetElementValueState(self->layout,kPreviousBase,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kPreviousHighlight,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kNextBase,kNormalButtonState);
            Layout_SetElementValueState(self->layout,kNextHighlight,kNormalButtonState);
            Layout_SetElementFeedbackState(self->layout,kPreviousBase,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kPreviousHighlight,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kNextBase,kEnabledFeedback);
            Layout_SetElementFeedbackState(self->layout,kNextHighlight,kEnabledFeedback);
            self->buttonDelayUpdateCount=0;
        }
        return 0;
    }
    // Phase 2: finish a deferred exit or collect this update's input batch.
    if(self->exitRequested) { self->finished=1; return 1; }
    if(!Sing_805DC5C8()) {
        if(!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager,InputManager_Dtor,s_inputManagerDtorRegistration_BgmCollection);
            s_inputManagerInitialized=1;
        }
        Sing_805DC5C8()=s_gameInputManager;
    }
    if(Input_IsActionPressed(Sing_805DC5C8(),kBackInputAction,kAnyPlayer)) self->exitRequested=1;
    if(Layout_CollectActivated(self->layout,&actions,&values)) { if (0 == self->buttonDelayActive) { volatile unsigned offset=0; volatile unsigned i=0;

        for(;actions.count>i;++i,offset+=4) {
            switch(*(unsigned*)((char*)actions.data+offset)) {
            case kActionNext: {
                self->buttonDelayActive=1;
                Layout_SetElementValueState(self->layout,kNextBase,kPressedButtonState);
                Layout_SetElementValueState(self->layout,kNextHighlight,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kNextBase,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kNextHighlight,kPressedButtonState);
                int old=self->selectedTrackIndex;
                do {
                    self->selectedTrackIndex = 1 + self->selectedTrackIndex;
                    BgmEntryTableView* list=self->items->list;
                    if(self->selectedTrackIndex >= (list?list->count:0)) self->selectedTrackIndex=0;
                    volatile BitRef ref(self->unlockedTracks,self->selectedTrackIndex);
                    BitRef copy(self->unlockedTracks,self->selectedTrackIndex);
                    if(copy.test()) { CBgmCollection_UpdateTrackInfo((BgmCollectionView*)self,self->selectedTrackIndex); break; }
                    if(old==self->selectedTrackIndex) break;
                } while(true);
                break;
            }
            case kActionPrevious: {
                self->buttonDelayActive=1;
                Layout_SetElementValueState(self->layout,kPreviousBase,kPressedButtonState);
                Layout_SetElementValueState(self->layout,kPreviousHighlight,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kPreviousBase,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kPreviousHighlight,kPressedButtonState);
                int old=self->selectedTrackIndex;
                do {
                    if(--self->selectedTrackIndex<0) { BgmEntryTableView* list=self->items->list; self->selectedTrackIndex=(list?list->count:0)-1; }
                    BitRef copy(self->unlockedTracks,self->selectedTrackIndex);
                    volatile BitRef ref(self->unlockedTracks,self->selectedTrackIndex);
                    if(copy.test()) { CBgmCollection_UpdateTrackInfo((BgmCollectionView*)self,self->selectedTrackIndex); break; }
                    if(old==self->selectedTrackIndex) break;
                } while(true);
                break;
            }
            case kActionPlay:
                self->buttonDelayActive=1;
                if(!SoundManager_HasActiveMusicSlot(g_soundManager))
                    SoundManager_RequestBgmChange(g_soundManager,GetText(self->items,self->selectedTrackIndex,BgmCollectionTable_GetSoundName),0,0);
                else
                    SoundManager_RequestBgmChange(g_soundManager,GetText(self->items,self->selectedTrackIndex,BgmCollectionTable_GetSoundName),0,1);
                Layout_SetElementValueState(self->layout,kPlayBase,kPressedButtonState);
                Layout_SetElementValueState(self->layout,kPlayHighlight,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kPlayBase,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kPlayHighlight,kPressedButtonState);
                break;
            case kActionStop:
                self->buttonDelayActive=1;
                SoundManager_RequestBgmStop(g_soundManager,1);
                Layout_SetElementValueState(self->layout,kStopBase,kPressedButtonState);
                Layout_SetElementValueState(self->layout,kStopHighlight,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kStopBase,kPressedButtonState);
                Layout_SetElementFeedbackState(self->layout,kStopHighlight,kPressedButtonState);
                break;
            }
        }
    } }
    // Phase 3: publish layout/text progress only on the ungated path.
    Layout_Update(self->layout);
    CMsgDraw_UpdateReveal(self->text);
    return 0;
}
#pragma pop
} // extern "C"

