/************************************************************************

						CPP:  	Noise Renderer2D

					2D��Ⱦ��ȫ����fx��ʵ�֣�d2d���ü���

************************************************************************/
#pragma once
#include "Noise3D.h"

NoiseRenderer2D::NoiseRenderer2D()
{

};

NoiseRenderer2D::~NoiseRenderer2D()
{

};


BOOL	NoiseRenderer2D::mFunction_Init()
{
	HRESULT hr = S_OK;

	mFunction_Init_CreateEffectFromMemory("Main3D.fxo");

	//����Technique
	m_pFX_Tech_Default = m_pFX->GetTechniqueByName("DefaultDraw");
	m_pFX_Tech_DrawLine3D = m_pFX->GetTechniqueByName("DrawLine3D");

	//Ȼ��Ҫ����InputLayout
	//Ĭ�϶���
	D3DX11_PASS_DESC passDesc;
	m_pFX_Tech_Default->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = g_pd3dDevice->CreateInputLayout(
		&g_VertexDesc_Default[0],
		g_VertexDesc_Default_ElementNum,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pVertexLayout_Default);
	HR_DEBUG(hr, "����input Layoutʧ�ܣ�");

	//simple�����
	m_pFX_Tech_DrawLine3D->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = g_pd3dDevice->CreateInputLayout(
		&g_VertexDesc_Simple[0],
		g_VertexDesc_Simple_ElementNum,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pVertexLayout_Simple);
	HR_DEBUG(hr, "����input Layoutʧ�ܣ�");


	//����Cbuffer
	m_pFX_CbPerFrame = m_pFX->GetConstantBufferByName("cbPerFrame");
	m_pFX_CbPerObject = m_pFX->GetConstantBufferByName("cbPerObject");
	m_pFX_CbPerSubset = m_pFX->GetConstantBufferByName("cbPerSubset");
	m_pFX_CbRarely = m_pFX->GetConstantBufferByName("cbRarely");
	m_pFX_CbDrawLine3D = m_pFX->GetConstantBufferByName("cbDrawLine3D");

	//����
	m_pFX_Texture_Diffuse = m_pFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	m_pFX_Texture_Normal = m_pFX->GetVariableByName("gNormalMap")->AsShaderResource();
	m_pFX_Texture_Specular = m_pFX->GetVariableByName("gSpecularMap")->AsShaderResource();

#pragma region CreateRasterState
	//����Ԥ��Ĺ�դ��state
	//Create Raster State;If you want various Raster State,you should pre-Create all of them in the beginning
	D3D11_RASTERIZER_DESC tmpRasterStateDesc;//��դ������
	ZeroMemory(&tmpRasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	tmpRasterStateDesc.AntialiasedLineEnable = TRUE;//���������
	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_Solid_CullNone);
	HR_DEBUG(hr, "����m_pRasterState_Solid_CullNoneʧ��");

	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_WireFrame_CullNone);
	HR_DEBUG(hr, "����m_pRasterState_WireFrame_CullNoneʧ��");

	tmpRasterStateDesc.CullMode = D3D11_CULL_BACK;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_Solid_CullBack);
	HR_DEBUG(hr, "����m_pRasterState_Solid_CullBackʧ��");

	tmpRasterStateDesc.CullMode = D3D11_CULL_BACK;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_WireFrame_CullBack);
	HR_DEBUG(hr, "����m_pRasterState_WireFrame_CullBackʧ��");

	tmpRasterStateDesc.CullMode = D3D11_CULL_FRONT;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_Solid_CullFront);
	HR_DEBUG(hr, "����m_pRasterState_Solid_CullFrontʧ��");

	tmpRasterStateDesc.CullMode = D3D11_CULL_FRONT;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc, &m_pRasterState_WireFrame_CullFront);
	HR_DEBUG(hr, "����m_pRasterState_WireFrame_CullFrontʧ��");

#pragma endregion CreateRasterState

	return TRUE;
};

BOOL	NoiseRenderer2D::mFunction_Init_CreateEffectFromFile(LPCWSTR fxPath)
{
	HRESULT hr = S_OK;

	DWORD shaderFlags = 0;
#if defined(DEBUG)||defined(_DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	ID3D10Blob*	compiledFX;
	ID3D10Blob*	compilationMsg;

	//����fx�ļ�
	hr = D3DX11CompileFromFile(
		fxPath, 0, 0, 0, "fx_5_0",
		shaderFlags, 0, 0, &compiledFX,
		&compilationMsg, 0);

	//�����������޴�����Ϣ
	//To see if there is compiling error
	if (compilationMsg != 0)
	{
		assert(FALSE:"Shader Compilation Failed !!");
		ReleaseCOM(compilationMsg);
	}


	hr = D3DX11CreateEffectFromMemory(
		compiledFX->GetBufferPointer(),
		compiledFX->GetBufferSize(),
		0, g_pd3dDevice, &m_pFX);
	HR_DEBUG(hr, "Create Basic Effect Fail!");
	ReleaseCOM(compiledFX);

	return TRUE;
};

BOOL	NoiseRenderer2D::mFunction_Init_CreateEffectFromMemory(char* compiledShaderPath)
{
	std::vector<char> compiledShader;

	//����fxo�ļ�
	if (!NoiseFileManager::ImportFile_PURE(compiledShaderPath, &compiledShader))
	{
		return FALSE;
	}

	//����fx��Ч���
	HRESULT hr = S_OK;
	hr = D3DX11CreateEffectFromMemory(&compiledShader[0], compiledShader.size(), 0, g_pd3dDevice, &m_pFX);
	HR_DEBUG(hr, "load compiled shader failed");

	//����Technique
	m_pFX_Tech_Default = m_pFX->GetTechniqueByName("DefaultDraw");
	m_pFX_Tech_DrawLine3D = m_pFX->GetTechniqueByName("DrawLine3D");


	return TRUE;
};