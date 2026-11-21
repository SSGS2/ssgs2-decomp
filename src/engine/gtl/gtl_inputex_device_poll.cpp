// InputEx controller/device polling, 0x803E8244..0x803E833C.
// Reads the low-level IInputDevice into the concrete InputEx object's sample
// buffer, then publishes buttons/axes through its post-read hook. Concrete pad,
// keyboard, mouse and screen-pointer implementations are in gtl_input_ex_devices_part1.cpp.
// For controller mapping edits, change those hooks and their buffers together;
// this routine coordinates acquisition, conversion and failure cleanup.


typedef void* (*InputExPointerSlot)(void*);
typedef int (*InputExIntSlot)(void*);
typedef void (*InputExVoidSlot)(void*);

struct GtlInputExHeader {
    int unk0;
};

struct GtlInputExDeviceBase : GtlInputExHeader {
    virtual void vfunc0();
    virtual void* vfunc1();
    virtual int vfunc2();
    virtual void vfunc3();
    virtual void vfunc4();
    virtual void vfunc5();
    virtual int PublishRawState();
    virtual void ResetAfterPollFailure();
    void* device;
    unsigned char initialized;
};

#define INPUT_EX_DEVICE(object) ((object)->device)

// All borrow a live concrete InputEx receiver; no adapter retains the result.
#pragma push
#pragma always_inline on
// Borrow writable raw storage owned by the concrete pad/keyboard/mouse device.
static inline void* InputEx_GetRawStateBuffer(GtlInputExDeviceBase* self) {
    return (self->vfunc1());
}
// Return that buffer's required device-descriptor byte size (not button count).
static inline int InputEx_GetRawStateBytes(GtlInputExDeviceBase* self) {
    return (self->vfunc2());
}
// Convert the just-read sample into the concrete object's published button state.
static inline int InputEx_PublishRawState(GtlInputExDeviceBase* self) {
    return (self->PublishRawState());
}
// Reset concrete button state after acquisition/conversion failure.
static inline void InputEx_ResetAfterPollFailure(GtlInputExDeviceBase* self) {
    (self->ResetAfterPollFailure());
}
#pragma pop

extern "C" {

int InputDevice_NoOpSuccess_80435D34(void* device);
int gtl_IInputDevice_RefreshDriverState(void* device);
int gtl_IInputDevice_ReadState(void* device, int stateBytes, void* stateBuffer);
int PadManager_SetMotorEnabled(void* device, int flag);

// self and its +8 device must be live; virtual dispatch prefix is at +4 and the
// attempt flag at +0xC. Slot 1 returns borrowed writable sample storage, slot 2
// its byte size; the concrete type must supply a compatible pair. Neither is
// retained here. Slot 6 consumes the freshly read sample and returns success;
// slot 7 resets concrete input state after failure. See CPad/CMouse/etc in
// gtl_input_ex_devices_part1.cpp for each sample schema and the UpdateButtons consumer.
// Slot map (byte offsets include RTTI/null prefix, from object vptr at +4):
// +0C=1 get raw buffer; +10=2 get size; +20=6 convert/publish;
// +24=7 reset each button. gtl_C*_vfunc_2 (gtl_input_ex_devices_part2.cpp) reports pad/screen
// pointer 0xB0, keyboard 0x100, mouse 0x10 bytes. Raw buffers start at self+14
// in those concrete types; CPadSample/CMouseSample and gtl_input.h define them.
// ReadState (gtl_texture_projection_input_device.cpp) requires size equal to device descriptor stateSizeBytes;
// it rejects a missing buffer/descriptor before clearing, clears the entire
// buffer before provider I/O, then returns the provider's status. Failure can
// therefore mean untouched, zeroed, or partially filled raw bytes. Reset does
// not promise to restore the previous raw sample. The first "NoOpSuccess" call
// is currently unconditional success, not a device-present/connected test.
// Return 1 leaves initialized=1 and converted state published. Any failed stage
// clears initialized, disables the device motor and invokes slot 7, returning 0.
// A failed read may have touched raw storage; there is no buffer rollback here.
// initialized is set BEFORE attempting I/O, so hooks observe the attempted state.
int gtl_CInputExDeviceBase_Poll(GtlInputExDeviceBase* self) {
    self->initialized = 1;
    if (InputDevice_NoOpSuccess_80435D34(INPUT_EX_DEVICE(self)) == 0) {
        self->initialized = 0;
        goto fail;
    }
    if (gtl_IInputDevice_RefreshDriverState(INPUT_EX_DEVICE(self)) != 0) {
        void* stateBuffer = InputEx_GetRawStateBuffer(self);
        int stateBytes = InputEx_GetRawStateBytes(self);
        if (gtl_IInputDevice_ReadState(INPUT_EX_DEVICE(self), stateBytes, stateBuffer) != 0) {
            if (InputEx_PublishRawState(self) != 0) {
                return 1;
            }
        }
    }
fail:
    self->initialized = 0;
    PadManager_SetMotorEnabled(INPUT_EX_DEVICE(self), 0);
    InputEx_ResetAfterPollFailure(self);
    return 0;
}

} // extern "C"
