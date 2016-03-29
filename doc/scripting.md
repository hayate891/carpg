# CaRpg scripting reference #

**CaRpg** use AngelScript language (c++ like syntax), for more information check [website](http://www.angelcode.com/angelscript) or [manual](http://www.angelcode.com/angelscript/sdk/docs/manual/).

## Item object
Item is generic item type (sword, long bow) not specific (sword wielded by player, long bow inside chest).
###Properties:
* string name

## Player object
###Properties:
* Unit@ unit

## Unit object
###Methods:
**void AddItem(Item@ item)** - Add item to unit (with count 1 and team).

## Dialog functions ##
**void Dialog_Continue(const string& in dialog\_id)** - Continue dialog from quest, must be called from current quest when it is called from dialog.

## Item functions ##
**void Item_CreateQuestItem(const string& in base\_item\_id)** - Create copy of base item that is marked as quest item connected to current quest. Must be called from quest.

## Journal functions
**void Journal_AddQuestEntry(TextPtr@ name, TextPtr@ text, TextPtr@ text2 = null)** - Add new quest entry to journal with two texts. Quest entry is connected with current quest.

**void Journal_UpdateQuestEntry(TextPtr@ text)** - Add new text to quest entry. Quest entry is taken from current quest.

## Quest functions
**void Quest_AddTimer(int progress\, int days)** - Add new timer for quest, when **days** pass it will call on_progress with specified **progress**.

## World functions ##
**Location@ World_GetRandomSettlement(Location@ current = null)** - Get random settlement (city or village) that is not **current**.

## Global objects ##
* **Player player** - current player
* **Unit talker** - unit that talks to player
* **Unit user** - unit eating consumeable item
