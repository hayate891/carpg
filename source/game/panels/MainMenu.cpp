#include "Pch.h"
#include "Base.h"
#include "MainMenu.h"
#include "Language.h"
#include "Version.h"
#include "Dialog2.h"
#define far
#include <wininet.h>

//-----------------------------------------------------------------------------
#pragma comment(lib, "wininet.lib")

//-----------------------------------------------------------------------------
TEX MainMenu::tBackground;
TEX MainMenu::tLogo;

//-----------------------------------------------------------------------------
enum CheckVersionResult
{
	CVR_None,
	CVR_InternetOpenFailed,
	CVR_InternetOpenUrlFailed,
	CVR_ReadFailed,
	CVR_InvalidSignature,
	CVR_Ok
};
CriticalSection csCheckVersion;
CheckVersionResult version_check_result;
uint version_check_error, version_new;

//=================================================================================================
CheckVersionResult CheckVersion(HINTERNET internet, cstring url, uint& error, uint& version)
{
	HINTERNET file = InternetOpenUrl(internet, url, NULL, 0, 0, NULL);
	if(!file)
	{
		// Nie mo�na pobra� pliku z serwera
		error = GetLastError();
		return CVR_InternetOpenUrlFailed;
	}

	int data[2];
	DWORD read;

	InternetReadFile(file, &data, sizeof(data), &read);
	if(read != 8)
	{
		// Nie mo�na odczyta� pliku z serwera
		error = GetLastError();
		InternetCloseHandle(file);
		return CVR_ReadFailed;
	}

	InternetCloseHandle(file);

	if(data[0] != 0x475052CA) // magic number [najpierw 0xCA a p�niej w ascii RPG]
	{
		error = data[0];
		return CVR_InvalidSignature;
	}

	version = (data[1]&0xFFFFFF);	
	return CVR_Ok;
}

//=================================================================================================
DWORD WINAPI CheckVersion(void*)
{
	HINTERNET internet = InternetOpen("carpg", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	if(!internet)
	{
		// Nie mo�na nawi�za� po��czenia z serwerem
		csCheckVersion.Enter();
		version_check_error = GetLastError();
		version_check_result = CVR_InternetOpenFailed;
		csCheckVersion.Leave();
		return 1;
	}

	uint error = 0, version = 0;
	CheckVersionResult result = CheckVersion(internet, "http://carpg.pl/carpgdata/wersja", error, version);
	if(result != CVR_Ok)
		result = CheckVersion(internet, "http://dhost.info/radsun/carpgdata/wersja", error, version);

	InternetCloseHandle(internet);
	csCheckVersion.Enter();
	version_check_result = result;
	version_check_error = error;
	version_new = version;
	csCheckVersion.Leave();

	return result == CVR_Ok ? 0 : 1;
}

//=================================================================================================
MainMenu::MainMenu() : check_version(0), check_version_thread(NULL), check_updates(true), skip_version(0)
{
	focusable = true;
	visible = false;

	txInfoText = Str("infoText");
	txUrl = Str("url");
	txVersion = Str("version");

	const cstring names[] = {
		"newGame",
		"loadGame",
		"multiplayer",
		"options",
		"website",
		"info",
		"quit"
	};

	INT2 maxsize(0,0);

	// stw�rz przyciski
	for(int i=0; i<7; ++i)
	{
		Button& b = bt[i];
		b.id = IdNewGame+i;
		b.parent = this;
		b.text = Str(names[i]);
		b.size = GUI.default_font->CalculateSize(b.text) + INT2(24,24);

		maxsize = Max(maxsize, b.size);
	}

	// ustaw rozmiar
	for(int i=0; i<7; ++i)
		bt[i].size = maxsize;

	PlaceButtons();
}

//=================================================================================================
void MainMenu::Draw(ControlDrawData* /*cdd*/)
{
	GUI.DrawSpriteFull(tBackground, WHITE);
	GUI.DrawSprite(tLogo, INT2(GUI.wnd_size.x-512-16, 16));

	RECT r = {0, 0, GUI.wnd_size.x, GUI.wnd_size.y};
	r.top = r.bottom - 64;
	GUI.DrawText(GUI.default_font, "Devmode(2013,2015) Tomashu & Leinnan", DT_CENTER|DT_BOTTOM|DT_OUTLINE, WHITE, r);

	r.left = GUI.wnd_size.x-512-16;
	r.right = GUI.wnd_size.x-16;
	r.top = 256+24;
	r.bottom = r.top + 64;
	GUI.DrawText(GUI.default_font, Format(txVersion, VERSION_STR), DT_CENTER|DT_OUTLINE, WHITE, r);

	r.left = 0;
	r.right = GUI.wnd_size.x;
	r.bottom = GUI.wnd_size.y - 16;
	r.top = r.bottom - 64;
	GUI.DrawText(GUI.default_font, version_text, DT_CENTER|DT_BOTTOM|DT_OUTLINE, WHITE, r);

	for(int i=0; i<7; ++i)
		bt[i].Draw();
}

//=================================================================================================
void MainMenu::Update(float dt)
{
	for(int i=0; i<7; ++i)
	{
		bt[i].mouse_focus = focus;
		bt[i].Update(dt);
	}

	if(check_version == 0)
	{
#ifdef _DEBUG
		check_updates = false;
#endif
		if(check_updates)
		{
			version_text = Str("checkingVersion");
			LOG(version_text.c_str());
			check_version = 1;
			csCheckVersion.Create();
			check_version_thread = CreateThread(NULL, 1024, CheckVersion, NULL, 0, NULL);
			if(!check_version_thread)
			{
				csCheckVersion.Free();
				check_version = 2;
				version_text = Str("checkingError");
				ERROR(version_text.c_str());
			}
		}
		else
			check_version = 3;
	}
	else if(check_version == 1)
	{
		bool cleanup = false;
		csCheckVersion.Enter();
		if(version_check_result != CVR_None)
		{
			cleanup = true;
			if(version_check_result == CVR_Ok)
			{
				if(version_new > VERSION)
				{
					check_version = 4;
					cstring str = VersionToString(version_new);
					version_text = Format(Str("newVersion"), str);
					LOG(Format("New version %s is available.", str));
					if(version_new > skip_version)
					{
						// wy�wietl pytanie o pobranie nowej wersji
						DialogInfo info;
						info.event = fastdelegate::FastDelegate1<int>(this, &MainMenu::OnNewVersion);
						info.name = "new_version";
						info.order = ORDER_TOP;
						info.parent = NULL;
						info.pause = false;
						info.text = Format(Str("newVersionDialog"), VERSION_STR, VersionToString(version_new));
						info.type = DIALOG_YESNO;
						cstring names[] = {Str("download"), Str("skip")};
						info.custom_names = names;

						GUI.ShowDialog(info);
					}
				}
				else if(version_new < VERSION)
				{
					check_version = 3;
					version_text = Str("newerVersion");
					LOG("You have newer version then available.");
				}
				else
				{
					check_version = 3;
					version_text = Str("noNewVersion");
					LOG("No new version available.");
				}
			}
			else
			{
				check_version = 2;
				version_text = Format(Str("checkVersionError"), version_check_result, version_check_error);
				ERROR(version_text.c_str());
			}
		}
		csCheckVersion.Leave();

		if(cleanup)
		{
			csCheckVersion.Free();
			CloseHandle(check_version_thread);
			check_version_thread = NULL;
		}
	}
}

//=================================================================================================
void MainMenu::Event(GuiEvent e)
{
	if(e == GuiEvent_WindowResize)
		PlaceButtons();
	else if(e >= GuiEvent_Custom)
	{
		if(event)
			event(e);
	}
}

//=================================================================================================
void MainMenu::PlaceButtons()
{
	float kat = -PI/2;
	for(int i=0; i<7; ++i)
	{
		bt[i].pos = bt[i].global_pos = INT2(16+GUI.wnd_size.x-200+int(sin(kat)*(GUI.wnd_size.x-200)), 100+int(cos(kat)*GUI.wnd_size.y));
		kat += PI/4/7;
	}
}

//=================================================================================================
void MainMenu::OnNewVersion(int id)
{
	if(id == BUTTON_YES)
		ShellExecute(NULL, "open", Str("versionUrl"), NULL, NULL, SW_SHOWNORMAL);
}
