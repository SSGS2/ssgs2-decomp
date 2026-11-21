extern "C" {
// Borrowed 8-byte table row: a is the lookup key, b/c/d feed the five outputs.
// The row's three distinct values expand to {b,c,c,d,d}; their stat identities
// are not established. Output assignment narrows each short to an unsigned byte.
struct Result { short key, output0Value, outputs1And2Value, outputs3And4Value; };
// Player view: selected club byte at +0x2D; upgrade levels at +0x4AD + 2*club.
// Intervening bytes are skipped (club_custom_table.cpp writes this same level-byte offset).
struct Input { char pad[0x2d]; signed char selectedClubIndex; char gap[0x47f]; signed char upgradeLevelBytes[30]; };
// Non-owning handle: ptr is loaded table data, table is secondary dispatch metadata.
struct Handle { void* borrowedTableData; void* dispatchVtable; };
extern void* g_sharedFileForMenuData;
extern char __vt__14CClubCustomTbl[];
void* CSharedFilePtr_GetLoadedBuffer(void*);
Result* List_FindMatchingEntry(Handle*, short, int);
void __dl__FPv(void*);
// Optionally delete the handle's own storage; never release its borrowed table.
// deleteStorage > 0 frees the handle; return can therefore be a dangling address.
void* __dt__16ClubCustomHandleFv(void* handle, int deleteStorage) {
    if (handle && deleteStorage > 0) __dl__FPv(handle);
    return handle;
}
}

enum ClubCustomizationConstants {
    kClubCount = 15, kClubValueCount = 5, kClubPairBytes = 2,
    kLookupKeyStride = 4, kFirstTableEntry = 0
};

// Probe the shared provider without acquiring ownership of its loaded table.
inline bool MenuDataTableIsLoaded() { return CSharedFilePtr_GetLoadedBuffer(g_sharedFileForMenuData) != 0; }

// Expand the selected club's table row into five caller-owned customization bytes.
// Requires non-null input/output, club [0,14], value [0,4], and a matching row.
// The value read precedes club validation; rejected input or a missing row still
// reaches the unconditional result dereference. No failure result is provided.
// Valid indices busy-wait without a timeout; shared data must outlive the lookup.
extern "C" void ClubCustomization_GetFivePartValues(unsigned char* outValues, const Input* player) {
    Result* result;

    // Phase 1: preserve byte signedness and the existing validation/read order.
    unsigned char rawClubIndex = player->selectedClubIndex;
    int clubIndex; clubIndex = (signed char)rawClubIndex;
    char rawValue = player->upgradeLevelBytes[(kClubPairBytes * clubIndex)];
    int value; value = (signed char)rawValue;
    do {
        if (!(value <= kClubValueCount - 1 && (signed char)value >= 0)) { result = 0; break; }
        if (!(kClubCount > clubIndex && (signed char)clubIndex >= 0)) { result = 0; break; }
        // Phase 2: wait for data, then borrow it through a temporary dispatch handle.
        while (!MenuDataTableIsLoaded()) {}
        { Handle tableHandle;

            void* loadedTable = CSharedFilePtr_GetLoadedBuffer(g_sharedFileForMenuData);
            tableHandle.borrowedTableData = 0;
            if (loadedTable) tableHandle.borrowedTableData = loadedTable;
            tableHandle.dispatchVtable = __vt__14CClubCustomTbl;
            // Five values are accepted but the key stride is four. Value 4 shares
            // the next club's value-0 key; changing the stride changes behavior.
            result = List_FindMatchingEntry(&tableHandle, (short)(kLookupKeyStride * clubIndex + value), kFirstTableEntry);
        }
    } while (0);
    // Phase 3: copy the three row values to the five output positions, with repeats.
    outValues[0] = result->output0Value;
    outValues[1] = result->outputs1And2Value;
    outValues[2] = result->outputs1And2Value;
    outValues[3] = result->outputs3And4Value;
    outValues[4] = result->outputs3And4Value;
}

