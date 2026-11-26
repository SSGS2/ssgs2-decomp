// GtlFileIoDeviceDVD::ReadFile: a synchronous DVD read at a byte offset. A fatal drive status makes it
// wait and then fail, and a cancelled read is reported as an error.

#include "classes/GtlFileIoDeviceDVD.h"

extern "C" {

int File_SetLastError(int);
void OSSleepTicks(unsigned long long);
int DVDReadPrio(void*, void*, int, int, int);
#include "sdk_protos.h"

enum {
    kInvalidDvdFile = 0xFFFFFFFFu,
    kDvdReadPriority = 2,
    kDvdFatalError = -1,
    kDvdCancelled = -3,
    kInvalidHandleError = 6,
    kReadFaultError = 0x1E,
    kCancelledError = 0x4C7,
    kNoError = 0,
    kBusClockAddress = 0x800000F8,
    kBusToTimerClockShift = 2,
    kSleepClockMultiplier = 0x431BDE83,
    kSleepProductShift = 15,
    kSleepScaleNumerator = 30,
    kSleepScaleDenominator = 8
};

} // extern "C"

// Read synchronously at a byte offset and report the SDK byte count through bytesRead.
// The device is unused. fileHandle is a borrowed DVD file-info address encoded as a word;
// only -1 is rejected. dst/length/offset must meet DVDReadPrio's buffer/alignment rules.
// Returns 1 for all results except -1/-3; optional bytesRead is untouched on those
// failures. A fatal read waits while drive status is fatal (-1), then fails WITHOUT retrying.
#pragma use_lmw_stmw on
int gtl::CFileIoDeviceDVD::ReadFile(unsigned int fileHandle, void* dst, int length, int* bytesRead, int offset) {
    int readResult;
    unsigned int timerClock;
    unsigned int scaledClock;
    long long sleepTicks;

    if (fileHandle == kInvalidDvdFile) {
        File_SetLastError(kInvalidHandleError);
        return 0;
    }

    readResult = DVDReadPrio((void*)fileHandle, dst, length, offset, kDvdReadPriority);
    if (readResult == kDvdFatalError) {
        File_SetLastError(kReadFaultError);
        while (DVDGetDriveStatus() == kDvdFatalError) {
            // Derive the sleep count from the bus clock's quarter-rate timer.
            // mulhwu followed by srwi 15 discards 47 product bits in total.
            // Keep the following multiply/divide in signed 64-bit arithmetic.
            timerClock = *(volatile unsigned int*)kBusClockAddress >> kBusToTimerClockShift;
            scaledClock =
                (unsigned int)(((unsigned long long)timerClock * (unsigned long long)kSleepClockMultiplier) >> 32) >>
                kSleepProductShift;
            sleepTicks = ((long long)scaledClock * kSleepScaleNumerator) / kSleepScaleDenominator;
            OSSleepTicks((unsigned long long)sleepTicks);
        }
        return 0;
    }

    if (readResult == kDvdCancelled) {
        File_SetLastError(kCancelledError);
        return 0;
    }
    if (bytesRead != 0) {
        *bytesRead = readResult;
    }
    File_SetLastError(kNoError);
    return 1;
}
#pragma use_lmw_stmw reset
