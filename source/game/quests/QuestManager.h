#pragma once

//-----------------------------------------------------------------------------
#include "QuestHandle.h"
#include "Quest.h"
#include "GameFile.h"

//-----------------------------------------------------------------------------
struct BuiltinQuest;
class QuestManager;
extern QuestManager QM;

//-----------------------------------------------------------------------------
struct QuestEntry
{
	Quest::State state;
	cstring name;
	const vector<string>* msgs;
};

//-----------------------------------------------------------------------------
struct QuestItem
{
	Item* item;
	Quest2Instance* quest;
	int refs;
};

//-----------------------------------------------------------------------------
class QuestManager
{
	friend struct QuestEntryIterator;

	struct QuestEntryIterator
	{
		uint index;
		static QuestEntry entry;

		inline QuestEntryIterator(uint index) : index(index) {}
		inline bool operator != (const QuestEntryIterator& it) const { return index != it.index; }
		inline void operator ++ () { ++index; }
		inline const QuestEntry& operator * () const { return QM.GetQuestEntry(index); }
	};

	struct QuestEntryContainer
	{
		inline QuestEntryIterator begin() { return QuestEntryIterator(0u); }
		inline QuestEntryIterator end() { return QuestEntryIterator(QM.quests.size()); }
	};

public:
	void Init();
	void Reset();
	void Save(GameWriter& f);
	void Load(GameReader& f);
	void Write(BitStream& stream);
	bool Read(BitStream& stream);

	// quests
	Quest* CreateQuest(QUEST quest_id);
	Quest2Instance* CreateQuest(Quest2* quest);
	QuestHandle CreateQuest(cstring quest_id);
	inline const string& GetForcedQuest() const { return forced_quest_id; }
	bool SetForcedQuest(const string& forced);
	QuestHandle GetRandomQuest(Quest::Type type);
	void PrintListOfQuests(PrintMsgFunc print_func, const string* filter);
	bool CallQuestFunction(Quest2Instance* instance, int index, bool is_if);
	void SetProgress(Quest2Instance* instance, int progress);
	Quest* FindNeedTalkQuest(cstring topic, bool active = true);
	Quest* FindQuest(int location, Quest::Type type);
	Quest* FindQuest(int refid, bool active = true);
	Quest* FindQuestById(QUEST quest_id);
	Quest* FindUnacceptedQuest(int location, Quest::Type type);
	Quest* FindUnacceptedQuest(int refid);
	void AddPlaceholderQuest(Quest* quest);
	void AcceptQuest(Quest* quest, int timeout = 0);
	void RemoveTimeout(Quest* quest, int timeout);
	void RemoveUnacceptedQuest(Quest* quest) { RemoveElement(unaccepted_quests, quest); }
	void UpdateTimeouts(int days);

	// quest items
	void AddQuestItemRequest(const Item** item, int quest_refid, vector<ItemSlot>* items, Unit* unit = nullptr);
	void ApplyQuestItemRequests();
	const Item* FindQuestItem(int refid);

	// client quest items
	inline void AddClientQuestItem(Item* item) { assert(item); client_quest_items.push_back(item); }
	Item* FindClientQuestItem(int refid, cstring id);
	bool ReadClientQuestItems(BitStream& stream);

	// quest entries
	inline uint GetQuestEntryCount() { return quests.size(); }
	inline QuestEntryContainer GetQuestEntries() { return QuestEntryContainer(); }
	const QuestEntry& GetQuestEntry(int index);

	// unique quests
	inline void MarkAllQuestsCompleted() { unique_completed_can_show = true; }
	bool CheckShowAllQuestsCompleted();
	void EndUniqueQuest();

	void ParseQuests();
	Quest2* FindNewQuest(cstring str);

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
		int quest_refid;
		bool is_new;
	};

	Quest* CreateQuestInstance(QUEST quest_id);
	void LoadQuests(GameReader& f, vector<Quest*>& v_quests);
	bool ParseQuest(Tokenizer& t);
	QuestIndex FindQuest(cstring quest_id);

	vector<Quest*> unaccepted_quests;
	vector<Quest*> quests;
	vector<Quest_Dungeon*> quests_timeout;
	vector<Quest*> quests_timeout2;
	vector<QuestItemRequest> quest_item_requests;
	vector<Item*> client_quest_items;
	QuestIndex forced_quest;
	string forced_quest_id;
	vector<Quest2*> new_quests;
	vector<WeightPair<QuestIndex>> tmp_list;
	vector<Quest2Instance*> quest_instances;
	vector<QuestItem> quest_items;
	int quest_counter;
	int unique_count;
	int unique_quests_completed;
	bool unique_completed_shown, unique_completed_can_show;
};
