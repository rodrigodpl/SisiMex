#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

enum UCP_State
{
	UCP_ST_INIT,
	UCP_ST_REQUESTING_UCCs,
	UCP_ST_ITERATING_OVER_UCCs,

	// TODO: Other states

	UCP_ST_NEGOTIATION_FAILED,
	UCP_ST_NEGOTIATION_SUCCEEDED,
	UCP_ST_NEGOTIATION_FINISHED
};

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLoc, unsigned int searchDepth) :
		Agent(node), requestedItemId(requestedItemId), contributedItemId(contributedItemId), uccLoc(uccLoc), searchDepth(searchDepth) {};

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	uint16_t requestedItemId = 0;
	uint16_t contributedItemId = 0;
	const AgentLocation uccLoc;
	unsigned int searchDepth = 0;
};

