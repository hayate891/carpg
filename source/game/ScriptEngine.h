#pragma once

class Quest2
{
public:
	Quest2() : code(nullptr) {}

	string id;
	string* code;
};

class ScriptEngine
{
public:
	ScriptEngine();
	inline ScriptEngine& Get()
	{
		return script_engine;
	}

	void Init();
	void Cleanup();

	Quest2* FindQuest(cstring str)
	{
		for(Quest2* q : quests)
		{
			if(q->id == str)
				return q;
		}
		return nullptr;
	}

private:
	static ScriptEngine script_engine;
	asIScriptEngine* engine;

	void RegisterTypes();

	// temporary quests data
	// to be moved
	void ParseQuests();
	bool ParseQuest(Tokenizer& t);
	vector<Quest2*> quests;
};
