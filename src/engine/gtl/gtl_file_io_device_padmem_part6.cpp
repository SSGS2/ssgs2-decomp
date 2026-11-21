// Translation unit 0x80420C30-0x80420C40 (2 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

// PADMEM virtual slots are indexed by phase1_class_tables.cpp; keep their ABI names.
enum PadMemDeviceConstants { kPadMemSlot24Result = 0x20, kPadMemSlot9Result = 1 };

// Return the fixed slot-24 value (32); its PADMEM-specific interpretation is unresolved.
// No arguments, object access or ownership changes. DVD slot 24 reports alignment,
// but there is no direct PADMEM call site here to establish the same contract.
int gtl_CFileIoDevicePADMEM_vfunc_24(void) {
    return kPadMemSlot24Result;
}

// Return the fixed slot-9 value (1); the meaning of this status is unresolved.
// No arguments, object access or ownership changes; do not infer a handle check.
int gtl_CFileIoDevicePADMEM_vfunc_9(void) {
    return kPadMemSlot9Result;
}

} // extern "C"
