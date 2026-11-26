// Original helper ModeRecordUiContext_Initialize initializes this state; semantics of these integer slots are unresolved.
#ifndef CLASSES_OPTIONSTATE802EB540_H
#define CLASSES_OPTIONSTATE802EB540_H

class OptionState802EB540 {
public:
    int mapFile;           // 0x0: ModeRecordUiContext_Initialize initializes 0
    int itemFile;          // 0x4: ModeRecordUiContext_Initialize initializes 0
    int messageFile;       // 0x8: ModeRecordUiContext_Initialize initializes 0
    int cameraController;  // 0xC: ModeRecordUiContext_Initialize initializes 0
    int previewTreeHeader; // 0x10: ModeRecordUiContext_Initialize initializes 0
    char pad_014[0x8];
    int tourWindow;            // 0x1C: ModeRecordUiContext_Initialize initializes 0
    int tourHintWindow;        // 0x20: ModeRecordUiContext_Initialize initializes 0
    int tourVsWindow;          // 0x24: ModeRecordUiContext_Initialize initializes 0
    int particleEffects;       // 0x28: ModeRecordUiContext_Initialize initializes 0
    int messageResourceHolder; // 0x2C: ModeRecordUiContext_Initialize initializes 0
    int flashMemorySaveMenu;   // 0x30: ModeRecordUiContext_Initialize initializes 0
    int mapData;               // 0x34: ModeRecordUiContext_Initialize initializes 0
    int coinItemData;          // 0x38: ModeRecordUiContext_Initialize initializes 0
    int unk_3C;                // 0x3C: ModeRecordUiContext_Initialize initializes 0
    int shop;                  // 0x40: ModeRecordUiContext_Initialize initializes 0
    int numberDraw;            // 0x44: ModeRecordUiContext_Initialize initializes 0
    char pad_048[0xC];
    int mapPackedFile;  // 0x54: ModeRecordUiContext_Initialize initializes 0
    int itemPackedFile; // 0x58: ModeRecordUiContext_Initialize initializes 0
    int mapTexture;     // 0x5C: ModeRecordUiContext_Initialize initializes 0
    int itemTexture;    // 0x60: ModeRecordUiContext_Initialize initializes 0
    char pad_064[0x8];
    int modeGroup;   // 0x6C: ModeRecordUiContext_Initialize initializes -1
    int itemGroup;   // 0x70: ModeRecordUiContext_Initialize initializes -1
    int numberGroup; // 0x74: ModeRecordUiContext_Initialize initializes -1
    int layout;      // 0x78: ModeRecordUiContext_Initialize initializes 0
};

#define OPTIONSTATE802EB540_OFF(f, o) \
    typedef char OptionState802EB540_off_##f[((int)&((OptionState802EB540*)0)->f == (o)) ? 1 : -1]
OPTIONSTATE802EB540_OFF(mapFile, 0x0);
OPTIONSTATE802EB540_OFF(itemFile, 0x4);
OPTIONSTATE802EB540_OFF(messageFile, 0x8);
OPTIONSTATE802EB540_OFF(cameraController, 0xC);
OPTIONSTATE802EB540_OFF(previewTreeHeader, 0x10);
OPTIONSTATE802EB540_OFF(tourWindow, 0x1C);
OPTIONSTATE802EB540_OFF(tourHintWindow, 0x20);
OPTIONSTATE802EB540_OFF(tourVsWindow, 0x24);
OPTIONSTATE802EB540_OFF(particleEffects, 0x28);
OPTIONSTATE802EB540_OFF(messageResourceHolder, 0x2C);
OPTIONSTATE802EB540_OFF(flashMemorySaveMenu, 0x30);
OPTIONSTATE802EB540_OFF(mapData, 0x34);
OPTIONSTATE802EB540_OFF(coinItemData, 0x38);
OPTIONSTATE802EB540_OFF(unk_3C, 0x3C);
OPTIONSTATE802EB540_OFF(shop, 0x40);
OPTIONSTATE802EB540_OFF(numberDraw, 0x44);
OPTIONSTATE802EB540_OFF(mapPackedFile, 0x54);
OPTIONSTATE802EB540_OFF(itemPackedFile, 0x58);
OPTIONSTATE802EB540_OFF(mapTexture, 0x5C);
OPTIONSTATE802EB540_OFF(itemTexture, 0x60);
OPTIONSTATE802EB540_OFF(modeGroup, 0x6C);
OPTIONSTATE802EB540_OFF(itemGroup, 0x70);
OPTIONSTATE802EB540_OFF(numberGroup, 0x74);
OPTIONSTATE802EB540_OFF(layout, 0x78);
#undef OPTIONSTATE802EB540_OFF

#endif
