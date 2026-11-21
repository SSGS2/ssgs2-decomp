// ResourceManager: non-virtual layout view; methods/accesses in resource_manager_cache_and_loading.cpp.
// No inheritance or virtual declarations: the original vtables remain external symbols.
#ifndef CLASSES_RESOURCEMANAGER_H
#define CLASSES_RESOURCEMANAGER_H

class ResourceManager {
public:
    void* vtbl; // 0x0: ResourceManager_dtor: owner virtual release
    char pad_004[0x4];
    int recordCount; // 0x8: ResourceManager_GetOrLoadBitmapByPath increments, dtor60550Release decrements
    void* recordSentinel; // 0xC: ObjectList_FindById compares node with this address
    void* firstRecord; // 0x10: ObjectList_FindById follows the record list
    unsigned resourceBytes; // 0x14: ResourceManager_ReleaseId subtracts object->size
    unsigned modelCacheVtable; // 0x18: ResourceManager_LoadByExtension passes cache at +0x18
    void* cachedModels; // 0x1C: ResourceManager_ReleaseObject iterates 16-byte entries
    unsigned cachedModelCount; // 0x20: ResourceManager_ReleaseObject entry count
    unsigned cachedModelCapacity; // 0x24: cache vector trailing capacity
    unsigned objectCacheVtable; // 0x28: ResourceManager_dtor destroys vector at +0x28
    void* cachedObjects; // 0x2C: ResourceManager_ReleaseCachedObject iterates 16-byte entries
    unsigned cachedObjectCount; // 0x30: ResourceManager_ReleaseCachedObject entry count
    unsigned cachedObjectCapacity; // 0x34: cache vector trailing capacity
    char resourceHash[0x38]; // 0x38: ResourceManager_dtor passes +0x38 to dtor60550Hash
    void* ownedResource; // 0x70: ResourceManager_dtor releases through owner
    char pad_074[0x84];
    void* activeHashNodes; // 0xF8: ResourceManager_GetOrLoadBitmapByPath links registered hash nodes
    void* hashIterator; // 0xFC: dtor60550Next advances through activeHashNodes
    void* hashBlocks; // 0x100: ResourceManager_GetOrLoadBitmapByPath links allocated blocks
    void* freeHashNodes; // 0x104: ResourceManager_GetOrLoadBitmapByPath pops a reusable node
    int hashBlockNodeCount; // 0x108: ResourceManager_GetOrLoadBitmapByPath allocation count times 0x18
    unsigned hashMask; // 0x10C: hash62520 masks computed hash
    int hashBucketCount; // 0x110: ResourceManager_dtor clears bucket pointers
    void* hashBuckets; // 0x114: find62520 indexes bucket array
    char hashMutex[0x18]; // 0x118: hash62520/link62520/find62520: WInstanceLock
    void* loadedModel; // 0x130: ResourceManager_RequestModelLoad stores cached/created model
    void* resourceData; // 0x134: ResourceManager_PollResourceRead allocates async read buffer
    unsigned resourceDataBytes; // 0x138: ResourceManager_PollResourceRead saves CFileAsync_GetSize size
    char loadPath[0x400]; // 0x13C: ResourceManager_RequestModelLoad copies path with limit 0x400
    int loadFormat; // 0x53C: ResourceManager_RequestModelLoad extension-dependent values, not an error code
    void* asyncRead; // 0x540: ResourceManager_PollResourceRead polls/finishes asynchronous read
    int loadState; // 0x544: ResourceManager_RequestModelLoad/ResourceManager_PollResourceRead states 1 through 5
};

#define OFF(f, o) typedef char ResourceManager_off_##f[((int)&((ResourceManager*)0)->f == (o)) ? 1 : -1]
OFF(vtbl, 0x0);
OFF(recordCount, 0x8);
OFF(recordSentinel, 0xC);
OFF(firstRecord, 0x10);
OFF(resourceBytes, 0x14);
OFF(modelCacheVtable, 0x18);
OFF(cachedModels, 0x1C);
OFF(cachedModelCount, 0x20);
OFF(cachedModelCapacity, 0x24);
OFF(objectCacheVtable, 0x28);
OFF(cachedObjects, 0x2C);
OFF(cachedObjectCount, 0x30);
OFF(cachedObjectCapacity, 0x34);
OFF(resourceHash, 0x38);
OFF(ownedResource, 0x70);
OFF(activeHashNodes, 0xF8);
OFF(hashIterator, 0xFC);
OFF(hashBlocks, 0x100);
OFF(freeHashNodes, 0x104);
OFF(hashBlockNodeCount, 0x108);
OFF(hashMask, 0x10C);
OFF(hashBucketCount, 0x110);
OFF(hashBuckets, 0x114);
OFF(hashMutex, 0x118);
OFF(loadedModel, 0x130);
OFF(resourceData, 0x134);
OFF(resourceDataBytes, 0x138);
OFF(loadPath, 0x13C);
OFF(loadFormat, 0x53C);
OFF(asyncRead, 0x540);
OFF(loadState, 0x544);
#undef OFF

#endif
