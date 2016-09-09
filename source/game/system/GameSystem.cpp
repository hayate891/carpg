#include "Pch.h"
#include "Common.h"
#include "GameSystem.h"
#include "GameSystemManager.h"

#include "BuildingGroup2.h"

void RegisterAllTypes()
{
	GameSystemManager* gs_mgr = new GameSystemManager;

	BuildingGroup2::Register(gs_mgr);
}
