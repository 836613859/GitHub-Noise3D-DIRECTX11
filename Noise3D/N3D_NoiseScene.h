
/***********************************************************************

                           h��NoiseScene

************************************************************************/

#pragma once

public class _declspec(dllexport) NoiseScene
{
	friend class NoiseMesh;
	friend class NoiseRenderer;
	friend class NoiseCamera;
	friend class NoiseGraphicObject;
	friend class NoiseMaterialManager;
	friend class NoiseTextureManager;
	friend class NoiseAtmosphere;


public:
	//���캯��
	NoiseScene();

	void				ReleaseAllChildObject();

	BOOL			CreateMesh(NoiseMesh* pMesh);
	
	BOOL			CreateRenderer(NoiseRenderer* pRenderer);

	BOOL			CreateCamera(NoiseCamera* pSceneCam);

	BOOL			CreateLightManager(NoiseLightManager* pLightMgr);

	BOOL			CreateGraphicObject(NoiseGraphicObject* pGraphicObj);

	BOOL			CreateTextureManager(NoiseTextureManager* pTexMgr);

	BOOL			CreateMaterialManager(NoiseMaterialManager* pMatMgr);

	BOOL			CreateAtmosphere(NoiseAtmosphere* pAtmosphere);

	void				SetCamera(NoiseCamera* pSceneCam);

	NoiseRenderer*					GetRenderer();

	NoiseCamera*					GetCamera();

	NoiseLightManager*			GetLightManager();

	NoiseTextureManager*		GetTextureMgr();

	NoiseMaterialManager*	GetMaterialMgr();

private:

	NoiseCamera*							m_pChildCamera;

	NoiseRenderer*							m_pChildRenderer;

	NoiseLightManager*					m_pChildLightMgr;

	NoiseTextureManager*				m_pChildTextureMgr;

	NoiseMaterialManager*					m_pChildMaterialMgr;

	NoiseAtmosphere*							m_pChildAtmosphere;

	std::vector<NoiseMesh*> *				m_pChildMeshList;

	std::vector<NoiseGraphicObject*>*	m_pChildGraphicObjectList;

};

