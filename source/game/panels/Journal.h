#pragma once

//-----------------------------------------------------------------------------
#include "GamePanel.h"

//-----------------------------------------------------------------------------
struct Game;

//-----------------------------------------------------------------------------
struct QuestEntry
{
	enum State
	{
		Normal,
		Failed,
		Complete
	};

	int id;
	string name;
	vector<string> msgs;
	State state;
};

//-----------------------------------------------------------------------------
class Journal : public GamePanel
{
public:
	struct Text
	{
		cstring text;
		int x, y, color;
	};

	enum Mode
	{
		Quests,
		Rumors,
		Notes,
		Invalid
	};

	Journal();
	void Draw(ControlDrawData* cdd = nullptr);
	void Update(float dt);
	void Event(GuiEvent e);

	void Reset();
	void Show();
	void Hide();
	void Build();
	void AddEntry(cstring text, int color, bool singleline);
	void OnAddNote(int id);
	void NeedUpdate(Mode mode, int quest_index=0);
	QuestEntry* FindQuestEntry(int id);
	void AddQuestEntry(QuestEntry* entry);
	void UpdateQuestEntry(QuestEntry* entry);
	inline bool CheckUpdated()
	{
		bool tmp = updated;
		updated = false;
		return tmp;
	}

	Mode mode;
	bool details;
	int page, prev_page, open_quest, x, y, size_x, size_y, rect_w, rect_lines;
	RECT rect, rect2;
	vector<Text> texts;
	string input;
	cstring txAdd, txNoteText, txNoQuests, txNoRumors, txNoNotes, txAddNote, txAddTime;

	static TEX tBook, tPage[3], tArrowL, tArrowR;

private:
	Game& game;
	int font_height;
	vector<QuestEntry*> quest_entries;
	bool updated;
};
