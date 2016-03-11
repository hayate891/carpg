#pragma once

#include <ctime>
#include <fstream>
#include <string>
#include <vector>

using std::string;
using std::vector;

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int uint;
typedef const char* cstring;

cstring Format(cstring str, ...);

//-----------------------------------------------------------------------------
// Funkcje obs³uguj¹ce vector
//-----------------------------------------------------------------------------
// Usuwanie elementów wektora
template<typename T>
inline void DeleteElements(vector<T>& v)
{
	for(vector<T>::iterator it = v.begin(), end = v.end(); it != end; ++it)
		delete *it;
	v.clear();
}

template<typename T>
inline void DeleteElements(vector<T>* v)
{
	DeleteElements(*v);
}

//-----------------------------------------------------------------------------
// Loggery
//-----------------------------------------------------------------------------
// interfejs logera
struct Logger
{
	enum LOG_LEVEL
	{
		L_INFO,
		L_WARN,
		L_ERROR
	};

	virtual ~Logger() {}
	void GetTime(tm& time);


	virtual void Log(cstring category, cstring text, LOG_LEVEL level) = 0;
	virtual void Log(cstring category, cstring text, LOG_LEVEL level, const tm& time) = 0;
	virtual void Flush() = 0;

	inline void Info(cstring text) { Log(nullptr, text, L_INFO); }
	inline void Info(cstring category, cstring text) { Log(category, text, L_INFO); }
	inline void Warn(cstring text) { Log(nullptr, text, L_WARN); }
	inline void Warn(cstring category, cstring text) { Log(category, text, L_WARN); }
	inline void Error(cstring text) { Log(nullptr, text, L_ERROR); }
	inline void Error(cstring category, cstring text) { Log(category, text, L_ERROR); }
};

// pusty loger, nic nie robi
struct NullLogger : public Logger
{
	NullLogger() {}
	void Log(cstring category, cstring text, LOG_LEVEL level) {}
	void Log(cstring category, cstring text, LOG_LEVEL, const tm& time) {}
	void Flush() {}
};

// loger do konsoli
struct ConsoleLogger : public Logger
{
	~ConsoleLogger();
	void Log(cstring category, cstring text, LOG_LEVEL level);
	void Log(cstring category, cstring text, LOG_LEVEL level, const tm& time);
	void Flush() {}
};

// loger do pliku txt
struct TextLogger : public Logger
{
	std::ofstream out;
	string path;

	explicit TextLogger(cstring filename, bool append=false);
	~TextLogger();
	void Log(cstring category, cstring text, LOG_LEVEL level);
	void Log(cstring category, cstring text, LOG_LEVEL level, const tm& time);
	void Flush();
};

// loger do kilku innych logerów
struct MultiLogger : public Logger
{
	vector<Logger*> loggers;

	~MultiLogger();
	void Log(cstring category, cstring text, LOG_LEVEL level);
	void Log(cstring category, cstring text, LOG_LEVEL level, const tm& time);
	void Flush();
};

// loger który przechowuje informacje przed wybraniem okreœlonego logera
struct PreLogger : public Logger
{
private:
	struct Prelog
	{
		string category, str;
		LOG_LEVEL level;
		tm time;
	};

	vector<Prelog*> prelogs;
	bool flush;

public:
	PreLogger() : flush(false) {}
	void Apply(Logger* logger);
	void Clear();
	void Log(cstring category, cstring text, LOG_LEVEL level);
	void Log(cstring category, cstring text, LOG_LEVEL level, const tm& time);
	void Flush();
};
