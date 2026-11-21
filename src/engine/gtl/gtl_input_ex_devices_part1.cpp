// Translation unit 0x803E833C-0x803E9BB8: the gtl InputEx device file (CInputExDeviceBase, CPad, CKeyboard, CMouse,
// CScreenPointer). One original file: its functions share the conversion constants kFloat_0_InputEx/kSignedIntToDoubleMagic,
// runs on to 0x803EB544 (the vtable block at 0x804DA4E4 is shared with gtl_transform_frustum_stopwatch_input.cpp, gtl_screen_pointer_reset_buttons.cpp..gtl_input_ex_button_part1.cpp).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "types.h"
#include "game/gtl_input.h"
// Controller input adaptation, not hardware polling: providers fill sample
// storage before these vfunc_6 calls; vfunc_7 runs per-button follow-up hooks.
// Mod lookup: threshold/repeat -> IInputExButton in game/gtl_input.h and its
// ProcessSample implementation; pad channel order -> CPad_vfunc_6; normalized
// screen-pointer history -> CScreenPointer_vfunc_6. Mouse relative motion
// and clamp bounds -> gtl_CMouse_AdaptRelativeSample / CMouse constructor; query formats
// and missing-key output -> gtl_CInputExDeviceBase_ReadItemValueOrRange below. This generic library does
// not establish stock support for Classic Controller or other optional devices.
enum InputSampleLayout {
    kPadDigitalButtonCount = 20, kPadAnalogButtonCount = 30,
    kPadLogicalButtonCount = 50, kPointerButtonCount = 16,
    kProviderSampleStride = 0xB0, kPointerHistoryOffset = 0xB14
};

// ---- 0x803E833C ------------------------------------------------------------------------------------------

union UpdateButtonsValue {
    s32 i;
    f32 f;
};

struct UpdateButtonsSample {
    s32 flag;
    void* value;
};

struct UpdateButtonsPad {
    char pad[4];
};

struct UpdateButtonsDevice : UpdateButtonsPad {
    virtual void unk0() = 0;
    virtual void unk1() = 0;
    virtual void unk2() = 0;
    virtual void* GetButton(s32 i) = 0;
    virtual void unk4() = 0;
    virtual s32 GetIndex(s32 i) = 0;
};

struct UpdateButtonsButton : UpdateButtonsPad {
    virtual void unk0() = 0;
    virtual void unk1() = 0;
    virtual void unk2() = 0;
    virtual void unk3() = 0;
    virtual s32 GetResult() = 0;
};

extern "C" {

extern const f32 kFloat_0_InputEx;
extern const f64 kSignedIntToDoubleMagic;

int gtl_CInputExDeviceBase_ReadItemValueOrRange(void* self, s32 type, s32 index, void* out);
void gtl_IInputExButton_ProcessSample(void* button, UpdateButtonsSample* sample);

// Dispatch one provider word to each logical button [0,count). Required self,
// count>=0 within device button capacity, and buf[count] readable native s32s.
// GetButton(vslot3) returns a borrowed nonnull button; its GetResult(vslot4)
// MUST be 0 (raw digital word) or 1 (analog). Base vfunc_4 returns 2, which is
// unsupported here and leaves scratch flag/value unspecified. GetIndex(slot5)
// maps a logical analog button to provider range-item ID. Parameter query type1
// supplies two floats [min,max]; only max is used. Positive max -> signed word
// / max, otherwise zero; no min subtraction, absolute value or [0,1] clamp.
// ProcessSample borrows the stack union through {tag 0=int,1=float; &value}; it
// must consume synchronously, never retain its address. It updates button state
// (down/pressed/repeat/released), not buf. Returns 1 regardless of query status;
// no per-button failure is propagated. No hardware read or allocation here.
s32 gtl_CInputExDeviceBase_UpdateButtons(void* self, s32* buf, s32 count) {
    UpdateButtonsValue value;
    f32 param[2];
    UpdateButtonsSample sample;
    s32* p;
    u32 i;
    s32 flag;
    void* button;
    s32 res;
    s32 index;
    f32 scale;

    p = buf;
    for (i = 0; i < (u32)count; i++) {
        button = ((UpdateButtonsDevice*)self)->GetButton(i);
        res = ((UpdateButtonsButton*)button)->GetResult();
        switch (res) {
        case 0:
            flag = 0;
            value.i = *p;
            break;
        case 1:
            index = ((UpdateButtonsDevice*)self)->GetIndex(i);
            gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, index, param);
            if (param[1] > kFloat_0_InputEx) {
                scale = (f32)*p / param[1];
            } else {
                scale = kFloat_0_InputEx;
            }
            value.f = scale;
            flag = 1;
            break;
        }
        sample.flag = flag;
        sample.value = &value;
        gtl_IInputExButton_ProcessSample(button, &sample);
        p++;
    }
    return 1;
}

} // extern "C"


// ---- 0x803E848C ------------------------------------------------------------------------------------------

extern "C" {

// Base sample-kind hook returns unsupported tag 2. UpdateButtons requires
// concrete button overrides returning 0 (digital) or 1 (analog); self is unused.
int gtl_IInputExButton_GetSampleType(void* self) {
    return 2;
}

} // extern "C"


// ---- 0x803E8494 ------------------------------------------------------------------------------------------

struct InputRangeQueryResult {
    u32 templateWord0;
    u32 templateWord4;
    u32 itemKey;
    u32 lookupKind;
    u32 value;
    u32 rangeMaximum;
};

struct InputScalarQueryResult {
    u32 templateWord0;
    u32 templateWord4;
    u32 itemKey;
    u32 lookupKind;
    u32 value;
};

extern "C" {

extern InputScalarQueryResult s_inputScalarQueryTemplate;
extern InputRangeQueryResult s_inputRangeQueryTemplate;
extern const f64 kUnsignedIntToDoubleMagic;
extern const f32 kInputScalarValueDivisor;
extern const f64 kSignedIntToDoubleMagic;

int gtl_IInputDevice_ReadItemDataByKind(void* device, s32 valueKind, void* queryAndValue);

// Read a provider's current button/axis word or its range, without polling.
// Required self with borrowed driver at +0x08, installed descriptor, refreshed
// provider channel and valid item key. out is required, writable and disjoint
// from self; it covers one float for kinds 0/2 and two for kind 1.
// kind 0 = button, 2 = axis: unsigned word / 10000.0f (kInputScalarValueDivisor), no clamp.
// kind 1 = range: signed minimum/maximum converted to floats, no rescaling.
// Other kinds write nothing. These are provider units, not pixels or seconds.
// Local request/result schema (native u32): +0x00/+0x04 copied template words
// (20/16 for scalar, 24/16 for range), unused by this resolver; +0x08 item key;
// +0x0C lookup kind=1 (exact-key lookup, not an ordinal); +0x10 result/minimum;
// +0x14 maximum for range. Both templates initialize result words to zero.
// gtl_IInputDevice_ResolveItemHandle / ReadItemDataByKind in gtl_texture_projection_input_device.cpp preserve
// the query prefix. A missing key returns 0 without changing result words:
// this wrapper discards that status, writes zero output(s), and still returns 1.
// Successful reads borrow the provider payload synchronously; retain nothing.
int gtl_CInputExDeviceBase_ReadItemValueOrRange(void* self, s32 valueKind, s32 itemKey, void* out) {
    InputScalarQueryResult scalarQuery = s_inputScalarQueryTemplate;
    scalarQuery.itemKey = itemKey;
    InputRangeQueryResult rangeQuery = s_inputRangeQueryTemplate;
    f32* output = (f32*)out;
    rangeQuery.itemKey = itemKey;
    switch (valueKind) {
    case 0:
        gtl_IInputDevice_ReadItemDataByKind(*(void**)((u8*)self + 8), valueKind, &scalarQuery);
        output[0] = (f32)scalarQuery.value / kInputScalarValueDivisor;
        break;
    case 2:
        gtl_IInputDevice_ReadItemDataByKind(*(void**)((u8*)self + 8), valueKind, &scalarQuery);
        output[0] = (f32)scalarQuery.value / kInputScalarValueDivisor;
        break;
    case 1:
        gtl_IInputDevice_ReadItemDataByKind(*(void**)((u8*)self + 8), valueKind, &rangeQuery);
        output[0] = (f32)(s32)rangeQuery.value;
        output[1] = (f32)(s32)rangeQuery.rangeMaximum;
        break;
    }
    return 1;
}

} // extern "C"


// ---- 0x803E8618 ------------------------------------------------------------------------------------------

extern "C" {

struct gtl_CInputExDeviceBaseHead {
    unsigned int head;
};

struct gtl_CInputExDeviceBase : gtl_CInputExDeviceBaseHead {
// The original passes the caller's button ordinal (r4) through to slot 3 untouched.
    virtual void f0();
    virtual void f1();
    virtual void f2();
    virtual gtl_CInputExDeviceBase* f3();
    unsigned int pad[3];
    unsigned int flags;
};

// Read bit 0 (down) from the required nonnull button returned by receiver
// virtual slot 3. This local ABI view supplies no explicit button ordinal.
// Borrow receiver/result; do not sample hardware, mutate state or retain it.
unsigned int gtl_CInputExDeviceBase_GetButtonDown(gtl_CInputExDeviceBase* self) {
    gtl_CInputExDeviceBase* p = self->f3();
    return p->flags & 1;
}

} // extern "C"


// ---- 0x803E864C ------------------------------------------------------------------------------------------

struct GtlInputExHeader {
    int unk0;
};

struct GtlInputExState {
    char reserved00[0x14];
    unsigned int flags;
};

struct GtlInputExDeviceBase : GtlInputExHeader {
    virtual void vfunc0();
    virtual void vfunc1();
    virtual void vfunc2();
    virtual GtlInputExState* vfunc3();
    virtual void vfunc4();
    virtual void vfunc5();
};

extern "C" {

// Read bit 1 (new press) from the required nonnull slot-3 button; same
// receiver/borrowed-result contract as GetButtonDown, with no sampling/update.
int gtl_CInputExDeviceBase_GetButtonPressed(GtlInputExDeviceBase* self) {
    return (self->vfunc3()->flags >> 1) & 1;
}

} // extern "C"


// ---- 0x803E8680 ------------------------------------------------------------------------------------------

struct CPad;

struct CPadSample {
    s32 axis[6];
    char unk18[0x8];
    s32 povHatAngle;
    char unk24[0xC];
    s8 buttons[0x10];
    char unk40[0x10];
    s32 extensionWord00;
    s32 extensionWord01;
    s32 extensionWord02;
    s32 extensionWord03;
    s32 extensionWord04;
    s32 extensionWord05;
    char reserved68[0x8];
    s32 extensionWord06;
    s32 extensionWord07;
    s32 extensionWord08;
    s32 extensionWord09;
    s32 extensionWord10;
    s32 extensionWord11;
    char unk88[0x8];
    s32 extensionWord12;
    s32 extensionWord13;
    s32 extensionWord14;
    s32 extensionWord15;
    s32 extensionWord16;
    s32 extensionWord17;
};

struct CPadBase {
    s32 unk0;
};

struct CPad : CPadBase {
    virtual void vfunc_0();
    virtual CPadSample* vfunc_1();
};

struct CInputExDeviceBaseGDR {
    char unk0[0x8];
    void* device;
};

struct CInputExDeviceBaseProv {
    char unk0[0x8];
    void* device;
};

struct gtl_IInputExButton {
    virtual void vfunc_0();
};

// The game data holds a 12-byte CodeWarrior member-function record {this adjustment, slot, code token}.
typedef void (gtl_IInputExButton::*ButtonMethod)();

struct ButtonMemFun {
    ButtonMethod method;
    // Copy the synchronous teardown/follow-up callback; borrow its future targets.
    ButtonMemFun(ButtonMethod m) : method(m) {}
    // Invoke on a required live button; the supplied callback defines mutation/cleanup.
    void operator()(gtl_IInputExButton* b) const { (b->*method)(); }
};

template <class F>
// Invoke a copied callback on every live borrowed button in [first,last); return its copy.
static inline F ForEachButton(gtl_IInputExButton** first, gtl_IInputExButton** last, F f) {
    for (; first != last; ++first) f(*first);
    return f;
}

extern "C" {


// the device fills *resultOut (CScreenPointer_vfunc_6 reads the sample count this way)
s32 gtl_IInputDevice_GetDeviceResult(void* device, s32* resultOut);
// Forward the cached provider-result query (pointer users interpret it as
// sample count). Required self/driver and one writable s32 resultOut; return
// the underlying status verbatim, with its output effects; retain nothing.
s32 gtl_CInputExDeviceBase_GetDeviceResult(CInputExDeviceBaseGDR* self, s32* resultOut) {
    return gtl_IInputDevice_GetDeviceResult(self->device, resultOut);
}

// The original tail-calls gtl_IInputDevice_GetProviderId(device, out) with r4 untouched; the driver stores the
// provider id through that pointer and returns 1.
s32 gtl_IInputDevice_GetProviderId(void* device);
// Return the borrowed driver's provider identifier; required live self and
// installed driver. No allocation, reference change, sample update or output writes.
s32 gtl_CInputExDeviceBase_GetProviderId(CInputExDeviceBaseProv* self) {
    return gtl_IInputDevice_GetProviderId(self->device);
}

void Memory_Clear(void* dst, u32 size);

} // extern "C"

// Own 20 digital + 30 analog logical buttons and clear 16 provider sample slots
// (0xB00/0xB0) plus history. Concrete channel names remain provider-specific.
#pragma push
#pragma use_lmw_stmw on
// Base starts with no driver. Own the buttons described above; allocation
// exceptions propagate with no local rollback of earlier raw-button allocations.
// A completed pad releases them and its eventual driver in vfunc_0.
gtl::CPad::CPad() {
    for (u32 i = 0; i < kPadDigitalButtonCount; i++) {
        m_buttons[i] = new CInputExDigitalButton;
    }
    for (u32 j = 0; j < kPadAnalogButtonCount; j++) {
        m_buttons[j + kPadDigitalButtonCount] = new CInputExAnalogButton;
    }
    Memory_Clear(m_samples, 0xB00);
    Memory_Clear(m_history, 0x180);
}
#pragma pop

extern "C" {

extern const char* s_padLogicalButtonItemKeys[];
void __dl__Q23gtl25IInputResourceMemOperatorFPv(void* p);
// Null-safe base-button teardown: no payload cleanup; free this allocation
// through IInputResourceMemOperator only for flags>0. Return original address,
// possibly freed; no reference-count adjustment or borrowed-driver release.
void* gtl_IInputExButton_dtor(void* self, s32 flags) {
    if (self != 0) {
        if (flags > 0) {
            __dl__Q23gtl25IInputResourceMemOperatorFPv(self);
        }
    }
    return self;
}

struct RefCountedBase { s32 refCount; };

// a reference-counted object: the count, then the vtable pointer; the deleting destructor is the first virtual
struct RefCounted : RefCountedBase { 
virtual ~RefCounted();
};

extern ButtonMethod s_padButtonReleaseReferenceMethod;
extern char __vt__Q23gtl4CPad[];
extern char __vt__Q23gtl18CInputExDeviceBase[];

// gtl::CPad's deleting destructor
// Null-safe pad teardown: run teardown callback on all 50 owned buttons,
// release retained driver (delete at zero refs), then free self only for flags>0.
// Return original address, possibly freed; serialize with sample adaptation.
void* gtl_CPad_dtor(void* self, s32 flags) {
    if (self != 0) {
        char* p = (char*)self;
        *(void**)(p + 0x4) = __vt__Q23gtl4CPad;
        gtl_IInputExButton** last = (gtl_IInputExButton**)(p + 0xd5c);
        ForEachButton((gtl_IInputExButton**)(p + 0xc94), last, ButtonMemFun(s_padButtonReleaseReferenceMethod));
        if (self != 0) {
            RefCounted* d = *(RefCounted**)(p + 0x8);
            *(void**)(p + 0x4) = __vt__Q23gtl18CInputExDeviceBase;
            if (d != 0 && --d->refCount == 0) delete d;
        }
        if (flags > 0) __dl__Q23gtl25IInputResourceMemOperatorFPv(self);
    }
    return self;
}

// Map logical ordinal 0..49 to its table entry, consumed by UpdateButtons
// as a numeric provider item key. const char* is a pointer-shaped ABI word here,
// not a string dereference; self unused, no bounds check or ownership transfer.
const char* gtl_CPad_vfunc_5(void* self, s32 index) {
    return s_padLogicalButtonItemKeys[index];
}

#pragma push
#pragma use_lmw_stmw on
// Convert FIRST cached provider sample only into 50 logical button words:
// 0..15 = sample bytes +30..3F !=0; 16..19 = four overlapping POV sectors from
// sample+20, negative disables all. Sector centers are 0/9000/18000/27000 in
// provider angle units, inclusive half-width 4500, wrap at 36000. This implies
// hundredths of a degree if the provider follows that convention; no conversion
// is performed here. Boundary directions intentionally activate two sectors.
// 20..31 = six signed axes split into negative magnitude then positive magnitude;
// preserve branchless signed shifts/negation, including extreme-word behavior.
// 32..49 = three six-word groups at sample+50/+70/+90, copied unchanged, not split.
// extensionWords holds these 18 extension words; logicalButtons is the complete logical
// vector. Expanded eight/six-word copies preserve original evaluation order.
// Dispatch once via UpdateButtons (normalization there), always return 1.
// vfunc_1 borrows the sample base at self+14; no fresh sampling/history mutation
// is requested here and no provider count is consulted (unlike screen pointer).
s32 gtl_CPad_UpdateLogicalButtonsFromSample(CPad* self) {
    CPadSample* sample = self->vfunc_1();
    s32 axisValue;
    s32 axisOrSignScratch;
    s32 splitAxes[12];
    s32 k;

    axisValue = sample->axis[0];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[0] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[0];
    splitAxes[1] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);
    axisValue = sample->axis[1];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[2] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[1];
    splitAxes[3] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);
    axisValue = sample->axis[2];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[4] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[2];
    splitAxes[5] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);
    axisValue = sample->axis[3];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[6] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[3];
    splitAxes[7] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);
    axisValue = sample->axis[4];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[8] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[4];
    splitAxes[9] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);
    axisValue = sample->axis[5];
    axisOrSignScratch = axisValue >> 0x1F;
    splitAxes[10] = (-axisValue) & axisOrSignScratch;
    axisOrSignScratch = sample->axis[5];
    splitAxes[11] = axisOrSignScratch & ((-axisOrSignScratch & ~axisOrSignScratch) >> 0x1F);

    s32 group0Value0 = sample->extensionWord00;
    s32 group0Value1 = sample->extensionWord01;
    s32 group0Value2 = sample->extensionWord02;
    s32 group0Value3 = sample->extensionWord03;
    s32 group0Value4 = sample->extensionWord04;
    s32 group1Value0 = sample->extensionWord06;
    s32 group0Value5 = sample->extensionWord05;
    s32 group1Value1 = sample->extensionWord07;
    s32 group1Value2 = sample->extensionWord08;
    s32 group1Value3 = sample->extensionWord09;
    s32 group1Value4 = sample->extensionWord10;
    s32 group1Value5 = sample->extensionWord11;
    s32 group2Value0 = sample->extensionWord12;
    s32 group2Value1 = sample->extensionWord13;
    s32 group2Value2 = sample->extensionWord14;
    s32 group2Value3 = sample->extensionWord15;
    s32 group2Value4 = sample->extensionWord16;
    s32 extensionWords[18];
    extensionWords[0] = group0Value0;
    extensionWords[1] = group0Value1;
    extensionWords[2] = group0Value2;
    extensionWords[3] = group0Value3;
    extensionWords[4] = group0Value4;
    extensionWords[5] = group0Value5;
    extensionWords[6] = group1Value0;
    extensionWords[7] = group1Value1;
    extensionWords[8] = group1Value2;
    extensionWords[9] = group1Value3;
    extensionWords[10] = group1Value4;
    extensionWords[11] = group1Value5;
    extensionWords[12] = group2Value0;
    extensionWords[13] = group2Value1;
    extensionWords[14] = group2Value2;
    extensionWords[15] = group2Value3;
    extensionWords[16] = group2Value4;
    extensionWords[17] = sample->extensionWord17;

    s32 logicalButtons[50] = {0};
    s32 *digitalDestination = &logicalButtons[0];
    s32 digitalByteIndex = 0;

    for (k = 0; k < 2; k++) {
        s32 temp_r29 = (digitalByteIndex + 1) * 4;
        s32 temp_r12 = (digitalByteIndex + 2) * 4;
        s32 temp_r11 = (digitalByteIndex + 3) * 4;
        s32 temp_r10 = (digitalByteIndex + 4) * 4;
        s32 temp_r9 = (digitalByteIndex + 5) * 4;
        s32 temp_r8 = (digitalByteIndex + 6) * 4;
        s32 temp_r0_7 = (digitalByteIndex + 7) * 4;
        *digitalDestination = sample->buttons[digitalByteIndex + 0] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r29) = sample->buttons[digitalByteIndex + 1] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r12) = sample->buttons[digitalByteIndex + 2] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r11) = sample->buttons[digitalByteIndex + 3] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r10) = sample->buttons[digitalByteIndex + 4] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r9) = sample->buttons[digitalByteIndex + 5] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r8) = sample->buttons[digitalByteIndex + 6] != 0;
        *(s32 *)((char *)&logicalButtons[0] + temp_r0_7) = sample->buttons[digitalByteIndex + 7] != 0;
        digitalByteIndex += 8;
        digitalDestination += 8;
    }

    s32 sectorIndex = 0;
    s32 *sectorDestination = &logicalButtons[0];
    s32 sectorLowerBoundary = -0x1194;
    for (k = 0; k < 2; k++) {
        s32 firstPovSample = sample->povHatAngle;
        if (firstPovSample >= 0) {
            if (sectorIndex == 0) {
                if ((firstPovSample >= (s32)(sectorLowerBoundary + 0x8CA0)) || (firstPovSample <= (s32)(sectorLowerBoundary + 0x2328))) {
                    sectorDestination[16] = 1;
                }
            } else if ((firstPovSample >= sectorLowerBoundary) && (firstPovSample <= (s32)(sectorLowerBoundary + 0x2328))) {
                sectorDestination[16] = 1;
            }
        }
        s32 secondPovSample = sample->povHatAngle;
        s32 secondSectorLowerBoundary = sectorLowerBoundary + 0x2328;
        s32 secondSectorIndex = sectorIndex + 1;
        if (secondPovSample >= 0) {
            if (secondSectorIndex == 0) {
                if ((secondPovSample >= (s32)(secondSectorLowerBoundary + 0x8CA0)) || (secondPovSample <= (s32)(secondSectorLowerBoundary + 0x2328))) {
                    sectorDestination[17] = 1;
                }
            } else if ((secondPovSample >= secondSectorLowerBoundary) && (secondPovSample <= (s32)(secondSectorLowerBoundary + 0x2328))) {
                sectorDestination[17] = 1;
            }
        }
        sectorDestination += 2;
        sectorLowerBoundary = secondSectorLowerBoundary + 0x2328;
        sectorIndex = secondSectorIndex + 1;
    }

    for (k = 0; k < 12; k++) {
        logicalButtons[20 + k] = splitAxes[k];
    }

    s32 *extensionDestination = &logicalButtons[32];
    char *extensionBaseBytes = (char *)extensionDestination;
    s32 extensionIndex = 0;
    for (k = 0; k < 3; k++) {
        s32 temp_r29 = (extensionIndex + 1) * 4;
        s32 temp_r12 = (extensionIndex + 2) * 4;
        s32 temp_r11 = (extensionIndex + 3) * 4;
        s32 temp_r10 = (extensionIndex + 4) * 4;
        s32 temp_r9 = (extensionIndex + 5) * 4;
        *extensionDestination = extensionWords[extensionIndex];
        *(s32 *)(extensionBaseBytes + temp_r29) = extensionWords[extensionIndex + 1];
        *(s32 *)(extensionBaseBytes + temp_r12) = extensionWords[extensionIndex + 2];
        *(s32 *)(extensionBaseBytes + temp_r11) = extensionWords[extensionIndex + 3];
        *(s32 *)(extensionBaseBytes + temp_r10) = extensionWords[extensionIndex + 4];
        *(s32 *)(extensionBaseBytes + temp_r9) = extensionWords[extensionIndex + 5];
        extensionDestination += 6;
        extensionIndex += 6;
    }

    gtl_CInputExDeviceBase_UpdateButtons(self, logicalButtons, kPadLogicalButtonCount);
    return 1;
}
// Borrow the cached sample base at required self+0x14. Pad owns 16 records
// of stride 0xB0; adaptation reads only the first and retains no pointer.
CPadSample* gtl_CPad_vfunc_1(char* self) {
    return (CPadSample*)(self + 0x14);
}

extern ButtonMethod s_padButtonResetMethod;
// Run s_padButtonResetMethod follow-up on all 50 required live pad buttons, synchronously.
// No sample polling or ownership transfer; return 1 after the callbacks.
s32 gtl_CPad_ResetButtons(CPad* self) {
    ForEachButton((gtl_IInputExButton**)((char*)self + 0xc94),
                  (gtl_IInputExButton**)((char*)self + 0xd5c),
                  ButtonMemFun(s_padButtonResetMethod));
    return 1;
}

extern void* __vt__Q23gtl9CKeyboard[];
}

struct CKeyboard {
    s32 unk0;
    void* vtable;
    RefCounted* device;
    char unkC[0x108];
    gtl_IInputExButton* buttons[256];
};

extern "C" {

#pragma push
#pragma use_lmw_stmw on


#pragma pop
} // extern "C"

// Construct 256 owned digital buttons and clear all provider key bytes.
// Base starts with no driver reference. Allocation exceptions propagate;
// earlier raw-button allocations have no local rollback on constructor failure.
// Completed objects release buttons in vfunc_0. No hardware polling occurs.
gtl::CKeyboard::CKeyboard() {
    for (u32 i = 0; i < 0x100; i++) {
        m_buttons[i] = new CInputExDigitalButton;
    }
    Memory_Clear(m_keys, 0x100);
}

extern "C" {

extern ButtonMethod s_keyboardButtonReleaseReferenceMethod;
// Null-safe keyboard teardown: invoke each of 256 button release callbacks,
// then release the retained driver (delete at zero refs), then free self only
// for flags>0 using IInputResourceMemOperator. Return the original address,
// possibly freed; callers must not dereference it after deleting teardown.
CKeyboard* gtl_CKeyboard_dtor(CKeyboard* self, s32 flags) {
    if (self != 0) {
        self->vtable = __vt__Q23gtl9CKeyboard;
        gtl_IInputExButton** last = self->buttons + 256;
        ForEachButton(self->buttons, last, ButtonMemFun(s_keyboardButtonReleaseReferenceMethod));
        if (self != 0) {
            RefCounted* d = self->device;
            self->vtable = __vt__Q23gtl18CInputExDeviceBase;
            if (d != 0 && --d->refCount == 0) delete d;
        }
        if (flags > 0) __dl__Q23gtl25IInputResourceMemOperatorFPv(self);
    }
    return self;
}

// Identity mapping from keyboard logical-button ordinal 0..255 to provider item ID; self unused.
s32 gtl_CKeyboard_vfunc_5(void* self, s32 arg1) {
    return arg1;
}

#pragma pop

} // extern "C"


// ---- 0x803E8FD4 ------------------------------------------------------------------------------------------

struct CKeyboardKeysBase {
    char unk0[4];
};

// CKeyboard seen through its vtable: vfunc_1 returns the 256-byte key state
struct CKeyboardKeys : CKeyboardKeysBase {
    virtual void vfunc0();
    virtual s8* vfunc1();
};

extern "C" {


// Read all 256 provider key bytes (slot 1); nonzero becomes digital down.
// Dispatch synchronously to the 256 owned buttons. Requires valid self/state;
// does not poll hardware or clear keys, retain buffers, or report failure.
void gtl_CKeyboard_AdaptKeySample(CKeyboardKeys* self) {
    s8* state;
    s32 buf[0x100];
    s32* dst;
    u32 i;
    s8* row;
    u32 off;

    state = self->vfunc1();
    dst = buf;
    off = 0;
    for (i = 0; i < 0x20; i++) {
        row = state + off;
        off += 8;
        dst[0] = row[0] != 0;
        dst[1] = row[1] != 0;
        dst[2] = row[2] != 0;
        dst[3] = row[3] != 0;
        dst[4] = row[4] != 0;
        dst[5] = row[5] != 0;
        dst[6] = row[6] != 0;
        dst[7] = row[7] != 0;
        dst += 8;
    }
    gtl_CInputExDeviceBase_UpdateButtons(self, buf, 0x100);
}

} // extern "C"


// ---- 0x803E90FC ------------------------------------------------------------------------------------------

extern "C" {

// Borrow the 256-byte key sample at self+14 for the device lifetime; self required.
void* gtl_CKeyboard_GetKeyBytes(void* self) {
    return (char*)self + 0x14;
}

} // extern "C"


// ---- 0x803E9104 ------------------------------------------------------------------------------------------

struct DummyPmf {};
typedef void (DummyPmf::*Dpmf)();

extern "C" {

extern Dpmf s_keyboardButtonResetMethod;

// Run s_keyboardButtonResetMethod follow-up on all 256 live buttons synchronously; no
// ownership transfer. self/buttons required; return 1 after every callback.
s32 gtl_CKeyboard_RunAllButtonCallbacks(void* arg0) {
    char* self = (char*)arg0;
    void** p = (void**)(self + 0x114);
    void** end = (void**)(self + 0x514);
    u32 pmfs[6];
    *(Dpmf*)&pmfs[0] = s_keyboardButtonResetMethod;
    *(Dpmf*)&pmfs[3] = s_keyboardButtonResetMethod;
    while (p != end) {
        DummyPmf* obj = (DummyPmf*)*p;
        (obj->*(*(Dpmf*)&pmfs[3]))();
        p++;
    }
    return 1;
}

} // extern "C"


// ---- 0x803E9184 ------------------------------------------------------------------------------------------

struct CMouseSample {
    s32 deltaX; // +0: signed relative motion, accumulated in position units
    s32 deltaY; // +4: signed relative motion; no axis inversion
    s32 wheelDelta; // +8: split into positive and negative analog channels
    s8 buttons[4]; // +C: nonzero is down, provider-owned sample
};

struct Vec2 {
    f32 x;
    f32 y;
    // Leave scratch coordinates unspecified until both components are assigned.
    Vec2() {}
    // Initialize the pair without unit conversion or allocation.
    Vec2(f32 x_, f32 y_) : x(x_), y(y_) {}
};

struct Vec2Bits {
    s32 x;
    s32 y;
};

struct CMouseBase {
    char unk0[4];
};

struct CMouse : CMouseBase {
    virtual void vfunc_0();
    virtual CMouseSample* vfunc_1();
    char unk8[0x34];
    Vec2 pos; // +3C: accumulated/clamped position, same units as provider deltas
    f32 maximumX; // +44: inclusive X upper bound (m_screenSize.x)
    f32 maximumY; // +48: inclusive Y upper bound (m_screenSize.y)
};

// std::mem_fun(&T::f) and std::for_each, as the game's code inlines them: the member-function pointer is copied
// once into the mem_fun object and once more into for_each's by-value functor parameter.
struct MemFunRef {
    Dpmf pmf;
    // Copy the release/follow-up member callback; no ownership of targets.
    MemFunRef(Dpmf f) : pmf(f) {}
    // Invoke synchronously on a required live button; callback defines effects.
    void operator()(void* obj) const { (((DummyPmf*)obj)->*pmf)(); }
};

// Apply a by-value callback to each borrowed pointer in [first,last), then return its copy.
template <class F> static inline F ForEach(void** first, void** last, F f) {
    for (; first != last; ++first) f(*first);
    return f;
}

// Return a borrowed input reference; a<b selects b, otherwise a (including unordered).
static inline const f32& MaxF32(const f32& a, const f32& b) {
    return (a < b) ? b : a;
}

// Return a borrowed input reference; b<a selects b, otherwise a (including unordered).
static inline const f32& MinF32(const f32& a, const f32& b) {
    return (b < a) ? b : a;
}

// Clamp with max THEN min; preserve comparison order, ties and unordered selection.
static inline f32 ClampF32(f32 v, f32 lo, f32 hi) {
    return MinF32(MaxF32(v, lo), hi);
}

extern "C" {

void gtl_Vec2_Add(void*, void*, void*);
extern const f32 kFloat_0_InputEx;
extern const f32 kMouseDefaultMaximumX;
extern const f32 kMouseDefaultMaximumY;
extern const double kSignedIntToDoubleMagic;
extern void* s_mouseLogicalButtonItemKeys[];
extern void* s_screenPointerLogicalButtonItemKeys[];
extern const f32 kInputExPointerSensitivityDefault;
extern char __vt__Q23gtl6CMouse[];
extern char __vt__Q23gtl14CScreenPointer[];
extern char __vt__Q23gtl18CInputExDeviceBase[];
extern void __dl__Q23gtl25IInputResourceMemOperatorFPv(void*);
extern Dpmf s_mouseButtonReleaseReferenceMethod;
extern Dpmf s_mouseButtonResetMethod;
extern Dpmf s_screenPointerButtonReleaseReferenceMethod;

// Adapt one already-populated borrowed sample (virtual slot 1, self+14).
// Dispatch four digital buttons, then positive/negative wheel magnitudes to
// UpdateButtons (which normalizes analogs using provider maxima). The positive
// sign-mask expression is kept verbatim; the negative channel likewise negates
// only a negative signed wheel word. Provider wheel words must exclude INT_MIN
// (signed negation is undefined there); no overflow handling is added here.
// Convert Y then X deltas to float, add them to accumulated pos, then clamp
// X followed by Y to [0,screenSize] with the comparison order in ClampF32.
// Provider units are not proven pixels; no sensitivity or time scaling here.
// Vec2Bits carries identical float bits through two integer words, never an
// integer-coordinate conversion. Preserve add/conversion/clamp order and NaN
// selection. Writes buttons and pos only; sample is unchanged; returns 1 with
// no propagated query status. Required live self/sample/buttons; no retention.
s32 gtl_CMouse_AdaptRelativeSample(CMouse* self) {
    CMouseSample* sample;
    s32* unusedInputCursor;
    s32* unusedOutputCursor;
    s32 buttonIndex;
    s32 unusedSampleWord;
    s32 unusedSampleWord2;
    s32 positiveWheelSource;
    s32 negativeWheelSource;
    s32 buttonValues[6];
    s32 wheelChannels[2];
    Vec2Bits positionBits;
    Vec2 motionDelta;
    Vec2 result;
    Vec2 unclampedPosition;
    s32 unusedLoopCounter;
    f32 zeroX;
    f32 zeroY;
    f32 x;
    f32 maximumX;
    f32 y;
    f32 maximumY;
    f32 clampedX;
    f32 clampedY;

    sample = self->vfunc_1();
    positiveWheelSource = sample->wheelDelta;
    wheelChannels[0] = positiveWheelSource & ((s32) (-positiveWheelSource & ~positiveWheelSource) >> 0x1F);
    negativeWheelSource = sample->wheelDelta;
    wheelChannels[1] = negativeWheelSource < 0 ? -negativeWheelSource : 0;
    for (buttonIndex = 0; buttonIndex < 6; buttonIndex++) {
        if (buttonIndex < 4) {
            buttonValues[buttonIndex] = sample->buttons[buttonIndex] != 0;
        } else {
            buttonValues[buttonIndex] = wheelChannels[buttonIndex - 4];
        }
    }

    gtl_CInputExDeviceBase_UpdateButtons(self, buttonValues, 6);
    f32 deltaY = (f32) sample->deltaY;
    f32 deltaX = (f32) sample->deltaX;
    motionDelta.y = deltaY;
    motionDelta.x = deltaX;
    gtl_Vec2_Add(&unclampedPosition, &self->pos, &motionDelta);

    positionBits = *(Vec2Bits*)&unclampedPosition;
    maximumX = self->maximumX;
    zeroX = kFloat_0_InputEx;
    x = *(f32*)&positionBits.x;
    clampedX = ClampF32(x, zeroX, maximumX);
    maximumY = self->maximumY;
    zeroY = kFloat_0_InputEx;
    y = *(f32*)&positionBits.y;
    clampedY = ClampF32(y, zeroY, maximumY);

    result.x = clampedX;
    result.y = clampedY;
    self->pos = result;
    return 1;
}

} // extern "C"

// 4 digital buttons, then the 2 analog ones (the wheel) after them
// Own four digital and two wheel analog buttons; clear the 16-byte sample,
// zero accumulated position and set inclusive bounds to 640/480 provider
// units (kMouseDefaultMaximumX/2184). These clamp position; they do not resize the
// render viewport or change the provider's sample range. Base owns no
// driver initially. new failures propagate without local rollback of earlier
// button allocations; vfunc_0 releases completed devices.
gtl::CMouse::CMouse() {
    for (unsigned int i = 0; i < 4; i++) {
        m_buttons[i] = new CInputExDigitalButton;
    }
    unsigned int j = 0;
    while (j < 2) {
        // the game indexes the analog buttons by byte offset here (an indexed m_buttons[j + 4] is strength-reduced
        // into a moving pointer, which the game's loop does not do)
        *(IInputExButton**)((char*)m_buttons + (j + 4) * 4) = new CInputExAnalogButton;
        j++;
    }
    Memory_Clear(m_sample, 0x10);
    m_position = InputVec2(kFloat_0_InputEx, kFloat_0_InputEx);
    m_screenSize = InputVec2(kMouseDefaultMaximumX, kMouseDefaultMaximumY);
}

extern "C" {

// Borrow the 16-byte relative-motion/button sample at self+14; live self required.
CMouseSample* gtl_CMouse_GetRawSample(CMouse* arg0) {
    return (CMouseSample*)((char*)arg0 + 0x14);
}

// Map mouse logical-button ordinal 0..5 through the borrowed provider-ID table; arg0 unused.
void* gtl_CMouse_vfunc_5(CMouse* arg0, s32 arg1) {
    return s_mouseLogicalButtonItemKeys[arg1];
}

// Map pointer logical-button ordinal 0..15 through its provider-ID table; arg0 unused.
void* gtl_CScreenPointer_vfunc_5(void* arg0, s32 arg1) {
    return s_screenPointerLogicalButtonItemKeys[arg1];
}

// Run s_mouseButtonResetMethod follow-up on all six live mouse buttons. Borrow self,
// retain nothing, return 1 after the synchronous callbacks; no polling.
s32 gtl_CMouse_vfunc_7(CMouse* arg0) {
    ForEach((void**)((char*)arg0 + 0x24), (void**)((char*)arg0 + 0x3c), MemFunRef(s_mouseButtonResetMethod));
    return 1;
}

} // extern "C"

// Own 16 digital buttons; initialize four cached KPAD filter requests to
// {playRadius=0, sensitivity=1}; these values are not sent to KPAD here.
// Clear provider samples/history and reset flag. No driver is acquired here.
// Allocation failure propagates without local rollback of earlier buttons;
// vfunc_0 releases a completed pointer device.
gtl::CScreenPointer::CScreenPointer() {
    m_flag = 0;
    for (unsigned int i = 0; i < 16; i++) {
        m_buttons[i] = new CInputExDigitalButton;
    }
    f32 one;
    f32 zero;
    zero = kFloat_0_InputEx;
    one = kInputExPointerSensitivityDefault;
    m_samplingParameters[0].playRadius = zero;
    m_samplingParameters[0].sensitivity = one;
    m_samplingParameters[1].playRadius = zero;
    m_samplingParameters[1].sensitivity = one;
    m_samplingParameters[2].playRadius = zero;
    m_samplingParameters[2].sensitivity = one;
    m_samplingParameters[3].playRadius = zero;
    m_samplingParameters[3].sensitivity = one;
    Memory_Clear(m_samples, 0xB00);
    Memory_Clear(m_history, 0x240);
}

extern "C" {

// Null-safe mouse teardown: release six owned buttons via s_mouseButtonReleaseReferenceMethod,
// drop the retained driver's reference (delete at zero), and free this object
// only when arg1>0. Return original address, potentially freed. Serialize with
// sample adaptation; no callbacks may retain button/sample addresses.
void* gtl_CMouse_dtor(CMouse* arg0, s32 arg1) {
    if (arg0 != 0) {
        char* self = (char*)arg0;
        *(void**)(self + 0x4) = (void*)__vt__Q23gtl6CMouse;
        void** last = (void**)(self + 0x3C);
        ForEach((void**)(self + 0x24), last, MemFunRef(s_mouseButtonReleaseReferenceMethod));
        if (arg0 != 0) {
            RefCounted* dev = *(RefCounted**)(self + 0x8);
            *(void**)(self + 0x4) = (void*)__vt__Q23gtl18CInputExDeviceBase;
            if (dev != 0 && --dev->refCount == 0) delete dev;
        }
        if (arg1 > 0) {
            __dl__Q23gtl25IInputResourceMemOperatorFPv(arg0);
        }
    }
    return arg0;
}

// Null-safe pointer teardown: release 16 owned buttons via s_screenPointerButtonReleaseReferenceMethod,
// then the retained driver (delete at zero refs), then self only for arg1>0.
// Return original address, possibly freed; serialize with provider updates.
void* gtl_CScreenPointer_dtor(void* arg0, s32 arg1) {
    if (arg0 != 0) {
        char* self = (char*)arg0;
        *(void**)(self + 0x4) = (void*)__vt__Q23gtl14CScreenPointer;
        void** last = (void**)(self + 0xD94);
        ForEach((void**)(self + 0xD54), last, MemFunRef(s_screenPointerButtonReleaseReferenceMethod));
        if (arg0 != 0) {
            RefCounted* dev = *(RefCounted**)(self + 0x8);
            *(void**)(self + 0x4) = (void*)__vt__Q23gtl18CInputExDeviceBase;
            if (dev != 0 && --dev->refCount == 0) delete dev;
        }
        if (arg1 > 0) {
            __dl__Q23gtl25IInputResourceMemOperatorFPv(arg0);
        }
    }
    return arg0;
}

} // extern "C"


// ---- 0x803E985C ------------------------------------------------------------------------------------------

struct CScreenPointerParam {
    s32 unk0; // Native provider words, normalized using range-item ID == byte offset.
    s32 unk4; // Neither pixels nor world units are proven: see vfunc_6 contract.
    char unk8[4];
    s32 unkC;
    s32 unk10;
    char unk14[0x3C];
    s32 unk50;
    s32 unk54;
    s32 unk58;
    s32 unk5C;
    s32 unk60;
    char unk64[0x4C];
};

struct CScreenPointerEntry {
    f32 vals[8]; // Ratios from sample offsets 50,54,58,0,4,C,10,5C, in this order.
    s32 unk20; // sample+60 clamped to nonnegative integer (no normalization).
};

struct CScreenPointerVtblBase {
    s32 unk0;
};

struct CScreenPointerVtbl : CScreenPointerVtblBase {
    virtual void vf0();
    virtual u8* vf1();
};

extern "C" {

extern const double kSignedIntToDoubleMagic;

// Adapt cached pointer samples to normalized history, no smoothing filter.
// Phase 1: read first sample's 16 digital bytes +30..3F and dispatch them once.
// Phase 2: query eight [min,max] range pairs by raw field offset; ignore min.
// Phase 3: GetDeviceResult writes count (initially zero); consume count samples
// at stride 0xB0, writing 36-byte CScreenPointerEntry records at self+0xB14.
// Provider MUST supply 0..16 samples (0xB00 raw / 0x240 history capacity), a
// valid sample base even at count zero (phase 1 reads it), and usable query
// outputs. Status values are ignored; negative count writes no history, while
// excessive positive count overruns it. Old history beyond count stays intact.
// Each signed integer / per-field maximum yields a dimensionless ratio; no
// min subtraction, range clamp, zero/NaN-denominator guard or pixel mapping.
// Last integer is max(sample+60,0). Raw channel identities/physical units are
// provider-defined and unproven here; do not rename these as screen X/Y or
// accelerometer values without tracing the provider. Consumers must use count.
// The repeated vf1 calls and division/store order are preserved; self owns
// storage, all queried/provider pointers are borrowed. Always returns 1.
s32 gtl_CScreenPointer_PollAndNormalizeHistory(u8* self) {
    s32 logicalButtons[16];
    f32 rangeAt58[2];
    f32 rangeAt54[2];
    f32 rangeAt50[2];
    f32 rangeAt04[2];
    f32 rangeAt00[2];
    f32 rangeAt10[2];
    f32 rangeAt0C[2];
    f32 rangeAt5C[2];
    CScreenPointerParam* sample;
    s32 providerCount;
    s32* buttonOutput;
    s32 buttonGroup;
    CScreenPointerEntry* historyEntry;
    s32 sampleCount;
    s32 sampleIndex;
    s8* digitalBytes;
    u32 sampleByteOffset;
    u8* firstSample;

    firstSample = ((CScreenPointerVtbl*)self)->vf1();

    buttonOutput = logicalButtons;
    sampleByteOffset = 0;
    for (buttonGroup = 0; buttonGroup < 2; buttonGroup++) {
        digitalBytes = (s8*)(firstSample + sampleByteOffset);
        buttonOutput[0] = digitalBytes[0x30] != 0;
        buttonOutput[1] = digitalBytes[0x31] != 0;
        buttonOutput[2] = digitalBytes[0x32] != 0;
        buttonOutput[3] = digitalBytes[0x33] != 0;
        buttonOutput[4] = digitalBytes[0x34] != 0;
        buttonOutput[5] = digitalBytes[0x35] != 0;
        buttonOutput[6] = digitalBytes[0x36] != 0;
        buttonOutput[7] = digitalBytes[0x37] != 0;
        sampleByteOffset += 8;
        buttonOutput += 8;
    }

    gtl_CInputExDeviceBase_UpdateButtons(self, logicalButtons, kPointerButtonCount);

    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x50, rangeAt50);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x54, rangeAt54);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x58, rangeAt58);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x0, rangeAt00);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x4, rangeAt04);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0xC, rangeAt0C);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x10, rangeAt10);
    gtl_CInputExDeviceBase_ReadItemValueOrRange(self, 1, 0x5C, rangeAt5C);

    providerCount = 0;
    gtl_IInputDevice_GetDeviceResult(*(void**)(self + 8), &providerCount);
    sampleCount = providerCount;

    historyEntry = (CScreenPointerEntry*)(self + kPointerHistoryOffset);
    sampleIndex = 0;
    sampleByteOffset = 0;
    for (; sampleIndex < sampleCount; sampleIndex++) {
        sample = (CScreenPointerParam*)(((CScreenPointerVtbl*)self)->vf1() + sampleByteOffset);
        {
            f32 normalizedAt58 = (f32)sample->unk58 / rangeAt58[1];
            f32 normalizedAt50 = (f32)sample->unk50 / rangeAt50[1];
            f32 normalizedAt54 = (f32)sample->unk54 / rangeAt54[1];
            historyEntry->vals[0] = normalizedAt50;
            historyEntry->vals[1] = normalizedAt54;
            historyEntry->vals[2] = normalizedAt58;
        }
        {
            f32 normalizedAt00 = (f32)sample->unk0 / rangeAt00[1];
            f32 normalizedAt04 = (f32)sample->unk4 / rangeAt04[1];
            historyEntry->vals[3] = normalizedAt00;
            historyEntry->vals[4] = normalizedAt04;
        }
        {
            f32 normalizedAt0C = (f32)sample->unkC / rangeAt0C[1];
            f32 normalizedAt10 = (f32)sample->unk10 / rangeAt10[1];
            historyEntry->vals[5] = normalizedAt0C;
            historyEntry->vals[6] = normalizedAt10;
        }
        {
            f32 normalizedAt5C = (f32)sample->unk5C / rangeAt5C[1];
            historyEntry->vals[7] = normalizedAt5C;
        }
        {
            u32 negativeFlag = (u32)sample->unk60 >> 31;
            historyEntry->unk20 = sample->unk60 & (negativeFlag - 1);
        }
        sampleByteOffset += kProviderSampleStride;
        historyEntry++;
    }

    return 1;
}

} // extern "C"
