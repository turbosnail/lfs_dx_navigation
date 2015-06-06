#include "stdafx.h"
#include "GAMEINFO.h"

//shame yes
IS_MCI GAMEINFO::PlayerMCI;

void PacketCallback( CInsim* InSim, void* Packet );

GAMEINFO::GAMEINFO()
	{
	bConnected = false;
	}

void* GAMEINFO::DataLoop( void* This)
	{
	//Connection loop
	int _Res = -1;
	do
		{
		_Res = ((GAMEINFO*)This)->Insim.init("127.0.0.1", 29999, "LFSNavSystem", "", (IS_VER*)0,
			(byte)'\000', (word)0U, 1, (word)0U); 
		}
	while (_Res < 0);
	
	((GAMEINFO*)This)->Insim.Bind(ISP_MCI, &GAMEINFO::MCIPacketCallback);
	
	((GAMEINFO*)This)->bConnected = true;

	//Recieve loop
	do
		{ 
		((GAMEINFO*)This)->Insim.SendTiny(TINY_MCI, 1);
		_Res = ((GAMEINFO*)This)->Insim.next_packet(); 
		}
	while (_Res >= 0);

	return NULL;
	}

GAMEINFO::~GAMEINFO()
	{
	Insim.isclose();
	}



//must be out of class static function
void GAMEINFO::MCIPacketCallback( CInsim* InSim, void* Packet )
	{
	//if  (!InSim || !Packet) return;
	IS_MCI* Info = (IS_MCI*)Packet;
	memcpy(&GAMEINFO::PlayerMCI, Info, sizeof(IS_MCI));
	}