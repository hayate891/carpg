#pragma once

//-----------------------------------------------------------------------------
#include "QuestHandle.h"
#include "Quest.h"

//-----------------------------------------------------------------------------
class GameReader;
class GameWriter;

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
	inline bool CanShowAllCompleted() const { return all_quests_completed && !unique_shown; }
	inline void ShownAllCompleted() { unique_shown = true; }
	inline void MarkAllCompleted() { all_quests_completed = true; unique_shown = false; }
	void EndUniqueQuest();
	bool CallQuestFunction(Quest2Instance* instance, int index, bool is_if);
	void SetProgress(Quest2Instance* instance, int progress);

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

	int counter; // quest counter for unique quest id
	int unique_count; // number of unique quests
	int unique_completed; // number of completed unique quests
	bool unique_shown;
	bool all_quests_completed;
	QuestIndex forced_quest;
	string forced_quest_id;
	vector<Quest2*> new_quests;
	vector<WeightPair<QuestIndex>> tmp_list;
};
