// przedmiot na ziemi
#include "Pch.h"
#include "Base.h"
#include "Game.h"

//=================================================================================================
void GroundItem::Save(HANDLE file)
{
	WriteFile(file, &pos, sizeof(pos), &tmp, nullptr);
	WriteFile(file, &rot, sizeof(rot), &tmp, nullptr);
	SaveItem(StreamWriter(file), slot);
	WriteFile(file, &netid, sizeof(netid), &tmp, nullptr);
}

//=================================================================================================
void GroundItem::Load(HANDLE file)
{
	ReadFile(file, &pos, sizeof(pos), &tmp, nullptr);
	ReadFile(file, &rot, sizeof(rot), &tmp, nullptr);
	LoadItem(StreamReader(file), slot);
	ReadFile(file, &netid, sizeof(netid), &tmp, nullptr);
}
