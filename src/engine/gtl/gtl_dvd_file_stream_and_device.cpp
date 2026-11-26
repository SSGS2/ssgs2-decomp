// DvdFileStream virtual functions (alignment, length, position, busy state and RTTI), the
// DVD file I/O device (gtl::CFileIoDeviceDVD: init, availability and destruction), and the
// runtime type links that tie the stream to its base type.

#include <decomp/static_init.h>
#include "classes/GtlDvdFileStream.h"
#include "classes/GtlFileIoDeviceDVD.h"
#include "classes/MWStringStorage.h"

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

enum DvdStreamConstants {
    kDvdTransferAlignment = 0x20,
    kDvdOffsetAlignment = 4,
    kStreamSlotTrue = 1,
    kStreamSlotFalse = 0,
    kShortStringLengthMask = 0x7f
};

} // extern "C"

// Return slot 13's transfer alignment in bytes.
int gtl::nw4r::ut::DvdFileStream::vfunc_13() {
    return kDvdTransferAlignment;
}

// Return slot 12's transfer alignment in bytes.
int gtl::nw4r::ut::DvdFileStream::vfunc_12() {
    return kDvdTransferAlignment;
}

// Return the stream offset alignment in bytes.
int gtl::nw4r::ut::DvdFileStream::vfunc_11() {
    return kDvdOffsetAlignment;
}

// Return fixed true for slot 19.
int gtl::nw4r::ut::DvdFileStream::vfunc_19() {
    return kStreamSlotTrue;
}

// Return fixed false for slot 10.
int gtl::nw4r::ut::DvdFileStream::vfunc_10() {
    return kStreamSlotFalse;
}

// Return fixed true for slot 9.
int gtl::nw4r::ut::DvdFileStream::vfunc_9() {
    return kStreamSlotTrue;
}

// Return fixed true for slot 18.
int gtl::nw4r::ut::DvdFileStream::vfunc_18() {
    return kStreamSlotTrue;
}

// Return fixed true for slot 8.
int gtl::nw4r::ut::DvdFileStream::vfunc_8() {
    return kStreamSlotTrue;
}

// Return the cached byte length/read limit using the signed integer return slot.
int gtl::nw4r::ut::DvdFileStream::GetLength() {
    return (int)length;
}

// Return current byte position in a pointer slot; it is not a dereferenceable address.
void* gtl::nw4r::ut::DvdFileStream::GetPosition() {
    return (void*)position;
}

// Return the async-read busy byte without polling the DVD hardware.
unsigned char gtl::nw4r::ut::DvdFileStream::IsBusy() {
    return busy;
}

// Return borrowed static runtime type information for DVD streams.
void* gtl::nw4r::ut::DvdFileStream::GetRTTI() {
    return &s_dvdFileStreamTypeInfo;
}

// Report the device's fixed availability/support result; no media check is performed.
int gtl::CFileIoDeviceDVD::IsAvailable() {
    return 1;
}

// Set the root path to "/" and initialize the DVD subsystem; returns this.
void* gtl::CFileIoDeviceDVD::Init() {
    // Start the embedded 12-byte root-path string empty under the base-device vtable.
    rootPath.flags = 0;
    rootPath.length = 0;
    rootPath.data = 0;
    vtbl = &gtlFileIoDeviceVtable;
    unsigned char inlineLength = ((unsigned char*)&rootPath)[0] & kShortStringLengthMask;
    unsigned char allocatorByte = 0; // the original passes an unused stack byte here
    size_t slashLength = strlen(&pathSeparatorString);
    FileIOString_ReplaceRangeWithAllocator(&rootPath, 0, inlineLength, &pathSeparatorString,
                                           &pathSeparatorString + slashLength, (char*)&allocatorByte);
    // The root path is now copied; install the derived vtable before initializing DVD.
    vtbl = &__vt__Q23gtl16CFileIoDeviceDVD;
    DVDInit();
    return this;
}

// Release the root path's heap storage and optionally the device itself.
// deleteStorage > 0 frees the device; zero/negative values only destroy its contents.
void* gtl::CFileIoDeviceDVD::Destroy(int deleteStorage) {
    if (this != 0) {
        if (&rootPath.flags != 0 && (rootPath.flags >> 31) != 0)
            FileIO_Free(rootPath.data);
        if (deleteStorage > 0)
            FileIO_Free(this);
    }
    return this;
}

// Link the stream's runtime type to its base and initialize the unit's two-pi cache.
void gtl::nw4r::ut::DvdFileStream::InitializeTypeInfoAndConstants() {
    float value = kFloat_6_2831855;
    *(void**)&s_dvdFileStreamTypeInfo = &s_dvdFileStreamBaseTypeInfo;
    s_dvdFileStreamTwoPi = value;
}

extern "C" {

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
STATIC_INIT_ENTRY(".CRT$XCU80462714", _ctors_u80412664_0, gtl::nw4r::ut::DvdFileStream::InitializeTypeInfoAndConstants);
#pragma pop
