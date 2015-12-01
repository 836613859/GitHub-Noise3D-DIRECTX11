
/***********************************************************************

                           h��NoiseGlobal

************************************************************************/


#pragma once
#include "Noise3D.h"

const UINT					g_VertexDesc_Default_ElementCount = 5;

const UINT					g_VertexDesc_Simple_ElementCount = 3;

//����Ⱦ��������سߴ�
extern UINT					gMainBufferPixelWidth;

extern UINT					gMainBufferPixelHeight;

//�����Ʒ��
extern UINT						g_Device_MSAA4xQuality ;
//������Ƿ���
extern BOOL						g_Device_MSAA4xEnabled ;
//
extern D3D_DRIVER_TYPE			g_Device_driverType;
//
extern D3D_FEATURE_LEVEL		g_Device_featureLevel;
//���ڵ���������Ϣ��ostringstream
extern std::ostringstream			g_Debug_MsgString;
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
extern HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut );

extern BOOL gFunction_IsPointInRect2D(NVECTOR2 v, NVECTOR2 vTopLeft, NVECTOR2 vBottomRight);

extern int	gFunction_GetCharAlignmentOffsetPixelY(UINT boundaryPxHeight, UINT charRealHeight, wchar_t inputChar);

extern float gFunction_Lerp(float a, float b, float t);

/*template <typename T>
extern T gFunction_Clamp(T val, T min, T max);*/
extern float gFunction_Clamp(float val, float min, float max);
