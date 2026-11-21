// ResourceModel801B: layout view owned by player_pet_model_resources.cpp.
// Evidence: AvatarModel_LoadCharacterResources; AvatarModel_ReloadPartResources; ResourceModel_SetMatrixRecursive; ObjectList_GetSelected. No virtual declarations or runtime objects.
#ifndef CLASSES_RESOURCEMODEL801B_H
#define CLASSES_RESOURCEMODEL801B_H

class ResourceModel801B {
public:
    char pad_0[0x4];
    unsigned int key; // 0x4; AvatarModel_LoadCharacterResources
    void* model; // 0x8; AvatarModel_LoadCharacterResources
    char pad_C[0xC];
    void* resource; // 0x18; AvatarModel_LoadCharacterResources
    char pad_1C[0x4];
    int entryCount; // 0x20; AvatarModel_LoadCharacterResources
    char pad_24[0x4];
    void* selected; // 0x28; AvatarModel_LoadCharacterResources
    unsigned int primaryPartCount; // 0x2C; AvatarModel_LoadCharacterResources
    unsigned int resourceCount; // 0x30; AvatarModel_LoadCharacterResources
    char pad_34[4];
    void* partModels[24]; // 0x38: PlayerModel_ReleaseResourcesByMask releases models and clears 0x60 bytes
    void* partTextures[72]; // 0x98: AvatarModel_ReloadPartResources retains three bitmap IDs per part
    unsigned char unk_1B8; // 0x1B8; AvatarModel_LoadCharacterResources
    char pad_1B9[0xCB];
    char copiedParameters[0x14]; // 0x284: AvatarModel_LoadCharacterResources copies optional 0x14-byte input, else zeros
};

#define OFF(f, o) typedef char ResourceModel801B_off_##f[((int)&((ResourceModel801B*)0)->f == (o)) ? 1 : -1]
OFF(key, 0x4);
OFF(model, 0x8);
OFF(resource, 0x18);
OFF(entryCount, 0x20);
OFF(selected, 0x28);
OFF(primaryPartCount, 0x2C);
OFF(resourceCount, 0x30);
OFF(unk_1B8, 0x1B8);
OFF(partModels, 0x38);
OFF(partTextures, 0x98);
OFF(copiedParameters, 0x284);
#undef OFF

#endif
