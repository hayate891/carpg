#include "Common.h"
#include <cstdarg>
#include <cstdio>
#include <cassert>

const uint FORMAT_STRINGS = 8;
const uint FORMAT_LENGTH = 2048;
char format_buf[FORMAT_STRINGS][FORMAT_LENGTH];
int format_marker;

//=================================================================================================
// Formatowanie ci¹gu znaków
//=================================================================================================
cstring Format(cstring str, ...)
{
	assert(str);

	va_list list;
	va_start(list, str);
	char* cbuf = format_buf[format_marker];
	_vsnprintf_s(cbuf, FORMAT_LENGTH, FORMAT_LENGTH - 1, str, list);
	cbuf[FORMAT_LENGTH - 1] = 0;
	format_marker = (format_marker + 1) % FORMAT_STRINGS;
	va_end(list);

	return cbuf;
}

cstring log_level_name[3] = {
	"INFO",
	"WARN",
	"ERRO"
};

void Logger::GetTime(tm& out)
{
	time_t t = time(0);
	localtime_s(&out, &t);
}

ConsoleLogger::~ConsoleLogger()
{
	Log(nullptr, "*** End of log.", L_INFO);
}

void ConsoleLogger::Log(cstring category, cstring text, LOG_LEVEL level)
{
	assert(text);

	tm time;
	GetTime(time);

	if(category)
		printf("%02d:%02d:%02d %s - %s: %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], category, text);
	else
		printf("%02d:%02d:%02d %s - %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], text);
	fflush(stdout);
}

void ConsoleLogger::Log(cstring category, cstring text, LOG_LEVEL level, const tm& time)
{
	assert(text);

	if(category)
		printf("%02d:%02d:%02d %s - %s: %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], category, text);
	else
		printf("%02d:%02d:%02d %s - %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], text);
	fflush(stdout);
}

TextLogger::TextLogger(cstring filename, bool append) : path(filename)
{
	assert(filename);
	std::ios_base::openmode mode = std::ios_base::out;
	if(append)
		mode |= std::ios_base::app;
	out.open(filename, mode);
}

TextLogger::~TextLogger()
{
	Log(nullptr, "*** End of log.", L_INFO);
}

void TextLogger::Log(cstring category, cstring text, LOG_LEVEL level)
{
	assert(text);

	tm time;
	GetTime(time);

	if(category)
		out << Format("%02d:%02d:%02d %s - %s: %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], category, text);
	else
		out << Format("%02d:%02d:%02d %s - %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], text);
}

void TextLogger::Log(cstring category, cstring text, LOG_LEVEL level, const tm& time)
{
	assert(text);

	if(category)
		out << Format("%02d:%02d:%02d %s - %s: %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], category, text);
	else
		out << Format("%02d:%02d:%02d %s - %s\n", time.tm_hour, time.tm_min, time.tm_sec, log_level_name[level], text);
}

void TextLogger::Flush()
{
	out.flush();
}

MultiLogger::~MultiLogger()
{
	DeleteElements(loggers);
}

void MultiLogger::Log(cstring category, cstring text, LOG_LEVEL level)
{
	assert(text);

	for(Logger* logger : loggers)
		logger->Log(category, text, level);
}

void MultiLogger::Log(cstring category, cstring text, LOG_LEVEL level, const tm& time)
{
	assert(text);

	for(Logger* logger : loggers)
		logger->Log(category, text, level, time);
}

void MultiLogger::Flush()
{
	for(Logger* logger : loggers)
		logger->Flush();
}

void PreLogger::Apply(Logger* logger)
{
	assert(logger);

	for(Prelog* log : prelogs)
		logger->Log(log->category.empty() ? nullptr : log->category.c_str(), log->str.c_str(), log->level, log->time);

	if(flush)
		logger->Flush();
	DeleteElements(prelogs);
}

void PreLogger::Clear()
{
	DeleteElements(prelogs);
}

void PreLogger::Log(cstring category, cstring text, LOG_LEVEL level)
{
	assert(text);

	Prelog* p = new Prelog;
	if(category)
		p->category = category;
	p->str = text;
	p->level = level;
	GetTime(p->time);

	prelogs.push_back(p);
}

void PreLogger::Log(cstring category, cstring text, LOG_LEVEL level, const tm& time)
{
	assert(text);

	Prelog* p = new Prelog;
	if(category)
		p->category = category;
	p->str = text;
	p->level = level;
	p->time = time;

	prelogs.push_back(p);
}

void PreLogger::Flush()
{
	flush = true;
}
