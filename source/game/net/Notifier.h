#pragma once

struct Unit;
struct Item;

class Notifier
{
private:
	Notifier() {}
	~Notifier() {}
	static Notifier notifier;

public:
	inline static Notifier& Get()
	{
		return notifier;
	}

	void Add(const Unit* unit, int property);
	void Add(const Item* item, int property);
};
