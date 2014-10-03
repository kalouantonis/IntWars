#include "Champion.h"
#include "RAFManager.h"
#include "Inibin.h"
#include "Map.h"
#include "Game.h"
#include "LuaScript.h"
#include <sstream>

Champion::Champion(const std::string& type, Map* map, uint32 id, uint32 playerId) : Unit(map, id, type, new Stats(), 30, 0, 0, 1200), type(type), skillPoints(0), respawnTimer(0), playerId(playerId)  {
   stats->setGold(475.0f);
   stats->setAttackSpeedMultiplier(1.0f);
   stats->setGoldPerSecond(map->getGoldPerSecond());
   stats->setGeneratingGold(false);

   std::vector<unsigned char> iniFile;
   if(!RAFManager::getInstance()->readFile("DATA/Characters/"+type+"/"+type+".inibin", iniFile)) {
      printf("ERR : couldn't find champion stats for %s\n", type.c_str());
      return;
   }
   
   Inibin inibin(iniFile);
   
   stats->setCurrentHealth(inibin.getFloatValue("Data", "BaseHP"));
   stats->setMaxHealth(inibin.getFloatValue("Data", "BaseHP"));
   stats->setCurrentMana(inibin.getFloatValue("Data", "BaseMP"));
   stats->setMaxMana(inibin.getFloatValue("Data", "BaseMP"));
   stats->setBaseAd(inibin.getFloatValue("DATA", "BaseDamage"));
   stats->setRange(inibin.getFloatValue("DATA", "AttackRange"));
   stats->setBaseMovementSpeed(inibin.getFloatValue("DATA", "MoveSpeed"));
   stats->setArmor(inibin.getFloatValue("DATA", "Armor"));
   stats->setMagicArmor(inibin.getFloatValue("DATA", "SpellBlock"));
   stats->setHp5(inibin.getFloatValue("DATA", "BaseStaticHPRegen"));
   stats->setMp5(inibin.getFloatValue("DATA", "BaseStaticMPRegen"));
   
   stats->setHealthPerLevel(inibin.getFloatValue("DATA", "HPPerLevel"));
   stats->setManaPerLevel(inibin.getFloatValue("DATA", "MPPerLevel"));
   stats->setAdPerLevel(inibin.getFloatValue("DATA", "DamagePerLevel"));
   stats->setArmorPerLevel(inibin.getFloatValue("DATA", "ArmorPerLevel"));
   stats->setMagicArmorPerLevel(inibin.getFloatValue("DATA", "SpellBlockPerLevel"));
   stats->setHp5RegenPerLevel(inibin.getFloatValue("DATA", "HPRegenPerLevel"));
   stats->setMp5RegenPerLevel(inibin.getFloatValue("DATA", "MPRegenPerLevel"));
   stats->setBaseAttackSpeed(0.625f/(1+inibin.getFloatValue("DATA", "AttackDelayOffsetPercent")));
   
   spells.push_back(new Spell(this, inibin.getStringValue("Data", "Spell1"), 0));
   spells.push_back(new Spell(this, inibin.getStringValue("Data", "Spell2"), 1));
   spells.push_back(new Spell(this, inibin.getStringValue("Data", "Spell3"), 2));
   spells.push_back(new Spell(this, inibin.getStringValue("Data", "Spell4"), 3));
   
   setMelee(inibin.getBoolValue("DATA", "IsMelee"));
   setCollisionRadius(inibin.getIntValue("DATA", "PathfindingCollisionRadius"));
   
   iniFile.clear();
   if(!RAFManager::getInstance()->readFile("DATA/Characters/"+type+"/Spells/"+type+"BasicAttack.inibin", iniFile)) {
      if(!RAFManager::getInstance()->readFile("DATA/Spells/"+type+"BasicAttack.inibin", iniFile)) {
         printf("ERR : couldn't find champion auto-attack data for %s\n", type.c_str());
         return;
      }
   }
   
   Inibin autoAttack(iniFile);
   
   autoAttackDelay = autoAttack.getFloatValue("SpellData", "castFrame")/30.f;
   autoAttackProjectileSpeed = autoAttack.getFloatValue("SpellData", "MissileSpeed");
   
   std::string scriptloc = "../../lua/champions/" + this->getType() + "/Passive.lua";
	printf("Loading %s\n", scriptloc.c_str());
   try{
    unitScript = LuaScript(true);//fix
    
    unitScript.lua.set("me", this);

    unitScript.loadScript(scriptloc);
    
    unitScript.lua.set_function("dealMagicDamage", [this](Unit* target, float amount) { this->dealDamageTo(target,amount,DAMAGE_TYPE_MAGICAL,DAMAGE_SOURCE_SPELL); });
    unitScript.lua.set_function("addBuff", [this](Unit* target, Buff b){
      target->addBuff(new Buff(b));
      return;
   });
   
    unitScript.lua.set_function("addParticleTarget", [this](const std::string& particle, Target* u) { 
      this->getMap()->getGame()->notifyParticleSpawn(this, u, particle);
      return;
   });

   // unitScript.lua.set ("me", this);
   }catch(sol::error e){//lua error? don't crash the whole server
      printf("Champion passive load error: \n%s\n", e.what());
   }
}

Spell* Champion::castSpell(uint8 slot, float x, float y, Unit* target, uint32 futureProjNetId, uint32 spellNetId) {
   if(slot >= spells.size()) {
      return 0;
   }
   
   Spell* s = spells[slot];
   
   s->setSlot(slot);//temporary hack until we redo spells to be almost fully lua-based
   
   if((s->getCost() * (1 - stats->getSpellCostReduction())) > stats->getCurrentMana() || s->getState() != STATE_READY) {
      return 0;
   }
   
   s->cast(x, y, target, futureProjNetId, spellNetId);
   stats->setCurrentMana(stats->getCurrentMana() - (s->getCost() * (1 - stats->getSpellCostReduction())));
   return s;
}

Spell* Champion::levelUpSpell(uint8 slot) {
   if(slot >= spells.size()) {
      return 0;
   }
   
   if(skillPoints == 0) {
      return 0;
   }
   
   spells[slot]->levelUp();
   --skillPoints;
   
   return spells[slot];
}

void Champion::update(int64 diff) {
   Unit::update(diff);

   if(!stats->isGeneratingGold() && map->getGameTime() >= map->getFirstGoldTime()) {
   		stats->setGeneratingGold(true);
   		puts("Generating Gold!");
   }

   if (respawnTimer > 0) {
      respawnTimer -= diff;
      if (respawnTimer <= 0) {
         float respawnX, respawnY;
         std::tie(respawnX, respawnY) = getRespawnPosition();
         setPosition(respawnX, respawnY);
         map->getGame()->notifyChampionRespawn(this);
         getStats().setCurrentHealth(getStats().getMaxHealth());
         getStats().setCurrentMana(getStats().getMaxMana());
         deathFlag = false;
      }
   }

   bool levelup = false;

   while (getStats().getLevel() < map->getExpToLevelUp().size() && getStats().getExp() >= map->getExpToLevelUp()[getStats().getLevel()]) {
      levelUp();
      levelup = true;
   }

   if (levelup) {
      map->getGame()->notifyLevelUp(this);
   }

   for (Spell* s : spells) {
      s->update(diff);
   }

   if (championHitFlagTimer > 0) {
      championHitFlagTimer -= diff;
      if (championHitFlagTimer <= 0) {
         championHitFlagTimer = 0;
      }
   }
}

uint32 Champion::getChampionHash() {
   char szSkin[4];
   sprintf(szSkin, "%02d", skin);
   uint32 hash = 0;
   const char *gobj = "[Character]";
   for(unsigned int i = 0; i < strlen(gobj); i++) {
     hash = tolower(gobj[i]) + (0x1003F * hash);
   }
   for(unsigned int i = 0; i < type.length(); i++) {
     hash = tolower(type[i]) + (0x1003F * hash);
   }
   for(unsigned int i = 0; i < strlen(szSkin); i++) {
     hash = tolower(szSkin[i]) + (0x1003F * hash);
   }
   return hash;
}

void Champion::levelUp() {
   printf("Champion %s Levelup to %d\n", getType().c_str(), getStats().getLevel()+1);
   getStats().levelUp();
   ++skillPoints;
}
std::pair<float, float> Champion::getRespawnPosition() {
   LuaScript configScript(false);
   //get map ID
   configScript.loadScript("../../lua/config.lua");
   sol::table gameTable = configScript.getTable("game");
   sol::table playersTable = configScript.getTable("players");
   uint32 mapId = gameTable.get<int>("map");
   //get player team
   std::string playerTeam;
   std::ostringstream playerTableName;
   playerTableName << "player" << playerId;
   puts(playerTableName.str().c_str());
   sol::table playerTable = playersTable.get<sol::table>(playerTableName.str());
   playerTeam = playerTable.get<std::string>("team");
   int spawnNumber = 0;
   //get player spawn position number
   int playersBefore = 0;
   for (int i = 1; i < 13; i++){
      std::ostringstream iplayerTableName;
      iplayerTableName << "player" << i;
      sol::table iplayerTable = playersTable.get<sol::table>(iplayerTableName.str());
      if (i == playerId) {
         playersBefore++;
         spawnNumber = playersBefore;
         break;
      }
      if (iplayerTable.get<std::string>("team") == playerTeam){
         playersBefore++;
      }

   }
   printf("team: %s\n", playerTeam.c_str());
   printf("players before %i \n", playersBefore);
   //printf("player position in spawn list: %s", to_string(spawnNumber).c_str());
   LuaScript mapScript(false);
   std::ostringstream mapPath;
   mapPath << "../../lua/maps/map" << mapId << ".lua";
   mapScript.loadScript(mapPath.str());
   //printf("Map script: %s \n", mapPath.str().c_str());
   //printf(playerTeam.c_str());
   sol::table teamSizeSpawners;
   sol::table teamSpawners;
   int teamSize = getTeamSize();
   try {
      sol::table spawnersTable = mapScript.getTable("spawnpoints");
      teamSpawners = spawnersTable.get<sol::table>(playerTeam);

   }
   catch (sol::error e) {
      printf("Error loading champion for \n%s\n", e.what());
   }
   printf("team size: %i\n", teamSize);
   teamSizeSpawners = teamSpawners.get<sol::table>(to_string(teamSize));
   printf("%f\n", teamSizeSpawners.get<float>("player" + to_string(spawnNumber) + "X"));
   return std::make_pair(teamSizeSpawners.get<float>("player" + to_string(spawnNumber) + "X"), teamSizeSpawners.get<float>("player" + to_string(spawnNumber) + "Y"));
}
void Champion::die(Unit* killer) {
   respawnTimer = 5000000 + getStats().getLevel()*2500000;
   map->stopTargeting(this);
   
   Champion* cKiller = dynamic_cast<Champion*>(killer);
   
	if(!cKiller && this->championHitFlagTimer > 0){
      cKiller = dynamic_cast<Champion*>(map->getObjectById(this->playerHitId));
      printf("Killed by turret, minion or monster, but still  give gold to the enemy.\n");
   }
   
   if (!cKiller) {
      map->getGame()->notifyChampionDie(this, killer);
      return;
   }
   
   map->getGame()->notifyChampionDie(this, cKiller);
   
   cKiller->setChampionGoldFromMinions(0);
   
   float gold = map->getGoldFor(this);
   printf("Before: getGoldFromChamp: %f\n Killer: %i\n Victim: %i\n", gold, cKiller->killDeathCounter,this->killDeathCounter);
   
   if(cKiller->killDeathCounter < 0){
      cKiller->killDeathCounter = 0;
   }
   
   if(cKiller->killDeathCounter >= 0){
      cKiller->killDeathCounter += 1;
   }
   
   if(this->killDeathCounter > 0){
      this->killDeathCounter = 0;
   }
   
   if(this->killDeathCounter <= 0){
      this->killDeathCounter -= 1;
   }
    
   if(!gold) {
      return;
   }
    
   if(map->getKillReduction() && !map->getFirstBlood()){
      gold -= gold*0.25f;
      printf("Still some minutes for full gold reward on champion kills");
   }
   
   if(map->getFirstBlood()){
      gold += 100;
      map->setFirstBlood(false);
   }
   
	cKiller->getStats().setGold(cKiller->getStats().getGold() + gold);
	map->getGame()->notifyAddGold(cKiller, this, gold);
   
   printf("After: getGoldFromChamp: %f\n Killer: %i\n Victim: %i\n", gold, cKiller->killDeathCounter,this->killDeathCounter);
   
   map->stopTargeting(this);
}

void Champion::dealDamageTo(Unit* target, float damage, DamageType type, DamageSource source) {
   Unit::dealDamageTo(target,damage,type,source);;
   
   Champion* cTarget = dynamic_cast<Champion*>(target);
   
   if (!cTarget) {
      return;
   }
   
   cTarget->setChampionHitFlagTimer(15*1000000); //15 seconds timer, so when you get executed the last enemy champion who hit you gets the gold
   cTarget->playerHitId = this->id;
   printf("15 second execution timer on you. Do not get killed by a minion, turret or monster!\n");
}


int Champion::getTeamSize(){
   LuaScript script(false);

   script.loadScript("../../lua/config.lua");

   //  sol::state lua;
   //  lua.open_libraries(sol::lib::base, sol::lib::table);

   //  lua.open_file("../../lua/config.lua");
   sol::table playerList = script.getTable("players");
   int blueTeamSize = 0;
   int purpTeamSize = 0;
   for (int i = 1; i<12; i++) {
      try {
         std::string playerIndex = "player" + to_string(i);
         sol::table playerData = playerList.get<sol::table>(playerIndex);
         std::string team = playerData.get<std::string>("team");
         if (team == "BLUE"){
            blueTeamSize++;
         }
         else {
            purpTeamSize++;
         }
      }
      catch (sol::error e) {
         // printf("Error loading champion for %i: \n%s\n", i, e.what());
         break;
      }
   }
   std::string playerTeam;
   std::ostringstream playerTableName;
   playerTableName << "player" << playerId;
   puts(playerTableName.str().c_str());
   sol::table playerTable = playerList.get<sol::table>(playerTableName.str());
   std::string team = playerTable.get<std::string>("team");
   if (team == "BLUE") {
      return blueTeamSize;
   }
   else { return purpTeamSize; }
}

void Champion::onCollision(Object *collider)
{
   if (collider == 0)
   {
      //std::cout << "I bumped into a wall!\n";
   }
   else
   {
      //std::cout << "I bumped into someone else!\n";
   }
}
