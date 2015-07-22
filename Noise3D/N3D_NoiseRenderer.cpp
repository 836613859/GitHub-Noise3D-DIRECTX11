
/***********************************************************************

                           �ࣺNOISE Renderer

						������������Ⱦ

************************************************************************/

#include "Noise3D.h"

static UINT VBstride = sizeof(N_DefaultVertex);		//VertexBuffer��ÿ��Ԫ�ص��ֽڿ��
static UINT VBoffset = 0;				//VertexBuffer�������ƫ�� ��Ϊ��ͷ��ʼ����offset��0

static NoiseMesh*							tmp_pMesh;
static NoiseCamera*						tmp_pCamera;
static D3DX11_TECHNIQUE_DESC	tmp_pTechDesc;

NoiseRenderer::NoiseRenderer()
{
	m_pFatherScene				= NULL;
};

NoiseRenderer::~NoiseRenderer()
{
	ReleaseCOM(m_pFX);
};

void	NoiseRenderer::RenderMeshInList()
{
	UINT i=0;UINT j =0;
	 tmp_pCamera = m_pFatherScene->m_pChildCamera;

	//����ConstantBuffer:�޸Ĺ��͸���(cbRarely)
	 m_Function_RenderMeshInList_UpdateCbRarely();


	//����ConstantBuffer:ÿ֡����һ�� (cbPerFrame)
	 m_Function_RenderMeshInList_UpdateCbPerFrame();


#pragma region Render Mesh
	for(i = 0;	i<(m_pFatherScene->m_pRenderList_Mesh->size());	i++)
	{
		//ȡ����Ⱦ�б��е�meshָ��
		tmp_pMesh = m_pFatherScene->m_pRenderList_Mesh->at(i);


		//����ConstantBuffer:ÿ�������һ��(cbPerObject)
		m_Function_RenderMeshInList_UpdateCbPerObject();

		//����ConstantBuffer:ÿSubset,��һ��mesh�����в�ͬMat��Tex�Ķ���һ��subset
		m_Function_RenderMeshInList_UpdateCbPerSubset();

		//������cb�Ϳ��Կ�ʼdraw��
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout);
		g_pImmediateContext->IASetVertexBuffers(0,1,&tmp_pMesh->m_pVertexBuffer,&VBstride,&VBoffset);
		g_pImmediateContext->IASetIndexBuffer(tmp_pMesh->m_pIndexBuffer,DXGI_FORMAT_R32_UINT,0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_Function_SetRasterState(m_FillMode);

		//��������tech������pass
		m_pFX_Tech_Basic->GetDesc(&tmp_pTechDesc);
		for(j=0;j<tmp_pTechDesc.Passes;j++)
		{
			m_pFX_Tech_Basic->GetPassByIndex(j)->Apply(0,g_pImmediateContext);
			g_pImmediateContext->DrawIndexed(tmp_pMesh->m_IndexCount,0,0);
		}

}
#pragma endregion Render Mesh

	m_pFatherScene->m_pRenderList_Mesh->clear();
};

void	NoiseRenderer::ClearViews()
{
	float ClearColor[4] = { 0.0f, 0.3f, 0.3f, 1.0f };
	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	//�Ҳ�������������ô��ԭ����ҪclearDepth!!!!!!
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);
};

void	NoiseRenderer::RenderToScreen()
{
		g_pSwapChain->Present( 0, 0 );
};

void NoiseRenderer::SetFillMode(NOISE_FILLMODE iMode)
{
	m_FillMode = iMode;
};

/************************************************************************
                                            PRIVATE                        
************************************************************************/
BOOL	NoiseRenderer::m_Function_Init()
{
	HRESULT hr = S_OK;

	m_Function_Init_CreateEffectFromFile();

	//����Cbuffer
	m_pFX_CbPerFrame=m_pFX->GetConstantBufferByName("cbPerFrame");
	m_pFX_CbPerObject=m_pFX->GetConstantBufferByName("cbPerObject");
	m_pFX_CbPerSubset = m_pFX->GetConstantBufferByName("cbPerSubset");
	m_pFX_CbRarely=m_pFX->GetConstantBufferByName("cbRarely");

	#pragma region CreateRasterState

	//����Ԥ��Ĺ�դ��state
	//Create Raster State;If you want various Raster State,you should pre-Create all of them in the beginning

	D3D11_RASTERIZER_DESC tmpRasterStateDesc;//��դ������
	ZeroMemory(&tmpRasterStateDesc,sizeof(D3D11_RASTERIZER_DESC));
	tmpRasterStateDesc.AntialiasedLineEnable = TRUE;//���������
	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc,&g_pRasterState_FillMode_Solid);
	HR_DEBUG(hr,"����RASTERIZER STATE_solidʧ��");

	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc,&g_pRasterState_FillMode_WireFrame);
	HR_DEBUG(hr,"����RASTERIZER STATE_wireframeʧ��");

	#pragma endregion CreateRasterState

	return TRUE;
};

BOOL	NoiseRenderer::m_Function_Init_CreateEffectFromFile()
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
		L"Main.fx",0,0,0,"fx_5_0",
		shaderFlags,0,0,&compiledFX,
		&compilationMsg,0);

	//�����������޴�����Ϣ
	//To see if there is compiling error
	if (compilationMsg != 0)
	{
		DEBUG_MSG("Compile Basic Shader Fail!",0,0);
		ReleaseCOM(compilationMsg);
	}


	hr = D3DX11CreateEffectFromMemory(
		compiledFX->GetBufferPointer(),
		compiledFX->GetBufferSize(),
		0,g_pd3dDevice,&m_pFX);
	HR_DEBUG(hr,"Create Basic Effect Fail!");
	ReleaseCOM(compiledFX);

	//����Technique
	m_pFX_Tech_Basic =m_pFX->GetTechniqueByName("BasicTech");

	//Ȼ��Ҫ����InputLayout
	D3DX11_PASS_DESC passDesc;
	m_pFX_Tech_Basic->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = g_pd3dDevice->CreateInputLayout(
		&g_VertexDesc_Default[0],
		g_VertexDesc_ElementNum,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pVertexLayout);
	HR_DEBUG(hr,"����input Layoutʧ�ܣ�");


	return TRUE;
};


BOOL	NoiseRenderer::m_Function_Init_CreateEffectFromMemory()
{

	return TRUE;
};


void		NoiseRenderer::m_Function_SetRasterState(NOISE_FILLMODE iMode)
{
	switch(iMode)
	{
	case NOISE_FILLMODE_SOLID:
		g_pImmediateContext->RSSetState(g_pRasterState_FillMode_Solid);
		break;
	case NOISE_FILLMODE_WIREFRAME:
		g_pImmediateContext->RSSetState(g_pRasterState_FillMode_WireFrame);
		break;
	}
};

void		NoiseRenderer::m_Function_RenderMeshInList_UpdateCbRarely()
{
	
	BOOL tmpCanUpdateCbRarely = FALSE;
	//������������Proj Matrix��������
	if(tmp_pCamera->mCanUpdateProjMatrix	== TRUE)
	{
		tmp_pCamera->mFunction_UpdateProjMatrix();
		m_CbRarely.mProjMatrix = * (tmp_pCamera->m_pMatrixProjection);
		tmp_pCamera->mCanUpdateProjMatrix = FALSE;
		tmpCanUpdateCbRarely = TRUE;
	}

	//������������Static Light������������
	NoiseLightManager* tmpLightMgr = m_pFatherScene->m_pChildLightMgr;

	if((tmpLightMgr != NULL)&& (tmpLightMgr->mCanUpdateStaticLights))
	{
		int tmpLight_Dir_Count = tmpLightMgr->m_pLightList_Dir_Static->size();
		int tmpLight_Point_Count=tmpLightMgr->m_pLightList_Point_Static->size();
		int tmpLight_Spot_Count=tmpLightMgr->m_pLightList_Spot_Static->size();

		m_CbRarely.mIsLightingEnabled_Static			=		tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbRarely.mDirLightCount_Static				=		tmpLight_Dir_Count;
		m_CbRarely.mPointLightCount_Static			=		tmpLight_Point_Count;
		m_CbRarely.mSpotLightCount_Static				=		tmpLight_Spot_Count;

		int i =0;

		for(i=0; i<(tmpLight_Dir_Count); i++)
		{m_CbRarely.mDirectionalLight_Static[i]	=	(tmpLightMgr->m_pLightList_Dir_Static->at(i));}

		for(i=0; i<(tmpLight_Point_Count); i++)
		{m_CbRarely.mPointLight_Static[i]			=	(tmpLightMgr->m_pLightList_Point_Static->at(i));}

		for(i=0; i<(tmpLight_Spot_Count); i++)
		{m_CbRarely.mSpotLight_Static[i]			=	(tmpLightMgr->m_pLightList_Spot_Static->at(i));}

		//���� ���ɸ��¡�״̬
		tmpLightMgr->mCanUpdateStaticLights = FALSE;
	}


	//���������������µ�GPU������������
	if(tmpCanUpdateCbRarely == TRUE)
	{
		m_pFX_CbRarely->SetRawValue(&m_CbRarely,0,sizeof(m_CbRarely));
	};
};

void		NoiseRenderer::m_Function_RenderMeshInList_UpdateCbPerFrame()
{
	//������������View Matrix��������
	tmp_pCamera->mFunction_UpdateViewMatrix();
	m_CbPerFrame.mViewMatrix = *(tmp_pCamera->m_pMatrixView);


	//������������Dynamic�ƹ⡪������
	NoiseLightManager* tmpLightMgr = m_pFatherScene->m_pChildLightMgr;
	if(tmpLightMgr != NULL)
	{
		int tmpLight_Dir_Count = tmpLightMgr->m_pLightList_Dir_Dynamic->size();
		int tmpLight_Point_Count=tmpLightMgr->m_pLightList_Point_Dynamic->size();
		int tmpLight_Spot_Count=tmpLightMgr->m_pLightList_Spot_Dynamic->size();

		m_CbPerFrame.mIsLightingEnabled_Dynamic			=		tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbPerFrame.mDirLightCount_Dynamic					=		tmpLight_Dir_Count;
		m_CbPerFrame.mPointLightCount_Dynamic				=		tmpLight_Point_Count;
		m_CbPerFrame.mSpotLightCount_Dynamic				=		tmpLight_Spot_Count;
		m_CbPerFrame.mCamPos											=		*(tmp_pCamera->m_pPosition);

		int i =0;

		for(i=0; i<(tmpLight_Dir_Count); i++)
		{m_CbPerFrame.mDirectionalLight_Dynamic[i]	=	*(tmpLightMgr->m_pLightList_Dir_Dynamic->at(i));}

		for(i=0; i<(tmpLight_Point_Count); i++)
		{m_CbPerFrame.mPointLight_Dynamic[i]			=	*(tmpLightMgr->m_pLightList_Point_Dynamic->at(i));}

		for(i=0; i<(tmpLight_Spot_Count); i++)
		{m_CbPerFrame.mSpotLight_Dynamic[i]			=	*(tmpLightMgr->m_pLightList_Spot_Dynamic->at(i));}

	}

	//�����������µ�GPU������������

	m_pFX_CbPerFrame->SetRawValue(&m_CbPerFrame,0,sizeof(m_CbPerFrame));
};

void		NoiseRenderer::m_Function_RenderMeshInList_UpdateCbPerSubset()
{
	m_CbPerSubset.mMaterial = tmp_pMesh->m_pMaterialInMem->at(0);
	m_pFX_CbPerSubset->SetRawValue(&m_CbPerSubset,0,sizeof(m_CbPerSubset));
};

void		NoiseRenderer::m_Function_RenderMeshInList_UpdateCbPerObject()
{
	//������������World Matrix��������
	tmp_pMesh->mFunction_UpdateWorldMatrix();
	m_CbPerObject.mWorldMatrix =	*(tmp_pMesh->m_pMatrixWorld);
	m_CbPerObject.mWorldInvTransposeMatrix = *(tmp_pMesh->m_pMatrixWorldInvTranspose);

	//���������������µ�GPU������������
	m_pFX_CbPerObject->SetRawValue(&m_CbPerObject,0,sizeof(m_CbPerObject));
};
