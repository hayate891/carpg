#include <Common.h>
#include <RakPeerInterface.h>
#include <RakNetTypes.h>
#include <MessageIdentifiers.h>
#define __BITSTREAM_NATIVE_END
#include <BitStream.h>
#include <cassert>

using namespace RakNet;

int main()
{
	RakPeerInterface* peer = RakPeerInterface::GetInstance();

	SocketDescriptor desc;
	desc.socketFamily = AF_INET;
	StartupResult result = peer->Startup(1, &desc, 1);
	assert(result == RAKNET_STARTED);

	ConnectionAttemptResult result2 = peer->Connect("localhost", 37556, nullptr, 0);
	assert(result2 == CONNECTION_ATTEMPT_STARTED);
	while(true)
	{
		Packet* packet;
		for(packet = peer->Receive(); packet; peer->DeallocatePacket(packet), packet = peer->Receive())
		{
			//packet->systemAddress
		}
	}

	return 0;
}
