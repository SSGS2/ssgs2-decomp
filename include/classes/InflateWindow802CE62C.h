// InflateWindow: layout view from Zlib_InflateFlushWindow (0x802CE62C); provisional type name.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_INFLATEWINDOW802CE62C_H
#define CLASSES_INFLATEWINDOW802CE62C_H

struct InflateWindow {
    char pad_000[0x28];
    unsigned char* windowBegin; // 0x28: Zlib_InflateFlushWindow wraps read cursor here
    unsigned char* windowEnd;   // 0x2c: Zlib_InflateFlushWindow detects wrap
    unsigned char* readCursor;  // 0x30: Zlib_InflateFlushWindow advances after copies
    unsigned char* writeCursor; // 0x34: Zlib_InflateFlushWindow limits readable range
    void* checksumCallback;     // 0x38: Zlib_InflateFlushWindow indirect call over copied bytes
    void* checksum;             // 0x3c: Zlib_InflateFlushWindow stores callback result
};

#define OFF(f, o) typedef char InflateWindow_off_##f[((int)&((InflateWindow*)0)->f == (o)) ? 1 : -1]
OFF(windowBegin, 0x28);
OFF(windowEnd, 0x2c);
OFF(readCursor, 0x30);
OFF(writeCursor, 0x34);
OFF(checksumCallback, 0x38);
OFF(checksum, 0x3c);
#undef OFF
#define AsInflateWindow(p) ((InflateWindow*)(p))

#endif
