// Shared 0x5C-byte header of a Wii pangya.iff item record.
#ifndef CLASSES_SHOPPRODUCTRECORD_H
#define CLASSES_SHOPPRODUCTRECORD_H

#include "iff/IffStruct.h"
typedef IffItemCommon ShopProductRecord;

#define OFF(f, o) typedef char ShopProductRecord_off_##f[((int)&((ShopProductRecord*)0)->f == (o)) ? 1 : -1]
OFF(typeId, 0x50);
#undef OFF

#endif
