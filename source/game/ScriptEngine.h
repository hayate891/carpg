#pragma once

#include "Quest2.h"

typedef fastdelegate::FastDelegate2<StreamWriter&, void*> WriteFunc;
typedef fastdelegate::FastDelegate2<StreamReader&, void*, bool> ReadFunc;

struct ScriptEngineType
{
	int id;
	cstring name;
	WriteFunc write;
	ReadFunc read;

	ScriptEngineType() {}
	ScriptEngineType(cstring name, WriteFunc& write, ReadFunc& read) : name(name), write(write), read(read), id(-1) {}
	ScriptEngineType(asETypeIdFlags id, WriteFunc& write, ReadFunc& read) : write(write), read(read), id((int)id) {}
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

	void AddType(ScriptEngineType& type);

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
	void AddStandardTypes();

	// temporary quests data
	// to be moved
	bool ParseQuest(Tokenizer& t);
	vector<Quest2*> quests;

	std::unordered_map<int, ScriptEngineType> script_types;
};
