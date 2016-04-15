// miasto
#include "Pch.h"
#include "Base.h"
#include "City.h"
#include "SaveState.h"

//=================================================================================================
City::~City()
{
	DeleteElements(inside_buildings);
}

//=================================================================================================
void City::Save(HANDLE file, bool local)
{
	OutsideLocation::Save(file, local);

	WriteFile(file, &citizens, sizeof(citizens), &tmp, nullptr);
	WriteFile(file, &citizens_world, sizeof(citizens_world), &tmp, nullptr);

	if(last_visit != -1)
	{
		FileWriter f(file);
		f.WriteVector1(entry_points);
		f << have_exit;
		f.Write<byte>(gates);

		uint ile = buildings.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		WriteFile(file, &buildings[0], sizeof(CityBuilding)*ile, &tmp, nullptr);
		WriteFile(file, &inside_offset, sizeof(inside_offset), &tmp, nullptr);
		ile = inside_buildings.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<InsideBuilding*>::iterator it = inside_buildings.begin(), end = inside_buildings.end(); it != end; ++it)
			(*it)->Save(file, local);

		WriteFile(file, &quest_mayor, sizeof(quest_mayor), &tmp, nullptr);
		WriteFile(file, &quest_mayor_time, sizeof(quest_mayor_time), &tmp, nullptr);
		WriteFile(file, &quest_captain, sizeof(quest_captain), &tmp, nullptr);
		WriteFile(file, &quest_captain_time, sizeof(quest_captain_time), &tmp, nullptr);
		WriteFile(file, &arena_time, sizeof(arena_time), &tmp, nullptr);
		WriteFile(file, &arena_pos, sizeof(arena_pos), &tmp, nullptr);
	}
}

//=================================================================================================
void City::Load(HANDLE file, bool local)
{
	OutsideLocation::Load(file, local);

	ReadFile(file, &citizens, sizeof(citizens), &tmp, nullptr);
	ReadFile(file, &citizens_world, sizeof(citizens_world), &tmp, nullptr);

	if(last_visit != -1)
	{
		int side;
		BOX2D spawn_area;
		BOX2D exit_area;
		float spawn_dir;

		FileReader f(file);
		f.ReadVector1(entry_points);
		f >> have_exit;
		gates = f.Read<byte>();

		uint ile;
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		buildings.resize(ile);
		ReadFile(file, &buildings[0], sizeof(CityBuilding)*ile, &tmp, nullptr);
		ReadFile(file, &inside_offset, sizeof(inside_offset), &tmp, nullptr);
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		inside_buildings.resize(ile);
		int index = 0;
		for(vector<InsideBuilding*>::iterator it = inside_buildings.begin(), end = inside_buildings.end(); it != end; ++it, ++index)
		{
			*it = new InsideBuilding;
			(*it)->Load(file, local);
			(*it)->ctx.building_id = index;
			(*it)->ctx.mine = INT2((*it)->level_shift.x * 256, (*it)->level_shift.y * 256);
			(*it)->ctx.maxe = (*it)->ctx.mine + INT2(256, 256);
		}

		ReadFile(file, &quest_mayor, sizeof(quest_mayor), &tmp, nullptr);
		ReadFile(file, &quest_mayor_time, sizeof(quest_mayor_time), &tmp, nullptr);
		ReadFile(file, &quest_captain, sizeof(quest_captain), &tmp, nullptr);
		ReadFile(file, &quest_captain_time, sizeof(quest_captain_time), &tmp, nullptr);
		ReadFile(file, &arena_time, sizeof(arena_time), &tmp, nullptr);
		ReadFile(file, &arena_pos, sizeof(arena_pos), &tmp, nullptr);
	}
}

//=================================================================================================
void City::BuildRefidTable()
{
	OutsideLocation::BuildRefidTable();

	for(vector<InsideBuilding*>::iterator it2 = inside_buildings.begin(), end2 = inside_buildings.end(); it2 != end2; ++it2)
	{
		for(vector<Unit*>::iterator it = (*it2)->units.begin(), end = (*it2)->units.end(); it != end; ++it)
		{
			(*it)->refid = (int)Unit::refid_table.size();
			Unit::refid_table.push_back(*it);
		}

		for(vector<Useable*>::iterator it = (*it2)->useables.begin(), end = (*it2)->useables.end(); it != end; ++it)
		{
			(*it)->refid = (int)Useable::refid_table.size();
			Useable::refid_table.push_back(*it);
		}
	}
}

//=================================================================================================
Unit* City::FindUnitInsideBuilding(const UnitData* ud, BUILDING building_type) const
{
	assert(ud);

	for(vector<InsideBuilding*>::const_iterator it = inside_buildings.begin(), end = inside_buildings.end(); it != end; ++it)
	{
		if((*it)->type == building_type)
		{
			return (*it)->FindUnit(ud);
		}
	}

	assert(0);
	return nullptr;
}

//=================================================================================================
bool City::FindUnit(Unit* unit, int* level)
{
	assert(unit);

	for(Unit* u : units)
	{
		if(u == unit)
		{
			if(level)
				*level = -1;
			return true;
		}
	}

	for(uint i = 0; i < inside_buildings.size(); ++i)
	{
		for(Unit* u : inside_buildings[i]->units)
		{
			if(u == unit)
			{
				if(level)
					*level = i;
				return true;
			}
		}
	}

	return false;
}

//=================================================================================================
Unit* City::FindUnit(UnitData* data, int& at_level)
{
	assert(data);

	for(Unit* u : units)
	{
		if(u->data == data)
		{
			at_level = -1;
			return u;
		}
	}

	for(uint i = 0; i < inside_buildings.size(); ++i)
	{
		for(Unit* u : inside_buildings[i]->units)
		{
			if(u->data == data)
			{
				at_level = i;
				return u;
			}
		}
	}

	return nullptr;
}
