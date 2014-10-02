#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <unordered_map>

#include "Item.h"

class ItemManager {
private:
   std::unordered_map<uint32, ItemTemplatePtr> itemTemplates;

   ItemManager() { }
public:
   static ItemManager& getInstance() {
      static ItemManager instance;    
      return instance;
   }
   
   void init();

   const ItemTemplatePtr getItemTemplateById(uint32 id);
};

#endif