#ifndef CLASSES_CNPC_H
#define CLASSES_CNPC_H

#include "game/types.h"

struct NpcMessageRequest;
struct fn_801978F0_List;

// A link in the NPC's circular list of owned actors. The nodes are allocated
// separately; the list header embedded in the NPC holds only the two pointers.
struct NpcChildLink {
    NpcChildLink* previous; // 0x00
    NpcChildLink* next;     // 0x04
    void* object;           // 0x08: the actor this link owns
};

struct NpcChildList {
    NpcChildLink* previous; // 0x00
    NpcChildLink* next;     // 0x04
};

// A course NPC. It builds creature actors from the course definitions and
// owns the circular list of the actors it created.
class CNPC {
public:
    static char* GetRTTI();

    void HandleMsg(NpcMessageRequest* request);
    void CreateObjectsFromCourseName(void* layout, void* nameArg);
    void OnProcess(float value);
    void OnInit();

    static void OnDestroy(fn_801978F0_List* list);

    static void CreateActorsFromDefinition(void* selfIn, void* parentIn, const char* definition);

    ~CNPC();

    void* vtbl;             // 0x00
    char pad_004[0x14];     // 0x04: native actor base
    int childCount;         // 0x18
    NpcChildList childList; // 0x1C
};

#define CNPC_OFF(f, o) typedef char CNPC_off_##f[((int)&((CNPC*)0)->f == (o)) ? 1 : -1]
CNPC_OFF(vtbl, 0x00);
CNPC_OFF(childCount, 0x18);
CNPC_OFF(childList, 0x1C);
#undef CNPC_OFF
typedef char CNPC_size[(sizeof(CNPC) == 0x24) ? 1 : -1];
typedef char NpcChildLink_size[(sizeof(NpcChildLink) == 0x0C) ? 1 : -1];
typedef char NpcChildList_size[(sizeof(NpcChildList) == 0x08) ? 1 : -1];

#endif // CLASSES_CNPC_H
