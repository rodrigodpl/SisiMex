#include "UCC.h"

void UCC::update()
{
	switch (state)
	{
	case UCC_INIT:

		break;
	case UCC_REGISTERING:

		break;
	case UCC_IDLE:

		break;
	case UCC_UNREGISTERING:

		break;
	case UCC_FINISHED:

		break;

	default:
		break;
	}
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}
