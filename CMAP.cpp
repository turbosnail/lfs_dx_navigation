#include "stdafx.h"
#include "CMAP.h"
#include "LFSNavSystem.h"

MAP::MAP()
	{
	//ArrbufMap = NULL;
	Color = D3DCOLOR_ARGB(255, 255, 255, 255);

	bInitialized = false;
	}

MAP::MAP(float x, float y)
	{
	MAP();
	}

bool MAP::Initialize( IDirect3DDevice9 *device, std::string MapFName, std::string MaskFName )
	{
	if (!SUCCEEDED(D3DXCreateTextureFromFile(device, MapFName.c_str(),&PMapTexture))) return false;
	HRESULT _Res = D3DXCreateTextureFromFile(device, MaskFName.c_str(),  &PMaskTexture);
	if (!SUCCEEDED(_Res)) return false;
	if (!SUCCEEDED(D3DXCreateSprite(device, &PSprite))) 	return false;
	
	SetRender(D3DXVECTOR2(0,0), D3DXVECTOR2(100,100));
	SetMapCoordinates(D3DXVECTOR2(0,0), D3DXVECTOR2(1,1), 0);
	
	bInitialized = true;
	return true;
	}

bool MAP::Initialized()
	{
	return bInitialized;
	}

void MAP::Update(float time)
	{

	}

void MAP::SetRender( D3DXVECTOR2& vec2Pos, D3DXVECTOR2& vec2Sizes )
	{
	vec3RenderPos = D3DXVECTOR3(vec2Pos.x - vec2Sizes.x/2, vec2Pos.y-vec3RenderSizes.y/2, 0);
	vec3RenderSizes = D3DXVECTOR3(vec2Sizes.x, vec2Sizes.y, 0);
	vec2RotationCenter = vec2Pos;

	//Для карты нужны особые размеры
	//Vertex for map
	float Radius = vec2Sizes.x/2;
	static D3DXVECTOR3 Pos; Pos.x = 0; Pos.y = 0; Pos.z = 0;
	ArrbufMap[0].Pos = Pos;
	for(int J = 0; J < MAPPOINTS-1; J++)
		{
		int I = J+1;
		Pos.x = cos(J * 2.0f * float(M_PI) / (MAPPOINTS-2)) * Radius;
		Pos.y = sin(J * 2.0f * float(M_PI) / (MAPPOINTS-2)) * Radius;
		ArrbufMap[I].Pos = Pos;
		}

	//Translation
	D3DXVECTOR3 vecTranslate(vec2Pos.x, vec2Pos.y, 0);
	for(int I = 0; I < MAPPOINTS; I++)
		D3DXVec3Add(&ArrbufMap[I].Pos, &ArrbufMap[I].Pos, &vecTranslate);
	}

void MAP::SetMapCoordinates( D3DXVECTOR2& vec2UVPos, D3DXVECTOR2& vec2UVSizes, float Rotation )
	{
	static D3DXVECTOR3 vecRaduis;
	D3DXVec3Subtract(&vecRaduis, &ArrbufMap[0].Pos, &ArrbufMap[1].Pos);
	float Radius = D3DXVec3Length(&vecRaduis);

	for(int J = 0; J < MAPPOINTS; J++)
		{
		float RelativePosX = (ArrbufMap[J].Pos.x -  ArrbufMap[0].Pos.x) / Radius;
		float RelativePosY = (ArrbufMap[J].Pos.y -  ArrbufMap[0].Pos.y) / Radius;
		float Alpha = atan2(RelativePosY, RelativePosX);
		Alpha += -Rotation + M_PI;
		if (J != 0)
			{
			ArrbufMap[J].Tex.x = vec2UVPos.x - cos(Alpha) * vec2UVSizes.x;
			ArrbufMap[J].Tex.y = vec2UVPos.y - sin(Alpha) * vec2UVSizes.y ;
			}
		else
			{
			ArrbufMap[J].Tex.x = vec2UVPos.x;
			ArrbufMap[J].Tex.y = vec2UVPos.y;
			}
		}
	}


void MAP::Draw( D3DXVECTOR2& vec2SrcCenter, float Rotate, float Scale )
	{
	static LPDIRECT3DDEVICE9 pD3D = NULL;
	if (!pD3D) PSprite->GetDevice(&pD3D);

	if (PSprite && PMapTexture && pD3D)
		{
		//Области для изображений
		RECT _SrcRect = {LONG(vec2SrcCenter.x - vec3RenderSizes.x/2),
			LONG(vec2SrcCenter.y - vec3RenderSizes.y/2),
			LONG(vec2SrcCenter.x + vec3RenderSizes.x/2),
			LONG(vec2SrcCenter.y + vec3RenderSizes.y/2)};
		static D3DXVECTOR3 vecMapCorner(0, 0, 0);

		RECT _SrcMaskRect = {0, 0, MASK_IMAGE_SIZE, MASK_IMAGE_SIZE};
		static D3DXVECTOR3 vecMaskCorner(-vec3RenderSizes.x/2 + MASK_IMAGE_SIZE/2, 
			-vec3RenderSizes.y/2 + MASK_IMAGE_SIZE/2, 0);

		if (ArrbufMap)
			{
			D3DXVECTOR2 vec2ScaledCenter (vec2SrcCenter.x / MAP_IMAGE_WIDTH, vec2SrcCenter.y / MAP_IMAGE_WIDTH);
			D3DXVECTOR2 vec2Sizes (Scale, Scale);
			
			SetMapCoordinates(vec2ScaledCenter, vec2Sizes, Rotate);

			pD3D->SetTexture(0, PMapTexture);

			static DWORD FVFSave;
			pD3D->GetFVF(&FVFSave);

			pD3D->SetFVF(MAPFVF_DECS);
			pD3D->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, MAPPOINTS-2 , ArrbufMap, sizeof(MAPFVF));

			pD3D->SetFVF(FVFSave);
			}

		if (PSprite)
			{
			//поворот вокруг центра
			static D3DXMATRIX mtrTramsform;
			D3DXMatrixTransformation2D(&mtrTramsform, NULL, NULL, NULL, &vec2RotationCenter, Rotate, NULL);
			PSprite->SetTransform(&mtrTramsform);	

			PSprite->Begin(D3DXSPRITE_ALPHABLEND);
			PSprite->Draw(PMaskTexture, &_SrcMaskRect, &vecMaskCorner, &vec3RenderPos, Color);

			//Все это может пригодиться для прозрачной карты итд
			//pD3D->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD );
			//pD3D->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_INVSRCCOLOR);
			//pD3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);

			//pD3D->SetTexture(0, PMapTexture);
			//pD3D->SetTexture(1, PMaskTexture);

			//pD3D->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);   
			//pD3D->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			//pD3D->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			//pD3D->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1); 
			//pD3D->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

			//pD3D->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE); 
			//pD3D->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);

			//pD3D->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE); 
			//pD3D->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

			// pD3D->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			// pD3D->SetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			// pD3D->SetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
			// pD3D->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );		
			// pD3D->SetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
			// pD3D->SetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_PASSTHRU );

			//PSprite->Draw(PMapTexture, &_SrcRect, &vecMapCorner, &vec3RenderPos, Color);
		
			//pD3D->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_MAX );
			//pD3D->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			//pD3D->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_DESTCOLOR);
			//PSprite->Draw(PMaskTexture, &_SrcMaskRect, &vecMaskCorner, &vec3RenderPos, Color);

			PSprite->End();
			}
		}
	}


MAP::~MAP()
	{
	if (PSprite)
		{
		PSprite->Release();
		PSprite = NULL;
		}

	if (PMapTexture)
		{
		PMapTexture->Release();
		PMapTexture = NULL;
		}
	if (PMapTexture)
		{
		//ArrbufMap->Release();
		PMapTexture = NULL;
		}
	}

void
	MAP::OnLostDevice()
	{
	if (PSprite)
		PSprite->OnLostDevice();
	}

void
	MAP::OnResetDevice()
	{
	if (PSprite)
		PSprite->OnResetDevice();
	}