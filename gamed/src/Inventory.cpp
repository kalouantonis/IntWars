#include "Inventory.h"
#include "stdafx.h"
#include "ItemManager.h"

using namespace std;

Inventory::~Inventory() {
   for(ItemInstance* instance : items) {
      if(instance != nullptr) 
         delete instance;
   }
}

const ItemInstance* Inventory::addItem(const ItemTemplatePtr itemTemplate) {
   int slot = -1;
   
   if(itemTemplate->isTrinket()) {
      if(items[6] == nullptr) {
         items[6] = new ItemInstance(itemTemplate, 6, 1);
         return items[6];
      }
      
      return nullptr;
   }

   if(itemTemplate->getMaxStack() > 1) {
      for(slot = 0; slot < 6; ++slot) {
         if(items[slot] == nullptr) {
            continue;
         }
      
         if(items[slot]->getTemplate() == itemTemplate) {
            if(items[slot]->getStacks() < itemTemplate->getMaxStack()) {
               items[slot]->incrementStacks();
               return items[slot];
            } else if(items[slot]->getStacks() == itemTemplate->getMaxStack()) {
               return nullptr;
            }
         }
      }
   }
   
   for(slot = 0; slot < 6; ++slot) {
      if(items[slot] == nullptr) {
         break;
      }
   }
   
   if(slot == 6) { // Inventory full
      return nullptr;
   }
   
   printf("Adding item %d to slot %d\n", itemTemplate->getId(), slot);
   items[slot] = new ItemInstance(itemTemplate, slot, 1);
   
   return items[slot];
}

vector<ItemInstance*> Inventory::getAvailableRecipeParts(const ItemTemplatePtr recipe) {
   vector<ItemInstance*> toReturn;
   
   for(uint32 itemId : recipe->getRecipeParts()) {
      const ItemTemplatePtr item = ItemManager::getInstance().getItemTemplateById(itemId);
      if(!item) {
         continue;
      }
      vector<ItemInstance*> parts = _getAvailableRecipeParts(item);
      toReturn.insert(toReturn.begin(), parts.begin(), parts.end());
   }
   
   for(ItemInstance* i : items) {
      if(i) {
         i->setRecipeSearchFlag(false);
      }
   }
   
   return toReturn;
}

vector<ItemInstance*> Inventory::_getAvailableRecipeParts(const ItemTemplatePtr recipe) {
   vector<ItemInstance*> toReturn;
   
   for(ItemInstance* i : items) {
      if(!i) {
         continue;
      }
      
      if(i->getTemplate()->getId() == recipe->getId() && !i->getRecipeSearchFlag()) {
         toReturn.push_back(i);
         i->setRecipeSearchFlag(true);
         return toReturn;
      }
   }
   
   for(uint32 itemId : recipe->getRecipeParts()) {
      vector<ItemInstance*> parts = _getAvailableRecipeParts(ItemManager::getInstance().getItemTemplateById(itemId));
      toReturn.insert(toReturn.begin(), parts.begin(), parts.end());
   }
   
   return toReturn;
}

void Inventory::swapItems(uint8 slotFrom, uint8 slotTo) {
   ItemInstance* to = items[slotTo];
   items[slotTo] = items[slotFrom];
   items[slotFrom] = to;
}

void Inventory::removeItem(uint8 slot) {
   if(items[slot] == nullptr) {
      return;
   }
   
   delete items[slot];
   items[slot] = nullptr;
}