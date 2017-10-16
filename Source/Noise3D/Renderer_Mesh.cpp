
/***********************************************************************

							Desc:  Noise Mesh Renderer (D3D)
	
************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

void	IRenderer::RenderMeshes()
{
	ICamera* const tmp_pCamera = GetScene()->GetCamera();

	//����ConstantBuffer: Only update when modified(cbRarely)
	mFunction_RenderMeshInList_UpdateCbRarely();

	//Update ConstantBuffer: Once Per Frame (cbPerFrame)
	mFunction_RenderMeshInList_UpdateCbPerFrame(tmp_pCamera);

	//Update ConstantBuffer : Proj / View Matrix (this function could be used elsewhere)
	mFunction_CameraMatrix_Update(tmp_pCamera);


#pragma region Render Mesh
	//for every mesh
	for (UINT i = 0; i<(m_pRenderList_Mesh->size()); i++)
	{
		//ȡ����Ⱦ�б��е�meshָ��
		IMesh* const pMesh = m_pRenderList_Mesh->at(i);

		//����ConstantBuffer:ÿ�������һ��(cbPerObject)
		mFunction_RenderMeshInList_UpdateCbPerObject(pMesh);

		//������cb��׼����ʼdraw��
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Default);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &pMesh->m_pVB_Gpu, &g_cVBstride_Default, &g_cVBoffset);
		g_pImmediateContext->IASetIndexBuffer(pMesh->m_pIB_Gpu, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(pMesh->GetFillMode(), pMesh->GetCullMode());

		//����blend state
		mFunction_SetBlendState(pMesh->GetBlendMode());

		//����samplerState
		m_pFX_SamplerState_Default->SetSampler(0, m_pSamplerState_FilterLinear);

		//����depth/Stencil State
		g_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState_EnableDepthTest, 0xffffffff);


		//for every subset
		UINT meshSubsetCount = pMesh->mSubsetInfoList.size();
		for (UINT j = 0;j < meshSubsetCount;j++)
		{
			//subset info
			UINT currSubsetIndicesCount = pMesh->mSubsetInfoList.at(j).primitiveCount * 3;
			UINT currSubsetStartIndex = pMesh->mSubsetInfoList.at(j).startPrimitiveID * 3;

			//����ConstantBuffer:ÿSubset,��һ��mesh�����в�ͬMaterial�Ķ���һ��subset
			mFunction_RenderMeshInList_UpdateCbPerSubset(pMesh,j);

			//��������tech������pass ---- index starts from 1
			D3DX11_TECHNIQUE_DESC tmpTechDesc;
			m_pFX_Tech_Default->GetDesc(&tmpTechDesc);
			for (UINT k = 0;k < tmpTechDesc.Passes; k++)
			{
				m_pFX_Tech_Default->GetPassByIndex(k)->Apply(0, g_pImmediateContext);
				g_pImmediateContext->DrawIndexed(currSubsetIndicesCount, currSubsetStartIndex, 0);
			}
		}
	}
#pragma endregion Render Mesh

}


/***********************************************************************
									P R I V A T E
************************************************************************/


void		IRenderer::mFunction_RenderMeshInList_UpdateCbRarely()
{

	bool tmpCanUpdateCbRarely = false;

	//������������Static Light������������
	ILightManager* tmpLightMgr = GetScene()->GetLightMgr();

	if ((tmpLightMgr != nullptr) && (tmpLightMgr->mCanUpdateStaticLights))
	{
		UINT tmpLight_Dir_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_STATIC_DIR);
		UINT tmpLight_Point_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_STATIC_POINT);
		UINT tmpLight_Spot_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_STATIC_SPOT);

		m_CbRarely.mIsLightingEnabled_Static = tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbRarely.mDirLightCount_Static = tmpLight_Dir_Count;
		m_CbRarely.mPointLightCount_Static = tmpLight_Point_Count;
		m_CbRarely.mSpotLightCount_Static = tmpLight_Spot_Count;

		for (UINT i = 0; i<(tmpLight_Dir_Count); i++)
		{
			//static directional light description
			m_CbRarely.mDirectionalLight_Static[i] = (tmpLightMgr->GetDirLightS(i)->GetDesc());
		}

		for (UINT i = 0; i<(tmpLight_Point_Count); i++)
		{
			m_CbRarely.mPointLight_Static[i] = (tmpLightMgr->GetPointLightS(i)->GetDesc());
		}

		for (UINT i = 0; i<(tmpLight_Spot_Count); i++)
		{
			m_CbRarely.mSpotLight_Static[i] = (tmpLightMgr->GetSpotLightS(i)->GetDesc());
		}

		//static light only need to update once for INITIALIZATION
		tmpLightMgr->mCanUpdateStaticLights = false;
	}


	//���������������µ�GPU������������
	if (tmpCanUpdateCbRarely == true)
	{
		m_pFX_CbRarely->SetRawValue(&m_CbRarely, 0, sizeof(m_CbRarely));
	};
};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerFrame(ICamera*const pCamera)
{
	//��������Update Dynamic Light��������
	ILightManager* tmpLightMgr = GetScene()->GetLightMgr();
	if (tmpLightMgr != NULL)
	{
		UINT tmpLight_Dir_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_DYNAMIC_DIR);
		UINT tmpLight_Point_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_DYNAMIC_POINT);
		UINT tmpLight_Spot_Count = tmpLightMgr->GetLightCount(NOISE_LIGHT_TYPE_DYNAMIC_SPOT);

		m_CbPerFrame.mIsLightingEnabled_Dynamic = tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbPerFrame.mDirLightCount_Dynamic = tmpLight_Dir_Count;
		m_CbPerFrame.mPointLightCount_Dynamic = tmpLight_Point_Count;
		m_CbPerFrame.mSpotLightCount_Dynamic = tmpLight_Spot_Count;

		for (UINT i = 0; i<(tmpLight_Dir_Count); i++)
		{
			m_CbPerFrame.mDirectionalLight_Dynamic[i] = tmpLightMgr->GetDirLightD(i)->GetDesc();
		}

		for (UINT i = 0; i<(tmpLight_Point_Count); i++)
		{
			m_CbPerFrame.mPointLight_Dynamic[i] = tmpLightMgr->GetPointLightD(i)->GetDesc();
		}

		for (UINT i = 0; i<(tmpLight_Spot_Count); i++)
		{
			m_CbPerFrame.mSpotLight_Dynamic[i] = tmpLightMgr->GetSpotLightD(i)->GetDesc();
		}

	}


	//��������Update to GPU������������
	m_pFX_CbPerFrame->SetRawValue(&m_CbPerFrame, 0, sizeof(m_CbPerFrame));
};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerSubset(IMesh* const pMesh,UINT subsetID)
{
	//we dont accept invalid material ,but accept invalid texture
	ITextureManager*		pTexMgr = GetScene()->GetTextureMgr();
	IMaterialManager*		pMatMgr = GetScene()->GetMaterialMgr();

	//Get Material ID by unique name
	N_UID	 currSubsetMatName = pMesh->mSubsetInfoList.at(subsetID).matName;
	bool  IsMatNameValid = pMatMgr->FindUid(currSubsetMatName);

	//if material ID == INVALID_MAT_ID , then we should use default mat defined in mat mgr
	//then we should check if its child textureS are valid too 
	N_MaterialDesc tmpMat;
	if (IsMatNameValid== false)
	{
		WARNING_MSG("IRenderer : material UID not valid when rendering mesh.");
		pMatMgr->GetDefaultMaterial()->GetDesc(tmpMat);
	}
	else
	{
		pMatMgr->GetMaterial(currSubsetMatName)->GetDesc(tmpMat);
	}


	//Validate Indices of MATERIALS/TEXTURES
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	//m_CbPerSubset.basicMaterial = tmpMat.baseMaterial;
	m_CbPerSubset.SetBaseMat(tmpMat);

	ITexture* pDiffMap = pTexMgr->GetTexture(tmpMat.diffuseMapName);
	ITexture* pNormalMap = pTexMgr->GetTexture(tmpMat.normalMapName);
	ITexture* pSpecMap = pTexMgr->GetTexture(tmpMat.specularMapName);
	ITexture* pEnvMap = pTexMgr->GetTexture(tmpMat.environmentMapName);

	//first validate if ID is valid (within range / valid ID) valid== return original texID
	if(pDiffMap)			m_CbPerSubset.IsDiffuseMapValid = pDiffMap->IsTextureType(NOISE_TEXTURE_TYPE_COMMON);
						else	m_CbPerSubset.IsDiffuseMapValid = FALSE;

	if (pNormalMap)	m_CbPerSubset.IsNormalMapValid = pNormalMap->IsTextureType(NOISE_TEXTURE_TYPE_COMMON);
						else	m_CbPerSubset.IsNormalMapValid = FALSE;

	if (pSpecMap)		m_CbPerSubset.IsSpecularMapValid = pSpecMap->IsTextureType(NOISE_TEXTURE_TYPE_COMMON);
						else	m_CbPerSubset.IsSpecularMapValid = FALSE;

	if (pEnvMap)		m_CbPerSubset.IsEnvironmentMapValid = pEnvMap->IsTextureType(NOISE_TEXTURE_TYPE_CUBEMAP);
						else	m_CbPerSubset.IsEnvironmentMapValid = FALSE;

	//update textures, bound corresponding ShaderResourceView to the pipeline
	//if tetxure is  valid ,then set diffuse map
	if (m_CbPerSubset.IsDiffuseMapValid)
	{
		tmp_pSRV = pDiffMap->m_pSRV;
		m_pFX_Texture_Diffuse->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set normal map
	if (m_CbPerSubset.IsNormalMapValid)
	{
		tmp_pSRV = pNormalMap->m_pSRV;
		m_pFX_Texture_Normal->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set specular map
	if (m_CbPerSubset.IsSpecularMapValid)
	{
		tmp_pSRV = pSpecMap->m_pSRV;
		m_pFX_Texture_Specular->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set environment map (cube map)
	if (m_CbPerSubset.IsEnvironmentMapValid)
	{
		tmp_pSRV = pEnvMap->m_pSRV;
		m_pFX_Texture_CubeMap->SetResource(tmp_pSRV);//environment map is a cube map
	}

	//transmit all data to gpu
	m_pFX_CbPerSubset->SetRawValue(&m_CbPerSubset, 0, sizeof(m_CbPerSubset));

};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerObject(IMesh* const pMesh)
{
	//������������World Matrix��������
	pMesh->GetWorldMatrix(m_CbPerObject.mWorldMatrix, m_CbPerObject.mWorldInvTransposeMatrix);
	
	//���������������µ�GPU������������
	m_pFX_CbPerObject->SetRawValue(&m_CbPerObject, 0, sizeof(m_CbPerObject));
};

