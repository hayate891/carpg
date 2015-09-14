#pragma once

struct FunctionInfo
{
	FunctionInfo() {}
	FunctionInfo(void* ptr, bool is_method = false) : ptr(ptr), is_method(is_method) {}

	void* ptr;
	bool is_method;
};

template<int N>
struct MethodHelper
{
	template<class M>
	FunctionInfo Convert(M mthd)
	{
		assert(N == 1);
		void* ptr = &mthd;
		int adr = ((int*)ptr)[0];
		return FunctionInfo((void*)adr, true);
	}
};

#define FUNCTION(f) FunctionInfo(f)
#define FUNCTION_EX(f,p,r) FunctionInfo((void (*)())((r (*)p)(f)))
#define METHOD(c,m) MethodHelper<sizeof(void (c::*)())>().Convert((void (c::*)())(&c::m))
#define METHOD_EX(c,m,p,r) MethodHelper<sizeof(void (c::*)())>().Convert((r (c::*)p)(&c::m))

//#undef RegisterClass

struct Str
{
	string s;
	int refs;
};

ObjectPool<Str> StrPool;

enum VarType
{
	V_Void,
	V_Int,
	V_Cstring,
	V_Params,
	V_String,
	V_Class
};

enum FuncType
{
	//					C++			CAS
	Func,			//	func(a)		func(a)
	FuncObjFirst,	//	func(a,b)	a.func(b)
	MethodThis,		//	a.func(b)	a.func(b)
	MethodObj		//	a.func(b)	func(b)
};

namespace cas
{
	struct TypeInfo
	{
		cstring name;
		bool validParam;
		bool validResult;
		bool validVar;
	};

	struct Function
	{
		string id;
		VarType result;
		vector<VarType> args;
		FuncType type;
		void* f;
		void* obj;
		int index;
		bool variadic;
	};

	struct ParamList;

	struct StackItem
	{
		VarType type;
		int subtype;
		union
		{
			int i;
			cstring cs;
			ParamList* p;
			Str* s;
		};

		inline StackItem() {}
		inline explicit StackItem(int _int) : type(V_Int), i(_int) {}
		inline explicit StackItem(cstring s) : type(V_Cstring), cs(s) {}
		inline explicit StackItem(ParamList* p) : type(V_Params), p(p) {}
		inline explicit StackItem(Str* s) : type(V_String), s(s) {}

		void Release();
	};

	struct ParamItem
	{
		VarType type;
		int subtype;
		int value;

		inline ParamItem() {}
		inline explicit ParamItem(StackItem& s) : type(s.type), subtype(s.subtype), value(s.i) {}

		cstring ToString() const
		{
			switch(type)
			{
			case V_Void:
				return "void";
			case V_Int:
				return Format("%d", value);
			case V_Cstring:
				return (cstring)value;
			case V_Params:
				return "params";
			case V_String:
				return ((Str*)value)->s.c_str();
			case V_Class:
				return Format("class %d %p", subtype, value);
			default:
				return "???";
			}
		}
	};

	struct ParamList
	{
		vector<ParamItem> items;
		int refs;
	};

	ObjectPool<ParamList> ParamListPool;

	inline void StackItem::Release()
	{
		if(type == V_String)
		{
			if(--s->refs)
				StrPool.Free(s);
		}
		else if(type == V_Params)
		{
			if(--p->refs)
				ParamListPool.Free(p);
		}
	}

	class Engine;

	class Class
	{
		friend class Engine;
	public:
		void AddFunction(cstring def, const FunctionInfo& f);

		string id;
		int index;

	private:
		Engine* engine;

		Class(Engine* engine, cstring id) : id(id), engine(engine) {}
	};

	/*enum ScriptTypeId
	{
		Void,
		Int,
		Uint,
		Char,
		Byte,
		Long,
		Ulong,
		Cstring,
		String,
		Class,
		Bool,

		CustomType
	};

	class ScriptType
	{
	public:
		string name;
	};

	class Enum
	{
	public:
		void Add(cstring id, int value);

	private:
		string id;
	};

	enum class VarType
	{
		Void,
		Int,
		Class
	};

	class Type
	{
	public:
		string name;

	};

	class Class
	{
	public:
		void RegisterFunction(cstring def, const FunctionInfo& f);

		void SetBridge(Class* clas, const FunctionInfo& f);
	};*/

	class Global
	{
	public:
		string name;
		VarType type;
		int subtype;
		void* ptr;
	};

	class Engine
	{
	public:
		/*template<typename T>
		struct StringPair
		{
			cstring id;
			T value;
		};

		template<typename T>
		void RegisterEnum(cstring id, std::initializer_list<StringPair<T>> const & values);

		Class* RegisterClass(cstring id);

		void RegisterFunction(cstring id, const FunctionInfo& f);

		void AddGlobal(cstring def);

		void Prepare();

		void RunLine(cstring code);

		//=======================================================================================================================
		// ENUM
		Enum* AddEnum(cstring id);
		Enum* FindEnum(cstring id);

		void Init();
		void RegisterTypes();
		void Prepare();
		void Parse(cstring code);
		void ParseDefinition(cstring def);

	private:
		vector<ScriptType> types;
		Tokenizer t;*/

		void Init();

		void Parse();
		void Parse(cstring code);
		void ParseFile(cstring file);
		void ParseFunction(Function* f);
		VarType ParseExpression();
		VarType ParseItem();

		//==============================================================
		// Function
		void AddFunction(cstring def, const FunctionInfo& f, void* obj = NULL);
		void AddClassFunction(Class* clas, cstring def, const FunctionInfo& f);
		Function* FindFunction(const string& id);
		Function ParseFunctionDef(cstring def);

		//==============================================================
		// Class
		Class* AddClass(cstring id);
		Class* FindClass(const string& id);

		//==============================================================
		// Global
		void AddGlobal(cstring def, void* ptr);
		Global* FindGlobal(const string& id);

		void Run(byte* code, vector<string>& strs);

	private:
		Tokenizer t;
		vector<Function> functions;
		vector<Class*> classes;
		vector<Global> globals;
	};
};
