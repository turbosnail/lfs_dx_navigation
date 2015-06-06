#pragma once
#include "stdafx.h"

typedef class GAMEINFO
	{
	public:
		GAMEINFO();
		~GAMEINFO();

		//functio for external pthread start
		static void* DataLoop(void*);
		bool bConnected;
		static void MCIPacketCallback( CInsim* InSim, void* Packet );
		static IS_MCI PlayerMCI;
		CInsim Insim;
	} GAMEINFO;