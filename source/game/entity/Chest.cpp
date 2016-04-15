// skrzynia
#include "Pch.h"
#include "Base.h"
#include "Chest.h"
#include "Game.h"

//=================================================================================================
void Chest::Save(HANDLE file, bool local)
{
	SaveItems(StreamWriter(file), items);

	WriteFile(file, &pos, sizeof(pos), &tmp, nullptr);
	WriteFile(file, &rot, sizeof(rot), &tmp, nullptr);
	WriteFile(file, &netid, sizeof(netid), &tmp, nullptr);

	if(local)
	{
		AnimeshInstance::Group& group = ani->groups[0];
		if(group.IsPlaying())
		{
			WriteFile(file, &group.state, sizeof(group.state), &tmp, nullptr);
			WriteFile(file, &group.time, sizeof(group.time), &tmp, nullptr);
			WriteFile(file, &group.blend_time, sizeof(group.blend_time), &tmp, nullptr);
		}
		else
		{
			int b = 0;
			WriteFile(file, &b, sizeof(b), &tmp, nullptr);
		}
	}

	int chest_event_handler_quest_refid = (handler ? handler->GetChestEventHandlerQuestRefid() : -1);
	WriteFile(file, &chest_event_handler_quest_refid, sizeof(chest_event_handler_quest_refid), &tmp, nullptr);
}

//=================================================================================================
void Chest::Load(HANDLE file, bool local)
{
	LoadItems(StreamReader(file), items);

	ReadFile(file, &pos, sizeof(pos), &tmp, nullptr);
	ReadFile(file, &rot, sizeof(rot), &tmp, nullptr);
	ReadFile(file, &netid, sizeof(netid), &tmp, nullptr);
	looted = false;

	if(local)
	{
		int b;
		ReadFile(file, &b, sizeof(b), &tmp, nullptr);

		ani = new AnimeshInstance(Game::Get().aSkrzynia);

		if(b != 0)
		{
			AnimeshInstance::Group& group = ani->groups[0];
			group.anim = &ani->ani->anims[0];
			group.state = b;
			ReadFile(file, &group.time, sizeof(group.time), &tmp, nullptr);
			ReadFile(file, &group.blend_time, sizeof(group.blend_time), &tmp, nullptr);
			group.used_group = 0;
		}
	}
	else
		ani = nullptr;

	int refid;
	ReadFile(file, &refid, sizeof(refid), &tmp, nullptr);
	if(refid == -1)
		handler = nullptr;
	else
	{
		handler = (ChestEventHandler*)refid;
		Game::Get().load_chest_handler.push_back(this);
	}
}
