#include "Pch.h"
#include "Base.h"

#include <conio.h>
#include <iostream>

enum Op
{
	ADD,
	TO_STR,

	LOCALS,
	SET_LOCAL,
	PUSH_LOCAL,
	/*PUSH_TRUE,
	PUSH_FALSE,*/
	PUSH_INT,
	/*PUSH_FLOAT,*/
	PUSH_STR,
	POP,

	CALL,

	RET
};

enum VarType
{
	VAR_VOID,
	//VAR_BOOL,
	VAR_INT,
	//VAR_FLOAT
	VAR_STRING
};

cstring var_name[] = {
	"void",
	"int",
	"string"
};

struct String
{
	string s;
	int refs;

	inline String() {}
	inline explicit String(cstring str) : s(str), refs(1) {}
	inline explicit String(const string& str) : s(str), refs(1) {}
};

struct Var
{
	VarType type;
	union
	{
		int _int;
		String* _str;
	};

	inline Var() {}
	inline explicit Var(int _int) : type(VAR_INT), _int(_int) {}
	inline explicit Var(String* _str) : type(VAR_STRING), _str(_str) {}

	inline void AddRef()
	{
		if(type == VAR_STRING)
			++_str->refs;
	}

	inline void Release()
	{
		if(type == VAR_STRING)
		{
			if(--_str->refs == 0)
				delete _str;
		}
	}
};

/*struct Var
{
	VarType type;
};

struct GlobalVar
{
	VarType type;
};

struct CustomType
{

};*/

struct Function
{
	string name;
	int index;
	VarType result, arg;
};

enum ParseNodeType
{
	NODE_INT,
	NODE_STRING,
	NODE_FUNC,
	NODE_VAR,
	NODE_ASSIGN,
	NODE_OP,
	NODE_TO_STR
};

struct ParseNode
{
	ParseNodeType type;
	int value;
	VarType result;
	vector<ParseNode*> nodes;
};

enum ParseGroup
{
	G_TYPE,
	G_VAR,
	G_FUNCTION
};

struct ParseVar
{
	VarType type;
	string name;
	int index;
};

vector<string*> dynamic_keywords;

class ScriptEngine
{
public:
	/*vector<Var> stack;
	vector<GlobalVar> globals;
	vector<CustomType> types;*/
	vector<Function> functions;
	Tokenizer t;
	vector<String*> pstrs;
	vector<ParseVar> pvars;

	void InitFunctions();

	void RunCode(vector<byte>& code, vector<String*>& strs);
	
	ParseNode* ParseItem();
	ParseNode* ParseStatement();
	ParseNode* ParseLine();
	void RunNode(ParseNode* node, vector<byte>& code);
	
	void ParseAndRun(cstring code);
	void ParseAndRunFile(cstring filename);
};

void ScriptEngine::InitFunctions()
{
	{
		Function& f = Add1(functions);
		f.name = "print_int";
		f.index = 0;
		f.arg = VAR_INT;
		f.result = VAR_VOID;
	}
	{
		Function& f = Add1(functions);
		f.name = "get_int";
		f.index = 1;
		f.arg = VAR_VOID;
		f.result = VAR_INT;
	}
	{
		Function& f = Add1(functions);
		f.name = "pause";
		f.index = 2;
		f.arg = VAR_VOID;
		f.result = VAR_VOID;
	}
	{
		Function& f = Add1(functions);
		f.name = "print";
		f.index = 3;
		f.arg = VAR_STRING;
		f.result = VAR_VOID;
	}
	{
		Function& f = Add1(functions);
		f.name = "get_string";
		f.index = 4;
		f.arg = VAR_VOID;
		f.result = VAR_STRING;
	}

	for(Function& f : functions)
	{
		t.AddKeyword(f.name.c_str(), f.index, G_FUNCTION);
	}

	t.AddKeyword("int", VAR_INT, G_TYPE);
	t.AddKeyword("string", VAR_STRING, G_TYPE);
}

void ScriptEngine::RunCode(vector<byte>& code, vector<String*>& strs)
{
	byte* c = code.data();
	byte* cend = code.data() + code.size();
	vector<Var> stack;
	vector<Var> locals;

	while(true)
	{
		Op op = (Op)*c;
		++c;
		assert(c <= cend);

		switch(op)
		{
		case ADD:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_STRING);
				if(l.type == VAR_INT)
					l._int += r._int;
				else
				{
					String* s = new String(l._str->s + r._str->s);
					l.Release();
					r.Release();
					l._str = s;
				}
			}
			break;
		case TO_STR:
			{
				assert(!stack.empty());
				Var& v = stack.back();
				assert(v.type == VAR_INT);
				String* str = new String(Format("%d", v._int));
				v._str = str;
				v.type = VAR_STRING;
			}
			break;
		case LOCALS:
			{
				assert(c + 1 < cend);
				byte count = *c;
				++c;
				if(locals.empty())
					locals.resize(count);
			}
			break;
		case SET_LOCAL:
			{
				assert(c + 1 < cend);
				assert(!stack.empty());
				byte b = *c;
				++c;
				assert(b < locals.size());
				locals[b] = stack.back();
				stack.pop_back();
			}
			break;
		case PUSH_LOCAL:
			{
				assert(c + 1 < cend);
				byte b = *c;
				++c;
				assert(b < locals.size());
				Var& v = locals[b];
				v.AddRef();
				stack.push_back(v);
			}
			break;
		case PUSH_INT:
			{
				assert(c + 4 < cend);
				stack.push_back(Var(*(int*)c));
				c += 4;
			}
			break;
		case PUSH_STR:
			{
				assert(c + 1 < cend);
				byte b = *c;
				++c;
				assert(b < strs.size());
				String* s = strs[b];
				s->refs++;
				stack.push_back(Var(s));
			}
			break;
		case POP:
			assert(stack.empty());
			stack.back().Release();
			stack.pop_back();
			break;
		case CALL:
			{
				assert(c + 1 < cend);
				byte func_id = *c;
				++c;
				switch(func_id)
				{
				case 0: // print_int
					assert(stack.size() > 0);
					assert(stack.back().type == VAR_INT);
					printf("%d", stack.back()._int);
					stack.pop_back();
					break;
				case 1: // get_int
					{
						int num;
						scanf_s("%d", &num);
						stack.push_back(Var(num));
					}
					break;
				case 2: // pause
					_getch();
					break;
				case 3: // print
					assert(stack.size() > 0);
					assert(stack.back().type == VAR_STRING);
					printf(stack.back()._str->s.c_str());
					stack.back().Release();
					stack.pop_back();
					break;
				case 4: // get_string
					{
						String* str = new String;
						std::cin >> str->s;
						str->refs = 1;
						stack.push_back(Var(str));
					}
					break;
				default:
					assert(0);
					break;
				}
			}
			break;
		case RET:
			assert(stack.empty());
			return;
		}
	}
}

ParseNode* ScriptEngine::ParseItem()
{
	if(t.IsInt())
	{
		// int literal
		ParseNode* node = new ParseNode;
		node->type = NODE_INT;
		node->result = VAR_INT;
		node->value = t.GetInt();
		t.Next();
		return node;
	}
	else if(t.IsKeyword())
	{
		int group = (ParseGroup)t.GetKeywordGroup();
		if(group == G_FUNCTION)
		{
			// function call
			int func_id = t.GetKeywordId();
			Function& f = functions[func_id];
			t.Next();
			t.AssertSymbol('(');
			t.Next();
			ParseNode* node = new ParseNode;
			node->type = NODE_FUNC;
			node->result = f.result;
			node->value = func_id;
			if(f.arg == VAR_VOID)
			{
				t.AssertSymbol(')');
				t.Next();
			}
			else
			{
				ParseNode* arg = ParseStatement();
				if(arg->result != f.arg)
					t.Throw("Invalid argument type %s for function '%s' (expected %s).", var_name[arg->result], f.name.c_str(), var_name[f.arg]);
				node->nodes.push_back(arg);
				t.AssertSymbol(')');
				t.Next();
			}
			return node;
		}
		else if(group == G_VAR)
		{
			// variable
			int var_id = t.GetKeywordId();
			ParseNode* node = new ParseNode;
			node->type = NODE_VAR;
			node->value = var_id;
			node->result = pvars[var_id].type;
			t.Next();
			return node;
		}
		else
			t.Unexpected();
	}
	else if(t.IsString())
	{
		// string
		ParseNode* node = new ParseNode;
		node->type = NODE_STRING;
		node->result = VAR_STRING;
		node->value = pstrs.size();
		pstrs.push_back(new String(t.GetString().c_str()));
		t.Next();
		return node;
	}
	else
		t.Unexpected();
}

ParseNode* ScriptEngine::ParseStatement()
{
	ParseNode* left = ParseItem();
	while(true)
	{
		if(t.IsSymbol('+'))
		{
			t.Next();
			ParseNode* right = ParseItem();
			if(left->result == VAR_STRING || right->result == VAR_STRING)
			{
				ParseNode* node = new ParseNode;
				node->type = NODE_OP;
				node->value = ADD;
				node->result = VAR_STRING;
				if(left->result == VAR_STRING)
					node->nodes.push_back(left);
				else
				{
					ParseNode* cast = new ParseNode;
					cast->type = NODE_TO_STR;
					cast->nodes.push_back(left);
					node->nodes.push_back(cast);
				}
				if(right->result == VAR_STRING)
					node->nodes.push_back(right);
				else
				{
					ParseNode* cast = new ParseNode;
					cast->type = NODE_TO_STR;
					cast->nodes.push_back(right);
					node->nodes.push_back(cast);
				}
				left = node;
			}
			else if(left->result == VAR_INT && right->result == VAR_INT)
			{
				ParseNode* node = new ParseNode;
				node->type = NODE_OP;
				node->value = ADD;
				node->result = VAR_INT;
				node->nodes.push_back(left);
				node->nodes.push_back(right);
				left = node;
			}
			else
				t.Throw("Can't add %s and %s.", var_name[left->result], var_name[right->result]);
		}
		else
			return left;
	}
}

ParseNode* ScriptEngine::ParseLine()
{
	ParseNode* node;
	if(t.IsKeywordGroup(G_TYPE))
	{
		// variable declaration
		int index = (int)pvars.size();
		ParseVar pvar;
		pvar.type = (VarType)t.GetKeywordId();
		t.Next();
		pvar.name = t.MustGetItem();
		t.Next();
		pvar.index = index;
		pvars.push_back(pvar);

		string* name = new string(pvar.name);
		dynamic_keywords.push_back(name);
		t.AddKeyword(name->c_str(), pvar.index, G_VAR);

		if(t.IsSymbol('='))
		{
			t.Next();
			ParseNode* child = ParseStatement();
			if(child->result != pvar.type)
				t.Throw("Can't assign result type %s to variable of type %s (%s).", var_name[child->result], var_name[pvar.type], pvar.name.c_str());
			node = new ParseNode;
			node->type = NODE_ASSIGN;
			node->result = VAR_VOID;
			node->value = pvar.index;
			node->nodes.push_back(child);
		}
		else
			node = NULL;
	}
	else
	{
		// statement
		node = ParseStatement();
	}
	t.AssertSymbol(';');
	return node;
}

void ScriptEngine::RunNode(ParseNode* node, vector<byte>& code)
{
	for(ParseNode* child : node->nodes)
		RunNode(child, code);

	switch(node->type)
	{
	case NODE_INT:
		code.push_back(PUSH_INT);
		code.push_back(node->value & 0xFF);
		code.push_back((node->value & 0xFF00) >> 8);
		code.push_back((node->value & 0xFF0000) >> 16);
		code.push_back((node->value & 0xFF000000) >> 24);
		break;
	case NODE_STRING:
		code.push_back(PUSH_STR);
		code.push_back(node->value);
		break;
	case NODE_FUNC:
		code.push_back(CALL);
		code.push_back(node->value);
		break;
	case NODE_VAR:
		code.push_back(PUSH_LOCAL);
		code.push_back(node->value);
		break;
	case NODE_ASSIGN:
		code.push_back(SET_LOCAL);
		code.push_back(node->value);
		break;
	case NODE_OP:
		code.push_back(node->value);
		break;
	case NODE_TO_STR:
		code.push_back(TO_STR);
		break;
	}
}

void ScriptEngine::ParseAndRun(cstring code)
{
	assert(code);

	vector<ParseNode*> nodes;

	try
	{
		t.FromString(code);

		while(t.Next())
		{
			ParseNode* node = ParseLine();
			if(node)
				nodes.push_back(node);
		}
	}
	catch(Tokenizer::Exception& e)
	{
		printf("ERROR: %s", e.ToString());
		_getch();
		return;
	}

	vector<byte> c;
	if(!pvars.empty())
	{
		c.push_back(LOCALS);
		c.push_back(pvars.size());
	}
	for(ParseNode* node : nodes)
	{
		RunNode(node, c);
		if(node->result != VAR_VOID)
			c.push_back(POP);
	}
	c.push_back(RET);

	/*byte* bs = &c[0];
	byte* be = bs + c.size();
	while(bs < be)
	{
		switch((Op)*bs++)
		{
		case ADD:
			printf("ADD\n");
			break;
		case TO_STR:
			printf("TO_STR\n");
			break;
		case LOCALS:
			{
				byte b = *bs++;
				printf("LOCALS %d\n", b);
			}
			break;
		case SET_LOCAL:
			{
				byte b = *bs++;
				printf("SET_LOCAL %d\n", b);
			}
			break;
		case PUSH_LOCAL:
			{
				byte b = *bs++;
				printf("PUSH_LOCAL %d\n", b);
			}
			break;
		case PUSH_INT:
			{
				int i = *(int*)bs;
				bs += 4;
				printf("PUSH_INT %d\n", i);
			}
			break;
		case PUSH_STR:
			{
				byte b = *bs++;
				printf("PUSH_STR %d\n", b);
			}
			break;
		case POP:
			printf("POP\n");
			break;
		case CALL:
			{
				byte b = *bs++;
				printf("CALL %d\n", b);
			}
			break;
		case RET:
			printf("RET\n");
		}
	}
	_getch();
	system("cls");*/

	RunCode(c, pstrs);
}

void ScriptEngine::ParseAndRunFile(cstring filename)
{
	string str;
	if(!LoadFileToString(filename, str))
		return;
	ParseAndRun(str.c_str());
}

int main()
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	vector<byte> bcode = { 
		LOCALS, 3,
		PUSH_STR, 0,
		CALL, 3,
		CALL, 1,
		SET_LOCAL, 0,
		PUSH_STR, 1,
		CALL, 3,
		CALL, 1,
		SET_LOCAL, 1,
		PUSH_LOCAL, 0,
		PUSH_LOCAL, 1,
		ADD,
		SET_LOCAL, 2,
		PUSH_LOCAL, 0,
		TO_STR,
		PUSH_STR, 2,
		ADD,
		PUSH_LOCAL, 1,
		TO_STR,
		ADD,
		PUSH_STR, 3,
		ADD,
		PUSH_LOCAL, 2,
		TO_STR,
		ADD,
		CALL, 3,
		CALL, 2,
		RET
	};

	vector<String*> strs = {
		new String("Podaj a: "),
		new String("Podaj b: "),
		new String(" + "),
		new String(" = ")
	};

	ScriptEngine script;
	script.InitFunctions();
	script.ParseAndRunFile("../doc/script/3.txt");
	//script.RunCode(bcode, strs);

	return 0;
}
