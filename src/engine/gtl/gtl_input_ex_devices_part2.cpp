// GTL extended controller input: enumerate pad/keyboard/mouse/screen-pointer
// devices, replace per-provider wrappers, configure buttons and poll samples.
// API/mod lookup:
// - Enabled device kinds: Initialize desc.w[3], bit map at InputEx_KindEnabled.
// - Controller slot policy: InputEx_ChooseSlot; exactly four slots per kind.
// - Threshold/repeat defaults: ResetButtonConfigurations / s_defaultButtonConfiguration;
//   GetButtonConfiguration gives writable 12-byte records (layout below).
// - Wii Remote sampling: SetSamplingParameter indices 0..3 call KPAD setters.
// - Per-call sampling: Update first updates the driver, enumerates, then invokes
//   s_deviceUpdateCallbacks member callbacks for nonnull wrappers. No dt conversion here.
// Types and refcount/allocator family: game/gtl_input.h; initialization and final
// driver-reference release: CInputExDeviceBase in gtl_transform_frustum_stopwatch_input.cpp.
// All globals/caches here require single-threaded, non-reentrant use. Initialize
// once before Update. There is no shutdown/rollback operation in this unit.
// Translation unit 0x803E9C40-0x803EAC48 (29 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "game/gtl_input.h"

struct gtl_IInputExButton;

struct gtl_CKeyboard {
    char unk0[0x114];
    gtl_IInputExButton* buttons[1];
};

struct GtlCMouseButtons {
    char unk0[0x24];
    void* buttons[1];
};

struct GtlCPadButtonsView {
    char unk0[0xC94];
    void* buttons[1];
};

struct GtlCScreenPointerVf3 {
    char unk0[0xD54];
    void* buttons[1];
};

struct GtlInputExScreenPointerDevice {
    char unk0[0xC];
    u8 connected;
};

struct GtlSamplingParameterPair {
    f32 playRadius; // +0 KPAD play radius, NOT a minimum sample value.
    f32 sensitivity; // +4 KPAD sensitivity, NOT a maximum sample value.
};

struct GtlCScreenPointerSampling {
    char unk0[0xD94];
    GtlSamplingParameterPair samplingParameters[4];
};

struct GtlInputExButtonSampleBase {
    s32 refCount;
};

// IInputExButton's virtuals as ProcessSample calls them: 0x8 the destructor, 0xC Sample, 0x10 Update
struct GtlInputExButtonSample : GtlInputExButtonSampleBase {
// The original leaves r4 (the driver sample the caller passed) untouched across the call.
    virtual ~GtlInputExButtonSample();
    virtual s32 Sample();
    virtual void Update();
};

struct SamplingParameter {
    f32 playRadius; // +0 KPAD play radius; same native pair as GtlSamplingParameterPair.
    f32 sensitivity; // +4 KPAD sensitivity; passed unchanged to the SDK.
};
// Pair storage is binary32, 8 bytes; the four pairs at +D94 are cached requested
// SDK filter settings, not min/max ranges and not returned sensor readings.
// Channel must be 0..3. Index0=acceleration, 1=IR distance, 2=horizontal vector,
// 3=IR position. Both floats pass unchanged to the corresponding KPAD setter.
// This API accepts EVERY float bit pattern (no comparison, clamp or rejection);
// SDK-supported limits and the dimensional definition of each play radius are
// absent from the available SDK declarations. In particular, do not call them
// pixels, radians, seconds or acceleration-g without the SDK implementation.
// Concrete position baseline: InputManager_Init (balloon_system.cpp) independently calls
// KPADSetPosParam on channels0..3 with {0.15f,0.9f}, defined in its data companion.
// Those calls change the same SDK channel filters without updating this cache.
// Getter therefore reports last requested local values, not SDK effective state.
// Other three families have no established baseline/range here; changing them
// safely requires recovering their SDK filter equations and testing real samples.

struct GtlCScreenPointer {
    char unk0[0x10];
    s32 channel;
    char unk14[0xD94 - 0x14];
    SamplingParameter params[4];
};

struct GtlScreenPointerSample {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
    f32 unk1C;
    s32 unk20;
};

struct GtlCScreenPointerSampleOwner {
    char unk0[8];
    void* device;
    char unkC[0xB14 - 0xC];
    GtlScreenPointerSample samples[1];
};

struct InputExInitDesc {
    u32 w[4];
};

struct InputExGlobalState {
    InputExInitDesc desc;
    void* pads[4];
    void* keyboards[4];
    void* mice[4];
    void* pointers[4];
};

struct GtlButtonConfigEntry {
    f32 threshold; // threshold, passed unchanged to SetThreshold
    s32 repeatInterval; // repeatInterval, input update calls (not seconds)
    s32 repeatDelay; // repeatDelay, input update calls (not seconds)
};

struct GtlButtonConfigRow {
    GtlButtonConfigEntry e[4];
};

// a device the input driver enumerates
struct InputExDeviceDesc {
    s32 unk0;
    s32 providerId;
    s32 index;      // the slot it asks for; 5 = any free slot
};

extern "C" {

struct ButtonConfigDefaults {
    float threshold;
    s32 repeatInterval;
    s32 repeatDelay;
};

extern char s_inputExGlobalState[];
extern ButtonConfigDefaults s_defaultButtonConfiguration;

struct GtlInputExPtmfTarget;
// The game data holds a 12-byte CodeWarrior member-function record {this adjustment, slot, code token}.
typedef void (GtlInputExPtmfTarget::*GtlInputExPtmf)(void);
extern GtlInputExPtmf s_deviceUpdateCallbacks[];

struct GtlInputExPtmfPair {
    GtlInputExPtmf a;
    GtlInputExPtmf b;
};

struct GtlInputExPtmfRaw {
    u32 w[3];
};

void InputDriver_UpdateAll(void);
void InputDriver_EnumerateDevices(s32 deviceFilter, s32 (*callback)(InputExDeviceDesc*), s32 arg, s32 arg2);
s32 gtl_InputEx_EnumeratePadCallback(InputExDeviceDesc* desc);
s32 gtl_InputEx_EnumerateKeyboardCallback(InputExDeviceDesc* desc);
s32 gtl_InputEx_EnumerateMouseCallback(InputExDeviceDesc* desc);
s32 gtl_InputEx_EnumerateScreenPointerCallback(InputExDeviceDesc* desc);

s32 gtl_IInputDevice_GetDeviceResult(void* device, s32* count);
s32 gtl_CScreenPointer_GetSample(const GtlCScreenPointerSampleOwner* self, s32 index, GtlScreenPointerSample* out);

s32 gtl_CScreenPointer_GetSamplingParameter(GtlCScreenPointerSampling* self, u32 index, GtlSamplingParameterPair* out);

void KPADSetAccParam(s32 chan, f32 play_radius, f32 sensitivity);
void KPADSetDistParam(s32 chan, f32 play_radius, f32 sensitivity);
void KPADSetHoriParam(s32 chan, f32 play_radius, f32 sensitivity);
void KPADSetPosParam(s32 chan, f32 play_radius, f32 sensitivity);
s32 gtl_CScreenPointer_SetSamplingParameter(GtlCScreenPointer* self, s32 index, SamplingParameter* param);

s32 gtl_CScreenPointer_GetButtonCount(void);

void* gtl_CScreenPointer_vfunc_3(GtlCScreenPointerVf3* self, s32 index);

s32 gtl_CScreenPointer_GetDriverStateSizeBytes(void);

s32 gtl_CMouse_GetButtonCount(void);

void* gtl_CMouse_vfunc_3(GtlCMouseButtons* self, s32 index);

s32 gtl_CMouse_GetDriverStateSizeBytes(void);

s32 gtl_CKeyboard_GetButtonCount(void);

gtl_IInputExButton* gtl_CKeyboard_vfunc_3(gtl_CKeyboard* self, s32 index);

s32 gtl_CKeyboard_GetDriverStateSizeBytes(void);

s32 gtl_CPad_GetButtonCount(void);

void* gtl_CPad_vfunc_3(GtlCPadButtonsView* self, s32 index);

s32 gtl_CPad_GetDriverStateSizeBytes(void);

void gtl_CInputExDeviceBase_GetProviderId(void* self, s32* out);
void gtl_IInputDevice_Create(void* key, void** out);
void gtl_CInputExDeviceBase_Initialize(void* self, void* params);

} // extern "C"

extern "C" InputExInitDesc* s_activeInputExInitDescriptor;


// what IInputDevice_Create opens, and what CInputExDeviceBase_Initialize gets
struct InputExDeviceKey {
    s32 providerId;
    s32 deviceIndex;
};

struct InputExDeviceInit {
    void* device;
    s32 arg;  // per-kind driver descriptor address, passed to SetDescriptor
    s32 requestedIndex; // original requested index, carried as config column ABI word
};

// Replace one REQUIRED slot and descriptor; arg is the borrowed per-kind driver
// descriptor address forwarded to Initialize. The intended reuse test asks
// GetProviderId to fill id and compares it to desc->providerId. ABI caveat:
// gtl_input_ex_devices_part1.cpp's definition currently declares a return-value-only provider query,
// while this TU declares an output parameter. Do not treat that conflict as
// resolved by these comments; reconciling it requires a separate behavior audit.
// If the comparison succeeds, return 1 with old unchanged. Otherwise drop the
// slot's owned wrapper ref; delete at zero (its dtor
// drops the underlying driver-device ref). Open key={providerId,requestedIndex}.
// Create returns a driver reference transferred to the wrapper by Initialize.
// Publish *slot BEFORE Initialize. The initializer result is ignored: return 1
// does not promise an active device, nor successful driver open. No null-created
// guard. A throwing open/new/constructor leaves the old slot address unrepaired
// (possibly dangling after the release); a failing initializer leaves the new
// partially initialized wrapper published. Callers must honor allocator/driver
// success preconditions; this is not transactional hotplug replacement.
// The resolved free slot is not written back into desc: its ORIGINAL index is
// passed to both driver open and initialization. In particular, the any-slot
// sentinel 5 must not silently be treated as a valid configuration column 0..3.
template <class Device>
static inline s32 InputEx_CreateDevice(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc) {
    void* device;
    s32 id;
    gtl::CInputExDeviceBase* old = *slot;
    if (old != 0) {
        gtl_CInputExDeviceBase_GetProviderId(old, &id);
        if (id == desc->providerId) {
            return 1;
        }
    }
    if (old != 0 && --old->m_refCount == 0) {
        delete old;
    }
    {
        InputExDeviceKey key;
        key.providerId = desc->providerId;
        key.deviceIndex = desc->index;
        device = 0;
        gtl_IInputDevice_Create(&key, &device);
        InputExDeviceInit init;
        init.device = device;
        init.arg = arg;
        init.requestedIndex = desc->index;
        Device* created = new Device;
        *slot = created;
        gtl_CInputExDeviceBase_Initialize(created, &init);
    }
    return 1;
}

// the init flags enable each device kind: 1 pads, 2 mice, 4 keyboards, 8 screen pointers
static inline u32 InputEx_KindEnabled(u32 mask) {
    InputExGlobalState* st = (InputExGlobalState*)s_activeInputExInitDescriptor;
    u32 on;
    if (st == 0) on = 0; else on = (st->desc.w[3] & mask) != 0;
    return on;
}

// Select from required slots[4]. requested must be 0..3, or 5 for first empty.
// With 5 and all occupied, returns 5; enumeration callbacks DO NOT check it and
// would form an out-of-range slot address. Thus driver enumeration must supply
// a free slot for "any", and never 4/negative/>5. Preserve this existing policy.
static inline s32 InputEx_ChooseSlot(gtl::CInputExDeviceBase** slots, s32 requested) {
    s32 slot = 5;
    if (requested == 5) {
        if (slot == 5) {
            for (u32 i = 0; i < 4; i++) {
                if (slots[i] == 0) {
                    slot = i;
                    break;
                }
            }
        }
    } else {
        slot = requested;
    }
    return slot;
}

extern "C" {

extern gtl::CInputExDeviceBase* s_padDeviceSlots[4];
extern gtl::CInputExDeviceBase* s_keyboardDeviceSlots[4];
extern gtl::CInputExDeviceBase* s_mouseDeviceSlots[4];
extern gtl::CInputExDeviceBase* s_screenPointerDeviceSlots[4];
extern char s_padAndScreenPointerInitParameters[], s_keyboardDeviceInitParameters[], s_mouseDeviceInitParameters[];

s32 gtl_InputEx_CreatePad(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);
s32 gtl_InputEx_CreateKeyboard(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);
s32 gtl_InputEx_CreateMouse(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);
s32 gtl_InputEx_CreateScreenPointer(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);

s32 gtl_InputEx_EnumeratePadCallback(InputExDeviceDesc* desc);

s32 gtl_InputEx_CreatePad(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);

s32 gtl_InputEx_EnumerateKeyboardCallback(InputExDeviceDesc* desc);

s32 gtl_InputEx_CreateKeyboard(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);

s32 gtl_InputEx_EnumerateMouseCallback(InputExDeviceDesc* desc);

s32 gtl_InputEx_CreateMouse(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);

s32 gtl_InputEx_EnumerateScreenPointerCallback(InputExDeviceDesc* desc);

s32 gtl_InputEx_CreateScreenPointer(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc);

s32 gtl_InputEx_ResetButtonConfigurations(void);

// The real signature; the original call leaves r3 = the init descriptor.
void InputDriver_InitializeSystem(void);
// Alternate four-slot initializer retained from the recovered declarations;
// Initialize below uses the explicitly unrolled sibling. Neither releases refs.
static inline void InputEx_ClearDevicePtrs(void** p) {
    int i;
    for (i = 0; i < 4; i++) {
        p[i] = 0;
    }
}
s32 gtl_InputEx_Initialize(const InputExInitDesc* desc);

s32 gtl_InputEx_Update(void);
GtlInputExScreenPointerDevice* gtl_InputEx_GetScreenPointer(s32 index);
extern s32 kGcDriverCapabilityCode;
extern s32 kInputProviderPointing;
extern s32 kNunchukDriverCapabilityCode;
extern s32 kInputProviderClassic;
GtlButtonConfigEntry* gtl_InputEx_GetButtonConfiguration(s32 provider, s32 index, s32 button);

s32 gtl_IInputExButton_ProcessSample(GtlInputExButtonSample* self);

// Copy one 0x24-byte sample, bounded by the driver's reported count. self/device
// required; invalid index or null out returns 0 with output untouched; else 1.
// The driver count must fit the 16 decoded history entries at +B14..D54.
s32 gtl_CScreenPointer_GetSample(const GtlCScreenPointerSampleOwner* self, s32 index, GtlScreenPointerSample* out) {
    void* device = self->device;
    s32 count = 0;
    gtl_IInputDevice_GetDeviceResult(device, &count);
    s32 n = count;
    if (index < 0 || index >= n) {
        return 0;
    }
    if (out == 0) {
        return 0;
    }
    *out = self->samples[index];
    return 1;
}

// Copy stored pair for index 0..3; invalid index/null out returns 0 unchanged.
// Same cached {playRadius,sensitivity} pair as the setter; not a sample range.
s32 gtl_CScreenPointer_GetSamplingParameter(GtlCScreenPointerSampling* self, u32 index, GtlSamplingParameterPair* out) {
    if (index >= 4) {
        return 0;
    }
    if (out == 0) {
        return 0;
    }
    {
        GtlSamplingParameterPair* r = &self->samplingParameters[index];
        f32 sensitivity;
        f32 playRadius;
        playRadius = r->playRadius;
        sensitivity = r->sensitivity;
        out->playRadius = playRadius;
        out->sensitivity = sensitivity;
    }
    return 1;
}

// Store caller's {play_radius,sensitivity} floats, then update KPAD on self's
// channel: 0 acceleration, 1 distance, 2 horizontal, 3 position. Required self;
// invalid index/null param returns 0 unchanged. Returns 1 after SDK call; no
// clamping or unit conversion. See SamplingParameter for baseline, unchecked
// float domain and distinction between cached requests and effective SDK state.
s32 gtl_CScreenPointer_SetSamplingParameter(GtlCScreenPointer* self, s32 index, SamplingParameter* param) {
    if ((u32)index >= 4) {
        return 0;
    }
    if (param == 0) {
        return 0;
    }
    self->params[index].playRadius = param->playRadius;
    self->params[index].sensitivity = param->sensitivity;
    switch (index) {
    case 0:
        KPADSetAccParam(self->channel, param->playRadius, param->sensitivity);
        break;
    case 1:
        KPADSetDistParam(self->channel, param->playRadius, param->sensitivity);
        break;
    case 2:
        KPADSetHoriParam(self->channel, param->playRadius, param->sensitivity);
        break;
    case 3:
        KPADSetPosParam(self->channel, param->playRadius, param->sensitivity);
        break;
    }
    return 1;
}

// Report screen-pointer button count (16); no receiver read.
s32 gtl_CScreenPointer_GetButtonCount(void) {
    return 0x10;
}

// Borrow a screen-pointer button; required self, unchecked index in [0,16).
void* gtl_CScreenPointer_vfunc_3(GtlCScreenPointerVf3* self, s32 index) {
    return self->buttons[index];
}

// Report 0xB0 bytes per driver state record. Poll (gtl_inputex_device_poll.cpp) passes this to
// IInputDevice_ReadState; this is a byte size, not a capability/provider code.
s32 gtl_CScreenPointer_GetDriverStateSizeBytes(void) {
    return 0xB0;
}

// Report mouse button count (6, including analog wheel entries).
s32 gtl_CMouse_GetButtonCount(void) {
    return 6;
}

// Borrow a mouse button; required self, unchecked index in [0,6).
void* gtl_CMouse_vfunc_3(GtlCMouseButtons* self, s32 index) {
    return self->buttons[index];
}

// Report 0x10 driver-state bytes for mouse polling (gtl_inputex_device_poll.cpp).
s32 gtl_CMouse_GetDriverStateSizeBytes(void) {
    return 0x10;
}

// Report keyboard button count (256).
s32 gtl_CKeyboard_GetButtonCount(void) {
    return 0x100;
}

// Borrow a keyboard button; required self, unchecked index in [0,256).
gtl_IInputExButton* gtl_CKeyboard_vfunc_3(gtl_CKeyboard* self, s32 index) {
    return self->buttons[index];
}

// Report 0x100 driver-state bytes for keyboard polling (gtl_inputex_device_poll.cpp).
s32 gtl_CKeyboard_GetDriverStateSizeBytes(void) {
    return 0x100;
}

// Report pad button count (50: 20 digital, 30 analog; see gtl_input.h).
s32 gtl_CPad_GetButtonCount(void) {
    return 0x32;
}

// Borrow a pad button; required self, unchecked index in [0,50).
void* gtl_CPad_vfunc_3(GtlCPadButtonsView* self, s32 index) {
    return self->buttons[index];
}

// Report 0xB0 bytes per pad driver-state record (gtl_inputex_device_poll.cpp polling ABI).
s32 gtl_CPad_GetDriverStateSizeBytes(void) {
    return 0xB0;
}

#pragma push
#pragma opt_propagation off
// Enabled pad entry -> selected slot and pad descriptor; disabled -> 1, no writes.
// Required desc; all four callbacks require the ChooseSlot preconditions above.
s32 gtl_InputEx_EnumeratePadCallback(InputExDeviceDesc* desc) {
    if (!InputEx_KindEnabled(1)) return 1;
    return gtl_InputEx_CreatePad(&s_padDeviceSlots[InputEx_ChooseSlot(s_padDeviceSlots, desc->index)], (s32)s_padAndScreenPointerInitParameters, desc);
}

// Apply InputEx_CreateDevice's ownership/failure policy with a CPad wrapper.
s32 gtl_InputEx_CreatePad(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc) {
    return InputEx_CreateDevice<gtl::CPad>(slot, arg, desc);
}

// Enabled keyboard entry -> keyboard slot/descriptor; disabled -> 1 unchanged.
s32 gtl_InputEx_EnumerateKeyboardCallback(InputExDeviceDesc* desc) {
    if (!InputEx_KindEnabled(4)) return 1;
    return gtl_InputEx_CreateKeyboard(&s_keyboardDeviceSlots[InputEx_ChooseSlot(s_keyboardDeviceSlots, desc->index)], (s32)s_keyboardDeviceInitParameters, desc);
}

// Apply InputEx_CreateDevice's ownership/failure policy with a CKeyboard wrapper.
s32 gtl_InputEx_CreateKeyboard(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc) {
    return InputEx_CreateDevice<gtl::CKeyboard>(slot, arg, desc);
}

// Enabled mouse entry -> mouse slot/descriptor; disabled -> 1 unchanged.
s32 gtl_InputEx_EnumerateMouseCallback(InputExDeviceDesc* desc) {
    if (!InputEx_KindEnabled(2)) return 1;
    return gtl_InputEx_CreateMouse(&s_mouseDeviceSlots[InputEx_ChooseSlot(s_mouseDeviceSlots, desc->index)], (s32)s_mouseDeviceInitParameters, desc);
}

// Apply InputEx_CreateDevice's ownership/failure policy with a CMouse wrapper.
s32 gtl_InputEx_CreateMouse(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc) {
    return InputEx_CreateDevice<gtl::CMouse>(slot, arg, desc);
}

// Enabled screen-pointer entry -> pointer slot/pad descriptor; disabled -> 1.
s32 gtl_InputEx_EnumerateScreenPointerCallback(InputExDeviceDesc* desc) {
    if (!InputEx_KindEnabled(8)) return 1;
    return gtl_InputEx_CreateScreenPointer(&s_screenPointerDeviceSlots[InputEx_ChooseSlot(s_screenPointerDeviceSlots, desc->index)], (s32)s_padAndScreenPointerInitParameters, desc);
}

// Apply InputEx_CreateDevice's ownership/failure policy with CScreenPointer.
s32 gtl_InputEx_CreateScreenPointer(gtl::CInputExDeviceBase** slot, s32 arg, const InputExDeviceDesc* desc) {
    return InputEx_CreateDevice<gtl::CScreenPointer>(slot, arg, desc);
}
#pragma pop

// Restore every provider/button/slot record to s_defaultButtonConfiguration defaults; return 1.
// Global table schema from s_inputExGlobalState (Wii byte offsets):
//   +0050 pointing[16][4], +0350 GC[50][4], +0CB0 Nunchuk[50][4],
//   +1610 Classic[50][4]. Entry = {f32 threshold,s32 interval,s32 delay}.
// The second dimension is controller slot, NOT button number. GetConfiguration
// uses explicit buttonRow/controllerSlot argument names; keep them distinct.
// Expanded store map: off=slot*12 for slots 0..3; inner i visits ten blocks of
// five rows (+0,+30,+60,+90,+C0, advance F0) in the three 50-row tables. The
// pointing table stores row 0 at base+off, then rows 1..15 in the following
// block. This covers all 16/50 rows exactly once without clearing pointers.
// Preserve this store order; changing to memset would destroy typed defaults.
s32 gtl_InputEx_ResetButtonConfigurations(void) {
    char* inputState = s_inputExGlobalState;
    char* gcTable = inputState + 0x350;
    char* nunchukTable = inputState + 0xCB0;
    char* classicTable = inputState + 0x1610;
    ButtonConfigDefaults defaults = s_defaultButtonConfiguration;
    float threshold = defaults.threshold;
    s32 repeatInterval = defaults.repeatInterval;
    s32 repeatDelay = defaults.repeatDelay;
    s32 slotByteOffset = 0;
    char* pointingTable = inputState + 0x50;
    s32 slotIndex = 0;

    do {
        char* gcEntry = gcTable + slotByteOffset;
        char* nunchukEntry = nunchukTable + slotByteOffset;
        char* classicEntry = classicTable + slotByteOffset;
        s32 fiveRowBlock;

        for (fiveRowBlock = 0; fiveRowBlock < 10; fiveRowBlock++) {
            *(float*)(gcEntry + 0x00) = threshold;
            *(float*)(nunchukEntry + 0x00) = threshold;
            *(float*)(classicEntry + 0x00) = threshold;
            *(s32*)(gcEntry + 0x04) = repeatInterval;
            *(s32*)(nunchukEntry + 0x04) = repeatInterval;
            *(s32*)(classicEntry + 0x04) = repeatInterval;
            *(s32*)(gcEntry + 0x08) = repeatDelay;
            *(s32*)(nunchukEntry + 0x08) = repeatDelay;
            *(s32*)(classicEntry + 0x08) = repeatDelay;
            *(float*)(gcEntry + 0x30) = threshold;
            *(float*)(nunchukEntry + 0x30) = threshold;
            *(float*)(classicEntry + 0x30) = threshold;
            *(s32*)(gcEntry + 0x34) = repeatInterval;
            *(s32*)(nunchukEntry + 0x34) = repeatInterval;
            *(s32*)(classicEntry + 0x34) = repeatInterval;
            *(s32*)(gcEntry + 0x38) = repeatDelay;
            *(s32*)(nunchukEntry + 0x38) = repeatDelay;
            *(s32*)(classicEntry + 0x38) = repeatDelay;
            *(float*)(gcEntry + 0x60) = threshold;
            *(float*)(nunchukEntry + 0x60) = threshold;
            *(float*)(classicEntry + 0x60) = threshold;
            *(s32*)(gcEntry + 0x64) = repeatInterval;
            *(s32*)(nunchukEntry + 0x64) = repeatInterval;
            *(s32*)(classicEntry + 0x64) = repeatInterval;
            *(s32*)(gcEntry + 0x68) = repeatDelay;
            *(s32*)(nunchukEntry + 0x68) = repeatDelay;
            *(s32*)(classicEntry + 0x68) = repeatDelay;
            *(float*)(gcEntry + 0x90) = threshold;
            *(float*)(nunchukEntry + 0x90) = threshold;
            *(float*)(classicEntry + 0x90) = threshold;
            *(s32*)(gcEntry + 0x94) = repeatInterval;
            *(s32*)(nunchukEntry + 0x94) = repeatInterval;
            *(s32*)(classicEntry + 0x94) = repeatInterval;
            *(s32*)(gcEntry + 0x98) = repeatDelay;
            *(s32*)(nunchukEntry + 0x98) = repeatDelay;
            *(s32*)(classicEntry + 0x98) = repeatDelay;
            *(float*)(gcEntry + 0xC0) = threshold;
            *(float*)(nunchukEntry + 0xC0) = threshold;
            *(float*)(classicEntry + 0xC0) = threshold;
            *(s32*)(gcEntry + 0xC4) = repeatInterval;
            *(s32*)(nunchukEntry + 0xC4) = repeatInterval;
            *(s32*)(classicEntry + 0xC4) = repeatInterval;
            *(s32*)(gcEntry + 0xC8) = repeatDelay;
            gcEntry += 0xF0;
            *(s32*)(nunchukEntry + 0xC8) = repeatDelay;
            nunchukEntry += 0xF0;
            *(s32*)(classicEntry + 0xC8) = repeatDelay;
            classicEntry += 0xF0;
        }

        *(float*)(pointingTable + slotByteOffset) = threshold;
        {
            char* pointingSlot = pointingTable + slotByteOffset;

            slotIndex += 1;
            slotByteOffset += 0xC;
            *(s32*)(pointingSlot + 0x04) = repeatInterval;
            *(s32*)(pointingSlot + 0x08) = repeatDelay;
            *(float*)(pointingSlot + 0x30) = threshold;
            *(s32*)(pointingSlot + 0x34) = repeatInterval;
            *(s32*)(pointingSlot + 0x38) = repeatDelay;
            *(float*)(pointingSlot + 0x60) = threshold;
            *(s32*)(pointingSlot + 0x64) = repeatInterval;
            *(s32*)(pointingSlot + 0x68) = repeatDelay;
            *(float*)(pointingSlot + 0x90) = threshold;
            *(s32*)(pointingSlot + 0x94) = repeatInterval;
            *(s32*)(pointingSlot + 0x98) = repeatDelay;
            *(float*)(pointingSlot + 0xC0) = threshold;
            *(s32*)(pointingSlot + 0xC4) = repeatInterval;
            *(s32*)(pointingSlot + 0xC8) = repeatDelay;
            *(float*)(pointingSlot + 0xF0) = threshold;
            *(s32*)(pointingSlot + 0xF4) = repeatInterval;
            *(s32*)(pointingSlot + 0xF8) = repeatDelay;
            *(float*)(pointingSlot + 0x120) = threshold;
            *(s32*)(pointingSlot + 0x124) = repeatInterval;
            *(s32*)(pointingSlot + 0x128) = repeatDelay;
            *(float*)(pointingSlot + 0x150) = threshold;
            *(s32*)(pointingSlot + 0x154) = repeatInterval;
            *(s32*)(pointingSlot + 0x158) = repeatDelay;
            *(float*)(pointingSlot + 0x180) = threshold;
            *(s32*)(pointingSlot + 0x184) = repeatInterval;
            *(s32*)(pointingSlot + 0x188) = repeatDelay;
            *(float*)(pointingSlot + 0x1B0) = threshold;
            *(s32*)(pointingSlot + 0x1B4) = repeatInterval;
            *(s32*)(pointingSlot + 0x1B8) = repeatDelay;
            *(float*)(pointingSlot + 0x1E0) = threshold;
            *(s32*)(pointingSlot + 0x1E4) = repeatInterval;
            *(s32*)(pointingSlot + 0x1E8) = repeatDelay;
            *(float*)(pointingSlot + 0x210) = threshold;
            *(s32*)(pointingSlot + 0x214) = repeatInterval;
            *(s32*)(pointingSlot + 0x218) = repeatDelay;
            *(float*)(pointingSlot + 0x240) = threshold;
            *(s32*)(pointingSlot + 0x244) = repeatInterval;
            *(s32*)(pointingSlot + 0x248) = repeatDelay;
            *(float*)(pointingSlot + 0x270) = threshold;
            *(s32*)(pointingSlot + 0x274) = repeatInterval;
            *(s32*)(pointingSlot + 0x278) = repeatDelay;
            *(float*)(pointingSlot + 0x2A0) = threshold;
            *(s32*)(pointingSlot + 0x2A4) = repeatInterval;
            *(s32*)(pointingSlot + 0x2A8) = repeatDelay;
            *(float*)(pointingSlot + 0x2D0) = threshold;
            *(s32*)(pointingSlot + 0x2D4) = repeatInterval;
            *(s32*)(pointingSlot + 0x2D8) = repeatDelay;
        }
    } while (slotIndex < 4);

    return 1;
}

// Initialize exactly four slot addresses to null; does not release old objects.
static inline void InputEx_ClearDevicePtrs4(void** p) {
    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p[3] = 0;
}
// Required desc supplies four borrowed words, copied before any callbacks run.
// Clear all 16 wrapper slots, publish descriptor, initialize driver/defaults,
// enumerate in pad/keyboard/mouse/pointer order. Returns 1 regardless of callback
// results; no rollback. Reinitializing live slots leaks their owned references.
s32 gtl_InputEx_Initialize(const InputExInitDesc* desc) {
    u32 w0;
    u32 w1;
    u32 w2;
    u32 w3;
    InputExGlobalState* st;
    InputExInitDesc* dst;

    w0 = desc->w[0];
    w1 = desc->w[1];
    w2 = desc->w[2];
    w3 = desc->w[3];
    st = (InputExGlobalState*)s_inputExGlobalState;

    InputEx_ClearDevicePtrs4(st->pads);
    InputEx_ClearDevicePtrs4(st->keyboards);
    InputEx_ClearDevicePtrs4(st->mice);
    InputEx_ClearDevicePtrs4(st->pointers);
    dst = &st->desc;
    dst->w[0] = w0;
    dst->w[1] = w1;
    dst->w[2] = w2;
    dst->w[3] = w3;
    s_activeInputExInitDescriptor = dst;

// The real signature takes the allocator callback pair (descriptor words 0 and 1).
    InputDriver_InitializeSystem();
    gtl_InputEx_ResetButtonConfigurations();
    InputDriver_EnumerateDevices(4, gtl_InputEx_EnumeratePadCallback, 0, 0);
    InputDriver_EnumerateDevices(3, gtl_InputEx_EnumerateKeyboardCallback, 0, 0);
    InputDriver_EnumerateDevices(0x12, gtl_InputEx_EnumerateMouseCallback, 0, 0);
    InputDriver_EnumerateDevices(0x1A, gtl_InputEx_EnumerateScreenPointerCallback, 0, 0);
    return 1;
}

// Poll once after Initialize: update driver -> enumerate replacements -> invoke
// per-kind member callbacks on each nonnull slot in [0,4), in kind order. Slots
// own wrappers across calls. Member-pointer table is ABI data, not vtable slots;
// duplicated p.a/p.b copies preserve its CodeWarrior representation. Returns 1
// with no aggregate error reporting; do not mutate tables/reenter from callbacks.
s32 gtl_InputEx_Update(void) {
    char* inputState = s_inputExGlobalState;
    GtlInputExPtmf* updateCallbacks = s_deviceUpdateCallbacks;

    InputDriver_UpdateAll();
    InputDriver_EnumerateDevices(4, gtl_InputEx_EnumeratePadCallback, 0, 0);
    InputDriver_EnumerateDevices(3, gtl_InputEx_EnumerateKeyboardCallback, 0, 0);
    InputDriver_EnumerateDevices(0x12, gtl_InputEx_EnumerateMouseCallback, 0, 0);
    InputDriver_EnumerateDevices(0x1A, gtl_InputEx_EnumerateScreenPointerCallback, 0, 0);

    {
        GtlInputExPtmfPair callbackPair;
        callbackPair.a = updateCallbacks[0];
        callbackPair.b = callbackPair.a;
        GtlInputExPtmfTarget** slotsEnd;
        GtlInputExPtmfTarget** slot = (GtlInputExPtmfTarget**)(inputState + 0x10);
        slotsEnd = slot + 4;

        for (; slotsEnd != slot; slot++) {
            if (*slot != 0) {
                ((*slot)->*callbackPair.b)();
            }
        }
    }
    {
        GtlInputExPtmfPair callbackPair;
        callbackPair.a = updateCallbacks[1];
        callbackPair.b = callbackPair.a;
        GtlInputExPtmfTarget** slot = (GtlInputExPtmfTarget**)(inputState + 0x20);
        GtlInputExPtmfTarget** slotsEnd = slot + 4;

        for (; slot != slotsEnd; slot++) {
            if (*slot != 0) {
                ((*slot)->*callbackPair.b)();
            }
        }
    }
    {
        GtlInputExPtmfPair callbackPair;
        callbackPair.a = updateCallbacks[2];
        callbackPair.b = callbackPair.a;
        GtlInputExPtmfTarget** slot = (GtlInputExPtmfTarget**)(inputState + 0x30);
        GtlInputExPtmfTarget** slotsEnd = slot + 4;

        for (; slot != slotsEnd; slot++) {
            if (*slot != 0) {
                ((*slot)->*callbackPair.b)();
            }
        }
    }
    {
        GtlInputExPtmfPair callbackPair;
        callbackPair.a = updateCallbacks[3];
        callbackPair.b = callbackPair.a;
        GtlInputExPtmfTarget** slot = (GtlInputExPtmfTarget**)(inputState + 0x40);
        GtlInputExPtmfTarget** slotsEnd = slot + 4;

        for (; slot != slotsEnd; slot++) {
            if (*slot != 0) {
                ((*slot)->*callbackPair.b)();
            }
        }
    }

    return 1;
}

// Borrow a connected screen pointer, or null when absent/disconnected. Caller
// must supply index 0..3 (unchecked). Borrow expires at the next replacement.
GtlInputExScreenPointerDevice* gtl_InputEx_GetScreenPointer(s32 index) {
    GtlInputExScreenPointerDevice* dev = (GtlInputExScreenPointerDevice*)s_screenPointerDeviceSlots[index];
    if (dev != 0 && dev->connected) {
        return dev;
    }
    return 0;
}

#pragma push
#pragma opt_propagation off
// Borrow mutable config: provider must match one of the four named global codes;
// unsupported -> null. buttonRow is pointing0..15, otherwise0..49;
// controllerSlot is0..3; both unchecked. Keyboard/mouse provider codes are not
// handled here. Writes affect later initialization, not already-configured
// objects; Reset overwrites them. Return address lives in static global storage.
// Address = tableBase + buttonRow*48 + controllerSlot*12, not slot-major.
// Example: pointing row2/slot3 -> global+50+2*30+3*C = global+D4 (hex).
// Enumeration's any-slot sentinel5 is NOT a valid column. See CreateDevice's
// existing mismatch: it forwards requested index rather than the resolved slot.
GtlButtonConfigEntry* gtl_InputEx_GetButtonConfiguration(s32 provider, s32 buttonRow, s32 controllerSlot) {
    char* lb = s_inputExGlobalState;
    if (provider == kGcDriverCapabilityCode) {
        u32 table = (u32)(lb + 0x350);
        buttonRow *= 0x30;
        controllerSlot *= 12;
        return (GtlButtonConfigEntry*)(controllerSlot + (buttonRow + table));
    }
    if (provider == kInputProviderPointing) {
        u32 table = (u32)(lb + 0x50);
        buttonRow *= 0x30;
        controllerSlot *= 12;
        return (GtlButtonConfigEntry*)(controllerSlot + (buttonRow + table));
    }
    if (provider == kNunchukDriverCapabilityCode) {
        u32 table = (u32)(lb + 0xCB0);
        buttonRow *= 0x30;
        controllerSlot *= 12;
        return (GtlButtonConfigEntry*)(controllerSlot + (buttonRow + table));
    }
    if (provider == kInputProviderClassic) {
        u32 table = (u32)(lb + 0x1610);
        buttonRow *= 0x30;
        controllerSlot *= 12;
        return (GtlButtonConfigEntry*)(controllerSlot + (buttonRow + table));
    }
    return 0;
}
#pragma pop

// Required button receiver: Sample first, Update only on a nonzero result.
// Returns 0 in both cases; caller reads stored button state, not return status.
s32 gtl_IInputExButton_ProcessSample(GtlInputExButtonSample* self) {
    if (self->Sample() == 0) {
        return 0;
    }
    self->Update();
    return 0;
}

} // extern "C"
