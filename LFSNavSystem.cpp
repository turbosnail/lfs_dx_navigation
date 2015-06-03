#include "stdafx.h"

LFSNavSystem*
LFSNavSystem::self = nullptr;

LFSNavSystem*
LFSNavSystem::getInstance()
{
	if (!self)
		self = new LFSNavSystem();

	return self;
}

LFSNavSystem::LFSNavSystem()
{
	
}

LFSNavSystem::~LFSNavSystem()
{
	if (self)
	{
		self->Realize();
		delete self;
	}
		
}

void 
LFSNavSystem::Initialize(IDirect3DDevice9 *pD3DDevice)
{
	OutputDebugString("LFSNavSystem initialize.\r\n");

	m_pD3DDevice = pD3DDevice;

	map = new Sprite();
	map->Initialize(pD3DDevice, "LFSNavSystem\\WE.jpg",2560,2560);

	frameCount = 0;
	frameSec = timeGetTime();
}

void
LFSNavSystem::Realize()
{
	if (map)
		delete map;
}

void
LFSNavSystem::OnLostDevice()
{
	if (map)
		map->OnLostDevice();
}

void
LFSNavSystem::OnResetDevice()
{
	if (map)
		map->OnResetDevice();
}

void
LFSNavSystem::Render()
{
	m_pD3DDevice->BeginScene();
	
	D3DVIEWPORT9 vp;
	m_pD3DDevice->GetViewport(&vp);

	window_aspect = (float)(vp.Width / vp.Height);
	window_offset = 0;
	if (window_aspect >= 3)
	{
		vp.Width /= 3;
		window_offset = (float)vp.Width;
	}
	render_width = (float)(vp.Width);
	render_height = (float)(vp.Height);

	if (map)
		map->SetRenderWH(render_width, render_height);
	
	if (map)
		map->Draw(render_width - 200, render_height-200, mapX, mapY, 200, 200, 0, 0.5);

	mapX++;
	mapY++;

	if (mapX > (1280 - 200))
		mapX = 0;

	if (mapY > (1280 - 200))
		mapY = 0;

	_frameSec = timeGetTime();
	frameCount++;

	frameSec = _frameSec;
	
	m_pD3DDevice->EndScene();
	//m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}
