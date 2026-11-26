// GameInputManager: game-level input methods in game_input_manager_cursor_rumble.cpp, not engine CInputManager.
// Non-virtual layout view; field offsets follow the original function addresses below.
#ifndef CLASSES_GAMEINPUTMANAGER_H
#define CLASSES_GAMEINPUTMANAGER_H

class GameInputManager {
public:
    int selectedPlayer;    // 0x0: InputManager_GetSelectedPlayer / InputManager_SetSelectedPlayer
    void** cursorLayouts;  // 0x4: InputManager_GetCursorDepth / InputManager_Update: Layout cursor objects
    int cursorLayoutCount; // 0x8: InputManager_Update / Dtor: cursor array end
    char pad_00C[0x4];
    void**
        cursorTrails; // 0x10: InputManager_LoadCursorResources: TrailRibbon pointer array; InputManager_Dtor frees trails
    int cursorTrailCount; // 0x14: number of cursor trail slots
    char pad_018[0x4];
    int actionBlocked[39];          // 0x1C: Input_IsActionPressed (0x8006F5B0) suppresses physical input
    int actionBlockedSecondary[39]; // 0xB8: same query's second suppression array; distinction unknown
    int selectedPadLookupDisabled;  // 0x154: InputManager_GetSelectedPad: gates selected pad lookup
    int pendingActionPress[39];     // 0x158: InputManager_Update (0x8006F060) consumes and clears synthetic presses
    int actionPress[39];            // 0x1F4: Update publishes pending presses; action queries return true here
    int rumbleFramesRemaining[4];   // 0x290: Update decrements; zero stops PadManager channel; -1 inactive
    int rumblePaused;               // 0x2a0: InputManager_PauseRumble / ResumeRumble / StartRumble
    char pad_2A4[0xc];
    int cursorDrawingSuspendCount; // 0x2b0: InputManager_SuspendCursorDrawing / InputManager_ResumeCursorDrawing: increment and saturating decrement
    unsigned char cursorBaseColors[4][4];   // 0x2B4: InputManager_LoadCursorResources copies RGBA to cursorColors
    unsigned char cursorActiveColors[4][4]; // 0x2C4: InputManager_UpdateCursorColor chooses when cursorAppearance is 2
    unsigned char cursorAlternateColors[4][4]; // 0x2D4: InputManager_UpdateCursorColor fallback palette
    unsigned char cursorColors
        [4]
        [4]; // 0x2E4: InputManager_DrawPlayerCursorWithOptions/InputManager_DrawPlayerCursor read RGBA for cursor vertices
    char pad_2F4[0xc0];
    int cursorAppearance
        [4]; // 0x3B4: InputManager_RaiseCursorAppearance raises state; InputManager_UpdateCursorColor selects palette
    int cursorMode;             // 0x3c4: InputManager_Update: 2 with golf players, otherwise 0 or 1
    int cursorModeRequestCount; // 0x3c8: InputManager_Update: selects fallback cursor mode
    char pad_3CC[0x30];
    int pointerOverrideEnabled; // 0x3fc: InputManager_SetPointerPositionOverride / GetPointerPosition
    float pointerOverrideX;     // 0x400: InputManager_SetPointerPositionOverride: first coordinate
    float pointerOverrideY;     // 0x404: InputManager_SetPointerPositionOverride: second coordinate
};

#define GAMEINPUTMANAGER_OFF(f, o) \
    typedef char GameInputManager_off_##f[((int)&((GameInputManager*)0)->f == (o)) ? 1 : -1]
GAMEINPUTMANAGER_OFF(selectedPlayer, 0x0);
GAMEINPUTMANAGER_OFF(cursorLayouts, 0x4);
GAMEINPUTMANAGER_OFF(cursorLayoutCount, 0x8);
GAMEINPUTMANAGER_OFF(cursorTrails, 0x10);
GAMEINPUTMANAGER_OFF(cursorTrailCount, 0x14);
GAMEINPUTMANAGER_OFF(actionBlocked, 0x1C);
GAMEINPUTMANAGER_OFF(actionBlockedSecondary, 0xB8);
GAMEINPUTMANAGER_OFF(selectedPadLookupDisabled, 0x154);
GAMEINPUTMANAGER_OFF(pendingActionPress, 0x158);
GAMEINPUTMANAGER_OFF(actionPress, 0x1F4);
GAMEINPUTMANAGER_OFF(rumbleFramesRemaining, 0x290);
GAMEINPUTMANAGER_OFF(rumblePaused, 0x2a0);
GAMEINPUTMANAGER_OFF(cursorDrawingSuspendCount, 0x2b0);
GAMEINPUTMANAGER_OFF(cursorBaseColors, 0x2B4);
GAMEINPUTMANAGER_OFF(cursorActiveColors, 0x2C4);
GAMEINPUTMANAGER_OFF(cursorAlternateColors, 0x2D4);
GAMEINPUTMANAGER_OFF(cursorColors, 0x2E4);
GAMEINPUTMANAGER_OFF(cursorAppearance, 0x3B4);
GAMEINPUTMANAGER_OFF(cursorMode, 0x3c4);
GAMEINPUTMANAGER_OFF(cursorModeRequestCount, 0x3c8);
GAMEINPUTMANAGER_OFF(pointerOverrideEnabled, 0x3fc);
GAMEINPUTMANAGER_OFF(pointerOverrideX, 0x400);
GAMEINPUTMANAGER_OFF(pointerOverrideY, 0x404);
#undef GAMEINPUTMANAGER_OFF

#endif
