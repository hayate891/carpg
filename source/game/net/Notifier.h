#pragma once

struct Unit;

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

	void Add(Unit* unit, int property);
};
