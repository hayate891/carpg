// r�ne statystyki postaci
#pragma once

//-----------------------------------------------------------------------------
#include "Attribute.h"
#include "Skill.h"

//-----------------------------------------------------------------------------
// Typ broni
enum WeaponType
{
	W_NONE,
	W_ONE_HANDED,
	W_BOW
};

//-----------------------------------------------------------------------------
enum class StatState
{
	NORMAL,
	POSITIVE,
	POSITIVE_MIXED,
	MIXED,
	NEGATIVE_MIXED,
	NEGATIVE,
	MAX
};

//-----------------------------------------------------------------------------
inline char StatStateToColor(StatState s)
{
	switch(s)
	{
	default:
	case StatState::NORMAL:
		return 'k';
	case StatState::POSITIVE:
		return 'g';
	case StatState::POSITIVE_MIXED:
		return '0';
	case StatState::MIXED:
		return 'y';
	case StatState::NEGATIVE_MIXED:
		return '1';
	case StatState::NEGATIVE:
		return 'r';
	}
}

//-----------------------------------------------------------------------------
struct StatInfo
{
	int value, unmod, base;
	StatState state;
};

//-----------------------------------------------------------------------------
struct UnitStats
{
	int attrib[(int)Attribute::MAX];
	int skill[(int)Skill::MAX];

	inline void WriteAttributes(BitStream& stream) const
	{
		stream.Write((cstring)attrib, sizeof(attrib));
	}

	inline void WriteSkills(BitStream& stream) const
	{
		stream.Write((cstring)skill, sizeof(skill));
	}

	inline void Write(BitStream& stream) const
	{
		WriteAttributes(stream);
		WriteSkills(stream);
	}

	inline bool ReadAttributes(BitStream& stream)
	{
		return stream.Read((char*)attrib, sizeof(attrib));
	}

	inline bool ReadSkills(BitStream& stream)
	{
		return stream.Read((char*)skill, sizeof(skill));
	}

	inline bool Read(BitStream& stream)
	{
		return ReadAttributes(stream) && ReadSkills(stream);
	}

	inline void Save(FileWriter& f) const
	{
		f << attrib;
		f << skill;
	}

	inline void Load(FileReader& f)
	{
		f >> attrib;
		f >> skill;
	}
};
