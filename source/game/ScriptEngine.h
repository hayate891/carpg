#pragma once

#include "Quest2.h"

class ScriptEngine
{
public:
	ScriptEngine();
	inline static ScriptEngine& Get()
	{
		return script_engine;
	}

	void Init();
	void ParseQuests();
	void Cleanup();
	void StartQuest(cstring quest_id);

	Quest2* FindQuest(cstring str)
	{
		for(Quest2* q : quests)
		{
			if(q->id == str)
				return q;
		}
		return nullptr;
	}

	inline asIScriptEngine* GetASEngine() const
	{
		return engine;
	}

private:
	static ScriptEngine script_engine;
	asIScriptEngine* engine;
	asIScriptModule* module;
	asIScriptContext* context;

	void RegisterTypes();
	void RegisterGlobals();
	void RegisterVEC2();
	void RegisterVEC3();
	void RegisterVEC4();
	void RegisterWorld();
	void RegisterItems();
	void RegisterQuestInstance();
	void RegisterJournal();

	// temporary quests data
	// to be moved
	bool ParseQuest(Tokenizer& t);
	vector<Quest2*> quests;
};
