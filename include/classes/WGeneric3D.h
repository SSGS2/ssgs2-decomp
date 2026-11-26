// WGeneric3D: declaration moved unchanged from wgeneric3d_render_queue_and_soft_keyboard.cpp.
// Offsets: WGeneric3D_ctor initializes these fields; WGeneric3D_Flush resets queue counters.
#ifndef CLASSES_WGENERIC3D_H
#define CLASSES_WGENERIC3D_H
struct WGeneric3D {
    virtual void slot0() = 0;
    virtual void slot1() = 0;
    virtual void slot2() = 0;
    virtual void slot3() = 0;
    virtual void slot4() = 0;
    virtual void slot5() = 0;
    virtual void slot6() = 0;
    virtual void slot7() = 0;
    virtual void slot8() = 0;
    virtual void slot9() = 0;
    virtual void dispose(unsigned int) = 0;
    virtual void slot11() = 0;
    virtual void slot12() = 0;
    virtual void slot13() = 0;
    virtual void slot14() = 0;
    virtual void slot15() = 0;
    virtual void v16(int) = 0;
    virtual void slot17() = 0;
    virtual void slot18() = 0;
    virtual void slot19() = 0;
    virtual void slot20() = 0;
    virtual void slot21() = 0;
    virtual void slot22() = 0;
    virtual void slot23() = 0;
    virtual void slot24() = 0;
    virtual void slot25() = 0;
    virtual void* slot26(int) = 0;
    virtual void* slot27(int) = 0;
    virtual void slot28() = 0;
    virtual void slot29() = 0;
    virtual void slot30() = 0;
    virtual void Execute(int, int, int, int, int, int) = 0;
    virtual void slot32() = 0;
    virtual void Flush(int) = 0;
    virtual void slot34() = 0;
    virtual void slot35(int) = 0;
    virtual void slot36(int) = 0;
    virtual void slot37() = 0;
    virtual void slot38() = 0;
    virtual void slot39(int, int, int, int, int, int, int) = 0;
    virtual void slot40(int, int) = 0;
    virtual void slot41(int, void*, void*, int) = 0;

    // Non-virtual bodies of vtable slots 21, 22, 26 and 27.
    int vfunc_21();
    unsigned int GetRenderEntryCount();
    int vfunc_26();
    int vfunc_27();

    char unk04[0x0C];
    unsigned int separateTexturePassesEnabled;
    char unk14[0x08];
    void* vertexBuffer;
    unsigned int vertexBufferBytesUsed;
    void* queue[0x100];
    char entries[0x2000];
    unsigned int nextRenderSlot;
    unsigned int nextReservedRenderSlot;
    unsigned int items[0x800];
    unsigned char colors[0x400];
    unsigned char colorRampEndTransparency;
    float colorRampStart;
    float colorRampEnd;
    float colorRampInverseSpan;
    unsigned int renderEntryCount;
    unsigned int queuedEntryCount;
    unsigned int submissionState;
    unsigned char fogEnabled;
    char unk4849[3];
    unsigned int colorRampRgb;
    unsigned int value4850;
    float value4854;
    float value4858;
    unsigned int value485C;
    unsigned int resourceOverride;
    unsigned int usePositionMode1;
};
#define OFF(f, o) typedef char WGeneric3D_off_##f[((int)&((WGeneric3D*)0)->f == (o)) ? 1 : -1]
OFF(separateTexturePassesEnabled, 0x10);
OFF(vertexBuffer, 0x1C);
OFF(vertexBufferBytesUsed, 0x20);
OFF(queue, 0x24);
OFF(entries, 0x424);
OFF(nextRenderSlot, 0x2424);
OFF(nextReservedRenderSlot, 0x2428);
OFF(items, 0x242C);
OFF(colors, 0x442C);
OFF(colorRampEndTransparency, 0x482C);
OFF(colorRampStart, 0x4830);
OFF(colorRampEnd, 0x4834);
OFF(colorRampInverseSpan, 0x4838);
OFF(renderEntryCount, 0x483C);
OFF(queuedEntryCount, 0x4840);
OFF(submissionState, 0x4844);
OFF(fogEnabled, 0x4848);
OFF(colorRampRgb, 0x484C);
OFF(value4850, 0x4850);
OFF(value4854, 0x4854);
OFF(value4858, 0x4858);
OFF(value485C, 0x485C);
OFF(resourceOverride, 0x4860);
OFF(usePositionMode1, 0x4864);
#undef OFF
typedef char WGeneric3D_size[(sizeof(WGeneric3D) == 0x4868) ? 1 : -1];
#define AsWGeneric3D(p) ((WGeneric3D*)(p))
#endif
