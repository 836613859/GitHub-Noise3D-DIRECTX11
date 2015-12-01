
/***********************************************************************

							*ȫ��Ԫ�ض�����

				��������NoiseGlobal.h������ȫ��Ԫ���ڴ˶���   

***********************************************************************/

#include "Noise3D.h"


/*--------------------------ȫ�ֱ���----------------------------*/

//����������ؿ��
UINT					gMainBufferPixelWidth = 640;
//����������ظ߶�
UINT					gMainBufferPixelHeight = 480;

//�����Ʒ��
UINT				g_Device_MSAA4xQuality	= 1  ;
//������Ƿ���
BOOL				g_Device_MSAA4xEnabled	= FALSE;
//ֻ�ǳ�ʼ���趨����������
D3D_DRIVER_TYPE		g_Device_driverType	= D3D_DRIVER_TYPE_SOFTWARE;
//
D3D_FEATURE_LEVEL	g_Device_featureLevel = D3D_FEATURE_LEVEL_11_0;
//�������ֵ�MSG
std::ostringstream	g_Debug_MsgString;

//SemanticName,Index,Format,InputSlot,ByteOffset,InputSlotClass,InstanceDataStepRate
D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Simple[]=
{	
	{"POSITION",0,	DXGI_FORMAT_R32G32B32_FLOAT,		0,0,	D3D11_INPUT_PER_VERTEX_DATA,0},
	{  "COLOR",	0,	DXGI_FORMAT_R32G32B32A32_FLOAT,	0,12,	D3D11_INPUT_PER_VERTEX_DATA,0},
	{ "TEXCOORD",0,	DXGI_FORMAT_R32G32_FLOAT,		0,28,	D3D11_INPUT_PER_VERTEX_DATA,0 },
};

D3D11_INPUT_ELEMENT_DESC g_VertexDesc_Default[]=
{
	{"POSITION",0,	DXGI_FORMAT_R32G32B32_FLOAT,		0,0,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	{"COLOR",0,		DXGI_FORMAT_R32G32B32A32_FLOAT,	0,12,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	{"NORMAL",0,	DXGI_FORMAT_R32G32B32_FLOAT,		0,28,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	{"TEXCOORD",0,	DXGI_FORMAT_R32G32_FLOAT,		0,40,	D3D11_INPUT_PER_VERTEX_DATA,	0},
	{ "TANGENT",0,	DXGI_FORMAT_R32G32_FLOAT,			0,48,	D3D11_INPUT_PER_VERTEX_DATA,	0 },
};



/*-----------------------------ȫ�ֽӿ�-----------------------------*/

ID3D11Device*           g_pd3dDevice11			= NULL;

ID3D11DeviceContext*    g_pImmediateContext		= NULL;
//������
IDXGISwapChain*         g_pSwapChain			= NULL;
//��Ⱦ�ӿ� ���������� ���ں�pipeline��
ID3D11RenderTargetView* g_pRenderTargetView		= NULL;
//���&ģ�� ֻ����һ��
ID3D11DepthStencilView*	g_pDepthStencilView		= NULL;
//���㲼��
ID3D11InputLayout*      g_pVertexLayout_Default			= NULL;
ID3D11InputLayout*	     g_pVertexLayout_Simple			=	NULL;

ID3D11RasterizerState*		g_pRasterState_Solid_CullNone = NULL;
ID3D11RasterizerState*		g_pRasterState_WireFrame_CullNone= NULL;

/*------------------------------ȫ�ֺ���--------------------------*/

 HRESULT CompileShaderFromFile( WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{

	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile( szFileName, NULL, NULL, szEntryPoint, szShaderModel, 
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL );
	if( FAILED(hr) )
	{
		if( pErrorBlob != NULL )
			OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
		if( pErrorBlob ) pErrorBlob->Release();
		return hr;
	}
	if( pErrorBlob ) pErrorBlob->Release();

	return S_OK;
};

 BOOL gFunction_IsPointInRect2D(NVECTOR2 v, NVECTOR2 vTopLeft, NVECTOR2 vBottomRight)
 {
	 if (v.x >= vTopLeft.x &&
		 v.x <= vBottomRight.x &&
		 v.y >= vTopLeft.y &&
		 v.y <= vBottomRight.y)
	 {
		 return TRUE;
	 }

	 return FALSE;
 }

 int gFunction_GetCharAlignmentOffsetPixelY(UINT boundaryPxHeight, UINT charRealHeight, wchar_t inputChar)
 {
	 //!!!!!!the return type is signed INT ,because  top left might go beyond the upper boundary
	
	//Ascii alignment
	 switch (inputChar)
	 {
		 //1, upper alignment ,the top of each character lies right beneath the top
	 case '\"':	 case '\'':	 case '^':	 case '*':	 case '`':	case'j':
		 return 0;
		 break;
		
	//2,lower-bound alignment
	 case L'��':  case L'��' : case L'��': case L'��':  case L'��':  case L'��':  case L'��':
	 case L'��':  case L'��': case L'��':  case L'��':	case L'��':	case L'��':  case L'��':
	 case L'��':  case L'��': case L'��':  case L'��':	
		 return (int(boundaryPxHeight *0.75) -charRealHeight );
		 break;

		 //3,upper 1/3 alignment
	 case 'g':	case 'p':	case 'q': case 'y':
		 return boundaryPxHeight / 4;
		 break;

		 //4, lower 2/3 alignment, the bottom of each character lies right on the 2/3 line
	 default:
		 if(inputChar>0x007f)
		 {
			 return int((boundaryPxHeight *0.5) - charRealHeight*0.5);//CHINESE
		 }
		 else
		 {
			 return (int(boundaryPxHeight *0.75) - charRealHeight);
		 }
		 break;

	 }

	 return 0;
 }

 float gFunction_Lerp(float a, float b, float t)
 {
	 return( a + (b - a)*t);
 };


 float gFunction_Clamp(float val, float min, float max)
 {
	 return (val >= min ? (val <= max ? val : max) : min);
 };

