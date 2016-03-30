#pragma once

//-----------------------------------------------------------------------------
#include "Quest.h"

//-----------------------------------------------------------------------------
class Quest2;

//-----------------------------------------------------------------------------
class QuestManager
{
public:
	Quest* CreateQuest(QUEST quest_id);
	Quest* GetMayorQuest(int force = -1);
	Quest* GetCaptainQuest(int force = -1);
	Quest* GetAdventurerQuest(int force = -1);
	inline void SetForcedQuest(const string& forced) { forced_quest = forced; }

private:
	string forced_quest;
	vector<Quest2*> new_quests;
	vector<Quest2*> new_mayor_quests;
};
