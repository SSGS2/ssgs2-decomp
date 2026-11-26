// CEnemyRenderTable: layout view for enemy-render table in character_preview_avatars.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CENEMYRENDERTABLE_H
#define CLASSES_CENEMYRENDERTABLE_H

class CEnemyRenderTable {
public:
    char recordStorage[0x290]; // 0x0: four 0xA4-byte records; CCharacterPreviewTable::GetRecordByIndex
    int count; // 0x290: CCharacterPreviewTable::SetAnimationName and update/destructor bound record iteration
};

#define OFF(f, o) typedef char CEnemyRenderTable_off_##f[((int)&((CEnemyRenderTable*)0)->f == (o)) ? 1 : -1]
OFF(recordStorage, 0x0);
OFF(count, 0x290);
#undef OFF

#endif
