// MenuMessage802CE2BC: layout view from MenuMessage_Ctor and MenuMessage_Dtor.
// No constructors, inheritance, virtual declarations, or emitted code.
#ifndef CLASSES_MENUMESSAGE802CE2BC_H
#define CLASSES_MENUMESSAGE802CE2BC_H

struct MenuMessage802CE2BC {
    void* taggedText; // 0x0: MenuMessage_SetText passes to Text_ExpandTaggedText
    void* textFactory; // 0x4: MenuMessage_Ctor passes to WFixedFont_CreateMsgEcbResource / WFixedFont_CreateAndRegisterMsgDraw
    void* text; // 0x8: MenuMessage_SetPosition / MenuMessage_DrawText set position and draw
};

#define OFF(f, o) typedef char MenuMessage802CE2BC_off_##f[((int)&((MenuMessage802CE2BC*)0)->f == (o)) ? 1 : -1]
OFF(taggedText, 0x0);
OFF(textFactory, 0x4);
OFF(text, 0x8);
#undef OFF
#define AsMenuMessage802CE2BC(p) ((MenuMessage802CE2BC*)(p))

#endif

