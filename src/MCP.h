#pragma once
#include "Agent.h"

// Forward declaration
class UCP;
using UCPPtr = std::shared_ptr<UCP>;

class MCP :
	public Agent
{
public:

	// Constructor and destructor
	MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth);
	~MCP();

	// Agent methods
	void update() override;
	void stop() override;
	MCP* asMCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	void AddChildUCP(UCP* new_ucp);

	// Getters
	uint16_t requestedItemId() const { return _requestedItemId; }
	uint16_t contributedItemId() const { return _contributedItemId; }

	// Whether or not the negotiation finished
	bool negotiationFinished() const;

	// Whether or not there was a negotiation agreement
	bool negotiationAgreement() const;

	// It returns the search depth of this MCP
	unsigned int searchDepth() const { return _searchDepth; }

	MCP* parent_mcp = nullptr;

private:

	bool queryMCCsForItem(int itemId);
	bool requestNegotation(const AgentLocation& receiver_loc);

	uint16_t _requestedItemId;
	uint16_t _contributedItemId;

	int _mccRegisterIndex; /**< Iterator through _mccRegisters. */
	std::vector<AgentLocation> _mccRegisters; /**< MCCs returned by the YP. */

	unsigned int _searchDepth = 0;

	std::vector<UCP> child_ucp;  // UCP dependant on this MCP
};

