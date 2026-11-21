#include "game/chatmsg_match.h"

// Install the chat-message interface and acquire its default fixed-font resource.
// WSingleton's base constructor has already published this object as g_chatMsg.
// The returned resource is kept for CChatMsg::~CChatMsg, which releases it through
// ResourceManager_ReleaseCachedObject when the manager and resource are non-null.
// The current loader definition uses its own default-font key and does not read
// the supplied font-name argument; keep the existing call and null-result behavior.
CChatMsg::CChatMsg() {
    vtable = __vt__8CChatMsg;
    defaultFixedFont = 0;
    defaultFixedFont = ResourceManager_GetOrLoadDefaultFixedFont(g_resourceManager, kChatDefaultFixedFontName);
}
