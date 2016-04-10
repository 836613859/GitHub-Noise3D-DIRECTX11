
/***********************************************************************

							Desc:  Noise Mesh Renderer (D3D)
	
************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

void	IRenderer::RenderMeshes()
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
	ICamera* const tmp_pCamera = m_pFatherScene->GetCamera();

	//����ConstantBuffer:�޸Ĺ��͸���(cbRarely)
	mFunction_RenderMeshInList_UpdateCbRarely();


	//Update ConstantBuffer: Once Per Frame (cbPerFrame)
	mFunction_RenderMeshInList_UpdateCbPerFrame(tmp_pCamera);

	//Update ConstantBuffer : Proj / View Matrix (this function could be used elsewhere)
	mFunction_CameraMatrix_Update(tmp_pCamera);


#pragma region Render Mesh
	//for every mesh
	for (i = 0; i<(m_pRenderList_Mesh->size()); i++)
	{
		//ȡ����Ⱦ�б��е�meshָ��
		IMesh* const tmp_pMesh = m_pRenderList_Mesh->at(i);

		//����ConstantBuffer:ÿ�������һ��(cbPerObject)
		mFunction_RenderMeshInList_UpdateCbPerObject(tmp_pMesh);

		//������cb��׼����ʼdraw��
		g_pImmediateContext->IASetInputLayout(g_pVertexLayout_Default);
		g_pImmediateContext->IASetVertexBuffers(0, 1, &tmp_pMesh->m_pVB_Gpu, &c_VBstride_Default, &c_VBoffset);
		g_pImmediateContext->IASetIndexBuffer(tmp_pMesh->m_pIB_Gpu, DXGI_FORMAT_R32_UINT, 0);
		g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//����fillmode��cullmode
		mFunction_SetRasterState(m_FillMode, m_CullMode);

		//����blend state
		mFunction_SetBlendState(m_BlendMode);

		//����samplerState
		m_pFX_SamplerState_Default->SetSampler(0, m_pSamplerState_FilterAnis);

		//����depth/Stencil State
		g_pImmediateContext->OMSetDepthStencilState(m_pDepthStencilState_EnableDepthTest, 0xffffffff);


		//for every subset
		UINT meshSubsetCount = tmp_pMesh->m_pSubsetInfoList->size();
		for (j = 0;j < meshSubsetCount;j++)
		{
			//subset info
			UINT currSubsetIndicesCount = tmp_pMesh->m_pSubsetInfoList->at(j).primitiveCount * 3;
			UINT currSubsetStartIndex = tmp_pMesh->m_pSubsetInfoList->at(j).startPrimitiveID * 3;

			//����ConstantBuffer:ÿSubset,��һ��mesh�����в�ͬMaterial�Ķ���һ��subset
			mFunction_RenderMeshInList_UpdateCbPerSubset(tmp_pMesh,j);

			//��������tech������pass ---- index starts from 1
			D3DX11_TECHNIQUE_DESC tmpTechDesc;
			m_pFX_Tech_Default->GetDesc(&tmpTechDesc);
			for (k = 0;k < tmpTechDesc.Passes; k++)
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

	BOOL tmpCanUpdateCbRarely = FALSE;

	//������������Static Light������������
	ILightManager* tmpLightMgr = m_pFatherScene->m_pChildLightMgr;

	if ((tmpLightMgr != NULL) && (tmpLightMgr->mCanUpdateStaticLights))
	{
		int tmpLight_Dir_Count = tmpLightMgr->m_pLightList_Dir_Static->size();
		int tmpLight_Point_Count = tmpLightMgr->m_pLightList_Point_Static->size();
		int tmpLight_Spot_Count = tmpLightMgr->m_pLightList_Spot_Static->size();

		m_CbRarely.mIsLightingEnabled_Static = tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbRarely.mDirLightCount_Static = tmpLight_Dir_Count;
		m_CbRarely.mPointLightCount_Static = tmpLight_Point_Count;
		m_CbRarely.mSpotLightCount_Static = tmpLight_Spot_Count;

		int i = 0;

		for (i = 0; i<(tmpLight_Dir_Count); i++)
		{
			m_CbRarely.mDirectionalLight_Static[i] = (tmpLightMgr->m_pLightList_Dir_Static->at(i));
		}

		for (i = 0; i<(tmpLight_Point_Count); i++)
		{
			m_CbRarely.mPointLight_Static[i] = (tmpLightMgr->m_pLightList_Point_Static->at(i));
		}

		for (i = 0; i<(tmpLight_Spot_Count); i++)
		{
			m_CbRarely.mSpotLight_Static[i] = (tmpLightMgr->m_pLightList_Spot_Static->at(i));
		}

		//���� ���ɸ��¡�״̬����֤static light ֻ���г�ʼ��
		tmpLightMgr->mCanUpdateStaticLights = FALSE;
	}


	//���������������µ�GPU������������
	if (tmpCanUpdateCbRarely == TRUE)
	{
		m_pFX_CbRarely->SetRawValue(&m_CbRarely, 0, sizeof(m_CbRarely));
	};
};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerFrame(ICamera*const pCamera)
{
	//������������Dynamic Light��������
	ILightManager* tmpLightMgr = m_pFatherScene->m_pChildLightMgr;
	if (tmpLightMgr != NULL)
	{
		int tmpLight_Dir_Count = tmpLightMgr->m_pLightList_Dir_Dynamic->size();
		int tmpLight_Point_Count = tmpLightMgr->m_pLightList_Point_Dynamic->size();
		int tmpLight_Spot_Count = tmpLightMgr->m_pLightList_Spot_Dynamic->size();

		m_CbPerFrame.mIsLightingEnabled_Dynamic = tmpLightMgr->mIsDynamicLightingEnabled;
		m_CbPerFrame.mDirLightCount_Dynamic = tmpLight_Dir_Count;
		m_CbPerFrame.mPointLightCount_Dynamic = tmpLight_Point_Count;
		m_CbPerFrame.mSpotLightCount_Dynamic = tmpLight_Spot_Count;
		m_CbPerFrame.mCamPos = *(pCamera->m_pPosition);

		int i = 0;

		for (i = 0; i<(tmpLight_Dir_Count); i++)
		{
			m_CbPerFrame.mDirectionalLight_Dynamic[i] = *(tmpLightMgr->m_pLightList_Dir_Dynamic->at(i));
		}

		for (i = 0; i<(tmpLight_Point_Count); i++)
		{
			m_CbPerFrame.mPointLight_Dynamic[i] = *(tmpLightMgr->m_pLightList_Point_Dynamic->at(i));
		}

		for (i = 0; i<(tmpLight_Spot_Count); i++)
		{
			m_CbPerFrame.mSpotLight_Dynamic[i] = *(tmpLightMgr->m_pLightList_Spot_Dynamic->at(i));
		}

	}


	//�����������µ�GPU������������
	m_pFX_CbPerFrame->SetRawValue(&m_CbPerFrame, 0, sizeof(m_CbPerFrame));
};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerSubset(IMesh* const pMesh,UINT subsetID)
{
	//we dont accept invalid material ,but accept invalid texture
	ITextureManager*		pSceneTexMgr = m_pFatherScene->m_pChildTextureMgr;
	IMaterialManager*	pSceneMatMgr = m_pFatherScene->m_pChildMaterialMgr;

	//Get Material ID by unique name
	UINT	 currSubsetMatID = pSceneMatMgr->GetMatID(pMesh->m_pSubsetInfoList->at(subsetID).matName);

	//if material ID == INVALID_MAT_ID , then we should use default mat defined in mat mgr
	//then we should check if its child textureS are valid too 
	N_Material tmpMat;
	if (currSubsetMatID == NOISE_MACRO_INVALID_MATERIAL_ID)
	{
		pSceneMatMgr->GetDefaultMaterial(tmpMat);
	}
	else
	{
		pSceneMatMgr->GetMaterial(currSubsetMatID, tmpMat);
	}


	//Validate Indices of MATERIALS/TEXTURES
	ID3D11ShaderResourceView* tmp_pSRV = nullptr;
	m_CbPerSubset.basicMaterial = tmpMat.baseMaterial;

	UINT diffMapIndex = pSceneTexMgr->GetTextureID(tmpMat.diffuseMapName);
	UINT normalMapIndex = pSceneTexMgr->GetTextureID(tmpMat.normalMapName);
	UINT specularMapIndex = pSceneTexMgr->GetTextureID(tmpMat.specularMapName);
	UINT envMapIndex = pSceneTexMgr->GetTextureID(tmpMat.environmentMapName);

	//first validate if ID is valid (within range / valid ID) valid== return original texID
	m_CbPerSubset.IsDiffuseMapValid = (pSceneTexMgr->ValidateIndex(diffMapIndex, NOISE_TEXTURE_TYPE_COMMON)
		== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
	m_CbPerSubset.IsNormalMapValid = (pSceneTexMgr->ValidateIndex(normalMapIndex, NOISE_TEXTURE_TYPE_COMMON)
		== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
	m_CbPerSubset.IsSpecularMapValid = (pSceneTexMgr->ValidateIndex(specularMapIndex, NOISE_TEXTURE_TYPE_COMMON)
		== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);
	m_CbPerSubset.IsEnvironmentMapValid = (pSceneTexMgr->ValidateIndex(envMapIndex, NOISE_TEXTURE_TYPE_CUBEMAP)
		== NOISE_MACRO_INVALID_TEXTURE_ID ? FALSE : TRUE);


	//update textures, bound corresponding ShaderResourceView to the pipeline
	//if tetxure is  valid ,then set diffuse map
	if (m_CbPerSubset.IsDiffuseMapValid)
	{
		tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(diffMapIndex).m_pSRV;
		m_pFX_Texture_Diffuse->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set normal map
	if (m_CbPerSubset.IsNormalMapValid)
	{
		tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(normalMapIndex).m_pSRV;
		m_pFX_Texture_Normal->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set specular map
	if (m_CbPerSubset.IsSpecularMapValid)
	{
		tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(specularMapIndex).m_pSRV;
		m_pFX_Texture_Specular->SetResource(tmp_pSRV);
	}

	//if tetxure is  valid ,then set environment map (cube map)
	if (m_CbPerSubset.IsEnvironmentMapValid)
	{
		tmp_pSRV = m_pFatherScene->m_pChildTextureMgr->m_pTextureObjectList->at(envMapIndex).m_pSRV;
		m_pFX_Texture_CubeMap->SetResource(tmp_pSRV);//environment map is a cube map
	}

	//transmit all data to gpu
	m_pFX_CbPerSubset->SetRawValue(&m_CbPerSubset, 0, sizeof(m_CbPerSubset));

};

void		IRenderer::mFunction_RenderMeshInList_UpdateCbPerObject(IMesh* const pMesh)
{
	//������������World Matrix��������
	pMesh->mFunction_UpdateWorldMatrix();
	m_CbPerObject.mWorldMatrix = *(pMesh->m_pMatrixWorld);
	m_CbPerObject.mWorldInvTransposeMatrix = *(pMesh->m_pMatrixWorldInvTranspose);
	
	//���������������µ�GPU������������
	m_pFX_CbPerObject->SetRawValue(&m_CbPerObject, 0, sizeof(m_CbPerObject));
};

