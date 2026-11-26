// The name-entry screen (CNameEntryMain) with its on-screen keyboard and texture, plus small
// utilities: the hardware clock, calendar fields, the screen rectangle and init hooks.

#include <decomp/static_init.h>

#include "game/singletons.h"
#include "sdk_protos.h"
#include "classes/CNameEntryMain.h"
#include "classes/CNowReloading.h"
#include "classes/CPackTextureResource.h"

inline CNameEntryMain* AsCNameEntryMain(void* object) {
    return (CNameEntryMain*)object;
}

enum ScreenGeometry {
    kScreenWidth = 640,
    kScreenHeight = 480
};
enum HardwareTime {
    kBusClockAddress = 0x800000F8u,
    kMillisecondsDivisorMultiplier = 0x10624DD3u
};
enum ScreenAxis {
    kHorizontalAxis = 0,
    kVerticalAxis = 1
};
enum NameEntryLayout {
    kNameEntryLayoutIndex = 5,
    kInitializeKeyboardSlot = 3
};
struct OwnedAllocation {
    void* data;
    void* end;
};

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_8007D2D4;
extern float s_initializedTwoPi_8007D3F0;
extern float s_twoPi_8007D2D4_805E27C8;

extern void OSReport(const char*, ...);
extern const char s_messageBoxReportText[];

extern unsigned long long __div2i(unsigned long long, unsigned long long);
extern void OSTicksToCalendarTime(unsigned long long, int*);
extern unsigned int vsprintf(void*, const void*, unsigned int);
extern void* memset(void*, int, unsigned int);
extern void* memcpy(void*, const void*, unsigned int);
extern void appFreeUni__FPv(void*);
extern void __dl__FPv(void*);
extern void* MPBase1_Ctor(void*);
extern void SaveData_InitPlayerRecord(void*);
extern void* __nw__FUl(unsigned int);
extern void* ResourceTable_FindEntryByName(void*, const char*, int*);
extern void* SoftKeyboard_Ctor(void*, int);
extern void* Layout_Init(void*, int);
extern void MPBase1_SetAutoStart(void*, int);
extern void Allocation_Dtor(void*);
extern void SaveData_Ctor(const char*);
extern unsigned int SaveData_GetCurrentSlotIndex(void*);
extern void SaveData_InitBlankRecord(void*, int, int);
extern void* SaveData_GetCurrentRecord(void*);
extern void __dt__13CNowReloadingFv(void*);
extern void LoadingIndicator_CreatePlayer(void*);
// The deleting destructor takes the flag in r4; a base subobject is destroyed with 0 (it never frees itself).
extern void __dt__7MPBase1Fv(void*);
extern void SoftKeyboard_dtor(void*, int);
extern const char __vt__14CNameEntryMain[];
extern const char nameEntryAndStringErrorStrings[];
extern const char __vt__22CSoftKeyboardNameEntry[];
extern const char s_resourceTableDtorRegistration_NameEntry[];
extern const char s_saveData[];
extern const char s_nowReloadingDtorRegistration_804E9154[];
extern signed char s_resourceTableInitialized;
extern void* s_resourceTableStorage;
extern signed char s_saveDataInitialized;
extern signed char s_nowReloadingInitialized;
extern void* s_nowReloading;
extern float kFloat_0_MessageCursor;

// Convert the hardware time base to milliseconds using the bus-clock frequency.
unsigned int OS_GetTimeMilliseconds() {
    unsigned long long time = OSGetTime();
    unsigned int busClockHz = *(volatile unsigned int*)kBusClockAddress;
    unsigned int scaledClock = __mulhwu(kMillisecondsDivisorMultiplier, busClockHz >> 2);
    unsigned long long divisor = scaledClock >> 6;
    return __div2i(time, divisor);
}

// Release an owned allocation and optionally delete its two-pointer wrapper.
void* dtor_8007D2D4(void* self, int deallocate) {
    if (self) {
        ((OwnedAllocation*)self)->end = 0;
        if (self && ((OwnedAllocation*)self)->data) {
            ((OwnedAllocation*)self)->end = 0;
            appFreeUni__FPv(((OwnedAllocation*)self)->data);
        }
        if (deallocate > 0)
            __dl__FPv(self);
    }
    return self;
}

// Write year, month (1-based), weekday, day, hour, minute, second and milliseconds as shorts.
void Calendar_GetCurrentShortFields(short* out) {
    // OSCalendarTime is 10 ints (usec included).
    int values[9];
    OSTicksToCalendarTime(OSGetTime(), values);
    out[0] = values[5];
    out[1] = values[4] + 1;
    out[2] = values[6];
    out[3] = values[3];
    out[4] = values[2];
    out[5] = values[1];
    out[6] = values[0];
    out[7] = values[8];
}

// Return the 640x480 screen extent for the requested axis, or zero for an invalid axis.
int Screen_GetDimensionByAxis(int mode) {
    int result = 0;
    switch (mode) {
    case kHorizontalAxis:
        result = kScreenWidth;
        break;
    case kVerticalAxis:
        result = kScreenHeight;
        break;
    }
    return result;
}

// Emit the fixed message-box diagnostic and report success.
int MessageBox_Report() {
    OSReport(s_messageBoxReportText);
    return 1;
}

struct Rect {
    int left;
    int top;
    int right;
    int bottom;
};

// Translate both corners of an integer rectangle.
int Rect_Offset(Rect* rect, int dx, int dy) {
    rect->left += dx;
    rect->right += dx;
    rect->top += dy;
    rect->bottom += dy;
    return 1;
}

// Compatibility callback that always reports success.
int Compatibility_ReportSuccess() {
    return 1;
}

// Format into a zeroed temporary, copy at most size bytes, and return min(size, formatted length).
// Only the copy is bounded: vsprintf must fit its output and NUL in buffer[0x800].
// destination needs size writable bytes; strncpy pads short output, but does not
// guarantee a terminator when the formatted length is at least size.
unsigned int String_VFormatBoundedCopy(void* destination, unsigned int size, const void* format,
                                       unsigned int argumentsAddress) {
    char buffer[0x800];
    memset(buffer, 0, sizeof(buffer));
    unsigned int length = vsprintf(buffer, format, argumentsAddress);
    strncpy((char*)destination, buffer, size);
    if (size > length)
        size = length;
    return size;
}

// Initialize the first writable two-pi copy registered by this unit.
void NameEntryUnit_InitFirstTwoPi() {
    s_twoPi_8007D2D4 = kFloat_6_2831855;
}

// Initialize the second writable two-pi copy registered by this unit.
void NameEntryUnit_InitSecondTwoPi() {
    s_initializedTwoPi_8007D3F0 = kFloat_6_2831855;
}

// Return the low 32 bits of the hardware time-base counter.
unsigned int OS_GetTimeLow() {
    return OSGetTime();
}

// Write the full-screen rectangle as left, top, right, bottom.
int Rect_GetScreenBounds(int* out) {
    out[0] = 0;
    out[1] = 0;
    out[2] = kScreenWidth;
    out[3] = kScreenHeight;
    return 1;
}

// Initialize the third writable two-pi copy registered by this unit.
void NameEntryUnit_InitThirdTwoPi() {
    s_twoPi_8007D2D4_805E27C8 = kFloat_6_2831855;
}

} // extern "C"

void* operator new(size_t);
void operator delete(void*) throw();

struct KeyboardLayout {
    char data[0x5c];
    KeyboardLayout(int n) {
        Layout_Init(this, n);
    }
};

struct NameEntryTextureResource {
    char data[0x20];
    NameEntryTextureResource(const char* name, int textureNamesAddress, void* textureCountWord) {
        ((CPackTextureResource*)this)
            ->Init((const char*)name, (void**)textureNamesAddress, (unsigned int)textureCountWord);
    }
};

struct SoftKeyboardBase {
    void* vtable;
    char pad04[0xA8];
    SoftKeyboardBase() {
        SoftKeyboard_Ctor(this, 0);
    }
    ~SoftKeyboardBase() {
        SoftKeyboard_dtor(this, -1);
    }
};

struct NameEntryKeyboard {
    void* vtable;
    char pad04[0xA8];
    unsigned char flagAC;
    char padAD[3];
    int valueB0;
    char padB4[4];
    float valueB8;
    float valueBC;
    int valueC0;
    void* children[4];
    NameEntryKeyboard() {
        SoftKeyboard_Ctor(this, 0);
        vtable = (void*)__vt__22CSoftKeyboardNameEntry;
        float zero = kFloat_0_MessageCursor;
        valueB0 = -1;
        flagAC = 0;
        valueB8 = zero;
        valueBC = zero;
        valueC0 = 0;
        children[0] = new KeyboardLayout(kNameEntryLayoutIndex);
        children[1] = new KeyboardLayout(kNameEntryLayoutIndex);
        children[2] = new KeyboardLayout(kNameEntryLayoutIndex);
        children[3] = new KeyboardLayout(kNameEntryLayoutIndex);
    }
};

// Preserve this word/halfword/byte copy view: its assignments copy the 0x828-byte save record exactly.
struct PlayerRecordCopyWords {
    unsigned int words0[151];
    unsigned int words25c[28];
    unsigned short shorts2cc[208];
    unsigned int words46c[16];
    unsigned char bytes4ac[24];
    unsigned char b4c4, b4c5, b4c6, b4c7, b4c8, b4c9;
    unsigned char bytes4ca[8];
    unsigned char b4d2, b4d3, b4d4;
    unsigned int words4d8[5];
    unsigned short shorts4ec[414];
};

struct NameEntryBase {
    char data[0x20];
    NameEntryBase() {
        MPBase1_Ctor(this);
    }
    ~NameEntryBase() {
        __dt__7MPBase1Fv(this);
    }
};

inline void* operator new(size_t, void* storage) {
    return storage;
}

struct NameEntryConstruction : NameEntryBase {
    void* object;
    PlayerRecordCopyWords config;
    void* dialog;
    void* state;
    void* resource;
    void initialize();
};

// Build the keyboard and texture resource, reset the current save slot, and start the loading indicator.
inline void NameEntryConstruction::initialize() {
    void* self = this;
    MPBase1_Ctor(this);
    object = 0;
    *(void**)self = (void*)__vt__14CNameEntryMain;
    SaveData_InitPlayerRecord(AsCNameEntryMain(self)->pendingPlayerRecord);
    void* resourceTable = Sing_805DC620();
    resource = 0;
    int textureNamesAddress = 0;
    if (!resourceTable) {
        if (!s_resourceTableInitialized) {
            s_resourceTableStorage = 0;
            __register_global_object(&s_resourceTableStorage, (void*)Allocation_Dtor,
                                     (void*)s_resourceTableDtorRegistration_NameEntry);
            s_resourceTableInitialized = 1;
        }
        Sing_805DC620() = &s_resourceTableStorage;
    }
    void* textureResource;
    {
        void* textureCountWord =
            ResourceTable_FindEntryByName(Sing_805DC620(), nameEntryAndStringErrorStrings, &textureNamesAddress);
        textureResource =
            new NameEntryTextureResource(nameEntryAndStringErrorStrings, textureNamesAddress, textureCountWord);
    }
    resource = textureResource;
    NameEntryKeyboard* created = new NameEntryKeyboard;
    object = created;
    ((void (**)(void*))(*(void**)created))[kInitializeKeyboardSlot](created);
    MPBase1_SetAutoStart(self, 1);
    void* saveData = GetSaveDataSlot();
    textureResource = 0;
    AsCNameEntryMain(self)->dialog = 0;
    AsCNameEntryMain(self)->state = 0;
    if (!saveData) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    unsigned int slotIndex = SaveData_GetCurrentSlotIndex(GetSaveDataSlot());
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    SaveData_InitBlankRecord(GetSaveDataSlot(), slotIndex, 0);
    slotIndex = 0;
    if (!GetSaveDataSlot()) {
        if (!s_saveDataInitialized) {
            SaveData_Ctor(s_saveData);
            s_saveDataInitialized = 1;
        }
        GetSaveDataSlot() = (void*)s_saveData;
    }
    void* currentRecord = SaveData_GetCurrentRecord(GetSaveDataSlot());
    config = *(const PlayerRecordCopyWords*)currentRecord;
    if (!Sing_805DC610()) {
        if (!s_nowReloadingInitialized) {
            ((CNowReloading*)(&s_nowReloading))->Init();
            __register_global_object(&s_nowReloading, (void*)__dt__13CNowReloadingFv,
                                     (void*)s_nowReloadingDtorRegistration_804E9154);
            s_nowReloadingInitialized = 1;
        }
        Sing_805DC610() = &s_nowReloading;
    }
    LoadingIndicator_CreatePlayer(Sing_805DC610());
}

extern "C" {

// Construct the name-entry screen in caller-provided storage.
CNameEntryMain::CNameEntryMain() {
    ((NameEntryConstruction*)this)->initialize();
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462010", _ctors_u8007D2D4_0, NameEntryUnit_InitFirstTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462010", _ctors_u8007D2D4_1, NameEntryUnit_InitSecondTwoPi);
STATIC_INIT_ENTRY(".CRT$XCU80462010", _ctors_u8007D2D4_2, NameEntryUnit_InitThirdTwoPi);
#pragma pop
