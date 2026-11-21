#include <decomp/static_init.h>
#include "iff/IffStruct.h"
#include "classes/CSharedDoc.h"
#include "classes/CGolfDoc.h"
// Borrow the document layout; this conversion neither allocates nor retains it.
// Retained as a cast macro because inline_depth(0) below emits a call for a helper.
#define AsCSharedDoc(p) ((CSharedDoc*)(p))
// Translation unit 0x801F44A4-0x801F5570 (12 functions; boundaries inferred by decomp-toolkit from extab).
// Function names: config/R2PE9B/symbols.txt. Plain names go inside extern "C".

extern "C" {

extern float kFloat_6_2831855;
extern float s_twoPi_801F44A4;

// Local views expose storage that is still padding in the shared class header.
struct SharedPlayerStateStorage {
    char objectHeader[0x10];
    char playerState[0x750];
    unsigned char stateValid;
};

struct PlayerItemState {
    char beforeItems[0x8C];
    int itemIds[8];
    char afterItems[0x1D4 - 0xAC];
};

struct SharedCourseState {
    char beforeCourseId[0x8B8];
    unsigned char encodedCourseId;
    char beforeMapName[0x9E4 - 0x8B9];
    char mapResourceName[0x20];
};
typedef char PlayerItemState_stride_check[(sizeof(PlayerItemState) == 0x1D4) ? 1 : -1];
typedef char SharedCourseState_map_offset_check[((int)&((SharedCourseState*)0)->mapResourceName == 0x9E4) ? 1 : -1];

// Item-type tag (high byte) OR'd into a course id to form the ItemInventory lookup key.
enum ItemInventoryKeyTag { kCourseItemType = 0x28000000 };
enum MapResourceNameConstants { kMapResourceNameLength = 0x20 };

enum CourseEncoding {
    kCourseVariantBit = 0x80, kBaseCourseMask = 0x7F, kUnselectedCourse = 0xFD,
    kCourseVariantMask = 0xF,
    kFirstVariant = 1, kSecondVariant = 2, kPlayerCount = 4, kItemSlotCount = 8,
    kLastInventorySlot = 12,
    kSpecialCourseEight = 8, kSpecialCourseThirteen = 13
};

// Return the task hook's unchanged zero status; there is no work or ownership transfer.
int GolfTask_NoOpStatus() {
    return 0;
}

// Initialize this unit's cached two-pi constant.
void SharedDoc_InitializeAngleConstant() {
    s_twoPi_801F44A4 = kFloat_6_2831855;
}

// Invalidate the shared player-state storage after a reset.
void SharedDoc_InvalidatePlayerState(SharedPlayerStateStorage* storage) {
    storage->stateValid = 0;
}

extern CSharedDoc* g_sharedDoc;

void* memset(void*, int, unsigned long);

// Clear all four player records while preserving the shared document header.
void* CSharedDoc_ClearPlayerStateStorage(char* sharedDoc) {
    return memset(((SharedPlayerStateStorage*)sharedDoc)->playerState, 0, sizeof(((SharedPlayerStateStorage*)0)->playerState));
}

// Strip the variant bit except for the reserved unselected-course value.
unsigned char SharedDoc_GetCourseId() {
    unsigned int courseId = ((SharedCourseState*)g_sharedDoc)->encodedCourseId;
    int hasVariant = 0;
    if (courseId > kBaseCourseMask && courseId != kUnselectedCourse) hasVariant = 1;
    if (hasVariant) courseId -= kCourseVariantBit;
    return (unsigned char)courseId;
}

// Test for the two courses that need the caddie's special breathing-bone rotation.
int SharedDoc_IsCourse8Or13() {
    unsigned int courseId = ((SharedCourseState*)g_sharedDoc)->encodedCourseId;
    int hasVariant = 0;
    if (courseId > kBaseCourseMask && courseId != kUnselectedCourse) hasVariant = 1;
    if (hasVariant) courseId -= kCourseVariantBit;
    courseId = (unsigned char)courseId;
    if (courseId == kSpecialCourseEight) return 1;
    return courseId == kSpecialCourseThirteen;
}

struct TreeNode { TreeNode* left; TreeNode* right; };
void appFreeUni__FPv(void*);
#pragma use_lmw_stmw on
#pragma inline_depth(3)
// Free a non-null binary-tree subtree in postorder; callers own the sentinel/count.
void Tree_DestroyNodes(void* self, TreeNode* node) {
    if (node->left) Tree_DestroyNodes(self, node->left);
    if (node->right) Tree_DestroyNodes(self, node->right);
    appFreeUni__FPv(node);
}

#pragma inline_depth(3)
// Equivalent tree-node cleanup for the second template instantiation.
void Tree_DestroyNodesAlt(void* self, TreeNode* node) {
    if (node->left) Tree_DestroyNodesAlt(self, node->left);
    if (node->right) Tree_DestroyNodesAlt(self, node->right);
    appFreeUni__FPv(node);
}

// Runtime-container map relative to CSharedDoc (payload gameplay types unresolved):
// +784 first tree, +794 second tree; each header is {count,root,reserved,end}.
// An empty tree has root=null and end=&root; end is not a separately allocated node.
// +7a4 + 0x0c*i, i=0..6: {count, embedded {prev,next} sentinel}.
// This cleanup frees whole node allocations only. It invokes no payload destructor
// and cannot establish ownership of pointers that might be stored inside payloads.
// +7f8 bonusItemIds/+7fc bonusItemCount: storage is retained; only count is cleared.
struct ClearTree { int count; TreeNode* root; int reservedWord; TreeNode** end; };
struct ListNode { ListNode* prev; ListNode* next; };
struct ClearList { int count; ListNode head; };
struct ClearObject {
    char pad[0x784];
    ClearTree first;
    ClearTree second;
    ClearList lists[7];
    int bonusItemIds;
    int bonusItemCount;
};
#pragma inline_depth(0)
#pragma opt_common_subs off
// Empty both shared-document trees, seven circular lists, and the bonus-item count.
// The explicit tree levels preserve the original compiler's partial inlining.
// Reset policy (the whole function, in order): (1) free every node of tree 1 and reset its header
// to count 0 / root null / end &root; (2) same for tree 2; (3) for each of the 7 lists free all
// counted nodes and leave the embedded sentinel in place; (4) set bonusItemCount to 0 (the
// bonusItemIds storage is not freed or zeroed). The big if-nests below are one inlined
// tree-free template, repeated per tree; they carry no other logic. Payload types stored in the
// nodes are unresolved, so no payload cleanup is attempted.
void SharedDoc_FreeTreesAndLists(ClearObject* self) {
    // Tree +784: drain left branch, then right branch, then root; reset header.
    // Each explicit grandchild repeats postorder cleanup; deeper levels recurse.
    {
        TreeNode* root = self->first.root;
        if (root) {
            {
                // +784 left subtree: descendants must be freed before their parent.
                TreeNode* child = root->left;
                if (child) {
                    {
                        TreeNode* grandchild = child->left;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    {
                        TreeNode* grandchild = child->right;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    appFreeUni__FPv(child);
                }
            }
            {
                // +784 right subtree: same postorder schedule as the left side.
                TreeNode* child = root->right;
                if (child) {
                    {
                        TreeNode* grandchild = child->left;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    {
                        TreeNode* grandchild = child->right;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodesAlt(&self->first, descendant->left);
                                    if (descendant->right) Tree_DestroyNodesAlt(&self->first, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    appFreeUni__FPv(child);
                }
            }
            appFreeUni__FPv(root);
            self->first.count = 0;
            self->first.root = 0;
            self->first.end = &self->first.root;
        }
    }
    {
        // Tree +794: independent allocation graph, same postorder/reset protocol.
        TreeNode* root = self->second.root;
        if (root) {
            {
                TreeNode* child = root->left;
                if (child) {
                    {
                        TreeNode* grandchild = child->left;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    {
                        TreeNode* grandchild = child->right;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    appFreeUni__FPv(child);
                }
            }
            {
                TreeNode* child = root->right;
                if (child) {
                    {
                        TreeNode* grandchild = child->left;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    {
                        TreeNode* grandchild = child->right;
                        if (grandchild) {
                            {
                                TreeNode* descendant = grandchild->left;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            {
                                TreeNode* descendant = grandchild->right;
                                if (descendant) {
                                    if (descendant->left) Tree_DestroyNodes(&self->second, descendant->left);
                                    if (descendant->right) Tree_DestroyNodes(&self->second, descendant->right);
                                    appFreeUni__FPv(descendant);
                                }
                            }
                            appFreeUni__FPv(grandchild);
                        }
                    }
                    appFreeUni__FPv(child);
                }
            }
            appFreeUni__FPv(root);
            self->second.count = 0;
            self->second.root = 0;
            self->second.end = &self->second.root;
        }
    }
    {
        ListNode* end;
        // List 0 (+7a4): detach [first,last], leaving the embedded sentinel alone.
        // Save next before free; each iteration consumes one counted node.
        ListNode* node = self->lists[0].head.next;
        end = &self->lists[0].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[0].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 1 (+7b0): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[1].head.next;
        end = &self->lists[1].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[1].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 2 (+7bc): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[2].head.next;
        end = &self->lists[2].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[2].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 3 (+7c8): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[3].head.next;
        end = &self->lists[3].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[3].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 4 (+7d4): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[4].head.next;
        end = &self->lists[4].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[4].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 5 (+7e0): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[5].head.next;
        end = &self->lists[5].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[5].count--;
            }
        }
    }
    {
        ListNode* end;
        // List 6 (+7ec): same detach/free protocol; payload meaning unresolved.
        ListNode* node = self->lists[6].head.next;
        end = &self->lists[6].head;
        if (node != end) {
            ListNode* last = end->prev;
            node->prev->next = last->next;
            ListNode* before = node->prev;
            ListNode* after = last->next;
            after->prev = before;
            while (node != end) {
                ListNode* old = node;
                node = node->next;
                appFreeUni__FPv(old);
                self->lists[6].count--;
            }
        }
    }
    // Logical clear of bonus IDs: the original count-count expression retains capacity.
    self->bonusItemCount -= ((char*)self + self->bonusItemCount) - (char*)self;
}

#pragma opt_common_subs on
#pragma inline_depth()
// Stably move nonzero item IDs into the first occupied-count slots; return that count.
int SharedDoc_CompactPlayerItemSlots(char* base, unsigned int playerIndex) {
    // Worked example (8 slots, 0 = empty): [A 0 B 0 0 C 0 0] -> occupiedCount = 3. The loop visits
    // destinations 0..2: slot 1 is empty, so the next non-empty slot after it (B) moves in and its
    // old slot is zeroed; slot 2 is empty, so C moves in. Result [A B C 0 0 0 0 0]; order is kept.
    // The scan for each hole starts after the hole and stops at slot 7 (kItemSlotCount).
    // base is the indexing origin supplied by the caller: no header skip is added.
    // Slot p's items are at base + p*0x1d4 + 0x8c. Do not add the +0x10
    // used by ClearPlayerStateStorage: that function clears a different byte range.
    // Invalid playerIndex returns 0 without access; valid input is modified in place.
    if (playerIndex >= kPlayerCount) return 0;
    register int* destination;
    register int i;
    PlayerItemState* playerState = (PlayerItemState*)base + playerIndex;
    int* itemIds = playerState->itemIds;
    int occupiedCount = 0;
    if (playerState->itemIds[0] != 0) occupiedCount = 1;
    if (itemIds[1] != 0) occupiedCount++;
    if (itemIds[2] != 0) occupiedCount++;
    if (itemIds[3] != 0) occupiedCount++;
    if (itemIds[4] != 0) occupiedCount++;
    if (itemIds[5] != 0) occupiedCount++;
    if (itemIds[6] != 0) occupiedCount++;
    if (itemIds[7] != 0) occupiedCount++;
    destination = itemIds;
    for (i = 0; i < occupiedCount; destination++, i++) {
        if (*destination == 0) {
            int j = i + 1;
            for (; j < kItemSlotCount; j++) {
                if (itemIds[j] != 0) {
                    *destination = itemIds[j];
                    itemIds[j] = 0;
                    break;
                }
            }
            if (j == kItemSlotCount) break;
        }
    }
    return occupiedCount;
}

unsigned int ItemInventory_GetItemIdAt(void*, int);

// Count the selected inventory item in the current player's eight equipped slots.
int Count_SelectedInventoryItemOccurrences(char* base, int inventorySlot) {
    // Borrow base at the same indexing origin as CompactPlayerItemSlots above.
    // Global golfDoc supplies currentPlayer (caller must keep it in 0..3), while
    // global itemInventory resolves the selection. base need not equal g_sharedDoc.
    // Clamp the requested inventory slot to 0..12 before resolving its full item ID.
    // Return 0 without a golfDoc. Otherwise count all eight equal IDs, including
    // zero IDs if the inventory lookup returns zero; empty slots are not filtered.
    CSharedDoc* sharedDoc = g_sharedDoc;
    if (sharedDoc->golfDoc == 0) return 0;
    char* inventory = sharedDoc->itemInventory;
    if (inventorySlot < 0) inventorySlot = 0;
    else {
        int t = inventorySlot > kLastInventorySlot ? kLastInventorySlot : inventorySlot;
        inventorySlot = t;
    }
    // Resolution and equipped-slot counting use different storage owners.
    unsigned int itemId = ItemInventory_GetItemIdAt(inventory, inventorySlot);
    unsigned int* equippedItemIds = (unsigned int*)((PlayerItemState*)base)[((CGolfDoc*)g_sharedDoc->golfDoc)->currentPlayer].itemIds;
    int matchCount = 0;
    if (itemId == equippedItemIds[0]) matchCount = 1;
    if (itemId == equippedItemIds[1]) matchCount++;
    if (itemId == equippedItemIds[2]) matchCount++;
    if (itemId == equippedItemIds[3]) matchCount++;
    if (itemId == equippedItemIds[4]) matchCount++;
    if (itemId == equippedItemIds[5]) matchCount++;
    if (itemId == equippedItemIds[6]) matchCount++;
    if (itemId == equippedItemIds[7]) matchCount++;
    return matchCount;
}


struct CourseAliasStringPool {
    char strokePlay[11], matchPlay[7], caddiePractice[17], gbinFileFormat[13];
    char blueAlias[5], blueLagoon[12], blueWater[11];
    char windAlias[5], sepiaWind[11], windHill[10];
    char treeInsertError[26], lengthErrorType[55], padding[1];
};
extern CourseAliasStringPool playModeAndCourseAliasStrings;
void* ItemInventory_FindCourseData(void*, unsigned int);
int strcmp(const char*, const char*);
void strncpy(char*, const char*, int);

// Select the IFF course record and resolve the legacy blue/wind map aliases by variant.
// encodedCourseId: low 7 bits = base course id; bit 7 set = variant of that course; 0xFD = nothing selected.
// Returns 0 if the course is not in the inventory; otherwise stores the record in g_sharedDoc and fills the
// 0x20-byte map resource name (alias maps "blue"/"wind" pick a named map from the course's variant flags).
int SharedDoc_SelectCourse(unsigned char encodedCourseId) {
    int hasVariant = 0;
    char* inventory = ((char*)&AsCSharedDoc(g_sharedDoc)->itemInventory);
    if (encodedCourseId > kBaseCourseMask && encodedCourseId != kUnselectedCourse) hasVariant = 1;
    unsigned int courseId = encodedCourseId;
    if (hasVariant) courseId -= kCourseVariantBit;
    IFF_STRUCT::sCourse* course = (IFF_STRUCT::sCourse*)ItemInventory_FindCourseData(inventory, courseId | kCourseItemType);
    if (course == 0) return 0;
    ((SharedCourseState*)g_sharedDoc)->encodedCourseId = encodedCourseId;
    AsCSharedDoc(g_sharedDoc)->selectedCourseData = course; // 0x801F53F8: selected inventory record at +0x8BC
    memset(((SharedCourseState*)g_sharedDoc)->mapResourceName, 0, kMapResourceNameLength);
    CourseAliasStringPool* aliases = &playModeAndCourseAliasStrings;
    if (strcmp(course->mapName, aliases->blueAlias) == 0) {
        unsigned int variant = course->courseFlags & kCourseVariantMask;
        if (variant == kFirstVariant) strncpy(((SharedCourseState*)g_sharedDoc)->mapResourceName, aliases->blueLagoon, kMapResourceNameLength);
        else if (variant == kSecondVariant) strncpy(((SharedCourseState*)g_sharedDoc)->mapResourceName, aliases->blueWater, kMapResourceNameLength);
    } else if (strcmp(course->mapName, aliases->windAlias) == 0) {
        unsigned int variant = course->courseFlags & kCourseVariantMask;
        if (variant == kFirstVariant) strncpy(((SharedCourseState*)g_sharedDoc)->mapResourceName, aliases->sepiaWind, kMapResourceNameLength);
        else if (variant == kSecondVariant) strncpy(((SharedCourseState*)g_sharedDoc)->mapResourceName, aliases->windHill, kMapResourceNameLength);
    } else {
        strncpy(((SharedCourseState*)g_sharedDoc)->mapResourceName, course->mapName, kMapResourceNameLength);
    }
    return 1;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804621BC", _ctors_u801F44A4_0, SharedDoc_InitializeAngleConstant);
#pragma pop
