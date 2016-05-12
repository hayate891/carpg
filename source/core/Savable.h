#pragma once

class StreamWriter;
class StreamReader;

struct SaveContext
{
	int version;
};

class Savable
{
	virtual void Save(StreamWriter& f, SaveContext& sc) const = 0;
	virtual bool Load(StreamReader& f, SaveContext& sc) = 0;
	virtual uint GetMinimumSize(SaveContext& sc) const { return 0u; }
};
