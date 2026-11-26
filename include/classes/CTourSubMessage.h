// CTourSubMessage: a sub-object of the tour hint window. It owns four message
// renderers and one message, which it releases together.
#ifndef CLASSES_CTOURSUBMESSAGE_H
#define CLASSES_CTOURSUBMESSAGE_H

#include "game/eventconte.h"

class CTourSubMessage : public CEventConte {
public:
    CTourSubMessage();
    // Unregister and delete the renderers and the owned message. flags > 0 also frees this object.
    void* Destroy(int flags);

    // The vtable pointer is the CEventConte base.
    void* ownedMessage; // 0x04
    void* items[4];     // 0x08
    void* fontSource;   // 0x18
    int baseGlyphColor; // 0x1C  RGB mask combined with the fade opacity
    char pad_20[8];     // 0x20
};

#define CTOURSUBMESSAGE_OFFSET(field, offset) \
    typedef char CTourSubMessage_offset_##field[((int)&((CTourSubMessage*)0)->field == (offset)) ? 1 : -1]
CTOURSUBMESSAGE_OFFSET(ownedMessage, 0x04);
CTOURSUBMESSAGE_OFFSET(items, 0x08);
CTOURSUBMESSAGE_OFFSET(baseGlyphColor, 0x1C);
#undef CTOURSUBMESSAGE_OFFSET
typedef char CTourSubMessage_size[(sizeof(CTourSubMessage) == 0x28) ? 1 : -1];

#endif // CLASSES_CTOURSUBMESSAGE_H
