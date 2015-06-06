#pragma once

#include "Sprite.h"
#include "cinsim.h"
#include "GAMEINFO.h"

#define MAP_MAX_WIDTH 2560;
#define MAP_MAX_HEIGHT 2560;

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

	void MapInGamePosToPic(int X, int Y, float& OutX, float& OutY);

	static LFSNavSystem *self;

	IDirect3DDevice9*	m_pD3DDevice;

	Sprite *map;

	int					frameCount;
	DWORD	frameSec, _frameSec;

	float			window_aspect;
	float			window_offset;
	float			render_width;
	float			render_height;

	float mapX;
	float mapY;

	//Ingame data
	GAMEINFO GameInfo;
	pthread_t ThInfo;
};