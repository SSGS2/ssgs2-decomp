#ifndef CLASSES_CBPCTRL_H
#define CLASSES_CBPCTRL_H

#include "game/types.h"

struct Node;

// Owns the tree of registered entries. The root also serves as the end sentinel (links only,
// never a payload); firstEntry is the least ID, or that sentinel when the map is empty.
struct MapOwner {
    int count;        // 0x0
    Node* root;       // 0x4
    Node* firstEntry; // 0x8
    // Initialised by the enclosing controller constructor.
    MapOwner() {}
    ~MapOwner();
};

// The BP controller: the table of BP objects, its drawing flag and its callback-pause state.
class CBpCtrl {
public:
    // Request (1) becomes paused (2) at DrawEntries even when drawing is disabled.
    // State 1 still permits callbacks/resets; ResumeCallbacks returns either state to 0.
    enum CallbackPauseState {
        kCallbacksRunning = 0,
        kPauseRequested = 1,
        kCallbacksPaused = 2
    };
    enum {
        kEntryIdCount = 32,
        kInvalidEntryId = -1
    };

    CBpCtrl();

    // Singleton helper: the controller's receiver is not used.
    int PollSingletonEntriesLoaded();
    // Clear all 40 controller slots, restore unused IDs and colour unity, and reset draw callbacks.
    static int InitializeObjectEntries();

    // Set the drawing flag without changing callback pause state.
    int SetDrawingEnabled(int enabled);
    // Resume callbacks by clearing the pause state.
    void ResumeCallbacks();
    // Request a callback pause, which becomes effective on the next draw pass.
    void RequestCallbackPause();
    // Poll every registered object and report whether all loads are complete.
    int PollAllEntriesLoaded();
    // Invoke object callbacks unless the requested pause has become effective.
    int InvokeEntryCallbacks(int a);
    // Apply a pending callback pause and draw every entry when drawing is enabled.
    int DrawEntries();
    // Clear each object's frame flag and, while callbacks run, its callback and sprite-vertex state.
    int ResetEntryFrameState();
    // Construct and register a BP object for a script path under a free ID.
    int CreateEntry(const void* scriptPath, void* entryIdOut, int searchFromLastId);
    // Return the first or last unused entry ID, or kInvalidEntryId.
    int FindFreeEntryId(int searchFromLastId);
    // Destroy the entry with this ID; an absent ID is a no-op.
    int DeleteEntryById(int key);

    void* vtbl;             // 0x00: original vtable pointer
    MapOwner map;           // 0x04: registered entries, keyed by ID
    int drawingEnabled;     // 0x10
    int callbackPauseState; // 0x14: a CallbackPauseState
    char pad_018[0x8C];     // 0x18
    void* unk_A4;           // 0xA4
};

#define CBPCTRL_OFF(f, o) typedef char CBpCtrl_off_##f[((int)&((CBpCtrl*)0)->f == (o)) ? 1 : -1]
CBPCTRL_OFF(map, 0x4);
CBPCTRL_OFF(drawingEnabled, 0x10);
CBPCTRL_OFF(callbackPauseState, 0x14);
CBPCTRL_OFF(unk_A4, 0xa4);
#undef CBPCTRL_OFF

#endif // CLASSES_CBPCTRL_H
