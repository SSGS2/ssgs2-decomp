// The head of the HUD text panel classes and the vtable entry they use to load
// a font. A panel stores its vtable pointer first; this class only types a call
// through that table, it is never instantiated.
#ifndef CLASSES_HUDTEXTPANEL_H
#define CLASSES_HUDTEXTPANEL_H

struct HudTextPanelVTable {
    virtual void unused08();
    virtual void unused0C();
    virtual void unused10();
    virtual void loadFont(const char* path, int width, int height);
};

#endif // CLASSES_HUDTEXTPANEL_H
