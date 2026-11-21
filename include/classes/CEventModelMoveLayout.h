// CEventModelMoveLayout: layout view for CEventModelSC_MDL_MOVE in event_script_model_commands.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CEVENTMODELMOVELAYOUT_H
#define CLASSES_CEVENTMODELMOVELAYOUT_H

class CEventModelMoveLayout {
public:
    void* vtable; // 0x0: constructor installs __vt__22CEventModelSC_MDL_MOVE
    unsigned int count; // 0x4: constructor scales duration; vfunc_1 divides elapsed time
    void* object; // 0x8: constructor resolves SlotTable entry
    float startPosition[3]; // 0xC: constructor copies object +0x1C; vfunc_1 adds scaled delta
    float deltaPosition[3]; // 0x18: constructor optionally subtracts start; vfunc_1 scales
};

#define OFF(f, o) typedef char CEventModelMoveLayout_off_##f[((int)&((CEventModelMoveLayout*)0)->f == (o)) ? 1 : -1]
OFF(vtable, 0x0);
OFF(count, 0x4);
OFF(object, 0x8);
OFF(startPosition, 0xC);
OFF(deltaPosition, 0x18);
#undef OFF

#endif
