#ifndef CHATMSG_MATCH_H
#define CHATMSG_MATCH_H

extern "C" {
extern char lbl_804B5DB8[];
extern char __vt__8CChatMsg[];
extern char kChatDefaultFixedFontName[];
extern void* g_resourceManager;
extern void* g_chatMsg;
void BaseObject_ctor(void*);
void* ResourceManager_GetOrLoadDefaultFixedFont(void*, void*);
void ResourceManager_ReleaseCachedObject(void*, void*);
void BaseObject_dtor(void*, int);
}

class CChatMsg;

template<class T> struct WSingleton {
    void* vtable;
    char unk04[12];

    WSingleton() {
        BaseObject_ctor(this);
        vtable = lbl_804B5DB8;
        g_chatMsg = this;
    }

    ~WSingleton();
};

struct CChatMsg : WSingleton<CChatMsg> {
    void* defaultFixedFont;
    CChatMsg();
    ~CChatMsg();
};

#endif
