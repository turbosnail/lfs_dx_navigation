#include "stdafx.h"

//Temp
//std::ostringstream s;
#include <sstream>

LFSNavSystem *LFSNavSystem::self = nullptr;

LFSNavSystem *LFSNavSystem::getInstance()
	{
	if(!self) self = new LFSNavSystem();

	return self;
	}

LFSNavSystem::LFSNavSystem()
	{
	mapX = 0;
	mapY = 0;

	//the way to start Insim witouth blocking game graphic
	pthread_create(&ThInfo, NULL, &GAMEINFO::DataLoop, (void*)(&GameInfo));
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

	map = new Sprite();
	map->Initialize(pD3DDevice, "LFSNavSystem\\WE.jpg");

	frameCount = 0;
	frameSec = timeGetTime();
	}

void LFSNavSystem::Realize()
	{
	if(map) delete map;
	}

void LFSNavSystem::OnLostDevice()
	{
	if(map) map->OnLostDevice();
	}

void LFSNavSystem::OnResetDevice()
	{
	if(map) map->OnResetDevice();
	}

void LFSNavSystem::Render()
	{
	m_pD3DDevice->BeginScene();

	D3DVIEWPORT9 vp;
	m_pD3DDevice->GetViewport(&vp);

	window_aspect = (float)(vp.Width / vp.Height);
	window_offset = 0;
	if(window_aspect >= 3)
		{
		vp.Width /= 3;
		window_offset = (float)vp.Width;
		}
	render_width = (float)(vp.Width);
	render_height = (float)(vp.Height);

	//какая-то проблема с координатами при отрисовке, т.к. игровые координаты точно проецируются на изображение
	if(map) map->SetRenderWH(render_width, render_height);
	if(map) map->Draw(render_width - 200, render_height - 200, mapX - 100, mapY - 100, 200, 200, 0, 1);

	MapInGamePosToPic(GAMEINFO::PlayerMCI.Info[0].X, GAMEINFO::PlayerMCI.Info[0].Y, mapX, mapY);

	//if(mapX > (1280 - 200)) mapX = 0;
	//if(mapY > (1280 - 200)) mapY = 0;

	_frameSec = timeGetTime();
	frameCount++;

	frameSec = _frameSec;

	m_pD3DDevice->EndScene();
	// m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
	}

void LFSNavSystem::MapInGamePosToPic( int X, int Y, float& OutX, float& OutY )
	{
	//функция точно корректная игровые координаты-> изображение
	//проблема с отрисовкой
	OutX = 0.00001f * X + 1024;
	OutY = -0.00001f * Y + 1024;


	if (rand() > 32755)
		{
		//Temp
		std::ostringstream s;
		s << "X " << X  << " Y " << Y << "    ";
		s << "mapX " << OutX  << " mapY " << OutY;
		const std::string Str(s.str());	
		GameInfo.Insim.SendMSX(Str);
		}
	}