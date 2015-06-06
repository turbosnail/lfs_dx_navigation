#pragma once

class Sprite
{
public:
	Sprite();
	Sprite(float x, float y);
	~Sprite();

	bool Initialize(IDirect3DDevice9 *device, std::string fileName);
	bool Initialized();
	void Update(float time);
	void Draw(float positionX, float positionY, float srcX, float srcY, float srcWidth, float srcHeight, float rotate, float scale);
	void SetRenderWH(float widht, float height);

	void OnLostDevice(void);
	void OnResetDevice(void);

private:
	LPD3DXSPRITE		sprite;
	LPDIRECT3DTEXTURE9	tex;

	D3DXVECTOR3			position;
	D3DXCOLOR			color;
	bool				initialized;

	float			render_width;
	float			render_height;
};

