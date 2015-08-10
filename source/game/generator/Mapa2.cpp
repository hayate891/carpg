#include "Pch.h"
#include "Base.h"
#include "mapa2.h"

//-----------------------------------------------------------------------------
// Kod b��du
#define ZLE_DANE 1
#define BRAK_MIEJSCA 2

//-----------------------------------------------------------------------------
namespace Mapa
{
	//-----------------------------------------------------------------------------
	// Kierunek
	enum DIR
	{
		DOL,
		LEWO,
		GORA,
		PRAWO,
		BRAK
	};

	//-----------------------------------------------------------------------------
	// Odwrotno�� kierunku
	const DIR odwrotnosc[5] = {
		GORA,
		PRAWO,
		DOL,
		LEWO,
		BRAK
	};

	//-----------------------------------------------------------------------------
	// Przesuni�cie dla tego kierunku
	const INT2 kierunek[5] = {
		INT2(0,-1), // Dӣ
		INT2(-1,0), // LEWO
		INT2(0,1), // G�RA
		INT2(1,0), // PRAWO
		INT2(0,0) // BRAK
	};

	Pole* mapa;
	OpcjeMapy* opcje;
	vector<int> wolne;

	//-----------------------------------------------------------------------------
	// Opcje dodawania pokoju
	enum DODAJ
	{
		NIE_DODAWAJ,
		DODAJ_POKOJ,
		DODAJ_KORYTARZ
	};

	//-----------------------------------------------------------------------------
	// Graficzna reprezentacja pola mapy
	const char znak[] = {
		' ', // NIEUZYTE
		'_', // PUSTE
		'<', // SCHODY_GORA
		'>', // SCHODY_DOL
		'+', // DRZWI
		'-', // OTWOR_NA_DRZWI
		'/', // KRATKA_PODLOGA
		'^', // KRATKA_SUFIT
		'|', // KRATKA
		'#', // SCIANA
		'@', // BLOKADA
		'$' // BLOKADA_SCIANA
	};

	bool czy_sciana_laczaca(int x, int y, int id1, int id2);
	void dodaj_pokoj(int x, int y, int w, int h, DODAJ dodaj, int id=-1);
	inline void dodaj_pokoj(const Pokoj& pokoj, int id=-1) { dodaj_pokoj(pokoj.pos.x, pokoj.pos.y, pokoj.size.x, pokoj.size.y, NIE_DODAWAJ, id); }
	void generuj();
	void oznacz_korytarze();
	void polacz_korytarze();
	void polacz_pokoje();
	void rysuj();
	bool sprawdz_pokoj(int x, int y, int w, int h);
	void stworz_korytarz(INT2& pt, DIR dir);
	void szukaj_polaczenia(int x, int y, int id);
	void ustaw_flagi();
	inline void ustaw_sciane(POLE& pole);
	void ustaw_wzor();
	DIR wolna_droga(int id);

#define H(_x,_y) mapa[(_x)+(_y)*opcje->w].co
#define HR(_x,_y) mapa[(_x)+(_y)*opcje->w].pokoj

	inline bool wolne_pole(POLE p)
	{
		return (p == PUSTE || p == KRATKA || p == KRATKA_PODLOGA || p == KRATKA_SUFIT);
	}

	//=================================================================================================
	// Sprawdza czy dana �ciana ��czy dwa pokoje (NS)
	//=================================================================================================
	bool czy_sciana_laczaca(int x, int y, int id1, int id2)
	{
		if(mapa[x-1+y*opcje->w].pokoj == id1 && wolne_pole(mapa[x-1+y*opcje->w].co))
		{
			if(mapa[x+1+y*opcje->w].pokoj == id2 && wolne_pole(mapa[x+1+y*opcje->w].co))
				return true;
		}
		else if(mapa[x-1+y*opcje->w].pokoj == id2 && wolne_pole(mapa[x-1+y*opcje->w].co))
		{
			if(mapa[x+1+y*opcje->w].pokoj == id1 && wolne_pole(mapa[x+1+y*opcje->w].co))
				return true;
		}
		if(mapa[x+(y-1)*opcje->w].pokoj == id1 && wolne_pole(mapa[x+(y-1)*opcje->w].co))
		{
			if(mapa[x+(y+1)*opcje->w].pokoj == id2 && wolne_pole(mapa[x+(y+1)*opcje->w].co))
				return true;
		}
		else if(mapa[x+(y-1)*opcje->w].pokoj == id2 && wolne_pole(mapa[x+(y-1)*opcje->w].co))
		{
			if(mapa[x+(y+1)*opcje->w].pokoj == id1 && wolne_pole(mapa[x+(y+1)*opcje->w].co))
				return true;
		}
		return false;
	}

	//=================================================================================================
	// Dodaje pok�j/korytarz (NS)
	//=================================================================================================
	void dodaj_pokoj(int x, int y, int w, int h, DODAJ dodaj, int _id)
	{
		assert(x >= 0 && y >= 0 && w >= 3 && h >= 3 && x+w < int(opcje->w) && y+h < int(opcje->h));

		int id;
		if(dodaj == NIE_DODAWAJ)
			id = _id;
		else
			id = (int)opcje->pokoje->size();

		for(int yy=y+1; yy<y+h-1; ++yy)
		{
			for(int xx=x+1; xx<x+w-1; ++xx)
			{
				assert(H(xx,yy) == NIEUZYTE);
				H(xx,yy) = PUSTE;
				HR(xx,yy) = (word)id;
			}
		}

		for(int i=0; i<w; ++i)
		{
			ustaw_sciane(H(x+i,y));
			HR(x+i,y) = (word)id;
			ustaw_sciane(H(x+i,y+h-1));
			HR(x+i,y+h-1) = (word)id;
		}

		for(int i=0; i<h; ++i)
		{
			ustaw_sciane(H(x,y+i));
			HR(x,y+i) = (word)id;
			ustaw_sciane(H(x+w-1,y+i));
			HR(x+w-1,y+i) = (word)id;
		}

		for(int i=1; i<w-1; ++i)
		{
			if(wolna_droga(x+i+y*opcje->w) != BRAK)
				wolne.push_back(x+i+y*opcje->w);
			if(wolna_droga(x+i+(y+h-1)*opcje->w) != BRAK)
				wolne.push_back(x+i+(y+h-1)*opcje->w);
		}
		for(int i=1; i<h-1; ++i)
		{
			if(wolna_droga(x+(y+i)*opcje->w) != BRAK)
				wolne.push_back(x+(y+i)*opcje->w);
			if(wolna_droga(x+w-1+(y+i)*opcje->w) != BRAK)
				wolne.push_back(x+w-1+(y+i)*opcje->w);
		}

		if(dodaj != NIE_DODAWAJ)
		{
			Pokoj& pokoj = Add1(*opcje->pokoje);
			pokoj.pos.x = x;
			pokoj.pos.y = y;
			pokoj.size.x = w;
			pokoj.size.y = h;
			pokoj.korytarz = (dodaj == DODAJ_KORYTARZ);
			pokoj.cel = POKOJ_CEL_BRAK;
		}
	}

	//=================================================================================================
	// Generuje podziemia
	//=================================================================================================
	void generuj()
	{
		// ustaw wz�r obszaru (np wype�nij obszar nieu�ytkiem, na oko�o blokada)
		ustaw_wzor();

		// je�li s� jakie� pocz�tkowe pokoje to je dodaj
		if(!opcje->pokoje->empty())
		{
			int index = 0;
			for(vector<Pokoj>::iterator it = opcje->pokoje->begin(), end = opcje->pokoje->end(); it != end; ++it, ++index)
				dodaj_pokoj(*it, index);
		}
		else
		{
			// stw�rz pocz�tkowy pok�j
			int w = random2(opcje->rozmiar_pokoj),
				h = random2(opcje->rozmiar_pokoj);
			dodaj_pokoj((opcje->w-w)/2, (opcje->h-h)/2, w, h, DODAJ_POKOJ);
		}

		// generuj
		while(!wolne.empty())
		{
			int idx = rand2()%wolne.size();
			int id = wolne[idx];
			if(idx != wolne.size()-1)
				std::iter_swap(wolne.begin()+idx, wolne.end()-1);
			wolne.pop_back();

			// sprawd� czy to miejsce jest dobre
			DIR dir = wolna_droga(id);
			if(dir == BRAK)
				continue;

			INT2 pt(id%opcje->w, id/opcje->w);

			// pok�j czy korytarz
			if(rand2()%100 < opcje->korytarz_szansa)
				stworz_korytarz(pt, dir);
			else
			{
				int w = random2(opcje->rozmiar_pokoj),
					h = random2(opcje->rozmiar_pokoj);

				if(dir == LEWO)
				{
					pt.x -= w-1;
					pt.y -= h/2;
				}
				else if(dir == PRAWO)
					pt.y -= h/2;
				else if(dir == GORA)
					pt.x -= w/2;
				else
				{
					pt.x -= w/2;
					pt.y -= h-1;
				}

				if(sprawdz_pokoj(pt.x, pt.y, w, h))
				{
					mapa[id].co = DRZWI;
					dodaj_pokoj(pt.x, pt.y, w, h, DODAJ_POKOJ);
				}
			}

			//rysuj();
			//_getch();
		}

		// szukaj po��cze� pomi�dzy pokojami/korytarzami
		int index = 0;
		for(vector<Pokoj>::iterator it = opcje->pokoje->begin(), end = opcje->pokoje->end(); it != end; ++it, ++index)
		{
			for(int x=1; x<it->size.x-1; ++x)
			{
				szukaj_polaczenia(it->pos.x+x, it->pos.y, index);
				szukaj_polaczenia(it->pos.x+x, it->pos.y+it->size.y-1, index);
			}
			for(int y=1; y<it->size.y-1; ++y)
			{
				szukaj_polaczenia(it->pos.x, it->pos.y+y, index);
				szukaj_polaczenia(it->pos.x+it->size.x-1, it->pos.y+y, index);
			}
		}

		// po��cz korytarze
		if(opcje->polacz_korytarz > 0)
			polacz_korytarze();
		else
			oznacz_korytarze();

		// losowe dziury
		if(opcje->kraty_szansa > 0)
		{
			for(int y = 1; y<opcje->w-1; ++y)
			{
				for(int x = 1; x<opcje->h-1; ++x)
				{
					Pole& p = mapa[x+y*opcje->w];
					if(p.co == PUSTE && rand2() % 100 < opcje->kraty_szansa)
					{
						if(!IS_SET(p.flagi, Pole::F_NISKI_SUFIT))
						{
							int j = rand2()%3;
							if(j == 0)
								p.co = KRATKA_PODLOGA;
							else if(j == 1)
								p.co = KRATKA_SUFIT;
							else
								p.co = KRATKA;
						}
						else if(rand2() % 3 == 0)
							p.co = KRATKA_PODLOGA;
					}
				}
			}
		}

		if(opcje->stop)
			return;

		// po��cz pokoje
		if(opcje->polacz_pokoj > 0)
			polacz_pokoje();

		// generowanie schod�w
		if(opcje->schody_dol != OpcjeMapy::BRAK || opcje->schody_gora != OpcjeMapy::BRAK)
			generuj_schody(*opcje);

		// generuj flagi p�l
		ustaw_flagi();
	}

	//=================================================================================================
	// Oznacza pola na mapie niskim sufitem tam gdzie jest korytarz
	//=================================================================================================
	void oznacz_korytarze()
	{
		for(vector<Pokoj>::iterator it = opcje->pokoje->begin(), end = opcje->pokoje->end(); it != end; ++it)
		{
			if(!it->korytarz)
				continue;

			for(int y=0; y<it->size.y; ++y)
			{
				for(int x=0; x<it->size.x; ++x)
				{
					Pole& p = mapa[x+it->pos.x+(y+it->pos.y)*opcje->w];
					if(p.co == PUSTE || p.co == DRZWI || p.co == KRATKA_PODLOGA)
						p.flagi = Pole::F_NISKI_SUFIT;
				}
			}
		}
	}

	//=================================================================================================
	// ��czy korytarze ze sob�
	//=================================================================================================
	void polacz_korytarze()
	{
		int index = 0;
		for(vector<Pokoj>::iterator it = opcje->pokoje->begin(), end = opcje->pokoje->end(); it != end; ++it, ++index)
		{
			if(!it->korytarz)
				continue;

			Pokoj& p = *it;

			for(vector<int>::iterator it2 = p.polaczone.begin(), end2 = p.polaczone.end(); it2 != end2; ++it2)
			{
				Pokoj& p2 = opcje->pokoje->at(*it2);

				if(!p2.korytarz || rand2()%100 >= opcje->polacz_korytarz)
					continue;

				int x1 = max(p.pos.x,p2.pos.x),
					x2 = min(p.pos.x+p.size.x,p2.pos.x+p2.size.x),
					y1 = max(p.pos.y,p2.pos.y),
					y2 = min(p.pos.y+p.size.y,p2.pos.y+p2.size.y);

				assert(x1 < x2 && y1 < y2);

				for(int y=y1; y<y2; ++y)
				{
					for(int x=x1; x<x2; ++x)
					{
						Pole& po = mapa[x+y*opcje->w];
						if(po.co == DRZWI)
						{
							assert(po.pokoj == index || po.pokoj == *it2);
							po.co = PUSTE;
							goto usunieto_drzwi;
						}
					}
				}

				// brak po��czenia albo zosta�o ju� usuni�te

usunieto_drzwi:
				continue;
			}

			// oznacz niski sufit
			for(int y=0; y<it->size.y; ++y)
			{
				for(int x=0; x<it->size.x; ++x)
				{
					Pole& p = mapa[x+it->pos.x+(y+it->pos.y)*opcje->w];
					if(p.co == PUSTE || p.co == DRZWI || p.co == KRATKA_PODLOGA)
						p.flagi = Pole::F_NISKI_SUFIT;
				}
			}
		}
	}

	//=================================================================================================
	// Usuwa r�g z zaznaczenia przy ��czeniu pokoi
	//=================================================================================================
	inline void usun_rog(int& x1, int& x2, int& y1, int& y2, int x, int y)
	{
		if(x1 == x2)
		{
			// | y1
			// |
			// |
			// | y2
			// x1/x2
			if(x1 == x)
			{
				if(y1 == y)
					++y1;
				else if(y2 == y)
					--y2;
			}
		}
		else
		{
			// --------- y1/y2
			// x1      x2
			if(y1 == y)
			{
				if(x1 == x)
					++x1;
				else if(x2 == x)
					--x2;
			}
		}
	}

	//=================================================================================================
	// ��czy pokoje ze sob�
	//=================================================================================================
	void polacz_pokoje()
	{
		int index = 0;
		for(vector<Pokoj>::iterator it = opcje->pokoje->begin(), end = opcje->pokoje->end(); it != end; ++it, ++index)
		{
			if(it->korytarz || it->cel == POKOJ_CEL_SKARBIEC || it->cel == POKOJ_CEL_WIEZIENIE || it->cel == POKOJ_CEL_TRON || it->cel == POKOJ_CEL_PORTAL || it->cel == POKOJ_CEL_PORTAL_STWORZ)
				continue;

			Pokoj& p = *it;

			for(vector<int>::iterator it2 = p.polaczone.begin(), end2 = p.polaczone.end(); it2 != end2; ++it2)
			{
				Pokoj& p2 = opcje->pokoje->at(*it2);

				if(p2.korytarz || p2.cel == POKOJ_CEL_SKARBIEC || p2.cel == POKOJ_CEL_WIEZIENIE || p2.cel == POKOJ_CEL_TRON || p2.cel == POKOJ_CEL_PORTAL || p2.cel == POKOJ_CEL_PORTAL_STWORZ ||
					rand2()%100 >= opcje->polacz_pokoj)
				{
					continue;
				}

				// znajd� wsp�lny obszar
				int x1 = max(p.pos.x,p2.pos.x),
					x2 = min(p.pos.x+p.size.x,p2.pos.x+p2.size.x),
					y1 = max(p.pos.y,p2.pos.y),
					y2 = min(p.pos.y+p.size.y,p2.pos.y+p2.size.y);

				if(x1 == 0)
					++x1;
				if(y1 == 0)
					++y1;
				if(x2 == opcje->w-1)
					--x2;
				if(y2 == opcje->h-1)
					--y2;

				// usu� rogi
				/*usun_rog(x1, x2, y1, y2, p.pos.x, p.pos.y);
				usun_rog(x1, x2, y1, y2, p.pos.x+p.size.x, p.pos.y);
				usun_rog(x1, x2, y1, y2, p.pos.x, p.pos.y+p.size.y);
				usun_rog(x1, x2, y1, y2, p.pos.x+p.size.x, p.pos.y+p.size.y);
				usun_rog(x1, x2, y1, y2, p2.pos.x, p2.pos.y);
				usun_rog(x1, x2, y1, y2, p2.pos.x+p2.size.x, p2.pos.y);
				usun_rog(x1, x2, y1, y2, p2.pos.x, p2.pos.y+p2.size.y);
				usun_rog(x1, x2, y1, y2, p2.pos.x+p2.size.x, p2.pos.y+p2.size.y);*/

				assert(x1 < x2 && y1 < y2);

				for(int y=y1; y<y2; ++y)
				{
					for(int x=x1; x<x2; ++x)
					{
						if(czy_sciana_laczaca(x, y, index, *it2))
						{
							Pole& po = mapa[x+y*opcje->w];
							if(po.co == SCIANA || po.co == DRZWI)
							{
								po.co = PUSTE;
							}
						}
					}
				}
			}
		}
	}

	//=================================================================================================
	// Rysuje podziemia
	//=================================================================================================
	void rysuj()
	{
		for(int y=opcje->h-1; y>=0; --y)
		{
			for(int x=0; x<opcje->w; ++x)
				putchar(znak[mapa[x+y*opcje->w].co]);
			putchar('\n');
		}
	}

	//=================================================================================================
	// Sprawdza czy w tym miejscu mo�na doda� korytarz
	//=================================================================================================
	bool sprawdz_pokoj(int x, int y, int w, int h)
	{
		if(!(x >= 0 && y >= 0 && w >= 3 && h >= 3 && x+w < int(opcje->w) && y+h < int(opcje->h)))
			return false;

		for(int yy=y+1; yy<y+h-1; ++yy)
		{
			for(int xx=x+1; xx<x+w-1; ++xx)
			{
				if(H(xx,yy) != NIEUZYTE)
					return false;
			}
		}

		for(int i=0; i<w; ++i)
		{
			POLE p = H(x+i,y);
			if(p != NIEUZYTE && p != SCIANA && p != BLOKADA && p != BLOKADA_SCIANA)
				return false;
			p = H(x+i,y+h-1);
			if(p != NIEUZYTE && p != SCIANA && p != BLOKADA && p != BLOKADA_SCIANA)
				return false;
		}

		for(int i=0; i<h; ++i)
		{
			POLE p = H(x,y+i);
			if(p != NIEUZYTE && p != SCIANA && p != BLOKADA && p != BLOKADA_SCIANA)
				return false;
			p = H(x+w-1,y+i);
			if(p != NIEUZYTE && p != SCIANA && p != BLOKADA && p != BLOKADA_SCIANA)
				return false;
		}

		return true;
	}

	//=================================================================================================
	// Tworzy korytarz
	//=================================================================================================
	void stworz_korytarz(INT2& _pt, DIR dir)
	{
		int dl = random2(opcje->rozmiar_korytarz);
		int w, h;

		INT2 pt(_pt);

		if(dir == LEWO)
		{
			pt.x -= dl-1;
			pt.y -= 1;
			w = dl;
			h = 3;
		}
		else if(dir == PRAWO)
		{
			pt.y -= 1;
			w = dl;
			h = 3;
		}
		else if(dir == GORA)
		{
			pt.x -= 1;
			w = 3;
			h = dl;
		}
		else
		{
			pt.y -= dl-1;
			pt.x -= 1;
			w = 3;
			h = dl;
		}

		if(sprawdz_pokoj(pt.x, pt.y, w, h))
		{
			H(_pt.x,_pt.y) = DRZWI;
			dodaj_pokoj(pt.x, pt.y, w, h, DODAJ_KORYTARZ);
		}
	}

	//=================================================================================================
	// Szuka po��cza� pomi�dzy pomieszczeniami
	//=================================================================================================
	void szukaj_polaczenia(int x, int y, int id)
	{
		Pokoj& p = opcje->pokoje->at(id);

		if(H(x,y) == DRZWI)
		{
			if(x>0 && H(x-1,y) == PUSTE)
			{
				int to_id = HR(x-1,y);
				if(to_id != id)
				{
					bool jest = false;
					for(vector<int>::iterator it = p.polaczone.begin(), end = p.polaczone.end(); it != end; ++it)
					{
						if(*it == to_id)
						{
							jest = true;
							break;
						}
					}
					if(!jest)
					{
						p.polaczone.push_back(to_id);
						return;
					}
				}
			}

			if(x<int(opcje->w-1) && H(x+1,y) == PUSTE)
			{
				int to_id = HR(x+1,y);
				if(to_id != id)
				{
					bool jest = false;
					for(vector<int>::iterator it = p.polaczone.begin(), end = p.polaczone.end(); it != end; ++it)
					{
						if(*it == to_id)
						{
							jest = true;
							break;
						}
					}
					if(!jest)
					{
						p.polaczone.push_back(to_id);
						return;
					}
				}
			}

			if(y>0 && H(x,y-1) == PUSTE)
			{
				int to_id = HR(x,y-1);
				if(to_id != id)
				{
					bool jest = false;
					for(vector<int>::iterator it = p.polaczone.begin(), end = p.polaczone.end(); it != end; ++it)
					{
						if(*it == to_id)
						{
							jest = true;
							break;
						}
					}
					if(!jest)
					{
						p.polaczone.push_back(to_id);
						return;
					}
				}
			}

			if(y<int(opcje->h-1) && H(x,y+1) == PUSTE)
			{
				int to_id = HR(x,y+1);
				if(to_id != id)
				{
					bool jest = false;
					for(vector<int>::iterator it = p.polaczone.begin(), end = p.polaczone.end(); it != end; ++it)
					{
						if(*it == to_id)
						{
							jest = true;
							break;
						}
					}
					if(!jest)
					{
						p.polaczone.push_back(to_id);
						return;
					}
				}
			}
		}
	}

	void ustaw_flagi()
	{
		for(int y=0; y<opcje->h; ++y)
		{
			for(int x=0; x<opcje->w; ++x)
			{
				Pole& p = mapa[x+y*opcje->w];
				if(p.co != PUSTE && p.co != DRZWI && p.co != KRATKA && p.co != KRATKA_PODLOGA && p.co != KRATKA_SUFIT && p.co != SCHODY_DOL)
					continue;

				// pod�oga
				if(p.co == KRATKA || p.co == KRATKA_PODLOGA)
				{
					p.flagi |= Pole::F_KRATKA_PODLOGA;

					if(!OR2_EQ(mapa[x-1+y*opcje->w].co, KRATKA, KRATKA_PODLOGA))
						p.flagi |= Pole::F_DZIURA_PRAWA;
					if(!OR2_EQ(mapa[x+1+y*opcje->w].co, KRATKA, KRATKA_PODLOGA))
						p.flagi |= Pole::F_DZIURA_LEWA;
					if(!OR2_EQ(mapa[x+(y-1)*opcje->w].co, KRATKA, KRATKA_PODLOGA))
						p.flagi |= Pole::F_DZIURA_TYL;
					if(!OR2_EQ(mapa[x+(y+1)*opcje->w].co, KRATKA, KRATKA_PODLOGA))
						p.flagi |= Pole::F_DZIURA_PRZOD;
				}
				else if(p.co != SCHODY_DOL)
					p.flagi |= Pole::F_PODLOGA;

				if(p.co == KRATKA || p.co == KRATKA_SUFIT)
				{
					assert(!IS_SET(p.flagi, Pole::F_NISKI_SUFIT));
				}

				if(!IS_SET(p.flagi, Pole::F_NISKI_SUFIT))
				{
					if(IS_SET(mapa[x-1+y*opcje->w].flagi, Pole::F_NISKI_SUFIT))
						p.flagi |= Pole::F_PODSUFIT_PRAWA;
					if(IS_SET(mapa[x+1+y*opcje->w].flagi, Pole::F_NISKI_SUFIT))
						p.flagi |= Pole::F_PODSUFIT_LEWA;
					if(IS_SET(mapa[x+(y-1)*opcje->w].flagi, Pole::F_NISKI_SUFIT))
						p.flagi |= Pole::F_PODSUFIT_TYL;
					if(IS_SET(mapa[x+(y+1)*opcje->w].flagi, Pole::F_NISKI_SUFIT))
						p.flagi |= Pole::F_PODSUFIT_PRZOD;

					// dziura w suficie
					if(p.co == KRATKA || p.co == KRATKA_SUFIT)
					{
						p.flagi |= Pole::F_KRATKA_SUFIT;

						if(!OR2_EQ(mapa[x-1+y*opcje->w].co, KRATKA, KRATKA_SUFIT))
							p.flagi |= Pole::F_GORA_PRAWA;
						if(!OR2_EQ(mapa[x+1+y*opcje->w].co, KRATKA, KRATKA_SUFIT))
							p.flagi |= Pole::F_GORA_LEWA;
						if(!OR2_EQ(mapa[x+(y-1)*opcje->w].co, KRATKA, KRATKA_SUFIT))
							p.flagi |= Pole::F_GORA_TYL;
						if(!OR2_EQ(mapa[x+(y+1)*opcje->w].co, KRATKA, KRATKA_SUFIT))
							p.flagi |= Pole::F_GORA_PRZOD;
					}
					else
					{
						// normalny sufit w tym miejscu
						p.flagi |= Pole::F_SUFIT;
					}
				}

				// �ciany
				if(OR2_EQ(mapa[x-1+y*opcje->w].co, SCIANA, BLOKADA_SCIANA))
					p.flagi |= Pole::F_SCIANA_PRAWA;
				if(OR2_EQ(mapa[x+1+y*opcje->w].co, SCIANA, BLOKADA_SCIANA))
					p.flagi |= Pole::F_SCIANA_LEWA;
				if(OR2_EQ(mapa[x+(y-1)*opcje->w].co, SCIANA, BLOKADA_SCIANA))
					p.flagi |= Pole::F_SCIANA_TYL;
				if(OR2_EQ(mapa[x+(y+1)*opcje->w].co, SCIANA, BLOKADA_SCIANA))
					p.flagi |= Pole::F_SCIANA_PRZOD;
			}
		}
	}

	//=================================================================================================
	// Ustawia �cian� na danym polu
	//=================================================================================================
	inline void ustaw_sciane(POLE& pole)
	{
		assert(pole == NIEUZYTE || pole == SCIANA || pole == BLOKADA || pole == BLOKADA_SCIANA || pole == DRZWI);

		if(pole == NIEUZYTE)
			pole = SCIANA;
		else if(pole == BLOKADA)
			pole = BLOKADA_SCIANA;
	}

	//=================================================================================================
	// Ustawia wz�r podziemi
	//=================================================================================================
	void ustaw_wzor()
	{
		memset(mapa, 0, sizeof(Pole)*opcje->w*opcje->h);

		if(opcje->ksztalt == OpcjeMapy::PROSTOKAT)
		{
			for(int x=0; x<opcje->w; ++x)
			{
				H(x,0) = BLOKADA;
				H(x,opcje->h-1) = BLOKADA;
			}

			for(int y=0; y<opcje->h; ++y)
			{
				H(0,y) = BLOKADA;
				H(opcje->w-1,y) = BLOKADA;
			}
		}
		else if(opcje->ksztalt == OpcjeMapy::OKRAG)
		{
			int w = (opcje->w-3)/2,
				h = (opcje->h-3)/2;

			for(int y=0; y<opcje->h; ++y)
			{
				for(int x=0; x<opcje->w; ++x)
				{
					if(distance(float(x-1)/w, float(y-1)/h, 1.f, 1.f) > 1.f)
						mapa[x+y*opcje->w].co = BLOKADA;
				}
			}
		}
		else
		{
			assert(0);
		}
	}

	//=================================================================================================
	// Sprawdza czy w danym punkcie mo�na doda� wej�cie do korytarza/pokoju
	//=================================================================================================
	DIR wolna_droga(int id)
	{
		int x = id%opcje->w,
			y = id/opcje->w;
		if(x == 0 || y == 0 || x == opcje->w-1 || y == opcje->h-1)
			return BRAK;

		DIR jest = BRAK;
		int ile = 0;

		if(mapa[id-1].co == PUSTE)
		{
			++ile;
			jest = LEWO;
		}
		if(mapa[id+1].co == PUSTE)
		{
			++ile;
			jest = PRAWO;
		}
		if(mapa[id+opcje->w].co == PUSTE)
		{
			++ile;
			jest = GORA;
		}
		if(mapa[id-opcje->w].co == PUSTE)
		{
			++ile;
			jest = DOL;
		}

		if(ile != 1)
			return BRAK;

		const INT2& od = kierunek[odwrotnosc[jest]];
		if(mapa[id+od.x+od.y*opcje->w].co != NIEUZYTE)
			return BRAK;

		return odwrotnosc[jest];
	}

#undef H
#undef HR
};

//=================================================================================================
// Generuje map� podziemi
//=================================================================================================
bool generuj_mape2(OpcjeMapy& _opcje, bool recreate)
{
	// sprawd� opcje
	assert(_opcje.w && _opcje.h && _opcje.rozmiar_pokoj.x >= 4 && _opcje.rozmiar_pokoj.y >= _opcje.rozmiar_pokoj.x && 
		in_range(_opcje.korytarz_szansa, 0, 100) && _opcje.pokoje);
	assert(_opcje.korytarz_szansa == 0 || (_opcje.rozmiar_korytarz.x >= 3 && _opcje.rozmiar_korytarz.y >= _opcje.rozmiar_korytarz.x));

	if(!recreate)
		Mapa::mapa = new Pole[_opcje.w * _opcje.h];
	Mapa::opcje = &_opcje;
	Mapa::wolne.clear();

	_opcje.blad = 0;
	_opcje.mapa = Mapa::mapa;

	Mapa::generuj();

	if(_opcje.stop)
		return (_opcje.blad == 0);

#ifdef _DEBUG
	Mapa::rysuj();
#endif

	return (_opcje.blad == 0);
}

bool kontynuuj_generowanie_mapy(OpcjeMapy& _opcje)
{
	// po��cz pokoje
	if(_opcje.polacz_pokoj > 0)
		Mapa::polacz_pokoje();

	// generowanie schod�w
	if(_opcje.schody_dol != OpcjeMapy::BRAK || _opcje.schody_gora != OpcjeMapy::BRAK)
		generuj_schody(_opcje);

	// generuj flagi p�l
	Mapa::ustaw_flagi();

#ifdef _DEBUG
	Mapa::rysuj();
#endif

	return (_opcje.blad == 0);
}

// aktualnie najwy�szy priorytet maj� schody w �cianie po �rodku �ciany
struct PosDir
{
	INT2 pos;
	int dir, prio;
	bool w_scianie;

	PosDir(int _x, int _y, int _dir, bool _w_scianie, const Pokoj& _pokoj) : pos(_x,_y), dir(_dir), prio(0), w_scianie(_w_scianie)
	{
		if(w_scianie)
			prio += (_pokoj.size.x+_pokoj.size.y)*2;
		_x -= _pokoj.pos.x;
		_y -= _pokoj.pos.y;
		prio -= abs(_pokoj.size.x/2-_x) + abs(_pokoj.size.y/2-_y);
	}

	inline bool operator < (const PosDir& p)
	{
		return prio > p.prio;
	}
};

bool dodaj_schody(OpcjeMapy& _opcje, Pokoj& _pokoj, INT2& _pozycja, int& _kierunek, POLE _schody, bool& _w_scianie)
{
#define B(_xx,_yy) (czy_blokuje2(_opcje.mapa[x+_xx+(y+_yy)*_opcje.w].co))
#define P(_xx,_yy) (!czy_blokuje21(_opcje.mapa[x+_xx+(y+_yy)*_opcje.w].co))

	static vector<PosDir> wybor;
	wybor.clear();
	// im wi�kszy priorytet tym lepiej

	for(int y=max(1,_pokoj.pos.y); y<min(_opcje.h-1,_pokoj.size.y+_pokoj.pos.y); ++y)
	{
		for(int x=max(1,_pokoj.pos.x); x<min(_opcje.w-1,_pokoj.size.x+_pokoj.pos.x); ++x)
		{
			Pole& p = _opcje.mapa[x+y*_opcje.w];
			if(p.co == PUSTE)
			{
				const bool left = (x>0);
				const bool right = (x<int(_opcje.w-1));
				const bool top = (y<int(_opcje.h-1));
				const bool bottom = (y>0);

				if(left && top)
				{
					// ##
					// #>
					if(B(-1,1) && B(0,1) && B(-1,0))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(3),false,_pokoj));
				}
				if(right && top)
				{
					// ##
					// >#
					if(B(0,1) && B(1,1) && B(1,0))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(1),false,_pokoj));
				}
				if(left && bottom)
				{
					// #>
					// ##
					if(B(-1,0) && B(-1,-1) && B(0,-1))
						wybor.push_back(PosDir(x,y,BIT(2)|BIT(3),false,_pokoj));
				}
				if(right && bottom)
				{
					// <#
					// ##
					if(B(1,0) && B(0,-1) && B(1,-1))
						wybor.push_back(PosDir(x,y,BIT(1)|BIT(2),false,_pokoj));
				}
				if(left && top && bottom)
				{
					// #_
					// #>
					// #_
					if(B(-1,1) && P(0,1) && B(-1,0) && B(-1,-1) && P(0,-1))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(2)|BIT(3),false,_pokoj));
				}
				if(right && top && bottom)
				{
					// _#
					// <#
					// _#
					if(P(0,1) && B(1,1) && B(1,0) && P(0,-1) && B(1,-1))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(1)|BIT(2),false,_pokoj));
				}
				if(top && left && right)
				{
					// ###
					// _>_
					if(B(-1,1) && B(0,1) && B(1,1) && P(-1,0) && P(1,0))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(1)|BIT(3),false,_pokoj));
				}
				if(bottom && left && right)
				{
					// _>_
					// ###
					if(P(-1,0) && P(1,0) && B(-1,-1) && B(0,-1) && B(1,-1))
						wybor.push_back(PosDir(x,y,BIT(1)|BIT(2)|BIT(3),false,_pokoj));
				}
				if(left && right && top && bottom)
				{
					//  ___
					//  _<_
					//  ___
					if(P(-1,-1) && P(0,-1) && P(1,-1) && P(-1,0) && P(1,0) && P(-1,1) && P(0,1) && P(1,1))
						wybor.push_back(PosDir(x,y,BIT(0)|BIT(1)|BIT(2)|BIT(3),false,_pokoj));
				}
			}
			else if((p.co == SCIANA || p.co == BLOKADA_SCIANA) && (x>0) && (x<int(_opcje.w-1)) && (y>0) && (y<int(_opcje.h-1)))
			{
				// ##
				// #>_
				// ##
				if(B(-1,1) && B(0,1) && B(-1,0) && P(1,0) && B(-1,-1) && B(0,-1))
					wybor.push_back(PosDir(x,y,BIT(3),true,_pokoj));

				//  ##
				// _<#
				//  ##
				if(B(0,1) && B(1,1) && P(-1,0) && B(1,0) && B(0,-1) && B(1,-1))
					wybor.push_back(PosDir(x,y,BIT(1),true,_pokoj));

				// ###
				// #>#
				//  _
				if(B(-1,1) && B(0,1) && B(1,1) && B(-1,0) && B(1,0) && P(0,-1))
					wybor.push_back(PosDir(x,y,BIT(0),true,_pokoj));

				//  _
				// #<#
				// ###
				if(P(0,1) && B(-1,0) && B(1,0) && B(-1,-1) && B(0,-1) && B(1,-1))
					wybor.push_back(PosDir(x,y,BIT(2),true,_pokoj));
			}
		}
	}

	if(wybor.empty())
		return false;

	std::sort(wybor.begin(), wybor.end());

	/*for(vector<PosDir>::iterator it = wybor.begin(), end = wybor.end(); it != end; ++it)
	{
		_opcje.mapa[it->pos.x+it->pos.y*_opcje.w].co = SCHODY;
		system("cls");
		Mapa::rysuj();
		OutputDebugString(format("%d, ", it->prio));
		_getch();
	}*/

	int best_prio = wybor.front().prio;
	int ile = 0;

	vector<PosDir>::iterator it = wybor.begin(), end = wybor.end();
	while(it != end && it->prio == best_prio)
	{
		++it;
		++ile;
	}

	PosDir& pd = wybor[rand2()%ile];
	_pozycja = pd.pos;
	_opcje.mapa[pd.pos.x+pd.pos.y*_opcje.w].co = _schody;
	_w_scianie = pd.w_scianie;

	for(int y=max(0,pd.pos.y-1); y<=min(int(_opcje.h),pd.pos.y+1); ++y)
	{
		for(int x=max(0,pd.pos.x-1); x<=min(int(_opcje.w),pd.pos.x+1); ++x)
		{
			POLE& p = _opcje.mapa[x+y*_opcje.w].co;
			if(p == NIEUZYTE)
				p = SCIANA;
			else if(p == BLOKADA)
				p = BLOKADA_SCIANA;
		}
	}

	switch(pd.dir)
	{
	case 0:
		assert(0);
		return false;
	case BIT(0):
		_kierunek = 0;
		break;
	case BIT(1):
		_kierunek = 1;
		break;
	case BIT(2):
		_kierunek = 2;
		break;
	case BIT(3):
		_kierunek = 3;
		break;
	case BIT(0)|BIT(1):
		if(rand2()%2 == 0)
			_kierunek = 0;
		else
			_kierunek = 1;
		break;
	case BIT(0)|BIT(2):
		if(rand2()%2 == 0)
			_kierunek = 0;
		else
			_kierunek = 2;
		break;
	case BIT(0)|BIT(3):
		if(rand2()%2 == 0)
			_kierunek = 0;
		else
			_kierunek = 3;
		break;
	case BIT(1)|BIT(2):
		if(rand2()%2 == 0)
			_kierunek = 1;
		else
			_kierunek = 2;
		break;
	case BIT(1)|BIT(3):
		if(rand2()%2 == 0)
			_kierunek = 1;
		else
			_kierunek = 3;
		break;
	case BIT(2)|BIT(3):
		if(rand2()%2 == 0)
			_kierunek = 2;
		else
			_kierunek = 3;
		break;
	case BIT(0)|BIT(1)|BIT(2):
		{
			int t = rand2()%3;
			if(t == 0)
				_kierunek = 0;
			else if(t == 1)
				_kierunek = 1;
			else
				_kierunek = 2;
		}
		break;
	case BIT(0)|BIT(1)|BIT(3):
		{
			int t = rand2()%3;
			if(t == 0)
				_kierunek = 0;
			else if(t == 1)
				_kierunek = 1;
			else
				_kierunek = 3;
		}
		break;
	case BIT(0)|BIT(2)|BIT(3):
		{
			int t = rand2()%3;
			if(t == 0)
				_kierunek = 0;
			else if(t == 1)
				_kierunek = 2;
			else
				_kierunek = 3;
		}
		break;
	case BIT(1)|BIT(2)|BIT(3):
		{
			int t = rand2()%3;
			if(t == 0)
				_kierunek = 1;
			else if(t == 1)
				_kierunek = 2;
			else
				_kierunek = 3;
		}
		break;
	case BIT(0)|BIT(1)|BIT(2)|BIT(3):
		_kierunek = rand2()%4;
		break;
	default:
		assert(0);
		return false;
	}

	return true;

#undef B
#undef P
}

bool SortujPokoje(INT2& a, INT2& b)
{
	return a.y < b.y;
}

bool generuj_schody2(OpcjeMapy& _opcje, vector<Pokoj*>& _pokoje, OpcjeMapy::GDZIE_SCHODY _gdzie, Pokoj*& _pokoj, INT2& _pozycja, int& _kierunek, bool _gora, bool& _w_scianie)
{
	switch(_gdzie)
	{
	case OpcjeMapy::LOSOWO:
		while(!_pokoje.empty())
		{
			uint id = rand2()%_pokoje.size();
			Pokoj* p = _pokoje.at(id);
			if(id != _pokoje.size()-1)
				std::iter_swap(_pokoje.begin()+id, _pokoje.end()-1);
			_pokoje.pop_back();

			if(dodaj_schody(_opcje, *p, _pozycja, _kierunek, (_gora ? SCHODY_GORA : SCHODY_DOL), _w_scianie))
			{
				p->cel = (_gora ? POKOJ_CEL_SCHODY_GORA : POKOJ_CEL_SCHODY_DOL);
				_pokoj = p;
				return true;
			}
		}
		assert(0);
		_opcje.blad = BRAK_MIEJSCA;
		return false;
	case OpcjeMapy::NAJDALEJ:
		{
			vector<INT2> p;
			INT2 pos = _pokoje[0]->Srodek2();
			int index = 1;
			for(vector<Pokoj*>::iterator it = _pokoje.begin()+1, end = _pokoje.end(); it != end; ++it, ++index)
				p.push_back(INT2(index, distance(pos, (*it)->Srodek2())));
			std::sort(p.begin(), p.end(), SortujPokoje);

			while(!p.empty())
			{
				int p_id = p.back().x;
				p.pop_back();

				if(dodaj_schody(_opcje, *_pokoje[p_id], _pozycja, _kierunek, (_gora ? SCHODY_GORA : SCHODY_DOL), _w_scianie))
				{
					_pokoje[p_id]->cel = (_gora ? POKOJ_CEL_SCHODY_GORA : POKOJ_CEL_SCHODY_DOL);
					_pokoj = _pokoje[p_id];
					return true;
				}
			}

			assert(0);
			_opcje.blad = BRAK_MIEJSCA;
			return false;
		}
		break;
	/*case OpcjeMapy::KIERUNEK:
	case OpcjeMapy::POKOJ:
	case OpcjeMapy::PIERWSZY:
	case OpcjeMapy::NAJDALEJ:
		if(_gora)
		{
			assert(0);
			_opcje.blad = ZLE_DANE;
			return false;
		}
		else
		{
			
		}
		break;
	case OpcjeMapy::POZYCJA:*/
	default:
		assert(0);
		_opcje.blad = ZLE_DANE;
		return false;
	}
}

//=================================================================================================
// Generuje schody
//=================================================================================================
bool generuj_schody(OpcjeMapy& _opcje)
{
	assert((_opcje.schody_dol != OpcjeMapy::BRAK || _opcje.schody_gora != OpcjeMapy::BRAK) && _opcje.pokoje && _opcje.mapa);

	static vector<Pokoj*> pokoje;
	for(vector<Pokoj>::iterator it = _opcje.pokoje->begin(), end = _opcje.pokoje->end(); it != end; ++it)
	{
		if(!it->korytarz && it->cel == POKOJ_CEL_BRAK)
			pokoje.push_back(&*it);
	}

	if(_opcje.schody_gora != OpcjeMapy::BRAK)
	{
		bool w_scianie;
		if(!generuj_schody2(_opcje, pokoje, _opcje.schody_gora, _opcje.schody_gora_pokoj, _opcje.schody_gora_pozycja, _opcje.schody_gora_kierunek, true, w_scianie))
		{
			pokoje.clear();
			return false;
		}
	}

	if(_opcje.schody_dol != OpcjeMapy::BRAK)
	{
		if(!generuj_schody2(_opcje, pokoje, _opcje.schody_dol, _opcje.schody_dol_pokoj, _opcje.schody_dol_pozycja, _opcje.schody_dol_kierunek, false, _opcje.schody_dol_w_scianie))
		{
			pokoje.clear();
			return false;
		}
	}

	pokoje.clear();
	return true;
}

void rysuj_mape_konsola(Pole* _mapa, uint _w, uint _h)
{
	assert(_mapa && _w && _h);

	OpcjeMapy opcje;
	opcje.mapa = _mapa;
	opcje.w = _w;
	opcje.h = _h;

	Mapa::mapa = _mapa;
	Mapa::opcje = &opcje;
	Mapa::rysuj();
}

vector<bool> maze;

int generate_labirynth2(const INT2& maze_size, const INT2& room_size, INT2& room_pos, INT2& doors)
{
	list<INT2> drillers;
	maze.resize(maze_size.x * maze_size.y, false);
	for(vector<bool>::iterator it = maze.begin(), end = maze.end(); it != end; ++it)
		*it = false;

	int mx = maze_size.x/2 - room_size.x/2,
		my = maze_size.y/2 - room_size.y/2;
	room_pos.x = mx;
	room_pos.y = my;

	for(int y=0; y<room_size.y; ++y)
	{
		for(int x=0; x<room_size.x; ++x)
		{
			maze[x+mx+(y+my)*maze_size.x] = true;
		}
	}

	INT2 start, start2;
	int dir;

	switch(rand2()%4)
	{
	case 0:
		start.x = maze_size.x/2;
		start.y = maze_size.y/2 + room_size.y/2;
		start2 = start;
		start.y--;
		dir = 1;
		break;
	case 1:
		start.x = maze_size.x/2 - room_size.x/2;
		start.y = maze_size.y/2;
		start2 = start;
		start2.x--;
		dir = 2;
		break;
	case 2:
		start.x = maze_size.x/2;
		start.y = maze_size.y/2 - room_size.y/2;
		start2 = start;
		start2.y--;
		dir = 0;
		break;
	case 3:
		start.x = maze_size.x/2 + room_size.x/2;
		start.y = maze_size.y/2;
		start2 = start;
		start.x--;
		dir = 3;
		break;
	}
	drillers.push_back(start2);
	/*maze[start2.x+start2.y*maze_size.x] = true;
	maze[start.x+start.y*maze_size.x] = false;
	draw_maze();
	_getch();
	system("cls");
	maze[start2.x+start2.y*maze_size.x] = false;
	maze[start.x+start.y*maze_size.x] = true;*/

	int pc = 0;

	while(!drillers.empty())
	{ 
		list<INT2>::iterator m,_m;//,temp; 
		m=drillers.begin(); 
		_m=drillers.end(); 
		while (m!=_m) 
		{ 
			bool remove_driller=false; 
			switch(dir) 
			{ 
			case 0: 
				m->y -= 2; 
				if (m->y <0 || maze[m->y*maze_size.x+m->x]) 
				{ 
					remove_driller=true; 
					break; 
				} 
				maze[(m->y+1)*maze_size.x+m->x]=true; 
				break; 
			case 1: 
				m->y += 2; 
				if (m->y>=maze_size.y || maze[m->y*maze_size.x+m->x]) 
				{ 
					remove_driller=true; 
					break; 
				} 
				maze[(m->y-1)*maze_size.x+m->x]=true; 
				break; 
			case 2: 
				m->x -= 2; 
				if (m->x<0 || maze[m->y*maze_size.x+m->x]) 
				{ 
					remove_driller=true; 
					break; 
				} 
				maze[m->y*maze_size.x+m->x+1]=true; 
				break; 
			case 3: 
				m->x += 2; 
				if (m->x>=maze_size.x || maze[m->y*maze_size.x+m->x]) 
				{ 
					remove_driller=true; 
					break; 
				} 
				maze[m->y*maze_size.x+m->x-1]=true; 
				break; 
			} 
			if (remove_driller) 
				m = drillers.erase(m); 
			else 
			{ 
				drillers.push_back(*m); 
				// uncomment the line below to make the maze easier 
				// if (rand2()%2) 
				drillers.push_back(*m); 

				maze[m->x+m->y*maze_size.x] = true;
				++m; 
				++pc;
			}

			dir = rand2()%4;
		} 
	}

	for(int x=0; x<room_size.x; ++x)
	{
		maze[mx+x+my*maze_size.x] = false;
		maze[mx+x+(my+room_size.y-1)*maze_size.x] = false;
	}
	for(int y=0; y<room_size.y; ++y)
	{
		maze[mx+(my+y)*maze_size.x] = false;
		maze[mx+room_size.x-1+(my+y)*maze_size.x] = false;
	}
	maze[start.x+start.y*maze_size.x] = true;
	maze[start2.x+start2.y*maze_size.x] = true;

	doors = INT2(start.x,start.y);
	
	return pc;
}

inline bool IsInside(const INT2& pt, const INT2& start, const INT2& size)
{
	return (pt.x >= start.x && pt.y >= start.y && pt.x < start.x+size.x && pt.y < start.y+size.y);
}

void generate_labirynth(Pole*& mapa, const INT2& size, const INT2& room_size, INT2& stairs, int& stairs_dir,  INT2& room_pos, int kraty_szansa)
{
	// mo�na by da�, �e nie ma centralnego pokoju
	assert(room_size.x > 4 && room_size.y > 4 && size.x >= room_size.x*2+4 && size.y >= room_size.y*2+4);
	INT2 maze_size(size.x-2, size.y-2), doors;

	while(generate_labirynth2(maze_size, room_size, room_pos, doors) < 600);

	++room_pos.x;
	++room_pos.y;
	mapa = new Pole[size.x*size.y];
	memset(mapa, 0, sizeof(Pole)*size.x*size.y);

	// kopiuj map�
	for(int y=0; y<maze_size.y; ++y)
	{
		for(int x=0; x<maze_size.x; ++x)
		{
			mapa[x+1+(y+1)*size.x].co = (maze[x+y*maze_size.x] ? PUSTE : SCIANA);
		}
	}

	mapa[doors.x+1+(doors.y+1)*size.x].co = DRZWI;

	// blokady
	for(int x=0; x<size.x; ++x)
	{
		mapa[x].co = BLOKADA_SCIANA;
		mapa[x+(size.y-1)*size.x].co = BLOKADA_SCIANA;
	}

	for(int y=1; y<size.y-1; ++y)
	{
		mapa[y*size.x].co = BLOKADA_SCIANA;
		mapa[size.x-1+y*size.x].co = BLOKADA_SCIANA;
	}

	// schody
	int order[4] = {0,1,2,3};
	for(int i=0; i<5; ++i)
		std::swap(order[rand2()%4], order[rand2()%4]);
	bool ok = false;
	for(int i=0; i<4 && !ok; ++i)
	{
		switch(order[i])
		{
		case 0: // d�
			{
				int start = random(1,size.x-1);
				int p = start;
				do 
				{
					if(mapa[p+size.x].co == PUSTE)
					{
						int ile = 0;
						if(mapa[p-1+size.x].co != PUSTE)
							++ile;
						if(mapa[p+1+size.x].co != PUSTE)
							++ile;
						if(mapa[p].co != PUSTE)
							++ile;
						if(mapa[p+size.x*2].co != PUSTE)
							++ile;

						if(ile == 3)
						{
							stairs.x = p;
							stairs.y = 1;
							ok = true;
							break;
						}
					}
					++p;
					if(p == size.x-1)
						p = 1;
				}
				while(p != start);
			}
			break;
		case 1: // lewa
			{
				int start = random(1,size.y-1);
				int p = start;
				do 
				{
					if(mapa[1+p*size.x].co == PUSTE)
					{
						int ile = 0;
						if(mapa[p*size.x].co != PUSTE)
							++ile;
						if(mapa[2+p*size.x].co != PUSTE)
							++ile;
						if(mapa[1+(p-1)*size.x].co != PUSTE)
							++ile;
						if(mapa[1+(p+1)*size.x].co != PUSTE)
							++ile;

						if(ile == 3)
						{
							stairs.x = 1;
							stairs.y = p;
							ok = true;
							break;
						}
					}
					++p;
					if(p == size.x-1)
						p = 1;
				}
				while(p != start);
			}
			break;
		case 2: // g�ra
			{
				int start = random(1,size.x-1);
				int p = start;
				do 
				{
					if(mapa[p+(size.y-2)*size.x].co == PUSTE)
					{
						int ile = 0;
						if(mapa[p-1+(size.y-2)*size.x].co != PUSTE)
							++ile;
						if(mapa[p+1+(size.y-2)*size.x].co != PUSTE)
							++ile;
						if(mapa[p+(size.y-1)*size.x].co != PUSTE)
							++ile;
						if(mapa[p+(size.y-3)*size.x].co != PUSTE)
							++ile;

						if(ile == 3)
						{
							stairs.x = p;
							stairs.y = size.y-2;
							ok = true;
							break;
						}
					}
					++p;
					if(p == size.x-1)
						p = 1;
				}
				while(p != start);
			}
			break;
		case 3: // prawa
			{
				int start = random(1,size.y-1);
				int p = start;
				do 
				{
					if(mapa[size.x-2+p*size.x].co == PUSTE)
					{
						int ile = 0;
						if(mapa[size.x-3+p*size.x].co != PUSTE)
							++ile;
						if(mapa[size.x-1+p*size.x].co != PUSTE)
							++ile;
						if(mapa[size.x-2+(p-1)*size.x].co != PUSTE)
							++ile;
						if(mapa[size.x-2+(p+1)*size.x].co != PUSTE)
							++ile;

						if(ile == 3)
						{
							stairs.x = size.x-2;
							stairs.y = p;
							ok = true;
							break;
						}
					}
					++p;
					if(p == size.x-1)
						p = 1;
				}
				while(p != start);
			}
			break;
		}
	}
	mapa[stairs.x+stairs.y*size.x].co = SCHODY_GORA;

	// ustal kierunek schod�w
	if(mapa[stairs.x+(stairs.y+1)*size.x].co == PUSTE)
		stairs_dir = 2;
	else if(mapa[stairs.x-1+stairs.y*size.x].co == PUSTE)
		stairs_dir = 1;
	else if(mapa[stairs.x+(stairs.y-1)*size.x].co == PUSTE)
		stairs_dir = 0;
	else if(mapa[stairs.x+1+stairs.y*size.x].co == PUSTE)
		stairs_dir = 3;
	else
	{
		assert(0);
	}

	// kraty
	if(kraty_szansa > 0)
	{
		for(int y=1; y<size.y-1; ++y)
		{
			for(int x=1; x<size.x-1; ++x)
			{
				Pole& p = mapa[x+y*size.x];
				if(p.co == PUSTE && !IsInside(INT2(x,y), room_pos, room_size) && rand2()%100 < kraty_szansa)
				{
					int j = rand2()%3;
					if(j == 0)
						p.co = KRATKA_PODLOGA;
					else if(j == 1)
						p.co = KRATKA_SUFIT;
					else
						p.co = KRATKA;
				}
			}
		}
	}

	// flagi
	Mapa::mapa = mapa;
	OpcjeMapy opcje;
	opcje.w = size.x;
	opcje.h = size.y;
	Mapa::opcje = &opcje;
	Mapa::ustaw_flagi();

	// rysuj
	DEBUG_DO(Mapa::rysuj());
}

namespace Cave
{
	int size = 45, size2 = size*size;
	bool* m1 = NULL, *m2 = NULL;
	int fill = 50;
	int iter = 3;
	int minx, miny, maxx, maxy;

	void fill_map(bool* m)
	{
		for(int i=0; i<size2; ++i)
		{
			m[i] = (rand2()%100 < fill);
		}
	}

	void draw_map(bool* m)
	{
		for(int y=0; y<size; ++y)
		{
			for(int x=0; x<size; ++x)
				printf("%c", m[x+y*size] ? '#' : ' ');
			printf("\n");
		}
		printf("\n");
	}

	void make(bool* m1, bool* m2)
	{
		for(int y=1; y<size-1; ++y)
		{
			for(int x=1; x<size-1; ++x)
			{
				int ile = 0;
				for(int yy=-1; yy<=1; ++yy)
				{
					for(int xx=-1; xx<=1; ++xx)
					{
						if(m1[x+xx+(y+yy)*size])
							++ile;
					}
				}

				m2[x+y*size] = (ile >= 5);
			}
		}

		for(int i=0; i<size; ++i)
		{
			m2[i] = m1[i];
			m2[i+(size-1)*size] = m1[i+(size-1)*size];
			m2[i*size] = m1[i*size];
			m2[i*size+size-1] = m1[i*size+size-1];
		}
	}

	std::vector<int> v;

	int calc_fill(bool* m, bool* m2, int start)
	{
		int ile = 0;
		v.push_back(start);
		m2[start] = true;

		do 
		{
			int i = v.back();
			v.pop_back();
			++ile;
			m2[i] = true;

			int x = i%size;
			int y = i/size;

#define T(x) if(!m[x] && !m2[x]) { m2[x] = true; v.push_back(x); }

			if(x > 0)
				T(x-1+y*size);
			if(x < size-1)
				T(x+1+y*size);
			if(y < 0)
				T(x+(y-1)*size);
			if(y < size-1)
				T(x+(y+1)*size);

#undef T
		}
		while(!v.empty());

		return ile;
	}

	int fill_cave(bool* m, bool* m2, int start)
	{
		v.push_back(start);
		m[start] = true;
		memset(m2, 1, sizeof(bool)*size2);
		int c = 1;

		do 
		{
			int i = v.back();
			v.pop_back();

			int x = i%size;
			int y = i/size;

			if(x < minx)
				minx = x;
			if(y < miny)
				miny = y;
			if(x > maxx)
				maxx = x;
			if(y > maxy)
				maxy = y;

#define T(t) if(!m[t]) { v.push_back(t); m[t] = true; m2[t] = false; ++c; }

			if(x > 0)
				T(x-1+y*size);
			if(x < size-1)
				T(x+1+y*size);
			if(y < 0)
				T(x+(y-1)*size);
			if(y < size-1)
				T(x+(y+1)*size);

#undef T
		}
		while(!v.empty());

		//draw_map(m2);
		//_getch();

		return c;
	}

	int finish(bool* m, bool* m2)
	{
		memset(m2, 0, sizeof(bool)*size2);
		int top = -1, topi = -1;

		// znajd� najwi�kszy obszar
		for(int i=0; i<size2; ++i)
		{
			if(!m[i] || m2[i])
				continue;

			int ile = calc_fill(m, m2, i);
			if(ile > top)
			{
				top = ile;
				topi = i;
			}
		}

		// wype�nij obszar
		minx = size;
		miny = size;
		maxx = 0;
		maxy = 0;
		return fill_cave(m, m2, topi);
	}

	int generate(int _size)
	{
		size2 = _size*_size;

		if(!m1 || _size > size)
		{
			delete[] m1;
			delete[] m2;
			m1 = new bool[size2];
			m2 = new bool[size2];
		}
		
		size = _size;

		// wype�nij losowo
		fill_map(m1);
		//draw_map(m1);
		//_getch();

		// celluar automata
		for(int i=0; i<iter; ++i)
		{
			make(m1, m2);
			bool* m = m1;
			m1 = m2;
			m2 = m;

			// kraw�dzie
			for(int j=0; j<size; ++j)
			{
				m1[j] = true;
				m1[j+(size-1)*size] = true;
				m1[j*size+size-1] = true;
				m1[j*size] = true;
			}

			//draw_map(m1);
			//_getch();
		}

		// wybierz najwi�kszy obszar i go wype�nij
		return finish(m1, m2);
	}

	void free_cave_data()
	{
		delete[] m1;
		delete[] m2;
	}
};

void generate_cave(Pole*& mapa, int size, INT2& stairs, int& stairs_dir, vector<INT2>& holes, IBOX2D* ext)
{
	assert(in_range(size, 10, 100));

	int size2 = size*size;

	while(Cave::generate(size) < 200);

	mapa = new Pole[size2];
	memset(mapa, 0, sizeof(Pole)*size2);

	// rozmiar
	if(ext)
		ext->Set(Cave::minx, Cave::miny, Cave::maxx+1, Cave::maxy+1);

	// kopiuj
	for(int i=0; i<size2; ++i)
		mapa[i].co = (Cave::m2[i] ? SCIANA : PUSTE);

	// schody
	do 
	{
		INT2 pt, dir;

		switch(rand2()%4)
		{
		case 0: // d�
			dir = INT2(0,-1);
			pt = INT2((random(1,size-2)+random(1,size-2))/2,size-1);
			break;
		case 1: // lewa
			dir = INT2(1,0);
			pt = INT2(0,(random(1,size-2)+random(1,size-2))/2);
			break;
		case 2: // g�ra
			dir = INT2(0,1);
			pt = INT2((random(1,size-2)+random(1,size-2))/2,0);
			break;
		case 3: // prawa
			dir = INT2(-1,0);
			pt = INT2(size-1,(random(1,size-2)+random(1,size-2))/2);
			break;
		}

		do 
		{
			pt += dir;
			if(pt.x == -1 || pt.x == size || pt.y == -1 || pt.y == size)
				break;
			if(mapa[pt.x+pt.y*size].co == PUSTE)
			{
				pt -= dir;
				// sprawd� z ilu stron jest puste pole
				int ile = 0, dir2;
				if(mapa[pt.x-1+pt.y*size].co == PUSTE)
				{
					++ile;
					dir2 = 1;
				}
				if(mapa[pt.x+1+pt.y*size].co == PUSTE)
				{
					++ile;
					dir2 = 3;
				}
				if(mapa[pt.x+(pt.y-1)*size].co == PUSTE)
				{
					++ile;
					dir2 = 0;
				}
				if(mapa[pt.x+(pt.y+1)*size].co == PUSTE)
				{
					++ile;
					dir2 = 2;
				}

				if(ile == 1)
				{
					stairs = pt;
					stairs_dir = dir2;
					mapa[pt.x+pt.y*size].co = SCHODY_GORA;
					goto dalej;
				}
				else
					break;
			}
		}
		while(1);
	}
	while(1);

dalej:

	// losowe dziury w suficie
	for(int count=0, tries=50; tries>0 && count<15; --tries)
	{
		INT2 pt(random(1,size-1), random(1,size-1));
		if(mapa[pt.x+pt.y*size].co == PUSTE)
		{
			bool ok = true;
			for(vector<INT2>::iterator it = holes.begin(), end = holes.end(); it != end; ++it)
			{
				if(distance(pt, *it) < 5)
				{
					ok = false;
					break;
				}
			}

			if(ok)
			{
				mapa[pt.x+pt.y*size].co = KRATKA_SUFIT;
				holes.push_back(pt);
				++count;
			}
		}
	}

	// flagi
	Mapa::mapa = mapa;
	OpcjeMapy opcje;
	opcje.w = opcje.h = size;
	Mapa::opcje = &opcje;
	Mapa::ustaw_flagi();

	// rysuj
#ifdef _DEBUG
	Mapa::rysuj();
#endif
}

void regenerate_cave_flags(Pole* mapa, int size)
{
	assert(mapa && in_range(size, 10, 100));

	// czy�� flagi (wszystko opr�cz F_NISKI_SUFIT, F_BLOKADA, F_DRUGA_TEKSTURA, F_ODKRYTE)
	for(int i=0, s = size*size; i<s; ++i)
		CLEAR_BIT(mapa[i].flagi, 0xFFFFFFFF & ~Pole::F_NISKI_SUFIT & ~Pole::F_BLOKADA & ~Pole::F_DRUGA_TEKSTURA & ~Pole::F_ODKRYTE);

	// ustaw flagi
	Mapa::mapa = mapa;
	OpcjeMapy opcje;
	opcje.w = opcje.h = size;
	Mapa::opcje = &opcje;
	Mapa::ustaw_flagi();
}

void free_cave_data()
{
	Cave::free_cave_data();
}

extern DWORD tmp;

void Pokoj::Save(HANDLE file)
{
	WriteFile(file, &pos, sizeof(pos), &tmp, NULL);
	WriteFile(file, &size, sizeof(size), &tmp, NULL);
	uint ile = polaczone.size();
	WriteFile(file, &ile, sizeof(ile), &tmp, NULL);
	if(ile)
		WriteFile(file, &polaczone[0], sizeof(int)*ile, &tmp, NULL);
	WriteFile(file, &cel, sizeof(cel), &tmp, NULL);
	WriteFile(file, &korytarz, sizeof(korytarz), &tmp, NULL);
}

void Pokoj::Load(HANDLE file)
{
	ReadFile(file, &pos, sizeof(pos), &tmp, NULL);
	ReadFile(file, &size, sizeof(size), &tmp, NULL);
	uint ile;
	ReadFile(file, &ile, sizeof(ile), &tmp, NULL);
	polaczone.resize(ile);
	if(ile)
		ReadFile(file, &polaczone[0], sizeof(int)*ile, &tmp, NULL);
	ReadFile(file, &cel, sizeof(cel), &tmp, NULL);
	ReadFile(file, &korytarz, sizeof(korytarz), &tmp, NULL);
}

// zwraca pole oznaczone ?
// ###########
// #    #    #
// # p1 ? p2 #
// #    #    #
// ###########
// je�li jest kilka takich p�l to zwraca pierwsze
INT2 pole_laczace(int pokoj1, int pokoj2)
{
	assert(pokoj1 >= 0 && pokoj2 >= 0 && max(pokoj1,pokoj2) < (int)Mapa::opcje->pokoje->size());

	Pokoj& p1 = Mapa::opcje->pokoje->at(pokoj1);
	Pokoj& p2 = Mapa::opcje->pokoje->at(pokoj2);

	// sprawd� czy istnieje po��czenie
#ifdef _DEBUG
	for(vector<int>::iterator it = p1.polaczone.begin(), end = p1.polaczone.end(); it != end; ++it)
	{
		if(*it == pokoj2)
			goto ok;
	}
	assert(0 && "Pokoj1 i pokoj2 nie s� po��czone!");
ok:
#endif

	// znajd� wsp�lne pola
	int x1 = max(p1.pos.x,p2.pos.x),
		x2 = min(p1.pos.x+p1.size.x,p2.pos.x+p2.size.x),
		y1 = max(p1.pos.y,p2.pos.y),
		y2 = min(p1.pos.y+p1.size.y,p2.pos.y+p2.size.y);

	assert(x1 < x2 && y1 < y2);

	for(int y=y1; y<y2; ++y)
	{
		for(int x=x1; x<x2; ++x)
		{
			Pole& po = Mapa::mapa[x+y*Mapa::opcje->w];
			if(po.co == PUSTE || po.co == DRZWI)
				return INT2(x,y);
		}
	}

	assert(0 && "Brak pola ��cz�cego!");
	return INT2(-1,-1);
}

void ustaw_flagi(Pole* mapa, uint wh)
{
	assert(mapa && wh > 0);

	OpcjeMapy opcje;
	opcje.mapa = mapa;
	opcje.w = wh;
	opcje.h = wh;

	Mapa::opcje = &opcje;
	Mapa::mapa = mapa;
	Mapa::ustaw_flagi();
}

void Light::Save(File& f)
{
	f << pos;
	f << color;
	f << range;
}

void Light::Load(File& f)
{
	f >> pos;
	f >> color;
	f >> range;
}
