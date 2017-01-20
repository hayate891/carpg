#pragma once

#undef ID_CMD

enum Message : byte
{
	ID_HELLO = ID_USER_PACKET_ENUM,
	ID_QUIT,
	ID_HOST,
	ID_UPDATE_SERVER,
	ID_CLOSE_SERVER,
	ID_QUERY,
	ID_JOIN,
	ID_GET_STAT,
	ID_SET_STAT,
	ID_SYS_STAT,
	ID_CMD,
	//ID_CRASH crash report
};
