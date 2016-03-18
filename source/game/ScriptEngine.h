#pragma once

#include "Quest.h"

class Quest2
{
public:
	Quest2() : code(nullptr) {}

	string id;
	Quest::Type type;
	string* code;
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
	void Cleanup();

	/*Quest2* FindQuest(cstring str)
	{
		for(Quest2* q : quests)
		{
			if(q->id == str)
				return q;
		}
		return nullptr;
	}*/

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

	// temporary quests data
	// to be moved
	void ParseQuests();
	bool ParseQuest(Tokenizer& t);
	vector<Quest2*> quests;
};
