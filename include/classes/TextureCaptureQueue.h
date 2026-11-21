// TextureCaptureQueue: provisional descriptive name; methods at 0x801EDE70, 0x801EE19C-0x801EE640.
// Layout-only view; no virtual declarations or generated RTTI.
#ifndef CLASSES_TEXTURECAPTUREQUEUE_H
#define CLASSES_TEXTURECAPTUREQUEUE_H

class TextureCaptureQueue {
public:
    void* resource; // 0x0: TextureCaptureQueue_EndDraw releases the reference after restoring texture state
    int captureCount; // 0x4: TextureCaptureQueue_AddTexture increments after inserting a capture
    void* headPrevious; // 0x8: TextureCaptureQueue_Init initializes the list sentinel at +8
    void* headNext; // 0xC: TextureCaptureQueue_Clear traverses from this link
    void* drawCursor; // 0x10: TextureCaptureQueue_BeginDraw resets; TextureCaptureQueue_DrawNextCapture advances to the next capture
    char pad_014[0x8];
    void* captureWidth;                // 0x1C: TextureCaptureQueue_AddTexture passes as first capture input to Texture_Create2D
    void* captureHeight;               // 0x20: TextureCaptureQueue_AddTexture passes as second capture input to Texture_Create2D
};

#define OFF(f, o) typedef char TextureCaptureQueue_off_##f[((int)&((TextureCaptureQueue*)0)->f == (o)) ? 1 : -1]
OFF(resource, 0x0);
OFF(captureCount, 0x4);
OFF(headPrevious, 0x8);
OFF(headNext, 0xC);
OFF(drawCursor, 0x10);
OFF(captureWidth, 0x1C);
OFF(captureHeight, 0x20);
#undef OFF

#endif
