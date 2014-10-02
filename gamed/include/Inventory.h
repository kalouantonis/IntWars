#ifndef INVENTORY_H
#define INVENTORY_H

#define MAX_STACK_AMOUNT 5

#include "Item.h"

#include <array>

class Inventory {
public:
   typedef std::array<ItemInstance*, 7> ItemArray;

private:
   ItemArray items;
   std::vector<ItemInstance*> _getAvailableRecipeParts(const ItemTemplatePtr recipe);

public:
    
   Inventory() {
      items.fill(nullptr);
   }
   
   // TODO: Add destructor that clears items
   ~Inventory();

   const ItemInstance* addItem(const ItemTemplatePtr itemTemplate);
   void swapItems(uint8 slotFrom, uint8 slotTo);
   const ItemArray& getItems() const { return items; }
   void removeItem(uint8 slot);
   
   std::vector<ItemInstance*> getAvailableRecipeParts(const ItemTemplatePtr recipe);
    
};

#endif   /* INVENTORY_H */

