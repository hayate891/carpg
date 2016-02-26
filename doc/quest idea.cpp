class Quest; // schemat zadania (po stronie serwera)
class QuestInstance; // pojedyñcza kopia zadania (po stronie serwera)
class QuestEntry; // wpis w dzienniku o zadaniu

quest find_herbs {
	default:
		Unit owner;
		int progress, prev_progress, state;
		
	progress {
		none
		started
		timeout
		complete
	}

	function on_init {
		assert_var(owner);
		
		owner.AddGreet("start", 100); // id dialogu, priorytet
	}
	
	// wbudowana funkcja, progress i prev_progress jest ustawione
	function on_progress {
		if(progress == started) {
			AddQuestEntry(6 "W poszukiwaniu kociomiêtki", 7 "Alchemik chce ¿ebym mu dostarczy³ ziele. Mam na to 10 dni.");
			owner.RemoveGreet("start");
			owner.AddDialog("check_item", 100);
			owner.SetTimer(timeout, 10); // timeout po 10 dniach na progress timeout
		}
		else if(progress == complete)
		{
			QuestDone(10 "Dostarczy³em ziele i otrzyma³em zap³atê.");
			player.unit.RemoveItem("ziele");
			owner.AddItem("ziele");
			Team.GoldReward(300);
		}
		else if(progress == timeout)
		{
			QuestFail(8 "Nie zd¹¿y³em na czas zanieœæ kocimiêtki.");
			owner.RemoveDialog("check_item");
		}
	}
	
	
	dialog start {
		talk 0 "Witaj wêdrowcze!"
		talk 1 "Mam do ciebie proœbê, szukam kogoœ kto dostarczy mi herbovit kociomiêtki."
		talk 2 "Zap³acê za niego 500 sztuk z³ota, jesteœ zainteresowany?"
		choice 3 "Tak"
			do_progress started // jak set_progress ale nie wraca
		escape choice 4 "Nie"
			end // bêdzie gada³ przy ka¿dym rozpoczêciu dialogu
		show_choices
	}
	
	dialog accepted {
		talk 5 "Dobrze tylko siê poœpiesz. Masz na to 10 dni."
		end
	}
	
	dialog check_item {
		if have_item "ziele" {
			choice 9 "Daj ziele"
				do_progress complete
		}
	}
}

AddQuest("find_herbs", "xxx"); /// xxx - identyfikator alchemika

