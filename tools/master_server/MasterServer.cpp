#include "Pch.h"
#include "Common.h"
#include "MasterServer.h"
#ifndef LINUX
#include <Windows.h>
#else
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#endif

MasterServer MasterServer::_this;

MasterServer::MasterServer() : peer(nullptr), in_shutdown(false)
{
	
}

void MasterServer::Shutdown()
{
	logger->Info("Shuting down.");
	
	in_shutdown = true;

	delete logger;

	if(peer)
		RakPeerInterface::DestroyInstance(peer);

	exit(0);
}

#ifndef LINUX
BOOL WINAPI ConsoleHandlerRoutine(DWORD type)
{
	switch(type)
	{
	case CTRL_C_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		MasterServer::Get().Shutdown();
		return TRUE;
	default:
		return FALSE;
	}
}
#elsevoid my_handler(int s)
{
	MasterServer::Get().Shutdown();
}
#endif

void MasterServer::Init()
{
#ifndef LINUX
	SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
#else	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = my_handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);
#endif
	
	logger = new MultiLogger;
	logger->loggers.push_back(new ConsoleLogger);
	logger->loggers.push_back(new TextLogger("log.txt", true));

	time_t t = time(0);
	tm t2;
#ifdef LINUX
	t2 = *localtime(&t);
#else
	localtime_s(&t2, &t);
#endif
	logger->Info(Format("CaRpg Master Server ver %d", VERSION));
	logger->Info(Format("Date: %04d-%02d-%02d", t2.tm_year + 1900, t2.tm_mon + 1, t2.tm_mday));
}

bool MasterServer::LoadConfig()
{
	// load
	port = 37556;
	max_users = 1;
	pswd = "";
	admin_pswd = "12345678";
	debug_timeout = true;

	if(admin_pswd.length() < 8)
	{
		logger->Error("Missing admin password or too short (minimum 8 characters)!");
		return false;
	}

	return true;
}

bool MasterServer::StartupServer()
{
	logger->Info(Format("Starting server (port %d)...", port));

	peer = RakPeerInterface::GetInstance();
	peer->AttachPlugin(this);
	
	SocketDescriptor sd(port, nullptr);
	sd.socketFamily = AF_INET;
	StartupResult r = peer->Startup(max_users, &sd, 1);
	if(r != RAKNET_STARTED)
	{
		logger->Error(Format("Failed to create server: RakNet error %d.", r));
		return false;
	}

	if(!pswd.empty())
		peer->SetIncomingPassword(pswd.c_str(), pswd.length());

	peer->SetMaximumIncomingConnections(max_users);

	if(debug_timeout)
		peer->SetTimeoutTime(60 * 60 * 1000, UNASSIGNED_SYSTEM_ADDRESS);

	logger->Info("Server created. Waiting for connection.");

	return true;
}

void MasterServer::Update(float dt)
{
	Packet* packet;
	for(packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
	{
		logger->Info(Format("Packet %d (%d) from %s.", packet->data[0], packet->length, packet->systemAddress.ToString()));
	}
}

int MasterServer::Start()
{
	Init();

	if(!LoadConfig())
	{
		logger->Error("Failed to load config.");
		return 2;
	}

	if(!StartupServer())
	{
		logger->Error("Failed to start server.");
		return 1;
	}

	auto old_time = std::chrono::system_clock::now();
	float dt = 0.f;

	while(true)
	{
		if(!in_shutdown)
			Update(dt);

#ifdef LINUX
		usleep(100000);
#else
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif

		auto new_time = std::chrono::system_clock::now();
		dt = ((float)std::chrono::duration_cast<std::chrono::milliseconds>(new_time - old_time).count()) / 1000;
		old_time = new_time;
	}
	
	return 0;
}

int main()
{
	return MasterServer::Get().Start();
}
