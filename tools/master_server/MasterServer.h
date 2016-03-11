#pragma once

#include "Message.h"

struct User
{
	SystemAddress adr;
	float timer;
	bool ok, admin;
};

class MasterServer
{
public:
	inline static MasterServer& Get() { return _this; }
	int Start();
	void Shutdown();

private:
	MasterServer();
	void Init();
	bool LoadConfig();
	bool StartupServer();
	void Update(float dt);

	static const int VERSION = 0;
	static const int PROTOCOL_VERSION = 0;
	static MasterServer _this;

	// config
	word port;
	uint max_users;
	string pswd, admin_pswd;
	bool debug_timeout;

	vector<User> users;
	RakPeerInterface* peer;
	MultiLogger* logger;
	bool in_shutdown;
};
