#include "Pch.h"
#include "Base.h"
#include "ScriptEngine.h"
#include "Unit.h"
#include "PlayerController.h"

ScriptEngine ScriptEngine::script_engine;
extern string g_system_dir;

// script globals
namespace globals
{
	Unit* user;
	PlayerController* player;
}

ScriptEngine::ScriptEngine() : engine(nullptr)
{

}

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	Logger::LOG_LEVEL level;
	cstring s;
	switch(msg->type)
	{
	default:
	case asMSGTYPE_ERROR:
		level = Logger::L_ERROR;
		s = "ERROR";
		break;
	case asMSGTYPE_WARNING:
		level = Logger::L_WARN;
		s = "WARN";
		break;
	case asMSGTYPE_INFORMATION:
		level = Logger::L_INFO;
		s = "INFO";
		break;
	}
	//logger->Log(Format("%s(%d,%d): %s", msg->section, msg->row, msg->col, msg->message), level);
	printf("%s: %s(%d,%d): %s\n", s, msg->section, msg->row, msg->col, msg->message);
}

void ScriptEngine::Init()
{
	engine = asCreateScriptEngine();

	engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL);

	RegisterTypes();
	RegisterGlobals();
}

void ScriptEngine::Cleanup()
{
	if(engine)
		R(engine->ShutDownAndRelease());
}

void ScriptEngine::RegisterTypes()
{
	RegisterVEC2();
	RegisterVEC3();
	RegisterVEC4();

	Unit::Register(engine);
	PlayerController::Register(engine);
}

void ScriptEngine::RegisterGlobals()
{
	R(engine->RegisterGlobalProperty("Unit@ user", &globals::user));
	R(engine->RegisterGlobalProperty("Player@ player", &globals::player));

	globals::user = new Unit;
	globals::user->human_data = new Human;
	globals::user->human_data->hair_color = VEC4(1, 0.5f, 0, 1);
}

void VEC2_ctor(VEC2* self)
{
	new(self)VEC2();
}

void VEC2_ctor_floats(float x, float y, VEC2* self)
{
	new(self)VEC2(x, y);
}

void VEC2_ctor_VEC2(const VEC2& v, VEC2* self)
{
	new(self)VEC2(v);
}

void VEC2_ctor_list(float* list, VEC2* self)
{
	new(self)VEC2(list);
}

void ScriptEngine::RegisterVEC2()
{
	R(engine->RegisterObjectType("VEC2", sizeof(VEC2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS));
	R(engine->RegisterObjectProperty("VEC2", "float x", asOFFSET(VEC2, x)));
	R(engine->RegisterObjectProperty("VEC2", "float y", asOFFSET(VEC2, y)));
	R(engine->RegisterObjectBehaviour("VEC2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(VEC2_ctor), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC2", asBEHAVE_CONSTRUCT, "void f(float, float)", asFUNCTION(VEC2_ctor_floats), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC2", asBEHAVE_CONSTRUCT, "void f(const VEC2& in)", asFUNCTION(VEC2_ctor_VEC2), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC2", asBEHAVE_LIST_CONSTRUCT, "void f(const int& in){float, float}", asFUNCTION(VEC2_ctor_list), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectMethod("VEC2", "VEC2& opAddAssign(const VEC2& in)", asMETHODPR(VEC2, operator +=, (const VEC2&), VEC2&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2& opSubAssign(const VEC2& in)", asMETHODPR(VEC2, operator -=, (const VEC2&), VEC2&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2& opMulAssign(float)", asMETHODPR(VEC2, operator *=, (float), VEC2&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2& opDivAssign(float)", asMETHODPR(VEC2, operator /=, (float), VEC2&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2 opNeg()", asMETHODPR(VEC2, operator -, () const, VEC2), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2 opAdd(const VEC2& in) const", asMETHODPR(VEC2, operator +, (const VEC2&) const, VEC2), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2 opSub(const VEC2& in) const", asMETHODPR(VEC2, operator -, (const VEC2&) const, VEC2), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2 opMul(float) const", asMETHODPR(VEC2, operator *, (float) const, VEC2), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "VEC2 opDiv(float) const", asMETHODPR(VEC2, operator /, (float) const, VEC2), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC2", "bool opEquals(const VEC2& in) const", asMETHODPR(VEC2, operator ==, (const VEC2&) const, BOOL), asCALL_THISCALL));
}

void VEC3_ctor(VEC3* self)
{
	new(self)VEC3();
}

void VEC3_ctor_floats(float x, float y, float z, VEC3* self)
{
	new(self)VEC3(x, y, z);
}

void VEC3_ctor_VEC3(const VEC3& v, VEC3* self)
{
	new(self)VEC3(v);
}

void VEC3_ctor_list(float* list, VEC3* self)
{
	new(self)VEC3(list);
}

void ScriptEngine::RegisterVEC3()
{
	R(engine->RegisterObjectType("VEC3", sizeof(VEC3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS));
	R(engine->RegisterObjectProperty("VEC3", "float x", asOFFSET(VEC3, x)));
	R(engine->RegisterObjectProperty("VEC3", "float y", asOFFSET(VEC3, y)));
	R(engine->RegisterObjectProperty("VEC3", "float z", asOFFSET(VEC3, z)));
	R(engine->RegisterObjectBehaviour("VEC3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(VEC3_ctor_VEC3), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTION(VEC3_ctor_floats), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC3", asBEHAVE_CONSTRUCT, "void f(const VEC3& in)", asFUNCTION(VEC3_ctor), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC3", asBEHAVE_LIST_CONSTRUCT, "void f(const int& in){float, float, float}", asFUNCTION(VEC3_ctor_list), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectMethod("VEC3", "VEC3& opAddAssign(const VEC3& in)", asMETHODPR(VEC3, operator +=, (const VEC3&), VEC3&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3& opSubAssign(const VEC3& in)", asMETHODPR(VEC3, operator -=, (const VEC3&), VEC3&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3& opMulAssign(float)", asMETHODPR(VEC3, operator *=, (float), VEC3&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3& opDivAssign(float)", asMETHODPR(VEC3, operator /=, (float), VEC3&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3 opNeg()", asMETHODPR(VEC3, operator -, () const, VEC3), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3 opAdd(const VEC3& in) const", asMETHODPR(VEC3, operator +, (const VEC3&) const, VEC3), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3 opSub(const VEC3& in) const", asMETHODPR(VEC3, operator -, (const VEC3&) const, VEC3), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3 opMul(float) const", asMETHODPR(VEC3, operator *, (float) const, VEC3), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "VEC3 opDiv(float) const", asMETHODPR(VEC3, operator /, (float) const, VEC3), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC3", "bool opEquals(const VEC3& in) const", asMETHODPR(VEC3, operator ==, (const VEC3&) const, BOOL), asCALL_THISCALL));
}

void VEC4_ctor(VEC4* self)
{
	new(self)VEC4();
}

void VEC4_ctor_floats(float x, float y, float z, float w, VEC4* self)
{
	new(self)VEC4(x, y, z, w);
}

void VEC4_ctor_VEC4(const VEC4& v, VEC4* self)
{
	new(self)VEC4(v);
}

void VEC4_ctor_list(float* list, VEC4* self)
{
	new(self)VEC4(list);
}

void ScriptEngine::RegisterVEC4()
{
	R(engine->RegisterObjectType("VEC4", sizeof(VEC4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS));
	R(engine->RegisterObjectProperty("VEC4", "float x", asOFFSET(VEC4, x)));
	R(engine->RegisterObjectProperty("VEC4", "float y", asOFFSET(VEC4, y)));
	R(engine->RegisterObjectProperty("VEC4", "float z", asOFFSET(VEC4, z)));
	R(engine->RegisterObjectProperty("VEC4", "float w", asOFFSET(VEC4, w)));
	R(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(VEC4_ctor), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_CONSTRUCT, "void f(float, float, float, float)", asFUNCTION(VEC4_ctor_floats), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_CONSTRUCT, "void f(const VEC4& in)", asFUNCTION(VEC4_ctor_VEC4), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_LIST_CONSTRUCT, "void f(const int& in){float, float, float, float}", asFUNCTION(VEC4_ctor_list), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectMethod("VEC4", "VEC4& opAddAssign(const VEC4& in)", asMETHODPR(VEC4, operator +=, (const VEC4&), VEC4&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4& opSubAssign(const VEC4& in)", asMETHODPR(VEC4, operator -=, (const VEC4&), VEC4&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4& opMulAssign(float)", asMETHODPR(VEC4, operator *=, (float), VEC4&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4& opDivAssign(float)", asMETHODPR(VEC4, operator /=, (float), VEC4&), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4 opNeg()", asMETHODPR(VEC4, operator -, () const, VEC4), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4 opAdd(const VEC4& in) const", asMETHODPR(VEC4, operator +, (const VEC4&) const, VEC4), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4 opSub(const VEC4& in) const", asMETHODPR(VEC4, operator -, (const VEC4&) const, VEC4), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4 opMul(float) const", asMETHODPR(VEC4, operator *, (float) const, VEC4), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "VEC4 opDiv(float) const", asMETHODPR(VEC4, operator /, (float) const, VEC4), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("VEC4", "bool opEquals(const VEC4& in) const", asMETHODPR(VEC4, operator ==, (const VEC4&) const, BOOL), asCALL_THISCALL));
}

enum KeywordGroup
{
	G_TOP,
	G_KEYWORD,
	G_TYPE
};

enum Keyword
{
	K_TYPE,
	K_PROGRESS,
	K_CODE,
	//K_DIALOG
};

void ScriptEngine::ParseQuests()
{
	Tokenizer t;
	t.AddKeyword("quest", 0, G_TOP);

	t.AddKeywords(G_KEYWORD, {
		{ "type", K_TYPE },
		{ "progress", K_PROGRESS },
		{ "code", K_CODE },
		//{ "dialog", K_DIALOG }
	});

	t.AddKeywords(G_TYPE, {
		{ "mayor", Quest::Type::Mayor }
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
	quest->type = Quest::Type::Invalid;

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
			case K_TYPE:
				quest->type = (Quest::Type)t.MustGetKeywordId(G_TYPE);
				break;
			case K_PROGRESS:
				if(!quest->progress.empty())
					t.Throw("Quest progress already declared.");
				t.AssertSymbol('{');
				t.Next();
				do
				{
					const string& item = t.MustGetItemKeyword();
					for(string& other_item : quest->progress)
					{
						if(item == other_item)
							t.Throw("Quest progress '%s' already declared.", item.c_str());
					}
					quest->progress.push_back(item);
					t.Next();						
				} while(!t.IsSymbol('}'));
				break;
			case K_CODE:
				t.AssertSymbol("$(");
				t.Next();
				//t.AssertBlock
				//t.AssertSymbol()
			//case K_DIALOG:
				break;
			}
		}

		if(quest->type == Quest::Type::Invalid)
			t.Throw("Invalid quest type.");

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
