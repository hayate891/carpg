#include "Pch.h"
#include "Base.h"
#define NULL nullptr
#include <lua.hpp>

//extern "C"
//{
	int print_int(lua_State* L)
	{
		int a = luaL_checkinteger(L, 1);
		a += 1;
		return 0;
	}
//}

void lua_test()
{
	lua_State* L = luaL_newstate();

	lua_pushcfunction(L, print_int);
	lua_setglobal(L, "print_int");

	luaL_dostring(L, "print_int(7, 4)");

	lua_close(L);

	exit(0);
}

struct Statik
{
	Statik() { lua_test(); }
} statik;
