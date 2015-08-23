
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
static D3DX11_TECHNIQUE_DESC	tmp_pTechDesc;



NoiseRenderer::NoiseRenderer()
{
	m_pFatherScene			= NULL;
	m_pRenderList_Mesh	= new std::vector <NoiseMesh*>;
	m_pRenderList_Line		= new std::vector<NoiseLineBuffer*>;
	m_pFX = NULL;
	m_pFX_Tech_Default = NULL;
	m_pFX_Tech_DrawLine3D = NULL;
	m_FillMode = NOISE_FILLMODE_SOLID;
	m_CullMode = NOISE_CULLMODE_NONE;
};

NoiseRenderer::~NoiseRenderer()
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
	UINT i = 0, j = 0, k = 0;
	 tmp_pCamera = m_pFatherScene->GetCamera();

	//����ConstantBuffer:�޸Ĺ��͸���(cbRarely)
	 mFunction_RenderMeshInList_UpdateCbRarely();


	//����ConstantBuffer:ÿ֡����һ�� (cbPerFrame)
	 mFunction_RenderMeshInList_UpdateCbPerFrame();


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

		//����fillmode��cullmode
		mFunction_SetRasterStateAndTopology(m_FillMode,m_CullMode);


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

	m_pRenderList_Mesh->clear();
}

void NoiseRenderer::RenderLine3DInList()
{
	tmp_pCamera = m_pFatherScene->GetCamera();

	//����ConstantBuffer:ר�Ÿ�draw Line 3D����һ��cbuffer�����Ż�
	mFunction_RenderLine3D_UpdateCbDrawLine3D();



	//������cb�Ϳ��Կ�ʼdraw��
	ID3D11Buffer* tmp_pVB = NULL;
	for (UINT i = 0;i < m_pFatherScene->m_pChildLineBufferList->size();i++)
	{
		//��RenderList������LineBuffer����Ⱦһ��
		tmp_pVB = m_pFatherScene->m_pChildLineBufferList->at(i)->m_pVertexBuffer;
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Simple);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pVB, &VBstride_Simple, &VBoffset);
		g_pImmediateContext->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
		//����fillmode��cullmode
		mFunction_SetRasterStateAndTopology(NOISE_FILLMODE_SOLID, NOISE_CULLMODE_NONE);

		//draw line һ��pass�͹���
		m_pFX_Tech_DrawLine3D->GetPassByIndex(0)->Apply(0, g_pImmediateContext);
		UINT vCount = m_pFatherScene->m_pChildLineBufferList->at(i)->GetLineCount() * 2;
		g_pImmediateContext->Draw(vCount, 0);
	}

	//�����Ⱦ�б�
	m_pRenderList_Line->clear();

}

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
}

void NoiseRenderer::SetCullMode(NOISE_CULLMODE iMode)
{
	m_CullMode = iMode;
};



/************************************************************************
                                            PRIVATE                        
************************************************************************/
BOOL	NoiseRenderer::mFunction_Init()
{
	HRESULT hr = S_OK;

	mFunction_Init_CreateEffectFromMemory("Main.fxo");
	//mFunction_Init_CreateEffectFromFile(L"Main.fx");

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
	m_pFX_CbPerFrame=m_pFX->GetConstantBufferByName("cbPerFrame");
	m_pFX_CbPerObject=m_pFX->GetConstantBufferByName("cbPerObject");
	m_pFX_CbPerSubset = m_pFX->GetConstantBufferByName("cbPerSubset");
	m_pFX_CbRarely=m_pFX->GetConstantBufferByName("cbRarely");
	m_pFX_CbDrawLine3D = m_pFX->GetConstantBufferByName("cbDrawLine3D");

	//����
	m_pFX_Texture_Diffuse = m_pFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	m_pFX_Texture_Normal = m_pFX->GetVariableByName("gNormalMap")->AsShaderResource();
	m_pFX_Texture_Specular = m_pFX->GetVariableByName("gSpecularMap")->AsShaderResource();

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

	//����Technique
	m_pFX_Tech_Default = m_pFX->GetTechniqueByName("DefaultDraw");
	m_pFX_Tech_DrawLine3D = m_pFX->GetTechniqueByName("DrawLine3D");


	return TRUE;
};

void		NoiseRenderer::mFunction_SetRasterStateAndTopology(NOISE_FILLMODE iFillMode, NOISE_CULLMODE iCullMode)
{
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
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
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
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;



	//render points
	case 	NOISE_FILLMODE_POINT:
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		break;

	default:
		break;
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
	//������������Proj Matrix��������
	tmp_pCamera->mFunction_UpdateProjMatrix();
	m_CbPerFrame.mProjMatrix = *(tmp_pCamera->m_pMatrixProjection);


	//������������View Matrix��������
	tmp_pCamera->mFunction_UpdateViewMatrix();
	m_CbPerFrame.mViewMatrix = *(tmp_pCamera->m_pMatrixView);


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

		//otherwise if the material is valid
		//then we should check if its child textureS are valid too 
		N_Material tmpMat = m_pFatherScene->m_pChildMaterialMgr->m_pMaterialList->at(currSubsetMatID);
		ID3D11ShaderResourceView* tmp_pSRV = NULL;
		m_CbPerSubset.basicMaterial = tmpMat.baseColor;
		m_CbPerSubset.IsDiffuseMapValid	= (tmpMat.diffuseMapID == NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
		m_CbPerSubset.IsNormalMapValid	= (tmpMat.normalMapID == NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
		m_CbPerSubset.IsSpecularMapValid	= (tmpMat.specularMapID == NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);

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

void		NoiseRenderer::mFunction_RenderLine3D_UpdateCbDrawLine3D() 
{
	//update proj matrix
	tmp_pCamera->mFunction_UpdateProjMatrix();
	m_CbDrawLine3D.mProjMatrix		= *(tmp_pCamera->m_pMatrixProjection);

	// update view matrix
	tmp_pCamera->mFunction_UpdateViewMatrix();
	m_CbDrawLine3D.mViewMatrix	= *(tmp_pCamera->m_pMatrixView);

	//���������������µ�GPU������������
	m_pFX_CbDrawLine3D->SetRawValue(&m_CbDrawLine3D, 0, sizeof(m_CbDrawLine3D));
};