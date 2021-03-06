#pragma once

//-----------------------------------------------------------------------------
#include "GamePanel.h"
#include "Scrollbar.h"
#include "Button.h"

//-----------------------------------------------------------------------------
struct Game;
struct Unit;

//-----------------------------------------------------------------------------
class TeamPanel : public GamePanel
{
public:
	TeamPanel();
	void Draw(ControlDrawData*);
	void Update(float dt);
	void Event(GuiEvent e);
	void Show();
	void Hide();

	void Changed();
	void UpdateButtons();
	void OnPayCredit(int id);
	void GiveGold();
	void ChangeLeader();
	void Kick();
	void OnGiveGold(int id);
	void OnKick(int id);

	Scrollbar scrollbar;
	Button bt[4];
	cstring txTeam, txCharInTeam, txPing, txDays, txPickCharacter, txNoCredit, txPayCreditAmount, txNotEnoughGold, txPaidCredit, txPaidCreditPart, txGiveGoldSelf, txGiveGoldAmount, txOnlyPcLeader,
		txAlreadyLeader, txYouAreLeader, txCantChangeLeader, txPcAlreadyLeader, txPcIsLeader, txCantKickMyself, txCantKickAi, txReallyKick, txAlreadyLeft, txCAlreadyLeft;
	int counter, mode, picked;
	bool picking;
	vector<Hitbox> hitboxes;
	Unit* target;

	static TEX tKorona, tCzaszka;

private:
	Game& game;
};
