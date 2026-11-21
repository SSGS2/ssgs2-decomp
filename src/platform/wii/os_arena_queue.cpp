#include <decomp/static_init.h>
// Arena globals' initial values come from split .sdata/.sbss objects.
extern "C" {
DATA_SECTION(".sdata") void* __OSArenaLo_805DBCB8 = (void*)0xFFFFFFFFU;
DATA_SECTION(".sdata") void* s_mem2ArenaLo_805DBCBC = (void*)0xFFFFFFFFU;
DATA_SECTION(".sdata") void* __OSArenaHi_805DCC30;
DATA_SECTION(".sdata") void* s_mem2ArenaHi_805DCC34;

void* OSGetMEM1ArenaHi() { return __OSArenaHi_805DCC30; }
void* OSGetMEM2ArenaHi() { return s_mem2ArenaHi_805DCC34; }
void* OSGetMEM1ArenaLo() { return __OSArenaLo_805DBCB8; }
void* OSGetMEM2ArenaLo() { return s_mem2ArenaLo_805DBCBC; }
void OSSetMEM1ArenaLo(void* value) { __OSArenaLo_805DBCB8 = value; }
void OSSetMEM2ArenaLo(void* value) { s_mem2ArenaLo_805DBCBC = value; }

struct OSThreadQueue {
    void* head;
    void* tail;
};
void OSInitThreadQueue(OSThreadQueue* queue) {
    queue->tail = 0;
    queue->head = 0;
}
}
