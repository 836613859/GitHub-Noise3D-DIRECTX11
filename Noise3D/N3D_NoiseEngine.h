
/***********************************************************************

										h��NoiseEngine

************************************************************************/

#pragma once


class _declspec(dllexport) NoiseEngine:public NoiseClassLifeCycle
{
public:

	//���캯��
	NoiseEngine();

	HWND	CreateRenderWindow(UINT pixelWidth,UINT pixelHeight,LPCWSTR windowTitle, HINSTANCE hInstance);

	BOOL	InitD3D(HWND RenderHWND,UINT BufferWidth,UINT BufferHeight,BOOL IsWindowed);

	void	ReleaseAll();

	void Mainloop();

	void	SetMainLoopFunction( void (*pFunction)(void));//function pointer

	void	SetMainLoopStatus(NOISE_MAINLOOP_STATUS loopStatus);

	HWND GetRenderWindowHWND();

	HINSTANCE GetRenderWindowHINSTANCE();

	int	 GetRenderWindowWidth();

	int	 GetRenderWindowHeight();

private:

	void		Destroy();

	//this is for built-in window creation function
	LPCWSTR			mRenderWindowTitle;
	LPCWSTR			mRenderWindowClassName;
	HINSTANCE		mRenderWindowHINSTANCE;
	HWND				mRenderWindowHWND;

	UINT					mRenderWindowPixelWidth;
	UINT					mRenderWindowPixelHeight;

	//���ڴ�����ѭ���ĺ���ָ�� ��ΪҪ��װ��Ϣѭ�� ����ֻ��������
	 void				(*m_pMainLoopFunction)(void);
	 NOISE_MAINLOOP_STATUS		mMainLoopStatus;

private:

	 //������Ⱦ���ڵ��Ӻ���
	BOOL	mFunction_InitWindowClass(WNDCLASS* wc);
	//������Ⱦ���ڵ��Ӻ���
	HWND mFunction_InitWindow();
};

