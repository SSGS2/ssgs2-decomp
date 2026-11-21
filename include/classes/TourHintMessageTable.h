// Layout view of the message-pointer table within tourHintLayoutName, owned by tour_hint_window.cpp.
// TourHint_ShowFormattedText loads these eight words, indexes them by input.state, and passes
// the selected pointer to Text_ExpandTaggedText. Other table contents stay opaque.
#ifndef CLASSES_TOURHINTMESSAGETABLE_H
#define CLASSES_TOURHINTMESSAGETABLE_H

struct TourHintMessageTable {
    char pad_000[0x340];
    int stateMessageText[8]; // +0x340..+0x35C: TourHint_ShowFormattedText (0x802FF968)
};

#define OFF(f, o) typedef char TourHintMessageTable_off_##f[((int)&((TourHintMessageTable*)0)->f == (o)) ? 1 : -1]
OFF(stateMessageText, 0x340);
#undef OFF

#endif
