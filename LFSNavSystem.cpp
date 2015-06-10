#include "stdafx.h"
#include "LFSNavSystem.h"

#include <sstream>
using namespace std;

LFSNavSystem *LFSNavSystem::self = nullptr;

LFSNavSystem *LFSNavSystem::getInstance()
	{
	if(!self) self = new LFSNavSystem();

	return self;
	}

LFSNavSystem::LFSNavSystem()
	{
	vec2ImagePlayerPos = D3DXVECTOR2(0,0);

	//the way to start Insim witouth blocking game graphic
	pthread_create(&ThInfo, NULL, &GAMEINFO::DataLoop, NULL);
	}

LFSNavSystem::~LFSNavSystem()
	{
	pthread_cancel(ThInfo);
	//pthread_mutex_destroy(&ismutex);

	if(self)
		{
		self->Realize();
		delete self;
		}
	}

void LFSNavSystem::Initialize(IDirect3DDevice9 *pD3DDevice)
	{
	OutputDebugString("LFSNavSystem initialize.\r\n");

	m_pD3DDevice = pD3DDevice;

	PMap = new MAP();
	PMap->Initialize(pD3DDevice, MAP_FILE_NAME, MASK_FILE_NAME);

	frameCount = 0;
	frameSec = timeGetTime();
	}

void LFSNavSystem::Realize()
	{
	if(PMap) delete PMap;
	}

void LFSNavSystem::OnLostDevice()
	{
	if(PMap) PMap->OnLostDevice();
	}

void LFSNavSystem::OnResetDevice()
	{
	if(PMap) PMap->OnResetDevice();
	}

void LFSNavSystem::Render()
	{
	m_pD3DDevice->BeginScene();

	D3DVIEWPORT9 vp;
	m_pD3DDevice->GetViewport(&vp);

// 	window_aspect = (float)(vp.Width / vp.Height);
// 	window_offset = 0;
// 	if(window_aspect >= 3)
// 		{
// 		vp.Width /= 3;
// 		window_offset = (float)vp.Width;
// 		}
// 		
	ScreenWidth = (float)(vp.Width);
	ScreenHeight = (float)(vp.Height);

	static D3DXVECTOR2 _vec2LastPos;
	_vec2LastPos.x = float(GAMEINFO::LastPlayerCar.X);
	_vec2LastPos.y = float(GAMEINFO::LastPlayerCar.Y);
	MapInGamePosToPic(_vec2LastPos, vec2ImagePlayerPos);

	if(PMap) PMap->SetRender(D3DXVECTOR2(MAP_POSX, MAP_POSY), D3DXVECTOR2( MASK_IMAGE_SIZE-50, MASK_IMAGE_SIZE-50));
	if(PMap) PMap->Draw(vec2ImagePlayerPos, GAMEINFO::LastPlayerCar.Heading * float(M_PI) / 32768.0f , 0.1f);

	_frameSec = timeGetTime();
	frameCount++;

	frameSec = _frameSec;

	m_pD3DDevice->EndScene();
	// m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
	}

void LFSNavSystem::MapInGamePosToPic( D3DXVECTOR2& vec2InGamePos, D3DXVECTOR2& Outvec2ImagePos )
	{
	//функция точно корректная игровые координаты-> изображение
	//проблема с отрисовкой
	Outvec2ImagePos.x = MAP_IMAGE_SCALE_FACTOR * vec2InGamePos.x + MAP_IMAGE_WIDTH / 2;
	Outvec2ImagePos.y = -MAP_IMAGE_SCALE_FACTOR * vec2InGamePos.y + MAP_IMAGE_HEIGHT / 2;


// 	if (rand() > 32755)
// 		{
// 		GAMEINFO::LocalMessageBuf << "X " << vec2InGamePos  << " Y " << Outvec2ImagePos << "    ";
// 		GAMEINFO::LocalMessageBuf << "mapX " << OutX  << " mapY " << OutY;
// 		GAMEINFO::SendLocalMessage();
// 		}
	}

void SendLocalMessage(std::ostringstream s)
	{
	const std::string Str(s.str());	

	IS_MSL *pack = new IS_MSL;
	memset(pack, 0, sizeof(IS_MSL));
	pack->Size = sizeof(IS_MSL);
	pack->Type = ISP_MSL;
	strcpy(pack->Msg, Str.c_str());
	GAMEINFO::Insim.send_packet(pack);
	delete pack;
	}