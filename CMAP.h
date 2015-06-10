#pragma once

class MAP
{
public:
	MAP();
	MAP(float x, float y);
	~MAP();

	bool Initialize(IDirect3DDevice9 *device, std::string MapFName, std::string MaskFName);
	bool Initialized();
	void Update(float time);
	void Draw(D3DXVECTOR2& vec2SrcCenter, float Rotate, float Scale);
	void SetRender(D3DXVECTOR2& vec2Pos, D3DXVECTOR2& vec2Sizes);
	void SetMapCoordinates(D3DXVECTOR2& vec2UVPos, D3DXVECTOR2& vec2UVSizes, float Rotation);

	void OnLostDevice(void);
	void OnResetDevice(void);

private:
	LPD3DXSPRITE		PSprite;

	LPDIRECT3DTEXTURE9	PMapTexture;
	LPDIRECT3DTEXTURE9	PMaskTexture;
	
	struct MAPFVF { D3DXVECTOR3 Pos; float Rhw; D3DXVECTOR2 Tex; }; typedef MAPFVF* PMAPFVF;
    #define MAPFVF_DECS (D3DFVF_XYZRHW | D3DFVF_TEX1)
    #define MAPPOINTS 64
	MAPFVF ArrbufMap[MAPPOINTS+2];


	D3DXVECTOR3			vec3RenderPos;
	D3DXVECTOR3			vec3RenderSizes;
	D3DXVECTOR2			vec2RotationCenter;
	D3DXCOLOR			Color;
	bool				bInitialized;
};