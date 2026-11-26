// Tour versus window, part 2: the VS window's message text and status, the global VS window with its
// eight portrait textures, the map-data row getters, and the tour participant list manager.

#include <decomp/static_init.h>
// String-pool view; byte lengths include terminators and original padding.
struct TourWindowStringPool {
    char layoutFile[12];        // +0x0
    char defaultScript[7];      // +0xC
    char playerOnePortrait[11]; // +0x13
    char playerTwoPortrait[11]; // +0x1E
    char playerOneName[15];     // +0x29
    char playerOneNameBase[14]; // +0x38
    char playerTwoName[15];     // +0x46
    char playerTwoNameBase[14]; // +0x55
    char scrollGroup[9];        // +0x63
    char enterScript[11];       // +0x6C
    char versusAnimation[9];    // +0x77
    char iconScript[13];        // +0x80
    char exitScript[7];         // +0x8D
    char messageGroup[12];      // +0x94
};
#define TourStrings (*((TourWindowStringPool*)tourVsWindowStrings))

#include "classes/TourVSWindowData.h"

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CTourMessage.h"
#include "classes/CTourVSWindow.h"
#include "classes/CMapData.h"
#include "classes/CBpCtrl.h"
#pragma use_simple_stdpnew on
inline void* operator new(size_t, void* ptr) {
    return ptr;
}

enum TourWindowState {
    kTourWindowIdle = 0,
    kTourWindowMessage = 4
};
enum {
    kTourWindowReleased = 3,
    kPortraitTextureCount = 8,
    kMessageGroupCount = 9
};
enum {
    kPlayerOneMessageColor = 0x20206000,
    kPlayerTwoMessageColor = 0x60202000
};

extern "C" {

struct TourMessageWindowView {
    char reserved00[0x24];
    int status;
    void* layoutId;
    char reserved2C[0x20];
    int playerOneNameEnabled;
    int playerTwoNameEnabled;
    char reserved54[0x08];
    int scrollGroupEnabled;
};

struct CtrlFrame {
    int positionXBits;
    float sourceY;
};

// Map table rows begin with item, four signed connection styles, then per-node bytes.
// -1 disables the edge; style 2 hides selected lit links (tour_character_map_board.cpp). Destinations
// are computed separately from the fixed grid by OpenItemEntry_GetNeighborIndex.
struct MapNodeRecord {
    short itemIndex;
    short connectionStyles[4];
    unsigned char reserved0A, itemFlag13, itemFlag14;
};
struct GetterOwner {
    int nodeTable;
};

struct NameTable9 {
    void* names[9];
};

extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char tourVsWindowStrings[];
extern char s_bpCtrlDtorRecord_TourMap[];
extern NameTable9 tourVsMessageWindowHideGroupNames;
extern NameTable9 s_tourVsSourceFrameGroupNames; // Source-frame groups shifted between the two player portrait banks.
extern float kTourVsBannerOffsetX64;
extern const float kFloat_6_2831855;
extern float s_mapDataFullTurnAngle;
extern float s_tourParticipantIndexFullTurnAngle;

void CBpCtrl_dtor(void*, int);
void* IdMap_Lookup(void*, void*);
void* BpObject_GetOrCreateNamedGroup(void*, void*);
void UIGroup_GetPosition(void*, CtrlFrame*);
void UIGroup_SetEnabled(void*, int);
void UIGroup_GetSourcePosition(void*, CtrlFrame*);
void UIGroup_SetSourcePosition(void*, CtrlFrame*);
short* Table_GetEntry(int, int);

// Initialize the BP controller singleton and register its destructor on first use.
static inline void EnsureBpController() {
    if (!Sing_805DC5F0()) {
        if (!s_bpCtrlInitialized) {
            ((CBpCtrl*)s_bpCtrl)->__ct();
            __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
            s_bpCtrlInitialized = 1;
        }
        Sing_805DC5F0() = s_bpCtrl;
    }
}

// Keep these group expansions local: return-value and output-reference helpers change 11 text bytes.
#define BP_FIND(dst, nameexpr)                                                    \
    {                                                                             \
        void* bpName = (void*)(nameexpr);                                         \
        void* bpOwner;                                                            \
        EnsureBpController();                                                     \
        bpOwner = IdMap_Lookup(Sing_805DC5F0(), (void*)layoutId);                 \
        dst = bpOwner == 0 ? 0 : BpObject_GetOrCreateNamedGroup(bpOwner, bpName); \
    }

#define BP_SET(nameexpr, val)                \
    {                                        \
        void* bpItem;                        \
        BP_FIND(bpItem, nameexpr)            \
        if (bpItem) {                        \
            UIGroup_SetEnabled(bpItem, val); \
        }                                    \
    }

} // extern "C"

// Select the player name/color, update message text and shift source frames to the matching player bank.
void CTourVSWindow::SetMessageMode(int mode, TourMessageText* message) {
    NameTable9 visibleGroups;
    NameTable9 sourceFrameGroups;
    CtrlFrame messagePosition;
    float playerFrameOffset;
    void* ctrl;
    void* item;

    BP_FIND(ctrl, TourStrings.messageGroup)
    UIGroup_GetPosition(ctrl, &messagePosition);
    if (mode == 0) {
        ((CTourMessage*)this)->SetBaseGlyphColor(kPlayerOneMessageColor);
    } else {
        ((CTourMessage*)this)->SetBaseGlyphColor(kPlayerTwoMessageColor);
    }
    ((CTourMessage*)this)->SetText(message, &messagePosition);

    visibleGroups = tourVsMessageWindowHideGroupNames;
    for (unsigned int i = 0; i < kMessageGroupCount; i++) {
        BP_FIND(item, visibleGroups.names[i])
        if (item) {
            UIGroup_SetEnabled(item, 1);
        }
    }

    sourceFrameGroups = s_tourVsSourceFrameGroupNames;
    playerFrameOffset = kTourVsBannerOffsetX64;
    int modeIsZero = (signed char)mode == 0;
    for (unsigned int i = 0; i < kMessageGroupCount; i++) {
        CtrlFrame sourcePosition = {0};
        BP_FIND(item, sourceFrameGroups.names[i])
        if (item) {
            UIGroup_GetSourcePosition(item, &sourcePosition);
            sourcePosition.sourceY = (sourcePosition.sourceY >= playerFrameOffset && modeIsZero)
                                         ? sourcePosition.sourceY - playerFrameOffset
                                     : (sourcePosition.sourceY < playerFrameOffset && (signed char)mode != 0)
                                         ? playerFrameOffset + sourcePosition.sourceY
                                         : sourcePosition.sourceY;
            UIGroup_SetSourcePosition(item, &sourcePosition);
        }
    }

    if ((signed char)mode == 0) {
        BP_SET(TourStrings.playerOneName, 1)
        BP_SET(TourStrings.playerOneNameBase, 1)
    } else {
        BP_SET(TourStrings.playerTwoName, 1)
        BP_SET(TourStrings.playerTwoNameBase, 1)
    }

    {
        int off = mode == 0;
        if (!(signed char)off) {
            BP_SET(TourStrings.playerOneName, 0)
            BP_SET(TourStrings.playerOneNameBase, 0)
        } else {
            BP_SET(TourStrings.playerTwoName, 0)
            BP_SET(TourStrings.playerTwoNameBase, 0)
        }
        playerOneNameEnabled = off;
    }

    playerTwoNameEnabled = mode;
    scrollGroupEnabled = 1;
}

extern "C" {

// Store the VS window status value.
void TourVSWindow_SetStatus(TourMessageWindowView* self, int value) {
    self->status = value;
}

// Test whether the VS window has the requested status.
int TourVSWindow_HasStatus(TourMessageWindowView* self, int value) {
    return self->status == value;
}

// This tour-window lifecycle hook intentionally performs no work.
void TourWindow_EmptyHook0(void) {}

// This tour-window lifecycle hook intentionally performs no work.
void TourWindow_EmptyHook1(void) {}

// Initialize the map-data module angle storage to one full turn.
void MapData_InitAngleConstant(void) {
    s_mapDataFullTurnAngle = kFloat_6_2831855;
}

} // extern "C"

// Map rows: each getter returns zero when the row is absent.
short CMapData::GetItemIndexAt(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->itemIndex;
    return 0;
}

short CMapData::GetDirection0StyleAt(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->connectionStyles[0];
    return 0;
}

short CMapData::GetDirection1StyleAt(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->connectionStyles[1];
    return 0;
}

extern "C" {

// Initialize the participant-list module angle storage to one full turn.
void TourParticipantIndex_InitAngleConstant(void) {
    s_tourParticipantIndexFullTurnAngle = kFloat_6_2831855;
}

} // extern "C"

short CMapData::GetDirection2StyleAt(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->connectionStyles[2];
    return 0;
}

short CMapData::GetDirection3StyleAt(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->connectionStyles[3];
    return 0;
}

// Return the map row flag at byte eleven, or zero if absent.
unsigned char CMapData::GetItemFlag13At(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->itemFlag13;
    return 0;
}

// Return the map row flag at byte twelve, or zero if absent.
unsigned char CMapData::GetItemFlag14At(int index) const {
    MapNodeRecord* value = (MapNodeRecord*)Table_GetEntry(nodeTable, index);
    if (value)
        return value->itemFlag14;
    return 0;
}

extern "C" {

void* __nwa__FUl(unsigned long);
struct BannerItem {
    virtual void slot0();
    virtual void slot4(void*);
    virtual void slot8();
    virtual void slotC(void*);
    virtual int ready();
    char data[0x24];
};

struct Banner {
    char data[0x20];
    int state;
    int status;
    void* owner;
    char data2C[8];
    BannerItem* items;
};

extern signed char s_tourVsWindowInitialized;
extern Banner s_tourVSWindow;
extern char s_tourVsWindowDtorRegistration[];
void Destroy__13CTourVSWindowFi(void*, int);
int BpObject_PollLoad(void*);
void BpObject_TryInvokeCurrentCallback(void*, int);

// Create the global VS window and register its destructor on first use.
static inline void EnsureTourVSWindow() {
    if (!((Banner*&)Sing_805DC690())) {
        if (!s_tourVsWindowInitialized) {
            ((CTourVSWindow*)&s_tourVSWindow)->__ct();
            __register_global_object(&s_tourVSWindow, (void*)Destroy__13CTourVSWindowFi,
                                     s_tourVsWindowDtorRegistration);
            s_tourVsWindowInitialized = 1;
        }
        ((Banner*&)Sing_805DC690()) = &s_tourVSWindow;
    }
}

void BpObject_SetEnabled(void*, int);
void __destroy_new_array(void*, void*);
extern "C" void __dt__6CBpTexFv(void*, int);
extern "C" void __ct__6CBpTexFv(void*);
void __dl__FPv(void*);
void* __nwa__FUl(unsigned long);
struct NameTable8 {
    void* names[8];
};
extern NameTable8 s_tourVsPortraitResourceNames; // Eight portrait resource names, in their texture-slot order.
} // extern "C"

// Hide the global VS window, release portrait arrays and delete its layout entry.
void* CTourVSWindow::Dtor(int destroy) {
    if (this) {
        EnsureTourVSWindow();
        Banner* banner = ((Banner*&)Sing_805DC690());
        banner->status = kTourWindowReleased;
        banner->state = 0;
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, (void*)CBpCtrl_dtor, s_bpCtrlDtorRecord_TourMap);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        BpObject_SetEnabled(IdMap_Lookup(Sing_805DC5F0(), banner->owner), 0);
        if (banner->items) {
            __destroy_new_array(banner->items, (void*)__dt__6CBpTexFv);
            banner->items = 0;
        }
        if ((int)banner->owner != -1) {
            EnsureBpController();
            ((CBpCtrl*)Sing_805DC5F0())->DeleteEntryById((int)banner->owner);
            banner->owner = (void*)-1;
        }
        if (destroy > 0)
            __dl__FPv(this);
    }
    return this;
}

// Create the VS layout and load its eight portrait textures.
void CTourVSWindow::LoadUIResources() {
    void* block;
    int i;
    void** name;
    int offset;
    EnsureTourVSWindow();
    EnsureTourVSWindow();
    NameTable8 names = s_tourVsPortraitResourceNames;
    Banner* banner = ((Banner*&)Sing_805DC690());
    EnsureBpController();
    ((CBpCtrl*)Sing_805DC5F0())->CreateEntry(tourVsWindowStrings, &banner->owner, 0);
    if (banner->items) {
        __destroy_new_array(banner->items, (void*)__dt__6CBpTexFv);
        banner->items = 0;
    }
    block = __nwa__FUl(0x150);
    if (block)
        block = __construct_new_array(block, (void*)__ct__6CBpTexFv, (void*)__dt__6CBpTexFv, 0x28, 8);
    banner->items = (BannerItem*)block;
    name = names.names;
    i = 0;
    offset = 0;
    do {
        ((BannerItem*)((char*)banner->items + offset))->slot4(*name);
        ++i;
        ++name;
        offset += 0x28;
    } while (i < kPortraitTextureCount);
}

// Draw the lazily initialized global VS window.
void CTourVSWindow::DrawGlobal() {
    EnsureTourVSWindow();
    ((CTourVSWindow*)(((Banner*&)Sing_805DC690())))->Draw();
}

// Check that the global VS layout and all eight portrait textures have loaded.
static inline int bannerReady() {
    EnsureTourVSWindow();
    Banner* banner = ((Banner*&)Sing_805DC690());
    EnsureBpController();
    if (!BpObject_PollLoad(IdMap_Lookup(Sing_805DC5F0(), banner->owner)))
        return 0;
    for (int i = 0; i < kPortraitTextureCount; ++i) {
        if (!banner->items[i].ready())
            return 0;
    }
    return 1;
}

// Report whether the global VS layout and portrait textures are ready.
bool CTourVSWindow::IsReady() {
    return bannerReady();
}

extern "C" {

#undef BP_FIND
#undef BP_SET

// Return the global VS window, initializing and registering it on first use.
static inline Banner* getBanner() {
    EnsureTourVSWindow();
    return ((Banner*&)Sing_805DC690());
}

// Invoke the layout callback and refresh the scroll group while the message state is active.
int TourVSWindow_ProcessLayoutCallback() {
    Banner* self = getBanner();
    EnsureBpController();
    BpObject_TryInvokeCurrentCallback(IdMap_Lookup(Sing_805DC5F0(), self->owner), 0);
    switch (self->state) {
    case kTourWindowIdle:
        break;
    case kTourWindowMessage:
        if (((TourVSWindowData*)self)->scrollGroupEnabled) {
            void* name = TourStrings.scrollGroup;
            int value = *(int*)((char*)self + 4);
            EnsureBpController();
            void* owner = IdMap_Lookup(Sing_805DC5F0(), self->owner);
            void* item = owner == 0 ? 0 : BpObject_GetOrCreateNamedGroup(owner, name);
            if (item)
                UIGroup_SetEnabled(item, value);
        }
        break;
    }
    return 1;
}

struct TreeNode {
    char pad[12];
    int key;
    void* value;
};
struct TreePair {
    int key;
    void* value;
};
void* IntegerKeyTree_FindInsertionPosition(void*, int*, TreeNode**, unsigned char*, unsigned char*);
TreeNode* BpEntryTree_InsertNode(void*, void*, unsigned char, unsigned char, TreePair*);
extern void* s_tourParticipantDefaultValue; // Default tree value used only until the participant record is assigned.
void Layout_Init(void*, int);
void* SmallTree_dtor(void*, int);
struct SmallTree {
    int count;
    void* root;
    void* end;
    SmallTree() : count(0), root(0), end(&root) {}
    ~SmallTree() {
        SmallTree_dtor(this, -1);
    }
};
struct Allocated {
    char data[0x5c];
    Allocated(int n) {
        Layout_Init(this, n);
    }
};
// The source owns eight 0x4A-byte records; only presence/exclusion flag bits are consumed here.
struct TourParticipantRecord {
    unsigned char flags;
    char reserved01[0x49];
};
struct TourParticipantOwnerView {
    char reserved000[0x4EC];
    TourParticipantRecord records[8];
};
enum TourParticipantFlagBits {
    kParticipantPresentBit = 7,
    kParticipantExcludedBit = 6
};
struct Manager {
    int id;
    char pad4[0x10];
    int active;
    Allocated* first;
    Allocated* second;
    int field20;
    int field24;
    unsigned char* records;
    SmallTree tree;
    Manager(void**);
};
// Index present, nonexcluded participant records and create two five-entry layouts.
inline Manager::Manager(void** arg) {
    id = -1;
    active = 1;
    records = (unsigned char*)&((TourParticipantOwnerView*)*arg)->records;
    field20 = 0;
    field24 = 0;
    for (int i = 0; i < kPortraitTextureCount; ++i) {
        unsigned char flag = records[i * (int)sizeof(TourParticipantRecord)];
        if (((flag >> kParticipantPresentBit) & 1) && !((flag >> kParticipantExcludedBit) & 1)) {
            TreeNode* node;
            unsigned char b, a;
            void* found = IntegerKeyTree_FindInsertionPosition(&tree, &i, &node, &a, &b);
            TreePair* pair;
            if (!node || (pair = (TreePair*)&node->key, node->key < i)) {
                TreePair entry = {i, s_tourParticipantDefaultValue};
                pair = (TreePair*)&BpEntryTree_InsertNode(&tree, found, a, b, &entry)->key;
            }
            pair->value = records + i * (int)sizeof(TourParticipantRecord);
        }
    }
    first = new Allocated(5);
    second = new Allocated(5);
}

// Construct the participant manager and index enabled, nonexcluded records.
Manager* TourAreaSelect_Ctor(Manager* self, void** arg) {
    return new (self) Manager(arg);
}

void RBTree_FreeTrivialSubtree(void*, void*);
void appFreeUni__FPv(void*);
struct CleanupNode {
    CleanupNode* left;
    CleanupNode* right;
};

#define CLEAN_LEAF(expr)                                      \
    {                                                         \
        CleanupNode* node = (expr);                           \
        if (node) {                                           \
            if (node->left)                                   \
                RBTree_FreeTrivialSubtree(self, node->left);  \
            if (node->right)                                  \
                RBTree_FreeTrivialSubtree(self, node->right); \
            appFreeUni__FPv(node);                            \
        }                                                     \
    }
#define CLEAN_TWO(expr)               \
    {                                 \
        CleanupNode* parent = (expr); \
        if (parent) {                 \
            CLEAN_LEAF(parent->left)  \
            CLEAN_LEAF(parent->right) \
            appFreeUni__FPv(parent);  \
        }                             \
    }
#define CLEAN_THREE(expr)             \
    {                                 \
        CleanupNode* branch = (expr); \
        if (branch) {                 \
            CLEAN_TWO(branch->left)   \
            CLEAN_TWO(branch->right)  \
            appFreeUni__FPv(branch);  \
        }                             \
    }
#pragma use_lmw_stmw on
// Release the participant index tree nodes and optionally free its container.
void* SmallTree_dtor(void* self, int destroy) {
    if (self) {
        if (self) {
            if (self) {
                if (self) {
                    if (self) {
                        if (self) {
                            CleanupNode* root = ((CleanupNode**)self)[1];
                            if (root) {
                                CLEAN_THREE(root->left)
                                CLEAN_THREE(root->right)
                                appFreeUni__FPv(root);
                            }
                        }
                    }
                }
            }
        }
        if (destroy > 0)
            __dl__FPv(self);
    }
    return self;
}
#pragma use_lmw_stmw reset
#undef CLEAN_THREE
#undef CLEAN_TWO
#undef CLEAN_LEAF

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU8046259C", _ctors_u802F85E0_0, MapData_InitAngleConstant);
STATIC_INIT_ENTRY(".CRT$XCU8046259C", _ctors_u802F85E0_1, TourParticipantIndex_InitAngleConstant);
#pragma pop
