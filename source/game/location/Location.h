#pragma once

//-----------------------------------------------------------------------------
#include "SpawnGroup.h"
#include "LevelContext.h"

//-----------------------------------------------------------------------------
// rodzaj lokacji
//-----------------------------------------------------------------------------
// po dodaniu nowej lokacji trzeba w wielu miejscach pozmienia� co�, tu jest ich lista:
// RebuildMinimap
// CreateLocation
// WorldMapGui::LoadData
// EnterLocation
// Location::GenerateName
// DrawMinimap
// LevelAreaContext* Game::ForLevel(int loc, int level)
// -- niekompletna --
enum LOCATION
{
	L_CITY, // miasto otoczone kamiennym murem i wie�yczki, przez �rodek biegnie kamienna droga, 
	L_VILLAGE, // wioska otoczona palisad�, drewniane wie�e, udeptana droga
	L_CAVE, // jaskinia, jakie� zwierz�ta/potwory/ewentualnie bandyci
	L_CAMP, // ob�z bandyt�w/poszukiwaczy przyg�d/wojska (tymczasowa lokacja)
	L_DUNGEON, // podziemia, r�nej g��boko�ci, posiadaj� pomieszczenia o okre�lonym celu (skarbiec, sypialnie itp), zazwyczaj bandyci lub opuszczone
	L_CRYPT, // krypta, ma dwa cele: skarbiec lub wi�zienie dla nieumar�ych, zazwyczaj s� tu nieumarli, czasami przy wej�ciu bandyci lub potwory
	L_FOREST, // las, zazwyczaj pusty, czasem potwory lub bandyci maj� tu ma�y ob�z
	L_MOONWELL, // jak las ale ze wzg�rzem na �rodku i fontann�
	L_ENCOUNTER, // losowe spotkanie na drodze
	L_ACADEMY,
	L_MAX,
	L_NULL
};

//-----------------------------------------------------------------------------
enum LOCATION_TOKEN : byte
{
	LT_NULL,
	LT_OUTSIDE,
	LT_CITY,
	LT_VILLAGE,
	LT_CAVE,
	LT_SINGLE_DUNGEON,
	LT_MULTI_DUNGEON,
	LT_CAMP
};

//-----------------------------------------------------------------------------
// stan lokacji
enum LOCATION_STATE
{
	LS_HIDDEN,
	LS_UNKNOWN,
	LS_KNOWN,
	LS_VISITED,
	LS_ENTERED,
	LS_CLEARED
};

//-----------------------------------------------------------------------------
struct Quest_Dungeon;
struct Location;

//-----------------------------------------------------------------------------
struct Portal
{
	VEC3 pos;
	float rot;
	int at_level;
	int target; // portal wyj�ciowy
	int target_loc; // docelowa lokacja
	Portal* next_portal;

	static const int MIN_SIZE = 17;

	void Save(HANDLE file);
	void Load(Location* loc, HANDLE file);

	inline VEC3 GetSpawnPos() const
	{
		return pos + VEC3(sin(rot)*2, 0, cos(rot)*2);
	}
};

//-----------------------------------------------------------------------------
// przypisanie takiego quest do lokacji spowoduje �e nie zostanie zaj�ta przez inny quest
#define ACTIVE_QUEST_HOLDER 0xFFFFFFFE

//-----------------------------------------------------------------------------
// struktura opisuj�ca lokacje na mapie �wiata
struct Location : public ILevel
{
	LOCATION type; // typ lokacji
	LOCATION_STATE state; // stan lokacji
	VEC2 pos; // pozycja na mapie �wiata
	string name; // nazwa lokacji
	Quest_Dungeon* active_quest; // aktywne zadanie zwi�zane z t� lokacj�
	int last_visit; // czas ostatniej wizyty
	int st; // poziom trudno�ci
	uint seed;
	SPAWN_GROUP spawn; // rodzaj wrog�w w tej lokacji
	Portal* portal;
	bool reset; // resetowanie lokacji po wej�ciu
	bool outside; // czy poziom jest otwarty
	bool dont_clean;

	explicit Location(bool outside) : active_quest(nullptr), last_visit(-1), reset(false), state(LS_UNKNOWN), outside(outside), st(0), spawn(SG_BRAK), portal(nullptr), dont_clean(false)
	{

	}

	virtual ~Location()
	{
		Portal* p = portal;
		while(p)
		{
			Portal* next_portal = p->next_portal;
			delete p;
			p = next_portal;
		}
	}

	void GenerateName();
	virtual void Save(HANDLE file, bool local);
	virtual void Load(HANDLE file, bool local);

	virtual bool CheckUpdate(int& days_passed, int worldtime)
	{
		bool need_reset = reset;
		reset = false;
		if(last_visit == -1)
			days_passed = -1;
		else
			days_passed = worldtime - last_visit;
		last_visit = worldtime;
		if(dont_clean)
			days_passed = 0;
		return need_reset;
	}

	virtual int GetRandomLevel() const
	{
		return -1;
	}
	virtual int GetLastLevel() const
	{
		return 0;
	}

	virtual void BuildRefidTable() = 0;
	virtual bool FindUnit(Unit* unit, int* level = nullptr) = 0;
	virtual Unit* FindUnit(UnitData* data, int& at_level) = 0;

	virtual LOCATION_TOKEN GetToken() const
	{
		return LT_NULL;
	}
	Portal* GetPortal(int index);
	Portal* TryGetPortal(int index) const;
	void WritePortals(BitStream& stream) const;
	bool ReadPortals(BitStream& stream, int at_level);

	inline bool IsSingleLevel() const
	{
		return type != L_DUNGEON && type != L_CRYPT;
	}
	inline bool IsCityVillage() const
	{
		return type == L_CITY || type == L_VILLAGE;
	}
};

//-----------------------------------------------------------------------------
struct LocationEventHandler
{
	enum Event
	{
		CLEARED
	};

	virtual void HandleLocationEvent(Event event) = 0;
	virtual int GetLocationEventHandlerQuestRefid() = 0;
};

//-----------------------------------------------------------------------------
void SetLocationNames();
