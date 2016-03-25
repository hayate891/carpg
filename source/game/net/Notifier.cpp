#include "Pch.h"
#include "Base.h"
#include "Notifier.h"
#include "Item.h"

Notifier Notifier::notifier;

void Notifier::Add(const Unit* unit, int property)
{

}

void Notifier::Add(const Item* item, int property)
{
	if(IS_SET(item->flags, ITEM_REGISTERED))
	{

	}
}
