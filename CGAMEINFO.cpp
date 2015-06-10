#include "stdafx.h"
#include "CGAMEINFO.h"

namespace GAMEINFO
	{
	CInsim Insim;


	//function for external pthread start
	void* DataLoop(void*);
	void MCIPacketCallback( CInsim* PInSim, void* Packet );
	void CCPPacketCallback( CInsim* PInSim, void* Packet );
	CompCar LastPlayerCar;
	IS_MCI LastMCI;
	IS_CPP LastCPP;
	int ViewId;

	std::ostringstream LocalMessageBuf;
	void SendLocalMessage();
	};

//shame yes
void PacketCallback( CInsim* InSim, void* Packet );

void* GAMEINFO::DataLoop( void* pThis)
	{
	CInsim& ThisInsim = Insim;
	
	//Connection loop
	int _Res = -1;
	while (_Res < 0)
		{
		_Res = ThisInsim.init("127.0.0.1", 29999, "LFSNavSystem", "", (IS_VER*)0,
			(byte)'\000', ISF_MCI, 10, (word)0U);
		Sleep(10);
		}
	
	ThisInsim.Bind(ISP_MCI, &GAMEINFO::MCIPacketCallback);
	ThisInsim.Bind(ISP_CPP, &GAMEINFO::CCPPacketCallback);
	
	//Recieve loop
	int Counter = 0;
	while (_Res >= 0)
		{
		if (Counter > 10000) Counter = 0;

		if (Counter%10 == 0) ThisInsim.SendTiny(TINY_SCP, 2);
		_Res = ThisInsim.next_packet();
		
		Sleep(10);
		Counter++;
		}

	return NULL;
	}


//must be out of class static function
void GAMEINFO::MCIPacketCallback( CInsim* PInSim, void* Packet )
	{
	//if  (!InSim || !Packet) return;
	IS_MCI* Info = (IS_MCI*)Packet;
	if (Info->Type != ISP_MCI) return; //need crash here

	memcpy(&GAMEINFO::LastMCI, Info, sizeof(IS_MCI));

	//Инфа о машине игрока
	for (int I = 0; I < 8; I++ )
		if (LastMCI.Info[I].PLID == ViewId) 	memcpy(&LastPlayerCar, &LastMCI.Info[I], sizeof(CompCar));
	}

void GAMEINFO::CCPPacketCallback( CInsim* PInSim, void* Packet )
	{
	//if  (!InSim || !Packet) return;
	IS_CPP* Info = (IS_CPP*)Packet;
	if (Info->Type != ISP_CPP) return; //need crash here

	memcpy(&GAMEINFO::LastCPP, Info, sizeof(IS_CPP));
	
	ViewId = LastCPP.ViewPLID;
	}

void GAMEINFO::SendLocalMessage()
	{
	std::string Str(LocalMessageBuf.str());	

	static IS_MSL pack;
	memset(&pack, 0, sizeof(IS_MSL));
	pack.Size = sizeof(IS_MSL);
	pack.Type = ISP_MSL;
	strcpy(pack.Msg, Str.c_str());
	Insim.send_packet(&pack);

	LocalMessageBuf.flush();
	LocalMessageBuf.str("");
	}
