// CCharacterPreviewTable: the character-preview records of a render context,
// followed by the number of records in use.
#ifndef CLASSES_CCHARACTERPREVIEWTABLE_H
#define CLASSES_CCHARACTERPREVIEWTABLE_H

#include "classes/CCharacterPreviewRecord.h"
#include "game/types.h"

class CCharacterPreviewTable {
public:
    // Replace the record's animation-name string and mark it for application to
    // the avatar. Returns 0 when the index is outside the table.
    int SetAnimationName(int index, const char* text);

    static int Update(void* base, float dt);
    static int DestroyRenderObjects(void* base);
    static void* GetRecordByIndex(void* base, int idx);
    static int SetupRecordAvatar(void* base, int idx, void* p, int flag, float f);
    static void DrawAvatars(void* base);
    static void SetScriptEnabled(void* base, int arg);

    CCharacterPreviewRecord records[4]; // 0x0
    int count;                          // 0x290
};

#define CCHARACTERPREVIEWTABLE_OFF(f, o) \
    typedef char CCharacterPreviewTable_off_##f[((int)&((CCharacterPreviewTable*)0)->f == (o)) ? 1 : -1]
CCHARACTERPREVIEWTABLE_OFF(records, 0x0);
CCHARACTERPREVIEWTABLE_OFF(count, 0x290);
#undef CCHARACTERPREVIEWTABLE_OFF
typedef char CCharacterPreviewTable_size[(sizeof(CCharacterPreviewTable) == 0x294) ? 1 : -1];

#endif // CLASSES_CCHARACTERPREVIEWTABLE_H
