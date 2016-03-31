#pragma once

//-----------------------------------------------------------------------------
#include "Quest.h"

//-----------------------------------------------------------------------------
class Quest2;
class Quest2Instance;
class GameReader;
class GameWriter;

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

//-----------------------------------------------------------------------------
class QuestManager
{
public:
	QuestManager();

	Quest* CreateQuest(QUEST quest_id);
	Quest2Instance* CreateQuest(Quest2* quest);
	inline const string& GetForcedQuest() const { return forced_quest_id; }
	bool SetForcedQuest(const string& forced);
	QuestHandle GetRandomQuest(Quest::Type type);
	void PrintListOfQuests(PrintMsgFunc print_func, const string* filter);
	void ParseQuests();
	Quest2* FindQuest(cstring str);
	void Init();
	void Reset();
	void Save(GameWriter& f);
	void Load(GameReader& f);

private:
	struct QuestIndex
	{
		short index;
		bool is_new;

		inline QuestIndex() {}
		inline QuestIndex(short index, bool is_new) : index(index), is_new(is_new) {}
	};

	bool ParseQuest(Tokenizer& t);
	Quest* CreateQuestInstance(QUEST quest_id);

	int counter;
	QuestIndex forced_quest;
	string forced_quest_id;
	vector<Quest2*> new_quests;
	vector<WeightPair<QuestIndex>> tmp_list;
};
