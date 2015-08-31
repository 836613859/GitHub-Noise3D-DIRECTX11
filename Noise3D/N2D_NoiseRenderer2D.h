
/***********************************************************************

					h��NoiseRenderer2D
				��Ҫ���� ��������Ⱦ2D

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseRenderer2D
{

	friend class NoiseScene;

public:
	//���캯��
	NoiseRenderer2D();
	~NoiseRenderer2D();

private:

	BOOL	mFunction_Init();

	BOOL	mFunction_Init_CreateEffectFromFile(LPCWSTR fxPath);

	BOOL	mFunction_Init_CreateEffectFromMemory(char* compiledShaderPath);


private:
	
	NoiseScene*									m_pFatherScene;

};