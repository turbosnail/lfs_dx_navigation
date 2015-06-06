#include "stdafx.h"

Sprite::Sprite()
	{

	color = D3DCOLOR_ARGB(128, 255, 255, 255);

	initialized = false;
	}

Sprite::Sprite(float x, float y)
	{	

	color = D3DCOLOR_ARGB(128, 255, 255, 255);

	initialized = false;
	}

bool Sprite::Initialize( IDirect3DDevice9 *device, std::string fileName )
	{
	if (!SUCCEEDED(D3DXCreateTextureFromFile(device, fileName.c_str(),&tex)))
		{
		//MessageBox(NULL, "Can't load texture", NULL, MB_OK);
		return false;
		}

	if (!SUCCEEDED(D3DXCreateSprite(device, &sprite)))
		{
		//MessageBox(NULL, "Can't create sprite", NULL, MB_OK);
		return false;
		}


	initialized = true;
	return true;
	}

bool Sprite::Initialized()
	{
	return initialized;
	}

void Sprite::Update(float time)
	{

	}

void 
	Sprite::SetRenderWH(float widht, float height)
	{
	render_width = widht;
	render_height = height;
	}

void Sprite::Draw(float positionX, float positionY, float srcX, float srcY, float srcWidth, float srcHeight, float rotate, float scale)
	{
	if (sprite && tex)
		{
		sprite->Begin(D3DXSPRITE_ALPHABLEND);
		RECT rect = { (LONG)srcX, (LONG)srcY, (LONG)(srcX + srcWidth), (LONG)(srcY + srcHeight) };

		sprite->Draw(tex, &rect, 0, 0, color);
		sprite->End();

		}
	}


Sprite::~Sprite()
	{
	if (sprite)
		{
		sprite->Release();
		sprite = NULL;
		}

	if (tex)
		{
		tex->Release();
		tex = NULL;
		}
	}

void
	Sprite::OnLostDevice()
	{
	if (sprite)
		sprite->OnLostDevice();
	}

void
	Sprite::OnResetDevice()
	{
	if (sprite)
		sprite->OnResetDevice();
	}