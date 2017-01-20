#pragma once

enum ConnectionResponse
{
	CR_OK,
	CR_BROKEN,
	CR_INVALID_VERSION, // [version]
	CR_INVALID_PASSWORD,
	CR_INVALID_PASSWORD_BAN, // [datetime]
	CR_BAN, // [datetime]
	CR_PERM_BAN
};
