#pragma once
#include "stdafx.h"

namespace GAMEINFO
	{
		extern CInsim Insim;

		
		//function for external pthread start
		extern void* DataLoop(void*);
		extern void MCIPacketCallback( CInsim* PInSim, void* Packet );
		extern void CCPPacketCallback( CInsim* PInSim, void* Packet );
		extern CompCar LastPlayerCar;
		extern IS_MCI LastMCI;
		extern IS_CPP LastCPP;
		extern int ViewId;

		extern std::ostringstream LocalMessageBuf;
		extern void SendLocalMessage();
	};

#define UCID_HOST 0