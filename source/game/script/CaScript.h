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

namespace cas
{
	enum Type
	{
		Void,
		Int,
		Cstring,
		Params
	};

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
		Type result;
		vector<Type> args;
		FunctionInfo f;
		int index;
		bool variadic;
	};

	struct ParamList;

	struct StackItem
	{
		Type type;
		union
		{
			int i;
			cstring cs;
			ParamList* p;
		};

		inline StackItem() {}
		inline StackItem(int _int) : type(cas::Int), i(_int) {}
		inline StackItem(cstring s) : type(cas::Cstring), cs(s) {}
		inline StackItem(ParamList* p) : type(cas::Params), p(p) {}
	};

	struct ParamItem
	{
		Type type;
		int value;

		inline ParamItem() {}
		inline ParamItem(StackItem& s) : type(s.type), value(s.i) {}

		cstring ToString() const
		{
			switch(type)
			{
			case Void:
				return "void";
			case Int:
				return Format("%d", value);
			case Cstring:
				return (cstring)value;
			case Params:
				return "params";
			default:
				return "???";
			}
		}
	};

	struct ParamList
	{
		vector<ParamItem> items;
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
	};

	class Global
	{
	public:

	};*/

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
		Type ParseExpression();
		Type ParseItem();

		void AddFunction(cstring def, const FunctionInfo& f);
		Function* FindFunction(const string& id);

		void Run(byte* code, vector<string>& strs);

	private:
		Tokenizer t;
		vector<Function> functions;
	};
};
