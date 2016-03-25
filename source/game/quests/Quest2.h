#pragma once

#include "Quest.h"
#include "Dialog.h"

class Quest2
{
public:
	Quest2() {}

	string id, code;
	Quest::Type type;
	vector<string> progress;
	vector<Dialog2*> dialogs;
	asIObjectType* obj_type;
	uint crc, prop_size;

	inline int FindProgress(const string& prog) const
	{
		int p = 0;
		for(const string& s : progress)
		{
			if(s == prog)
				return p;
			++p;
		}
		return -1;
	}
};

class Quest2Instance
{
public:
	Quest2* quest;
	asIScriptObject* obj; // pointer to script object
	int refid; // unique quest identifier
	int progress;
	int start_loc, target_loc; // index of location or -1

	const string* GetText(const string& id);
	void AddTimer(int progress, int days);

	int S_GetProgress() const;
	Location* S_GetStartLoc() const;
	Location* S_GetTargetLoc() const;

	void S_SetTargetLoc(Location* loc);

	void Save(StreamWriter& s);
	void Load(StreamReader& s);
};
