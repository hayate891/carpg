// zewn�trzna lokacja
#include "Pch.h"
#include "Base.h"
#include "OutsideLocation.h"
#include "SaveState.h"

namespace OLD
{
	enum TERRAIN_TILE : byte
	{
		TT_GRASS,
		TT_GRASS2,
		TT_GRASS3,
		TT_SAND,
		TT_ROAD,
		TT_BUILD_GRASS,
		TT_BUILD_SAND,
		TT_BUILD_ROAD
	};
}

//=================================================================================================
OutsideLocation::~OutsideLocation()
{
	delete[] tiles;
	delete[] h;
	DeleteElements(units);
	DeleteElements(chests);
	DeleteElements(useables);
	DeleteElements(items);
}

//=================================================================================================
void OutsideLocation::ApplyContext(LevelContext& ctx)
{
	ctx.units = &units;
	ctx.objects = &objects;
	ctx.chests = &chests;
	ctx.traps = nullptr;
	ctx.doors = nullptr;
	ctx.items = &items;
	ctx.useables = &useables;
	ctx.bloods = &bloods;
	ctx.lights = nullptr;
	ctx.have_terrain = true;
	ctx.require_tmp_ctx = true;
	ctx.type = LevelContext::Outside;
	ctx.building_id = -1;
	ctx.mine = INT2(0,0);
	ctx.maxe = INT2(size, size);
	ctx.tmp_ctx = nullptr;
	ctx.masks = nullptr;
}

//=================================================================================================
void OutsideLocation::Save(HANDLE file, bool local)
{
	Location::Save(file, local);

	if(last_visit != -1)
	{
		// jednostki
		uint ile = units.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<Unit*>::iterator it = units.begin(), end = units.end(); it != end; ++it)
			(*it)->Save(file, local);

		// obiekty
		ile = objects.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<Object>::iterator it = objects.begin(), end = objects.end(); it != end; ++it)
			it->Save(file);

		// skrzynie
		ile = chests.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<Chest*>::iterator it = chests.begin(), end = chests.end(); it != end; ++it)
			(*it)->Save(file, local);

		// przedmioty
		ile = items.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<GroundItem*>::iterator it = items.begin(), end = items.end(); it != end; ++it)
			(*it)->Save(file);

		// u�ywalne
		ile = useables.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<Useable*>::iterator it = useables.begin(), end = useables.end(); it != end; ++it)
			(*it)->Save(file, local);

		// krew
		FileWriter f(file);
		ile = bloods.size();
		WriteFile(file, &ile, sizeof(ile), &tmp, nullptr);
		for(vector<Blood>::iterator it = bloods.begin(), end = bloods.end(); it != end; ++it)
			it->Save(f);

		// teren
		WriteFile(file, tiles, sizeof(TerrainTile)*size*size, &tmp, nullptr);
		int size2 = size+1;
		size2 *= size2;
		WriteFile(file, h, sizeof(float)*size2, &tmp, nullptr);
	}
}

//=================================================================================================
void OutsideLocation::Load(HANDLE file, bool local)
{
	Location::Load(file, local);

	if(last_visit != -1)
	{
		// jednostki
		uint ile;
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		units.resize(ile);
		for(vector<Unit*>::iterator it = units.begin(), end = units.end(); it != end; ++it)
		{
			*it = new Unit;
			Unit::AddRefid(*it);
			(*it)->Load(file, local);
		}

		// obiekty
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		objects.resize(ile);
		for(vector<Object>::iterator it = objects.begin(), end = objects.end(); it != end; ++it)
			it->Load(file);

		// skrzynie
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		chests.resize(ile);
		for(vector<Chest*>::iterator it = chests.begin(), end = chests.end(); it != end; ++it)
		{
			*it = new Chest;
			(*it)->Load(file, local);
		}

		// przedmioty
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		items.resize(ile);
		for(vector<GroundItem*>::iterator it = items.begin(), end = items.end(); it != end; ++it)
		{
			*it = new GroundItem;
			(*it)->Load(file);
		}

		// u�ywalne
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		useables.resize(ile);
		for(vector<Useable*>::iterator it = useables.begin(), end = useables.end(); it != end; ++it)
		{
			*it = new Useable;
			Useable::AddRefid(*it);
			(*it)->Load(file, local);
		}

		// krew
		FileReader f(file);
		ReadFile(file, &ile, sizeof(ile), &tmp, nullptr);
		bloods.resize(ile);
		for(vector<Blood>::iterator it = bloods.begin(), end = bloods.end(); it != end; ++it)
			it->Load(f);

		// teren
		int size2 = size+1;
		size2 *= size2;
		h = new float[size2];
		tiles = new TerrainTile[size*size];
		ReadFile(file, tiles, sizeof(TerrainTile)*size*size, &tmp, nullptr);
		ReadFile(file, h, sizeof(float)*size2, &tmp, nullptr);
	}
}

//=================================================================================================
void OutsideLocation::BuildRefidTable()
{
	for(vector<Unit*>::iterator it = units.begin(), end = units.end(); it != end; ++it)
	{
		(*it)->refid = (int)Unit::refid_table.size();
		Unit::refid_table.push_back(*it);
	}

	for(vector<Useable*>::iterator it = useables.begin(), end = useables.end(); it != end; ++it)
	{
		(*it)->refid = (int)Useable::refid_table.size();
		Useable::refid_table.push_back(*it);
	}
}

//=================================================================================================
bool OutsideLocation::FindUnit(Unit* unit, int* level)
{
	assert(unit);

	for(Unit* u : units)
	{
		if(unit == u)
		{
			if(level)
				*level = -1;
			return true;
		}
	}

	return false;
}

//=================================================================================================
Unit* OutsideLocation::FindUnit(UnitData* data, int& at_level)
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

	return nullptr;
}
