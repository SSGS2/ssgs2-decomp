// DirectInput keyboard adapter: answers key-down queries from a snapshot of the selected
// player's mapped actions, and forwards the input provider's destroy and availability calls.

#include "game/singletons.h"
#include "classes/DirectInputKeyboard.h"
#include "classes/WProc.h"

extern "C" {

// Kinds 2 and 3 both read the same snapshot; this adapter keeps no edge history.
enum KeyboardQuery {
    kQueryKind2 = 2,
    kQueryKind3 = 3,
    kUnspecifiedKeyCode = -1
};
// A negative input-manager player selects its current player, not any keyboard key.
enum {
    kSelectedInputPlayer = -1
};
enum KeyboardKey {
    kSpaceKey = 32,
    kKey5 = 5,
    kKey4 = 4
};
enum KeyboardAction {
    kSpaceAction = 4,
    kKey5Action = 3,
    kKey4Action = 2
};
// Adapter map: exposed key 32 <- action 4, key 5 <- action 3, key 4 <- action 2.

void* memset(void*, int, unsigned long);
void* memcpy(void*, const void*, unsigned long);
void __dl__FPv(void*);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_Keyboard[];
extern char __vt__5WProc[];
void InputManager_Init(void*);
void InputManager_Dtor(void*);
void __register_global_object(void*, void (*)(void*), void*);
int Input_IsActionButtonDown(void*, int, int);

} // extern "C"

// Unregister the keyboard's WProc subobject and optionally free the keyboard.
// A non-null registry must contain this subobject exactly once and have count > 0.
// Removal moves the last entry into the vacated slot, so registry order is not stable.
// flag > 0 frees this; the returned address must then not be dereferenced.
DirectInputKeyboard* DirectInputKeyboard::destroy(int flag) {
    if (this) {
        WProc* base = &this->proc;
        if (base) {
            WProcRegistry* registry = this->proc.registry;
            this->proc.vtbl = __vt__5WProc;
            if (registry) {
                // Find the registration before replacing it with the last live entry.
                int i = 0;
                WProcEntry* entry = registry->entries;
                int count = registry->count;
                for (; i < count && entry->object != base; ++entry, ++i) {
                }
                memcpy(&registry->entries[i], &registry->entries[count - 1], sizeof(WProcEntry));
                --registry->count;
            }
        }
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}

// Refresh three cached adapter keys from the selected player's mapped actions.
void DirectInputKeyboard::poll() {
    // Clear the snapshot, then lazily construct and register the shared input manager.
    memset(&this->key32, 0, sizeof(this->key32) + sizeof(this->key5) + sizeof(this->key4));
    if (!GetInputManagerSlot()) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, InputManager_Dtor, s_inputManagerDtorRegistration_Keyboard);
            s_inputManagerInitialized = 1;
        }
        GetInputManagerSlot() = s_gameInputManager;
    }
    void* input = GetInputManagerSlot();
    if (input) {
        // Copy action state into the exposed key-code slots; no previous frame is kept.
        if (Input_IsActionButtonDown(input, kSpaceAction, kSelectedInputPlayer))
            this->key32 = 1;
        if (Input_IsActionButtonDown(input, kKey5Action, kSelectedInputPlayer))
            this->key5 = 1;
        if (Input_IsActionButtonDown(input, kKey4Action, kSelectedInputPlayer))
            this->key4 = 1;
    }
}

// Read a cached key for query kinds 2/3; all other kinds and unlisted keys return 0.
// Code -1 returns 1 unconditionally for those two kinds, even if no key is down.
int DirectInputKeyboard::isKeyDown(int kind, int code) {
    switch (kind) {
    case kQueryKind2:
    case kQueryKind3:
        if (code == kUnspecifiedKeyCode)
            return 1;
        switch (code) {
        case kSpaceKey:
            return this->key32 != 0;
        case kKey5:
            return this->key5 != 0;
        case kKey4:
            return this->key4 != 0;
        }
        return 0;
    }
    return 0;
}

// Report that the keyboard adapter is available.
int DirectInputKeyboard::vfunc_4() {
    return 1;
}

extern "C" {

// Return the WProc callback's fixed zero result; its wider meaning is unresolved.
int DirectInputKeyboard_WProc_vfunc_1() {
    return 0;
}

} // extern "C"

// Unregister a WProc with last-entry replacement, then optionally free its storage.
// A non-null registry must contain this object and have count > 0; no validation occurs.
// flag > 0 deletes this; the returned address is then only an identity value.
WProc* WProc::destroy(int flag) {
    if (this) {
        WProcRegistry* registry = this->registry;
        this->vtbl = __vt__5WProc;
        if (registry) {
            // Locate this entry; replacing it does not preserve registration order.
            int i = 0;
            WProcEntry* entry = registry->entries;
            int count = registry->count;
            for (; i < count && entry->object != this; ++entry, ++i) {
            }
            memcpy(&registry->entries[i], &registry->entries[count - 1], sizeof(WProcEntry));
            --registry->count;
        }
        if (flag > 0)
            __dl__FPv(this);
    }
    return this;
}
