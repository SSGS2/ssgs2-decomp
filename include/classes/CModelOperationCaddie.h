// CModelOperationCaddie: the caddie-preview operation used by the shops. It
// opens the shared caddie preview resource, creates the pet preview, and maps
// the selected caddie and its equipment to that preview.
#ifndef CLASSES_CMODELOPERATIONCADDIE_H
#define CLASSES_CMODELOPERATIONCADDIE_H

#include "game/types.h"

class CModelOperationCaddie {
public:
    int OpenResource();
    int CreatePetPreview();
    int SelectCaddie(unsigned int caddieTypeId);
    int SetItem(int typeId);
    int TryToggleItem(int itemTypeId);
    int ToggleItem(unsigned int typeId);

    CModelOperationCaddie();
    ~CModelOperationCaddie();

    void* vtbl;         // 0x0: original vtable pointer; the shared operation base starts here
    char pad_004[0xc];  // 0x4
    void* exhibition;   // 0x10
    void* saveRecord;   // 0x14
    void* asyncPet;     // 0x18
    int inventoryIndex; // 0x1c
};

#define CMODELOPERATIONCADDIE_OFF(f, o) \
    typedef char CModelOperationCaddie_off_##f[((int)&((CModelOperationCaddie*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONCADDIE_OFF(exhibition, 0x10);
CMODELOPERATIONCADDIE_OFF(saveRecord, 0x14);
CMODELOPERATIONCADDIE_OFF(asyncPet, 0x18);
CMODELOPERATIONCADDIE_OFF(inventoryIndex, 0x1c);
#undef CMODELOPERATIONCADDIE_OFF

#endif // CLASSES_CMODELOPERATIONCADDIE_H
