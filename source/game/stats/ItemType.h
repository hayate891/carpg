#pragma once

//-----------------------------------------------------------------------------
/* Item types
Items are sorted by item type (so weapon first, then bow, then shield etc)
Adding new item type require changes in:
+ TeamItems.cpp
+ ItemSlot.h/cpp: add new item slot if equipable, GetItemString, ItemTypeToSlot
+ Unit.cpp AddItemAndEquipIfNone, IsBetterItem
+ Net.cpp removing equipped item
+ Item.cpp CreateItemCopy, LoadItem
+ Item.h IsWearable, Casting, ItemCmp
+ Game2.cpp UpdateUnitInventory, GenerateTreasure, GetItemSound, IsBetterItem
*/
enum ITEM_TYPE
{
	IT_WEAPON,
	IT_THROWABLE,
	IT_BOW,
	IT_AMMO,
	IT_SHIELD,
	IT_ARMOR,
	IT_HELMET,
	IT_BOOTS,
	IT_AMULET,
	IT_RING,
	IT_LAST_WEARABLE = IT_RING,
	IT_OTHER,
	IT_CONSUMEABLE,
	IT_BOOK,
	IT_GOLD,

	IT_MAX_GEN = IT_BOOK,
	IT_LIST,
	IT_LEVELED_LIST,
	IT_STOCK,
	IT_BOOK_SCHEMA
};
