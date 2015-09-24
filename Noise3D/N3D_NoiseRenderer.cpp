
/***********************************************************************

                           �ࣺNOISE Renderer

						������������Ⱦ

************************************************************************/

#include "Noise3D.h"

static UINT VBstride_Default = sizeof(N_DefaultVertex);		//VertexBuffer��ÿ��Ԫ�ص��ֽڿ��
static UINT VBstride_Simple = sizeof(N_SimpleVertex);
static UINT VBoffset = 0;				//VertexBuffer�������ƫ�� ��Ϊ��ͷ��ʼ����offset��0

static NoiseMesh*							tmp_pMesh;
static NoiseCamera*						tmp_pCamera;
static NoiseAtmosphere*				tmp_pAtmo;
static D3DX11_TECHNIQUE_DESC	tmp_pTechDesc;


NoiseRenderer::NoiseRenderer()
{
	m_pFatherScene			= nullptr;
	mCanUpdateCbCameraMatrix = FALSE;
	m_pRenderList_Mesh				= new std::vector <NoiseMesh*>;
	m_pRenderList_GraphicObject	= new std::vector<NoiseGraphicObject*>;
	m_pRenderList_Atmosphere		= new std::vector<NoiseAtmosphere*>;
	m_pFX = nullptr;
	m_pFX_Tech_Default = nullptr;
	m_pFX_Tech_Solid3D = nullptr;
	m_pFX_Tech_Solid2D = nullptr;
	m_pFX_Tech_Textured2D = nullptr;
	m_pFX_Tech_DrawSky = nullptr;
	m_FillMode = NOISE_FILLMODE_SOLID;
	m_CullMode = NOISE_CULLMODE_NONE;
	m_BlendMode = NOISE_BLENDMODE_OPAQUE;
};

void NoiseRenderer::SelfDestruction()
{
	ReleaseCOM(m_pFX);
	ReleaseCOM(m_pRasterState_Solid_CullNone);
	ReleaseCOM(m_pRasterState_Solid_CullBack);
	ReleaseCOM(m_pRasterState_Solid_CullFront);
	ReleaseCOM(m_pRasterState_WireFrame_CullFront);
	ReleaseCOM(m_pRasterState_WireFrame_CullNone);
	ReleaseCOM(m_pRasterState_WireFrame_CullBack);
};

void	NoiseRenderer::RenderMeshInList()
{
	//validation before rendering
	if (m_pFatherScene->m_pChildMaterialMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Material Mgr has not been created");
		return;
	};
	if (m_pFatherScene->m_pChildTextureMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Texture Mgr has not been created");
		return;
	};


	UINT i = 0, j = 0, k = 0;
	 tmp_pCamera = m_pFatherScene->GetCamera();

	//����ConstantBuffer:�޸Ĺ��͸���(cbRarely)
	 mFunction_RenderMeshInList_UpdateCbRarely();


	//����ConstantBuffer:ÿ֡����һ�� (cbPerFrame)
	 mFunction_RenderMeshInList_UpdateCbPerFrame();

	 //����ConstantBuffer : Proj / View Matrix
	 mFunction_CameraMatrix_Update();


#pragma region Render Mesh
	 //for every mesh
	for(i = 0;	i<(m_pRenderList_Mesh->size());	i++)
	{
		//ȡ����Ⱦ�б��е�meshָ��
		tmp_pMesh = m_pRenderList_Mesh->at(i);

		//����ConstantBuffer:ÿ�������һ��(cbPerObject)
		mFunction_RenderMeshInList_UpdateCbPerObject();

		//������cb��׼����ʼdraw��
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Default);
		g_pImmediateContext->IASetVertexBuffers(0,1,&tmp_pMesh->m_pVertexBuffer,&VBstride_Default,&VBoffset);
		g_pImmediateContext->IASetIndexBuffer(tmp_pMesh->m_pIndexBuffer,DXGI_FORMAT_R32_UINT,0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(m_FillMode,m_CullMode);

		//����blend state
		mFunction_SetBlendState(m_BlendMode);

		//for every subset
		UINT meshSubsetCount = tmp_pMesh->m_pSubsetInfoList->size();
		for (j = 0;j < meshSubsetCount;j++)
		{
			//����ConstantBuffer:ÿSubset,��һ��mesh�����в�ͬMaterial�Ķ���һ��subset
			mFunction_RenderMeshInList_UpdateCbPerSubset(j);

			//��������tech������pass ---- index starts from 1
			m_pFX_Tech_Default->GetDesc(&tmp_pTechDesc);
			for (k = 0;k < tmp_pTechDesc.Passes; k++)
			{
				m_pFX_Tech_Default->GetPassByIndex(k)->Apply(0, g_pImmediateContext);
				g_pImmediateContext->DrawIndexed(tmp_pMesh->m_IndexCount, 0, 0);
			}
		}
}
#pragma endregion Render Mesh

}

void NoiseRenderer::RenderGraphicObjectInList()
{
	//validation before rendering
	if (m_pFatherScene->m_pChildMaterialMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Material Mgr has not been created");
		return;
	};
	if (m_pFatherScene->m_pChildTextureMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Texture Mgr has not been created");
		return;
	};

	//����blend state
	mFunction_SetBlendState(m_BlendMode);

	mFunction_GraphicObj_RenderLine2DInList();
	mFunction_GraphicObj_RenderLine3DInList();
	mFunction_GraphicObj_RenderPoint3DInList();
	mFunction_GraphicObj_RenderPoint2DInList();
	mFunction_GraphicObj_RenderTriangle2DInList();
}

void NoiseRenderer::RenderAtmosphereInList()
{
	//validation before rendering
	if (m_pFatherScene->m_pChildMaterialMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Material Mgr has not been created");
		return;
	};
	if (m_pFatherScene->m_pChildTextureMgr == nullptr)
	{
		DEBUG_MSG1("Noise Renderer : Texture Mgr has not been created");
		return;
	};

	//...................
	UINT i = 0;
	tmp_pCamera = m_pFatherScene->GetCamera();


	//update view/proj matrix
	mFunction_CameraMatrix_Update();


	//actually there is only 1 atmosphere because you dont need more 
	for (i = 0;i < m_pRenderList_Atmosphere->size();i++)
	{
		tmp_pAtmo = m_pRenderList_Atmosphere->at(i);

		//enable/disable fog effect 
		mFunction_Atmosphere_Fog_Update();



	#pragma region Draw Sky

		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pAtmo->m_pVB_Gpu_Sky, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(tmp_pAtmo->m_pIB_Gpu_Sky, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		mFunction_SetRasterState(m_FillMode, m_CullMode);
		mFunction_SetBlendState(m_BlendMode);

		//update Vertices or atmo param to GPU
		//shaders will decide to draw skybox or sky dome
		//( there are SkyboxValid & SkyDomeValid BOOL)
		mFunction_Atmosphere_SkyDome_Update();
		mFunction_Atmosphere_SkyBox_Update();
		mFunction_Atmosphere_UpdateCbAtmosphere();


		//traverse passes in one technique ---- pass index starts from 1
		m_pFX_Tech_DrawSky->GetDesc(&tmp_pTechDesc);
		for (UINT k = 0;k < tmp_pTechDesc.Passes; k++)
		{
			m_pFX_Tech_DrawSky->GetPassByIndex(k)->Apply(0, g_pImmediateContext);
			g_pImmediateContext->DrawIndexed(tmp_pAtmo->m_pIB_Mem_Sky->size(), 0, 0);
		}


	#pragma endregion Draw Sky
	}

	//allow atmosphere to "add to render list" again 
	tmp_pAtmo->mFogHasBeenAddedToRenderList = FALSE;
};

void	NoiseRenderer::ClearBackground(NVECTOR4 color)
{
	float ClearColor[4] = { color.x,color.y,color.z,color.w };
	g_pImmediateContext->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
	//�Ҳ�������������ô��ԭ����ҪclearDepth!!!!!!
	g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView,
		D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL,1.0f,0);
};

void	NoiseRenderer::RenderToScreen()
{
		g_pSwapChain->Present( 0, 0 );

		//reset some state
		mCanUpdateCbCameraMatrix = TRUE;

		//clear render list
		m_pRenderList_GraphicObject->clear();
		m_pRenderList_Mesh->clear();
		m_pRenderList_Atmosphere->clear();
};

void NoiseRenderer::SetFillMode(NOISE_FILLMODE iMode)
{
	m_FillMode = iMode;
}

void NoiseRenderer::SetCullMode(NOISE_CULLMODE iMode)
{
	m_CullMode = iMode;
}

void NoiseRenderer::SetBlendingMode(NOISE_BLENDMODE iMode)
{
	m_BlendMode = iMode;
};



/************************************************************************
                                            PRIVATE                        
************************************************************************/
BOOL	NoiseRenderer::mFunction_Init()
{
	HRESULT hr = S_OK;

	mFunction_Init_CreateEffectFromMemory("Main.fxo");

	//����Technique
	m_pFX_Tech_Default = m_pFX->GetTechniqueByName("DefaultDraw");
	m_pFX_Tech_Solid3D = m_pFX->GetTechniqueByName("DrawSolid3D");
	m_pFX_Tech_Solid2D = m_pFX->GetTechniqueByName("DrawSolid2D");
	m_pFX_Tech_Textured2D = m_pFX->GetTechniqueByName("DrawTextured2D");
	m_pFX_Tech_DrawSky = m_pFX->GetTechniqueByName("DrawSky");

#pragma region Create Input Layout
	//default vertex input layout
	D3DX11_PASS_DESC passDesc;
	m_pFX_Tech_Default->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = g_pd3dDevice->CreateInputLayout(
		&g_VertexDesc_Default[0],
		g_VertexDesc_Default_ElementNum,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pVertexLayout_Default);
	HR_DEBUG(hr, "����input Layoutʧ�ܣ�");

	//simple vertex input layout
	m_pFX_Tech_Solid3D->GetPassByIndex(0)->GetDesc(&passDesc);
	hr = g_pd3dDevice->CreateInputLayout(
		&g_VertexDesc_Simple[0],
		g_VertexDesc_Simple_ElementNum,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&g_pVertexLayout_Simple);
	HR_DEBUG(hr, "����input Layoutʧ�ܣ�");
#pragma endregion Create Input Layout


#pragma region Create Fx Variable
	//����Cbuffer
	m_pFX_CbPerFrame=m_pFX->GetConstantBufferByName("cbPerFrame");
	m_pFX_CbPerObject=m_pFX->GetConstantBufferByName("cbPerObject");
	m_pFX_CbPerSubset = m_pFX->GetConstantBufferByName("cbPerSubset");
	m_pFX_CbRarely=m_pFX->GetConstantBufferByName("cbRarely");
	m_pFX_CbSolid3D = m_pFX->GetConstantBufferByName("cbCameraMatrix");
	m_pFX_CbAtmosphere = m_pFX->GetConstantBufferByName("cbAtmosphere");

	//����
	m_pFX_Texture_Diffuse = m_pFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	m_pFX_Texture_Normal = m_pFX->GetVariableByName("gNormalMap")->AsShaderResource();
	m_pFX_Texture_Specular = m_pFX->GetVariableByName("gSpecularMap")->AsShaderResource();
	m_pFX_Texture_CubeMap = m_pFX->GetVariableByName("gCubeMap")->AsShaderResource();
	m_pFX2D_Texture_Diffuse = m_pFX->GetVariableByName("g2D_DiffuseMap")->AsShaderResource();

#pragma endregion Create Fx Variable


#pragma region CreateRasterState
	//����Ԥ��Ĺ�դ��state
	//Create Raster State;If you want various Raster State,you should pre-Create all of them in the beginning
	D3D11_RASTERIZER_DESC tmpRasterStateDesc;//��դ������
	ZeroMemory(&tmpRasterStateDesc,sizeof(D3D11_RASTERIZER_DESC));
	tmpRasterStateDesc.AntialiasedLineEnable = TRUE;//���������
	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_SOLID;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc,&m_pRasterState_Solid_CullNone);
	HR_DEBUG(hr,"����m_pRasterState_Solid_CullNoneʧ��");
	
	tmpRasterStateDesc.CullMode = D3D11_CULL_NONE;//�޳�ģʽ
	tmpRasterStateDesc.FillMode = D3D11_FILL_WIREFRAME;
	hr = g_pd3dDevice->CreateRasterizerState(&tmpRasterStateDesc,&m_pRasterState_WireFrame_CullNone);
	HR_DEBUG(hr,"����m_pRasterState_WireFrame_CullNoneʧ��");

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

//source color : the first color in blending equation
#pragma region CreateBlendState

	D3D11_BLEND_DESC tmpBlendDesc;
	tmpBlendDesc.AlphaToCoverageEnable = FALSE; // ???related to multi-sampling
	tmpBlendDesc.IndependentBlendEnable = FALSE; //determine if 8 simultaneous render targets are rendered with same blend state
	tmpBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	tmpBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	tmpBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	tmpBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&tmpBlendDesc, &m_pBlendState_Opaque);
	HR_DEBUG(hr, "Create blend state(opaque) failed!!");


	tmpBlendDesc.AlphaToCoverageEnable = FALSE; // ???related to multi-sampling
	tmpBlendDesc.IndependentBlendEnable = FALSE; //determine if 8 simultaneous render targets are rendered with same blend state
	tmpBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	tmpBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&tmpBlendDesc, &m_pBlendState_ColorAdd);
	HR_DEBUG(hr, "Create blend state(Color Add) failed!!");


	tmpBlendDesc.AlphaToCoverageEnable = FALSE; // ???related to multi-sampling
	tmpBlendDesc.IndependentBlendEnable = FALSE; //determine if 8 simultaneous render targets are rendered with same blend state
	tmpBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	tmpBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
	tmpBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	tmpBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_DEST_ALPHA;
	tmpBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	tmpBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&tmpBlendDesc, &m_pBlendState_ColorMultiply);
	HR_DEBUG(hr, "Create blend state(Color Filter) failed!!");

	tmpBlendDesc.AlphaToCoverageEnable = FALSE; // ???related to multi-sampling
	tmpBlendDesc.IndependentBlendEnable = FALSE; //determine if 8 simultaneous render targets are rendered with same blend state
	tmpBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	tmpBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//what about D3D11_BLEND_SRC1_COLOR
	tmpBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	tmpBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	tmpBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	tmpBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	tmpBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = g_pd3dDevice->CreateBlendState(&tmpBlendDesc, &m_pBlendState_AlphaTransparency);
	HR_DEBUG(hr, "Create blend state(Transparency) failed!!");


#pragma endregion CreateBlendState

	return TRUE;
};

BOOL	NoiseRenderer::mFunction_Init_CreateEffectFromFile(LPCWSTR fxPath)
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
		fxPath,0,0,0,"fx_5_0",
		shaderFlags,0,0,&compiledFX,
		&compilationMsg,0);

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
		0,g_pd3dDevice,&m_pFX);
	HR_DEBUG(hr,"Create Basic Effect Fail!");
	ReleaseCOM(compiledFX);

	return TRUE;
};

BOOL	NoiseRenderer::mFunction_Init_CreateEffectFromMemory(char* compiledShaderPath)
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
	HR_DEBUG(hr,"load compiled shader failed");

	return TRUE;
};

void		NoiseRenderer::mFunction_SetRasterState(NOISE_FILLMODE iFillMode, NOISE_CULLMODE iCullMode)
{
	//fillMode , or "which primitive to draw, point?line?triangle?" , affect the decision of topology
	//like if user chooses WIREFRAME mode , the topology will be LINELIST

	switch(iFillMode)
	{
	//Solid Mode
	case NOISE_FILLMODE_SOLID:
		switch (iCullMode)
		{
		case NOISE_CULLMODE_NONE:
			g_pImmediateContext->RSSetState(m_pRasterState_Solid_CullNone);
			break;

		case NOISE_CULLMODE_FRONT:
			g_pImmediateContext->RSSetState(m_pRasterState_Solid_CullFront);
			break;

		case NOISE_CULLMODE_BACK:
			g_pImmediateContext->RSSetState(m_pRasterState_Solid_CullBack);
			break;

		default:
			break;
		}
		break;
	

	//WireFrame
	case NOISE_FILLMODE_WIREFRAME:
		switch (iCullMode)
		{
		case NOISE_CULLMODE_NONE:
			g_pImmediateContext->RSSetState(m_pRasterState_WireFrame_CullNone);
			break;

		case NOISE_CULLMODE_FRONT:
			g_pImmediateContext->RSSetState(m_pRasterState_WireFrame_CullFront);
			break;

		case NOISE_CULLMODE_BACK:
			g_pImmediateContext->RSSetState(m_pRasterState_WireFrame_CullBack);
			break;

		default:
			break;
		}
		break;



	//render points
	case 	NOISE_FILLMODE_POINT:
		g_pImmediateContext->RSSetState(m_pRasterState_WireFrame_CullNone);
		//g_pImmediateContext->RSSetState(m_pRasterState_Solid_CullNone);
		break;

	default:
		break;
	}
}

void		NoiseRenderer::mFunction_SetBlendState(NOISE_BLENDMODE iBlendMode)
{
	float tmpBlendFactor[4] = { 0,0,0,0 };
	switch (m_BlendMode)
	{
	case NOISE_BLENDMODE_OPAQUE:
		g_pImmediateContext->OMSetBlendState(m_pBlendState_Opaque, tmpBlendFactor, 0xffffffff);
		break;

	case NOISE_BLENDMODE_ADDITIVE:
		g_pImmediateContext->OMSetBlendState(m_pBlendState_ColorAdd, tmpBlendFactor, 0xffffffff);
		break;

	case NOISE_BLENDMODE_ALPHA:
		g_pImmediateContext->OMSetBlendState(m_pBlendState_AlphaTransparency, tmpBlendFactor, 0xffffffff);
		break;

	case NOISE_BLENDMODE_COLORFILTER:
		g_pImmediateContext->OMSetBlendState(m_pBlendState_ColorMultiply, tmpBlendFactor, 0xffffffff);
		break;

	}
}

void		NoiseRenderer::mFunction_CameraMatrix_Update()
{
	if (mCanUpdateCbCameraMatrix)
	{
		//update proj matrix
		tmp_pCamera->mFunction_UpdateProjMatrix();
		m_CbCameraMatrix.mProjMatrix = *(tmp_pCamera->m_pMatrixProjection);

		// update view matrix
		tmp_pCamera->mFunction_UpdateViewMatrix();
		m_CbCameraMatrix.mViewMatrix = *(tmp_pCamera->m_pMatrixView);

		//���������������µ�GPU������������
		m_pFX_CbSolid3D->SetRawValue(&m_CbCameraMatrix, 0, sizeof(m_CbCameraMatrix));

		//..........
		mCanUpdateCbCameraMatrix = FALSE;
	}
};

void		NoiseRenderer::mFunction_RenderMeshInList_UpdateCbRarely()
{
	
	BOOL tmpCanUpdateCbRarely = FALSE;

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

		//���� ���ɸ��¡�״̬����֤static light ֻ���г�ʼ��
		tmpLightMgr->mCanUpdateStaticLights = FALSE;
	}


	//���������������µ�GPU������������
	if(tmpCanUpdateCbRarely == TRUE)
	{
		m_pFX_CbRarely->SetRawValue(&m_CbRarely,0,sizeof(m_CbRarely));
	};
};

void		NoiseRenderer::mFunction_RenderMeshInList_UpdateCbPerFrame()
{



	//������������Dynamic Light��������
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

void		NoiseRenderer::mFunction_RenderMeshInList_UpdateCbPerSubset(UINT subsetID)
{
		//we dont accept invalid material ,but accept invalid texture
		UINT	 currSubsetMatID = tmp_pMesh->m_pSubsetInfoList->at(subsetID).matID;
		currSubsetMatID = mFunction_ValidateMaterialID(currSubsetMatID);

		//otherwise if the material is valid
		//then we should check if its child textureS are valid too 
		N_Material tmpMat = m_pFatherScene->m_pChildMaterialMgr->m_pMaterialList->at(currSubsetMatID);
		ID3D11ShaderResourceView* tmp_pSRV = nullptr;
		m_CbPerSubset.basicMaterial = tmpMat.baseColor;

		//first validate if ID is valid (within range / valid ID) valid== return original texID
		m_CbPerSubset.IsDiffuseMapValid = (mFunction_ValidateTextureID(tmpMat.diffuseMapID,NOISE_TEXTURE_TYPE_COMMON) 
			== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
		m_CbPerSubset.IsNormalMapValid = (mFunction_ValidateTextureID(tmpMat.normalMapID, NOISE_TEXTURE_TYPE_COMMON) 
			== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
		m_CbPerSubset.IsSpecularMapValid	= (mFunction_ValidateTextureID(tmpMat.specularMapID, NOISE_TEXTURE_TYPE_COMMON) 
			== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);


		//update textures, bound corresponding ShaderResourceView to the pipeline
		//if tetxure is  valid ,then set diffuse map
		if (m_CbPerSubset.IsDiffuseMapValid)
		{
			tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(tmpMat.diffuseMapID).m_pSRV;
			m_pFX_Texture_Diffuse->SetResource(tmp_pSRV);
		}

		//if tetxure is  valid ,then set normal map
		if (m_CbPerSubset.IsNormalMapValid)
		{
			tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(tmpMat.normalMapID).m_pSRV;
			m_pFX_Texture_Normal->SetResource(tmp_pSRV);
		}

		//if tetxure is  valid ,then set specular map
		if (m_CbPerSubset.IsSpecularMapValid)
		{
			tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(tmpMat.specularMapID).m_pSRV;
			m_pFX_Texture_Specular->SetResource(tmp_pSRV);
		}

		//transmit all data to gpu
		m_pFX_CbPerSubset->SetRawValue(&m_CbPerSubset, 0, sizeof(m_CbPerSubset));
	
};

void		NoiseRenderer::mFunction_RenderMeshInList_UpdateCbPerObject()
{
	//������������World Matrix��������
	tmp_pMesh->mFunction_UpdateWorldMatrix();
	m_CbPerObject.mWorldMatrix =	*(tmp_pMesh->m_pMatrixWorld);
	m_CbPerObject.mWorldInvTransposeMatrix = *(tmp_pMesh->m_pMatrixWorldInvTranspose);

	//���������������µ�GPU������������
	m_pFX_CbPerObject->SetRawValue(&m_CbPerObject,0,sizeof(m_CbPerObject));
};

void		NoiseRenderer::mFunction_GraphicObj_Update_RenderTextured2D(UINT TexID)
{
	//Get Shader Resource View
	ID3D11ShaderResourceView* tmp_pSRV = NULL;

	//......
	TexID = mFunction_ValidateTextureID(TexID,NOISE_TEXTURE_TYPE_COMMON);

	if (TexID != NOISE_MACRO_INVALID_TEXTURE_ID)
	{
		tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(TexID).m_pSRV;
		m_pFX2D_Texture_Diffuse->SetResource(tmp_pSRV);
	}
};

void		NoiseRenderer::mFunction_GraphicObj_RenderLine3DInList()
{
	tmp_pCamera = m_pFatherScene->GetCamera();

	//����ConstantBuffer : Proj / View Matrix
	mFunction_CameraMatrix_Update();

	//������cb�Ϳ��Կ�ʼdraw��
	ID3D11Buffer* tmp_pVB = NULL;
	for (UINT i = 0;i < m_pRenderList_GraphicObject->size();i++)
	{
		//��RenderList������GraphicObject��line3D����Ⱦһ��
		tmp_pVB = m_pRenderList_GraphicObject->at(i)->m_pVB_GPU[NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D];
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(NOISE_FILLMODE_WIREFRAME, NOISE_CULLMODE_NONE);

		//draw line һ��pass�͹���
		m_pFX_Tech_Solid3D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
		UINT vCount = m_pRenderList_GraphicObject->at(i)->m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D]->size();
		g_pImmediateContext->Draw(vCount, 0);
	}


}

void		NoiseRenderer::mFunction_GraphicObj_RenderPoint3DInList()
{
	tmp_pCamera = m_pFatherScene->GetCamera();

	//update view/proj matrix
	mFunction_CameraMatrix_Update();

	//������cb�Ϳ��Կ�ʼdraw��
	ID3D11Buffer* tmp_pVB = NULL;
	for (UINT i = 0;i < m_pRenderList_GraphicObject->size();i++)
	{
		//��RenderList������GraphicObject��line3D����Ⱦһ��
		tmp_pVB = m_pRenderList_GraphicObject->at(i)->m_pVB_GPU[NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D];
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(NOISE_FILLMODE_POINT, NOISE_CULLMODE_NONE);

		//draw line һ��pass�͹���
		m_pFX_Tech_Solid3D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
		UINT vCount = m_pRenderList_GraphicObject->at(i)->m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D]->size();
		g_pImmediateContext->Draw(vCount, 0);
	}

}

void		NoiseRenderer::mFunction_GraphicObj_RenderLine2DInList()
{

	//prepare to draw , various settings.....
	ID3D11Buffer* tmp_pVB = NULL;
	for (UINT i = 0;i < m_pRenderList_GraphicObject->size();i++)
	{
		//��RenderList������GraphicObject��line3D����Ⱦһ��
		tmp_pVB = m_pRenderList_GraphicObject->at(i)->m_pVB_GPU[NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D];
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(NOISE_FILLMODE_WIREFRAME, NOISE_CULLMODE_NONE);

		//draw line һ��pass�͹���
		m_pFX_Tech_Solid2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
		UINT vCount = m_pRenderList_GraphicObject->at(i)->m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D]->size();
		g_pImmediateContext->Draw(vCount, 0);
	}

};

void		NoiseRenderer::mFunction_GraphicObj_RenderPoint2DInList()
{
	//prepare to draw , various settings.....
	ID3D11Buffer* tmp_pVB = NULL;
	for (UINT i = 0;i < m_pRenderList_GraphicObject->size();i++)
	{
		//��RenderList������GraphicObject��line3D����Ⱦһ��
		tmp_pVB = m_pRenderList_GraphicObject->at(i)->m_pVB_GPU[NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D];
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(NOISE_FILLMODE_POINT, NOISE_CULLMODE_NONE);

		//draw line һ��pass�͹���
		m_pFX_Tech_Solid2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
		UINT vCount = m_pRenderList_GraphicObject->at(i)->m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D]->size();
		g_pImmediateContext->Draw(vCount, 0);
	}

};

void		NoiseRenderer::mFunction_GraphicObj_RenderTriangle2DInList()
{
	//prepare to draw , various settings.....
	ID3D11Buffer* tmp_pVB = NULL;


	for (UINT i = 0;i < m_pRenderList_GraphicObject->size();i++)
	{
		//��RenderList������GraphicObject��line3D����Ⱦһ��
		tmp_pVB = m_pRenderList_GraphicObject->at(i)->m_pVB_GPU[NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D];
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(NOISE_FILLMODE_SOLID, NOISE_CULLMODE_NONE);


		UINT j = 0, vCount = 0;
		//traverse all region list , to decide use which tech to draw (textured or not)


		//1,draw common triangle
		for (auto tmpTriangleID : *(m_pRenderList_GraphicObject->at(i)->m_pRegionList_TriangleID_Common))
		{
			m_pFX_Tech_Solid2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			m_pFX_Tech_Textured2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			g_pImmediateContext->Draw(3, tmpTriangleID * 3);
		}


		//2,draw rectangles
		for (auto tmpRegion : *(m_pRenderList_GraphicObject->at(i)->m_pRegionList_TriangleID_Rect))
		{
			//if current Rectangle disable Texture ,then draw in a solid way
			if (tmpRegion.texID == NOISE_MACRO_INVALID_TEXTURE_ID)
			{
				m_pFX_Tech_Solid2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			}
			else
			{
				//update texture to GPU first
				mFunction_GraphicObj_Update_RenderTextured2D(tmpRegion.texID);
				m_pFX_Tech_Textured2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			}

			//draw 
			g_pImmediateContext->Draw(6, tmpRegion.startID * 3);
		}


		//3, Draw Ellipse
		for (auto tmpRegion : *(m_pRenderList_GraphicObject->at(i)->m_pRegionList_TriangleID_Ellipse))
		{
			//if current Rectangle disable Texture ,then draw in a solid way
			if (tmpRegion.texID == NOISE_MACRO_INVALID_TEXTURE_ID)
			{
				m_pFX_Tech_Solid2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			}
			else
			{
				//update texture to GPU first
				mFunction_GraphicObj_Update_RenderTextured2D(tmpRegion.texID);
				m_pFX_Tech_Textured2D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
			}

			//draw ( but first translate triangle ID into vertex ID)
			g_pImmediateContext->Draw(tmpRegion.elememtCount*3, tmpRegion.startID * 3);
		}

	}

	//�����Ⱦ�б�
	m_pRenderList_GraphicObject->clear();
}

void		NoiseRenderer::mFunction_Atmosphere_Fog_Update()
{
	if (tmp_pAtmo->mFogCanUpdateToGpu)
	{
		//update fog param
		m_CbAtmosphere.mFogColor = *(tmp_pAtmo->m_pFogColor);
		m_CbAtmosphere.mFogFar = tmp_pAtmo->mFogFar;
		m_CbAtmosphere.mFogNear = tmp_pAtmo->mFogNear;
		m_CbAtmosphere.mIsFogEnabled = (BOOL)(tmp_pAtmo->mFogEnabled && tmp_pAtmo->mFogHasBeenAddedToRenderList);

		//udpate to GPU
		m_pFX_CbAtmosphere->SetRawValue(&m_CbAtmosphere, 0, sizeof(m_CbAtmosphere));
		tmp_pAtmo->mFogCanUpdateToGpu = FALSE;
	}
};

void		NoiseRenderer::mFunction_Atmosphere_SkyDome_Update()
{
	//validate texture and update BOOL value to gpu
	UINT skyDomeTexID = tmp_pAtmo->mSkyDomeTextureID;

	//check skyType
	if(tmp_pAtmo->mSkyType == NOISE_ATMOSPHERE_SKYTYPE_DOME)
	{
		//if texture pass ID validation and match current-set skytype
		m_CbAtmosphere.mIsSkyDomeValid = (mFunction_ValidateTextureID(skyDomeTexID,NOISE_TEXTURE_TYPE_COMMON) == NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
	}
	else
	{
		m_CbAtmosphere.mIsSkyDomeValid = FALSE;
	}

};

void		NoiseRenderer::mFunction_Atmosphere_SkyBox_Update()
{
	//skybox uses cube map to texture the box
	UINT skyboxTexID = tmp_pAtmo->mSkyBoxCubeTextureID;

	//check skyType
	if (tmp_pAtmo->mSkyType == NOISE_ATMOSPHERE_SKYTYPE_BOX)
	{
		//skybox texture must be a cube map
		m_CbAtmosphere.mIsSkyBoxValid		= (mFunction_ValidateTextureID(skyboxTexID, NOISE_TEXTURE_TYPE_CUBEMAP) == NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
		m_CbAtmosphere.mSkyBoxWidth		= tmp_pAtmo->mSkyBoxWidth;
		m_CbAtmosphere.mSkyBoxHeight	= tmp_pAtmo->mSkyBoxHeight;
		m_CbAtmosphere.mSkyBoxDepth		= tmp_pAtmo->mSkyBoxDepth;
		if (!m_CbAtmosphere.mIsSkyBoxValid)
		{
			DEBUG_MSG1("Noise Atmosphere Skybox : Texture Invalid!");
		}
	}
	else
	{
		m_CbAtmosphere.mIsSkyBoxValid = FALSE;
	}

};

void		NoiseRenderer::mFunction_Atmosphere_UpdateCbAtmosphere()
{
	//update valid texture to gpu
	if (m_CbAtmosphere.mIsSkyDomeValid)
	{
		//tmp_pAtmo->mSkyDomeTextureID has been validated in UPDATE function
		auto tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(tmp_pAtmo->mSkyDomeTextureID).m_pSRV;
		m_pFX_Texture_Diffuse->SetResource(tmp_pSRV);
	}


	//update skybox cube map to gpu
	if (m_CbAtmosphere.mIsSkyBoxValid)
	{
		//tmp_pAtmo->mSkyBoxTextureID has been validated  in UPDATE function
		//but how do you validate it's a valid cube map ?????
		auto tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(tmp_pAtmo->mSkyBoxCubeTextureID).m_pSRV;
		m_pFX_Texture_CubeMap->SetResource(tmp_pSRV);
	}


	m_pFX_CbAtmosphere->SetRawValue(&m_CbAtmosphere, 0, sizeof(m_CbAtmosphere));
};

UINT		NoiseRenderer::mFunction_ValidateTextureID(UINT texID, NOISE_TEXTURE_TYPE texType)
{
	//tex mgr had been validated
	NoiseTextureManager*		tmpTexMgr = m_pFatherScene->m_pChildTextureMgr;

	//invoke validation function in Tex Mgr
	UINT outTexID = tmpTexMgr->mFunction_ValidateTextureID(texID, texType);

	return outTexID;
}

UINT		NoiseRenderer::mFunction_ValidateMaterialID(UINT matID)
{
	//mat mgr had been validated
	NoiseMaterialManager*	tmpMatMgr = m_pFatherScene->m_pChildMaterialMgr;

	//if matID is out of range, a default mateial should be returned
	if (matID >= tmpMatMgr->m_pMaterialList->size())
	{
		return NOISE_MACRO_DEFAULT_MATERIAL_ID;
	}

	//a no-problem texID
	return matID;
}
