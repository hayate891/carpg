#pragma once

//-----------------------------------------------------------------------------
struct Quest;
class Quest2;
class Quest2Instance;

//-----------------------------------------------------------------------------
struct QuestHandle
{
	union
	{
		Quest* quest;
		Quest2Instance* quest2;
	};
	bool is_quest2;

	inline QuestHandle() : quest(nullptr), is_quest2(false) {}
	inline QuestHandle(Quest* quest) : quest(quest), is_quest2(false) {}
	inline QuestHandle(Quest2Instance* quest2) : quest2(quest2), is_quest2(true) {}
};
