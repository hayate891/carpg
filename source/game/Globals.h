#pragma once

struct ScriptException
{
	ScriptException(cstring err) : err(err) {}

	cstring err;
};

class Quest2Instance;
struct DialogContext;

// script globals
namespace globals
{
	extern Quest2Instance* current_quest;
	extern DialogContext* current_dialog;
	extern Unit* user;
	extern Unit* talker;
	extern PlayerController* player;

	inline Quest2Instance* GetCurrentQuest()
	{
		if(!current_quest)
			throw ScriptException("Not called from quest.");
		return current_quest;
	}

	inline DialogContext* GetCurrentDialog()
	{
		if(!current_dialog)
			throw ScriptException("Not called from dialog.");
		return current_dialog;
	}
};
