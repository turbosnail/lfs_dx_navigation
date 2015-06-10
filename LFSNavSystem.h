#pragma once

#include "CMAP.h"
#include "cinsim.h"
#include "CGAMEINFO.h"

//индусим
#define MAP_IMAGE_SCALE_FACTOR 0.00001208f
#define MAP_IMAGE_WIDTH 2048
#define MAP_IMAGE_HEIGHT 2048

#define MAP_POSX 100
#define MAP_POSY 100

#define MASK_IMAGE_SIZE 512

#define MAP_FILE_NAME "LFSNavSystem\\WE.jpg"
#define MASK_FILE_NAME "LFSNavSystem\\Mask.png"

class LFSNavSystem
{
public:
	
	static LFSNavSystem* getInstance();
	void Initialize(IDirect3DDevice9 *pD3DDevice);
	void Realize();

	void Render();
	void OnLostDevice();
	void OnResetDevice();

private:
	LFSNavSystem();
	~LFSNavSystem();

	void MapInGamePosToPic(D3DXVECTOR2& vec2InGamePos, D3DXVECTOR2& Outvec2ImagePos);

	static LFSNavSystem *self;

	IDirect3DDevice9*	m_pD3DDevice;

	MAP *PMap;

	int					frameCount;
	DWORD	frameSec, _frameSec;

	float			ScreenWidth;
	float			ScreenHeight;


	D3DXVECTOR2 vec2ImagePlayerPos;

	//Ingame data
	pthread_t ThInfo;
};