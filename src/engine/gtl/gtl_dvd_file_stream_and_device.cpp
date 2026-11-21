#include <decomp/static_init.h>
#include "classes/GtlDvdFileStream.h"
#include "classes/MWStringStorage.h"
// Access an existing DVD stream without constructing or retaining it.
static inline GtlDvdFileStream* AsGtlDvdFileStream(void* object) { return (GtlDvdFileStream*)object; }
// Translation unit 0x80412664-0x80412858 (17 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#include "sdk_protos.h"

extern "C" {

extern char s_dvdFileStreamTypeInfo;
extern char s_dvdFileStreamBaseTypeInfo;
extern char gtlFileIoDeviceVtable[0x90];
extern char pathSeparatorString;
extern char __vt__Q23gtl16CFileIoDeviceDVD[0x90];
extern float kFloat_6_2831855;
extern float s_dvdFileStreamTwoPi;

void FileIO_Free(void*);
void FileIOString_ReplaceRangeWithAllocator(void*, int, unsigned int, const char*, const char*, const char*);
void DVDInit();

struct DeviceDtorLayout {
    void* vtable;
    MWStringStorage rootPath;
};

enum DvdStreamConstants { kDvdTransferAlignment = 0x20, kDvdOffsetAlignment = 4,
       kStreamSlotTrue = 1, kStreamSlotFalse = 0, kShortStringLengthMask = 0x7f,
       kDeviceRootPathOffset = (int)&((DeviceDtorLayout*)0)->rootPath };

// Return virtual slot 13's transfer alignment in bytes (its specific caller role is unresolved).
int gtl_nw4r_ut_DvdFileStream_vfunc_13() {
    return kDvdTransferAlignment;
}

// Return virtual slot 12's transfer alignment in bytes (distinct from slot 13).
int gtl_nw4r_ut_DvdFileStream_vfunc_12() {
    return kDvdTransferAlignment;
}

// Return the stream offset alignment in bytes.
int gtl_nw4r_ut_DvdFileStream_vfunc_11() {
    return kDvdOffsetAlignment;
}

// Return fixed true for virtual slot 19; its exact query is not established here.
int gtl_nw4r_ut_DvdFileStream_vfunc_19() {
    return kStreamSlotTrue;
}

// Return fixed false for virtual slot 10; its exact query is not established here.
int gtl_nw4r_ut_DvdFileStream_vfunc_10() {
    return kStreamSlotFalse;
}

// Return fixed true for virtual slot 9; its exact query is not established here.
int gtl_nw4r_ut_DvdFileStream_vfunc_9() {
    return kStreamSlotTrue;
}

// Return fixed true for virtual slot 18; its exact query is not established here.
int gtl_nw4r_ut_DvdFileStream_vfunc_18() {
    return kStreamSlotTrue;
}

// Return fixed true for virtual slot 8; its exact query is not established here.
int gtl_nw4r_ut_DvdFileStream_vfunc_8() {
    return kStreamSlotTrue;
}

// Return the cached byte length/read limit using the ABI's signed integer return slot.
int gtl_nw4r_ut_DvdFileStream_vfunc_14(void* self) {
    return (int)AsGtlDvdFileStream(self)->length;
}

// Return current byte position in an ABI pointer slot; it is not a dereferenceable address.
void* gtl_nw4r_ut_DvdFileStream_vfunc_20(void* self) {
    return (void*)AsGtlDvdFileStream(self)->position;
}

// Return the async-read busy byte without polling the DVD hardware.
unsigned char gtl_nw4r_ut_DvdFileStream_vfunc_7(void* self) {
    return AsGtlDvdFileStream(self)->busy;
}

// Return borrowed static runtime type information for DVD streams.
void* gtl_nw4r_ut_DvdFileStream_GetRTTI() {
    return &s_dvdFileStreamTypeInfo;
}

// Report the device's fixed availability/support result; no media check is performed.
int gtl_CFileIoDeviceDVD_vfunc_1() {
    return 1;
}

// Construct the device with "/" as its root path and initialize the DVD subsystem.
void* gtl_CFileIoDeviceDVD_Ctor(void* self) {
    // Start the embedded 12-byte root-path string empty under the base-device vtable.
    // Retain the word view and byte-based subobject address for the original register lifetimes.
    unsigned int* fields = (unsigned int*)self;
    fields[1] = 0;
    fields[2] = 0;
    fields[3] = 0;
    fields[0] = (unsigned int)&gtlFileIoDeviceVtable;
    unsigned char inlineLength = ((unsigned char*)self)[kDeviceRootPathOffset] & kShortStringLengthMask;
    unsigned char allocatorByte = 0;   // the original passes an unused stack byte here
    size_t slashLength = strlen(&pathSeparatorString);
    FileIOString_ReplaceRangeWithAllocator((char*)self + kDeviceRootPathOffset, 0, inlineLength, &pathSeparatorString, &pathSeparatorString + slashLength, (char*)&allocatorByte);
    // The root path is now copied; install the derived vtable before initializing DVD.
    fields[0] = (unsigned int)&__vt__Q23gtl16CFileIoDeviceDVD;
    DVDInit();
    return self;
}

// Release the root path's heap storage and optionally the device itself.
// deleteStorage > 0 frees self; zero/negative values only destroy its contents.
void* gtl_CFileIoDeviceDVD_dtor(void* self, int deleteStorage) {
    if (self != 0) {
        DeviceDtorLayout* device = (DeviceDtorLayout*)self;
        if (&device->rootPath.flags != 0 && (device->rootPath.flags >> 31) != 0)
            FileIO_Free(device->rootPath.data);
        if (deleteStorage > 0)
            FileIO_Free(self);
    }
    return self;
}

// Link the stream's runtime type to its base and initialize the unit's two-pi cache.
void gtl_nw4r_ut_DvdFileStream_InitializeTypeInfoAndConstants() {
    float value = kFloat_6_2831855;
    *(void**)&s_dvdFileStreamTypeInfo = &s_dvdFileStreamBaseTypeInfo;
    s_dvdFileStreamTwoPi = value;
}

// Base device destruction uses the same root-path ownership convention.
// A set top flag bit owns rootPath.data; deleteStorage > 0 also frees self.
void* dtor_80412774(void* self, int deleteStorage) {
    if (self != 0) {
        DeviceDtorLayout* device = (DeviceDtorLayout*)self;
        if (&device->rootPath.flags != 0) {
            if ((device->rootPath.flags >> 31) != 0) {
                FileIO_Free(device->rootPath.data);
            }
        }
        if (deleteStorage > 0) {
            FileIO_Free(self);
        }
    }
    return self;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU80462714", _ctors_u80412664_0, gtl_nw4r_ut_DvdFileStream_InitializeTypeInfoAndConstants);
#pragma pop
