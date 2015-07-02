#include "Pch.h"
#include "Base.h"
#include "PlayerController.h"
#include "Unit.h"
#include "Game.h"
#include "SaveState.h"

//=================================================================================================
PlayerController::~PlayerController()
{
	if(dialog_ctx && !dialog_ctx->is_local)
		delete dialog_ctx;
}

//=================================================================================================
float PlayerController::CalculateAttack() const
{
	WeaponType b;

	switch(unit->stan_broni)
	{
	case BRON_CHOWA:
		b = unit->chowana;
		break;
	case BRON_SCHOWANA:
		if(ostatnia == W_NONE)
		{
			if(unit->HaveWeapon())
				b = W_ONE_HANDED;
			else if(unit->HaveBow())
				b = W_BOW;
			else
				b = W_NONE;
		}
		else
			b = ostatnia;
		break;
	case BRON_WYJMUJE:
	case BRON_WYJETA:
		b = unit->wyjeta;
		break;
	default:
		assert(0);
		break;
	}

	if(b == W_ONE_HANDED)
	{
		if(!unit->HaveWeapon())
		{
			if(!unit->HaveBow())
				b = W_NONE;
			else
				b = W_BOW;
		}
	}
	else if(b == W_BOW)
	{
		if(!unit->HaveBow())
		{
			if(!unit->HaveWeapon())
				b = W_NONE;
			else
				b = W_ONE_HANDED;
		}
	}

	if(b == W_ONE_HANDED)
		return unit->CalculateAttack(&unit->GetWeapon());
	else if(b == W_BOW)
		return unit->CalculateAttack(&unit->GetBow());
	else
		return 0.5f * unit->Get(Attribute::STR) + 0.5f * unit->Get(Attribute::DEX);
}

//=================================================================================================
void PlayerController::Init(Unit& _unit, bool partial)
{
	unit = &_unit;
	move_tick = 0.f;
	ostatnia = W_NONE;
	po_akcja = PO_BRAK;
	last_dmg_poison = last_dmg = dmgc = poison_dmgc = 0.f;
	idle_timer = random(1.f, 2.f);
	kredyt = 0;
	na_kredycie = false;
	godmode = false;
	noclip = false;
	action = Action_None;
	free_days = 0;
	recalculate_level = false;

	if(!partial)
	{
		kills = 0;
		dmg_done = 0;
		dmg_taken = 0;
		knocks = 0;
		arena_fights = 0;
		
		for(int i = 0; i<(int)Skill::MAX; ++i)
			sp[i] = 0;
		for(int i = 0; i<(int)Attribute::MAX; ++i)
			ap[i] = 0;
	}	
}

//=================================================================================================
void PlayerController::Update(float dt)
{
	dmgc += last_dmg;
	dmgc *= (1.f - dt*2);
	if(last_dmg == 0.f && dmgc < 0.1f)
		dmgc = 0.f;

	poison_dmgc += (last_dmg_poison - poison_dmgc) * dt;
	if(last_dmg_poison == 0.f && poison_dmgc < 0.1f)
		poison_dmgc = 0.f;

	if(recalculate_level)
	{
		recalculate_level = false;
		unit->level = unit->CalculateLevel();
	}
}

//=================================================================================================
void PlayerController::Train(Skill skill, int points)
{
	int s = (int)skill;

	sp[s] += points;

	int gained = 0,
		value = unit->GetUnmod(skill),
		base = base_stats.skill[s];

	while(sp[s] >= sn[s])
	{
		sp[s] -= sn[s];
		if(value != SkillInfo::MAX)
		{
			++gained;
			++value;
			sn[s] = GetRequiredSkillPoints(value);
		}
		else
		{
			sp[s] = sn[s];
			break;
		}
	}

	if(gained)
	{
		recalculate_level = true;
		unit->Set(skill, value);
		Game& game = Game::Get();
		if(IsLocal())
			game.ShowStatGain(true, s, gained);
		else
		{
			NetChangePlayer& c = game.AddChange(NetChangePlayer::GAIN_STAT, this);
			c.id = 1;
			c.a = s;
			c.ile = gained;

			NetChangePlayer& c2 = game.AddChange(NetChangePlayer::STAT_CHANGED, this);
			c2.id = (int)ChangedStatType::SKILL;
			c2.a = s;
			c2.ile = value;
		}
	}
}

//=================================================================================================
void PlayerController::Train(Attribute attrib, int points)
{
	int a = (int)attrib;

	ap[a] += points;

	int gained = 0,
		value = unit->GetUnmod(attrib),
		base = base_stats.attrib[a];

	while(ap[a] >= an[a])
	{
		ap[a] -= an[a];
		if(unit->stats.attrib[a] != AttributeInfo::MAX)
		{
			++gained;
			++value;
			an[a] = GetRequiredAttributePoints(value);
		}
		else
		{
			ap[a] = an[a];
			break;
		}
	}

	if(gained)
	{
		recalculate_level = true;
		unit->Set(attrib, value);
		Game& game = Game::Get();
		if(IsLocal())
			game.ShowStatGain(false, a, gained);
		else
		{
			NetChangePlayer& c = game.AddChange(NetChangePlayer::GAIN_STAT, this);
			c.id = 0;
			c.a = a;
			c.ile = gained;

			NetChangePlayer& c2 = game.AddChange(NetChangePlayer::STAT_CHANGED, this);
			c2.id = (int)ChangedStatType::ATTRIBUTE;
			c2.a = a;
			c2.ile = value;
		}
	}
}

//=================================================================================================
void PlayerController::TrainMove(float dt, bool run)
{
	move_tick += (run ? dt : dt/10);
	if(move_tick >= 1.f)
	{
		move_tick -= 1.f;
		Train(TrainWhat::Move, 0.f, 0);
	}
}

//=================================================================================================
void PlayerController::TravelTick()
{
	Rest(false);
	Train(TrainWhat::Move, 0.f, 0);

	// up�yw czasu efekt�w
	uint index = 0;
	for(vector<Effect>::iterator it = unit->effects.begin(), end = unit->effects.end(); it != end; ++it, ++index)
	{
		if((it->time -= 1.f) <= 0.f)
			_to_remove.push_back(index);
	}

	while(!_to_remove.empty())
	{
		index = _to_remove.back();
		_to_remove.pop_back();
		if(index == unit->effects.size()-1)
			unit->effects.pop_back();
		else
		{
			std::iter_swap(unit->effects.begin()+index, unit->effects.end()-1);
			unit->effects.pop_back();
		}
	}
}

//=================================================================================================
// wywo�ywane podczas podr�y
void PlayerController::Rest(bool resting)
{
	if(unit->hp != unit->hpmax)
	{
		float heal = 0.5f * unit->Get(Attribute::END);
		if(resting)
			heal *= 2;
		float reg;
		if(unit->FindEffect(E_NATURAL, &reg))
			heal *= reg;

		heal = min(heal, unit->hpmax-unit->hp);
		unit->hp += heal;

		Train(Attribute::END, int(heal));
	}

	last_dmg = 0;
	last_dmg_poison = 0;
	dmgc = 0;
	poison_dmgc = 0;
}

//=================================================================================================
void PlayerController::Rest(int days, bool resting)
{
	// up�yw czasu efekt�w
	int best_nat;
	unit->EndEffects(days, &best_nat);
	
	// regeneracja hp
	if(unit->hp != unit->hpmax)
	{
		float heal = 0.5f * unit->Get(Attribute::END);
		if(resting)
			heal *= 2;
		if(best_nat)
		{
			if(best_nat != days)
				heal = heal*best_nat*2 + heal*(days-best_nat);
			else
				heal *= 2*days;
		}
		else
			heal *= days;

		heal = min(heal, unit->hpmax-unit->hp);
		unit->hp += heal;

		Game& game = Game::Get();
		if(game.IsOnline())
		{
			NetChange& c = Add1(game.net_changes);
			c.type = NetChange::UPDATE_HP;
			c.unit = unit;
		}

		Train(Attribute::END, int(heal));
	}

	last_dmg = 0;
	last_dmg_poison = 0;
	dmgc = 0;
	poison_dmgc = 0;
}

//=================================================================================================
void PlayerController::Save(HANDLE file)
{
	if(recalculate_level)
	{
		unit->level = unit->CalculateLevel();
		recalculate_level = false;
	}

	WriteFile(file, &clas, sizeof(clas), &tmp, NULL);
	byte len = (byte)name.length();
	WriteFile(file, &len, sizeof(len), &tmp, NULL);
	WriteFile(file, name.c_str(), len, &tmp, NULL);
	WriteFile(file, &move_tick, sizeof(move_tick), &tmp, NULL);
	WriteFile(file, &last_dmg, sizeof(last_dmg), &tmp, NULL);
	WriteFile(file, &last_dmg_poison, sizeof(last_dmg_poison), &tmp, NULL);
	WriteFile(file, &dmgc, sizeof(dmgc), &tmp, NULL);
	WriteFile(file, &poison_dmgc, sizeof(poison_dmgc), &tmp, NULL);
	WriteFile(file, &idle_timer, sizeof(idle_timer), &tmp, NULL);
	WriteFile(file, sp, sizeof(sp), &tmp, NULL);
	WriteFile(file, ap, sizeof(ap), &tmp, NULL);
	WriteFile(file, &klawisz, sizeof(klawisz), &tmp, NULL);
	WriteFile(file, &po_akcja, sizeof(po_akcja), &tmp, NULL);
	WriteFile(file, &po_akcja_idx, sizeof(po_akcja_idx), &tmp, NULL);
	WriteFile(file, &ostatnia, sizeof(ostatnia), &tmp, NULL);
	WriteFile(file, &kredyt, sizeof(kredyt), &tmp, NULL);
	WriteFile(file, &godmode, sizeof(godmode), &tmp, NULL);
	WriteFile(file, &noclip, sizeof(noclip), &tmp, NULL);
	WriteFile(file, &id, sizeof(id), &tmp, NULL);
	WriteFile(file, &free_days, sizeof(free_days), &tmp, NULL);
	WriteFile(file, &kills, sizeof(kills), &tmp, NULL);
	WriteFile(file, &knocks, sizeof(knocks), &tmp, NULL);
	WriteFile(file, &dmg_done, sizeof(dmg_done), &tmp, NULL);
	WriteFile(file, &dmg_taken, sizeof(dmg_taken), &tmp, NULL);
	WriteFile(file, &arena_fights, sizeof(arena_fights), &tmp, NULL);
	File f(file);
	base_stats.Save(f);
	f << attrib_state;
	f << skill_state;
	f << (byte)perks.size();
	for(TakenPerk& tp : perks)
	{
		f << (byte)tp.perk;
		f << tp.value;
	}
}

//=================================================================================================
void PlayerController::Load(HANDLE file)
{
	ReadFile(file, &clas, sizeof(clas), &tmp, NULL);
	if(LOAD_VERSION < V_DEVEL)
		clas = ClassInfo::OldToNew(clas);
	byte len;
	ReadFile(file, &len, sizeof(len), &tmp, NULL);
	BUF[len] = 0;
	ReadFile(file, BUF, len, &tmp, NULL);
	name = BUF;
	if(LOAD_VERSION < V_0_2_10)
	{
		float old_weight;
		ReadFile(file, &old_weight, sizeof(old_weight), &tmp, NULL);
	}
	ReadFile(file, &move_tick, sizeof(move_tick), &tmp, NULL);
	ReadFile(file, &last_dmg, sizeof(last_dmg), &tmp, NULL);
	ReadFile(file, &last_dmg_poison, sizeof(last_dmg_poison), &tmp, NULL);
	ReadFile(file, &dmgc, sizeof(dmgc), &tmp, NULL);
	ReadFile(file, &poison_dmgc, sizeof(poison_dmgc), &tmp, NULL);
	ReadFile(file, &idle_timer, sizeof(idle_timer), &tmp, NULL);
	File f(file);
	if(LOAD_VERSION >= V_DEVEL)
	{
		// attribute points
		f >> ap;
		// skill points
		f >> sp;

		SetRequiredPoints();
	}
	else
	{
		// skill points
		int old_sp[(int)OldSkill::MAX];
		f >> old_sp;
		for(int i = 0; i<(int)Skill::MAX; ++i)
			sp[i] = 0;
		sp[(int)Skill::ONE_HANDED_WEAPON] = old_sp[(int)OldSkill::WEAPON];
		sp[(int)Skill::BOW] = old_sp[(int)OldSkill::BOW];
		sp[(int)Skill::SHIELD] = old_sp[(int)OldSkill::SHIELD];
		sp[(int)Skill::LIGHT_ARMOR] = old_sp[(int)OldSkill::LIGHT_ARMOR];
		sp[(int)Skill::HEAVY_ARMOR] = old_sp[(int)OldSkill::HEAVY_ARMOR];
		// skip skill need
		f.Skip(5 * sizeof(int));
		// attribute points (str, end, dex)
		for(int i = 0; i < 3; ++i)
			f >> ap[i];
		for(int i = 3; i < 6; ++i)
			ap[i] = 0;
		// skip attribute need
		f.Skip(3 * sizeof(int));

		// skip old version trainage data
		// __int64 spg[(int)Skill::MAX][T_MAX], apg[(int)Attribute::MAX][T_MAX];
		// T_MAX = 4
		// Skill::MAX = 5
		// Attribute::MAX = 3
		// size = sizeof(__int64) * T_MAX * (S_MAX + A_MAX)
		// size = 8 * 4 * (5 + 3) = 256
		f.Skip(256);

		// SetRequiredPoints called from Unit::Load after setting new attributes/skills
	}
	ReadFile(file, &klawisz, sizeof(klawisz), &tmp, NULL);
	ReadFile(file, &po_akcja, sizeof(po_akcja), &tmp, NULL);
	ReadFile(file, &po_akcja_idx, sizeof(po_akcja_idx), &tmp, NULL);
	ReadFile(file, &ostatnia, sizeof(ostatnia), &tmp, NULL);
	if(LOAD_VERSION == V_0_2)
	{
		bool resting;
		ReadFile(file, &resting, sizeof(resting), &tmp, NULL);
	}
	if(LOAD_VERSION < V_0_2_20)
	{
		// stary raise_timer, teraz jest w Unit
		float raise_timer;
		ReadFile(file, &raise_timer, sizeof(raise_timer), &tmp, NULL);
	}
	ReadFile(file, &kredyt, sizeof(kredyt), &tmp, NULL);
	ReadFile(file, &godmode, sizeof(godmode), &tmp, NULL);
	ReadFile(file, &noclip, sizeof(noclip), &tmp, NULL);
	ReadFile(file, &id, sizeof(id), &tmp, NULL);
	ReadFile(file, &free_days, sizeof(free_days), &tmp, NULL);
	if(LOAD_VERSION == V_0_2)
		kills = 0;
	else
		ReadFile(file, &kills, sizeof(kills), &tmp, NULL);
	if(LOAD_VERSION < V_0_2_10)
	{
		knocks = 0;
		dmg_done = 0;
		dmg_taken = 0;
		arena_fights = 0;
	}
	else
	{
		ReadFile(file, &knocks, sizeof(knocks), &tmp, NULL);
		ReadFile(file, &dmg_done, sizeof(dmg_done), &tmp, NULL);
		ReadFile(file, &dmg_taken, sizeof(dmg_taken), &tmp, NULL);
		ReadFile(file, &arena_fights, sizeof(arena_fights), &tmp, NULL);
	}
	if(LOAD_VERSION >= V_DEVEL)
	{
		base_stats.Load(f);
		f >> attrib_state;
		f >> skill_state;
		// perks
		byte count;
		f >> count;
		perks.resize(count);
		for(TakenPerk& tp : perks)
		{
			tp.perk = (Perk)f.Read<byte>();
			f >> tp.value;
		}
	}

	action = Action_None;
}

//=================================================================================================
void PlayerController::SetRequiredPoints()
{
	for(int i = 0; i < (int)Attribute::MAX; ++i)
		an[i] = GetRequiredAttributePoints(unit->unmod_stats.attrib[i]);
	for(int i = 0; i < (int)Skill::MAX; ++i)
		sn[i] = GetRequiredSkillPoints(unit->unmod_stats.skill[i]);
}

const float level_mod[21] = {
	0.5f, // -10
	0.55f, // -9
	0.64f, // -8
	0.72f, // -7
	0.79f, // -6
	0.85f, // -5
	0.9f, // -4
	0.94f, // -3
	0.97f, // -2
	0.99f, // -1
	1.f, // 0
	1.01f, // 1
	1.03f, // 2
	1.06f, //3
	1.1f, // 4
	1.15f, // 5
	1.21f, // 6
	1.28f, // 7
	1.36f, // 8
	1.45f, // 9
	1.5f, // 10
};

inline float GetLevelMod(int my_level, int target_level)
{
	return level_mod[clamp(my_level - target_level + 10, 0, 20)];
}

//=================================================================================================
void PlayerController::Train(TrainWhat what, float value, int level)
{
	switch(what)
	{
	case TrainWhat::TakeDamage:
		TrainMod(Attribute::END, value * 2500 * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::TakeDamageArmor:
		if(unit->HaveArmor())
			TrainMod(unit->GetArmor().skill, value * 2000 * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::AttackStart:
		{
			const float c_points = 50.f;
			const Weapon& weapon = unit->GetWeapon();
			const WeaponTypeInfo& info = weapon.GetInfo();
			if(weapon.req_str > unit->Get(Attribute::STR))
				TrainMod(Attribute::STR, c_points);
			TrainMod(Skill::ONE_HANDED_WEAPON, c_points);
			TrainMod2(weapon.GetInfo().skill, c_points);
			TrainMod(Attribute::STR, c_points * info.str2dmg);
			TrainMod(Attribute::DEX, c_points * info.dex2dmg);
		}		
		break;
	case TrainWhat::AttackNoDamage:
		{
			const float c_points = 150.f;
			const Weapon& weapon = unit->GetWeapon();
			const WeaponTypeInfo& info = weapon.GetInfo();
			TrainMod(Skill::ONE_HANDED_WEAPON, c_points);
			TrainMod2(weapon.GetInfo().skill, c_points);
			TrainMod(Attribute::STR, c_points * info.str2dmg);
			TrainMod(Attribute::DEX, c_points * info.dex2dmg);
		}
		break;
	case TrainWhat::AttackHit:
		{
			const float c_points = 2450.f * value;
			const Weapon& weapon = unit->GetWeapon();
			const WeaponTypeInfo& info = weapon.GetInfo();
			TrainMod(Skill::ONE_HANDED_WEAPON, c_points);
			TrainMod2(weapon.GetInfo().skill, c_points);
			TrainMod(Attribute::STR, c_points * info.str2dmg);
			TrainMod(Attribute::DEX, c_points * info.dex2dmg);
		}
		break;
	case TrainWhat::BlockBullet:
	case TrainWhat::BlockAttack:
		TrainMod(Skill::SHIELD, value * 2000 * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::BashStart:
		if(unit->GetShield().req_str > unit->Get(Attribute::STR))
			TrainMod(Attribute::STR, 50.f);
		TrainMod(Skill::SHIELD, 50.f);
		break;
	case TrainWhat::BashNoDamage:
		TrainMod(Skill::SHIELD, 150.f * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::BashHit:
		TrainMod(Skill::SHIELD, value * 1950 * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::BowStart:
		if(unit->GetBow().req_str > unit->Get(Attribute::STR))
			TrainMod(Attribute::STR, 50.f);
		TrainMod(Skill::BOW, 50.f);
		break;
	case TrainWhat::BowNoDamage:
		TrainMod(Skill::BOW, 150.f * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::BowAttack:
		TrainMod(Skill::BOW, 2450.f * value * GetLevelMod(unit->level, level));
		break;
	case TrainWhat::Move:
		{
			int dex, str;

			switch(unit->GetLoadState())
			{
			case 0: // brak
				dex = 49;
				str = 1;
				break;
			case 1: // lekkie
				dex = 40;
				str = 10;
				break;
			case 2: // �rednie
				dex = 25;
				str = 25;
				break;
			case 3: // ci�kie
				dex = 10;
				str = 40;
				break;
			case 4: // maksymalne
			default:
				dex = 1;
				str = 49;
				break;
			}

			if(unit->HaveArmor())
			{
				const Armor& armor = unit->GetArmor();
				if(armor.req_str > unit->Get(Attribute::STR))
					str += 50;
				TrainMod(armor.skill, 50.f);
			}
			
			TrainMod(Attribute::STR, (float)str);
			TrainMod(Attribute::DEX, (float)dex);
		}
		break;
	case TrainWhat::Talk:
		TrainMod(Attribute::CHA, 10.f);
		break;
	case TrainWhat::Trade:
		TrainMod(Attribute::CHA, 100.f);
		break;
	default:
		assert(0);
		break;
	}
}

//=================================================================================================
void PlayerController::Write(BitStream& s)
{
	s.Write(kills);
	s.Write(dmg_done);
	s.Write(dmg_taken);
	s.Write(knocks);
	s.Write(arena_fights);
	base_stats.Write(s);
	s.WriteCasted<byte>(perks.size());
	for(TakenPerk& perk : perks)
	{
		s.WriteCasted<byte>(perk.perk);
		s.Write(perk.value);
	}	
}

//=================================================================================================
bool PlayerController::Read(BitStream& s)
{
	byte count;
	if(!s.Read(kills) ||
		!s.Read(dmg_done) ||
		!s.Read(dmg_taken) ||
		!s.Read(knocks) ||
		!s.Read(arena_fights) ||
		!base_stats.Read(s) ||
		!s.Read(count))
		return false;
	perks.resize(count);
	for(TakenPerk& perk : perks)
	{
		if(!s.ReadCasted<byte>(perk.perk) ||
			!s.Read(perk.value))
			return false;
	}
	return true;
}
