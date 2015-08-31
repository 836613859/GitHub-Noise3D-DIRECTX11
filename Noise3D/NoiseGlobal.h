
/***********************************************************************

                           h��NoiseGlobal

************************************************************************/


#pragma once
#include "Noise3D.h"

;
CONST UINT					g_VertexDesc_Simple_ElementNum = 3;

CONST UINT					g_VertexDesc_Default_ElementNum = 4;

//����Ⱦ��������سߴ�
extern UINT					g_MainBufferPixelWidth;

extern UINT					g_MainBufferPixelHeight;

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
extern D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Default[];

extern D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Simple[];

//��������������������ȫ�ֽӿڡ�������������������

extern ID3D11Device*					g_pd3dDevice;

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