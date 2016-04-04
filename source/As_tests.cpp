#include "Pch.h"
#include "Base.h"
#include <conio.h>
#include "ScriptEngine.h"
#include "Unit.h"

#undef C
#ifdef _DEBUG
#define C(x) assert((x) >= 0)
#else
#define C(x) x
#endif


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

void printu(Unit2* u)
{
	printf("Unit: %p, human_data %p, hair %g %g %g %g\n", u, u->human_data, u->human_data->hair_color.x, u->human_data->hair_color.y,
		u->human_data->hair_color.z, u->human_data->hair_color.w);
}

void print_vec4(const VEC4& v)
{
	printf("%g, %g, %g, %g\n", v.x, v.y, v.z, v.w);
}

void print_vec2(const VEC2& v)
{
	printf("%g, %g\n", v.x, v.y);
}

void echo(int part, int p)
{
	printf("part %d: %d\n", part, p);
}

// Global script variables
Unit2* user; // unit drinked potion

class A
{
public:
	int a;
	A(int a) : a(a) {}
	void printme() { printf("%d", a); }
};

A* Get_A() { return new A(5); }
/*
void as_tests_main()
{
	ScriptEngine& e = ScriptEngine::Get();

	string code =

		R"###(

class Q
{
	A@ a;

	void setty()
	{
		@a = Get_A();
		a.printme();
	}
}

void script_main()
{
	Q q;
	q.setty();
}
		)###";

	e.Init();
	e.ParseQuests();
	e.StartQuest("deliver_letter");

	auto as = e.GetASEngine();

	R(as->RegisterObjectType("A", 0, asOBJ_REF | asOBJ_NOCOUNT));
	R(as->RegisterGlobalFunction("A@ Get_A()", asFUNCTION(Get_A), asCALL_CDECL));
	as->RegisterObjectMethod("A", "void printme()", asMETHOD(A, printme), asCALL_THISCALL);

	as->RegisterGlobalFunction("void print(const VEC2& in)", asFUNCTION(print_vec2), asCALL_CDECL);
	as->RegisterGlobalFunction("void print(const VEC4& in)", asFUNCTION(print_vec4), asCALL_CDECL);
	as->RegisterGlobalFunction("void echo(int, int)", asFUNCTION(echo), asCALL_CDECL);

	asIScriptModule* module = as->GetModule("module", asGM_ALWAYS_CREATE);
	assert(module);

	module->AddScriptSection("section1", code.c_str(), code.length());

	int rr = module->Build();
	assert(rr >= 0);

	asIScriptContext* context = as->CreateContext();
	assert(context);

	asIScriptFunction* fun = module->GetFunctionByName("script_main");
	assert(fun);

	context->Prepare(fun);

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

	context->Release();
	e.Cleanup();




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

	/*cstring quest_base = R"###(shared abstract class QuestBase {
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
	user->human_data = nullptr;*/
	/*user->human_data = new HumanData2;
	user->human_data->hair_color = VEC4(1, 0, 0, 0);*/

	/*C(engine->RegisterGlobalProperty("Unit@ user", &user));
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
	engine->ShutDownAndRelease();*/

	/*_getch();
	exit(0);
}
*/

void as_tests_main()
{
	exit(0);
}
