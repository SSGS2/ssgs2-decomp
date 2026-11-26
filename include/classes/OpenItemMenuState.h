// OpenItemMenuState: field view inferred from accesses in the cited functions.
// No virtual declarations or inheritance; this header emits no code or RTTI.
#ifndef CLASSES_OPENITEMMENUSTATE_H
#define CLASSES_OPENITEMMENUSTATE_H

class OpenItemMenuState {
public:
    void* mapFile;  // 0x0: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap
    void* itemFile; // 0x4: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap
    void*
        messageFile; // 0x8: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap, CTourMain_ProcessItemMenu, CTourMain_ProcessSelectedMapItem
    void*
        cameraController; // 0xc: F2FC_Draw, F2FC_Unused, CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap, CTourMain_UpdateItemMenuVisuals, CTourMain_UpdateItemMapLayout, OpenItemMenuState_UpdateCameraOffsets, CTourMain_HandleItemMapNavigation, CTourMain_ProcessSelectedMapItem
    char pad_010[0x8];
    void* firstItemNode; // 0x18: F2FC_Draw, F2FC_Unused, CTourMain_UpdateItemMenuVisuals
    void*
        tourWindow; // 0x1c: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_ProcessItemMenu, CTourMain_DrawItemMenu
    void*
        tourHintWindow; // 0x20: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_DrawItemMenu
    void*
        tourVsWindow; // 0x24: CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_DrawItemMenu
    void*
        particleEffects; // 0x28: F2FC_Draw, F2FC_Unused, CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_UpdateItemMenuVisuals
    void* messageResourceHolder; // 0x2c: CTourMain_ProcessItemMenu, CTourMain_DrawItemMenu
    void* flashMemorySaveMenu;   // 0x30: CTourMain_ProcessItemMenu, CTourMain_DrawItemMenu
    void** mapData;              // 0x34: OpenItemMenu_BuildEntries
    void* coinItemData;          // 0x38: CTourMain_InitItemMap
    char pad_03C[0x4];
    void* shop; // 0x40: CTourMain_ProcessItemMenu, CTourMain_DrawItemMenu
    void*
        numberDraw; // 0x44: F2FC_Draw, F2FC_Unused, CTourMain_PollItemMapResources, CTourMain_InitItemMap, CTourMain_UpdateItemMapLayout
    void*
        menuContext; // 0x48: CTourMain_InitItemMap, OpenItemMenu_UpdateEntryVisuals, CTourMain_ProcessSelectedMapItem, CTourMain_PrepareSelectedEncounter
    void*
        tourProgress; // 0x4c: CTourMain_UpdateItemMapLayout, CTourMain_ProcessSelectedMapItem, CTourMain_PrepareSelectedEncounter
    void*
        selectionRecord; // 0x50: CTourMain_InitItemMap, OpenItemMenu_BuildEntries, CTourMain_HandleItemMapNavigation, OpenItemMenu_QueueSelectedItemRewards
    void* mapPackedFile; // 0x54: CTourMain_PollItemMapResources, CTourMain_ProcessItemMenu
    void* itemPackedFile; // 0x58: CTourMain_PollItemMapResources, CTourMain_ProcessItemMenu
    void* mapTexture;     // 0x5c: CTourMain_PollItemMapResources, CTourMain_ProcessItemMenu
    void* itemTexture;    // 0x60: CTourMain_PollItemMapResources, CTourMain_ProcessItemMenu
    char pad_064[0x8];
    void*
        modeGroup; // 0x6c: F2FC_Draw, F2FC_Unused, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap, CTourMain_UpdateItemMenuVisuals
    void*
        itemGroup; // 0x70: F0BObject, F263C_Object, F2FC_Draw, F2FC_Unused, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap, CTourMain_UpdateItemMenuVisuals, OpenItemMenu_BuildEntries
    void*
        numberGroup; // 0x74: F2FC_Draw, F2FC_Unused, OpenItemMenu_PollResourcesLoaded, CTourMain_InitItemMap, CTourMain_ResetItemMapControls, CTourMain_UpdateItemMapLayout
    void*
        layout; // 0x78: F2FC_Draw, F2FC_Unused, CTourMain_PollItemMapResources, OpenItemMenu_PollResourcesLoaded, CTourMain_ProcessItemMenu, CTourMain_ResetItemMapControls, CTourMain_UpdateItemMapLayout, CTourMain_HandleItemMapNavigation
    int state;  // 0x7c: CTourMain_ProcessItemMenu
    int pendingState;    // 0x80: CTourMain_ProcessItemMenu, CTourMain_HandleItemMapNavigation
    int windowState;     // 0x84: CTourMain_DrawItemMenu, CTourMain_ProcessSelectedMapItem
    float cameraOffsetX; // 0x88: CTourMain_UpdateItemMenuVisuals, OpenItemMenuState_UpdateCameraOffsets
    float cameraOffsetY; // 0x8c: CTourMain_UpdateItemMenuVisuals, OpenItemMenuState_UpdateCameraOffsets
    unsigned char
        tourCharacterIndex; // 0x90: CTourMain_PollItemMapResources, CTourMain_InitItemMap, CTourMain_ProcessItemMenu, CTourMain_UpdateItemMenuVisuals, OpenItemMenu_UpdateEntryVisuals, CTourMain_ProcessSelectedMapItem, CTourMain_PrepareSelectedEncounter
    char pad_091[0x3];
    int cameraMovePending;  // 0x94: CTourMain_HandleItemMapNavigation
    short initialSelection; // 0x98: CTourMain_InitItemMap, OpenItemMenu_BuildEntries, CTourMain_ProcessSelectedMapItem
    short
        selectedItem; // 0x9a: CTourMain_ProcessItemMenu, CTourMain_ProcessSelectedMapItem, CTourMain_PrepareSelectedEncounter, OpenItemMenu_QueueSelectedItemRewards
};

#define OFF(f, o) typedef char OpenItemMenuState_off_##f[((int)&((OpenItemMenuState*)0)->f == (o)) ? 1 : -1]
OFF(mapFile, 0x0);
OFF(itemFile, 0x4);
OFF(messageFile, 0x8);
OFF(cameraController, 0xc);
OFF(firstItemNode, 0x18);
OFF(tourWindow, 0x1c);
OFF(tourHintWindow, 0x20);
OFF(tourVsWindow, 0x24);
OFF(particleEffects, 0x28);
OFF(messageResourceHolder, 0x2c);
OFF(flashMemorySaveMenu, 0x30);
OFF(mapData, 0x34);
OFF(coinItemData, 0x38);
OFF(shop, 0x40);
OFF(numberDraw, 0x44);
OFF(menuContext, 0x48);
OFF(tourProgress, 0x4c);
OFF(selectionRecord, 0x50);
OFF(mapPackedFile, 0x54);
OFF(itemPackedFile, 0x58);
OFF(mapTexture, 0x5c);
OFF(itemTexture, 0x60);
OFF(modeGroup, 0x6c);
OFF(itemGroup, 0x70);
OFF(numberGroup, 0x74);
OFF(layout, 0x78);
OFF(state, 0x7c);
OFF(pendingState, 0x80);
OFF(windowState, 0x84);
OFF(cameraOffsetX, 0x88);
OFF(cameraOffsetY, 0x8c);
OFF(tourCharacterIndex, 0x90);
OFF(cameraMovePending, 0x94);
OFF(initialSelection, 0x98);
OFF(selectedItem, 0x9a);
#undef OFF

#endif
