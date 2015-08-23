
/***********************************************************************

                           h��NoiseRenderer
					��Ҫ���� ��������Ⱦ

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseRenderer : private NoiseFileManager
{

	friend class NoiseScene;
	friend class NoiseMesh;
	friend class NoiseLineBuffer;

public:
	//���캯��
	NoiseRenderer();
	~NoiseRenderer();

	void			RenderMeshInList();

	void			RenderLine3DInList();

	void			ClearViews();

	void			RenderToScreen();

	void			SetFillMode(NOISE_FILLMODE iMode);

	void			SetCullMode(NOISE_CULLMODE iMode);



private:
	BOOL			mFunction_Init();

	BOOL			mFunction_Init_CreateEffectFromFile(LPCWSTR fxPath);

	BOOL			mFunction_Init_CreateEffectFromMemory(char* compiledShaderPath);

	void				mFunction_SetRasterStateAndTopology(NOISE_FILLMODE iFillMode,NOISE_CULLMODE iCullMode);

	void				mFunction_RenderMeshInList_UpdateCbPerObject();

	void				mFunction_RenderMeshInList_UpdateCbPerFrame();

	void				mFunction_RenderMeshInList_UpdateCbPerSubset(UINT subsetID);

	void				mFunction_RenderMeshInList_UpdateCbRarely();

	void				mFunction_RenderLine3D_UpdateCbDrawLine3D();

private:
	std::vector <NoiseMesh*>*			m_pRenderList_Mesh;
	std::vector<NoiseLineBuffer*>* 	m_pRenderList_Line;
	//��դ������
	ID3D11RasterizerState*					m_pRasterState_Solid_CullNone;
	ID3D11RasterizerState*					m_pRasterState_Solid_CullBack;
	ID3D11RasterizerState*					m_pRasterState_Solid_CullFront;
	ID3D11RasterizerState*					m_pRasterState_WireFrame_CullFront;
	ID3D11RasterizerState*					m_pRasterState_WireFrame_CullNone;
	ID3D11RasterizerState*					m_pRasterState_WireFrame_CullBack;

	NoiseScene*									m_pFatherScene ;
	NOISE_FILLMODE							m_FillMode;//���ģʽ
	NOISE_CULLMODE						m_CullMode;//�޳�ģʽ

	//��App���ȶ��������Struct��һ�θ���
	N_CbPerFrame							m_CbPerFrame;
	N_CbPerObject							m_CbPerObject;
	N_CbPerSubset							m_CbPerSubset;
	N_CbRarely								m_CbRarely;
	N_CbDrawLine3D						m_CbDrawLine3D;

	//���ڴ�app���µ�Gpu�Ľӿ�
	ID3DX11Effect*							m_pFX;
	ID3DX11EffectTechnique*			m_pFX_Tech_Default;
	ID3DX11EffectTechnique*			m_pFX_Tech_DrawLine3D;
	ID3DX11EffectConstantBuffer* m_pFX_CbPerObject;
	ID3DX11EffectConstantBuffer*	m_pFX_CbPerFrame;
	ID3DX11EffectConstantBuffer*	m_pFX_CbPerSubset;
	ID3DX11EffectConstantBuffer*	m_pFX_CbRarely;
	ID3DX11EffectConstantBuffer*	m_pFX_CbDrawLine3D;
	ID3DX11EffectShaderResourceVariable* m_pFX_Texture_Diffuse;
	ID3DX11EffectShaderResourceVariable* m_pFX_Texture_Normal;
	ID3DX11EffectShaderResourceVariable* m_pFX_Texture_Specular;
};