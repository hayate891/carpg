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
	void Cleanup();

	inline asIScriptEngine* GetEngine() const { return engine; }
	inline asIScriptModule* GetModule() const { return module; }
	inline asIScriptContext* GetContext() const { return context; }

	void AddType(ScriptEngineType& type);
	inline ScriptEngineType* FindType(int type)
	{
		auto it = script_types.find(type);
		if(it == script_types.end())
			return nullptr;
		return &it->second;
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
	void AddStandardTypes();

	void CallCurrentQuestFunction(cstring decl);
	
	std::unordered_map<int, ScriptEngineType> script_types;
};
