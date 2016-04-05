#pragma once

//-----------------------------------------------------------------------------
#include "QuestHandle.h"
#include "Quest.h"

//-----------------------------------------------------------------------------
class GameReader;
class GameWriter;
struct BuiltinQuest;

//-----------------------------------------------------------------------------
class QuestManager
{
public:
	QuestManager();

	Quest* CreateQuest(QUEST quest_id);
	Quest2Instance* CreateQuest(Quest2* quest);
	QuestHandle CreateQuest(cstring quest_id);
	inline const string& GetForcedQuest() const { return forced_quest_id; }
	bool SetForcedQuest(const string& forced);
	QuestHandle GetRandomQuest(Quest::Type type);
	void PrintListOfQuests(PrintMsgFunc print_func, const string* filter);
	void ParseQuests();
	Quest2* FindNewQuest(cstring str);
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
		union
		{
			const BuiltinQuest* quest;
			Quest2* quest2;
		};
		bool is_quest2;

		inline QuestIndex() : quest(nullptr), is_quest2(false) {}
		inline QuestIndex(const BuiltinQuest* quest) : quest(quest), is_quest2(false) {}
		inline QuestIndex(Quest2* quest2) : quest2(quest2), is_quest2(true) {}
	};

	bool ParseQuest(Tokenizer& t);
	Quest* CreateQuestInstance(QUEST quest_id);
	QuestIndex FindQuest(cstring quest_id);

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
