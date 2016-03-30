#include "Pch.h"
#include "Base.h"
#include "ScriptEngine.h"
#include "Unit.h"
#include "PlayerController.h"
#include "Game.h"
#include "script/scriptarray.h"
#include "script/scriptstdstring.h"
#include "Crc.h"
#include "Journal.h"

ScriptEngine ScriptEngine::script_engine;
extern string g_system_dir;

struct ScriptException
{
	ScriptException(cstring err) : err(err) {}

	cstring err;
};

namespace globals
{
	Unit* user;
	Unit* talker;
	PlayerController* player;

	Quest2Instance* current_quest;

	Quest2Instance* GetCurrentQuest()
	{
		if(!current_quest)
			throw ScriptException("Not called from quest.");
		return current_quest;
	}
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
	assert(engine);

	engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);

	module = engine->GetModule("default", asGM_ALWAYS_CREATE);
	assert(module);

	context = engine->CreateContext();
	assert(context);

	R(engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL));
	
	R(engine->RegisterFuncdef("void VoidCallback()"));

	AddStandardTypes();
	RegisterTypes();
	RegisterGlobals();

	cstring base_quest =
		R"###(
		class base_quest
		{
			QuestInstance@ instance;

			void on_init() {}
			void on_progress() {}

			TextPtr@ Text(const string& in str) const { return instance.GetText(str); }
		};
		)###";

	R(module->AddScriptSection("base_quest", base_quest));
}

void ScriptEngine::Cleanup()
{
	if(engine)
		R(engine->ShutDownAndRelease());
}

void ScriptEngine::RegisterTypes()
{
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);
	RegisterStdStringUtils(engine);

	RegisterVEC2();
	RegisterVEC3();
	RegisterVEC4();

	RegisterItems();
	Unit::Register(engine);
	PlayerController::Register(engine);
	RegisterWorld();
	RegisterQuestInstance();
	RegisterJournal();
}

void ScriptEngine::RegisterGlobals()
{
	R(engine->RegisterGlobalProperty("Unit@ user", &globals::user));
	R(engine->RegisterGlobalProperty("Unit@ talker", &globals::talker));
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

Location* World_GetRandomSettlement(Location* loc)
{
	Game& game = Game::Get();
	int index = (loc ? game.GetLocationIndex(*loc) : -1);
	return game.locations[game.GetRandomCity(index)];
}

Unit* World_GetMayor()
{
	return nullptr;
}

void ScriptEngine::RegisterWorld()
{
	// location
	R(engine->RegisterObjectType("Location", 0, asOBJ_REF | asOBJ_NOCOUNT));

	// world
	R(engine->RegisterGlobalFunction("Location@ World_GetRandomSettlement(Location@ current = null)", asFUNCTION(World_GetRandomSettlement), asCALL_CDECL));
	R(engine->RegisterGlobalFunction("Unit@ World_GetMayor()", asFUNCTION(World_GetMayor), asCALL_CDECL));
}

Item* Item_CreateQuestItem(const string& id, Quest2Instance* quest)
{
	ItemListResult result;
	const Item* item = FindItem(id.c_str(), false, &result);
	if(!item || result.lis)
		throw Format("Missing item '%s'.", id.c_str());
	Item* quest_item = CreateItemCopy(item);
	if(!quest_item)
		throw Format("Failed to create item copy '%s', only other items supported.", id.c_str());
	quest_item->id = Format("$%s", id.c_str());
	quest_item->refid = quest->id;
	Game::Get().Net_RegisterItem(quest_item, item);
	return quest_item;
}

const string& Item_GetName(const Item* item)
{
	return item->name;
}

void Item_SetName(const string& name, Item* item)
{
	item->name = name;
	Notifier::Get().Add(item, (int)Item::Property::NAME);
}

void Item_Write(StreamWriter& w, void* ptr)
{

}

bool Item_Read(StreamReader& r, void* ptr)
{
	return false;
}

void ScriptEngine::RegisterItems()
{
	R(engine->RegisterObjectType("Item", 0, asOBJ_REF | asOBJ_NOCOUNT));
	R(engine->RegisterObjectMethod("Item", "const string& get_name() const", asFUNCTION(Item_GetName), asCALL_CDECL_OBJLAST));
	R(engine->RegisterObjectMethod("Item", "void set_name(const string& in)", asFUNCTION(Item_SetName), asCALL_CDECL_OBJLAST));

	AddType(ScriptEngineType("Item", WriteFunc(Item_Write), ReadFunc(Item_Read)));
}

void Dialog_Continue(const string& id)
{

}

const string& TextPtr_ToString(const string* s)
{
	return *s;
}

void Quest_Fail()
{
	Quest2Instance* quest_instance = globals::GetCurrentQuest();
}

void Quest_Finish()
{
	Quest2Instance* quest_instance = globals::GetCurrentQuest();
}

typedef void (*VoidCallback)();

void Event_OnEnter(Location* loc, VoidCallback callback)
{

}

void Team_AddReward(int gold)
{

}

void ScriptEngine::RegisterQuestInstance()
{
	R(engine->RegisterObjectType("TextPtr", 0, asOBJ_REF | asOBJ_NOCOUNT));
	R(engine->RegisterObjectMethod("TextPtr", "const string& opImplConv() const", asFUNCTION(TextPtr_ToString), asCALL_CDECL_OBJLAST));

	R(engine->RegisterObjectType("QuestInstance", 0, asOBJ_REF | asOBJ_NOCOUNT));
	R(engine->RegisterObjectProperty("QuestInstance", "int progress", asOFFSET(Quest2Instance, progress)));
	R(engine->RegisterObjectMethod("QuestInstance", "Location@ get_start_loc() const", asMETHOD(Quest2Instance, S_GetStartLoc), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("QuestInstance", "Location@ get_target_loc() const", asMETHOD(Quest2Instance, S_GetTargetLoc), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("QuestInstance", "void set_target_loc(Location@)", asMETHOD(Quest2Instance, S_SetTargetLoc), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("QuestInstance", "TextPtr@ GetText(const string& in text)", asMETHOD(Quest2Instance, GetText), asCALL_THISCALL));
	R(engine->RegisterObjectMethod("QuestInstance", "void AddTimer(int progress, int days)", asMETHOD(Quest2Instance, AddTimer), asCALL_THISCALL));

	R(engine->RegisterGlobalFunction("Item@ Item_CreateQuestItem(QuestInstance@ instance, const string& in item_id)", asFUNCTION(Item_CreateQuestItem), asCALL_CDECL));

	R(engine->RegisterGlobalFunction("void Dialog_Continue(const string& in dialog_id)", asFUNCTION(Dialog_Continue), asCALL_CDECL));

	R(engine->RegisterGlobalFunction("void Quest_Fail()", asFUNCTION(Quest_Fail), asCALL_CDECL));
	R(engine->RegisterGlobalFunction("void Quest_Finish()", asFUNCTION(Quest_Finish), asCALL_CDECL));

	R(engine->RegisterGlobalFunction("void Event_OnEnter(Location@, VoidCallback@)", asFUNCTION(Event_OnEnter), asCALL_CDECL));

	R(engine->RegisterGlobalFunction("void Team_AddReward(int)", asFUNCTION(Team_AddReward), asCALL_CDECL));
}

// Create QuestEntry for current quest and add text or two. Throws if not inside quest or already created.
void Journal_AddQuestEntry(const string* name, const string* text, const string* text2)
{
	assert(name && text);

	Quest2Instance* quest_instance = globals::GetCurrentQuest();

	// check for existing quest entry
	Journal& journal = *Game::Get().game_gui->journal;
	QuestEntry* entry = journal.FindQuestEntry(quest_instance->id);
	if(entry)
		throw ScriptException("Quest entry already added.");

	// add new quest entry
	entry = new QuestEntry;
	entry->id = quest_instance->id;
	entry->name = *name;
	entry->state = QuestEntry::Normal;
	entry->msgs.push_back(*text);
	if(text2)
		entry->msgs.push_back(*text2);
	journal.AddQuestEntry(entry);
}

// Update QuestEntry for current quest. Throws if not inside quest or quest entry don't exists.
void Journal_UpdateQuestEntry(const string* text)
{
	assert(text);

	Quest2Instance* quest_instance = globals::GetCurrentQuest();

	// get quest entry
	Journal& journal = *Game::Get().game_gui->journal;
	QuestEntry* entry = journal.FindQuestEntry(quest_instance->id);
	if(!entry)
		throw ScriptException("Quest entry don't exists.");

	// add text
	entry->msgs.push_back(*text);
	journal.UpdateQuestEntry(entry);
}

void ScriptEngine::RegisterJournal()
{
	R(engine->RegisterGlobalFunction("void Journal_AddQuestEntry(TextPtr@ name, TextPtr@ text, TextPtr@ text2 = null)", asFUNCTION(Journal_AddQuestEntry), asCALL_CDECL));
	R(engine->RegisterGlobalFunction("void Journal_UpdateQuestEntry(TextPtr@ text)", asFUNCTION(Journal_UpdateQuestEntry), asCALL_CDECL));
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
	K_DIALOG
};

void ScriptEngine::ParseQuests()
{
	FIXME;
	g_system_dir = "../system";

	Tokenizer t;
	t.AddKeyword("quest", 0, G_TOP);

	t.AddKeywords(G_KEYWORD, {
		{ "type", K_TYPE },
		{ "progress", K_PROGRESS },
		{ "code", K_CODE },
		{ "dialog", K_DIALOG }
	});

	t.AddEnums<Quest::Type>(G_TYPE, {
		{ "mayor", Quest::Type::Mayor }
	});

	cstring filename = Format("%s/quests.txt", g_system_dir.c_str());
	if(!t.FromFile(filename))
		throw Format("Failed to open file '%s'.", filename);

	try
	{
		t.Next();
		
		while(!t.IsEof())
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
				t.Next();
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
				t.Next();
				break;
			case K_CODE:
				quest->code = t.MustGetString();
				t.Next();
				break;
			case K_DIALOG:
				if(!LoadQuestDialog(t, quest))
					t.Throw("Failed to parse quest dialog, check log for details.");
				break;
			}
		}
		t.Next();

		if(quest->type == Quest::Type::Invalid)
			t.Throw("Invalid quest type.");
		if(quest->progress.empty())
			t.Throw("No quest progress enums.");

		// compile
		fmt::MemoryWriter w;

		cstring id = quest->id.c_str();
		w.write("enum quest_{}_progress {{\n", id);
		for(string& s : quest->progress)
			w.write("\t{},\n", s);
		cstring qp = Format("quest_%s_progress", id);
		w.write(
			R"###(}};

			class quest_{0} : base_quest {{
				{1} get_progress() {{ return {1}(instance.progress); }}

				void Quest_AddTimer({1} p, int days) {{ instance.AddTimer(int(p), days); }}
				
				{2}
			}};
			)###", id, qp, quest->code.c_str());

		R(module->AddScriptSection(quest->id.c_str(), w.c_str()));
		R(module->Build());

		quest->obj_type = module->GetObjectTypeByDecl(Format("quest_%s", id));
		assert(quest->obj_type);

		uint properties = quest->obj_type->GetPropertyCount();
		assert(properties >= 1u); // minimum - instance

		cstring prop_name;
		R(quest->obj_type->GetProperty(0u, &prop_name));
		assert(strcmp(prop_name, "instance") == 0);

		quest->crc = 0;
		if(properties > 1u)
		{
			CRC32 crc;
			int type_id;
			for(uint i = 1u; i < properties; ++i)
			{
				R(quest->obj_type->GetProperty(i, &prop_name, &type_id));
				auto result = script_types.find(type_id);
				if(result == script_types.end())
				{
					auto type = engine->GetObjectTypeById(type_id);
					t.Throw("Invalid property %s %s, type not registered for serialization.", type->GetName(), prop_name);
				}
				crc.Update(result->second.id);
			}
			quest->crc = crc.Get();
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

void ScriptEngine::StartQuest(cstring quest_id)
{
	Game& game = Game::Get();

	assert(quest_id);
	Quest2* quest = FindQuest(quest_id);
	assert(quest);
	Quest2Instance* instance = new Quest2Instance;
	instance->quest = quest;
	instance->start_loc = game.current_location;
	instance->target_loc = -1;
	instance->progress = 0;
	instance->id = 0; // increment
	instance->obj = (asIScriptObject*)engine->CreateScriptObject(quest->obj_type);

	cstring name = instance->obj->GetPropertyName(0);
	void* adr = instance->obj->GetAddressOfProperty(0);
	Quest2Instance** prop = (Quest2Instance**)adr;
	*prop = instance;

	// prepare globals
	globals::current_quest = instance;
	globals::player - game.pc;
	globals::talker = nullptr; // set talker
	globals::user = nullptr;

	// call on_init
	CallCurrentQuestFunction("void on_init()");
	
	// apply globals changes
	// journal
}

void ScriptEngine::CallCurrentQuestFunction(cstring decl)
{
	assert(decl);

	Quest2Instance* instance = globals::current_quest;
	asIObjectType* type = instance->obj->GetObjectType();
	asIScriptFunction* func = type->GetMethodByDecl(decl);

	R(context->Prepare(func));
	R(context->SetObject(instance->obj));
	R(context->Execute());
}

void ScriptEngine::AddType(ScriptEngineType& type)
{
	if(type.id != -1)
		type.name = nullptr;
	else
	{
		assert(type.name);
		auto obj_type = engine->GetObjectTypeByName(type.name);
		assert(obj_type);
		type.id = obj_type->GetTypeId();
		if(IS_SET(obj_type->GetFlags(), asOBJ_REF))
			type.id |= asTYPEID_OBJHANDLE;
	}

	script_types[type.id] = type;
}

template<typename T>
void Write_SimpleType(StreamWriter& w, void* ptr)
{
	//w.Write(w, *(T*)ptr);
}

template<typename T>
bool Read_SimpleType(StreamReader& r, void* ptr)
{
	return r.Read(*(T*)ptr);
}

void ScriptEngine::AddStandardTypes()
{
	AddType(ScriptEngineType(asTYPEID_BOOL, WriteFunc(Write_SimpleType<bool>), ReadFunc(Read_SimpleType<bool>)));
	AddType(ScriptEngineType(asTYPEID_INT8, WriteFunc(Write_SimpleType<char>), ReadFunc(Read_SimpleType<char>)));
	AddType(ScriptEngineType(asTYPEID_INT16, WriteFunc(Write_SimpleType<short>), ReadFunc(Read_SimpleType<short>)));
	AddType(ScriptEngineType(asTYPEID_INT32, WriteFunc(Write_SimpleType<int>), ReadFunc(Read_SimpleType<int>)));
	AddType(ScriptEngineType(asTYPEID_INT64, WriteFunc(Write_SimpleType<__int64>), ReadFunc(Read_SimpleType<__int64>)));
	AddType(ScriptEngineType(asTYPEID_UINT8, WriteFunc(Write_SimpleType<byte>), ReadFunc(Read_SimpleType<byte>)));
	AddType(ScriptEngineType(asTYPEID_UINT16, WriteFunc(Write_SimpleType<word>), ReadFunc(Read_SimpleType<word>)));
	AddType(ScriptEngineType(asTYPEID_UINT32, WriteFunc(Write_SimpleType<uint>), ReadFunc(Read_SimpleType<uint>)));
	AddType(ScriptEngineType(asTYPEID_UINT64, WriteFunc(Write_SimpleType<unsigned __int64>), ReadFunc(Read_SimpleType<unsigned __int64>)));
	AddType(ScriptEngineType(asTYPEID_FLOAT, WriteFunc(Write_SimpleType<float>), ReadFunc(Read_SimpleType<float>)));
	AddType(ScriptEngineType(asTYPEID_DOUBLE, WriteFunc(Write_SimpleType<double>), ReadFunc(Read_SimpleType<double>)));
}

const string* Quest2Instance::GetText(const string& id)
{
	static const string dupa = "dupa";
	return &dupa;
}

void Quest2Instance::AddTimer(int prog, int days)
{

}
