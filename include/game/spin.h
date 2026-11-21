#ifndef GAME_CLASS1_SPIN_H
#define GAME_CLASS1_SPIN_H

extern "C" const float kFloat_0_spin; // original spin zero constant

// Wii actor storage. Preserve the existing native constructor/destructor ABI;
// declaring virtual functions here would emit a second vtable.
struct NativeBase80201D18 {
    char* vtable;                  // 0x00: constructor installs __vt__5CSpin
    char pad_04[4];
    int displayPriority;           // 0x08: Wii ctor = 4
    char pad_0C[0x0C];
    NativeBase80201D18();
    ~NativeBase80201D18();
};

class CSpin : public NativeBase80201D18 {
public:
    float hudX;                  // 0x18: hitX + centerX (OnProcess/HandleMsg)
    float hudY;                  // 0x1C: hitY + centerY
    float hitX;                    // 0x20: set by OnProcess/HandleMsg
    float hitY;                    // 0x24
    float rangeX;                  // 0x28: curve range, ellipse denominator
    float rangeY;                  // 0x2C: spin range, ellipse denominator
    float centerX;                 // 0x30: OnPreLoadInit, then Reset equivalent
    float centerY;                 // 0x34
    float angle;                   // 0x38: acos result, then sin/cos
    char pad_3C[4];
    int color;                     // 0x40: -1 / 0xA0FFFFFF, returned by msg 7
    unsigned char active;          // 0x44: messages 3/4, gates OnProcess
    char pad_45[3];
    int soundPlaying;              // 0x48: Wii-only; guards Sound_StopByAlias
    char pad_4C[4];                // initialized to zero; meaning unproven

    CSpin();

    // Reset, restricted to fields proven present in the Wii version.
    void Reset() {
        float x;
        float y;
        float zero = kFloat_0_spin;
        x = centerX;
        y = centerY;
        hudY = y;
        hudX = x;
        hitX = zero;
        hitY = zero;
        color = -1;
        active = 0;
        soundPlaying = 0;
        *(int*)pad_4C = 0;
    }
};

typedef char CSpin_size_check[sizeof(CSpin) == 0x50 ? 1 : -1];
#define SPIN_OFF(member, offset) typedef char CSpin_offset_##member[((int)&((CSpin*)0)->member == offset) ? 1 : -1]
SPIN_OFF(vtable, 0x00);
SPIN_OFF(displayPriority, 0x08);
SPIN_OFF(hudX, 0x18);
SPIN_OFF(hudY, 0x1C);
SPIN_OFF(hitX, 0x20);
SPIN_OFF(hitY, 0x24);
SPIN_OFF(rangeX, 0x28);
SPIN_OFF(rangeY, 0x2C);
SPIN_OFF(centerX, 0x30);
SPIN_OFF(centerY, 0x34);
SPIN_OFF(angle, 0x38);
SPIN_OFF(color, 0x40);
SPIN_OFF(active, 0x44);
SPIN_OFF(soundPlaying, 0x48);
SPIN_OFF(pad_4C, 0x4C);
#undef SPIN_OFF

// These are the live symbol names. Keep their C ABI for existing callers and
// vtable relocations; typed parameters share the one class layout above.
extern "C" {
void CSpin_OnPreLoadInit(CSpin*);
void CSpin_OnInit(CSpin*);
void CSpin_OnProcess(CSpin*, float);
void CSpin_HandleMsg(CSpin*, const void*);
void* CSpin_dtor(CSpin*, int);
void* CSpin_GetRTTI();
}
#endif
