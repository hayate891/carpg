#include "Pch.h"
#include "Base.h"
#include <conio.h>

class ScriptEngine
{
public:
	/*HSQUIRRELVM vm;

	ScriptEngine(HSQUIRRELVM vm) : vm(vm) {}

	void AddClass(cstring name)
	{
		sq_pushroottable(vm);
		sq_pushstring(vm, name, -1);
		C(sq_newclass(vm, false));
		sq_pop(vm, 1);
	}*/
};

#undef C
#ifdef _DEBUG
#define C(x) assert((x) >= 0)
#else
#define C(x) x
#endif

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

struct HumanData2
{
	VEC4 hair_color;
};

struct Unit2
{
	HumanData2* human_data;
	int test;
};
int dada;

void VEC4_ctor(VEC4* self)
{
	new(self)VEC4();
}

void VEC4_ctor2(float x, float y, float z, float w, VEC4* self)
{
	new(self)VEC4(x, y, z, w);
}

void printu(Unit2* u)
{
	printf("Unit: %p, human_data %p, hair %g %g %g %g\n", u, u->human_data, u->human_data->hair_color.x, u->human_data->hair_color.y,
		u->human_data->hair_color.z, u->human_data->hair_color.w);
}

void printv4(VEC4& v)
{
	printf("dada");
}

class Quest2
{
public:
};

class Quest2Instance
{
public:
	Quest2* quest;
	int progress;
};

class Quest2Manager
{
public:
	Quest2Instance* current_quest;
	int prev_quest_progress;
};

class QuestEntry
{
public:
};

// Global script variables
Unit2* user; // unit drinked potion

void squirrel_main()
{
	// konsola
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	// polskie znaki w konsoli, tymczasowe rozwi¹zanie
	SetConsoleCP(1250);
	SetConsoleOutputCP(1250);


	string code = //"void script_main() { if(user.human_data !is null) user.human_data.hair_color = VEC4(0,1,0,1); }";

		R"###(
		class Base
		{
			int progress;
		};

		class Quest1 : Base
		{
			enum Progress
			{
				none, started, timeout
			};
		};

		class Quest2 : Base { 
			enum Progress { none, started, timeon};
		};

		int main() {
			Quest1 q = Quest1();
			q.progress = Quest1::none;
			return 0;
        }

		)###";

	/*R"###(
		class Base
		{
			int progress;
		};

		class Quest1 : Base
		{
			const int none = 0;
			const int started = 1;
			const int timeout = 2;
		};

		class Quest2 : Base { 
			const int none = 0;
			const int started = 1;
			const int timeon = 2;
		};

		int main() {
			Quest1 q = Quest1();
			q.progress = Quest1::none;
			return 0;
        }

		)###";*/

	cstring quest_base = R"###(shared abstract class QuestBase {
		};
		)###";
		


	asIScriptEngine* engine = asCreateScriptEngine();
	assert(engine);

	C(engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, asCALL_CDECL));

	C(engine->RegisterObjectType("VEC4", sizeof(VEC4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_ALLFLOATS | asOBJ_APP_CLASS_C));
	C(engine->RegisterObjectProperty("VEC4", "float x", asOFFSET(VEC4, x)));
	C(engine->RegisterObjectProperty("VEC4", "float y", asOFFSET(VEC4, y)));
	C(engine->RegisterObjectProperty("VEC4", "float z", asOFFSET(VEC4, z)));
	C(engine->RegisterObjectProperty("VEC4", "float w", asOFFSET(VEC4, w)));
	C(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(VEC4_ctor), asCALL_CDECL_OBJLAST));
	C(engine->RegisterObjectBehaviour("VEC4", asBEHAVE_CONSTRUCT, "void f(float x, float y, float z, float w)", asFUNCTION(VEC4_ctor2), asCALL_CDECL_OBJLAST));

	C(engine->RegisterObjectType("HumanData", 0, asOBJ_REF | asOBJ_NOCOUNT));
	C(engine->RegisterObjectProperty("HumanData", "VEC4 hair_color", asOFFSET(HumanData2, hair_color)));

	C(engine->RegisterObjectType("Unit", 0, asOBJ_REF | asOBJ_NOCOUNT));
	C(engine->RegisterObjectProperty("Unit", "HumanData@ human_data", asOFFSET(Unit2, human_data)));
	C(engine->RegisterObjectProperty("Unit", "int test", asOFFSET(Unit2, test)));

	C(engine->RegisterGlobalFunction("void printv4(VEC4& in)", asFUNCTION(printv4), asCALL_CDECL));
	C(engine->RegisterGlobalFunction("void printu(Unit@)", asFUNCTION(printu), asCALL_CDECL));

	user = new Unit2;
	user->human_data = nullptr;
	/*user->human_data = new HumanData2;
	user->human_data->hair_color = VEC4(1, 0, 0, 0);*/

	C(engine->RegisterGlobalProperty("Unit@ user", &user));
	C(engine->RegisterGlobalProperty("int dada", &dada));

	asIScriptModule* module = engine->GetModule("module", asGM_ALWAYS_CREATE);
	assert(module);

	C(module->AddScriptSection("section1", code.c_str(), code.length()));

	C(module->Build());

	asIScriptContext* context = engine->CreateContext();
	assert(context);

	asIScriptFunction* fun = module->GetFunctionByName("script_main");
	assert(fun);

	C(context->Prepare(fun));

	int r = context->Execute();
	if(r == asEXECUTION_EXCEPTION)
	{
		cstring ex = context->GetExceptionString();
		asIScriptFunction* fun = context->GetExceptionFunction();
		int col;
		cstring section;
		int row = context->GetExceptionLineNumber(&col, &section);
		printf("ERROR: Unhandled exception in %s(%d,%d), function %s: %s", section, row, col, fun->GetName(), ex);
	}
	else
		assert(r == asEXECUTION_FINISHED);

	// cleanup
	context->Release();
	engine->ShutDownAndRelease();

	_getch();
	exit(0);
}
