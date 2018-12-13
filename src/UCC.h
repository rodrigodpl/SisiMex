#pragma once
#include "Agent.h"

enum UCC_State { UCC_INIT, UCC_REGISTERING, UCC_IDLE, UCC_UNREGISTERING, UCC_SUCCEEDED, UCC_FAILED, UCC_FINISHED };

class UCC :
	public Agent
{
public:

	// Constructor and destructor
	UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) : Agent(node), contributedItemId(contributedItemId), constraintItemId(constraintItemId) {};
	~UCC() {};

	// Agent methods
	void update() override;
	void stop() override;
	UCC* asUCC() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;

	uint16_t contributedItemId = 0;
	uint16_t constraintItemId = 0;
	UCC_State state = UCC_INIT;
};

