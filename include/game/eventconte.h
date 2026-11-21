#ifndef GAME_EVENTCONTE_H
#define GAME_EVENTCONTE_H

// Root base of the event hierarchy (CEvent<Kind>SC_* classes); abstract, only the vtable pointer.
// The deleting destructor is defined in story_conte_cutscene.cpp.
struct CEventConte {
    void* vtable;
    CEventConte() {}
    ~CEventConte();
};

#endif
