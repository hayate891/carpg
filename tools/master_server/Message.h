#pragma once

#undef ID_CMD

enum Message : byte
{
	ID_HELLO = ID_USER_PACKET_ENUM,
	ID_HOST,
	ID_HOST_STOP,
	ID_LIST,
	ID_LIST_STOP,
	ID_JOIN,
	ID_LOGIN,
	ID_CMD
};
