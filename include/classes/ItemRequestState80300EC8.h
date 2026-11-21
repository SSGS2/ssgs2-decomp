// Layout view owned by item_request_state.cpp; original class name is unknown.
// ItemRequestState_QueueRequestIfNeeded dispatches item type IDs and queues failed requests in the first vector.
// ItemRequestState_Ctor initializes both vectors; ItemRequestState_ClearEntries clears them and frees text.
#ifndef CLASSES_ITEMREQUESTSTATE80300EC8_H
#define CLASSES_ITEMREQUESTSTATE80300EC8_H

class ItemRequestState80300EC8 {
public:
    void* primaryProcessor; // 0x00: TourItemRequest_DispatchDisplay dispatches vtable +0x10; ItemRequestState_Dtor destroys
    void* secondaryProcessor; // 0x04: TourItemRequest_DispatchDisplay dispatches vtable +0x14 if primary absent
    void* pendingRequests; // 0x08: ItemRequestState_QueueRequestIfNeeded appends 12-byte item requests
    int pendingRequestCount; // 0x0C: ItemRequestState_GetEntryCount sums both counts
    unsigned int pendingRequestCapacity; // 0x10: ItemRequestState_Ctor Buffer12::reserve
    void* textEntries; // 0x14: ItemRequestState_ClearEntries iterates 20-byte entries, frees owned text
    int textEntryCount; // 0x18: ItemRequestState_GetEntryCount sums both counts
    unsigned int textEntryCapacity; // 0x1C: ItemRequestState_Ctor Buffer20::reserve
    int complete; // 0x20: ItemRequestState_UpdateCompletion sets after both checks; ItemRequestState_QueueRequestIfNeeded clears on queued request
    int dispatchEnabled; // 0x24: TourItemRequest_DispatchDisplay gates processor dispatch
};

#define OFF(f, o) typedef char ItemRequestState80300EC8_off_##f[((int)&((ItemRequestState80300EC8*)0)->f == (o)) ? 1 : -1]
OFF(primaryProcessor, 0x00);
OFF(secondaryProcessor, 0x04);
OFF(pendingRequests, 0x08);
OFF(pendingRequestCount, 0x0C);
OFF(pendingRequestCapacity, 0x10);
OFF(textEntries, 0x14);
OFF(textEntryCount, 0x18);
OFF(textEntryCapacity, 0x1C);
OFF(complete, 0x20);
OFF(dispatchEnabled, 0x24);
#undef OFF
#define AsItemRequestState80300EC8(p) ((ItemRequestState80300EC8*)(p))
#endif
