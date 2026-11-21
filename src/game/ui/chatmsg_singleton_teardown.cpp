#include "game/chatmsg_match.h"

// Unpublish the chat singleton, then run non-deleting base teardown.
// Teardown clears g_chatMsg unconditionally; this is not an identity-checked release.
// BaseObject_dtor receives deleteFlag=0; any deleting-destructor deallocation is compiler-generated.
template<> WSingleton<CChatMsg>::~WSingleton() {
    g_chatMsg = 0;
    BaseObject_dtor(this, 0);
}

// Release the default fixed-font resource acquired by CChatMsg's constructor.
// resource is a cached resource-manager reference, not directly allocated font storage.
// A missing manager skips both release and clearing; base teardown follows implicitly.
CChatMsg::~CChatMsg() {
    vtable = __vt__8CChatMsg;
    if (g_resourceManager && defaultFixedFont) {
        ResourceManager_ReleaseCachedObject(g_resourceManager, defaultFixedFont);
        defaultFixedFont = 0;
    }
}
