// CBuildProduct is declared in game/shop_product.h; the destructor stores its vtable.
// Evidence: __dt__11ProductBaseFv (0x802D...) frees indices and both per-product arrays.
#ifndef CLASSES_CBUILDPRODUCT_H
#define CLASSES_CBUILDPRODUCT_H
#include "game/shop_product.h"
typedef char CBuildProduct_off_4[((int)&((CBuildProduct*)0)->products.data == 0x4) ? 1 : -1];
typedef char CBuildProduct_off_8[((int)&((CBuildProduct*)0)->products.size == 0x8) ? 1 : -1];
typedef char CBuildProduct_off_C[((int)&((CBuildProduct*)0)->products.capacity == 0xC) ? 1 : -1];
typedef char CBuildProduct_off_10[((int)&((CBuildProduct*)0)->field10 == 0x10) ? 1 : -1];
typedef char CBuildProduct_off_14[((int)&((CBuildProduct*)0)->indices == 0x14) ? 1 : -1];
typedef char CBuildProduct_off_18[((int)&((CBuildProduct*)0)->textureBindingsBySlot == 0x18) ? 1 : -1];
typedef char CBuildProduct_off_1C[((int)&((CBuildProduct*)0)->count == 0x1C) ? 1 : -1];
typedef char CBuildProduct_off_20[((int)&((CBuildProduct*)0)->flag == 0x20) ? 1 : -1];
typedef char CBuildProduct_off_24[((int)&((CBuildProduct*)0)->object24 == 0x24) ? 1 : -1];
typedef char CBuildProduct_off_2D70[((int)&((CBuildProduct*)0)->ready == 0x2D70) ? 1 : -1];
#define AsCBuildProduct(p) ((CBuildProduct*)(p))
typedef std::vector<CShopProductInterface*> ShopProductVector;
typedef char ShopProductVector_size[(sizeof(ShopProductVector) == 12) ? 1 : -1];
typedef char ShopProductVector_off_data[((int)&((ShopProductVector*)0)->data == 0) ? 1 : -1];
typedef char ShopProductVector_off_size[((int)&((ShopProductVector*)0)->size == 4) ? 1 : -1];
typedef char ShopProductVector_off_capacity[((int)&((ShopProductVector*)0)->capacity == 8) ? 1 : -1];
#define AsShopProductVector(p) ((ShopProductVector*)(p))
#endif
