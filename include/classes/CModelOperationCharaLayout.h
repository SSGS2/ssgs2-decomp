// Layout view avoids the native CModelOperationChara constructor declaration in model_operation_chara_caddie.cpp.
#ifndef CLASSES_CMODELOPERATIONCHARALAYOUT_H
#define CLASSES_CMODELOPERATIONCHARALAYOUT_H

class CModelOperationCharaLayout {
public:
    char pad_000[0x10];
    void* exhibition; // 0x10: CModelOperationChara_SetHairStyle: virtual model operation target
    void* saveRecord; // 0x14: CModelOperationChara_WriteEquipmentToSave / CModelOperationChara_BuildEquipmentFromSave: saved character selection data
    void* asyncAvatar; // 0x18: CModelOperationChara_ToggleAttachment: operation receiver; constructor clears it
    char operation[0x68]; // 0x1c: constructor calls ItemSelection_Init; ItemSelection_BuildOutfitFromRecord / ItemSelection_Validate operate on it
    int selectedItemIndex; // 0x84: CModelOperationChara_SetHairStyle stores ItemInventory_FindHairStyleIndex result; CModelOperationChara_WriteEquipmentToSave writes save +0x2FE
    int toggledItemIndex; // 0x88: CModelOperationChara_ToggleAttachment toggles inventory lookup result against -1; CModelOperationChara_WriteEquipmentToSave writes save +0x2FF
};

#define CMODELOPERATIONCHARALAYOUT_OFF(f, o) typedef char CModelOperationCharaLayout_off_##f[((int)&((CModelOperationCharaLayout*)0)->f == (o)) ? 1 : -1]
CMODELOPERATIONCHARALAYOUT_OFF(exhibition, 0x10);
CMODELOPERATIONCHARALAYOUT_OFF(saveRecord, 0x14);
CMODELOPERATIONCHARALAYOUT_OFF(asyncAvatar, 0x18);
CMODELOPERATIONCHARALAYOUT_OFF(selectedItemIndex, 0x84);
CMODELOPERATIONCHARALAYOUT_OFF(toggledItemIndex, 0x88);
CMODELOPERATIONCHARALAYOUT_OFF(operation, 0x1c);
#undef CMODELOPERATIONCHARALAYOUT_OFF

#endif
