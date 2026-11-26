// The character-preview operation used by the shops. It opens the shared
// character preview resource, creates an avatar preview, and maps the saved
// outfit to item changes on that preview.
//
// The class has no virtual declarations: its vtable comes from the original
// data, so the base spells out the table pointer as its first member.
#ifndef CLASSES_CMODELOPERATIONCHARA_H
#define CLASSES_CMODELOPERATIONCHARA_H

#include "classes/CModelOperation.h"
#include "game/types.h"

class CAsyncExhibitionAvatar;

class CModelOperationChara : public CModelOperation {
public:
    // The outfit selection that the item-selection helpers build and validate:
    // a character id followed by 24 part ids.
    struct ItemSelection {
        unsigned int characterId;     // 0x00
        unsigned char primaryCount;   // 0x04
        unsigned char totalCount;     // 0x05
        unsigned char textureVariant; // 0x06
        unsigned char bitmapVariant;  // 0x07
        unsigned int partIds[24];     // 0x08
    };

    int OpenResource();
    int CreateAvatarPreview();
    int ApplyCharacterSelection(void* key);
    int BuildEquipmentFromSave(void* output, int characterIndex);
    int ApplyPartChanges(unsigned int partTypeId);
    int RequestModelUpdate(int modelIndex);
    int ToggleAttachment(int unused, unsigned int auxPartTypeId);
    int SetHairStyle(unsigned int hairStyleTypeId);
    int WriteEquipmentToSave();

    CModelOperationChara();
    ~CModelOperationChara();

    CAsyncExhibitionAvatar* asyncAvatar; // 0x18: avatar preview, or null
    ItemSelection selection;             // 0x1c
    int selectedItemIndex;               // 0x84: inventory index of the hair style, or -1
    int toggledItemIndex;                // 0x88: inventory index of the auxiliary part, or -1
};

#define CMODELOPERATIONCHARA_OFF(f, o) \
    typedef char CModelOperationChara_off_##f[((int)&((CModelOperationChara*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONCHARA_OFF(asyncAvatar, 0x18);
CMODELOPERATIONCHARA_OFF(selection, 0x1c);
CMODELOPERATIONCHARA_OFF(selectedItemIndex, 0x84);
CMODELOPERATIONCHARA_OFF(toggledItemIndex, 0x88);
#undef CMODELOPERATIONCHARA_OFF
typedef char CModelOperationChara_size[(sizeof(CModelOperationChara) == 0x8C) ? 1 : -1];
typedef char CModelOperationChara_selection_size[(sizeof(CModelOperationChara::ItemSelection) == 0x68) ? 1 : -1];

#endif // CLASSES_CMODELOPERATIONCHARA_H
