// GfxObjList: partial layout view owned by shadow_manager_capture.cpp.
// GfxObjList_FindByName and GfxObjList_RemoveGroupByName traverse/remove named render groups.
// No inheritance, virtual declarations, constructors, or emitted code.
#ifndef CLASSES_GFXOBJLIST_H
#define CLASSES_GFXOBJLIST_H

struct GfxObjList {
    char pad_000[0x14];
    void*
        renderResources; // 0x14: GfxObjList_RemoveGroupByName removes a resource; ShadowManager_RenderShadowTextures applies render target/state
    char pad_018[4];
    unsigned int groupCount;    // 0x1C: GfxObjList_RemoveGroupByName decrements after unlinking/deleting a group
    unsigned int groupSentinel; // 0x20: address terminates named group traversal in GfxObjList_FindByName
    unsigned int* firstGroup;   // 0x24: GfxObjList_FindByName starts traversal here
};

#define OFF(f, o) typedef char GfxObjList_off_##f[((int)&((GfxObjList*)0)->f == (o)) ? 1 : -1]
OFF(renderResources, 0x14);
OFF(groupCount, 0x1C);
OFF(groupSentinel, 0x20);
OFF(firstGroup, 0x24);
#undef OFF
#define AsGfxObjList(p) ((GfxObjList*)(p))

#endif
