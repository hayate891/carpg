#include "Pch.h"
#include "Base.h"
#include "CaScript.h"
#include <conio.h>
#include <iostream>

/*
statement (linijka kodu)
	function
	declare_var
	var = expr

expression (co� co zwraca warto��)
	int
	function
	cstring
	var
	expr op expr

function
	fun_name(params)

declare_var
	var_type name [, name ...]

op
	+
*/

/*#include "Attribute.h"
#include "Skill.h"
#include "Unit.h"
*/
/*
https://github.com/svn2github/angelscript-svn/blob/master/angelscript/source/as_callfunc_x86.cpp
https://github.com/svn2github/angelscript-svn/blob/fc64855bcff8ce5cfd442362f7c199c881f71ceb/angelscript/include/angelscript.h
*/

/*void Unit_Mod(Unit* unit, Attribute a, int value)
{

}

void Unit_Mod(Unit* unit, Skill s, int value)
{

}*/

enum FuncType
{
	Func,
	FuncObj,
	FuncThis
};

struct CallContext
{
	FuncType type;
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
		cmp ecx, 0;
		je endcopy;
	copyloop:
		push dword ptr[eax];
		add eax, 4;
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
		cmp ecx, 0;
		je endcopy;
	copyloop:
		push dword ptr[eax];
		add eax, 4;
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
		cmp ecx, 0;
		je endcopy;
	copyloop:
		push dword ptr[eax];
		add eax, 4;
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

/*float int2float(int a)
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

*/

enum KEYWORD_GROUP
{
	G_TYPE
};

// valid param result var
cas::TypeInfo types[] = {
	{ "void", false, true, false },
	{ "int", true, true, true },
	{ "cstring", true, true, false },
	{ "params", true, false, false },
	{ "string", true, true, true}
};

void cas::Engine::Init()
{
	t.AddKeywords(G_TYPE, {
		{ "void", Void },
		{ "int", Int },
		{ "cstring", Cstring },
		{ "params", Params },
		{ "string", String }
	});
}

enum Op
{
	add,
	sub,
	push_int,
	push_str,
	push_local,
	pop,
	locals,
	set_local,
	call,
	params,
	ret
};

struct ParseVar
{
	string id;
	cas::Type type;
	int index;
};

vector<string> pstrs;
vector<byte> pcode;
vector<ParseVar> pvars;

ParseVar* FindVar(const string& id)
{
	for(ParseVar& p : pvars)
	{
		if(p.id == id)
			return &p;
	}
	return NULL;
}

cas::Type cas::Engine::ParseItem()
{
	if(t.IsInt())
	{
		int num = t.GetInt();
		t.Next();
		pcode.push_back(push_int);
		pcode.push_back(num & 0xFF);
		pcode.push_back((num & 0xFF00) >> 8);
		pcode.push_back((num & 0xFF0000) >> 16);
		pcode.push_back((num & 0xFF000000) >> 24);
		return Int;
	}
	else if(t.IsItem())
	{
		Function* fun = FindFunction(t.GetItem());
		if(fun)
		{
			ParseFunction(fun);
			return fun->result;
		}
		else
		{
			ParseVar* var = FindVar(t.GetItem());
			if(!var)
				t.Unexpected();
			t.Next();
			pcode.push_back(push_local);
			pcode.push_back(var->index);
			return var->type;
		}
	}
	else if(t.IsString())
	{
		pcode.push_back(push_str);
		pcode.push_back(pstrs.size());
		pstrs.push_back(t.GetString());
		t.Next();
		return Cstring;
	}
	else
	{
		t.Unexpected();
		return Void;
	}

	return Void;
}

cas::Type cas::Engine::ParseExpression()
{
	Type left = ParseItem();
	if(t.IsSymbol('+') || t.IsSymbol('-'))
	{
		bool o_add = t.IsSymbol('+');
		t.Next();
		Type right = ParseItem();
		if(left != Int || right != Int)
			t.Throw("Can't %s types '%s' and '%s'.", o_add ? "add" : "subtract", types[left].name, types[right].name);
		pcode.push_back(o_add ? add : sub);
		return Int;
	}
	else
		return left;
}

void cas::Engine::ParseFunction(Function* f)
{
	t.Next();
	t.AssertSymbol('(');
	t.Next();
	vector<cas::Type> args;
	while(true)
	{
		if(t.IsSymbol(')'))
			break;
		// expression
		args.push_back(ParseExpression());
		if(t.IsSymbol(')'))
			break;
		t.AssertSymbol(',');
		t.Next();
	}
	t.Next();

	uint count = f->args.size();
	if(f->variadic)
		--count;

	// check arg count
	if(args.size() < count)
	{
		if(!f->variadic)
			t.Throw("Function '%s' takes %d arguments, not %d.", f->id.c_str(), f->args.size(), args.size());
		else
			t.Throw("Function '%s' takes minimum %d arguments, not %d.", f->id.c_str(), f->args.size(), args.size());
	}

	// check args
	for(uint i = 0; i < count; ++i)
	{
		if(f->args[i] != args[i])
			t.Throw("Function '%s' takes %s as %d argument, not %s.", f->id.c_str(), types[f->args[i]].name, i + 1, types[args[i]].name);
	}

	// params
	if(f->variadic)
	{
		pcode.push_back(params);
		if(args.size() == count)
			pcode.push_back(0);
		else
			pcode.push_back(args.size() - count);
	}

	// function call
	pcode.push_back(call);
	pcode.push_back(f->index);
}

void cas::Engine::Parse()
{
	try
	{
		t.Next();

		pcode.push_back(locals);
		pcode.push_back(0);

		while(true)
		{
			if(t.IsEof())
				break;

			if(t.IsKeywordGroup(G_TYPE))
			{
				Type type = (Type)t.GetKeywordId(G_TYPE);
				if(!types[type].validVar)
					t.Throw("Can't declare variable of type '%s'.", types[type].name);
				t.Next();
				while(true)
				{
					const string& s = t.MustGetItem();
					if(FindFunction(s))
						t.Throw("Can't declare variable '%s %s', already a function.", types[type].name, s.c_str());
					if(FindVar(s))
						t.Throw("Variable '%s' already declared.", s.c_str());
					ParseVar pv;
					pv.index = pvars.size();
					pv.id = s;
					pv.type = type;
					pvars.push_back(pv);
					t.Next();
					if(t.IsSymbol('='))
					{
						t.Next();
						Type result = ParseExpression();
						if(result != type)
							t.Throw("Can't assign type '%s' to variable '%s %s'.", types[result].name, types[type].name, pv.id.c_str());
						pcode.push_back(set_local);
						pcode.push_back(pv.index);
					}
					if(!t.IsSymbol(','))
						break;
					t.Next();
				}
			}
			else if(t.IsItem())
			{
				Function* fun = FindFunction(t.GetItem());
				if(fun)
				{
					ParseFunction(fun);
					// pop function unused result value
					if(fun->result != Void)
						pcode.push_back(pop);
				}
				else
				{
					ParseVar* var = FindVar(t.GetItem());
					if(!var)
						t.Unexpected();
					t.Next();
					t.AssertSymbol('=');
					t.Next();
					cas::Type result = ParseExpression();
					if(result != var->type)
						t.Throw("Can't assign to variable '%s', incompatibile types.", var->id.c_str());
					pcode.push_back(set_local);
					pcode.push_back(var->index);
				}
			}
			else
				t.Unexpected();
		}
	}
	catch(Tokenizer::Exception&)
	{
		throw;
	}

	pcode.push_back(ret);
	pcode[1] = pvars.size();

	Run(&pcode[0], pstrs);
}

void cas::Engine::Parse(cstring code)
{
	t.FromString(code);
	Parse();
}

void cas::Engine::ParseFile(cstring file)
{
	if(t.FromFile(file))
		Parse();
}

cas::Function* cas::Engine::FindFunction(const string& id)
{
	for(Function& f : functions)
	{
		if(f.id == id)
			return &f;
	}

	return NULL;
}

void cas::Engine::AddFunction(cstring def, const FunctionInfo& f)
{
	Function fun;
	fun.variadic = false;

	try
	{
		t.FromString(def);

		t.Next();
		fun.result = (Type)t.MustGetKeywordId(G_TYPE);
		TypeInfo& result = types[fun.result];
		if(!result.validResult)
			t.Throw("Function can't return '%s' type.", result.name);
		t.Next();
		fun.id = t.MustGetItem();
		t.Next();
		t.AssertSymbol('(');
		t.Next();
		while(true)
		{
			if(t.IsSymbol(')'))
				break;
			Type type = (Type)t.MustGetKeywordId(G_TYPE);
			TypeInfo& info = types[type];
			if(!info.validParam)
				t.Throw("Type '%s' is invalid parameter.", info.name);
			fun.args.push_back(type);
			t.Next();
			if(t.IsSymbol(')'))
				break;
			t.AssertSymbol(',');
			t.Next();
		}
		t.Next();
		t.AssertEof();

		for(uint i = 0; i < fun.args.size(); ++i)
		{
			if(fun.args[i] == Params)
			{
				if(i == fun.args.size() - 1)
					fun.variadic = true;
				else
					t.Throw("Function variadic params must be last parameter.");
			}
		}

		fun.f = f;
		fun.index = functions.size();
		functions.push_back(fun);
	}
	catch(Tokenizer::Exception&)
	{
		throw;
	}
}

Function cas::Engine::ParseFunctionDef(cstring def, const FunctionInfo& f)
{
	Function fun;
	fun.variadic = false;

	try
	{
		t.FromString(def);

		t.Next();
		fun.result = (Type)t.MustGetKeywordId(G_TYPE);
		TypeInfo& result = types[fun.result];
		if(!result.validResult)
			t.Throw("Function can't return '%s' type.", result.name);
		t.Next();
		fun.id = t.MustGetItem();
		t.Next();
		t.AssertSymbol('(');
		t.Next();
		while(true)
		{
			if(t.IsSymbol(')'))
				break;
			Type type = (Type)t.MustGetKeywordId(G_TYPE);
			TypeInfo& info = types[type];
			if(!info.validParam)
				t.Throw("Type '%s' is invalid parameter.", info.name);
			fun.args.push_back(type);
			t.Next();
			if(t.IsSymbol(')'))
				break;
			t.AssertSymbol(',');
			t.Next();
		}
		t.Next();
		t.AssertEof();

		for(uint i = 0; i < fun.args.size(); ++i)
		{
			if(fun.args[i] == Params)
			{
				if(i == fun.args.size() - 1)
					fun.variadic = true;
				else
					t.Throw("Function variadic params must be last parameter.");
			}
		}

		fun.f = f;
	}
	catch(Tokenizer::Exception&)
	{
		throw;
	}

	return fun;
}

void cas::Engine::AddFunction(cstring def, const FunctionInfo& f)
{
	Function fun = ParseFunctionDef(def, f);
	if(f.is_method)
		throw "Object method can't be used as function!";
	fun.index = functions.size();
	functions.push_back(fun);
}

cas::Class* cas::Engine::AddClass(cstring id)
{
	Class* c = new Class(this, id);
	classes.push_back(c);
	return c;
}

void cas::Class::AddFunction(cstring def, const FunctionInfo& f)
{
	Function fun = engine->ParseFunctionDef(def, f);
	//if(f.)
}

void cas::Engine::Run(byte* code, vector<string>& strs)
{
	vector<StackItem> stack;
	vector<StackItem> loc;
	vector<int> args;

	byte* b = code;
	while(true)
	{
		Op op = (Op)*b;
		++b;
		
		switch(op)
		{
		case add:
			{
				assert(stack.size() >= 2);
				StackItem a = stack.back();
				stack.pop_back();
				assert(a.type == Int);
				StackItem& b = stack.back();
				assert(b.type == Int);
				b.i += a.i;
			}
			break;
		case sub:
			{
				assert(stack.size() >= 2);
				StackItem a = stack.back();
				stack.pop_back();
				assert(a.type == Int);
				StackItem& b = stack.back();
				assert(b.type == Int);
				b.i -= a.i;
			}
			break;
		case push_int:
			stack.push_back(StackItem(*(int*)b));
			b += 4;
			break;
		case push_str:
			{
				byte a = *b++;
				assert(a < strs.size());
				stack.push_back(StackItem(strs[a].c_str()));
			}
			break;
		case push_local:
			{
				byte a = *b++;
				assert(a < loc.size());
				stack.push_back(loc[a]);
			}
			break;
		case pop:
			assert(!stack.empty());
			stack.back().Release();
			stack.pop_back();
			break;
		case locals:
			{
				byte a = *b++;
				loc.resize(a);
			}
			break;
		case set_local:
			{
				byte a = *b++;
				assert(a < loc.size());
				assert(!stack.empty());
				loc[a] = stack.back();
				stack.pop_back();
			}
			break;
		case call:
			{
				byte f = *b++;
				assert(f < functions.size());
				Function& fun = functions[f];
				CallContext cc;

				cc.f = fun.f.ptr;
				cc.obj = NULL;
				cc.returnFloat = false;
				cc.type = Func;

				if(fun.args.empty())
				{
					cc.args = NULL;
					cc.paramsSize = 0;
				}
				else
				{
					args.clear();
					vector<StackItem>::reverse_iterator it = stack.rbegin();
					for(uint i = 0; i < fun.args.size(); ++i, ++it)
						args.push_back(it->i);
					cc.args = &args[0];
					cc.paramsSize = 4 * args.size();
				}

				int result = CallFunction(cc);

				// clean stack
				for(uint i = 0; i < fun.args.size(); ++i)
				{
					stack.back().Release();
					stack.pop_back();
				}

				if(fun.result == Int)
					stack.push_back(StackItem(result));
				else if(fun.result == String)
					stack.push_back(StackItem((Str*)result));
			}
			break;
		case params:
			{
				int a = (int)(*b++);
				assert(a <= (int)stack.size());
				ParamList* p = ParamListPool.Get();
				p->refs = 1;
				p->items.resize(a);
				for(int i = a-1; i >= 0; --i)
				{
					p->items[i] = ParamItem(stack.back());
					stack.pop_back();
				}
				stack.push_back(StackItem(p));
			}
			break;
		case ret:
			if(!stack.empty())
			{
				printf("ERROR: Not empty stack!");
				_getch();
			}
			return;
		}
	}
}

void print(cstring str, cas::ParamList& params)
{
	string b;

	cstring s = str;
	char c;
	while((c = *s++) != 0)
	{
		if(c == '{')
		{
			c = *s;
			if(c >= '0' && c <= '9')
			{
				int index = c - '0';
				if(index >= (int)params.items.size())
				{
					printf("ERROR: Invalid format string '%s' index %d.", str, index);
					_getch();
					return;
				}

				++s;
				c = *s;
				if(c != '}')
				{
					printf("ERROR: Invalid format string '%s'.", str);
					_getch();
					return;
				}

				++s;
				b += params.items[index].ToString();
			}
			else
			{
				printf("ERROR: Invalid format string '%s'.", str);
				_getch();
				return;
			}
		}
		else
			b += c;
	}

	printf(b.c_str());
}

void pause()
{
	_getch();
}

int getint()
{
	int d;
	scanf_s("%d", &d);
	return d;
}

Str* getstr()
{
	Str* s = StrPool.Get();
	s->refs = 1;
	std::cin >> s->s;
	return s;
}

struct PCX
{
	int a;

	void Mod(cstring s, int value)
	{
		printf("PCX Mod: %d %d (a = %d)", s, value, a);
	}
};

int main()
{
	cas::Engine e;
	e.Init();

	PCX pc;
	pc.a = 7;

	try
	{
		/*
		0 - print
		1 - pause
		2 - getint
		3 - getstr
		4
		*/
		e.AddFunction("void print(cstring, params)", FUNCTION(print));
		e.AddFunction("void pause()", FUNCTION(pause));
		e.AddFunction("int getint()", FUNCTION(getint));
		e.AddFunction("string getstr()", FUNCTION(getstr));

		cas::Class* c = e.AddClass("PCX");
		c->AddFunction("void Mod(cstring, int)", METHOD(PCX, Mod));

		e.AddGlobal("PCX pc", &pc);

		if(0)
		{
			e.ParseFile("script/1.txt");
		}
		else
		{
			byte code[] = {
				push_str, 0,
				push_int, 10, 0, 0, 0,
				push_global, 0,
				call, 4,
				ret
			};

			vector<string> strs = {
				"str"
			};

			e.Run(code, strs);
		}
	}
	catch(Tokenizer::Exception& e)
	{
		printf(e.ToString());
		_getch();
	}

	return 0;
}

/*
TODO:
+ overloaded functions

+ default params
*/