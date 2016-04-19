#include "Pch.h"
#include "Base.h"
#include "Item.h"
#include "ItemSlot.h"
#include "Unit.h"
#include "Language.h"
#include "QuestManager.h"
#include "SaveState.h"

cstring txAttack, txDefense, txMobility, txRequiredStrength, txDTBlunt, txDTPierce, txDTSlash, txDTBluntPierce, txDTBluntSlash, txDTSlashPierce, txDTMagical, txWeight,
	txValue, txInvalidArmor;

//=================================================================================================
void SetItemStatsText()
{
	txAttack = Str("attack");
	txDefense = Str("defense");
	txMobility = Str("mobility");
	txRequiredStrength = Str("requiredStrength");
	txDTBlunt = Str("dtBlunt");
	txDTPierce = Str("dtPierce");
	txDTSlash = Str("dtSlash");
	txDTBluntPierce = Str("dtBluntPierce");
	txDTBluntSlash = Str("dtBluntSlash");
	txDTSlashPierce = Str("dtSlashPierce");
	txDTMagical = Str("dtMagical");
	txWeight = Str("weight");
	txValue = Str("value");
	txInvalidArmor = Str("invalidArmor");
}

//=================================================================================================
// Funkcja u¿ywana do sortowanie przedmiotów
// true - s1 ma byæ wczeœniej
//=================================================================================================
static bool SortItemsPred(const ItemSlot& s1, const ItemSlot& s2)
{
	if(!s1.item)
		return false;
	if(!s2.item)
		return true;
	return ItemCmp(s1.item, s2.item);
}

//=================================================================================================
// Sortowanie przedmiotów
//=================================================================================================
void SortItems(vector<ItemSlot>& items)
{
	// sortuj przedmioty
	std::sort(items.begin(), items.end(), SortItemsPred);

	// usuñ puste elementy
	while(!items.empty() && !items.back().item)
		items.pop_back();
}

extern cstring armor_type_string[4];

void GetItemString(string& str, const Item* item, Unit* unit, uint count)
{
	assert(item);

	str = item->name;

	switch(item->type)
	{
	case IT_WEAPON:
		{
			/*
			Rapier - Short blade
			Attack: 30 (40) piercing
			Required strength: $50$
			*/
			const Weapon& weapon = item->ToWeapon();

			cstring dmg_type;
			switch(weapon.dmg_type)
			{
			case DMG_BLUNT:
				dmg_type = txDTBlunt;
				break;
			case DMG_PIERCE:
				dmg_type = txDTPierce;
				break;
			case DMG_SLASH:
				dmg_type = txDTSlash;
				break;
			case DMG_BLUNT | DMG_PIERCE:
				dmg_type = txDTBluntPierce;
				break;
			case DMG_BLUNT | DMG_SLASH:
				dmg_type = txDTBluntSlash;
				break;
			case DMG_SLASH | DMG_PIERCE:
				dmg_type = txDTSlashPierce;
				break;
			case DMG_BLUNT | DMG_PIERCE | DMG_SLASH:
				dmg_type = txDTMagical;
				break;
			default:
				dmg_type = "???";
				break;
			}

			str += Format(" - %s\n%s: %d (%d) %s\n%s: $c%c%d$c-\n",
				weapon_type_info[weapon.weapon_type].name,
				txAttack,
				weapon.dmg,
				(int)unit->CalculateAttack(item),
				dmg_type,
				txRequiredStrength,
				(unit->Get(Attribute::STR) >= weapon.req_str ? '-' : 'r'),
				weapon.req_str);
		}
		break;
	case IT_BOW:
		{
			/*
			Long bow
			Attack: 30 (40) piercing
			Required strength: $40$
			*/
			const Bow& bow = item->ToBow();
			str += Format("\n%s: %d (%d) %s\n%s: $c%c%d$c-\n",
				txAttack,
				bow.dmg,
				(int)unit->CalculateAttack(item),
				txDTPierce,
				txRequiredStrength,
				(unit->Get(Attribute::STR) >= bow.req_str ? '-' : 'r'),
				bow.req_str);
		}
		break;
	case IT_ARMOR:
		{
			/*
			Chainmail - Medium armor [(Does not fit)]
			Defense: 30 (40)
			Required strength: $40$
			Mobility: 50 (40) / Mobility: 50 (70->60)
			*/
			const Armor& armor = item->ToArmor();
			cstring mob_str, armor_type;

			cstring skill = g_skills[(int)armor.skill].name.c_str();
			if(unit->data->armor_type == armor.armor_type)
				armor_type = skill;
			else
				armor_type = Format("%s (%s)", skill, txInvalidArmor);

			int mob = unit->CalculateMobility(armor);
			int dex = unit->Get(Attribute::DEX);
			if(mob == dex)
				mob_str = Format("(%d)", dex);
			else
				mob_str = Format("(%d->%d)", dex, mob);

			str += Format(" - %s\n%s: %d (%d)\n%s: $c%c%d$c-\n%s: %d %s\n",
				armor_type,
				txDefense,
				armor.def,
				(int)unit->CalculateDefense(item),
				txRequiredStrength,
				(unit->Get(Attribute::STR) >= armor.req_str ? '-' : 'r'),
				armor.req_str,
				txMobility,
				armor.mobility,
				mob_str);
		}
		break;
	case IT_SHIELD:
		{
			/*
			Iron shield
			Defense: 30 (40)
			Required strength: $40$
			*/
			const Shield& shield = item->ToShield();
			str += Format("\n%s: %d (%d)\n%s: $c%c%d$c-\n",
				txDefense,
				shield.def,
				(int)unit->CalculateBlock(item),
				txRequiredStrength,
				(unit->Get(Attribute::STR) >= shield.req_str ? '-' : 'r'),
				shield.req_str);
		}
		break;
	default:
		str += "\n";
		break;
	}

	// waga
	str += Format(txWeight, item->GetWeight());
	if(count > 1)
		str += Format(" (%g)\n", item->GetWeight()*count);
	else
		str += "\n";

	// cena
	str += Format(txValue, item->value);
}

// dodaj stackuj¹cy siê przedmiot do wektora przedmiotów
bool InsertItemStackable(vector<ItemSlot>& items, ItemSlot& slot)
{
	vector<ItemSlot>::iterator it = std::lower_bound(items.begin(), items.end(), slot, SortItemsPred);
	if(it == items.end())
	{
		// dodaj na samym koñcu
		items.push_back(slot);
		return false;
	}
	else
	{
		if(it->item == slot.item)
		{
			// stackuj przedmiot
			it->count += slot.count;
			it->team_count += slot.team_count;
			return true;
		}
		else
		{
			// dodaj przedmiot
			items.insert(it, slot);
			return false;
		}
	}
}

// dodaj nie stackuj¹cy siê przedmiot do wektora przedmiotów
void InsertItemNotStackable(vector<ItemSlot>& items, ItemSlot& slot)
{
	vector<ItemSlot>::iterator it = std::lower_bound(items.begin(), items.end(), slot, SortItemsPred);
	uint size = items.size();
	ItemSlot slot2;
	slot2.item = slot.item;
	slot2.count = 1;
	if(slot.team_count)
	{
		slot2.team_count = 1;
		--slot.team_count;
	}
	else
		slot2.team_count = 0;

	if(it == items.end())
	{
		// powiêksz wektor
		items.reserve(size + slot.count);

		// dodaj na samym koñcu
		while(slot.count)
		{
			items.push_back(slot2);
			if(slot.team_count)
			{
				slot2.team_count = 1;
				--slot.team_count;
			}
			else
				slot2.team_count = 0;
			--slot.count;
		}
	}
	else
	{
		vector<ItemSlot> v_copy(items.begin(), it);
		v_copy.reserve(items.size()+slot.count);

		// dodaj nowe
		while(slot.count)
		{
			v_copy.push_back(slot2);
			if(slot.team_count)
			{
				slot2.team_count = 1;
				--slot.team_count;
			}
			else
				slot2.team_count = 0;
			--slot.count;
		}

		// dodaj stare z prawej
		for(vector<ItemSlot>::iterator end = items.end(); it != end; ++it)
			v_copy.push_back(*it);

		// zamieñ
		items.swap(v_copy);
	}
}

bool InsertItem(vector<ItemSlot>& items, ItemSlot& slot)
{
	assert(slot.item && slot.count && slot.team_count <= slot.count);

	if(slot.item->IsStackable())
		return InsertItemStackable(items, slot);
	else
	{
		InsertItemNotStackable(items, slot);
		return false;
	}
}

void InsertItemBare(vector<ItemSlot>& items, const Item* item, uint count, uint team_count)
{
	assert(item && count && count >= team_count);

	if(item->IsStackable())
	{
		for(vector<ItemSlot>::iterator it = items.begin(), end = items.end(); it != end; ++it)
		{
			if(it->item == item)
			{
				it->count += count;
				it->team_count += team_count;
				return;
			}
		}
	}
	else
	{
		assert(count == 1);
	}

	ItemSlot& slot = Add1(items);
	slot.Set(item, count, team_count);
}

template<bool team_count>
__forceinline void SaveItemImpl(StreamWriter& f, const ItemSlot& slot)
{
	f << slot.count;
	if(team_count)
		f << slot.team_count;
	f << slot.item->id;
	char c = slot.item->id[0];
	if(c == '$' || c == '!')
		f << slot.item->refid;
}

void SaveItem(StreamWriter& f, const ItemSlot& slot)
{
	SaveItemImpl<true>(f, slot);
}

void SaveItems(StreamWriter& f, const vector<ItemSlot>& items, bool team_count)
{
	f << items.size();
	if(team_count)
	{
		for(const ItemSlot& slot : items)
			SaveItemImpl<true>(f, slot);
	}
	else
	{
		for(const ItemSlot& slot : items)
			SaveItemImpl<false>(f, slot);
	}
}

template<bool team_count>
__forceinline bool LoadItemImpl(StreamReader& f, ItemSlot& slot, bool client)
{
	f.ReadString1();
	f >> slot.count;
	if(team_count)
		f >> slot.team_count;
	else
		slot.team_count = 0;
	if(!f)
		return false;

	char c = BUF[0];
	if(c == '$' || c == '!')
	{
		int refid;
		if(!(f >> refid))
			return false;

		QuestManager& QM = QuestManager::Get();
		bool is_new = (c == '!');

		if(!client)
		{
			QM.AddQuestItemRequest(&slot.item, BUF, refid, is_new);
			slot.item = QUEST_ITEM_PLACEHOLDER;
		}
		else
		{
			slot.item = QM.FindClientQuestItem(BUF, quest_refid, is_new);
			if(!slot.item)
			{
				WARN(Format("Missing quest item '%s' (%d).", BUF, quest_refid));
				return false;
			}
		}
	}
	else
	{
		slot.item = FindItem(BUF);
		if(!slot.item)
		{
			WARN(Format("Missing item '%s'.", BUF));
			return false;
		}
	}

	return true;
}

bool LoadItem(StreamReader& f, ItemSlot& slot, bool client)
{
	return LoadItemImpl<true>(f, slot, client);
}

bool LoadItems(StreamReader& f, vector<ItemSlot>& items, bool team_count)
{
	uint count;
	if(!(f >> count))
		return false;

	if(!count)
		return true;

	if(!f.Ensure(count))
		return false;

	if(team_count)
	{
		for(ItemSlot& slot : items)
		{
			if(!LoadItemImpl<true>(f, slot, false))
				return false;
		}
	}
	else
	{
		for(ItemSlot& slot : items)
		{
			if(!LoadItemImpl<false>(f, slot, false))
				return false;
		}
	}

	return true;
}
