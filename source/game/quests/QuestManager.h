#pragma once

//-----------------------------------------------------------------------------
#include "QuestHandle.h"
#include "Quest.h"

//-----------------------------------------------------------------------------
struct BuiltinQuest;

//-----------------------------------------------------------------------------
struct QuestItem
{

};

//-----------------------------------------------------------------------------
class QuestManager
{
public:
	static QuestManager& Get() { return quest_manager; }

	Quest* CreateQuest(QUEST quest_id);
	Quest2Instance* CreateQuest(Quest2* quest);
	QuestHandle CreateQuest(cstring quest_id);
	inline const string& GetForcedQuest() const { return forced_quest_id; }
	bool SetForcedQuest(const string& forced);
	QuestHandle GetRandomQuest(Quest::Type type);
	void PrintListOfQuests(PrintMsgFunc print_func, const string* filter);
	void ParseQuests();
	Quest2* FindNewQuest(cstring str);
	void Init(); // called on ???
	void Reset(); // called on NewGame
	void Clear(); // called on ClearGame
	void Save(StreamWriter& f);
	bool Load(StreamReader& f);
	inline bool CanShowAllCompleted() const { return all_quests_completed && !unique_shown; }
	inline void ShownAllCompleted() { unique_shown = true; }
	inline void MarkAllCompleted() { all_quests_completed = true; unique_shown = false; }
	void EndUniqueQuest();
	bool CallQuestFunction(Quest2Instance* instance, int index, bool is_if);
	void SetProgress(Quest2Instance* instance, int progress);
	void AddQuestItemRequest(const Item** item, cstring name, int quest_refid);
	Item* FindClientQuestItem(cstring id, int refid);
	inline void AddClientQuestItem(Item* item)
	{
		assert(item);
		client_quest_items.push_back(item);
	}
	bool ReadQuestItems(StreamReader& f);
	void ApplyQuestItemRequests();

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

	struct QuestItemRequest
	{
		const Item** item;
		string name;
		int quest_refid;
	};

	QuestManager();
	bool ParseQuest(Tokenizer& t);
	Quest* CreateQuestInstance(QUEST quest_id);
	QuestIndex FindQuest(cstring quest_id);
	void SaveQuest(StreamWriter& f, Quest2Instance& quest);
	bool LoadQuest(StreamReader& f);

	static QuestManager quest_manager;
	int counter; // quest counter for unique quest id
	int unique_count; // number of unique quests
	int unique_completed; // number of completed unique quests
	bool unique_shown;
	bool all_quests_completed;
	QuestIndex forced_quest;
	string forced_quest_id;
	vector<Quest2*> new_quests;
	vector<WeightPair<QuestIndex>> tmp_list;
	vector<Quest2Instance*> quest_instances;
	vector<QuestItemRequest*> quest_item_requests;
	vector<Item*> client_quest_items;
};
