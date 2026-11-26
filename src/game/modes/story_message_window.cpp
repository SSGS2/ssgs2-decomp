// Story message window: the panel and text renderers that draw story messages, the choice of
// transition scripts, and the tags that expand character and context names into message text.
// The panel draws an opacity-tinted sprite, then reveals both message renderers.

#include <decomp/static_init.h>
#include "classes/WFixedFont.h"
#include "classes/CMsgDraw.h"
#define STORY_MEMBER_OFFSET(type, member) ((int)&((type*)0)->member)
enum {
    kNoStoryResource = -1,
    kStoryPrimaryMessageTable = 0,
    kStorySecondaryMessageTable = 1,
    kStoryOpaque = 255,
    kStoryQuadFormat = 0x11f005,
    kStoryTriangleStrip = 0x98,
    kStoryVertexStride = 0x18,
    kStoryRenderEntryStride = 0x28
};
enum StoryStringEdit {
    kReplaceScript,
    kAppendScript
};
enum {
    kInlineScriptLengthMask = 0x7F,
    kStoryQuadVertexCount = 4
};
// The shared header represents opacity as a pointer-shaped word. It is an
// integer alpha in [0,255], not a float or owned pointer. FloatBits globals below
// are copied as words into {x,y} float pairs; no numeric int->float conversion.
struct StoryFloatPairBits {
    int xBits, yBits;
};
struct StoryQuadVertex {
    float x, y, z;
    unsigned char red, green, blue, alpha;
    float u, v;
};
// Four screen-space vertices, each xyz/RGBA8/uv (24 bytes). Template z/color/uv
// survive the copy; drawing edits pixel x/y and the integer opacity byte only.
struct Quad24 {
    StoryQuadVertex vertices[kStoryQuadVertexCount];
};
typedef char StoryQuadVertexSize[(sizeof(StoryQuadVertex) == kStoryVertexStride) ? 1 : -1];
struct StoryStrings {
    char messageTable[12];
    char windowLayout[11];
    char archive[17];
    char padding_28[1];
    char characterSprites[10];
    char defaultScript[8];
    char outScript[4];
    char windowOnlySuffix[9];
    char defaultWindowOnly[16];
    char characterWindowOut[17];
    char characterWindowOutNoScroll[27];
    char characterWindowIn[16];
    char inScript[3];
    char noScrollSuffix[11];
    char messageRevealSound[19];
};
#define StoryStringsView (*(StoryStrings*)storyMessageStrings)
#include "classes/CMessageOwnerLayout.h"
#define AsMessageOwner(p) ((CMessageOwnerLayout*)(p))

#include "game/singletons.h"
extern "C" {

extern void* characterNameMessageTagTable[];
extern void* storySecondaryMessageTagTable[];
extern void* storyAlternateNameMessageTagTable[];
extern DATA_SECTION(".sdata") void* messageContextTagTable[];
extern signed char s_pointerPairListInitialized;
extern signed char s_bpCtrlInitialized;
extern char s_bpCtrl[];
extern char s_bpCtrlDtorRegistration_StoryMessage[];
extern char s_messageDrawPairList[];
extern char s_partyItemVectorDtorRegistration[];
extern char storyMessageStrings[];
extern int kFloat_1_1_MessageGlyphScale, kFloat_1_1_StoryGlyphScale, kStoryMessageOriginXFloatBits,
    kStoryMessagePositionYBits;
void* __nw__FUl(unsigned int);

int Text_ExpandTaggedText(void*, void*, void*, int);
void MessageDrawPairList_Dtor();
void PairVector_InitReserved(void*);
void __register_global_object(void*, void*, void*);
void* PointerPairList_RemoveByKey(void*, void*);
void* PointerIntVector_AppendPair(void*, void*, int);
void appFreeUni__FPv(void*);
void Text_SetPosition(void*, void*);
void Text_SetContent(void*, void*);

struct Obj {
    virtual void destroy(int);
};
typedef CMessageOwnerLayout Owner;

// Unregister and destroy the secondary message renderer, then clear its owner pointer.
void TourMessageOwner_DestroyMessageDraw(Owner* self) {
    void* child = self->secondaryMessageDraw;
    if (!child)
        return;
    {
        if (child) {
            if (!Sing_805DC650()) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                Sing_805DC650() = s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey(Sing_805DC650(), child);
            if (child) {
                if (child) {
                    ((Obj*)child)->destroy(1);
                }
            }
        }
    }
    self->secondaryMessageDraw = 0;
    return;
}

struct ChildObj {
    char pad[0x4BC];
    // Construct an owned renderer using the owner's borrowed message resource.
    ChildObj(Owner* self) {
        ((CMsgDraw*)this)->__ct(AsWFixedFont(self->textFactory)->messageResource);
    }
};

} // extern "C"

// Lazily construct and register a secondary renderer with the story reveal sound.
void* CMessageOwnerLayout::GetOrCreateMessageDraw() {
    if (!secondaryMessageDraw) {
        ChildObj* child = new ChildObj(this);
        if (!Sing_805DC650()) {
            if (!s_pointerPairListInitialized) {
                PairVector_InitReserved(s_messageDrawPairList);
                __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                         s_partyItemVectorDtorRegistration);
                s_pointerPairListInitialized = 1;
            }
            Sing_805DC650() = s_messageDrawPairList;
        }
        PointerIntVector_AppendPair(Sing_805DC650(), child, 1);
        secondaryMessageDraw = child;
        ((CMsgDraw*)child)
            ->SetDefaultRevealSound(storyMessageStrings + STORY_MEMBER_OFFSET(StoryStrings, messageRevealSound));
    }
    return secondaryMessageDraw;
}

// Use the shared MSL wide-string ABI, whose inline text starts at +2.
// This alias exposes an owned expansion result, not a full std::string API.
#include "game/wstring.h"
#include <decomp/cw_bitfields.h>
#include "classes/CBpCtrl.h"
typedef WString StoryExpandedWideText;

extern "C" {

// Recreate the primary message renderer from the selected resource table when panel opacity is nonzero.
void EventMessage_RebuildMessageDraw(Owner* self) {
    Obj* child = (Obj*)self->messageDraw;
    if (child) {
        if (child) {
            if (!Sing_805DC650()) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                Sing_805DC650() = s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey(Sing_805DC650(), child);
            if (child) {
                if (child) {
                    child->destroy(1);
                }
            }
        }
        self->messageDraw = 0;
    }
    if (self->resourceTable < 0 || self->resourceIndex < 0)
        return;
    if ((*(int*)&self->panelOpacity) == 0)
        return;
    // Phase 1: expand the selected message tag into an owned wide-string result.
    StoryExpandedWideText result;
    int messageTable = self->resourceTable;
    switch (messageTable) {
    case kStoryPrimaryMessageTable: {
        int messageIndex = self->resourceIndex;
        void* resource = characterNameMessageTagTable[(unsigned char)messageIndex];
        if (!Text_ExpandTaggedText(self->textContext, &result, resource, 1)) {
            return;
        }
        break;
    }
    case kStorySecondaryMessageTable: {
        int messageIndex = self->resourceIndex;
        void* resource = storySecondaryMessageTagTable[(unsigned char)messageIndex];
        if (!Text_ExpandTaggedText(self->textContext, &result, resource, 1)) {
            return;
        }
        break;
    }
    default: {
        return;
    }
    }
    // Phase 2: register the new renderer before applying the expanded text.
    ChildObj* obj = new ChildObj(self);
    if (!Sing_805DC650()) {
        if (!s_pointerPairListInitialized) {
            PairVector_InitReserved(s_messageDrawPairList);
            __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                     s_partyItemVectorDtorRegistration);
            s_pointerPairListInitialized = 1;
        }
        Sing_805DC650() = s_messageDrawPairList;
    }
    PointerIntVector_AppendPair(Sing_805DC650(), obj, 1);
    self->messageDraw = obj;
    ((CMsgDraw*)obj)->SetColor(kStoryOpaque);
    ((CMsgDraw*)(self->messageDraw))->ClearFormattingAndRevealAll();
    StoryFloatPairBits glyphScale = {(int)kFloat_1_1_MessageGlyphScale, (int)kFloat_1_1_StoryGlyphScale};
    ((CMsgDraw*)(self->messageDraw))->SetGlyphScale((float*)&glyphScale);
    StoryFloatPairBits messagePosition = {(int)kStoryMessageOriginXFloatBits, (int)kStoryMessagePositionYBits};
    Text_SetPosition(self->messageDraw, &messagePosition);
    Text_SetContent(self->messageDraw, result.flags >> 31 == 0 ? (void*)((char*)&result + 2) : (void*)result.data);
}
void String_ReplaceRange(void*, int, int, char*, char*, char*);
unsigned int strlen(const char*);
#pragma push
#pragma opt_common_subs off
#pragma opt_propagation off
// Replace an entire narrow script name, or append a pooled suffix, in tagged storage.
// text is a borrowed 12-byte MSL string: inline length is byte 0's low seven
// bits, heap length is word 1 and heap pointer is word 2. inlineStorage may be
// precomputed by the caller; preserve it for the initial transition-name write.
// scriptLength and scriptName are scratch outputs, not retained state. Separate
// length scopes and the shared name temporary preserve register lifetimes.
// Keep the two storage-selection forms distinct under opt_propagation off.
static inline void editScriptNameWithStorageMode(int& scriptLength, char*& scriptName, unsigned int* text,
                                                 int poolOffset, StoryStringEdit edit, int inlineStorage) {
    scriptName = storyMessageStrings + poolOffset;
    scriptLength = inlineStorage ? (*(unsigned char*)text & kInlineScriptLengthMask) : text[1];
    // The allocator tag is an ABI byte copied by the original call sequence.
    // Its value has no script-selection meaning; retain the volatile read.
    union Alloc {
        char byte;
        int align;
    };
    Alloc temp;
    volatile Alloc original;
    temp.byte = original.byte;
    unsigned int len = strlen(scriptName);
    String_ReplaceRange((char*)text, edit == kAppendScript ? scriptLength : 0, edit == kAppendScript ? 0 : scriptLength,
                        scriptName, len + scriptName, &temp.byte);
}
// Choose storage mode from the current tag and edit a narrow script name.
static inline void editScriptName(int& scriptLength, char*& scriptName, unsigned int* text, int poolOffset,
                                  StoryStringEdit edit) {
    scriptName = storyMessageStrings + poolOffset;
    scriptLength = (text[0] >> 31) == 0 ? (*(unsigned char*)text & kInlineScriptLengthMask) : text[1];
    // The allocator tag is an ABI byte copied by the original call sequence.
    // Its value has no script-selection meaning; retain the volatile read.
    union Alloc {
        char byte;
        int align;
    };
    Alloc temp;
    volatile Alloc original;
    temp.byte = original.byte;
    unsigned int len = strlen(scriptName);
    String_ReplaceRange((char*)text, edit == kAppendScript ? scriptLength : 0, edit == kAppendScript ? 0 : scriptLength,
                        scriptName, len + scriptName, &temp.byte);
}
// Choose window/character IN, OUT and no-scroll script names from the current panel and character selection.
void StoryMessage_SelectTransitionScripts(void* self, int characterSelection) {
    Owner* owner = AsMessageOwner(self);
    char* scriptName;
    // Phase 1: seed baseline names and clear the optional character-out name.
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->defaultNoScrollScript,
                       STORY_MEMBER_OFFSET(StoryStrings, defaultScript), kReplaceScript);
    }
    if (!(owner->characterOutNoScrollScript[0] >> 31)) {
        ((char*)owner->characterOutNoScrollScript)[1] = 0;
        struct ShortHead {
            CW_BITS2(unsigned char flag : 1, unsigned char length : 7)
        };
        ((ShortHead*)owner->characterOutNoScrollScript)->length = 0;
    } else {
        *(*(char**)&owner->characterOutNoScrollScript[2]) = 0;
        owner->characterOutNoScrollScript[1] = 0;
    }
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->outNoScrollScript, STORY_MEMBER_OFFSET(StoryStrings, outScript),
                       kReplaceScript);
    }
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->defaultWindowScript,
                       STORY_MEMBER_OFFSET(StoryStrings, defaultScript), kReplaceScript);
    }
    unsigned int scriptFlags = owner->transitionNoScrollScript[0];
    int usesInlineScript = (unsigned int)__cntlzw(scriptFlags >> 31) >> 5;

    // Phase 2: select the transition base. "window" means character ID -1.
    // Existing transition?  old character   new character   transition base
    // yes                   window          window          defaultWindowOnly
    // yes                   character       window          characterWindowOut
    // yes                   window          character       characterWindowIn
    // yes                   character       character       default
    // no                    either          window          in + windowOnlySuffix
    // no                    either          character       in
    // Window-only selection also appends windowOnlySuffix to default/out names;
    // a new character installs characterWindowOutNoScroll as its separate out name.
    if ((usesInlineScript ? ((*(unsigned char*)&owner->transitionNoScrollScript) & kInlineScriptLengthMask)
                          : owner->transitionNoScrollScript[1])) {
        if (characterSelection == kNoStoryResource) {
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->outNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->defaultWindowScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
            if (owner->currentCharacterId == kNoStoryResource) {
                {
                    int scriptLength;
                    editScriptName(scriptLength, scriptName, owner->defaultNoScrollScript,
                                   STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
                }
                {
                    int scriptLength;
                    editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                                   STORY_MEMBER_OFFSET(StoryStrings, defaultWindowOnly), kReplaceScript);
                }
            } else {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, characterWindowOut), kReplaceScript);
            }
        } else {
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->characterOutNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, characterWindowOutNoScroll), kReplaceScript);
            }
            if (owner->currentCharacterId == kNoStoryResource) {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, characterWindowIn), kReplaceScript);
            } else {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, defaultScript), kReplaceScript);
            }
        }
    } else {
        {
            int scriptLength;
            editScriptNameWithStorageMode(scriptLength, scriptName, owner->transitionNoScrollScript,
                                          STORY_MEMBER_OFFSET(StoryStrings, inScript), kReplaceScript,
                                          usesInlineScript);
        }
        if (characterSelection == kNoStoryResource) {
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->defaultNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->outNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->defaultWindowScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
            {
                int scriptLength;
                editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                               STORY_MEMBER_OFFSET(StoryStrings, windowOnlySuffix), kAppendScript);
            }
        } else {
            int scriptLength;
            editScriptName(scriptLength, scriptName, owner->characterOutNoScrollScript,
                           STORY_MEMBER_OFFSET(StoryStrings, characterWindowOutNoScroll), kReplaceScript);
        }
    }
    // Phase 3: only these three selected paths receive the common no-scroll suffix.
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->defaultNoScrollScript,
                       STORY_MEMBER_OFFSET(StoryStrings, noScrollSuffix), kAppendScript);
    }
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->outNoScrollScript,
                       STORY_MEMBER_OFFSET(StoryStrings, noScrollSuffix), kAppendScript);
    }
    {
        int scriptLength;
        editScriptName(scriptLength, scriptName, owner->transitionNoScrollScript,
                       STORY_MEMBER_OFFSET(StoryStrings, noScrollSuffix), kAppendScript);
    }
}
#pragma pop
void EventMessage_ResetChildAndText(Owner*);
void CBpCtrl_dtor();
void* IdMap_Lookup(void*, void*);
void* BpObject_SetEnabled(void*, int);
// Clear a tagged script string while retaining its allocated capacity.
static inline void clearScriptString(unsigned int* text) {
    unsigned int flags = text[0];
    if ((flags >> 31) == 0) {
        ((unsigned char*)text)[1] = 0;
        signed char low = *(signed char*)text;
        *(signed char*)text = low & ~0x7F;
    } else {
        **(unsigned char**)&text[2] = 0;
        *(int*)&text[1] = 0;
    }
}

// Disable the panel script, destroy its child text, reset resource selections, and clear script names.
void StoryMessage_DisableAndResetPanel(Owner* self) {
    void* result;
    if (self->scriptId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        result = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&self->scriptId));
    } else {
        result = 0;
    }
    BpObject_SetEnabled(result, 0);
    void* child = self->secondaryMessageDraw;
    if (child) {
        if (child) {
            if (!Sing_805DC650()) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                Sing_805DC650() = s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey(Sing_805DC650(), child);
            if (child) {
                if (child) {
                    ((Obj*)child)->destroy(1);
                }
            }
        }
        self->secondaryMessageDraw = 0;
    }
    self->panelOpacity = 0;
    EventMessage_RebuildMessageDraw(self);
    StoryMessage_SelectTransitionScripts(self, -1);
    self->resourceIndex = -1;
    self->resourceTable = -1;
    self->currentCharacterId = -1;
    EventMessage_RebuildMessageDraw(self);

    clearScriptString(self->transitionNoScrollScript);
    clearScriptString(self->outNoScrollScript);
    clearScriptString(self->characterOutNoScrollScript);
    clearScriptString(self->defaultWindowScript);
    clearScriptString(self->defaultNoScrollScript);
}
// Destroy child text, reset resource selections, and clear all five script-name strings.
void EventMessage_ResetChildAndText(Owner* self) {
    void* child = self->secondaryMessageDraw;
    if (child) {
        if (child) {
            if (!Sing_805DC650()) {
                if (!s_pointerPairListInitialized) {
                    PairVector_InitReserved(s_messageDrawPairList);
                    __register_global_object(s_messageDrawPairList, MessageDrawPairList_Dtor,
                                             s_partyItemVectorDtorRegistration);
                    s_pointerPairListInitialized = 1;
                }
                Sing_805DC650() = s_messageDrawPairList;
            }
            PointerPairList_RemoveByKey(Sing_805DC650(), child);
            if (child) {
                if (child) {
                    ((Obj*)child)->destroy(1);
                }
            }
        }
        self->secondaryMessageDraw = 0;
    }
    self->panelOpacity = 0;
    EventMessage_RebuildMessageDraw(self);
    StoryMessage_SelectTransitionScripts(self, -1);
    self->resourceIndex = -1;
    self->resourceTable = -1;
    self->currentCharacterId = -1;
    EventMessage_RebuildMessageDraw(self);

    clearScriptString(self->transitionNoScrollScript);
    clearScriptString(self->outNoScrollScript);
    clearScriptString(self->characterOutNoScrollScript);
    clearScriptString(self->defaultWindowScript);
    clearScriptString(self->defaultNoScrollScript);
}

// Expand a message tag through owner context slot zero without the extra expansion mode.
void StoryMessage_ExpandContextTag(void** contexts, int outputAddress, unsigned char tagIndex) {
    Text_ExpandTaggedText(*contexts, (void*)outputAddress, messageContextTagTable[tagIndex], 0);
}

// Expand a story message tag through owner context slot one with the extra expansion mode.
void StoryMessage_ExpandCharacterNameTag(void** contexts, int outputAddress, unsigned char tagIndex) {
    Text_ExpandTaggedText(contexts[1], (void*)outputAddress, characterNameMessageTagTable[tagIndex], 1);
}

// Expand an alternate story message tag through owner context slot one with the extra expansion mode.
void StoryMessage_ExpandAlternateNameTag(void** contexts, int outputAddress, unsigned char tagIndex) {
    Text_ExpandTaggedText(contexts[1], (void*)outputAddress, storyAlternateNameMessageTagTable[tagIndex], 1);
}

void UIRender_SetDefaultState();
void* BpObject_TryInvokeCurrentCallback(void*, int);
void BpObject_Draw(void*);
void* SpdManager_GetSpriteAt(void*, int);
void Sprite_GetSourceSize2D(void*, float*);
void Sprite_GetPosition2D(void*, float*);
void Engine_SetFlexibleVertexFormat(int);
void* GX_GetPrimitiveCount(int, int);
void DrawPrimitiveUP(int, void*, void*, int);
void Text_DrawGlyphs(void*);
extern Quad24 kStoryPanelQuadTemplate;

struct RenderObj {
    virtual void v0();
    virtual void v1();
    virtual void v2();
    virtual void run();
};

// Run and draw the panel script, draw its opacity-tinted sprite quad, then reveal both message renderers.
void StoryConte_DrawPanelAndMessages(Owner* self) {
    if (!self->drawObject)
        return;
    UIRender_SetDefaultState();
    void* callbackObject;
    if (self->scriptId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        callbackObject = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&self->scriptId));
    } else {
        callbackObject = 0;
    }
    BpObject_TryInvokeCurrentCallback(callbackObject, 0);
    void* drawObject;
    if (self->scriptId != -1) {
        if (!Sing_805DC5F0()) {
            if (!s_bpCtrlInitialized) {
                ((CBpCtrl*)s_bpCtrl)->__ct();
                __register_global_object(s_bpCtrl, CBpCtrl_dtor, s_bpCtrlDtorRegistration_StoryMessage);
                s_bpCtrlInitialized = 1;
            }
            Sing_805DC5F0() = s_bpCtrl;
        }
        drawObject = IdMap_Lookup(Sing_805DC5F0(), (*(void**)&self->scriptId));
    } else {
        drawObject = 0;
    }
    BpObject_Draw(drawObject);
    if ((*(int*)&self->panelOpacity)) {
        int tableIndex = self->resourceTable;
        if (tableIndex >= 0) {
            int entryIndex = self->resourceIndex;
            if (entryIndex >= 0) {
                void* renderTable = self->renderTables[tableIndex];
                RenderObj* obj = (RenderObj*)((char*)renderTable + entryIndex * kStoryRenderEntryStride);
                obj->run();
                float size[2] = {}, pos[2] = {};
                Sprite_GetSourceSize2D(SpdManager_GetSpriteAt(self->drawObject, 0), size);
                Sprite_GetPosition2D(SpdManager_GetSpriteAt(self->drawObject, 0), pos);
                StoryQuadVertex vertices[kStoryQuadVertexCount];
                *(Quad24*)vertices = kStoryPanelQuadTemplate;
                float y = pos[1];
                float x = pos[0];
                float w = size[0];
                float h = size[1];
                vertices[0].x = x;
                vertices[0].y = y;
                vertices[0].alpha = (*(int*)&self->panelOpacity);
                vertices[1].x = x + w;
                vertices[1].y = y;
                vertices[1].alpha = (*(int*)&self->panelOpacity);
                vertices[2].x = x;
                vertices[2].y = y + h;
                vertices[2].alpha = (*(int*)&self->panelOpacity);
                vertices[3].x = x + w;
                vertices[3].y = y + h;
                vertices[3].alpha = (*(int*)&self->panelOpacity);
                Engine_SetFlexibleVertexFormat(kStoryQuadFormat);
                void* primitiveCount = GX_GetPrimitiveCount(kStoryQuadVertexCount, kStoryTriangleStrip);
                DrawPrimitiveUP(kStoryTriangleStrip, primitiveCount, vertices, kStoryVertexStride);
            }
        }
        if (self->messageDraw) {
            ((CMsgDraw*)(self->messageDraw))->UpdateReveal();
            Text_DrawGlyphs(self->messageDraw);
        }
        if (self->secondaryMessageDraw) {
            ((CMsgDraw*)(self->secondaryMessageDraw))->UpdateReveal();
            Text_DrawGlyphs(self->secondaryMessageDraw);
        }
    }
}

} // extern "C"
