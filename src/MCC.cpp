#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	ST_WAITING_RESULT,
	
	// TODO: Other states
	ST_UNREGISTERING,
	ST_FINISHED
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
	case ST_INIT:
		if (registerIntoYellowPages()) {
			setState(ST_REGISTERING);
		}
		else {
			setState(ST_FINISHED);
		}
		break;
		
	case ST_WAITING_RESULT:
		if (_ucc->state == UCC_SUCCEEDED)
		{
			unregisterFromYellowPages();
			setState(ST_UNREGISTERING);
		}
		else if (_ucc->state == UCC_FAILED)
		{
			if (_ucc != nullptr)
			{
				_ucc->stop();
				_ucc.reset();
			}
			setState(ST_IDLE);
		}
		break;

	case ST_FINISHED:
		destroy();
		break;
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	destroyChildUCC();

	unregisterFromYellowPages();
	setState(ST_FINISHED);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
			socket->Disconnect();
		}
		else
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		
		break;

	case PacketType::NegotationRequest:
		if (state() == ST_IDLE)
		{
			AgentLocation receiver_loc;
			receiver_loc.hostIP = socket->RemoteAddress().GetIPString();
			receiver_loc.hostPort = LISTEN_PORT_AGENTS;
			receiver_loc.agentId = packetHeader.srcAgentId;

			sendNegotiationAcceptedPacket(receiver_loc);
			setState(ST_WAITING_RESULT);

			socket->Disconnect();
		}
		else
			wLog << "OnPacketReceived() - PacketType::NegotationRequest was unexpected.";

		break;
		
	case PacketType::UnregisterMCC:
		if (state() == ST_UNREGISTERING)
			setState(ST_FINISHED);
		else
			wLog << "OnPacketReceived() - PacketType::NegotationRequest was unexpected.";

		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return negotiationFinished();
}


bool  MCC::sendNegotiationAcceptedPacket(AgentLocation receiver_loc)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::NegotationRequestResponse;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = receiver_loc.agentId;
	PacketNegotationRequestResponse packetData;
	packetData.result = true;

	_ucc = App->agentContainer->createUCC(node(), _contributedItemId, _constraintItemId);
	_ucc->parent_mcc = this; 

	packetData.id = _ucc->id();

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToAgent(receiver_loc.hostIP, receiver_loc.hostPort, stream);
}

bool MCC::registerIntoYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::unregisterFromYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	sendPacketToYellowPages(stream);
}

void MCC::createChildUCC()
{
	// TODO: Create a unicast contributor
}

void MCC::destroyChildUCC()
{
	// TODO: Destroy the unicast contributor child
}
