extern "C" char s_bpCtrlDtorRegistration_8051D0EC[];
extern "C" char s_bpPlayerDtorRegistration_TourItemMenu[];
extern "C" char s_pangyaEnemyDataDtorRegistration_TourItemMenu[];
// Translation unit 0x802EB60C-0x802EC144 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma use_lmw_stmw on

#include "game/singletons.h"
#include "sdk_protos.h"


extern "C" {

struct TreeNode {
    TreeNode* left;
    TreeNode* right;
};

void RBTree_FreeTrivialSubtree(void*, void*);
void appFreeUni__FPv(void*);
void __dl__FPv(void*);
void TourMap_ReleaseScreenResources(void*);

struct Tree {
    void* header;
    TreeNode* root;
};

// A tour map begins with four owners, followed by the integer-key preview tree.
struct TourMapTreeView { void* owners[4]; Tree previews; };
#define FIELD_OFFSET(type, member) ((int)&((type*)0)->member)
struct PreviewRecord { int itemIndex; void* preview; };
struct PreviewTreeNode { void* links[3]; PreviewRecord record; };
struct CountedListHeader { char reserved[8]; int count; };
struct TextTableState { char reserved[0x28]; int enabled; };

// Keep the original four inlined levels before the generic subtree destructor.
// TreeErase0..3 are one postorder operation expanded by inlining: destroy left,
// destroy right, then free node. Tree payloads must already have been released;
// these helpers free tree-node storage only. TreeDtor0's repeated guards retain
// nested container destructor expansions, not additional payload cleanup.
static inline void TreeErase3(TreeNode* node, void* tree) {
    if (node != 0) {
        if (node->left != 0)
            RBTree_FreeTrivialSubtree((char*)tree + FIELD_OFFSET(TourMapTreeView, previews), node->left);
        if (node->right != 0)
            RBTree_FreeTrivialSubtree((char*)tree + FIELD_OFFSET(TourMapTreeView, previews), node->right);
        appFreeUni__FPv(node);
    }
}

// Destroy this subtree with two explicit levels before the generic fallback.
static inline void TreeErase2(TreeNode* node, void* tree) {
    if (node != 0) {
        TreeErase3(node->left, tree);
        TreeErase3(node->right, tree);
        appFreeUni__FPv(node);
    }
}

// Destroy this subtree with three explicit levels before the generic fallback.
static inline void TreeErase1(TreeNode* node, void* tree) {
    if (node != 0) {
        TreeErase2(node->left, tree);
        TreeErase2(node->right, tree);
        appFreeUni__FPv(node);
    }
}

// Destroy this subtree with four explicit levels before the generic fallback.
static inline void TreeErase0(TreeNode* node, void* tree) {
    if (node != 0) {
        TreeErase1(node->left, tree);
        TreeErase1(node->right, tree);
        appFreeUni__FPv(node);
    }
}

// Destroy the map preview tree from its root.
static inline void TreeClear(void* object) {
    TreeErase0(((TourMapTreeView*)object)->previews.root, object);
}

// Run the original nested tree-owner guards before clearing the root.
static inline void TreeDtor0(void* object) {
    Tree* tree = (Tree*)((unsigned long)object + FIELD_OFFSET(TourMapTreeView, previews));
    if (tree != 0)
        if (tree != 0)
            if (tree != 0)
                if (tree != 0)
                    if (tree != 0)
                        if (tree != 0)
                            TreeClear(object);
}

// Release the tour map screen, destroy its preview tree, and optionally free the owner.
void* TourMap_DestroyScreenAndPreviewTree(void* object, int shouldDelete) {
    if (object != 0) {
        TourMap_ReleaseScreenResources(object);
        TreeDtor0(object);
        if (shouldDelete > 0)
            __dl__FPv(object);
    }
    return object;
}


struct CleanupObject {
    void* primaryFileHolder;
    void* screenFileHolder;
    void* sharedFileOwner;
    void* cameraOwner;
    char bossPreviewTree[0x0c]; // Integer-key map owning the TourBossPreview pointers.
    void* auxiliaryScreenResource;
    void* hintWindow;
    void* versusWindow;
    void* particleEffects;
    void* messageResourceHolder;
    void* reservedResource30;
    void* mapData;
    // Concrete kinds of 38/3C/40 remain unresolved. They are independently owned:
    // 38 uses a vtable at +4; 3C/40 at +0. The destructor ABIs are not interchangeable.
    void* ownedResource38;
    void* ownedResource3C;
    void* ownedResource40;
    void* textTable;
    char reserved48[0x0c];
    // Their concrete resource classes are still unknown; teardown uses the same virtual slot.
    void* auxResource54;
    void* auxResource58;
    void* auxResource5C;
    void* auxResource60;
    char reserved64[8];
    int mainBpEntryId;
    int overlayBpEntryId;
    int effectBpEntryId;
    void* layout;
    char reserved7C[0x14];
    unsigned char musicIndex;
};

struct TwoInts {
    int a;
    int b;
};

extern char s_bpCtrl[];
extern char s_bpPlayer[];
extern char s_fontNumDrawRegistry[];
extern char s_itemMenuSingletonRegistration_8051D0E0[];
extern TwoInts s_bpCtrlHandle;
extern unsigned char s_bpCtrlInitialized;
extern unsigned char s_bpCtrlHandleInitialized;
extern unsigned char s_pointerVectorInitialized;
extern unsigned char s_bpPlayerInitialized;
extern int g_bpEntryDefaultMappedValue;
extern void* g_soundManager;
extern void* s_tourMapMusicNames[];

void CBpCtrlHandle_Dtor(void*);

struct VObj {
    virtual void Release(int);
};

struct VObj4 {
    char pad[4];
    virtual void Release(int);
};

// Delete an owned resource through the virtual interface at +0.
static inline void DestroyResource(void* resource) { ((VObj*)resource)->Release(1); }
// Delete an owned resource through the virtual interface at +4.
static inline void DestroyOffsetResource(void* resource) { ((VObj4*)resource)->Release(1); }

// Read the count from the optional shared list header.
static inline int ListCount(void* list) {
    void* first = *(void**)list;
    return first ? ((CountedListHeader*)first)->count : 0;
}

void __ct__7CBpCtrlFv(void*);
void CBpCtrl_dtor(void*);
void* IdMap_Lookup(void*, int);
void CBpCtrl_DeleteEntryById(void*, int);
void BpObject_SetEnabled(void*, int);
void CBpCtrlHandle_ReleaseEntry(void*);
void MsgResourceHolder_Dtor(void*, int);
void TourBossPreview_Destroy(void*, int);
short CMapData_GetItemIndexAt(void*, int);
void* IntegerKeyTree_FindInsertionPosition(void*, int*, void**, unsigned char*, unsigned char*);
void* BpEntryTree_InsertNode(void*, void*, int, int, void*);
void appFreeUni__FPv(void*);
void UITextTable_ClearEntries(void*);
void PointerVector_InitReserved(void*);
void PointerVector_DestroyStorage(void*);
void PointerArray_DeleteValue(void*, void*);
void Layout_Destroy(void*, int);
void TourParticleEffects_Dtor(void*, int);
void CTourVSWindow_Dtor(void*, int);
void CTourHintWindow_ReleaseSingletonResources(void*, int);
void CTourWindow_dtor(void*, int);
void CBpPlayer_Ctor(void*);
void CBpPlayer_Dtor(void*);
void TourMessageOwner_DestroyMessageDraw(void*);
void CBpPlayer_ReleaseResources(void*);
void CSharedFileHandleOwner_Dtor(void*, int);
void CSharedFileHandleHolder_Dtor(void*, int);
void CCameraOwner_Dtor(void*, int);
void Sound_StopByAlias(void*, void*);

// Find or insert one preview-map slot; the tree owns the node, not a new payload.
// key is passed by reference because the tree API receives its address.
static inline void FindOrInsertPreviewRecord(CleanupObject* self, int& key, PreviewRecord*& record,
    unsigned char& insertionFlag1, unsigned char& insertionFlag2) {
    void* node;
    void* position = IntegerKeyTree_FindInsertionPosition(self->bossPreviewTree, &key, &node, &insertionFlag1, &insertionFlag2);
    if (node == 0 || ((PreviewTreeNode*)node)->record.itemIndex < key) {
        int initialRecord[2];
        initialRecord[0] = key;
        initialRecord[1] = g_bpEntryDefaultMappedValue;
        record = &((PreviewTreeNode*)BpEntryTree_InsertNode(self->bossPreviewTree, position, insertionFlag1, insertionFlag2, initialRecord))->record;
    } else {
        record = &((PreviewTreeNode*)node)->record;
    }
}

// Stop map animations, release UI/preview resources and singleton handles, then stop map music.
void TourMap_ReleaseScreenResources(void* arg) {
    // Phase 1: quiesce all three BP entries and the text table before releasing owners.
    // Repeated singleton blocks may construct a manager even during teardown; retain
    // each lookup in place because resource destruction can affect global state.
    char* reg = s_itemMenuSingletonRegistration_8051D0E0;

    if (Sing_805DC5F0() == 0) {
        if ((signed char)s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    CleanupObject* self = (CleanupObject*)arg;
    void* found = IdMap_Lookup(Sing_805DC5F0(), self->mainBpEntryId);
    if (found != 0) BpObject_SetEnabled(found, 0);

    if (Sing_805DC5F0() == 0) {
        if ((signed char)s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    found = IdMap_Lookup(Sing_805DC5F0(), self->overlayBpEntryId);
    if (found != 0) BpObject_SetEnabled(found, 0);

    if (Sing_805DC5F0() == 0) {
        if ((signed char)s_bpCtrlInitialized == 0) {
            __ct__7CBpCtrlFv(s_bpCtrl);
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
    found = IdMap_Lookup(Sing_805DC5F0(), self->effectBpEntryId);
    if (found != 0) BpObject_SetEnabled(found, 0);

    if (self->textTable != 0) ((TextTableState*)self->textTable)->enabled = 0;

    if (Sing_805DC5F8() == 0) {
        if ((signed char)s_bpCtrlHandleInitialized == 0) {
            s_bpCtrlHandle.b = -1;
            s_bpCtrlHandle.a = -1;
            __register_global_object(&s_bpCtrlHandle, (void*)CBpCtrlHandle_Dtor, (void*)((reg + 0x18)));
            s_bpCtrlHandleInitialized = 1;
        }
        Sing_805DC5F8() = &s_bpCtrlHandle;
    }
    // Phase 2: release the singleton entry handle and individually owned resource objects.
    CBpCtrlHandle_ReleaseEntry(Sing_805DC5F8());

    {
        // +3C uses a vtable at zero; its concrete screen component is unresolved.
        void* p = self->ownedResource3C;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->ownedResource3C = 0;
        }
    }
    if (self->messageResourceHolder != 0) {
        MsgResourceHolder_Dtor(self->messageResourceHolder, 1);
        self->messageResourceHolder = 0;
    }
    {
        // +40 has the same destructor ABI as +3C, but is a separate owner.
        void* p = self->ownedResource40;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->ownedResource40 = 0;
        }
    }
    {
        // +38 uses the offset-vtable ABI; do not exchange these resource slots.
        void* p = self->ownedResource38;
        if (p != 0) {
            if (p != 0) DestroyOffsetResource(p);
            self->ownedResource38 = 0;
        }
    }
    if (self->mapData != 0) {
        // Phase 3: delete preview PAYLOADS by item key, leaving null values in the tree.
        // Repeated map subscript expansions may insert default entries; TreeDtor0 later
        // frees the nodes. mapData must stay alive throughout this indexed traversal.
        int i = 0;
        while (i < ListCount(self->mapData)) {
            int key = CMapData_GetItemIndexAt(self->mapData, i);
            if (key >= 0) {
                // Flag storage stays in the caller to preserve the three inlined stack layouts.
                unsigned char insertionFlag21, insertionFlag11;
                PreviewRecord* recordForTest;
                FindOrInsertPreviewRecord(self, key, recordForTest, insertionFlag11, insertionFlag21);
                if ((*(int*)&recordForTest->preview) != 0) {
                    unsigned char insertionFlag22, insertionFlag12;
                    PreviewRecord* recordForDeletion;
                    FindOrInsertPreviewRecord(self, key, recordForDeletion, insertionFlag12, insertionFlag22);
                    TourBossPreview_Destroy(recordForDeletion->preview, 1);

                    // Destruction may mutate the map: re-find the slot before clearing its payload.

                    unsigned char insertionFlag23, insertionFlag13;
                    PreviewRecord* recordForClearing;
                    FindOrInsertPreviewRecord(self, key, recordForClearing, insertionFlag13, insertionFlag23);
                    (*(int*)&recordForClearing->preview) = 0;
                }
            }
            ++i;
        }
        {
            void* p = self->mapData;
            if (p != 0) {
                if (p != 0) DestroyOffsetResource(p);
                self->mapData = 0;
            }
        }
    }

    // Phase 4: detach registered text and destroy layout/effects/windows.
    if (self->textTable != 0) {
        UITextTable_ClearEntries(self->textTable);
        if (Sing_805DC830() == 0) {
            if ((signed char)s_pointerVectorInitialized == 0) {
                PointerVector_InitReserved(s_fontNumDrawRegistry);
                __register_global_object(s_fontNumDrawRegistry, (void*)PointerVector_DestroyStorage, (void*)((reg + 0x48)));
                s_pointerVectorInitialized = 1;
            }
            Sing_805DC830() = s_fontNumDrawRegistry;
        }
        PointerArray_DeleteValue(Sing_805DC830(), self->textTable);
    }

    if (self->layout != 0) { Layout_Destroy(self->layout, 1); self->layout = 0; }
    if (self->particleEffects != 0) { TourParticleEffects_Dtor(self->particleEffects, 1); self->particleEffects = 0; }
    if (self->versusWindow != 0) { CTourVSWindow_Dtor(self->versusWindow, 1); self->versusWindow = 0; }
    if (self->hintWindow != 0) { CTourHintWindow_ReleaseSingletonResources(self->hintWindow, 1); self->hintWindow = 0; }
    if (self->auxiliaryScreenResource != 0) { CTourWindow_dtor(self->auxiliaryScreenResource, 1); self->auxiliaryScreenResource = 0; }

    // Phase 5: unregister BP entries before releasing their shared file holders.
    if (self->effectBpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if ((signed char)s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->effectBpEntryId);
        self->effectBpEntryId = -1;
    }
    if (self->overlayBpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if ((signed char)s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->overlayBpEntryId);
        self->overlayBpEntryId = -1;
    }

    if (self->screenFileHolder != 0) { CSharedFileHandleHolder_Dtor(self->screenFileHolder, 1); self->screenFileHolder = 0; }

    if (self->mainBpEntryId != -1) {
        if (Sing_805DC5F0() == 0) {
            if ((signed char)s_bpCtrlInitialized == 0) {
                __ct__7CBpCtrlFv(s_bpCtrl);
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, (void*)((reg + 0xc)));
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        CBpCtrl_DeleteEntryById(Sing_805DC5F0(), self->mainBpEntryId);
        self->mainBpEntryId = -1;
    }

    if (Sing_805DC670() == 0) {
        if ((signed char)s_bpPlayerInitialized == 0) {
            CBpPlayer_Ctor(s_bpPlayer);
            __register_global_object(s_bpPlayer, (void*)CBpPlayer_Dtor, (void*)((reg + 0x90)));
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = s_bpPlayer;
    }
    // Phase 6: release global message playback, then the remaining file/camera owners.
    TourMessageOwner_DestroyMessageDraw(Sing_805DC670());

    if (Sing_805DC670() == 0) {
        if ((signed char)s_bpPlayerInitialized == 0) {
            CBpPlayer_Ctor(s_bpPlayer);
            __register_global_object(s_bpPlayer, (void*)CBpPlayer_Dtor, (void*)((reg + 0x90)));
            s_bpPlayerInitialized = 1;
        }
        Sing_805DC670() = s_bpPlayer;
    }
    CBpPlayer_ReleaseResources(Sing_805DC670());

    if (self->sharedFileOwner != 0) { CSharedFileHandleOwner_Dtor(self->sharedFileOwner, 1); self->sharedFileOwner = 0; }
    if (self->primaryFileHolder != 0) { CSharedFileHandleHolder_Dtor(self->primaryFileHolder, 1); self->primaryFileHolder = 0; }
    if (self->cameraOwner != 0) { CCameraOwner_Dtor(self->cameraOwner, 1); self->cameraOwner = 0; }

    {
        void* p = self->auxResource60;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->auxResource60 = 0;
        }
    }
    {
        void* p = self->auxResource5C;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->auxResource5C = 0;
        }
    }
    {
        void* p = self->auxResource58;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->auxResource58 = 0;
        }
    }
    {
        void* p = self->auxResource54;
        if (p != 0) {
            if (p != 0) DestroyResource(p);
            self->auxResource54 = 0;
        }
    }

    // Phase 7: owned resources are gone; stop the map's selected music alias last.
    Sound_StopByAlias(g_soundManager, s_tourMapMusicNames[self->musicIndex]);
}

} // extern "C"
