#pragma once

//-----------------------------------------------------------------------------
// dodatkowe definicje klawiszy
#define VK_NONE 0

//-----------------------------------------------------------------------------
// stan klawisza
enum InputState
{
	IS_UP,			// 00
	IS_RELEASED,	// 01
	IS_DOWN,		// 10
	IS_PRESSED		// 11
};

//-----------------------------------------------------------------------------
// stan klawiatury
struct KeyStates
{
	// proste sprawdzanie czy klawisz zosta� wci�ni�ty, wyci�ni�ty, jest wci�ni�ty, jest wyci�ni�ty
	inline bool Pressed(byte key) const { return keystate[key] == IS_PRESSED; }
	inline bool Released(byte key) const { return keystate[key] == IS_RELEASED; }
	inline bool Down(byte key) const { return keystate[key] >= IS_DOWN; }
	inline bool Up(byte key) const { return keystate[key] <= IS_RELEASED; }

	// jednorazowe sprawdzanie czy klawisz jest wci�ni�ty, je�li by� to go ustawia na wyci�ni�tego
	inline bool PressedRelease(byte key)
	{
		if(Pressed(key))
		{
			keystate[key] = IS_DOWN;
			return true;
		}
		else
			return false;
	}

	inline bool PressedUp(byte key)
	{
		if(Pressed(key))
		{
			keystate[key] = IS_UP;
			return true;
		}
		else
			return false;
	}

	// sprawdza czy jeden z dw�ch klawiszy zosta� wci�ni�ty
	inline byte Pressed2(byte k1, byte k2) const { return ReturnState2(k1, k2, IS_PRESSED); }
	// jw ale ustawia na wyci�ni�ty
	inline byte Pressed2Release(byte k1, byte k2)
	{
		if(keystate[k1] == IS_PRESSED)
		{
			keystate[k1] = IS_DOWN;
			return k1;
		}
		else if(keystate[k2] == IS_PRESSED)
		{
			keystate[k2] = IS_DOWN;
			return k2;
		}
		else
			return VK_NONE;
	}
	
	// sprawdza czy zosta�a wprowadzona kombinacja klawiszy (np alt+f4)
	inline bool DownPressed(byte k1, byte k2) const { return ((Down(k1) && Pressed(k2)) || (Down(k2) &&Pressed(k1))); }

	// zwraca kt�ry z podanych klawiszy ma taki stan
	inline byte ReturnState2(byte k1, byte k2, InputState state) const
	{
		if(keystate[k1] == state)
			return k1;
		else if(keystate[k2] == state)
			return k2;
		else
			return VK_NONE;
	}

	// ustawia stan klawisza
	inline void SetState(byte key, InputState istate) { keystate[key] = (byte)istate; }

	// aktualizacja stanu klawiszy
	inline void Update()
	{
		byte printscreen = keystate[VK_SNAPSHOT];
		for(uint i = 0; i < 256; ++i)
		{
			if(keystate[i] & 1)
				--keystate[i];
		}
		if(printscreen == IS_PRESSED)
			keystate[VK_SNAPSHOT] = IS_RELEASED;
	}

	// wyciskanie klawiszy
	inline void ReleaseKeys()
	{
		for(byte i=0; i<255; ++i)
		{
			if(keystate[i] & 0x2)
				keystate[i] = IS_RELEASED;
		}
	}

	// obs�uga wci�ni�cia klawisza
	inline void Process(byte key, bool down)
	{
		if(key != VK_SNAPSHOT)
		{
			if(down)
			{
				if(keystate[key] <= IS_RELEASED)
					keystate[key] = IS_PRESSED;
			}
			else
			{
				if(keystate[key] >= IS_DOWN)
					keystate[key] = IS_RELEASED;
			}
		}
		else
			keystate[key] = IS_PRESSED;
	}

	inline byte* GetKeystateData()
	{
		return keystate;
	}

	inline void SetFocus(bool f) { focus = f; }
	inline bool Focus() const { return focus; }

	// skr�t klawiszowy (np. Ctrl-A)
	inline bool Shortcut(byte k1, byte k2, bool up=true)
	{
		if(Down(k1) && Pressed(k2))
		{
			if(up)
				SetState(k2, IS_DOWN);
			return true;
		}
		else
			return false;
	}

	inline bool DownUp(byte key)
	{
		if(Down(key))
		{
			keystate[key] = IS_UP;
			return true;
		}
		else
			return false;
	}

private:
	byte keystate[256];
	bool focus;
};
extern KeyStates Key;

//-----------------------------------------------------------------------------
typedef bool (KeyStates::*KeyF)(byte);
typedef bool (KeyStates::*KeyFC)(byte) const;
