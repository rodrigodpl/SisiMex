#include "UCC.h"



void UCC::update()
{
	switch (state)
	{
	case UCC_INIT:
		setState(UCC_WAITING_AGREEMENT); 
		break;

	case UCC_SUCCEEDED:

		break;
	case UCC_FAILED:

		break;

	case UCC_FINISHED:
		destroy(); 
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

	// TODO: Handle packets
	switch (packetType)
	{
	case PacketType::AgreementRequest:
	{
		PacketAgreementRequest packet;
		packet.Read(stream);

		if (packet.offered_itemId == constraintItemId && packet.requested_itemId == contributedItemId)
		{
			setState(UCC_SUCCEEDED);

			AgentLocation receiver_loc;
			receiver_loc.hostIP = socket->RemoteAddress().GetIPString();
			receiver_loc.hostPort = LISTEN_PORT_AGENTS;
			receiver_loc.agentId = packetHeader.srcAgentId;

			sendPacketAgreementSuceeded(receiver_loc);
		}
		
		break;
	}
	
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCC::sendPacketAgreementSuceeded(AgentLocation receiver_loc)
{
	PacketHeader header; 
	header.dstAgentId = receiver_loc.agentId; 
	header.srcAgentId = id(); 
	header.packetType = PacketType::AgreementRequestResponse;

	PacketAgreementResult packet; 
	packet.result = true; 

	OutputMemoryStream output_stream;
	header.Write(output_stream);
	packet.Write(output_stream);

	sendPacketToAgent(receiver_loc.hostIP, receiver_loc.hostPort, output_stream);
}

void UCC::sendPacketAgreementConstrain(AgentLocation receiver_loc)
{
	PacketHeader header;
	header.dstAgentId = receiver_loc.agentId;
	header.srcAgentId = id();
	header.packetType = PacketType::AgreementRequestResponse;

	PacketAgreementConstrain packet;
	packet.constrain = constraintItemId;

	OutputMemoryStream output_stream;
	header.Write(output_stream);
	packet.Write(output_stream);

	sendPacketToAgent(receiver_loc.hostIP, receiver_loc.hostPort, output_stream);
}
