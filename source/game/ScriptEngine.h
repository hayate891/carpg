#pragma once

#include "Quest.h"

class Quest2
{
public:
	Quest2() {}

	string id, code;
	Quest::Type type;
	vector<string> progress;
};

class Quest2Instance
{
public:
	Quest2* quest;
};

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
	void StartQuest(Quest2* quest);

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

	void RegisterTypes();
	void RegisterGlobals();
	void RegisterVEC2();
	void RegisterVEC3();
	void RegisterVEC4();
	void RegisterWorld();

	// temporary quests data
	// to be moved
	bool ParseQuest(Tokenizer& t);
	vector<Quest2*> quests;
};
