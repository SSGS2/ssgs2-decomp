// CGridLayout: layout view for CGrid methods in golf_grid_layout_and_task.cpp.
// Native construction/EH classes stay in the translation unit; this view emits no methods or vtable.
#ifndef CLASSES_CGRIDLAYOUT_H
#define CLASSES_CGRIDLAYOUT_H

// CGrid_BuildCells steps cells by 0x1C and rows by 0x118 (ten cells).
// CGrid_Draw draws position at +0 and markerPosition at +0xC;
// CGrid_UpdateSelectionAndProfile sets marked at +0x18 for the selected cell.
struct CGridCell {
    float position[3];
    float markerPosition[3];
    unsigned char marked;
    char pad_19[3];
};
typedef char CGridCell_size_check[(sizeof(CGridCell) == 0x1C) ? 1 : -1];
#define OFF(f, o) typedef char CGridCell_off_##f[((int)&((CGridCell*)0)->f == (o)) ? 1 : -1]
OFF(position, 0x0);
OFF(markerPosition, 0xC);
OFF(marked, 0x18);
#undef OFF

class CGridLayout {
public:
    char pad_0[0x4];
    unsigned int flags; // 0x4: CGrid_BuildCells / CGrid_HandleMsg set and clear state bits
    int unk_8; // 0x8: CGrid::CGrid initializes to 1
    char pad_C[0xC];
    void* renderVtable; // 0x18: CGrid::CGrid installs __vt__5CGrid + 0x3C
    float origin[3]; // 0x1C: CGrid_BuildCells adds grid offsets; message 2 copies position
    CGridCell cells[21][10]; // 0x28: 0x16F8 bytes before heading; CGrid_BuildCells generates surface/marker positions
    float heading; // 0x1720: CGrid_BuildCells passes to Matrix_SetAxisRotation(axis 1)
    int columns; // 0x1724: CGrid_BuildCells inner column loop bound
    int rows; // 0x1728: CGrid_BuildCells outer row loop bound
    float spacing; // 0x172C: CGrid_BuildCells multiplies row/column displacement
    int extraColumns; // 0x1730: CGrid_BuildCells optional named mode column adjustment
    int extraRows; // 0x1734: CGrid_BuildCells optional named mode row adjustment
    float extraSpacing; // 0x1738: CGrid_BuildCells optional named mode spacing adjustment
    int selectedRow; // 0x173C: CGrid_UpdateSelectionAndProfile projects onto row axis; draw selects adjacent cells
    int selectedColumn; // 0x1740: CGrid_UpdateSelectionAndProfile projects onto column axis
    unsigned char selectionUpdated; // 0x1744: CGrid_UpdateSelectionAndProfile sets after computing selection
    char pad_1745[0x3];
    float rowAxis[3]; // 0x1748: CGrid_UpdateSelectionAndProfile dot product selects row
    float columnAxis[3]; // 0x1754: CGrid_UpdateSelectionAndProfile dot product selects column
    void* bitmapId; // 0x1760: CGrid_OnLoad: ResourceManager_LoadBitmap; OnDestroy releases ID
    void* secondaryResourceId; // 0x1764: CGrid_OnDestroy releases resource ID; load meaning unknown
    unsigned char hideEnabled; // 0x1768: CGrid_HandleMsg cases 3/5 gate hidden flag
    char pad_1769[0x3];
    float profileScale; // 0x176C: CGrid_HandleMsg scales an integer message value
    unsigned char alpha; // 0x1770: draw copies to Vtx.a
    unsigned char red; // 0x1771: draw copies to Vtx.r
    unsigned char green; // 0x1772: draw copies to Vtx.g
    unsigned char blue; // 0x1773: draw copies to Vtx.b
    void* quadVertex0; // 0x1774: CGrid_OnPreLoadInit links embedded records
    void* quadVertex1; // 0x1778: CGrid_OnPreLoadInit links embedded records
    void* quadVertex3; // 0x177C: CGrid_OnPreLoadInit links embedded records
    void* quadVertex2; // 0x1780: CGrid_OnPreLoadInit links embedded records
    void* quadVertexListTerminator; // 0x1784: CGrid_OnPreLoadInit links embedded records
    char pad_1788[0x18];
    float quadVertex0U; // 0x17A0: CGrid_OnPreLoadInit initializes embedded vertex UV component
    float quadVertex0V; // 0x17A4: CGrid_OnPreLoadInit initializes embedded vertex UV component
    char pad_17A8[0x30];
    float quadVertex1U; // 0x17D8: CGrid_OnPreLoadInit initializes embedded vertex UV component
    float quadVertex1V; // 0x17DC: CGrid_OnPreLoadInit initializes embedded vertex UV component
    char pad_17E0[0x30];
    float quadVertex2U; // 0x1810: CGrid_OnPreLoadInit initializes embedded vertex UV component
    float quadVertex2V; // 0x1814: CGrid_OnPreLoadInit initializes embedded vertex UV component
    char pad_1818[0x30];
    float quadVertex3U; // 0x1848: CGrid_OnPreLoadInit initializes embedded vertex UV component
    float quadVertex3V; // 0x184C: CGrid_OnPreLoadInit initializes embedded vertex UV component
    char pad_1850[0x20];
    void* vertexBuffer; // 0x1870: CGrid_CreateBuffers creates graphics buffer; draw binds and flushes it
    void* indexBuffer; // 0x1874: CGrid_CreateBuffers creates graphics buffer; draw binds and flushes it
    void* markerVertexBuffer; // 0x1878: CGrid_CreateBuffers creates graphics buffer; draw binds and flushes it
    void* markerIndexBuffer; // 0x187C: CGrid_CreateBuffers creates graphics buffer; draw binds and flushes it
};

#define OFF(f, o) typedef char CGridLayout_off_##f[((int)&((CGridLayout*)0)->f == (o)) ? 1 : -1]
OFF(flags, 0x4);
OFF(unk_8, 0x8);
OFF(renderVtable, 0x18);
OFF(origin, 0x1C);
OFF(cells, 0x28);
OFF(heading, 0x1720);
OFF(columns, 0x1724);
OFF(rows, 0x1728);
OFF(spacing, 0x172C);
OFF(extraColumns, 0x1730);
OFF(extraRows, 0x1734);
OFF(extraSpacing, 0x1738);
OFF(selectedRow, 0x173C);
OFF(selectedColumn, 0x1740);
OFF(selectionUpdated, 0x1744);
OFF(rowAxis, 0x1748);
OFF(columnAxis, 0x1754);
OFF(bitmapId, 0x1760);
OFF(secondaryResourceId, 0x1764);
OFF(hideEnabled, 0x1768);
OFF(profileScale, 0x176C);
OFF(alpha, 0x1770);
OFF(red, 0x1771);
OFF(green, 0x1772);
OFF(blue, 0x1773);
OFF(quadVertex0, 0x1774);
OFF(quadVertex1, 0x1778);
OFF(quadVertex3, 0x177C);
OFF(quadVertex2, 0x1780);
OFF(quadVertexListTerminator, 0x1784);
OFF(quadVertex0U, 0x17A0);
OFF(quadVertex0V, 0x17A4);
OFF(quadVertex1U, 0x17D8);
OFF(quadVertex1V, 0x17DC);
OFF(quadVertex2U, 0x1810);
OFF(quadVertex2V, 0x1814);
OFF(quadVertex3U, 0x1848);
OFF(quadVertex3V, 0x184C);
OFF(vertexBuffer, 0x1870);
OFF(indexBuffer, 0x1874);
OFF(markerVertexBuffer, 0x1878);
OFF(markerIndexBuffer, 0x187C);
#undef OFF

#endif
