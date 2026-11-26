// DirectInput mouse: polls pointer position and button state and reports button and axis values.
// Also holds the interface-pointer forwarding functions for its input object and small XML
// name and entity parsing helpers.

#include <decomp/static_init.h>

#include "classes/DirectInputMouseState.h"
#include "classes/WProc.h"
// Keep this accessor set: the public header omits a static emitted in the baseline object.
#include "game/singletons_baseline.h"
extern "C" {

int DirectInputKeyboard_WProc_vfunc_1(void* p);
void* destroy__19DirectInputKeyboardFi(void* p, int flag);
int DirectInputMouse_GetEventTime(void* p, int x);
void* DirectInputKeyboard_GetWProc(void* p);
int DirectInputMouse_GetAxisValue(void* p, int a, int b);
void DirectInputMouse_PollPosition(void* p);
void* DirectInputMouse_dtor(void* p, int flag);
void InputManager_Init(const char* name);
int InputManager_GetPointerPosition(void* p, float* out, int index);
unsigned int OS_GetTimeMilliseconds();
void TiXmlString_AssignCStr(const char* p, const char* name);
void TiXmlString_Append(const char* p, const char* chars);
int strncmp(const char* pattern, const char* text, int count);
extern signed char s_inputManagerInitialized;
extern char s_gameInputManager[];
extern char s_inputManagerDtorRegistration_mouse[];
extern const char tinyXmlParserStrings[];
struct LocaleCharacterTables {
    char prefix[8];
    unsigned short* classification;
    char reserved[4];
    unsigned char* lowercase;
};
struct LocaleData {
    char prefix[0x38];
    LocaleCharacterTables* characters;
};
extern LocaleData s_cLocaleData;
struct XmlEntity {
    const char* text;
    int length;
    unsigned char value;
    char padding[3];
};
enum XmlParsing {
    kXmlEntityCount = 5,
    kAlphabeticMask = 1,
    kNameCharacterMask = 9
};
enum MouseInput {
    kButtonCount = 4,
    kRelativeAxis = 0,
    kButtonAxis = 1,
    kAbsoluteAxis = 3
};
enum MouseCoordinate {
    kMouseX = 1,
    kMouseY = 2,
    kMouseWheel = 3
};
// ABI map: mouse input interface = owner+8; keyboard WProc interface = owner+12.
// Thunks receive those embedded interfaces and must subtract the offset exactly
// once. Numbered vfunc names are shared ABI symbols; their bodies document behavior.
// GetEventTime exposes the first bank; button-axis queries expose the second bank.
struct MouseButtonArrays {
    int queryStates[kButtonCount];
    int buttonStates[kButtonCount];
};
struct MouseButtonView {
    char prefix[0x14];
    MouseButtonArrays buttons;
};
struct KeyboardWindowProcView {
    char prefix[0xC];
    WProc windowProcedure;
};
extern const float kFloat_0_DirectInputMouse;
extern void InputManager_Dtor();
extern void __register_global_object(void*, void*, void*);
extern const float kFloat_6_2831855;
extern float s_twoPi_802A0698;
extern float s_twoPi_802A0A78;
#pragma inline_depth(1)
#pragma auto_inline off
// Return the mouse interface owner unchanged.
void* DirectInputMouse_ExternProc(void* p) {
    return p;
}
#pragma inline_depth()

// Report that the mouse interface is available.
int DirectInputMouse_InitDevice(void* p) {
    return 1;
}

// Return the default zero result for this input hook.
int DirectInputMouse_WinProc() {
    return 0;
}

// Initialize the companion input unit's writable two-pi cache.
void DirectInputMouse_InitCompanionTwoPiCache() {
    s_twoPi_802A0A78 = kFloat_6_2831855;
}

#pragma inline_depth(1)
#pragma auto_inline off
// Adjust the interface pointer to its owning input object and forward the call.
void* DirectInputMouse_vfunc_3_Thunk8(void* p) {
    return DirectInputMouse_ExternProc((void*)((char*)p - 0x8));
}
#pragma inline_depth()

// Adjust the interface pointer to its owning input object and forward the call.
int DirectInputKeyboard_WProc_vfunc_1_Thunk12(void* p) {
    return DirectInputKeyboard_WProc_vfunc_1((void*)((char*)p - 0xc));
}

// Adjust the interface pointer to its owning input object and forward the call.
void* DirectInputKeyboard_DtorThunk12(void* p, int flag) {
    return destroy__19DirectInputKeyboardFi((void*)((char*)p - 0xc), flag);
}

// Adjust the interface pointer to its owning input object and forward the call.
int DirectInputMouse_GetEventTimeThunk8(void* p, int x) {
    return DirectInputMouse_GetEventTime((void*)((char*)p - 0x8), x);
}

// Adjust the interface pointer to its owning input object and forward the call.
int DirectInputMouse_GetAxisValueThunk8(void* p, int queryKind, int coordinate) {
    return DirectInputMouse_GetAxisValue((void*)((char*)p - 0x8), queryKind, coordinate);
}

// Adjust the interface pointer to its owning input object and forward the call.
void DirectInputMouse_PollPositionThunk8(void* p) {
    DirectInputMouse_PollPosition((void*)((char*)p - 0x8));
}

// Return the embedded WProc interface, preserving a null owner as null.
void* DirectInputKeyboard_GetWProc(void* p) {
    if (p == 0)
        return p;
    return &((KeyboardWindowProcView*)p)->windowProcedure;
}

// Sample pointer position, accumulate integer deltas, and timestamp any movement.
void DirectInputMouse_PollPosition(void* p) {
    char* self = (char*)p;
    int changed = 0;
    float values[2];
    AsDirectInputMouseState(self)->changed = 0;
    if (((unsigned int&)GetInputManagerSlot()) == 0) {
        if (!s_inputManagerInitialized) {
            InputManager_Init(s_gameInputManager);
            __register_global_object(s_gameInputManager, (void*)InputManager_Dtor,
                                     s_inputManagerDtorRegistration_mouse);
            s_inputManagerInitialized = 1;
        }
        ((unsigned int&)GetInputManagerSlot()) = (unsigned int)s_gameInputManager;
    }
    if (InputManager_GetPointerPosition((void*)((unsigned int&)GetInputManagerSlot()), values, -1)) {
        if (AsDirectInputMouseState(self)->positionX != values[0]) {
            AsDirectInputMouseState(self)->deltaX = (int)(values[0] - AsDirectInputMouseState(self)->positionX);
            changed = 1;
        }
        if (AsDirectInputMouseState(self)->positionY != values[1]) {
            AsDirectInputMouseState(self)->deltaY = (int)(values[1] - AsDirectInputMouseState(self)->positionY);
            changed = 1;
        }
        AsDirectInputMouseState(self)->positionX = values[0];
        AsDirectInputMouseState(self)->positionY = values[1];
    }
    if (changed) {
        AsDirectInputMouseState(self)->changed = 1;
        AsDirectInputMouseState(self)->lastChangeTime = OS_GetTimeMilliseconds();
    }
}

// Read an XML name with locale-aware letter/digit tests and XML punctuation.
void* TiXmlBase_ReadName(signed char* cur, const char* nameString) {
    TiXmlString_AssignCStr(nameString, tinyXmlParserStrings);
    // Original 0x802A0698 unit: invalid starts share the null result.
    do {
        if (cur == 0)
            break;
        int character = (unsigned char)*cur;
        if ((signed char)character == 0)
            break;
        if (((character < 0 || character >= 0x100)
                 ? 0
                 : (s_cLocaleData.characters->classification[character] & kAlphabeticMask)) == 0) {
            if ((signed char)character != '_')
                break;
        }
        char pair[2];
        while (cur != 0 && (signed char)(character = (unsigned char)*cur) != 0 &&
               (((character < 0 || character >= 0x100)
                     ? 0
                     : (s_cLocaleData.characters->classification[character] & kNameCharacterMask)) != 0 ||
                (signed char)character == '_' || (signed char)character == '-' || (signed char)character == ':')) {
            pair[0] = (char)character;
            pair[1] = 0;
            TiXmlString_Append(nameString, pair);
            ++cur;
        }
        return (void*)cur;
    } while (0);
    return 0;
}

// Fold a signed locale character, leaving bytes outside the table range unchanged.
inline int FoldLocaleCharacter(int character) {
    return (character < 0 || character >= 0x100) ? character : (int)s_cLocaleData.characters->lowercase[character];
}
// Preserve repeated signed-byte loads: value/reference inline forms change the comparison loop.
#define FoldLocaleByte(tables, character)                                                 \
    (((signed char)(unsigned char)(character) < 0 || (unsigned char)(character) >= 0x100) \
         ? (int)(signed char)(unsigned char)(character)                                   \
         : (int)(tables)->lowercase[(unsigned char)(character)])

// Compare the prefix using the original mode convention: nonzero selects exact case.
// input is the input, prefix is the prefix (both NUL-terminated); prefix must be non-null.
// Return 1 only after the initial folded gate matches and prefix is exhausted; else 0.
// Empty input fails; an empty prefix does not bypass the initial gate. That gate
// also runs in exact-case mode; nonzero caseSensitive then requires byte-exact case.
int TiXml_MatchesPrefix(const signed char* input, const signed char* prefix, int caseSensitive) {
    if (input == 0 || *input == 0)
        return 0;
    signed char sa_signed = *input;
    signed char sb_signed = *prefix;
    int inputByte = (unsigned char)*input;
    int prefixByte = (unsigned char)*prefix;
    int prefixOutOfRange = (sb_signed < 0) || (prefixByte >= 0x100);
    int foldedPrefix = prefixOutOfRange ? sb_signed : (int)s_cLocaleData.characters->lowercase[prefixByte];
    int inputOutOfRange = (sa_signed < 0) || (inputByte >= 0x100);
    int foldedInput = inputOutOfRange ? sa_signed : (int)s_cLocaleData.characters->lowercase[inputByte];
    // Original 0x802A0E68: both comparison modes share the failure return.
    do {
        if (foldedInput != foldedPrefix)
            break;
        if (caseSensitive) {
            while (*input != 0 && *prefix != 0 && *input == *prefix) {
                ++input;
                ++prefix;
            }
            if (*prefix != 0)
                break;
            return 1;
        }
        {
            LocaleCharacterTables* base = s_cLocaleData.characters;
            while (*input != 0 && *prefix != 0 && FoldLocaleByte(base, *input) == FoldLocaleByte(base, *prefix)) {
                ++input;
                ++prefix;
            }
        }
        if (*prefix != 0)
            break;
        return 1;
    } while (0);
    return 0;
}

// Initialize the mouse unit's writable two-pi cache.
void DirectInputMouse_InitTwoPiCache() {
    s_twoPi_802A0698 = kFloat_6_2831855;
}

#pragma inline_depth(1)
#pragma auto_inline off
// Adjust the interface pointer to its owning input object and forward the call.
int DirectInputMouse_vfunc_4_Thunk8(void* p) {
    return DirectInputMouse_InitDevice((void*)((char*)p - 0x8));
}
#pragma inline_depth()

// Adjust the interface pointer to its owning input object and forward the call.
void* DirectInputMouse_DtorThunk8(void* p, int flag) {
    return DirectInputMouse_dtor((void*)((char*)p - 0x8), flag);
}

// Read one of the four query-state slots, or zero for an invalid button index.
int DirectInputMouse_GetEventTime(void* p, int buttonIndex) {
    int queryState = 0;
    if (buttonIndex >= 0 && buttonIndex < kButtonCount) {
        queryState = ((MouseButtonView*)p)->buttons.queryStates[buttonIndex];
    }
    return queryState;
}

// Decode a two-digit hexadecimal entity or one of the five named XML entities.
void* TiXmlBase_ReadEntity(char* cur, unsigned char* out) {
    if (strncmp(tinyXmlParserStrings + 1, cur, 3) == 0) {
        signed char first = cur[3];
        if (first != 0) {
            if (cur[4] != 0) {
                *out = 0;
                {
                    int outOfRange = (first < 0) || (first >= 0x100);
                    unsigned int isAlphabetic = outOfRange ? 0 : (s_cLocaleData.characters->classification[first] & 1);
                    if (isAlphabetic)
                        *out += (FoldLocaleCharacter(first) - 0x57) << 4;
                    else
                        *out += (first - 0x30) << 4;
                }
                signed char second = cur[4];
                {
                    int outOfRange = (second < 0) || (second >= 0x100);
                    unsigned int isAlphabetic = outOfRange ? 0 : (s_cLocaleData.characters->classification[second] & 1);
                    if (isAlphabetic)
                        *out += FoldLocaleCharacter(second) - 0x57;
                    else
                        *out += second - 0x30;
                }
                return cur + 6;
            }
        }
    }
    {
        extern XmlEntity xmlEntityTableStorage[];
        XmlEntity* entity = xmlEntityTableStorage;
        int i = 0;
        for (; i < kXmlEntityCount; ++i, ++entity) {
            if (strncmp(entity->text, cur, entity->length) == 0) {
                *out = xmlEntityTableStorage[i].value;
                return cur + xmlEntityTableStorage[i].length;
            }
        }
    }
    *out = cur[0];
    return cur + 1;
}

extern char __vt__16DirectInputMouse[0x6C];
extern char __vt__5WProc[0x6C];
extern unsigned int OS_GetTimeMilliseconds();
extern void* memcpy(void* dst, const void* src, unsigned int n);
extern void __dl__FPv(void* p);

// Initialize input state, button history, movement deltas, and the change timestamp.
void* DirectInputMouse_Ctor(void* p) {
    char* self = (char*)p;
    int zero = 0;
    AsDirectInputMouseState(self)->registry = zero;
    *(void**)self = (void*)__vt__16DirectInputMouse;
    AsDirectInputMouseState(self)->inputVtable = (void*)(__vt__16DirectInputMouse + 0x10);
    AsDirectInputMouseState(self)->deltaX = zero;
    AsDirectInputMouseState(self)->deltaY = zero;
    AsDirectInputMouseState(self)->wheel = zero;
    AsDirectInputMouseState(self)->enabled = 1;
    AsDirectInputMouseState(self)->changed = 0;
    unsigned int timestamp = OS_GetTimeMilliseconds();
    AsDirectInputMouseState(self)->lastChangeTime = timestamp;
    AsDirectInputMouseState(self)->button0QueryState = zero;
    AsDirectInputMouseState(self)->button0State = zero;
    AsDirectInputMouseState(self)->button1QueryState = zero;
    AsDirectInputMouseState(self)->button1State = zero;
    AsDirectInputMouseState(self)->button2QueryState = zero;
    AsDirectInputMouseState(self)->button2State = zero;
    AsDirectInputMouseState(self)->button3QueryState = zero;
    AsDirectInputMouseState(self)->button3State = zero;
    AsDirectInputMouseState(self)->positionX = kFloat_0_DirectInputMouse;
    AsDirectInputMouseState(self)->positionY = kFloat_0_DirectInputMouse;
    return self;
}

// Read a relative axis, a current button state, or an absolute pointer coordinate.
// queryKind 0: coordinate 1/2 = integer X/Y deltas, 3 = wheel;
// queryKind 1: coordinate is a zero-based button index 0..3;
// queryKind 3: coordinate 1/2 = float X/Y truncated to int, 3 = wheel.
// Unsupported pairs return 0. Pointer-space scale is supplied by InputManager.
int DirectInputMouse_GetAxisValue(void* p, int queryKind, int coordinate) {
    char* self = (char*)p;
    switch (queryKind) {
    case kRelativeAxis:
        switch (coordinate) {
        case kMouseX:
            return AsDirectInputMouseState(self)->deltaX;
        case kMouseY:
            return AsDirectInputMouseState(self)->deltaY;
        case kMouseWheel:
            return AsDirectInputMouseState(self)->wheel;
        }
        break;
    case kButtonAxis:
        if (coordinate >= 0 && coordinate < kButtonCount)
            return ((MouseButtonView*)self)->buttons.buttonStates[coordinate];
        break;
    case kAbsoluteAxis:
        switch (coordinate) {
        case kMouseX:
            return (int)AsDirectInputMouseState(self)->positionX;
        case kMouseY:
            return (int)AsDirectInputMouseState(self)->positionY;
        case kMouseWheel:
            return AsDirectInputMouseState(self)->wheel;
        }
        break;
    }
    return 0;
}

// Remove the mouse from its WProc registry and optionally free its storage.
void* DirectInputMouse_dtor(void* p, int flag) {
    if (p != 0) {
        *(void**)p = (void*)__vt__5WProc;
        char* inner = (char*)((WProc*)p)->registry; // DirectInputMouse_dtor: WProc prefix.
        if (inner != 0) {
            int i = 0;
            char* entry = inner;
            int count = ((WProcRegistry*)inner)->count;
            while (i < count && ((WProcEntry*)entry)->object != p) {
                entry += 8;
                i++;
            }
            memcpy(inner + (i << 3), inner + ((count - 1) << 3), 8);
            ((WProcRegistry*)inner)->count = ((WProcRegistry*)inner)->count - 1;
        }
        if (flag > 0) {
            __dl__FPv(p);
        }
    }
    return p;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624AC", _ctors_u802A0698_0, DirectInputMouse_InitTwoPiCache);
STATIC_INIT_ENTRY(".CRT$XCU804624AC", _ctors_u802A0698_1, DirectInputMouse_InitCompanionTwoPiCache);
#pragma pop
