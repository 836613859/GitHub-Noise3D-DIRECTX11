
/***********************************************************************

                           h��NoiseEngine

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseEngine
{
public:

	//���캯��
	NoiseEngine();

	BOOL	InitD3D(HWND RenderHWND,UINT BufferWidth,UINT BufferHeight,BOOL IsWindowed);

	void	ReleaseAll();

};

