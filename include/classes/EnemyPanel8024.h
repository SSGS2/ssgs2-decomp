// EnemyPanel8024: layout view owned by chara_entry_screen_part1.cpp.
// Evidence: CCharaEntry_RefreshPlayerRow; CCharaEntry_SetEnemyLevelSelection pass +0x2c to Layout_*. No virtual declarations or runtime objects.
#ifndef CLASSES_ENEMYPANEL8024_H
#define CLASSES_ENEMYPANEL8024_H

class EnemyPanel8024 {
public:
    char pad_0[0x2C];
    void* layout; // 0x2C; CCharaEntry_RefreshPlayerRow
};

#define OFF(f, o) typedef char EnemyPanel8024_off_##f[((int)&((EnemyPanel8024*)0)->f == (o)) ? 1 : -1]
OFF(layout, 0x2C);
#undef OFF

#endif
