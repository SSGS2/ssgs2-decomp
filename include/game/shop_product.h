#ifndef GAME_SHOP_PRODUCT_H
#define GAME_SHOP_PRODUCT_H

namespace IFF_STRUCT {
struct sChar;
}

template <class T> class CShopProduct;

template <> class CShopProduct<IFF_STRUCT::sChar> {
public:
    CShopProduct(void* value);
    ~CShopProduct();
    virtual void slot08();
    virtual void slot0C();
    virtual void slot10();
    virtual void slot14();
    virtual void slot18();
    virtual void slot1C(unsigned char);
    char data[0x10];
};

class CShopProductAvatar : public CShopProduct<IFF_STRUCT::sChar> {
public:
    virtual void slot08();
    CShopProductAvatar(void*, unsigned char);
    ~CShopProductAvatar();

    int IsOwnedOrEquipped(void* productRecord, int testEquipped);

    int RebuildFromCharacterTable(void* filterContext);
    static int ReturnFalse();
};

class CShopProductInterface;
namespace std {
template <class T> struct allocator {};
template <class T, class Alloc = allocator<T> > class vector {
public:
    T* data;
    unsigned size;
    unsigned capacity;
    vector() : data(0), size(0), capacity(0) {}
    ~vector();
};
}

class CBuildProduct {
public:
    virtual void slot08();
    std::vector<CShopProductInterface*> products;
    void* field10;
    int* indices;
    void* textureBindingsBySlot;
    int count;
    unsigned char flag;
    char pad21[3];
    char object24[0x2D70 - 0x24];
    unsigned char ready;
    CBuildProduct(int, unsigned char);

    int vfunc_1();
    int GetProductCount(int category);
    int SelectSaveRecord(void* src);
    int* GetProductDrawEntries(int category);
    void* GetTextureEntryAt(int category, int entryIndex);
    int* GetProductRecords(int category);
};

#endif
