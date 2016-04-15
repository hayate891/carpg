#include "Pch.h"
#include "Base.h"
#include "Blood.h"
#include "SaveState.h"
#include "BitStreamFunc.h"

//=================================================================================================
void Blood::Save(FileWriter& f) const
{
	f.Write<byte>(type);
	f << pos;
	f << normal;
	f << size;
	f << rot;
}

//=================================================================================================
void Blood::Load(FileReader& f)
{
	type = (BLOOD)f.Read<byte>();
	f >> pos;
	f >> normal;
	f >> size;
	f >> rot;
}

//=================================================================================================
void Blood::Write(BitStream& stream) const
{
	stream.WriteCasted<byte>(type);
	stream.Write(pos);
	stream.Write(normal);
	stream.Write(rot);
}

//=================================================================================================
bool Blood::Read(BitStream& stream)
{
	size = 1.f;
	return stream.ReadCasted<byte>(type)
		&& stream.Read(pos)
		&& stream.Read(normal)
		&& stream.Read(rot);
}
