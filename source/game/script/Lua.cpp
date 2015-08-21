#include "Pch.h"
#include "Base.h"
#include <lua.hpp>
#include <conio.h>

int main2()
{
	lua_State* L = luaL_newstate();
	luaL_openlibs(L);
	luaL_loadfile(L, "script/1.lua");
	lua_pcall(L, 0, 0, 0);
	lua_close(L);
	_getch();
	return 0;
}
