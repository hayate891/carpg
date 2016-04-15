// przedmiot na ziemi
#pragma once

//-----------------------------------------------------------------------------
#include "ItemSlot.h"

//-----------------------------------------------------------------------------
struct GroundItem
{
	VEC3 pos;
	float rot;
	ItemSlot slot;
	int netid;

	static const int MIN_SIZE = 23;

	void Save(HANDLE file);
	void Load(HANDLE file);
};
