#pragma once

//-----------------------------------------------------------------------------
#include "Control.h"
#include "Stream.h"

//-----------------------------------------------------------------------------
class GameMessages : public Control
{
public:
	GameMessages();

	void Draw(ControlDrawData* cdd = nullptr);
	void Update(float dt);
	void Reset();
	void Save(StreamWriter& f) const;
	bool Load(StreamReader& f);
	void AddMessage(cstring text, float time, int type);
	void AddMessageIfNotExists(cstring text, float time, int type);

private:
	struct GameMsg
	{
		static const uint MIN_SIZE = 36;

		string msg;
		float time, fade;
		VEC2 pos;
		INT2 size;
		int type;
	};

	list<GameMsg> msgs;
	int msgs_h;
	cstring txGamePausedBig;
};
