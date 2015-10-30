#include "Pch.h"
#include "Base.h"

#include <conio.h>
#include <iostream>

enum Op
{
	INVOP,

	ADD,
	SUB,
	MUL,
	DIV,
	MOD,
	CAST,
	IFE,
	IFNE,
	IFG,
	IFGE,
	IFL,
	IFLE,
	AND,
	OR,
	NOT,
	B_NOT,
	B_AND,
	B_OR,
	B_XOR,
	L_SHIFT,
	R_SHIFT,
	NEG,

	LOCALS,
	SET_LOCAL,
	PUSH_LOCAL,
	PUSH_TRUE,
	PUSH_FALSE,
	PUSH_INT,
	PUSH_FLOAT,
	PUSH_STR,
	POP,

	CALL,
	JMP,
	IFJMP,
	RET
};

enum VarType
{
	VAR_VOID,
	VAR_BOOL,
	VAR_INT,
	VAR_FLOAT,
	VAR_STRING
};

cstring var_name[] = {
	"void",
	"bool",
	"int",
	"float",
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
		bool _bool;
		int _int;
		float _float;
		String* _str;
	};

	inline Var() {}
	inline explicit Var(bool _bool) : type(VAR_BOOL), _bool(_bool) {}
	inline explicit Var(int _int) : type(VAR_INT), _int(_int) {}
	inline explicit Var(float _float) : type(VAR_FLOAT), _float(_float) {}
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

struct Function
{
	string name;
	int index;
	VarType result, arg;
};

enum ParseNodeType
{
	NODE_BOOL,
	NODE_INT,
	NODE_FLOAT,
	NODE_STRING,
	NODE_FUNC,
	NODE_VAR,
	NODE_ASSIGN,
	NODE_OP,
	NODE_CAST,
	NODE_IF,
	NODE_BLOCK
};

struct ParseNode
{
	ParseNodeType type;
	union
	{
		int value;
		float fvalue;
	};
	VarType result;
	vector<ParseNode*> nodes;
};

enum ParseGroup
{
	G_TYPE,
	G_VAR,
	G_FUNCTION,
	G_KEYWORD
};

enum Keyword
{
	K_IF,
	K_ELSE,
	K_TRUE,
	K_FALSE
};

enum Symbol
{
	S_INVALID,
	S_ADD, // +
	S_SUB, // -
	S_MUL, // *
	S_DIV, // /
	S_MOD, // %
	S_EQUAL, // ==
	S_NOT_EQUAL, // !=
	S_GREATER, // >
	S_GREATER_EQUAL, // >=
	S_LESS, // <
	S_LESS_EQUAL, // <=
	S_ASSIGN, // ==
	S_AND, // &&
	S_OR, // ||
	S_NOT, // !
	S_ADD_COMP, // +=
	S_SUB_COMP, // -=
	S_MUL_COMP, // *=
	S_DIV_COMP, // /=
	S_MOD_COMP, // %=
	S_B_NOT, // ~
	S_B_AND, // &
	S_B_OR, // |
	S_B_XOR, // ^
	S_L_SHIFT, // <<
	S_R_SHIFT, // >>
	S_B_AND_COMP, // &=
	S_B_OR_COMP, // |=
	S_B_XOR_COMP, // ^=
	S_L_SHIFT_COMP, // <<=
	S_R_SHIFT_COMP, // >>=
	S_UNARY_PLUS, // +
	S_UNARY_MINUS, // -
};

enum SymbolType
{
	ST_NORMAL,
	ST_SINGLE,
	ST_COMPOUND
};

struct SymbolInfo
{
	Symbol symbol;
	cstring name;
	Op op;
	int priority;
	bool left_to_right;
	SymbolType type;
};

SymbolInfo symbol_info[] = {
	S_INVALID, "invalid", INVOP, 0, false, ST_NORMAL,
	S_ADD, "add", ADD, 6, true, ST_NORMAL,
	S_SUB, "subtract", SUB, 6, true, ST_NORMAL,
	S_MUL, "multiply", MUL, 5, true, ST_NORMAL,
	S_DIV, "divide", DIV, 5, true, ST_NORMAL,
	S_MOD, "modulo", MOD, 5, true, ST_NORMAL,
	S_EQUAL, "equal", IFE, 9, true, ST_NORMAL,
	S_NOT_EQUAL, "not equal", IFNE, 9, true, ST_NORMAL,
	S_GREATER, "greater", IFG, 8, true, ST_NORMAL,
	S_GREATER_EQUAL, "greater or eqal", IFGE, 8, true, ST_NORMAL,
	S_LESS, "less", IFL, 8, true, ST_NORMAL,
	S_LESS_EQUAL, "less or equal", IFLE, 8, true, ST_NORMAL,
	S_ASSIGN, "assign", INVOP, 15, false, ST_NORMAL,
	S_AND, "and", AND, 13, true, ST_NORMAL,
	S_OR, "or", OR, 14, true, ST_NORMAL,
	S_NOT, "not", NOT, 3, false, ST_SINGLE,
	S_ADD_COMP, "add assign", ADD, 15, true, ST_COMPOUND,
	S_SUB_COMP, "subtract assign", SUB, 15, true, ST_COMPOUND,
	S_MUL_COMP, "multiply assign", MUL, 15, true, ST_COMPOUND,
	S_DIV_COMP, "divide assign", DIV, 15, true, ST_COMPOUND,
	S_MOD_COMP, "modulo assign", MOD, 15, true, ST_COMPOUND,
	S_B_NOT, "bitwise not", B_NOT, 3, false, ST_SINGLE,
	S_B_AND, "bitwise and", B_AND, 10, true, ST_NORMAL,
	S_B_OR, "bitwise or", B_OR, 12, true, ST_NORMAL,
	S_B_XOR, "bitwise xor", B_XOR, 11, true, ST_NORMAL,
	S_L_SHIFT, "left shift", L_SHIFT, 7, true, ST_NORMAL,
	S_R_SHIFT, "right shift", R_SHIFT, 7, true, ST_NORMAL,
	S_B_AND_COMP, "bitwise and assign", B_AND, 15, false, ST_COMPOUND,
	S_B_OR_COMP, "bitwise or assign", B_OR, 15, false, ST_COMPOUND,
	S_B_XOR_COMP, "bitwise xor assign", B_XOR, 15, false, ST_COMPOUND,
	S_L_SHIFT_COMP, "left shift assign", L_SHIFT, 15, false, ST_COMPOUND,
	S_R_SHIFT_COMP, "right shift assign", R_SHIFT, 15, false, ST_COMPOUND,
	S_UNARY_PLUS, "unary plus", INVOP, 3, false, ST_SINGLE,
	S_UNARY_MINUS, "unary minus", NEG, 3, false, ST_SINGLE,
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
	vector<Function> functions;
	Tokenizer t;
	vector<String*> pstrs;
	vector<ParseVar> pvars;

	void InitFunctions();

	void RunCode(vector<byte>& code, vector<String*>& strs);

	ParseNode* ParseCast(ParseNode* node, VarType type, bool safe = true);
	bool PeekItem();
	ParseNode* ParseItem();
	ParseNode* ParseStatement();
	ParseNode* ParseBlock();
	ParseNode* ParseLine();
	ParseNode* ParseTopLine();
	void RunNode(ParseNode* node, vector<byte>& code);

	void ParseAndRun(cstring code);
	void ParseAndRunFile(cstring filename);
	void Decode(vector<byte>& code);
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

	t.AddKeyword("bool", VAR_BOOL, G_TYPE);
	t.AddKeyword("int", VAR_INT, G_TYPE);
	t.AddKeyword("float", VAR_FLOAT, G_TYPE);
	t.AddKeyword("string", VAR_STRING, G_TYPE);

	t.AddKeyword("if", K_IF, G_KEYWORD);
	t.AddKeyword("else", K_ELSE, G_KEYWORD);
	t.AddKeyword("true", K_TRUE, G_KEYWORD);
	t.AddKeyword("false", K_FALSE, G_KEYWORD);
}

void ScriptEngine::RunCode(vector<byte>& code, vector<String*>& strs)
{
	byte* cstart = code.data();
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
		case INVOP:
			assert(0);
			break;
		case ADD:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_FLOAT || l.type == VAR_STRING);
				if(l.type == VAR_INT)
					l._int += r._int;
				else if(l.type == VAR_FLOAT)
					l._float += r._float;
				else
				{
					String* s = new String(l._str->s + r._str->s);
					l.Release();
					r.Release();
					l._str = s;
				}
			}
			break;
		case SUB:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_FLOAT);
				if(l.type == VAR_INT)
					l._int -= r._int;
				else
					l._float -= r._float;
			}
			break;
		case MUL:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_FLOAT);
				if(l.type == VAR_INT)
					l._int *= r._int;
				else
					l._float *= r._float;
			}
			break;
		case DIV:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_FLOAT);
				if(l.type == VAR_INT)
				{
					if(r._int != 0)
						l._int /= r._int;
					else
						l._int = 0;
				}
				else
				{
					if(r._float != 0)
						l._float *= r._float;
					else
						l._float = 0;
				}
			}
			break;
		case MOD:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT || l.type == VAR_FLOAT);
				if(l.type == VAR_INT)
				{
					if(r._int != 0)
						l._int %= r._int;
					else
						l._int = 0;
				}
				else
				{
					if(r._float != 0)
						l._float = fmod(l._float, r._float);
					else
						l._float = 0;
				}
			}
			break;
		case CAST:
			{
				assert(!stack.empty());
				assert(c + 1 < cend);
				VarType type = (VarType)*c;
				++c;
				Var& v = stack.back();
				switch(type)
				{
				case VAR_BOOL:
					switch(v.type)
					{
					case VAR_BOOL:
						break;
					case VAR_INT:
						v._bool = (v._int != 0);
						v.type = VAR_BOOL;
						break;
					case VAR_FLOAT:
						v._bool = (v._float != 0);
						v.type = VAR_BOOL;
						break;
					case VAR_STRING:
						{
							bool result = false;
							if(_stricmp(v._str->s.c_str(), "true") == 0)
								result = true;
							else
							{
								__int64 i;
								float f;
								StringToNumber(v._str->s.c_str(), i, f);
								result = (i != 0);
							}
							v.Release();
							v._bool = result;
							v.type = VAR_BOOL;
						}
						break;
					default:
						assert(0);
						break;
					}
					break;
				case VAR_INT:
					switch(v.type)
					{
					case VAR_BOOL:
						v._int = (v._bool ? 1 : 0);
						v.type = VAR_BOOL;
						break;
					case VAR_INT:
						break;
					case VAR_FLOAT:
						v._int = (int)v._float;
						v.type = VAR_INT;
						break;
					case VAR_STRING:
						{
							__int64 result;
							float fresult;
							StringToNumber(v._str->s.c_str(), result, fresult);
							v.Release();
							v._int = (int)result;
							v.type = VAR_INT;
						}
						break;
					default:
						assert(0);
						break;
					}
					break;
				case VAR_FLOAT:
					switch(v.type)
					{
					case VAR_BOOL:
						v._float = (v._bool ? 1.f : 0);
						v.type = VAR_FLOAT;
						break;
					case VAR_INT:
						v._float = (float)v._int;
						v.type = VAR_FLOAT;
						break;
					case VAR_FLOAT:
						break;
					case VAR_STRING:
						{
							__int64 result;
							float fresult;
							StringToNumber(v._str->s.c_str(), result, fresult);
							v.Release();
							v._float = fresult;
							v.type = VAR_FLOAT;
						}
						break;
					default:
						assert(0);
						break;
					}
					break;
				case VAR_STRING:
					switch(v.type)
					{
					case VAR_BOOL:
						v._str = new String(v._bool ? "true" : "false");
						v.type = VAR_STRING;
						break;
					case VAR_INT:
						v._str = new String(Format("%d", v._int));
						v.type = VAR_STRING;
						break;
					case VAR_FLOAT:
						v._str = new String(Format("%g", v._float));
						v.type = VAR_STRING;
						break;
					case VAR_STRING:
						break;
					default:
						assert(0);
						break;
					}
					break;
				default:
					assert(0);
					break;
				}
			}
			break;
		case IFE:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_BOOL:
					result = (l._bool == r._bool);
					break;
				case VAR_INT:
					result = (l._int == r._int);
					break;
				case VAR_FLOAT:
					result = (l._float == r._float);
					break;
				case VAR_STRING:
					result = (l._str->s == r._str->s);
					l.Release();
					r.Release();
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case IFNE:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_BOOL:
					result = (l._bool != r._bool);
					break;
				case VAR_INT:
					result = (l._int != r._int);
					break;
				case VAR_FLOAT:
					result = (l._float != r._float);
					break;
				case VAR_STRING:
					result = (l._str->s != r._str->s);
					l.Release();
					r.Release();
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case IFG:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_INT:
					result = (l._int > r._int);
					break;
				case VAR_FLOAT:
					result = (l._float > r._float);
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case IFGE:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_INT:
					result = (l._int >= r._int);
					break;
				case VAR_FLOAT:
					result = (l._float >= r._float);
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case IFL:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_INT:
					result = (l._int < r._int);
					break;
				case VAR_FLOAT:
					result = (l._float < r._float);
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case IFLE:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				bool result;
				switch(l.type)
				{
				case VAR_INT:
					result = (l._int <= r._int);
					break;
				case VAR_FLOAT:
					result = (l._float <= r._float);
					break;
				default:
					assert(0);
					break;
				}
				l._bool = result;
				l.type = VAR_BOOL;
			}
			break;
		case AND:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_BOOL);
				l._bool = (l._bool && r._bool);
			}
			break;
		case OR:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_BOOL);
				l._bool = (l._bool || r._bool);
			}
			break;
		case NOT:
			{
				assert(stack.size() >= 1u);
				Var& l = stack.back();
				assert(l.type == VAR_BOOL);
				l._bool = !l._bool;
			}
			break;
		case B_NOT:
			{
				assert(stack.size() >= 1u);
				Var& l = stack.back();
				assert(l.type == VAR_INT);
				l._int = ~l._int;
			}
			break;
		case B_AND:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT);
				l._int &= r._int;
			}
			break;
		case B_OR:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT);
				l._int |= r._int;
			}
			break;
		case B_XOR:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT);
				l._int ^= r._int;
			}
			break;
		case L_SHIFT:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT);
				l._int <<= r._int;
			}
			break;
		case R_SHIFT:
			{
				assert(stack.size() >= 2u);
				Var r = stack.back();
				stack.pop_back();
				Var& l = stack.back();
				assert(l.type == r.type);
				assert(l.type == VAR_INT);
				l._int >>= r._int;
			}
			break;
		case NEG:
			{
				assert(stack.size() >= 1u);
				Var& l = stack.back();
				assert(l.type == VAR_INT || l.type == VAR_FLOAT);
				if(l.type == VAR_INT)
					l._int = -l._int;
				else
					l._float = -l._float;
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
			}
			break;
		case PUSH_TRUE:
			stack.push_back(Var(true));
			break;
		case PUSH_FALSE:
			stack.push_back(Var(false));
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
		case PUSH_FLOAT:
			{
				assert(c + 4 < cend);
				stack.push_back(Var(*(float*)c));
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
			assert(!stack.empty());
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
		case JMP:
			{
				assert(c + 2 < cend);
				int off = *(short*)c;
				c += 2;
				byte* new_pos = c + off;
				assert(c >= cstart && c < cend);
				c = new_pos;
			}
			break;
		case IFJMP:
			{
				assert(c + 2 < cend);
				assert(!stack.empty());
				Var& v = stack.back();
				assert(v.type == VAR_BOOL);
				int off = *(short*)c;
				c += 2;
				byte* new_pos = c + off;
				assert(c >= cstart && c < cend);
				if(!v._bool)
					c = new_pos;
				stack.pop_back();
			}
			break;
		case RET:
			assert(stack.empty());
			return;
		default:
			assert(0);
			break;
		}
	}
}

ParseNode* ScriptEngine::ParseCast(ParseNode* node, VarType type, bool safe)
{
	assert(node);

	if(node->result == type)
		return node;

	int cast = -1; // -1 - no, 0 - unsafe, 1 - safe

	switch(type)
	{
	case VAR_BOOL:
		switch(node->result)
		{
		case VAR_INT:
		case VAR_FLOAT:
			cast = 1;
			break;
		case VAR_STRING:
			cast = 0;
			break;
		}
	case VAR_INT:
		switch(node->result)
		{
		case VAR_BOOL:
		case VAR_FLOAT:
			cast = 0;
			break;
		case VAR_STRING:
			cast = 0;
			break;
		}
		break;
	case VAR_FLOAT:
		switch(node->result)
		{
		case VAR_BOOL:
		case VAR_INT:
			cast = 1;
			break;
		case VAR_STRING:
			cast = 0;
			break;
		}
		break;
	case VAR_STRING:
		switch(node->result)
		{
		case VAR_BOOL:
		case VAR_INT:
		case VAR_FLOAT:
			cast = 1;
			break;
		}
		break;
	}

	if(cast == -1 || (cast == 0 && safe))
		return NULL;
	else
	{
		ParseNode* c = new ParseNode;
		c->type = NODE_CAST;
		c->value = type;
		c->result = type;
		c->nodes.push_back(node);
		return c;
	}
}

bool ScriptEngine::PeekItem()
{
	if(t.IsInt() || t.IsFloat() || t.IsString())
		return true;
	else if(t.IsKeyword())
	{
		int group = (ParseGroup)t.GetKeywordGroup();
		if(group == G_VAR || group == G_FUNCTION)
			return true;
		else if(group == G_KEYWORD)
		{
			int id = (Keyword)t.GetKeywordId();
			if(id == K_TRUE || id == K_FALSE)
				return true;
		}
	}
	return false;
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
	else if(t.IsFloat())
	{
		// float literal
		ParseNode* node = new ParseNode;
		node->type = NODE_FLOAT;
		node->result = VAR_FLOAT;
		node->fvalue = t.GetFloat();
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
				ParseNode* result = ParseStatement();
				ParseNode* arg = ParseCast(result, f.arg);
				if(!arg)
					t.Throw("Invalid argument 1 of type %s for function '%s' (expected %s).", var_name[result->result], f.name.c_str(), var_name[f.arg]);
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
	else if(t.IsKeyword(K_TRUE, G_KEYWORD) || t.IsKeyword(K_FALSE, G_KEYWORD))
	{
		// bool literal
		ParseNode* node = new ParseNode;
		node->type = NODE_BOOL;
		node->result = VAR_BOOL;
		node->value = (t.IsKeyword(K_TRUE, G_KEYWORD) ? 1 : 0);
		t.Next();
		return node;
	}
	else
		t.Unexpected();
}

VarType CanDoOp(VarType left, VarType right, Symbol symbol, VarType& cast_type)
{
	if(left == VAR_VOID || right == VAR_VOID)
		return VAR_VOID;
	cast_type = VAR_VOID;
	switch(symbol)
	{
	case S_ADD:
	case S_ADD_COMP:
		if(left == VAR_STRING || right == VAR_STRING)
			return VAR_STRING;
		else if(left == VAR_FLOAT || right == VAR_FLOAT)
			return VAR_FLOAT;
		else
			return VAR_INT;
	case S_SUB:
	case S_MUL:
	case S_DIV:
	case S_MOD:
	case S_SUB_COMP:
	case S_MUL_COMP:
	case S_DIV_COMP:
	case S_MOD_COMP:
		if(left == VAR_STRING || right == VAR_STRING)
			return VAR_VOID;
		else if(left == VAR_FLOAT || right == VAR_FLOAT)
			return VAR_FLOAT;
		else
			return VAR_INT;
	case S_EQUAL:
	case S_NOT_EQUAL:
		if(left == VAR_STRING || right == VAR_STRING)
			cast_type = VAR_STRING;
		else if(left == VAR_FLOAT || right == VAR_FLOAT)
			cast_type = VAR_FLOAT;
		else if(left == VAR_INT || right == VAR_INT)
			cast_type = VAR_INT;
		return VAR_BOOL;
	case S_GREATER:
	case S_GREATER_EQUAL:
	case S_LESS:
	case S_LESS_EQUAL:
		if(left == VAR_STRING || right == VAR_STRING)
			return VAR_VOID;
		else if(left == VAR_FLOAT || right == VAR_FLOAT)
			cast_type = VAR_FLOAT;
		else
			cast_type = VAR_INT;
		return VAR_BOOL;
	case S_ASSIGN:
		cast_type = left;
		return left;
	case S_AND:
	case S_OR:
		return VAR_BOOL;
	case S_NOT:
		if(left == VAR_STRING)
			return VAR_VOID;
		return VAR_BOOL;
	case S_B_NOT:
		if(left == VAR_STRING)
			return VAR_VOID;
		return VAR_INT;
	case S_B_AND:
	case S_B_OR:
	case S_B_XOR:
	case S_L_SHIFT:
	case S_R_SHIFT:
	case S_B_AND_COMP:
	case S_B_OR_COMP:
	case S_B_XOR_COMP:
	case S_L_SHIFT_COMP:
	case S_R_SHIFT_COMP:
		if(left == VAR_STRING || right == VAR_STRING)
			return VAR_VOID;
		return VAR_INT;
	case S_UNARY_PLUS:
	case S_UNARY_MINUS:
		if(left == VAR_STRING)
			return VAR_VOID;
		return left;
	default:
		assert(0);
		return VAR_VOID;
	}
}

struct NodeOrSymbol
{
	union
	{
		ParseNode* node;
		Symbol symbol;
	};
	bool is_symbol;

	inline explicit NodeOrSymbol(ParseNode* node) : node(node), is_symbol(false) {}
	inline explicit NodeOrSymbol(Symbol symbol) : symbol(symbol), is_symbol(true) {}
};

enum OnLeft
{
	LEFT_NONE,
	LEFT_SYMBOL,
	LEFT_NODE
};

ParseNode* ScriptEngine::ParseStatement()
{
	vector<NodeOrSymbol> rpn_exit;
	vector<Symbol> rpn_stack;
	OnLeft left = LEFT_NONE;

	//-------------------------------------------
	// convert infix to reverse polish notation
	while(true)
	{
		if(PeekItem())
		{
			if(left == LEFT_NONE || left == LEFT_SYMBOL)
			{
				rpn_exit.push_back(NodeOrSymbol(ParseItem()));
				left = LEFT_NODE;
			}
			else
				t.Unexpected();
		}
		else if(t.IsSymbol())
		{
			Symbol symbol = S_INVALID;

			switch(t.GetSymbol())
			{
			case '+':
				if(t.PeekSymbol('='))
				{
					symbol = S_ADD_COMP;
					t.NextChar();
				}
				else
				{
					if(left == LEFT_SYMBOL || left == LEFT_NONE)
						symbol = S_UNARY_PLUS;
					else
						symbol = S_ADD;
				}
				break;
			case '-':
				if(t.PeekSymbol('='))
				{
					symbol = S_SUB_COMP;
					t.NextChar();
				}
				else
				{
					if(left == LEFT_SYMBOL || left == LEFT_NONE)
						symbol = S_UNARY_MINUS;
					else
						symbol = S_SUB;
				}
				break;
			case '*':
				if(t.PeekSymbol('='))
				{
					symbol = S_MUL_COMP;
					t.NextChar();
				}
				else
					symbol = S_MUL;
				break;
			case '/':
				if(t.PeekSymbol('='))
				{
					symbol = S_DIV_COMP;
					t.NextChar();
				}
				else
					symbol = S_DIV;
				break;
			case '%':
				if(t.PeekSymbol('='))
				{
					symbol = S_MOD_COMP;
					t.NextChar();
				}
				else
					symbol = S_MOD;
				break;
			case '=':
				if(t.PeekChar('='))
				{
					symbol = S_EQUAL;
					t.NextChar();
				}
				else
					symbol = S_ASSIGN;
				break;
			case '!':
				if(t.PeekChar('='))
				{
					symbol = S_NOT_EQUAL;
					t.NextChar();
				}
				else
					symbol = S_NOT;
				break;
			case '>':
				if(t.IsSymbol('>'))
				{
					t.NextChar();
					if(t.PeekChar('='))
					{
						symbol = S_R_SHIFT_COMP;
						t.NextChar();
					}
					else
						symbol = S_R_SHIFT;
				}
				else if(t.PeekChar('='))
				{
					symbol = S_GREATER_EQUAL;
					t.NextChar();
				}
				else
					symbol = S_GREATER;
				break;
			case '<':
				if(t.PeekChar('<'))
				{
					t.NextChar();
					if(t.PeekChar('='))
					{
						symbol = S_L_SHIFT_COMP;
						t.NextChar();
					}
					else
						symbol = S_L_SHIFT;
				}
				else if(t.PeekChar('='))
				{
					symbol = S_LESS_EQUAL;
					t.NextChar();
				}
				else
					symbol = S_LESS;
				break;
			case '&':
				if(t.PeekChar('&'))
				{
					symbol = S_AND;
					t.NextChar();
				}
				else if(t.PeekChar('='))
				{
					symbol = S_B_AND_COMP;
					t.NextChar();
				}
				else
					symbol = S_B_AND;
				break;
			case '|':
				if(t.PeekChar('|'))
				{
					symbol = S_OR;
					t.NextChar();
				}
				else if(t.PeekChar('='))
				{
					symbol = S_B_OR_COMP;
					t.NextChar();
				}
				else
					symbol = S_B_OR;
				break;
			case '~':
				symbol = S_B_NOT;
				break;
			case '^':
				if(t.PeekChar('='))
				{
					symbol = S_B_XOR_COMP;
					t.NextChar();
				}
				else
					symbol = S_B_XOR;
				break;
			}

			if(symbol == S_INVALID)
				break;

			SymbolInfo& s_info = symbol_info[symbol];
			if(left != LEFT_NODE && s_info.left_to_right && s_info.type != ST_SINGLE)
				t.Unexpected();

			while(!rpn_stack.empty())
			{
				Symbol symbol2 = rpn_stack.back();
				SymbolInfo& s2_info = symbol_info[symbol2];
				bool ok = false;
				if(s_info.left_to_right)
				{
					if(s_info.priority >= s2_info.priority)
						ok = true;
				}
				else
				{
					if(s_info.priority > s2_info.priority)
						ok = true;
				}
				if(ok)
				{
					rpn_exit.push_back(NodeOrSymbol(symbol2));
					rpn_stack.pop_back();
				}
				else
					break;
			}

			rpn_stack.push_back(symbol);
			left = LEFT_SYMBOL;
			t.Next();
		}
	}

	if(left == LEFT_NONE || left == LEFT_SYMBOL)
		t.Unexpected();

	while(!rpn_stack.empty())
	{
		Symbol s = rpn_stack.back();
		//if(s == S_LPAR)
		//	t.Throw("Missing closing parenthesis.");
		rpn_exit.push_back(NodeOrSymbol(s));
		rpn_stack.pop_back();
	}

	//-------------------------------------------
	// convert reverse polish notation to parse nodes tree
	vector<ParseNode*> nodes;
	for(NodeOrSymbol& item : rpn_exit)
	{
		if(!item.is_symbol)
			nodes.push_back(item.node);
		else
		{
			SymbolInfo& sinfo = symbol_info[item.symbol];

			if(sinfo.type != ST_SINGLE)
			{
				if(nodes.size() < 2u)
					t.Throw("Failed to parse expression tree.");
				ParseNode* right = nodes.back();
				nodes.pop_back();
				ParseNode* left = nodes.back();
				nodes.pop_back();

				if(sinfo.symbol == S_ASSIGN)
				{
					if(left->type != NODE_VAR)
						t.Throw("Can't assign, left is not variable.");
					ParseVar& pvar = pvars[left->value];
					ParseNode* cast = ParseCast(right, pvar.type);
					if(cast == NULL)
						t.Throw("Can't assign type %s to variable '%s' of type %s.", var_name[right->result], pvar.name.c_str(), pvar.name.c_str(), var_name[pvar.type]);
					ParseNode* node = new ParseNode;
					node->type = NODE_ASSIGN;
					node->result = pvar.type;
					node->value = pvar.index;
					node->nodes.push_back(cast);
					nodes.push_back(node);
				}
				else if(sinfo.type == ST_COMPOUND)
				{
					if(left->type != NODE_VAR)
						t.Throw("Can't %s, left is not variable.", sinfo.name);
					ParseVar& pvar = pvars[left->value];
					VarType cast_type;
					VarType result = CanDoOp(left->result, right->result, item.symbol, cast_type);
					if(result == VAR_VOID)
						t.Throw("Invalid operator '%s' for types %s and %s.", sinfo.name, var_name[left->result], var_name[right->result]);
					if(cast_type == VAR_VOID)
						cast_type = result;
					ParseNode* op = new ParseNode;
					op->type = NODE_OP;
					op->value = sinfo.op;
					op->result = result;
					op->nodes.push_back(ParseCast(left, cast_type));
					op->nodes.push_back(ParseCast(right, cast_type));
					ParseNode* assign = new ParseNode;
					assign->type = NODE_ASSIGN;
					assign->value = left->value;
					assign->result = pvar.type;
					assign->nodes.push_back(op);
					nodes.push_back(assign);
				}
				else
				{
					VarType cast_type;
					VarType result = CanDoOp(left->result, right->result, item.symbol, cast_type);
					if(result == VAR_VOID)
						t.Throw("Invalid operator '%s' for types %s and %s.", sinfo.name, var_name[left->result], var_name[right->result]);
					if(cast_type == VAR_VOID)
						cast_type = result;
					ParseNode* node = new ParseNode;
					node->type = NODE_OP;
					node->value = sinfo.op;
					node->result = result;
					node->nodes.push_back(ParseCast(left, cast_type));
					node->nodes.push_back(ParseCast(right, cast_type));
					nodes.push_back(node);
				}
			}
			else
			{
				if(nodes.size() < 1u)
					t.Throw("Failed to parse expression tree.");
				ParseNode* left = nodes.back();
				nodes.pop_back();

				if(sinfo.op != INVOP)
				{
					VarType cast_type;
					VarType result = CanDoOp(left->result, left->result, item.symbol, cast_type);
					if(result == VAR_VOID)
						t.Throw("Invalid operator '%s' for type %s.", sinfo.name, var_name[left->result]);
					if(cast_type == VAR_VOID)
						cast_type = result;
					ParseNode* node = new ParseNode;
					node->type = NODE_OP;
					node->value = sinfo.op;
					node->result = result;
					node->nodes.push_back(ParseCast(left, cast_type));
					nodes.push_back(node);
				}
				else
					nodes.push_back(left);
			}
		}
	}

	if(nodes.size() != 1u)
		t.Throw("Broken expression tree.");

	return nodes[0];
}

ParseNode* ScriptEngine::ParseBlock()
{
	ParseNode* block = new ParseNode;
	block->type = NODE_BLOCK;
	block->result = VAR_VOID;
	if(t.IsSymbol('{'))
	{
		t.Next();
		while(true)
		{
			if(t.IsSymbol('}'))
			{
				t.Next();
				break;
			}
			block->nodes.push_back(ParseTopLine());
		}
		t.Next();
	}
	else
		block->nodes.push_back(ParseLine());
	return block;
}

ParseNode* ScriptEngine::ParseLine()
{
	if(t.IsKeyword(K_IF, G_KEYWORD))
	{
		// if
		ParseNode* node = new ParseNode;
		node->type = NODE_IF;
		node->result = VAR_VOID;
		t.Next();
		t.AssertSymbol('(');
		t.Next();
		ParseNode* cond = ParseStatement();
		ParseNode* cast = ParseCast(cond, VAR_BOOL);
		if(!cast)
			t.Throw("Can't cast from %s to bool for condition.", var_name[cond->result]);
		node->nodes.push_back(cast);
		t.AssertSymbol(')');
		t.Next();
		node->nodes.push_back(ParseBlock());
		// else
		if(t.IsKeyword(K_ELSE, G_KEYWORD))
		{
			t.Next();
			node->nodes.push_back(ParseBlock());
		}
		return node;
	}
	else
	{
		// statement
		ParseNode* node = ParseStatement();
		t.AssertSymbol(';');
		t.Next();
		return node;
	}
}

ParseNode* ScriptEngine::ParseTopLine()
{
	if(t.IsKeywordGroup(G_TYPE))
	{
		// variable declaration
		int index = (int)pvars.size();
		ParseVar pvar;
		pvar.type = (VarType)t.GetKeywordId();
		t.Next();
		ParseNode* block = new ParseNode;
		block->type = NODE_BLOCK;
		block->result = VAR_VOID;
		while(true)
		{
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
				ParseNode* cast = ParseCast(child, pvar.type);
				if(cast == NULL)
					t.Throw("Can't assign type %s to variable '%s' of type %s.", var_name[child->result], pvar.name.c_str(), var_name[pvar.type]);
				ParseNode* node = new ParseNode;
				node->type = NODE_ASSIGN;
				node->result = pvar.type;
				node->value = pvar.index;
				node->nodes.push_back(cast);
				block->nodes.push_back(node);
			}

			if(t.IsSymbol(','))
				t.Next();
			else
			{
				t.AssertSymbol(';');
				t.Next();
				break;
			}
		}
		return block;
	}
	else
	{
		// line
		return ParseLine();
	}
}

union ByteIntFloat
{
	struct
	{
		byte b[4];
	};
	int _int;
	float _float;
};

void ScriptEngine::RunNode(ParseNode* node, vector<byte>& code)
{
	if(node->type != NODE_IF)
	{
		for(ParseNode* child : node->nodes)
		{
			RunNode(child, code);
			if(node->type == NODE_BLOCK && child->result != VAR_VOID)
				code.push_back(POP);
		}
	}

	switch(node->type)
	{
	case NODE_BOOL:
		code.push_back(node->value == 1 ? PUSH_TRUE : PUSH_FALSE);
		break;
	case NODE_INT:
		{
			code.push_back(PUSH_INT);
			ByteIntFloat b;
			b._int = node->value;
			code.push_back(b.b[0]);
			code.push_back(b.b[1]);
			code.push_back(b.b[2]);
			code.push_back(b.b[3]);
		}
		break;
	case PUSH_FLOAT:
		{
			code.push_back(PUSH_FLOAT);
			ByteIntFloat b;
			b._float = node->fvalue;
			code.push_back(b.b[0]);
			code.push_back(b.b[0]);
			code.push_back(b.b[0]);
			code.push_back(b.b[0]);
		}
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
	case NODE_CAST:
		code.push_back(CAST);
		code.push_back(node->value);
		break;
	case NODE_BLOCK:
		break;
	case NODE_IF:
		RunNode(node->nodes[0], code);
		if(node->nodes.size() == 2u)
		{
			// if
			code.push_back(IFJMP);
			uint pos = code.size();
			code.push_back(0);
			code.push_back(0);
			RunNode(node->nodes[1], code);
			uint off = code.size() - pos - 2;
			code[pos] = (off & 0xFF);
			code[pos + 1] = ((off & 0xFF00) >> 8);
		}
		else
		{
			// if else
			code.push_back(IFJMP);
			uint pos_jmp_else = code.size();
			code.push_back(0);
			code.push_back(0);
			RunNode(node->nodes[1], code);
			code.push_back(JMP);
			uint pos_jmp_end = code.size();
			code.push_back(0);
			code.push_back(0);
			uint else_off = code.size() - pos_jmp_else - 2;
			RunNode(node->nodes[2], code);
			uint end_off = code.size() - pos_jmp_end - 2;
			code[pos_jmp_else] = (else_off & 0xFF);
			code[pos_jmp_else + 1] = ((else_off & 0xFF00) >> 8);
			code[pos_jmp_end] = (end_off & 0xFF);
			code[pos_jmp_end + 1] = ((end_off & 0xFF00) >> 8);
		}
		break;
	default:
		assert(0);
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
		t.Next();

		while(!t.IsEof())
		{
			ParseNode* node = ParseTopLine();
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

	//Decode(c);

	RunCode(c, pstrs);
}

void ScriptEngine::ParseAndRunFile(cstring filename)
{
	string str;
	if(!LoadFileToString(filename, str))
		return;
	ParseAndRun(str.c_str());
}

void ScriptEngine::Decode(vector<byte>& code)
{
	byte* bs = &code[0];
	byte* be = bs + code.size();

	while(bs < be)
	{
		Op op = (Op)*bs;
		++bs;
		switch(op)
		{
		case INVOP:
			printf("INVOP,\n");
			break;
		case ADD:
			printf("ADD,\n");
			break;
		case SUB:
			printf("SUB,\n");
			break;
		case MUL:
			printf("MUL,\n");
			break;
		case DIV:
			printf("DIV,\n");
			break;
		case MOD:
			printf("MOD,\n");
			break;
		case CAST:
			{
				byte b = *bs++;
				printf("CAST, %d,\n", b);
			}
			break;
		case IFE:
			printf("IFE,\n");
			break;
		case IFNE:
			printf("IFNE,\n");
			break;
		case IFG:
			printf("IFG,\n");
			break;
		case IFGE:
			printf("IFGE,\n");
			break;
		case IFL:
			printf("IFL,\n");
			break;
		case IFLE:
			printf("IFLE,\n");
			break;
		case AND:
			printf("AND,\n");
			break;
		case OR:
			printf("OR,\n");
			break;
		case NOT:
			printf("NOT,\n");
			break;
		case B_NOT:
			printf("B_NOT,\n");
			break;
		case B_OR:
			printf("B_OR,\n");
			break;
		case B_XOR:
			printf("B_XOR,\n");
			break;
		case L_SHIFT:
			printf("L_SHIFT,\n");
			break;
		case R_SHIFT:
			printf("R_SHIFT,\n");
			break;
		case NEG:
			printf("NEG,\n");
			break;
		case LOCALS:
			{
				byte b = *bs++;
				printf("LOCALS, %d,\n", b);
			}
			break;
		case SET_LOCAL:
			{
				byte b = *bs++;
				printf("SET_LOCAL, %d,\n", b);
			}
			break;
		case PUSH_LOCAL:
			{
				byte b = *bs++;
				printf("PUSH_LOCAL, %d,\n", b);
			}
			break;
		case PUSH_TRUE:
			printf("PUSH_TRUE,\n");
			break;
		case PUSH_FALSE:
			printf("PUSH_FALSE,\n");
			break;
		case PUSH_INT:
			{
				int i = *(int*)bs;
				bs += 4;
				printf("PUSH_INT, %d,\n", i);
			}
			break;
		case PUSH_STR:
			{
				byte b = *bs++;
				printf("PUSH_STR, %d,\n", b);
			}
			break;
		case PUSH_FLOAT:
			{
				float f = *(float*)bs;
				bs += 4;
				printf("PUSH_FLOAT, %g,\n", f);
			}
			break;
		case POP:
			printf("POP,\n");
			break;
		case CALL:
			{
				byte b = *bs++;
				printf("CALL, %d,\n", b);
			}
			break;
		case JMP:
			{
				short off = *(short*)bs;
				bs += 2;
				printf("JMP, %d,\n", off);
			}
			break;
		case IFJMP:
			{
				short off = *(short*)bs;
				bs += 2;
				printf("JMPIF, %d,\n", off);
			}
			break;
		case RET:
			printf("RET,\n");
			break;
		default:
			assert(0);
			printf("INVALID OP $d\n", op);
			break;
		}
	}
	_getch();
	system("cls");
}

int main()
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	vector<byte> bcode = {
		LOCALS, 1,
		PUSH_STR, 0,
		CALL, 3,
		CALL, 1,
		SET_LOCAL, 0,
		PUSH_LOCAL, 0,
		PUSH_INT, 0, 0, 0, 0,
		IFE,
		IFJMP, 6, 0,
		PUSH_STR, 1,
		CALL, 3,
		JMP, 4,
		PUSH_STR, 2,
		CALL, 3,
		CALL, 2,
		RET
	};

	vector<cstring> _strs = {
		"Podaj a: ",
		"a == 0",
		"a != 0"
	};

	vector<String*> strs;
	for(cstring s : _strs)
		strs.push_back(new String(s));

	ScriptEngine script;
	script.InitFunctions();
	script.ParseAndRunFile("../doc/script/7.txt");
	//script.RunCode(bcode, strs);

	return 0;
}
