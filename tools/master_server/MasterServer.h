#pragma once

#include "Message.h"

struct User
{
	SystemAddress adr;
	float timer;
	bool ok, admin;
};

class MasterServer : public PluginInterface2
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

	/// Called when the interface is attached
	virtual void OnAttach(void)
	{
		logger->Info("OnAttach");
	}

	/// Called when the interface is detached
	virtual void OnDetach(void)
	{
		logger->Info("OnDetach");
	}

	/// Update is called every time a packet is checked for .
	virtual void Update(void)
	{
		//logger->Info("Update");
	}

	cstring ToPacket(byte id)
	{
		switch(id)
		{
		case ID_OPEN_CONNECTION_REQUEST_1: return "ID_OPEN_CONNECTION_REQUEST_1";
		case ID_OPEN_CONNECTION_REPLY_1: return "ID_OPEN_CONNECTION_REPLY_1";
		case ID_OPEN_CONNECTION_REQUEST_2: return "ID_OPEN_CONNECTION_REQUEST_2";
		case ID_OPEN_CONNECTION_REPLY_2: return "ID_OPEN_CONNECTION_REPLY_2";
		default: return Format("Unknown(%d)", id);
		}
	}

	/// OnReceive is called for every packet.
	/// \param[in] packet the packet that is being returned to the user
	/// \return True to allow the game and other plugins to get this message, false to absorb it
	virtual PluginReceiveResult OnReceive(Packet *packet)
	{
		logger->Info(Format("%s (%d) from %s.", ToPacket(packet->data[0]), packet->length, packet->systemAddress.ToString()));
		return RR_CONTINUE_PROCESSING;
	}

	/// Called when RakPeer is initialized
	virtual void OnRakPeerStartup(void)
	{
		logger->Info("OnRakPeerStartup");
	}

	/// Called when RakPeer is shutdown
	virtual void OnRakPeerShutdown(void)
	{
		logger->Info("OnRakPeerShutdown");
	}

	/// Called when a connection is dropped because the user called RakPeer::CloseConnection() for a particular system
	/// \param[in] systemAddress The system whose connection was closed
	/// \param[in] rakNetGuid The guid of the specified system
	/// \param[in] lostConnectionReason How the connection was closed: manually, connection lost, or notification of disconnection
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason)
	{
		logger->Info(Format("OnClosedConnection from %s reason %d", systemAddress.ToString(), lostConnectionReason));
	}

	/// Called when we got a new connection
	/// \param[in] systemAddress Address of the new connection
	/// \param[in] rakNetGuid The guid of the specified system
	/// \param[in] isIncoming If true, this is ID_NEW_INCOMING_CONNECTION, or the equivalent
	virtual void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming)
	{
		logger->Info(Format("OnNewConnection from %s incoming %d", systemAddress.ToString(), isIncoming ? 1 : 0));
	}

	/// Called when a connection attempt fails
	/// \param[in] packet Packet to be returned to the user
	/// \param[in] failedConnectionReason Why the connection failed
	virtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason)
	{
		logger->Info(Format("OnFailedConnectionAttempt for %s packet %d (%d) reason %d", packet->systemAddress.ToString(), packet->data[0], packet->length, failedConnectionAttemptReason));
	}

	/// Queried when attached to RakPeer
	/// Return true to call OnDirectSocketSend(), OnDirectSocketReceive(), OnReliabilityLayerNotification(), OnInternalPacket(), and OnAck()
	/// If true, then you cannot call RakPeer::AttachPlugin() or RakPeer::DetachPlugin() for this plugin, while RakPeer is active
	virtual bool UsesReliabilityLayer(void) const { return true; }

	/// Called on a send to the socket, per datagram, that does not go through the reliability layer
	/// \pre To be called, UsesReliabilityLayer() must return true
	/// \param[in] data The data being sent
	/// \param[in] bitsUsed How many bits long \a data is
	/// \param[in] remoteSystemAddress Which system this message is being sent to
	virtual void OnDirectSocketSend(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress)
	{
		logger->Info(Format("OnDirectSocketSend %s for %s length %d", ToPacket(data[0]), remoteSystemAddress.ToString(), bitsUsed));
	}

	/// Called on a receive from the socket, per datagram, that does not go through the reliability layer
	/// \pre To be called, UsesReliabilityLayer() must return true
	/// \param[in] data The data being sent
	/// \param[in] bitsUsed How many bits long \a data is
	/// \param[in] remoteSystemAddress Which system this message is being sent to
	virtual void OnDirectSocketReceive(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress)
	{
		logger->Info(Format("OnDirectSocketReceive %s from %s length %d", ToPacket(data[0]), remoteSystemAddress.ToString(), bitsUsed));
	}

	/// Called when the reliability layer rejects a send or receive
	/// \pre To be called, UsesReliabilityLayer() must return true
	/// \param[in] bitsUsed How many bits long \a data is
	/// \param[in] remoteSystemAddress Which system this message is being sent to
	virtual void OnReliabilityLayerNotification(const char *errorMessage, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress, bool isError)
	{
		logger->Info(Format("OnReliabilityLayerNotification for %s msg \"%s\" length %d is error %d", remoteSystemAddress.ToString(), errorMessage, bitsUsed, isError ? 1 : 0));
	}

	/// Called on a send or receive of a message within the reliability layer
	/// \pre To be called, UsesReliabilityLayer() must return true
	/// \param[in] internalPacket The user message, along with all send data.
	/// \param[in] frameNumber The number of frames sent or received so far for this player depending on \a isSend .  Indicates the frame of this user message.
	/// \param[in] remoteSystemAddress The player we sent or got this packet from
	/// \param[in] time The current time as returned by RakNet::GetTimeMS()
	/// \param[in] isSend Is this callback representing a send event or receive event?
	virtual void OnInternalPacket(InternalPacket *internalPacket, unsigned frameNumber, SystemAddress remoteSystemAddress, RakNet::TimeMS time, int isSend)
	{
		logger->Info(Format("OnInternalPacket fr %s ????????", remoteSystemAddress.ToString()));
	}

	/// Called when we get an ack for a message we reliably sent
	/// \pre To be called, UsesReliabilityLayer() must return true
	/// \param[in] messageNumber The numerical identifier for which message this is
	/// \param[in] remoteSystemAddress The player we sent or got this packet from
	/// \param[in] time The current time as returned by RakNet::GetTimeMS()
	virtual void OnAck(unsigned int messageNumber, SystemAddress remoteSystemAddress, RakNet::TimeMS time)
	{
		logger->Info("OnAck ????????");
	}

	/// System called RakPeerInterface::PushBackPacket
	/// \param[in] data The data being sent
	/// \param[in] bitsUsed How many bits long \a data is
	/// \param[in] remoteSystemAddress The player we sent or got this packet from
	virtual void OnPushBackPacket(const char *data, const BitSize_t bitsUsed, SystemAddress remoteSystemAddress)
	{
		logger->Info("OnPushBackPacket ????????");
	}

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
