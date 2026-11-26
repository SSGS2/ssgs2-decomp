// CEventFx position view. Keeps the existing CEventFx storage and field names intact.
#ifndef CLASSES_CEVENTFXPOSITIONLAYOUT_H
#define CLASSES_CEVENTFXPOSITIONLAYOUT_H

class CEventFxPositionLayout {
public:
    char pad_0[0x1C];
    float position[3];     // 0x1C: CEventFx::Init passes position to effect creation
    float basePosition[3]; // 0x28: CEventFx::Init copies optional vector; adds displacement
};

#define OFF(f, o) typedef char CEventFxPositionLayout_off_##f[((int)&((CEventFxPositionLayout*)0)->f == (o)) ? 1 : -1]
OFF(position, 0x1C);
OFF(basePosition, 0x28);
#undef OFF

#endif
