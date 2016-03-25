#include "Pch.h"
#include "Base.h"
#include "Quest2.h"
#include "Game.h"

int Quest2Instance::S_GetProgress() const
{
	return progress;
}

Location* Quest2Instance::S_GetStartLoc() const
{
	return Game::Get().locations[start_loc];
}

Location* Quest2Instance::S_GetTargetLoc() const
{
	if(target_loc == -1)
		return nullptr;
	return Game::Get().locations[target_loc];
}

void Quest2Instance::S_SetTargetLoc(Location* loc)
{
	if(loc == nullptr)
		target_loc = -1;
	else
		target_loc = Game::Get().GetLocationIndex(*loc);
}

void Quest2Instance::Save(StreamWriter& s)
{
	/*s << quest->id;
	s << refid;
	s << progress;
	s << start_loc;
	s << target_loc;*/

	/*uint obj->GetPropertyCount()

	asIScriptObject* obj; // pointer to script object
	int refid; // unique quest identifier
	int progress;
	int start_loc, target_loc; // index of location or -1
	*/
}

void Quest2Instance::Load(StreamReader& s)
{

}
