// CEventModelRotateLayout: layout view for CEventModelSC_MDL_ROT in event_script_model_commands.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CEVENTMODELROTATELAYOUT_H
#define CLASSES_CEVENTMODELROTATELAYOUT_H

class CEventModelRotateLayout {
public:
    void* vtable; // 0x0: event header
    unsigned int count; // 0x4: vfunc_1 divides elapsed time
    void* object; // 0x8: vfunc_1 writes object angles
    float startX; // 0xC: vfunc_1 interpolates wrapped angle
    float startY; // 0x10: vfunc_1 interpolates wrapped angle
    float startZ; // 0x14: vfunc_1 interpolates wrapped angle
    float deltaX; // 0x18: vfunc_1 multiplies by time fraction
    float deltaY; // 0x1C: vfunc_1 multiplies by time fraction
    float deltaZ; // 0x20: vfunc_1 multiplies by time fraction
};

#define OFF(f, o) typedef char CEventModelRotateLayout_off_##f[((int)&((CEventModelRotateLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(count, 0x4);
OFF(object, 0x8);
OFF(startX, 0xC);
OFF(startY, 0x10);
OFF(startZ, 0x14);
OFF(deltaX, 0x18);
OFF(deltaY, 0x1C);
OFF(deltaZ, 0x20);
#undef OFF

#endif
