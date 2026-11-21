// Controller owned by Layout_Init (0x80077DB4) and Layout_Destroy (0x80077E2C).
// This is distinct from LayoutPointerCursor, whose first words are cursor bounds.
#ifndef CLASSES_LAYOUTCONTROLLER800779EC_H
#define CLASSES_LAYOUTCONTROLLER800779EC_H

struct LayoutResourceOwnerView {
    char pad_0[4];
    void* ownedObject; // +0x4: Layout_Destroy invokes its virtual destructor.
    unsigned int standardItems;
    char pad_C[4];
    unsigned int cachedPointerX0Bits; // Layout_Init clears this word through Memory_Clear.
    char pad_14[0x1C];
    unsigned int loadState;
    unsigned int inputModeRegistered; // +0x34: nonzero makes Layout_Destroy release GameInputManager +0x3C8.
    char pad_38[4];
    unsigned char activationSoundEnabled; // Layout_Init initializes to one; meaning not established.
    char pad_3D[3];
    void* pangyaIcons; // +0x40: destroyed as an array of PANGYA_ICON_OBJECT_VAR.
    void* sharedFile; // +0x44: CSharedFileHandle_Destroy.
    void* activationActionId; // +0x48: Layout_Init's second argument; role not established.
    unsigned int extendedFormat;
    unsigned int extendedItemTable;
    unsigned int extendedItems;
    void* secondarySharedFile; // +0x58: CSharedFileHandle_Destroy.
};

#define OFF(f, o) typedef char LayoutController800779EC_off_##f[((int)&((LayoutResourceOwnerView*)0)->f == (o)) ? 1 : -1]
OFF(ownedObject, 0x4);
OFF(standardItems, 0x8);
OFF(cachedPointerX0Bits, 0x10);
OFF(loadState, 0x30);
OFF(inputModeRegistered, 0x34);
OFF(activationSoundEnabled, 0x3C);
OFF(pangyaIcons, 0x40);
OFF(sharedFile, 0x44);
OFF(activationActionId, 0x48);
OFF(extendedFormat, 0x4C);
OFF(extendedItemTable, 0x50);
OFF(extendedItems, 0x54);
OFF(secondarySharedFile, 0x58);
#undef OFF
#define AsLayoutController800779EC(p) ((LayoutResourceOwnerView*)(p))
#endif
