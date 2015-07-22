
/***********************************************************************

                           h��NoiseRenderer
					��Ҫ���� ��������Ⱦ

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseRenderer
{
	//sceneҪ�����ܵ��඼����=v=
	friend class NoiseScene;
	friend class NoiseMesh;
	friend class NoiseLightManager;

public:
	//���캯��
	NoiseRenderer();
	~NoiseRenderer();

	void			RenderMeshInList();

	void			ClearViews();

	void			RenderToScreen();

	void			SetFillMode(NOISE_FILLMODE iMode);


private:
	BOOL			m_Function_Init();

	BOOL			m_Function_Init_CreateEffectFromFile();

	BOOL			m_Function_Init_CreateEffectFromMemory();

	void				m_Function_SetRasterState(NOISE_FILLMODE iMode);

	void				m_Function_RenderMeshInList_UpdateCbPerObject();

	void				m_Function_RenderMeshInList_UpdateCbPerFrame();

	void				m_Function_RenderMeshInList_UpdateCbPerSubset();

	void				m_Function_RenderMeshInList_UpdateCbRarely();


	NoiseScene*								m_pFatherScene ;
	NOISE_FILLMODE						m_FillMode;//���ģʽ
	//��App���ȶ��������Struct��һ�θ���
	N_CbPerFrame							m_CbPerFrame;
	N_CbPerObject							m_CbPerObject;
	N_CbPerSubset							m_CbPerSubset;
	N_CbRarely								m_CbRarely;

	//���ڴ�app���µ�Gpu�Ľӿ�
	ID3DX11Effect*							m_pFX;
	ID3DX11EffectTechnique*			m_pFX_Tech_Basic;
	ID3DX11EffectConstantBuffer* m_pFX_CbPerObject;
	ID3DX11EffectConstantBuffer* m_pFX_CbPerFrame;
	ID3DX11EffectConstantBuffer*	m_pFX_CbPerSubset;
	ID3DX11EffectConstantBuffer*	m_pFX_CbRarely;
};