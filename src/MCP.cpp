#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

#define MAX_SEARCH_DEPTH 3

enum State
{
	ST_INIT,
	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,
	ST_WAITING_RESPONSE,
	ST_WAITING_RESULT,
	// TODO: Other states

	ST_NEGOTIATION_FINISHED
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID),
	_searchDepth(searchDepth)
{
	setState(ST_INIT);
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
		queryMCCsForItem(_requestedItemId);
		setState(ST_REQUESTING_MCCs);
		break;

	case ST_ITERATING_OVER_MCCs:
		requestNegotation(_mccRegisters[_mccRegisterIndex]);
		setState(ST_WAITING_RESPONSE);
		break;

	case ST_WAITING_RESULT:

		if (!child_ucp.empty())
		{
			if (child_ucp[child_ucp.size() - 1].state() == UCP_ST_NEGOTIATION_SUCCEEDED)
				setState(ST_NEGOTIATION_FINISHED);
			else if (child_ucp[child_ucp.size() - 1].state() == UCP_ST_NEGOTIATION_FAILED)
			{
				for (auto it = child_ucp.begin(); it != child_ucp.end(); it++)
					(*it).stop();

				child_ucp.clear();

				_mccRegisterIndex++;
			}
		}
		break;

	default:
		break;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)

	destroy();
}

void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Read(stream);

			// Log the returned MCCs
			for (auto &mccdata : packetData.mccAddresses)
			{
				uint16_t agentId = mccdata.agentId;
				const std::string &hostIp = mccdata.hostIP;
				uint16_t hostPort = mccdata.hostPort;
				//iLog << " - MCC: " << agentId << " - host: " << hostIp << ":" << hostPort;
			}

			// Store the returned MCCs from YP
			_mccRegisters.swap(packetData.mccAddresses);

			// Select the first MCC to negociate
			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			socket->Disconnect();
		}
		else
			wLog << "OnPacketReceived() - Unexpected PacketType for current state.";
		
		break;

	case PacketType::NegotationRequestResponse:
		if (state() == ST_WAITING_RESPONSE)
		{
			// Read the packet
			PacketNegotationRequestResponse packetData;
			packetData.Read(stream);

			if (packetData.result)
			{
				AgentLocation ucc_location;

				ucc_location.hostIP = socket->RemoteAddress().GetIPString();
				ucc_location.hostPort = LISTEN_PORT_AGENTS;
				ucc_location.agentId = packetData.id;

				child_ucp.push_back(*App->agentContainer->createUCP(node(), _requestedItemId, _contributedItemId, ucc_location, MAX_SEARCH_DEPTH, this));

				setState(ST_WAITING_RESULT);
			}
			else
			{
				_mccRegisterIndex++;
				setState(ST_ITERATING_OVER_MCCs);
			}

			socket->Disconnect();
		}
		else
			wLog << "OnPacketReceived() - Unexpected PacketType for current state.";
		
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

void MCP::AddChildUCP(UCP * new_ucp)
{
	child_ucp.push_back(*new_ucp); 
}

bool MCP::negotiationFinished() const
{
	return state() == ST_NEGOTIATION_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	return false; // TODO: Did the child UCP find a solution?
}


bool MCP::queryMCCsForItem(int itemId)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}

bool MCP::requestNegotation(const AgentLocation& receiver_loc)
{

	PacketHeader packetHead;
	packetHead.packetType = PacketType::NegotationRequest;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = receiver_loc.agentId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);

	return sendPacketToAgent(receiver_loc.hostIP, receiver_loc.hostPort, stream);
}
