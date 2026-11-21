// CMessageOwnerLayout: layout view for message-owner functions in story_message_window.cpp (class name provisional).
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CMESSAGEOWNERLAYOUT_H
#define CLASSES_CMESSAGEOWNERLAYOUT_H

class CMessageOwnerLayout {
public:
    void* unk_0; // 0x0: opaque context
    void* textContext; // 0x4: EventMessage_RebuildMessageDraw passes to Text_ExpandTaggedText
    char pad_8[0x4];
    void* textFactory; // 0xC: ChildObj constructor reaches external field +0xAC
    int scriptId; // 0x10: StoryMessage_DisableAndResetPanel / StoryConte_DrawPanelAndMessages use IdMap_Lookup; -1 is absent
    void* renderTables[2]; // 0x14: StoryConte_DrawPanelAndMessages indexes by resource table selection
    void* drawObject; // 0x1C: StoryConte_DrawPanelAndMessages retrieves size/position of its first secondaryMessageDraw
    char pad_20[0x4];
    void* messageDraw; // 0x24: EventMessage_RebuildMessageDraw constructs CMsgDraw then sets text
    int currentCharacterId; // 0x28: StoryMessage_SelectTransitionScripts tests -1 to choose text
    int resourceTable; // 0x2C: EventMessage_RebuildMessageDraw selects characterNameMessageTagTable / storySecondaryMessageTagTable
    int resourceIndex; // 0x30: EventMessage_RebuildMessageDraw indexes selected resource table
    void* panelOpacity; // 0x34: existing field preserved; StoryConte_DrawPanelAndMessages also reads as quad alpha
    void* secondaryMessageDraw; // 0x38: existing field preserved; CTourMessage_GetOrCreateMessageDraw constructs second CMsgDraw
    char pad_3C[0x4];
    unsigned int transitionNoScrollScript[3]; // 0x40: StoryMessage_SelectTransitionScripts / CLEAN_SLOT operate on a 12-byte tagged string
    unsigned int outNoScrollScript[3]; // 0x4C: StoryMessage_SelectTransitionScripts / CLEAN_SLOT operate on a 12-byte tagged string
    unsigned int characterOutNoScrollScript[3]; // 0x58: StoryMessage_SelectTransitionScripts / CLEAN_SLOT operate on a 12-byte tagged string
    unsigned int defaultWindowScript[3]; // 0x64: StoryMessage_SelectTransitionScripts / CLEAN_SLOT operate on a 12-byte tagged string
    unsigned int defaultNoScrollScript[3]; // 0x70: StoryMessage_SelectTransitionScripts / CLEAN_SLOT operate on a 12-byte tagged string
};

#define OFF(f, o) typedef char CMessageOwnerLayout_off_##f[((int)&((CMessageOwnerLayout*)0)->f == (o)) ? 1 : -1]
OFF(unk_0, 0x0);
OFF(textContext, 0x4);
OFF(textFactory, 0xC);
OFF(scriptId, 0x10);
OFF(renderTables, 0x14);
OFF(drawObject, 0x1C);
OFF(messageDraw, 0x24);
OFF(currentCharacterId, 0x28);
OFF(resourceTable, 0x2C);
OFF(resourceIndex, 0x30);
OFF(panelOpacity, 0x34);
OFF(secondaryMessageDraw, 0x38);
OFF(transitionNoScrollScript, 0x40);
OFF(outNoScrollScript, 0x4C);
OFF(characterOutNoScrollScript, 0x58);
OFF(defaultWindowScript, 0x64);
OFF(defaultNoScrollScript, 0x70);
#undef OFF

#endif
