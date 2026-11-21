#include <decomp/offset_of.h>
// Translation unit 0x802F9B40-0x802FA3EC (3 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

#include "game/singletons.h"
#include "classes/Object802F9B40.h"
// Two ABI/alias views of the SAME tour-area selector. F9B40A is an opaque
// receiver for calls that use AsObject802F9B40; F9B40B exposes the display
// fields at +4/+8 (BP ids), +0x10 (id count), +0x18/+0x1C (layouts),
// +0x28 (borrowed area records). Neither view owns a second object.
struct F9B40A { char pad[0x2c]; };
struct F9B40B { char pad0[4]; void* primaryBpId; void* secondaryBpId; char pad1[4]; int bpEntryCount; char pad2[4]; void* primaryLayout; void* secondaryLayout; char pad3[8]; unsigned char* areaRecords; };
// Lookup tree owns trivial nodes: children at +0/+4; no separately owned
// payload is destroyed here. Free children before parents; root dies last.
struct TourAreaTreeNode { void* left; void* right; };
struct BpEntrySlot { char prefix[4]; void* id; };
// Keep the byte-cursor expression: forming a shared subobject pointer changes register allocation.
enum { kAreaTreeOffset = (int)DECOMP_OFFSETOF(Object802F9B40, pad_028[4]) };
enum TourAreaLimits { kTourAreaCount = 8, kSubgroupCount = 3, kAreaRecordSize = 0x4A, kAreaEnabledShift = 6, kFadeFrames = 8 };
// UI map: primary elements 0..7 correspond to tourAreaPointGroupNames[0..7]
// and records 0..7. Record byte 0 bit 6 controls both group and layout state.
// Secondary elements 0..2 are reset; hiddenGroup is always disabled here.
// Other record bytes and the identities of the secondary controls are unresolved.
struct TourAreaStringPool {
    char resourcePaths[0x32];
    char primaryLayoutPath[0x13];
    char secondaryLayoutPath[0x16];
    char bgmName[0xB];
    char primaryDisplayScript[8];
    char secondaryDisplayScript[0xB];
    char hiddenGroup[0x148 - 0x79];
};

extern "C" {

extern signed char s_bpCtrlInitialized;
extern void* g_soundManager;
extern signed char s_fadeOverlayInitialized;
extern void* s_tourAreaBpEntryNames[];
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_80522860[];
extern char s_fadeOverlayDtorRegistration_TourArea[]; // Tour-area fade-overlay destructor-registration record.
extern char s_fadeOverlay[];
extern void* tourAreaPointGroupNames[];
extern TourAreaStringPool tourAreaSelectStrings;
void CPackTextureResource_PollLoad(void*); int CPackTextureResource_IsLoadFinished(void*);
void CPackedFile_PollLoad(void*); int CPackedFile_IsLoadFinished(void*);
void __ct__7CBpCtrlFv(void*); void CBpCtrl_dtor(void);
void __register_global_object(void*, void*, void*);
void CBpCtrl_CreateEntry(void*, void*, void*, int);
void Layout_LoadExtended(void*, int, const char*);
void SoundManager_RequestBgmChange(void*, const char*, int, int);
void* IdMap_Lookup(void*, void*); void BpObject_StartScriptByName(void*, const char*, int);
void* BpObject_GetOrCreateNamedGroup(void*, void*); void UIGroup_SetEnabled(void*, int);
void Layout_SetElementPlayerFlagsAndState(void*, int, bool); void BpObject_TryInvokeCurrentCallback(void*, int);
void FadeOverlay_Init(void*); void ResourceState_Dtor(void);
void FadeState_StartFadeOut(void*, int);
void CBpCtrl_DeleteEntryById(void*, int);
void Layout_Destroy(void*, int);
void RBTree_FreeTrivialSubtree(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);

class X1 { public: 
virtual void destroy(int);
};
// Delete BP entries and layouts, release resources, then free the area lookup tree.
int TourAreaSelect_Destroy(F9B40A* p, int deleting) {
    if (p) {
        // Phase 1: unregister each valid BP id (-1 means absent), then release layouts.
        int i = 0;
        char* slot = (char*)p;
        while (i < AsObject802F9B40(p)->bpEntryCount) {
            int* sl = (int*)&((BpEntrySlot*)slot)->id;
            if (*sl != -1) {
                if (!Sing_805DC5F0()) {
                    if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl,CBpCtrl_dtor,s_bpCtrlDtorRegistration_80522860); s_bpCtrlInitialized=1; }
                    Sing_805DC5F0()=s_bpCtrl;
                }
                CBpCtrl_DeleteEntryById(Sing_805DC5F0(),*sl); *sl=-1;
            }
            slot += 4;
            i++;
        }
        if (AsObject802F9B40(p)->secondaryLayout) { Layout_Destroy(AsObject802F9B40(p)->secondaryLayout,1); AsObject802F9B40(p)->secondaryLayout=0; }
        if (AsObject802F9B40(p)->primaryLayout) { Layout_Destroy(AsObject802F9B40(p)->primaryLayout,1); AsObject802F9B40(p)->primaryLayout=0; }
        // Phase 2: virtual deleting destructors release the two owned resource objects.
        if (AsObject802F9B40(p)->packedFile) {
            if (AsObject802F9B40(p)->packedFile) {
                ((X1*)*(volatile void**)&AsObject802F9B40(p)->packedFile)->destroy(1);
            }
            AsObject802F9B40(p)->packedFile=0;
        }
        if (AsObject802F9B40(p)->textureResource) {
            if (AsObject802F9B40(p)->textureResource) {
                ((X1*)*(volatile void**)&AsObject802F9B40(p)->textureResource)->destroy(1);
            }
            AsObject802F9B40(p)->textureResource=0;
        }
        // Phase 3: destroy the embedded tree. The six address guards are
        // compiler-matching subobject destructor continuations, NOT six trees.
        // The next three levels are an unrolled postorder walk; the generic
        // helper handles deeper branches. Each appFreeUni frees exactly one node.
        if ((char* volatile)p + kAreaTreeOffset) {
          if ((char* volatile)p + kAreaTreeOffset) {
            if ((char* volatile)p + kAreaTreeOffset) {
              if ((char* volatile)p + kAreaTreeOffset) {
                if ((char* volatile)p + kAreaTreeOffset) {
                  if ((char* volatile)p + kAreaTreeOffset) {
            void* root = AsObject802F9B40(p)->treeRoot;
            if (root) {
                void* g0 = ((TourAreaTreeNode*)root)->left;
                if (g0) {
                    void* n0 = ((TourAreaTreeNode*)g0)->left;
                    if (n0) {
                        void* l0 = ((TourAreaTreeNode*)n0)->left;
                        if (l0) {
                            void* it = ((TourAreaTreeNode*)l0)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l0)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l0);
                        }
                        void* l1 = ((TourAreaTreeNode*)n0)->right;
                        if (l1) {
                            void* it = ((TourAreaTreeNode*)l1)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l1)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l1);
                        }
                        appFreeUni__FPv(n0);
                    }
                    void* n1 = ((TourAreaTreeNode*)g0)->right;
                    if (n1) {
                        void* l0 = ((TourAreaTreeNode*)n1)->left;
                        if (l0) {
                            void* it = ((TourAreaTreeNode*)l0)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l0)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l0);
                        }
                        void* l1 = ((TourAreaTreeNode*)n1)->right;
                        if (l1) {
                            void* it = ((TourAreaTreeNode*)l1)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l1)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l1);
                        }
                        appFreeUni__FPv(n1);
                    }
                    appFreeUni__FPv(g0);
                }
                void* g1 = ((TourAreaTreeNode*)root)->right;
                if (g1) {
                    void* n0 = ((TourAreaTreeNode*)g1)->left;
                    if (n0) {
                        void* l0 = ((TourAreaTreeNode*)n0)->left;
                        if (l0) {
                            void* it = ((TourAreaTreeNode*)l0)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l0)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l0);
                        }
                        void* l1 = ((TourAreaTreeNode*)n0)->right;
                        if (l1) {
                            void* it = ((TourAreaTreeNode*)l1)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l1)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l1);
                        }
                        appFreeUni__FPv(n0);
                    }
                    void* n1 = ((TourAreaTreeNode*)g1)->right;
                    if (n1) {
                        void* l0 = ((TourAreaTreeNode*)n1)->left;
                        if (l0) {
                            void* it = ((TourAreaTreeNode*)l0)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l0)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l0);
                        }
                        void* l1 = ((TourAreaTreeNode*)n1)->right;
                        if (l1) {
                            void* it = ((TourAreaTreeNode*)l1)->left; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            it = ((TourAreaTreeNode*)l1)->right; if (it) RBTree_FreeTrivialSubtree(((char*)p + kAreaTreeOffset),it);
                            appFreeUni__FPv(l1);
                        }
                        appFreeUni__FPv(n1);
                    }
                    appFreeUni__FPv(g1);
                }
                appFreeUni__FPv(root);
            }
            }
            }
            }
            }
            }
            }
        // Phase 4: deleting <= 0 retains the selector allocation; return its address in r3.
        if (deleting>0) __dl__FPv(p);
    }
    return (int)p;
}

// Wait for both resources, create the two BP entries and layouts, and request menu BGM.
// Return 0 while either resource is pending, 1 after UI creation; invoke in the loading phase only.
int TourAreaSelect_PollResourcesAndCreateUi(F9B40A* p) {
    CPackTextureResource_PollLoad(AsObject802F9B40(p)->textureResource);
    if (!CPackTextureResource_IsLoadFinished(AsObject802F9B40(p)->textureResource)) return 0;
    CPackedFile_PollLoad(AsObject802F9B40(p)->packedFile);
    if (!CPackedFile_IsLoadFinished(AsObject802F9B40(p)->packedFile)) return 0;
    // Resource gate passed: the following calls create UI entries rather than merely poll them.
    AsObject802F9B40(p)->bpEntryCount=2;
    void* ha=s_bpCtrl;
    void** arr=s_tourAreaBpEntryNames;
    char* pp=(char*)AsObject802F9B40(p)->bpEntryIds;
    int i=0;
    while (i < AsObject802F9B40(p)->bpEntryCount) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(ha); __register_global_object(ha, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860); s_bpCtrlInitialized=1; }
            Sing_805DC5F0()=ha;
        }
        CBpCtrl_CreateEntry(Sing_805DC5F0(),*arr,pp,0);
        pp+=4;
        arr++;
        i++;
    }
    Layout_LoadExtended(AsObject802F9B40(p)->primaryLayout,0,tourAreaSelectStrings.primaryLayoutPath);
    Layout_LoadExtended(AsObject802F9B40(p)->secondaryLayout,0,tourAreaSelectStrings.secondaryLayoutPath);
    SoundManager_RequestBgmChange(g_soundManager,tourAreaSelectStrings.bgmName,0,1);
    return 1;
}

// Start display scripts, apply each area availability flag, and begin the screen fade.
int TourAreaSelect_StartDisplayAndFadeOut(F9B40B* p) {
    // The original exposes no defined return value; retain its existing ABI declaration.
    // Phase 1: start both display scripts. Repeated singleton checks preserve call ordering;
    // callbacks can run between checks, so do not cache one result across all calls.
    if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl,CBpCtrl_dtor,s_bpCtrlDtorRegistration_80522860); s_bpCtrlInitialized=1; } Sing_805DC5F0()=s_bpCtrl; }
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(),p->primaryBpId),tourAreaSelectStrings.primaryDisplayScript,1);
    if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl,CBpCtrl_dtor,s_bpCtrlDtorRegistration_80522860); s_bpCtrlInitialized=1; } Sing_805DC5F0()=s_bpCtrl; }
    BpObject_StartScriptByName(IdMap_Lookup(Sing_805DC5F0(),p->secondaryBpId),tourAreaSelectStrings.secondaryDisplayScript,1);
    // Phase 2: mirror availability into BP groups and layout controls using the map above.
    void* name;
    void** arr=tourAreaPointGroupNames;
    int off=0;
    for (int i = 0; i < kTourAreaCount; i++, off += kAreaRecordSize) {
        name = *arr;
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        void* o = IdMap_Lookup(Sing_805DC5F0(), p->primaryBpId);
        void* e = !o ? 0 : BpObject_GetOrCreateNamedGroup(o, name);
        if (e) {
            if ((p->areaRecords[off] >> kAreaEnabledShift) & 1) UIGroup_SetEnabled(e, 1);
            else UIGroup_SetEnabled(e, 0);
        }
        arr++;
    }
    off=0;
    for(int i=0;i<kTourAreaCount;i++, off+=kAreaRecordSize) Layout_SetElementPlayerFlagsAndState(p->primaryLayout,i,(unsigned char)((p->areaRecords[off]>>kAreaEnabledShift)&1));
    for(int i=0;i<kSubgroupCount;i++) Layout_SetElementPlayerFlagsAndState(p->secondaryLayout,i,0);
    char* s=tourAreaSelectStrings.hiddenGroup;
    if (!Sing_805DC5F0()) { if (!s_bpCtrlInitialized) { __ct__7CBpCtrlFv(s_bpCtrl); __register_global_object(s_bpCtrl,CBpCtrl_dtor,s_bpCtrlDtorRegistration_80522860); s_bpCtrlInitialized=1; } Sing_805DC5F0()=s_bpCtrl; }
    void* o=IdMap_Lookup(Sing_805DC5F0(),p->secondaryBpId); void* e = !o ? 0 : BpObject_GetOrCreateNamedGroup(o,s); UIGroup_SetEnabled(e,0);
    // Phase 3: notify every BP entry only after all visible states have been applied.
    int i=0;
    char* q=(char*)p;
    for (; i < p->bpEntryCount; i++) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_80522860);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        o = IdMap_Lookup(Sing_805DC5F0(), ((BpEntrySlot*)q)->id);
        BpObject_TryInvokeCurrentCallback(o, 0);
        q += 4;
    }
    // Phase 4: start the shared overlay's eight-frame fade after the callbacks.
    if (!Sing_805DC608()) {
        if (!s_fadeOverlayInitialized) {
            FadeOverlay_Init(s_fadeOverlay);
            __register_global_object(s_fadeOverlay, ResourceState_Dtor, s_fadeOverlayDtorRegistration_TourArea);
            s_fadeOverlayInitialized = 1;
        }
        Sing_805DC608() = s_fadeOverlay;
    }
    FadeState_StartFadeOut(Sing_805DC608(),kFadeFrames);
// The original returns no value: its callers never read one.
}

} // extern "C"
