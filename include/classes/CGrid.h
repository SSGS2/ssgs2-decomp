// CGrid: the slope grid drawn around the ball. It samples the terrain into a
// rectangular cell array, builds the line and marker buffers from it, and
// highlights the cell under the ball and the cell with the pin.
//
// The grid is a native actor with a second render interface at 0x18. Its
// vtables come from the original data, so this class declares no virtual
// functions; the interface subobjects are spelled out as base records.
#ifndef CLASSES_CGRID_H
#define CLASSES_CGRID_H

#include "game/types.h"
#include "game/wvector.h"

struct GridCommand;

// One grid sample: the point on the terrain and the slope marker above it.
struct CGridCell {
    WVector position;       // 0x00
    WVector markerPosition; // 0x0C
    unsigned char marked;   // 0x18
    char pad_19[3];
};

// One of the four embedded quad vertices. Only the texture coordinates are
// used here; the rest of the vertex belongs to the render path.
struct CGridQuadVertex {
    char pad_00[0x18];
    float u; // 0x18
    float v; // 0x1C
    char pad_20[0x18];
};

// A screen vertex: position, colour and texture coordinates.
struct CGridVertex {
    float x, y, z;
    unsigned char r, g, b, a;
};

// The native actor subobject the grid is built on. It supplies the primary
// vtable, the grid's state flags and its display priority.
struct NativeBase80201D18 {
    void* vtbl;            // 0x00
    unsigned int flags;    // 0x04
    int unk_8;             // 0x08
    unsigned int skipMask; // 0x0C
    void* dispatchObject;  // 0x10
    unsigned int unk_14;   // 0x14
    NativeBase80201D18();
    ~NativeBase80201D18();
};

// The render interface subobject at 0x18.
struct NativeMember80201D18 {
    void* renderVtable; // 0x18
    NativeMember80201D18() {}
};

class CGrid : public NativeBase80201D18, public NativeMember80201D18 {
public:
    CGrid();
    ~CGrid();

    // Loading and teardown.
    void OnDestroy();
    void OnPreLoadInit();
    void OnLoad();
    void OnReload();

    // Cell geometry and palette.
    void ResetCellsAndColor();
    void CreateBuffers();
    void BuildCells();
    void OnProcess();

    // Drawing and the message interface.
    void Draw();
    void DrawThunk();
    void HandleMsg(const GridCommand* command);
    void UpdateSelectionAndProfile();
    void SetupRenderState();

    // Type information and static initialisation.
    static void* GetRTTI();
    static void InitializeFullTurnConstant();

    WVector origin;                 // 0x1C
    CGridCell cells[21][10];        // 0x28
    float heading;                  // 0x1720
    int columns;                    // 0x1724
    int rows;                       // 0x1728
    float spacing;                  // 0x172C
    int extraColumns;               // 0x1730
    int extraRows;                  // 0x1734
    float extraSpacing;             // 0x1738
    int selectedRow;                // 0x173C
    int selectedColumn;             // 0x1740
    unsigned char selectionUpdated; // 0x1744
    char pad_1745[3];
    WVector rowAxis;           // 0x1748
    WVector columnAxis;        // 0x1754
    void* bitmapId;            // 0x1760
    void* secondaryResourceId; // 0x1764
    unsigned char hideEnabled; // 0x1768
    char pad_1769[3];
    float profileScale;              // 0x176C
    unsigned char alpha;             // 0x1770
    unsigned char red;               // 0x1771
    unsigned char green;             // 0x1772
    unsigned char blue;              // 0x1773
    void* quadVertex0;               // 0x1774
    void* quadVertex1;               // 0x1778
    void* quadVertex3;               // 0x177C
    void* quadVertex2;               // 0x1780
    void* quadVertexListEnd;         // 0x1784
    CGridQuadVertex quadVertices[4]; // 0x1788
    char pad_1868[8];
    void* vertexBuffer;         // 0x1870
    void* indexBuffer;          // 0x1874
    void* markerVertexBuffer;   // 0x1878
    void* markerIndexBuffer;    // 0x187C
    CGridVertex overlayQuad[4]; // 0x1880
};

#define CGRID_OFF(f, o) typedef char CGrid_off_##f[((int)&((CGrid*)0)->f == (o)) ? 1 : -1]
CGRID_OFF(origin, 0x1C);
CGRID_OFF(cells, 0x28);
CGRID_OFF(heading, 0x1720);
CGRID_OFF(columns, 0x1724);
CGRID_OFF(rows, 0x1728);
CGRID_OFF(spacing, 0x172C);
CGRID_OFF(extraColumns, 0x1730);
CGRID_OFF(extraRows, 0x1734);
CGRID_OFF(extraSpacing, 0x1738);
CGRID_OFF(selectedRow, 0x173C);
CGRID_OFF(selectedColumn, 0x1740);
CGRID_OFF(selectionUpdated, 0x1744);
CGRID_OFF(rowAxis, 0x1748);
CGRID_OFF(columnAxis, 0x1754);
CGRID_OFF(bitmapId, 0x1760);
CGRID_OFF(secondaryResourceId, 0x1764);
CGRID_OFF(hideEnabled, 0x1768);
CGRID_OFF(profileScale, 0x176C);
CGRID_OFF(alpha, 0x1770);
CGRID_OFF(quadVertex0, 0x1774);
CGRID_OFF(quadVertices, 0x1788);
CGRID_OFF(vertexBuffer, 0x1870);
CGRID_OFF(markerIndexBuffer, 0x187C);
CGRID_OFF(overlayQuad, 0x1880);
#undef CGRID_OFF
typedef char CGrid_size[(sizeof(CGrid) == 0x18C0) ? 1 : -1];
typedef char CGridCell_size[(sizeof(CGridCell) == 0x1C) ? 1 : -1];
typedef char CGridQuadVertex_size[(sizeof(CGridQuadVertex) == 0x38) ? 1 : -1];

#endif // CLASSES_CGRID_H
