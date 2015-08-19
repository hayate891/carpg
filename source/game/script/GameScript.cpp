#include "Pch.h"
#include "Base.h"
/*#include "CaScript.h"
#include "Attribute.h"
#include "Skill.h"
#include "Unit.h"
*/
/*
https://github.com/svn2github/angelscript-svn/blob/master/angelscript/source/as_callfunc_x86.cpp
https://github.com/svn2github/angelscript-svn/blob/fc64855bcff8ce5cfd442362f7c199c881f71ceb/angelscript/include/angelscript.h
*/
/*
void Unit_Mod(Unit* unit, Attribute a, int value)
{

}

void Unit_Mod(Unit* unit, Skill s, int value)
{

}

enum Type
{
	Func,
	FuncObj,
	FuncThis
};

struct CallContext
{
	Type type;
	void* f;
	void* args;
	void* obj;
	int paramsSize;
	bool returnFloat;
};


int CallFunction(void* f, void* args, int paramsSize)
{
	int result = 0;

	__asm
	{
		// save registers
		push ecx;

		// clear fpu stack ?
		fninit;

		// copy arguments
		mov ecx, paramsSize;
		mov eax, args;
		add eax, ecx;
		cmp ecx, 0;
		je endcopy;
	copyloop:
		sub eax, 4;
		push dword ptr[eax];
		sub ecx, 4;
		jne copyloop;
	endcopy:

		// call
		call[f];

		// pop arguments
		add esp, paramsSize;

		// copy return value
		mov result, eax;

		// restore registers
		pop ecx;
	};

	return result;
}

int CallFunctionObj(void* f, void* obj, void* args, int paramsSize)
{
	int result = 0;

	__asm
	{
		// save registers
		push ecx;

		// clear fpu stack ?
		fninit;

		// copy arguments
		mov ecx, paramsSize;
		mov eax, args;
		add eax, ecx;
		cmp ecx, 0;
		je endcopy;
	copyloop:
		sub eax, 4;
		push dword ptr[eax];
		sub ecx, 4;
		jne copyloop;
	endcopy:

		// push obj
		push obj;

		// call
		call[f];

		// pop arguments
		add esp, paramsSize;
		add esp, 4;

		// copy return value
		mov result, eax;

		// restore registers
		pop ecx;
	};

	return result;
}

int CallFunctionThis(void* f, void* obj, void* args, int paramsSize)
{
	int result = 0;

	__asm
	{
		// save registers
		push ecx;

		// clear fpu stack ?
		fninit;

		// copy arguments
		mov ecx, paramsSize;
		mov eax, args;
		add eax, ecx;
		cmp ecx, 0;
		je endcopy;
	copyloop:
		sub eax, 4;
		push dword ptr[eax];
		sub ecx, 4;
		jne copyloop;
	endcopy:

		// set this
		mov ecx, obj;

		// call
		call[f];
		
		// copy return value
		mov result, eax;

		// restore registers
		pop ecx;
	};

	return result;
}

int CallFunction(CallContext& cc)
{
	int result;

	switch(cc.type)
	{
	case Func:
		result = CallFunction(cc.f, cc.args, cc.paramsSize);
		break;
	case FuncObj:
		result = CallFunctionObj(cc.f, cc.obj, cc.args, cc.paramsSize);
		break;
	case FuncThis:
		result = CallFunctionThis(cc.f, cc.obj, cc.args, cc.paramsSize);
		break;
	default:
		assert(0);
		return -1;
	}

	if(cc.returnFloat)
	{
		__asm
		{
			fstp dword ptr[result];
		};
	}

	return result;
}

float int2float(int a)
{
	union C
	{
		int i;
		float f;
	};

	C c;
	c.i = a;
	return c.f;
}

Unit* Player_GetUnit(PlayerController* player)
{
	return player->unit;
}

enum Op
{
	push_int,
	push_global,

	call_class,

	ret
};

byte code[] = {
	push_int, 10, 0, 0, 0,
	push_int, 0, 0, 0, 0,
	push_global, 0,
	call_class, 0,
	ret
};

enum TypeId
{
	Int,
	Class,
	CustomClass
};

class Type
{
public:
	string name;
	int id;
};

vector<Type> types;

struct StackItem
{
	int value;
	int type;
};

class A
{
public:
	void Foo(int a, int b)
	{

	}
};

void Run()
{
	types.resize(3);
	types[0].name = "int";
	types[0].id = Int;
	types[1].name = "class";
	types[1].id = Class;
	types[2].name = "A";
	types[2].id = CustomClass;

	A global;
	vector<StackItem> stack;

	byte* b = code;

	while(true)
	{
		Op op = (Op)*b;
		++b;

		switch(op)
		{
		case push_int:
			stack.push_back(StackItem{ *(int*b), 0 });
			b += 4;
			break;
		case push_global:
			stack.push_back(StackItem{ (int)&global, 2 });
			++b;
			break;
		case call_class:
			{
				byte f = *b;
				++b;
				StackItem s = stack.back();
				assert(types[s.type].id == CustomClass);
				stack.pop_back();
				global.Foo(0, 0);
			}
			break;
		}
	}
}

void InitGameScript()
{
	cas::Engine script;

	cas::Enum* attributes = script.RegisterEnum("Attribute");
	for(int i = 0; i < (int)Attribute::MAX; ++i)
		attributes->Add(g_attributes[i].id, i);

	cas::Enum* skills = script.RegisterEnum("Skill");
	for(int i = 0; i < (int)Skill::MAX; ++i)
		skills->Add(g_skills[i].id, i);

	cas::Class* unit = script.RegisterClass("Unit");
	unit->RegisterFunction("void Mod(Attribute, int)", FUNCTION_EX(Unit_Mod, (Unit*, Attribute, int), void));
	unit->RegisterFunction("void Mod(Skill, int)", FUNCTION_EX(Unit_Mod, (Unit*, Skill, int), void));

	cas::Class* player = script.RegisterClass("Player");
	player->SetBridge(unit, FUNCTION(Player_GetUnit));

	script.AddGlobal("Player pc");

	script.Prepare();

	script.RunLine("pc.Mod(Attribute.str, 10)");

	push_int 10
	push_int 0
	push_global 0
	call_class 0
	ret
}



struct XXX
{
	XXX()
	{
		InitGameScript();
	}
};

XXX xxxx;

void cas::Engine::Init()
{
	RegisterTypes();
}

void cas::Engine::RegisterTypes()
{
	types.resize(CustomType);

	types[Void].name = "void";
	types[Int].name = "int";
	types[Uint].name = "uint";
	types[Char].name = "char";
	types[Byte].name = "byte";
	types[Long].name = "long";
	types[Ulong].name = "ulong";
	types[Cstring].name = "cstring";
	types[String].name = "string";
	types[Class].name = "class";
	types[Bool].name = "bool";
}

enum KeywordGroup
{
	G_KEYWORD,
	G_TYPE
};

enum Keyword
{
	K_BREAK,
	K_CASE,
	K_CLASS,
	K_CONST,
	K_CONTINUE,
	K_DEFAULT,
	K_DO,
	K_ENUM,
	K_STRUCT,
	K_FOR,
	K_IF,
	K_RETURN,
	K_WHILE
};

void cas::Engine::Prepare()
{
	t.AddKeywords(G_KEYWORD, {
		{ "break", K_BREAK },
		{ "case", K_CASE },
		{ "class", K_CLASS },
		{ "const", K_CONST },
		{ "continue", K_CONTINUE },
		{ "default", K_DEFAULT },
		{ "do", K_DO },
		{ "enum", K_ENUM },
		{ "struct", K_STRUCT },
		{ "for", K_FOR },
		{ "if", K_IF },
		{ "return", K_RETURN },
		{ "while", K_WHILE }
	});
}

void cas::Engine::Parse(cstring code)
{

}

struct FunctionDef
{
	cas::ScriptTypeId result;
	string name;
	vector<cas::ScriptTypeId> params;
};

void cas::Engine::ParseDefinition(cstring def)
{
	t.FromString(def);

	FunctionDef fd;

	try
	{
		t.Next();
		fd.result (ScriptTypeId)t.MustGetKeywordId(G_TYPE);
		t.Next();
		fd.name = t.MustGetItem();
		t.Next();
		t.AssertSymbol('(');
		t.Next();
		while(!t.IsSymbol(')'))
		{

		}
	}
	catch(Tokenizer::Exception& e)
	{

	}
}
*/
