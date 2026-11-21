#include <decomp/static_init.h>
#include <decomp/offset_of.h>
// Translation unit 0x802BAAD0-0x802BACA4 (11 functions; a bundle of decomp-toolkit's units; the original file edges inside it are unknown).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

#pragma push
#pragma use_lmw_stmw on

extern "C" {
extern "C" float kFloat_6_2831855; // 2*pi, shared by the static initializers below.
extern "C" float g_clubCustomTableStaticTwoPi;
extern "C" float g_clubCustomLookupStaticTwoPi;

extern void* g_sharedFileForMenuData;
extern char __vt__14CClubCustomTbl[];
void* CSharedFilePtr_GetLoadedBuffer(void*);
void* Table_GetEntry(void*, int);
void* __dt__16ClubCustomHandleFv(void*, int);
void __dl__FPv(void*);

// Borrowed table at +0, dispatch vtable at +4; destruction releases neither.
// tableData borrows the table; the secondary dispatch pointer follows at +4.
struct ClubCustomHandle {
    void* tableData;
    void* vtable;
    // Leave the borrowed table and its dispatch metadata intact.
    ~ClubCustomHandle() {}
};

// Return true from table virtual slot 1; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_1() { return 1; }
// Return true from table virtual slot 2; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_2() { return 1; }
// Return true from table virtual slot 3; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_3() { return 1; }

// Return true from table virtual slot 4; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_4() { return 1; }
// Return true from table virtual slot 5; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_5() { return 1; }
// Return true from table virtual slot 6; its slot-specific contract is unresolved.
int CClubCustomTbl_vfunc_6() { return 1; }

// Initialize the first club-customization module's floating-point static.
void ClubCustomTable_InitializeStaticAngle() { g_clubCustomTableStaticTwoPi = kFloat_6_2831855; }
// Initialize the second club-customization module's floating-point static.
void ClubCustomLookup_InitializeStaticAngle() { g_clubCustomLookupStaticTwoPi = kFloat_6_2831855; }

enum { kUpgradeLevelCount = 5, kClubCount = 15, kClubUpgradeKeyStride = 4 };
struct TableHeader { void* entries; int reserved04; int count; };
struct TableEntryKey { short key; };
// The table pointer precedes the secondary vtable at +4 in ClubCustomHandle.
struct TableFilterBase { TableHeader* table; };
class TableFilter : public TableFilterBase {
public:
    virtual void reserved08(int);
    virtual void reserved0C();
    virtual int acceptEntry(void*, int);
};

// Return a borrowed entry matching key and the handle's predicate, or null.
// startIndex is inclusive and must be nonnegative; null entries stop the search.
// handle is a non-null {table pointer, secondary vtable} pair, not an owning list.
void* List_FindMatchingEntry(void* handle, short key, int startIndex) {
    TableHeader* list = *(TableHeader**)handle;
    int count;
    if (list != 0) count = list->count;
    else count = 0;
    void* entry;
    int index = startIndex;
    while (index < count) {
        entry = Table_GetEntry(*(void**)handle, index);
        if (entry == 0) break;
        if (((TableEntryKey*)entry)->key == key) {
            int accepted = ((TableFilter*)handle)->acceptEntry(entry, index);
            if (accepted != 0) return entry;
        }
        index++;
    }
    return 0;
}

// Probe whether the shared menu-data provider currently exposes a loaded buffer.
inline bool ClubUpgradeTableIsLoaded() { return CSharedFilePtr_GetLoadedBuffer(g_sharedFileForMenuData) != 0; }

// Return a borrowed upgrade record for club [0,14] and level [0,4], or null.
// Valid inputs busy-wait for menu data with no timeout; its buffer must outlive use.
void* CClubCustomTbl_FindUpgradeEntry(signed char clubIndex, signed char upgradeIndex) {
    // Reject unsupported indices before touching the shared provider.
    if (upgradeIndex > kUpgradeLevelCount - 1 || upgradeIndex < 0) return 0;
    if (clubIndex >= kClubCount || clubIndex < 0) return 0;
    while (!ClubUpgradeTableIsLoaded()) {}
    // Build a temporary, non-owning dispatch handle over the now-loaded table.
    void* object = CSharedFilePtr_GetLoadedBuffer(g_sharedFileForMenuData);
    ClubCustomHandle h;
    h.tableData = 0;
    if (object) h.tableData = object;
    h.vtable = __vt__14CClubCustomTbl;
    // Key = 4*club + level, despite five accepted levels. Level 4 therefore shares
    // a key with level 0 of the next club; do not change the stride to five.
    return List_FindMatchingEntry(&h, (short)(upgradeIndex + clubIndex * kClubUpgradeKeyStride), 0);
}

struct ClubUpgradeBytes { unsigned char value; unsigned char level; };
struct PlayerClubUpgradeFields { char precedingFields[0x4AC]; ClubUpgradeBytes upgrades[15]; };
typedef char ClubUpgradeLevelOffset[((int)DECOMP_OFFSETOF(PlayerClubUpgradeFields, upgrades[0].level) == 0x4AD) ? 1 : -1];

// Store only the second byte of a player's club pair; the first byte is preserved.
// playerData is borrowed writable storage; clubIndex must be [0,14]. No level clamp.
void PlayerData_SetClubUpgradeLevel(signed char clubIndex, unsigned char level, void* playerData) {
    ((PlayerClubUpgradeFields*)playerData)->upgrades[clubIndex].level = level;
}
} // extern "C"

#pragma pop

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804624CC", _ctors_u802BAAD0_0, ClubCustomTable_InitializeStaticAngle);
STATIC_INIT_ENTRY(".CRT$XCU804624CC", _ctors_u802BAAD0_1, ClubCustomLookup_InitializeStaticAngle);
#pragma pop
