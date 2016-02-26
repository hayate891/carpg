class Quest; // schemat zadania (po stronie serwera)
class QuestInstance; // pojedy�cza kopia zadania (po stronie serwera)
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
			AddQuestEntry(6 "W poszukiwaniu kociomi�tki", 7 "Alchemik chce �ebym mu dostarczy� ziele. Mam na to 10 dni.");
			owner.RemoveGreet("start");
			owner.AddDialog("check_item", 100);
			owner.SetTimer(timeout, 10); // timeout po 10 dniach na progress timeout
		}
		else if(progress == complete)
		{
			QuestDone(10 "Dostarczy�em ziele i otrzyma�em zap�at�.");
			player.unit.RemoveItem("ziele");
			owner.AddItem("ziele");
			Team.GoldReward(300);
		}
		else if(progress == timeout)
		{
			QuestFail(8 "Nie zd��y�em na czas zanie�� kocimi�tki.");
			owner.RemoveDialog("check_item");
		}
	}
	
	
	dialog start {
		talk 0 "Witaj w�drowcze!"
		talk 1 "Mam do ciebie pro�b�, szukam kogo� kto dostarczy mi herbovit kociomi�tki."
		talk 2 "Zap�ac� za niego 500 sztuk z�ota, jeste� zainteresowany?"
		choice 3 "Tak"
			do_progress started // jak set_progress ale nie wraca
		escape choice 4 "Nie"
			end // b�dzie gada� przy ka�dym rozpocz�ciu dialogu
		show_choices
	}
	
	dialog accepted {
		talk 5 "Dobrze tylko si� po�piesz. Masz na to 10 dni."
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

