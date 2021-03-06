#pragma once

//-----------------------------------------------------------------------------
struct Building;
struct UnitData;

//-----------------------------------------------------------------------------
// Old building enum - required for pre 0.5 compability
enum class OLD_BUILDING
{
	B_MERCHANT,
	B_BLACKSMITH,
	B_ALCHEMIST,
	B_TRAINING_GROUNDS,
	B_INN,
	B_CITY_HALL,
	B_VILLAGE_HALL,
	B_BARRACKS,
	B_HOUSE,
	B_HOUSE2,
	B_HOUSE3,
	B_ARENA,
	B_FOOD_SELLER,
	B_COTTAGE,
	B_COTTAGE2,
	B_COTTAGE3,
	B_VILLAGE_INN,
	B_NONE,
	B_VILLAGE_HALL_OLD,
	B_MAX
};

//-----------------------------------------------------------------------------
// Building group
struct BuildingGroup
{
	string id;
	vector<Building*> buildings;
};

//-----------------------------------------------------------------------------
// Base building
struct Building
{
	enum TileScheme
	{
		SCHEME_GRASS,
		SCHEME_BUILDING,
		SCHEME_SAND,
		SCHEME_PATH,
		SCHEME_UNIT,
		SCHEME_BUILDING_PART
	};

	enum Flags
	{
		FAVOR_CENTER = 1 << 0,
		FAVOR_ROAD = 1 << 1,
		HAVE_NAME = 1 << 2,
		LIST = 1 << 3
	};

	string id, mesh_id, inside_mesh_id, name;
	INT2 size, shift[4];
	vector<TileScheme> scheme;
	int flags;
	BuildingGroup* group;
	Animesh* mesh, *inside_mesh;
	UnitData* unit;
	bool name_set;

	Building() : size(0, 0), shift(), flags(0), mesh(nullptr), inside_mesh(nullptr), group(nullptr), name_set(false), unit(nullptr) {}
};

//-----------------------------------------------------------------------------
// Building used when constructing map
struct ToBuild
{
	Building* type;
	INT2 pt, unit_pt;
	int rot;
	bool required;

	ToBuild(Building* type, bool required = true) : type(type), required(required) {}
};

//-----------------------------------------------------------------------------
// Building script
struct BuildingScript
{
	enum Code
	{
		BS_BUILDING, // building identifier [Entry = BS_BUILDING, Building*]
		BS_GROUP, // buildings group identifier [Entry = BS_GROUP, BuildingGroup*]
		BS_ADD_BUILDING, // add building [Entry]
		BS_RANDOM, // pick random item from list [uint-count, Entry...]
		BS_SHUFFLE_START, // shuffle start
		BS_SHUFFLE_END, // shuffle end
		BS_REQUIRED_OFF, // end of required buildings
		BS_PUSH_INT, // push int to stack [int]
		BS_PUSH_VAR, // push var to stack [uint-var index]
		BS_SET_VAR, // set var value from stack [uint-var index]
		BS_INC, // increase var by 1 [uint-var index]
		BS_DEC, // decrease var by 1 [uint-var index]
		BS_IF, // if block [Op (BS_EQUAL, BS_NOT_EQUAL, BS_GREATER, BS_GREATER_EQUAL, BS_LESS, BS_LESS_EQUAL)], takes 2 values from stack
		BS_IF_RAND, // if rand block, takes value from stack
		BS_ELSE, // else
		BS_ENDIF, // end of if
		BS_EQUAL, // equal op
		BS_NOT_EQUAL, // not equal op
		BS_GREATER, // greater op
		BS_GREATER_EQUAL, // greater equal op
		BS_LESS, // less op
		BS_LESS_EQUAL, // less equal op
		BS_CALL, // call function (currentlty only random)
		BS_ADD, // add two values from stack and push result
		BS_SUB, // subtract two values from stack and push result
		BS_MUL, // multiply two values from stack and push result
		BS_DIV, // divide two values from stack and push result
		BS_NEG, // negate value on stack
	};

	enum VarIndex
	{
		V_COUNT,
		V_COUNTER,
		V_CITIZENS,
		V_CITIZENS_WORLD
	};

	struct Variant
	{
		uint index, vars;
		vector<int> code;
	};

	static const uint MAX_VARS = 10;

	string id;
	vector<Variant*> variants;
	Variant* variant;
	int vars[MAX_VARS];
	uint required_offset;

	~BuildingScript()
	{
		DeleteElements(variants);
	}

	bool HaveBuilding(const string& group_id) const;
	bool HaveBuilding(BuildingGroup* group, Variant* variant) const;

private:
	bool IsEntryGroup(const int*& code, BuildingGroup* group) const;
};

//-----------------------------------------------------------------------------
// Hardcoded building groups
extern BuildingGroup* BG_INN;
extern BuildingGroup* BG_HALL;
extern BuildingGroup* BG_TRAINING_GROUNDS;
extern BuildingGroup* BG_ARENA;
extern BuildingGroup* BG_FOOD_SELLER;
extern BuildingGroup* BG_ALCHEMIST;
extern BuildingGroup* BG_BLACKSMITH;
extern BuildingGroup* BG_MERCHANT;
