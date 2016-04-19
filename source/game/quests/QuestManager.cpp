#include "Pch.h"
#include "Base.h"
#include "QuestManager.h"
#include "Quest2.h"
#include "ScriptEngine.h"
#include "Crc.h"
#include "Game.h"
#include "Globals.h"

#include "Quest_Bandits.h"
#include "Quest_BanditsCollectToll.h"
#include "Quest_CampNearCity.h"
#include "Quest_Crazies.h"
#include "Quest_DeliverLetter.h"
#include "Quest_DeliverParcel.h"
#include "Quest_Evil.h"
#include "Quest_FindArtifact.h"
#include "Quest_Goblins.h"
#include "Quest_KillAnimals.h"
#include "Quest_LostArtifact.h"
#include "Quest_Mages.h"
#include "Quest_Main.h"
#include "Quest_Mine.h"
#include "Quest_Orcs.h"
#include "Quest_RescueCaptive.h"
#include "Quest_RetrivePackage.h"
#include "Quest_Sawmill.h"
#include "Quest_SpreadNews.h"
#include "Quest_StolenArtifact.h"
#include "Quest_Wanted.h"

static Quest2* QUEST_FORCED_NONE = (Quest2*)0xFFFFFFFF;
QuestManager QM;

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

struct BuiltinQuest
{
	cstring id;
	Quest::Type type;
	QUEST quest_id;
	int weight;
};

const BuiltinQuest builtin_quests[] = {
	"deliver_letter", Quest::Type::Mayor, Q_DELIVER_LETTER, 3,
	"deliver_parcel", Quest::Type::Mayor, Q_DELIVER_PARCEL, 3,
	"spread_news", Quest::Type::Mayor, Q_SPREAD_NEWS, 2,
	"retrive_package", Quest::Type::Mayor, Q_RETRIVE_PACKAGE, 2,
	"rescue_captive", Quest::Type::Captain, Q_RESCUE_CAPTIVE, 3,
	"bandits_collect_toll", Quest::Type::Captain, Q_BANDITS_COLLECT_TOLL, 3,
	"camp_near_city", Quest::Type::Captain, Q_CAMP_NEAR_CITY, 3,
	"kill_animals", Quest::Type::Captain, Q_KILL_ANIMALS, 3,
	"find_artifact", Quest::Type::Traveler, Q_FIND_ARTIFACT, 3,
	"lost_artifact", Quest::Type::Traveler, Q_LOST_ARTIFACT, 3,
	"stolen_artifact", Quest::Type::Traveler, Q_STOLEN_ARTIFACT, 3,
	"sawmill", Quest::Type::Unique, Q_SAWMILL, 0,
	"mine", Quest::Type::Unique, Q_MINE, 0,
	"bandits", Quest::Type::Unique, Q_BANDITS, 0,
	"mages", Quest::Type::Unique, Q_MAGES, 0,
	"mages2", Quest::Type::Unique, Q_MAGES2, 0,
	"orcs", Quest::Type::Unique, Q_ORCS, 0,
	"orcs2", Quest::Type::Unique, Q_ORCS2, 0,
	"goblins", Quest::Type::Unique, Q_GOBLINS, 0,
	"evil", Quest::Type::Unique, Q_EVIL, 0,
	"crazies", Quest::Type::Unique, Q_CRAZIES, 0,
	"wanted", Quest::Type::Unique, Q_WANTED, 0,
	"main", Quest::Type::Unique, Q_MAIN, 0,
};
const uint n_builtin_quests = countof(builtin_quests);

int chance_for_no_quest[] = {
	0, //Invalid
	2, //Mayor
	1, //Captain
	0, //Traveler
	0, //Unique
};

extern string g_system_dir;

//=================================================================================================
QuestManager::QuestManager()
{
}

//=================================================================================================
Quest* QuestManager::CreateQuest(QUEST quest_id)
{
	Quest* quest = CreateQuestInstance(quest_id);
	quest->refid = counter++;
	return quest;
}

//=================================================================================================
Quest* QuestManager::CreateQuestInstance(QUEST quest_id)
{
	switch(quest_id)
	{
	case Q_DELIVER_LETTER:
		return new Quest_DeliverLetter;
	case Q_DELIVER_PARCEL:
		return new Quest_DeliverParcel;
	case Q_SPREAD_NEWS:
		return new Quest_SpreadNews;
	case Q_RETRIVE_PACKAGE:
		return new Quest_RetrivePackage;
	case Q_RESCUE_CAPTIVE:
		return new Quest_RescueCaptive;
	case Q_BANDITS_COLLECT_TOLL:
		return new Quest_BanditsCollectToll;
	case Q_CAMP_NEAR_CITY:
		return new Quest_CampNearCity;
	case Q_KILL_ANIMALS:
		return new Quest_KillAnimals;
	case Q_FIND_ARTIFACT:
		return new Quest_FindArtifact;
	case Q_LOST_ARTIFACT:
		return new Quest_LostArtifact;
	case Q_STOLEN_ARTIFACT:
		return new Quest_StolenArtifact;
	case Q_SAWMILL:
		return new Quest_Sawmill;
	case Q_MINE:
		return new Quest_Mine;
	case Q_BANDITS:
		return new Quest_Bandits;
	case Q_MAGES:
		return new Quest_Mages;
	case Q_MAGES2:
		return new Quest_Mages2;
	case Q_ORCS:
		return new Quest_Orcs;
	case Q_ORCS2:
		return new Quest_Orcs2;
	case Q_GOBLINS:
		return new Quest_Goblins;
	case Q_EVIL:
		return new Quest_Evil;
	case Q_CRAZIES:
		return new Quest_Crazies;
	case Q_WANTED:
		return new Quest_Wanted;
	case Q_MAIN:
		return new Quest_Main;
	default:
		assert(0);
		return nullptr;
	}
}

//=================================================================================================
Quest2Instance* QuestManager::CreateQuest(Quest2* quest)
{
	assert(quest);

	Game& game = Game::Get();
	asIScriptEngine* engine = ScriptEngine::Get().GetEngine();
	asIScriptContext* context = ScriptEngine::Get().GetContext();

	// create quest instance
	Quest2Instance* instance = new Quest2Instance;
	instance->quest = quest;
	instance->start_loc = game.current_location;
	instance->target_loc = -1;
	instance->progress = 0;
	instance->id = counter++;

	// create script object
	instance->obj = (asIScriptObject*)engine->CreateScriptObject(quest->obj_type);
	cstring name = instance->obj->GetPropertyName(0);
	void* adr = instance->obj->GetAddressOfProperty(0);
	Quest2Instance** prop = (Quest2Instance**)adr;
	*prop = instance;

	// call on_init
	// prepare
	globals::current_quest = instance;
	
	// call
	asIObjectType* type = instance->obj->GetObjectType();
	asIScriptFunction* func = type->GetMethodByDecl("void on_init()");

	R(context->Prepare(func));
	R(context->SetObject(instance->obj));
	R(context->Execute());

	// cleanup
	globals::current_quest = nullptr;

	return instance;
}

//=================================================================================================
QuestHandle QuestManager::CreateQuest(cstring quest_id)
{
	assert(quest_id);

	QuestIndex quest = FindQuest(quest_id);
	if(quest.quest == nullptr)
		return QuestHandle();
	else if(!quest.is_quest2)
		return QuestHandle(CreateQuest(quest.quest->quest_id));
	else
		return QuestHandle(CreateQuest(quest.quest2));
}

//=================================================================================================
QuestManager::QuestIndex QuestManager::FindQuest(cstring quest_id)
{
	assert(quest_id);

	for(const BuiltinQuest& quest : builtin_quests)
	{
		if(strcmp(quest_id, quest.id) == 0)
			return QuestIndex(&quest);
	}

	for(Quest2* quest : new_quests)
	{
		if(quest->id == quest_id)
			return QuestIndex(quest);
	}

	return QuestIndex();
}

//=================================================================================================
bool QuestManager::SetForcedQuest(const string& forced)
{
	if(forced == "none")
	{
		forced_quest = QuestIndex(QUEST_FORCED_NONE);
		forced_quest_id = forced;
		return true;
	}

	QuestIndex found = FindQuest(forced.c_str());
	if(found.quest == nullptr)
		return false;

	forced_quest = found;
	forced_quest_id = forced;
	return true;
}

//=================================================================================================
QuestHandle QuestManager::GetRandomQuest(Quest::Type type)
{
	assert(type == Quest::Type::Mayor || type == Quest::Type::Captain || type == Quest::Type::Traveler);

	if(forced_quest.quest != nullptr)
	{
		QuestIndex forced = forced_quest;
		forced_quest = QuestIndex();

		if(forced.quest2 == QUEST_FORCED_NONE)
		{
			forced_quest_id.clear();
			return QuestHandle();
		}
		else if(!forced_quest.is_quest2)
		{
			if(forced_quest.quest->type == type)
			{
				forced_quest_id.clear();
				return QuestHandle(CreateQuest(forced_quest.quest->quest_id));
			}
		}
		else
		{
			if(forced_quest.quest2->type == type)
			{
				forced_quest_id.clear();
				return QuestHandle(CreateQuest(forced_quest.quest2));
			}
		}

		WARN(Format("Can't apply forced quest '%s' for type %d, using random.", forced_quest_id.c_str(), (int)type));
		forced_quest_id.clear();
	}

	// add all quests with this type to temporary list
	int max_weight = 0;
	for(const BuiltinQuest& quest : builtin_quests)
	{
		if(quest.type == type)
		{
			tmp_list.push_back(WeightPair<QuestIndex>(QuestIndex(&quest), quest.weight));
			max_weight += quest.weight;
		}
	}

	for(Quest2* quest : new_quests)
	{
		if(quest->type == type)
		{
			tmp_list.push_back(WeightPair<QuestIndex>(QuestIndex(quest), 3));
			max_weight += 3;
		}
	}

	if(tmp_list.empty())
		return QuestHandle();

	int chance_for_no_quest_type = chance_for_no_quest[(int)type];
	if(chance_for_no_quest_type > 0)
	{
		tmp_list.push_back(WeightPair<QuestIndex>(QuestIndex(QUEST_FORCED_NONE), chance_for_no_quest_type));
		max_weight += chance_for_no_quest_type;
	}

	// get random quest and create it
	QuestIndex result = RandomItemWeight(tmp_list, max_weight);
	tmp_list.clear();
	if(result.quest2 == QUEST_FORCED_NONE)
		return QuestHandle();
	else if(!result.is_quest2)
		return QuestHandle(CreateQuest(result.quest->quest_id));
	else
		return QuestHandle(CreateQuest(result.quest2));
}

//=================================================================================================
void QuestManager::PrintListOfQuests(PrintMsgFunc print_func, const string* filter)
{
	assert(print_func);

	struct QuestPtr
	{
		cstring name;
		bool script;

		QuestPtr(cstring name, bool script) : name(name), script(script) {}
	};

	LocalVector3<QuestPtr> quests;

	if(filter)
	{
		short i = 0;
		for(const BuiltinQuest& quest : builtin_quests)
		{
			if(_strnicmp(filter->c_str(), quest.id, filter->length()) == 0)
				quests.push_back(QuestPtr(quest.id, false));
			++i;
		}

		i = 0;
		for(Quest2* quest : new_quests)
		{
			if(_strnicmp(filter->c_str(), quest->id.c_str(), filter->length()) == 0)
				quests.push_back(QuestPtr(quest->id.c_str(), true));
			++i;
		}
	}
	else
	{
		short i = 0;
		for(const BuiltinQuest& quest : builtin_quests)
		{
			quests.push_back(QuestPtr(quest.id, false));
			++i;
		}

		i = 0;
		for(Quest2* quest : new_quests)
		{
			quests.push_back(QuestPtr(quest->id.c_str(), true));
			++i;
		}
	}

	if(quests.empty())
	{
		print_func(Format("No quests found starting with '%s'.", filter->c_str()));
		return;
	}

	quests.sort([](QuestPtr& q1, QuestPtr& q2) { return strcmp(q1.name, q2.name) < 0; });

	LocalString s = Format("Quests list (%d):\n", quests.size());
	for(QuestPtr& quest : quests)
	{
		if(quest.script)
			s += Format("%s (script)\n", quest.name);
		else
			s += Format("%s\n", quest.name);
	}
	print_func(s.c_str());
	LOG(s.c_str());
}

//=================================================================================================
void QuestManager::ParseQuests()
{
	Tokenizer t;
	t.AddKeyword("quest", 0, G_TOP);

	t.AddKeywords(G_KEYWORD, {
		{ "type", K_TYPE },
		{ "progress", K_PROGRESS },
		{ "code", K_CODE },
		{ "dialog", K_DIALOG }
	});

	t.AddEnums<Quest::Type>(G_TYPE, {
		{ "mayor", Quest::Type::Mayor },
		{ "captain", Quest::Type::Captain },
		{ "traveler", Quest::Type::Traveler }
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

//=================================================================================================
bool QuestManager::ParseQuest(Tokenizer& t)
{
	Quest2* quest = new Quest2;
	quest->type = Quest::Type::Invalid;
	vector<DialogFunction>* dialog_functions = nullptr;

	try
	{
		t.AssertKeyword(0, G_TOP);
		t.Next();

		// name
		quest->id = t.MustGetItemKeyword();
		Quest2* other_quest = FindNewQuest(quest->id.c_str());
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
				if(!LoadQuestDialog(t, quest, dialog_functions))
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
			R"###(}}; class quest_{0} : base_quest {{
				{1} get_progress() {{ return {1}(instance.progress); }}

				void Quest_AddTimer({1} p, int days) {{ instance.AddTimer(int(p), days); }}
				
				{2}

				)###",
				id, qp, quest->code.c_str());

		if(dialog_functions)
		{
			int index = 0;
			for(DialogFunction& f : *dialog_functions)
			{
				if(!f.is_if)
					w.write("void _f_{}() {{ {}; }}\n\n", index, *f.code);
				else
					w.write("bool _f_{}() {{ return !!({}); }}\n\n", index, *f.code);
				StringPool.Free(f.code);
				++index;
			}
			dialog_functions->clear();
		}

		w.write("}};\n\n");

		asIScriptEngine* engine = ScriptEngine::Get().GetEngine();
		asIScriptModule* module = engine->GetModule("default");

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
				auto result = ScriptEngine::Get().FindType(type_id);
				if(!result)
				{
					auto type = engine->GetObjectTypeById(type_id);
					t.Throw("Invalid property %s %s, type not registered for serialization.", type->GetName(), prop_name);
				}
				crc.Update(result->id);
			}
			quest->crc = crc.Get();
		}

		new_quests.push_back(quest);
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

//=================================================================================================
Quest2* QuestManager::FindNewQuest(cstring str)
{
	for(Quest2* quest : new_quests)
	{
		if(quest->id == str)
			return quest;
	}
	return nullptr;
}

//=================================================================================================
void QuestManager::Init()
{
	// register base quest type
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

	R(ScriptEngine::Get().GetModule()->AddScriptSection("base_quest", base_quest));

	// count unique quests
	unique_count = 0;
	for(const BuiltinQuest& quest : builtin_quests)
	{
		if(quest.type == Quest::Type::Unique)
			++unique_count;
	}
	for(Quest2* quest : new_quests)
	{
		if(quest->type == Quest::Type::Unique)
			++unique_count;
	}
}

//=================================================================================================
void QuestManager::Reset()
{
	counter = 0;
	unique_completed = 0;
	unique_shown = false;
	all_quests_completed = false;
	DeleteElements(client_quest_items);
}

//=================================================================================================
void QuestManager::Clear()
{
	DeleteElements(client_quest_items);
}

//=================================================================================================
void QuestManager::Save(StreamWriter& f)
{
	f << counter;
	f << unique_completed;
	f << unique_shown;

	f << quest_instances.size();
	for(Quest2Instance* instance : quest_instances)
		SaveQuest(f, *instance);
}

//=================================================================================================
void QuestManager::SaveQuest(StreamWriter& f, Quest2Instance& instance)
{
	Quest2& quest = *instance.quest;

	f << instance.id;
	f << quest.id;
	f << instance.progress;
	f << instance.start_loc;
	f << instance.target_loc;
	f << instance.quest->crc;

	uint properties = quest.obj_type->GetPropertyCount();
	for(uint i = 1u; i < properties; ++i)
	{
		int type_id;
		R(quest.obj_type->GetProperty(i, nullptr, &type_id));
		void* ptr = instance.obj->GetAddressOfProperty(i);
		ScriptEngineType* type = ScriptEngine::Get().FindType(type_id);
		type->write(f, ptr);
	}
}

//=================================================================================================
bool QuestManager::Load(StreamReader& f)
{
	f >> counter;
	f >> unique_completed;
	f >> unique_shown;

	all_quests_completed = (unique_completed >= unique_count);
	if(unique_shown && !all_quests_completed)
		unique_shown = false;

	uint count;
	f >> count;

	if(!f)
	{
		ERROR("Failed to load QuestManager data.");
		return false;
	}

	quest_instances.reserve(count);
	for(uint i = 0; i < count; ++i)
	{
		if(!LoadQuest(f))
		{
			ERROR(Format("Failed to load quest %u.", i));
			return false;
		}
	}

	return true;
}

//=================================================================================================
bool QuestManager::LoadQuest(StreamReader& f)
{
	Ptr<Quest2Instance> instance;
	f >> instance->id;
	f.ReadString1();
	if(!f)
		return false;

	instance->quest = FindNewQuest(BUF);
	if(!instance->quest)
	{
		ERROR(Format("Missing quest '%s'.", BUF));
		return false;
	}

	f >> instance->progress;
	f >> instance->start_loc;
	f >> instance->target_loc;
	uint crc;
	f >> crc;

	if(!f)
		return false;

	if(instance->quest->crc != crc)
	{
		ERROR(Format("Invalid quest data crc for '%s', %u and %u.", instance->quest->id.c_str(), instance->quest->crc, crc));
		return false;
	}

	uint properties = instance->quest->obj_type->GetPropertyCount();
	for(uint i = 1u; i < properties; ++i)
	{
		int type_id;
		R(instance->quest->obj_type->GetProperty(i, nullptr, &type_id));
		void* ptr = instance->obj->GetAddressOfProperty(i);
		ScriptEngineType* type = ScriptEngine::Get().FindType(type_id);
		if(!type->read(f, ptr))
			return false;
	}

	if(!f)
		return false;

	quest_instances.push_back(instance.Pin());
	return true;
}

//=================================================================================================
void QuestManager::EndUniqueQuest()
{
	++unique_completed;
	if(unique_completed >= unique_count && !all_quests_completed)
		all_quests_completed = true;
}

//=================================================================================================
bool QuestManager::CallQuestFunction(Quest2Instance* instance, int index, bool is_if)
{
	assert(instance);

	cstring method = Format("%s _f_%d()", is_if ? "bool" : "void", index);

	asIScriptContext* context = ScriptEngine::Get().GetContext();
	asIScriptFunction* func = instance->quest->obj_type->GetMethodByDecl(method);
	assert(func);

	globals::current_quest = instance;

	R(context->Prepare(func));
	R(context->SetObject(instance->obj));
	R(context->Execute());

	globals::current_quest = nullptr;

	if(is_if)
		return context->GetReturnDWord() != 0;
	else
		return true;
}

//=================================================================================================
void QuestManager::SetProgress(Quest2Instance* instance, int progress)
{
	assert(instance);

	instance->progress = progress;

	globals::current_quest = instance;

	asIScriptContext* context = ScriptEngine::Get().GetContext();
	asIScriptFunction* func = instance->quest->obj_type->GetMethodByDecl("void on_progress()");
	assert(func);

	R(context->Prepare(func));
	R(context->SetObject(instance->obj));
	R(context->Execute());

	globals::current_quest = nullptr;
}

void QuestManager::AddQuestItemRequest(const Item** item, cstring name, int quest_refid, bool is_new)
{
	assert(item && name && quest_refid != -1);

	QuestItemRequest* q = new QuestItemRequest;
	q->item = item;
	q->name = name;
	q->quest_refid = quest_refid;
	q->is_new = true;

	quest_item_requests.push_back(q);
}

Item* QuestManager::FindClientQuestItem(cstring id, int refid, bool is_new)
{
	assert(id);

	for(Item* item : client_quest_items)
	{
		if(item->id == id && (refid == -1 || item->IsQuest(refid)))
			return item;
	}

	return nullptr;
}

bool QuestManager::ReadQuestItems(StreamReader& f)
{
	const int QUEST_ITEM_MIN_SIZE = 7;

	word quest_items_count;
	if(!f.Read(quest_items_count)
		|| !f.Ensure(QUEST_ITEM_MIN_SIZE * quest_items_count))
	{
		ERROR("Read world: Broken packet for quest items.");
		return false;
	}

	client_quest_items.reserve(quest_items_count);
	for(word i = 0; i < quest_items_count; ++i)
	{
		if(!f.ReadString1())
		{
			ERROR(Format("Read world: Broken packet for quest item %u.", i));
			return false;
		}

		const Item* base_item;
		if(BUF[0] != '$')
			base_item = FindItem(BUF);
		else
			base_item = FindItem(BUF + 1);
		if(!base_item)
		{
			ERROR(Format("Read world: Missing base item '%s' for quest item %u.", BUF, i));
			return false;
		}

		Item* item = CreateItemCopy(base_item);
		if(!f.ReadString1(item->id)
			|| !f.ReadString1(item->name)
			|| !f.ReadString1(item->desc)
			|| !f.Read(item->refid))
		{
			ERROR(Format("Read world: Broken packet for quest item %u (2).", i));
			delete item;
			return false;
		}
		else
			client_quest_items.push_back(item);
	}

	return true;
}

void QuestManager::ApplyQuestItemRequests()
{
	/*for(QuestItemRequest* qir : quest_item_requests)
	{
		*qir->item = FindClientQuestItem(qir->name.c_str(), qir->quest_refid);
		delete qir;
	}
	quest_item_requests.clear();*/
}

const Item* QuestManager::FindQuestItem(cstring name, int refid, bool is_new)
{
	/*if(!is_new)
	{
		for(Quest* quest : )
	}
	else
	{

	}*/
	return nullptr;
}

void QuestManager::AcceptQuest(Quest* quest, int timeout)
{
	assert(quest);

	quest->start_time = Game::Get().worldtime;
	quest->state = Quest::Started;
	quest->quest_index = quests.size();
	quests.push_back(quest);
	RemoveElement(unaccepted_quests, quest);

	if(timeout == 1)
		quests_timeout.push_back((Quest_Dungeon*)quest);
	else if(timeout == 2)
		quests_timeout2.push_back(quest);
}

void QuestManager::RemoveTimeout(Quest* quest, int timeout)
{
	assert(quest);
	assert(timeout == 1 || timeout == 2);

	if(timeout == 1)
		RemoveElementTry(quests_timeout, (Quest_Dungeon*)quest);
	else
		RemoveElementTry(quests_timeout2, quest);
}

void QuestManager::AddPlaceholderQuest(Quest* quest)
{
	assert(quest);

	quest->quest_index = quests.size();
	quests.push_back(quest);
}
