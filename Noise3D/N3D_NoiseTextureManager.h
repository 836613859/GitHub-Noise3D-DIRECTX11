/***********************************************************************

                           h��NoiseTexturelManager

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseTextureManager
{
public:
	friend NoiseScene;
	friend NoiseRenderer;

	//���캯��
	NoiseTextureManager();


private:
	NoiseScene*		m_pFatherScene;

};
