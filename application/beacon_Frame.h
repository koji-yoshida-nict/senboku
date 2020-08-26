#ifndef BEACON_FRAME_H_
#define BEACON_FRAME_H_

struct Enhanced_Beacon_Frame_t __attribute__((__packed__))
{
	struct
	{
		uint8_t frameType					:3;
		uint8_t securityEnable				:1;
		uint8_t framePending				:1;
		uint8_t ackRequest					:1;
		uint8_t panID						:1;
		uint8_t reserve						:1;
	} Enhanced_Beacon_Frame_1;
	struct
	{
		uint8_t sequenceNumberSuppression	:1;
		uint8_t IEListPresent				:1;
		uint8_t destinationAddressingMode	:2;
		uint8_t frameVersion				:2;
		uint8_t sourceAddressingMode		:2;
	} Enhanced_Beacon_Frame_2;
};

Enhanced_Beacon_Frame_t IEEE_Beacon_frame =
{
		Enhanced_Beacon_Frame_1.frameType = 0;
		Enhanced_Beacon_Frame_1.securityEnable = 0;
		Enhanced_Beacon_Frame_1.framePending = 0;
		Enhanced_Beacon_Frame_1.ackRequest  = 1;
		Enhanced_Beacon_Frame_1.panID = 0;
		Enhanced_Beacon_Frame_1.reserve = 0;

		Enhanced_Beacon_Frame_2.sequenceNumberSuppression = 0;
		Enhanced_Beacon_Frame_2.IEListPresent = 0;
		Enhanced_Beacon_Frame_2.destinationAddressingMode = 3;
		Enhanced_Beacon_Frame_2.frameVersion = 2;
		Enhanced_Beacon_Frame_2.sourceAddressingMode = 3;
};

struct wisun_Mac_Header_Short_Addr_t
{
	Enhanced_Beacon_Frame_t frameControl;
	uint8_t sequenceNumber;
	uint8_t panID[2];
	uint8_t dAddress[2];
	uint8_t sAddress[8];
	uint16_t headerIE = 0x3F00;
};
