
/***********************************************************************

                           h��NoiseGlobal

************************************************************************/


#pragma once
#include "Noise3D.h"

#define NOISE_DLL_EXPORT _declspec(dllexport)



const UINT					g_VertexDesc_Default_ElementCount = 5;

const UINT					g_VertexDesc_Simple_ElementCount = 3;

//����Ⱦ��������سߴ�
extern  UINT	gMainBufferPixelWidth;

extern  UINT	gMainBufferPixelHeight;

//�����Ʒ��
extern UINT						g_Device_MSAA4xQuality ;
//������Ƿ���
extern BOOL						g_Device_MSAA4xEnabled ;
//
extern D3D_DRIVER_TYPE			g_Device_driverType;
//
extern D3D_FEATURE_LEVEL		g_Device_featureLevel;

//��������
extern D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Default[g_VertexDesc_Default_ElementCount];

extern D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Simple[g_VertexDesc_Simple_ElementCount];

//��������������������ȫ�ֽӿڡ�������������������

extern ID3D11Device*					g_pd3dDevice11;

extern ID3D11DeviceContext*		g_pImmediateContext;
//������
extern IDXGISwapChain*				g_pSwapChain;
//��Ⱦ�ӿ� ���������� ���ں�pipeline��
extern ID3D11RenderTargetView* g_pRenderTargetView;
//���&ģ�� ֻ����һ��
extern ID3D11DepthStencilView*	g_pDepthStencilView;
//���㲼��
extern ID3D11InputLayout*			g_pVertexLayout_Default;

extern ID3D11InputLayout*			g_pVertexLayout_Simple;

//��������������������ȫ�ֺ���������������������������
extern NOISE_DLL_EXPORT HRESULT gFunction_CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

extern NOISE_DLL_EXPORT BOOL gFunction_IsPointInRect2D(NVECTOR2 v, NVECTOR2 vTopLeft, NVECTOR2 vBottomRight);

extern NOISE_DLL_EXPORT int	gFunction_GetCharAlignmentOffsetPixelY(UINT boundaryPxHeight, UINT charRealHeight, wchar_t inputChar);

extern NOISE_DLL_EXPORT UINT gFunction_MapDInputScanCodeToAscii(UINT scanCode,BOOL isCapital);

extern NOISE_DLL_EXPORT std::string GetFileDirectory(std::string completeFilePath);

extern NOISE_DLL_EXPORT float gFunction_Lerp(float a, float b, float t);

extern NOISE_DLL_EXPORT float gFunction_Clampf(float val, float min, float max);

extern NOISE_DLL_EXPORT int	gFunction_Clamp(int val, int min, int max);

extern NOISE_DLL_EXPORT void DEBUG_MSG1(std::string msg);