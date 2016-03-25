## CaRpg scripting reference ##

**CaRpg** use AngelScript language (c++ like syntax), for more information check [website](http://www.angelcode.com/angelscript) or [manual](http://www.angelcode.com/angelscript/sdk/docs/manual/).

## Global functions ##
**Location@ World_GetRandomSettlement(Location@ current = null)** - Get random settlement (city or village) that is not **current**.

## Quest example ##
	quest deliver_letter { // <- quest identifier, this must be unique
		type mayor // <- quest type (currently mayor, captain, traveler or unique)
		progress { // <- quest progress enum, you can only set progress for one of this
			none
			started
			timeout
			get_response
			finished
		}

		/* hidden variables:
		progress progress - progress of current quest
		progress prev_progress - previous progress, set before call to on_progress
		Location@ start_loc - start location of quest (defaults to
		*/
		
		// block of raw code
		code ${
			void on_init() // called on initialization (in this example when player asks mayor if he have any quest for him
			{
				@target_loc = World_GetRandomSettlement(start_loc); // 
				ContinueDialog("start");
			}
			
			void on_progress()
			{
				if(progress == started)
				{
					Item letter = CreateQuestItem("letter");
					letter.name = "";
					player.unit.AddItem(letter);
					AddQuestEntry(text["name"], text["infoStart"], text["infoStart2"]);
				}
			}
		}$
		
		// dialogs (check dialog docs for details)
		dialog start {
			talk info
			choice gYes {
				talk accepted0
				talk accepted1
				set_quest_progress started
				end
			}
			escape choice gNo
				end
			show_choices
		}
	}
