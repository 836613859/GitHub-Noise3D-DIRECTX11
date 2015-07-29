
/***********************************************************************

                           h��NoiseRenderer
					��Ҫ���� ��������Ⱦ

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseRenderer : private NoiseFileManager
{

	friend class NoiseScene;
	friend class NoiseMesh;

public:
	//���캯��
	NoiseRenderer();
	~NoiseRenderer();

	void			RenderMeshInList();

	void			Draw_Line3D(NVECTOR3 v1, NVECTOR3 v2, NVECTOR4 color1, NVECTOR4 color2);

	void			Draw_CoordinateFrame(float fAxisLength);

	void			ClearViews();

	void			RenderToScreen();

	void			SetFillMode(NOISE_FILLMODE iMode);




private:
	BOOL			mFunction_Init();

	BOOL			mFunction_Init_CreateEffectFromFile(LPCWSTR fxPath);

	BOOL			mFunction_Init_CreateEffectFromMemory(char* compiledShaderPath);

	void				mFunction_SetRasterState(NOISE_FILLMODE iMode);

	void				mFunction_RenderMeshInList_UpdateCbPerObject();

	void				mFunction_RenderMeshInList_UpdateCbPerFrame();

	void				mFunction_RenderMeshInList_UpdateCbPerSubset();

	void				mFunction_RenderMeshInList_UpdateCbRarely();

	void				mFunction_RenderLine3D_UpdateCbDrawLine3D();



	std::vector <NoiseMesh*>*		m_pRenderList_Mesh;
	ID3D11Buffer*							m_pVertexBuffer_Line3D;//������������,���ڼ�ʱ����
	NoiseScene*								m_pFatherScene ;
	NOISE_FILLMODE						m_FillMode;//���ģʽ


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
	ID3DX11EffectConstantBuffer* m_pFX_CbPerFrame;
	ID3DX11EffectConstantBuffer*	m_pFX_CbPerSubset;
	ID3DX11EffectConstantBuffer*	m_pFX_CbRarely;
	ID3DX11EffectConstantBuffer* m_pFX_CbDrawLine3D;

};