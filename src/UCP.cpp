#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"
#include "ModuleNodeCluster.h"

// TODO: Make an enum with the states

void UCP::update()
{
	switch (state())
	{
	case UCP_ST_INIT:
		break;

	case UCP_ST_REQUESTING_UCCs:

		break;

	case UCP_ST_ITERATING_OVER_UCCs:

		break;
			// TODO: Other states
		
	case UCP_ST_NEGOTIATION_FINISHED:
		destroy();
		break;
		// TODO: Handle states

	default:
		break;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::AgreementRequestResponseSucced:

		//The final set was found, end here. 

		break;

	case PacketType::AgreementRequestResponseContinue:

	{

		PacketAgreementConstrain packetData;
		packetData.Read(stream);

		int next_contributed_item;
		next_contributed_item = packetData.constrain;

		AgentLocation ucc_location;

		ucc_location.hostIP = socket->RemoteAddress().GetIPString();
		ucc_location.hostPort = LISTEN_PORT_AGENTS;
		ucc_location.agentId = node()->id;

		MCP new_mcp = *App->agentContainer->createMCP(node(), requestedItemId, next_contributed_item, parent_mcp->searchDepth() + 1);
		UCP new_ucp = *App->agentContainer->createUCP(node(), requestedItemId, next_contributed_item, ucc_location, MAX_SEARCH_DEPTH, &new_mcp);
	}

		break;
		
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void UCP::sendAgreeementRequest()
{
	PacketHeader header;
	header.srcAgentId = id();
	header.packetType = PacketType::AgreementRequest;
	header.dstAgentId = uccLoc.agentId;

	PacketAgreementRequest packet;
	packet.offered_itemId = contributedItemId;
	packet.requested_itemId = requestedItemId;
	
	OutputMemoryStream stream;
	header.Write(stream);
	packet.Write(stream);

	sendPacketToAgent(uccLoc.hostIP, uccLoc.hostPort, stream);
}
