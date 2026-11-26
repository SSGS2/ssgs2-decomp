#ifndef GAME_CLASS2_ITEMWINDOW_H
#define GAME_CLASS2_ITEMWINDOW_H

class CItemWindow;
extern "C" int s_itemWindowState;

// Field names below are inferred from how the game code uses each member.
struct Message;
class CItemWindow {
public:
    void* vtable; // 00: __vt__11CItemWindow
    char pad_04[4];
    int displayPriority; // 08: actor priority = 4
    char pad_0C[0x0C];
    char pad_18[4];         // map bookkeeping, precise role unproven
    void* overlayRoot;      // 1C: tree root, destroyed recursively
    void* overlayFirst;     // 20: first node / &overlayRoot sentinel
    void* overlay;          // 24: WOverlay_Create result
    float bounds[4];        // 28: item window rectangle
    float overlayBounds[4]; // 38: WOverlay rectangle
    float panelOffsetX;     // 48: added to bounds[0]
    char pad_4C[8];
    float pointerX;                   // 54: copied from pointer service
    float pointerY;                   // 58
    unsigned char alternateIconFrame; // 5C: low byte of alternate-icon frame; equality 8 resets to zero
    char pad_5D[3];
    int activatedSlot; // 60: Layout_CollectActivated index
    int selectedSlot;  // 64: pending item slot, -1 when cleared
    char pad_68[4];
    unsigned int usedItemCode; // 6C: code saved when item removed
    char pad_70[8];
    float animationTime;            // 78: incremented by dt
    float animationX;               // 7C: used as drawing coordinate
    float animationY;               // 80
    float animationVelocityX;       // 84: added to animationX
    float animationVelocityY;       // 88: acceleration, then added to Y
    unsigned char animationActive;  // 8C: gates animation processing/drawing
    unsigned char useAlternateItem; // 8D: chooses replacement code 0x0E
    unsigned char active;           // 8E: messages 6/7, gates input processing
    unsigned char selectionEnabled; // 8F: messages 1/2, gates slot selection
    void* itemTextures;             // 90: 11 CBpTex objects, 0x28 stride
    void* itemLayout;               // 94: Layout_Init/LoadExtended result
    int controllerId;               // 98: CBpCtrl entry ID, -1 invalid
    int itemCount;                  // 9C: copied from player's byte 0x9C
    int panelOpen;                  // A0: guards open/close scripts
    int drawRequested;              // A4: message 4 sets, OnDisplay clears
    int layoutInitialized;          // A8: one-time element flag setup
    float slotOrigins[8][2];        // AC: element position minus element size

    CItemWindow();
    // Shared reset sequence in OnInit and HandleMsg(11); unknown words stay padding.
    void ResetSelection(float zero, float full) {
        panelOffsetX = zero;
        *(float*)&pad_4C[4] = zero;
        s_itemWindowState = 3;
        int unselected = -1;
        activatedSlot = unselected;
        *(int*)pad_68 = unselected;
        selectedSlot = unselected;
        usedItemCode = 0;
        *(float*)pad_70 = full;
        *(float*)&pad_70[4] = full;
        animationActive = 0;
        selectionEnabled = 1;
    }
    static int IsClosed();

    void UseItemForOwner(void* owner, void* itemIdArgument, void* randomValue);
    static void* GetRTTI();

    ~CItemWindow();

    void OnDestroy();
    void OnLoad();
    void OnInit();
    void OnProcess(float dt);
    void OnDisplay();
    void UpdatePanelVisibility();
    void HandleItemSelection();
    int UseItem(unsigned char playerIndex, void* itemIdArgument, int slotOrConsumeSelection, void* randomValue);
    void HandleMsg(Message* msg);
};

typedef char CItemWindow_size_check[sizeof(CItemWindow) == 0xEC ? 1 : -1];
#define ITEMWINDOW_OFF(f, o) typedef char CItemWindow_offset_##f[((int)&((CItemWindow*)0)->f == o) ? 1 : -1]
ITEMWINDOW_OFF(vtable, 0x00);
ITEMWINDOW_OFF(displayPriority, 0x08);
ITEMWINDOW_OFF(pad_18, 0x18);
ITEMWINDOW_OFF(overlayRoot, 0x1C);
ITEMWINDOW_OFF(overlayFirst, 0x20);
ITEMWINDOW_OFF(overlay, 0x24);
ITEMWINDOW_OFF(bounds, 0x28);
ITEMWINDOW_OFF(overlayBounds, 0x38);
ITEMWINDOW_OFF(panelOffsetX, 0x48);
ITEMWINDOW_OFF(pad_4C, 0x4C);
ITEMWINDOW_OFF(pointerX, 0x54);
ITEMWINDOW_OFF(pointerY, 0x58);
ITEMWINDOW_OFF(alternateIconFrame, 0x5C);
ITEMWINDOW_OFF(activatedSlot, 0x60);
ITEMWINDOW_OFF(selectedSlot, 0x64);
ITEMWINDOW_OFF(pad_68, 0x68);
ITEMWINDOW_OFF(usedItemCode, 0x6C);
ITEMWINDOW_OFF(pad_70, 0x70);
ITEMWINDOW_OFF(animationTime, 0x78);
ITEMWINDOW_OFF(animationX, 0x7C);
ITEMWINDOW_OFF(animationY, 0x80);
ITEMWINDOW_OFF(animationVelocityX, 0x84);
ITEMWINDOW_OFF(animationVelocityY, 0x88);
ITEMWINDOW_OFF(animationActive, 0x8C);
ITEMWINDOW_OFF(useAlternateItem, 0x8D);
ITEMWINDOW_OFF(active, 0x8E);
ITEMWINDOW_OFF(selectionEnabled, 0x8F);
ITEMWINDOW_OFF(itemTextures, 0x90);
ITEMWINDOW_OFF(itemLayout, 0x94);
ITEMWINDOW_OFF(controllerId, 0x98);
ITEMWINDOW_OFF(itemCount, 0x9C);
ITEMWINDOW_OFF(panelOpen, 0xA0);
ITEMWINDOW_OFF(drawRequested, 0xA4);
ITEMWINDOW_OFF(layoutInitialized, 0xA8);
ITEMWINDOW_OFF(slotOrigins, 0xAC);
#undef ITEMWINDOW_OFF
#endif
