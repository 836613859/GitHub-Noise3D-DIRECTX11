
/***********************************************************************

                           h��NoiseGlobal

************************************************************************/


#pragma once

//#define NOISE_DLL_EXPORT /*_declspec(dllexport)*/

namespace Noise3D
{

	const UINT					g_VertexDesc_Default_ElementCount = 5;

	const UINT					g_VertexDesc_Simple_ElementCount = 3;

	//����Ⱦ��������سߴ�
	extern  UINT					gMainBufferPixelWidth;

	extern  UINT					gMainBufferPixelHeight;

	//�����Ʒ��
	extern UINT						g_Device_MSAA4xQuality;
	//������Ƿ���
	extern BOOL						g_Device_MSAA4xEnabled;
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
	extern /*_declspec(dllexport)*/ HRESULT gFunction_CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

	extern /*_declspec(dllexport)*/ BOOL gFunction_IsPointInRect2D(NVECTOR2 v, NVECTOR2 vTopLeft, NVECTOR2 vBottomRight);

	extern /*_declspec(dllexport)*/ int	gFunction_GetCharAlignmentOffsetPixelY(UINT boundaryPxHeight, UINT charRealHeight, wchar_t inputChar);

	extern /*_declspec(dllexport)*/ std::string GetFileDirectory(std::string completeFilePath);

	extern /*_declspec(dllexport)*/ float Lerp(float a, float b, float t);

	extern /*_declspec(dllexport)*/ float Clamp(float val, float min, float max);

	extern /*_declspec(dllexport)*/ int	Clamp(int val, int min, int max);

	extern /*_declspec(dllexport)*/ NVECTOR3 Clamp(const NVECTOR3& target, const NVECTOR3& min, const NVECTOR3& max);

	//extern /*_declspec(dllexport)*/ void ERROR_MSG(std::string msg);

	//extern /*_declspec(dllexport)*/ void WARNING_MSG(std::string msg);

	//implementing these 2 op with functions is not acceptable because pre-processed
	//data will be wiped out.
	#define ERROR_MSG(msg)\
	{\
		std::ostringstream debugMsg;\
		debugMsg << "ERROR:" << std::endl;\
		debugMsg << msg << std::endl;\
		debugMsg << "file: " << __FILE__ << std::endl;\
		debugMsg << "line: " << __LINE__ << std::endl;\
		debugMsg << "function:" << __func__ << std::endl;\
		MessageBoxA(0, debugMsg.str().c_str(), 0, 0);\
		debugMsg.clear();\
	}\


	#define WARNING_MSG(msg)\
	{\
		std::ostringstream debugMsg;\
		debugMsg << "WARNING:" << std::endl;\
		debugMsg << msg << std::endl;\
		debugMsg << "file: " << __FILE__ << std::endl;\
		debugMsg << "line: " << __LINE__ << std::endl;\
		debugMsg << "function:" << __func__ << std::endl;\
		MessageBoxA(0, debugMsg.str().c_str(), 0, 0);\
		debugMsg.clear();\
	};\


}