#ifndef INVENTORY_H
#define INVENTORY_H

#define MAX_STACK_AMOUNT 5

#include "Item.h"


class Inventory {
private:
   // TODO: Use std::array<ItemInstance*, 6>
   std::vector<ItemInstance*> items;
   std::vector<ItemInstance*> _getAvailableRecipeParts(const ItemTemplatePtr recipe);

public:
    
   Inventory() {
      items = { 0, 0, 0, 0, 0, 0, 0 };
   }
   
   // TODO: Add destructor that clears items

   const ItemInstance* addItem(const ItemTemplatePtr itemTemplate);
   void swapItems(uint8 slotFrom, uint8 slotTo);
   const std::vector<ItemInstance*>& getItems() const { return items; }
   void removeItem(uint8 slot);
   
   std::vector<ItemInstance*> getAvailableRecipeParts(const ItemTemplatePtr recipe);
    
};

#endif   /* INVENTORY_H */

