// BGM Collection tasks and actors: creating and drawing the collection actors, their
// destructor thunks, and the BGM table lookups for metadata, resources and sound names.
// The menu logic is in bgm_collection_screen_part1.cpp.

#include <decomp/static_init.h>

#include "game/string.h"

// Stack-local narrow-string view; the canonical string ABI remains external.
class LocalNString_u80319AD8 {
public:
    unsigned int flags;
    unsigned int length;
    char* data;

    LocalNString_u80319AD8() : flags(0), length(0), data(0) {}
#if defined(GAME_NSTRING_TEXT_CONSTRUCTOR)
    LocalNString_u80319AD8(const char* text) {
        MWString_ConstructCString(this, text);
    }
#endif
#if defined(GAME_NSTRING_RANGE_CONSTRUCTOR)
    LocalNString_u80319AD8(const char* text) {
        LocalNString_u80319AD8* object = this;
        unsigned int size;
        const char* source = text;
        object->flags = 0;
        object->length = 0;
        object->data = 0;
        size = nstring_detail::strlen(source);
        nstring_detail::String_ReserveCapacity(object, size);
        int allocator;
        char copy = *(char*)&allocator;
        nstring_detail::String_ReplaceRange(object, 0, 0, source, source + size, &copy);
    }
#endif
    ~LocalNString_u80319AD8() {
        if (flags >> 31)
            appFreeUni__FPv(data);
    }
    unsigned int size() const {
        return (flags >> 31) == 0 ? (*(const unsigned char*)this & 0x7f) : length;
    }
    const char* c_str() const {
        return (flags >> 31) == 0 ? (const char*)this + 1 : data;
    }
};

#include "classes/CBgmCollectionMain.h"
#include "classes/CBgmCollectionTask.h"
#include "classes/CBgm.h"
#include "classes/CTask.h"

extern "C" {

void* __dt__18CBgmCollectionMainFv(void*, int);
void* __dt__18CBgmCollectionTaskFv(void*, int);
void* __ct__18NativeBase80201D18Fv(void*);
void* Table_GetEntry(void*, int);
extern float kFloat_6_2831855;
extern char __vt__18CBgmCollectionMain[];
extern char bgmCollectionTaskStrings[];
extern void* g_taskManager;
extern int MPBase1_Process(void*);
extern void TaskCommandQueue_AppendNamedRequest(void*, int, const char*, int, int, int, int, int);
extern void __dt__18NativeBase80201D18Fv(void*, int);
extern void* String_ReserveCapacity(void*, unsigned int);
extern void* String_ReplaceRange(void*, unsigned int, unsigned int, char*, char*, void*);
extern void MPBase1_DrawIfInitialized(void*);
extern void* TaskActorTable_CreateNamedActor(void*, const char*, const char*, int, void*);
extern void appFreeUni__FPv(void*);
extern unsigned int strlen(const char*);
extern float s_bgmCollectionMainFullTurnAngle;
extern float s_bgmCollectionTableFullTurnAngle;
extern char s_bgmCollectionTaskRttiDescriptor;
extern char s_bgmCollectionMainTypeInfo;

extern void* __dt__5CTaskFv(void*, int);
extern void __ct__14CBgmCollectionFv(void*);
extern void* __nw__FUl(unsigned int);
extern void __dl__FPv(void*);
extern char __vt__18CBgmCollectionTask[];
extern int g_defaultActorSkipMask;
extern int g_actorSkipMask;

void CBgmCollectionTask::StartActorLoading(void* taskBase) {
    ((CTask*)taskBase)->StartActorLoading();
}
void CBgmCollectionTask::ProcessActors(void* taskBase, float forwarded1) {
    ((CTask*)taskBase)->ProcessActors(forwarded1);
}
void CBgmCollectionTask::DisplayActorsByPriority(void* taskBase) {
    ((CTask*)taskBase)->DisplayActorsByPriority();
}

void CBgmCollectionMain::OnPreLoadInit() {}
void CBgmCollectionMain::OnInit() {}
char* CBgmCollectionTask::GetRTTI() {
    return &s_bgmCollectionTaskRttiDescriptor;
}
char* CBgmCollectionMain::GetRTTI() {
    return &s_bgmCollectionMainTypeInfo;
}
// Adjust the task-main interface pointer back to the owning actor before destruction.
void CBgmCollectionMain::DtorThunk18(int deleting) {
    __dt__18CBgmCollectionMainFv((char*)this - 0x18, deleting);
}
// Adjust the secondary-interface pointer back to the owning task before destruction.
void CBgmCollectionTask::DtorThunk8(int deleting) {
    __dt__18CBgmCollectionTaskFv((char*)this - 8, deleting);
}
// Initialize the collection-main unit's full-turn angle constant.
void BgmCollectionMain_InitFullTurnConstant() {
    s_bgmCollectionMainFullTurnAngle = kFloat_6_2831855;
}
// Initialize the collection-table unit's full-turn angle constant.
void BgmCollectionTable_InitFullTurnConstant() {
    s_bgmCollectionTableFullTurnAngle = kFloat_6_2831855;
}

int CBgm::collection_data_vfunc_7() {
    return 1;
}
int CBgm::collection_data_vfunc_6() {
    return 1;
}
int CBgm::collection_data_vfunc_5() {
    return 1;
}
int CBgm::collection_data_vfunc_4() {
    return 1;
}
int CBgm::collection_data_vfunc_3() {
    return 1;
}
int CBgm::collection_data_vfunc_2() {
    return 1;
}
int CBgm::collection_data_vfunc_1() {
    return 1;
}

// Table records expose the resource, sound name, and additional BGM metadata.
struct BgmCollectionEntry {
    int key;
    void* resource;
    void* soundName;
    void* metadata;
};
struct BgmCollectionStrings {
    char titleTask[11];
    char empty[1];
    char titleMain[11];
    char collectionMainClass[19];
    char collectionActor[14];
};

typedef char BgmCollectionEntry_size[(sizeof(BgmCollectionEntry) == 0x10) ? 1 : -1];
typedef char BgmCollectionStrings_actorOffset[((int)&((BgmCollectionStrings*)0)->collectionActor == 0x2A) ? 1 : -1];
#define BGM_STRING_OFFSET(field) ((int)&((BgmCollectionStrings*)0)->field)

// Return the metadata pointer for an indexed BGM collection entry, or null.
void* BgmCollectionTable_GetMetadata(const void* self, int index) {
    BgmCollectionEntry* p = (BgmCollectionEntry*)Table_GetEntry(*(void**)self, index);
    if (p)
        return p->metadata;
    return 0;
}

// Initialize the actor base and collection pointer, then install both interface vtables.
CBgmCollectionMain::CBgmCollectionMain() {
    __ct__18NativeBase80201D18Fv(this);
    (*(int*)&collection) = 0;
    char* v = __vt__18CBgmCollectionMain;
    *(void**)this = v;
    taskMainVtable = v + 0x3c;
}

class X2 {
public:
    virtual void v08(int);
};
struct CBgmCollectionStorage {
    char pad[0x888];
    CBgmCollectionStorage() {
        __ct__14CBgmCollectionFv(this);
    }
};
// Delete the owned BGM collection through its virtual destructor and clear the pointer.
void CBgmCollectionMain::OnDestroy() {
    void* p = collection;
    if (p) {
        if (p) {
            ((X2*)p)->v08(1);
        }
        collection = 0;
    }
}

// Initialize the task interfaces and reset the actor skip masks.
CBgmCollectionTask::CBgmCollectionTask() {
    ((CTask*)this)->Init();
    char* v = __vt__18CBgmCollectionTask;
    *(void**)this = v;
    interfaceVtable = v + 0x10;
    secondaryInterfaceVtable = v + 0x1c;
    g_defaultActorSkipMask = 0;
    g_actorSkipMask = 0;
}

// Destroy the task base and optionally delete this task.
CBgmCollectionTask::~CBgmCollectionTask() {
    __dt__5CTaskFv(this, 0);
}

// Replace any previous collection with a newly constructed collection object.
void CBgmCollectionMain::OnLoad() {
    void* p = collection;
    if (p) {
        if (p) {
            ((X2*)p)->v08(1);
        }
        collection = 0;
    }
    CBgmCollectionStorage* np = new CBgmCollectionStorage;
    collection = np;
}

// Return to the title task once the BGM collection reports completion.
void CBgmCollectionMain::OnProcess() {
    void* p = collection;
    if (p && MPBase1_Process(p)) {
        char* base = bgmCollectionTaskStrings;
        ((CTask*)g_taskManager)->RequestTransitionWithFade(base, base + BGM_STRING_OFFSET(empty), 0);
        TaskCommandQueue_AppendNamedRequest(g_taskManager, 0, base + BGM_STRING_OFFSET(titleMain), 7, 0, 0, 0, 0);
    }
}

// Delete the collection, destroy the actor base, and optionally delete this actor.
CBgmCollectionMain::~CBgmCollectionMain() {
    char* v = __vt__18CBgmCollectionMain;
    void* p = collection;
    *(void**)this = v;
    taskMainVtable = v + 0x3c;
    if (p) {
        if (p) {
            ((X2*)p)->v08(1);
        }
        collection = 0;
    }
    if (this) {
        __dt__18NativeBase80201D18Fv(this, 0);
    }
}

// Create the named collection actor and install it as the task's main actor.
void CBgmCollectionTask::CreateActors() {
    BgmCollectionStrings* base = (BgmCollectionStrings*)bgmCollectionTaskStrings;
    char* str = base->empty;
    struct Frame {
        char terminator;
        char pad1[3];
        volatile char sourceChar[4];
        LocalNString_u80319AD8 text;
    } frame;

    unsigned int len = strlen(str);
    String_ReserveCapacity(&frame.text, len);
    frame.terminator = frame.sourceChar[0];
    String_ReplaceRange(&frame.text, 0, 0, str, str + len, &frame.terminator);
    void* result =
        TaskActorTable_CreateNamedActor(this, base->collectionMainClass, base->collectionActor, 0, &frame.text);
    ((CTask*)this)->SetMainActor(result);
}

// Draw the collection once its menu initialization has succeeded.
void CBgmCollectionMain::OnDisplay() {
    void* p = collection;
    if (p == 0)
        return;
    MPBase1_DrawIfInitialized(p);
}

// Return the indexed collection resource pointer, or null if the table entry is absent.
void* BgmCollectionTable_GetResource(void* __restrict self, int index) {
    BgmCollectionEntry* p = (BgmCollectionEntry*)Table_GetEntry(*(void**)self, index);
    if (p)
        return p->resource;
    return 0;
}

void* BgmCollectionTable_GetSoundName(void* __restrict self, int index) {
    BgmCollectionEntry* p = (BgmCollectionEntry*)Table_GetEntry(*(void**)self, index);
    if (p)
        return p->soundName;
    return 0;
}

} // extern "C"

/* Static initialisers run from the .ctors table, as in the original. */
#pragma push
#pragma force_active on
STATIC_INIT_ENTRY(".CRT$XCU804625D0", _ctors_u80319AD8_0, BgmCollectionMain_InitFullTurnConstant);
STATIC_INIT_ENTRY(".CRT$XCU804625D0", _ctors_u80319AD8_1, BgmCollectionTable_InitFullTurnConstant);
#pragma pop
