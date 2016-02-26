#include "Pch.h"
#include "Base.h"
#include "ScriptEngine.h"

ScriptEngine ScriptEngine::script_engine;
extern string g_system_dir;

void ScriptEngine::Init()
{
	asIScriptEngine* engine = asCreateScriptEngine();
}

enum KeywordGroup
{
	G_TOP,
	G_KEYWORD
};

enum Keyword
{
	K_PROGRESS,
	K_CODE,
	K_DIALOG
};

void ScriptEngine::ParseQuests()
{
	Tokenizer t;
	t.AddKeyword("quest", 0, G_TOP);

	t.AddKeywords(G_KEYWORD, {
		{ "progress", K_PROGRESS },
		{ "code", K_CODE },
		{ "dialog", K_DIALOG }
	});

	cstring filename = Format("%s/quests.txt", g_system_dir.c_str());
	if(!t.FromFile(filename))
		throw Format("Failed to open file '%s'.", filename);

	try
	{
		t.Next();
		
		while(true)
		{
			bool skip = false;
			if(t.IsKeyword(0, G_TOP))
			{
				if(!ParseQuest(t))
					skip = true;
			}
			else
			{
				skip = true;
				int id_and_group = 0;
				ERROR(Format("Error loading quests: %s", t.StartUnexpected().Add(Tokenizer::T_KEYWORD, &id_and_group, &id_and_group).Get()));
			}

			if(skip)
				t.SkipToKeywordGroup(G_TOP);
		}
	}
	catch(const Tokenizer::Exception& e)
	{
		ERROR(Format("Failed to load quests: %s", e.ToString()));
	}
}

bool ScriptEngine::ParseQuest(Tokenizer& t)
{
	Quest2* quest = new Quest2;

	try
	{
		t.AssertKeyword(0, G_TOP);
		t.Next();

		// name
		quest->id = t.MustGetItemKeyword();
		Quest2* other_quest = FindQuest(quest->id.c_str());
		if(other_quest)
			t.Throw("Quest with that id already exists.");
		t.Next();

		// {
		t.AssertSymbol('{');
		t.Next();

		// properties
		while(!t.IsSymbol('}'))
		{
			Keyword key = (Keyword)t.MustGetKeywordId(G_KEYWORD);
			t.Next();

			switch(key)
			{
			case K_PROGRESS:
			case K_CODE:
			case K_DIALOG:
				break;
			}
		}

		quests.push_back(quest);
		return true;
	}
	catch(const Tokenizer::Exception& e)
	{
		cstring str;
		if(!quest->id.empty())
			str = Format("Failed to parse quest '%s': %s", quest->id.c_str(), e.ToString());
		else
			str = Format("Failed to parse quest: %s", e.ToString());
		ERROR(str);
		delete quest;
		return false;
	}
}
