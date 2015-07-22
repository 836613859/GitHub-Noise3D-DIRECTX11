#include "RenderWindow.h"

//using namespace Noise3D;
//NoiseEngine NsEngine;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR szCmdLine, int iCmdShow)
{
	MSG msg;//��Ϣ��
	ZeroMemory(&msg,sizeof(msg));
	WNDCLASS wndclass;
	HWND hwnd;//���

	//������ע��
	if(InitWindowClass(&wndclass,hInstance) == FALSE)
	{
		MessageBox(0,"�����ഴ��ʧ��","FAIL",0);
		return 0;
	};

	//hwnd�߸�ֵ���жϱ�д��if������= =
	hwnd = InitWindow(hInstance);
	if(hwnd  == 0)
	{
		MessageBox(0,"���崴��ʧ��","FAIL",0);
		return 0;
	};

	//��ʼ��3D����
	if(Init3D(hwnd) == FALSE)
	{
		MessageBox(0,"D3D��ʼ��ʧ��","FAIL",0);
		return 0;
	};

	//�ڳ���û�н��յ����˳�����Ϣ��ʱ��
	while(msg.message != WM_QUIT)
	{
		/*PM_REMOVE  PeekMessage�������Ϣ�Ӷ����������
		������PeekMesssge���Բ鿴�ķ�ʽ��ϵͳ�л�ȡ��Ϣ��
		���Բ�����Ϣ��ϵͳ���Ƴ����Ƿ�����������
		��ϵͳ����Ϣʱ������FALSE������ִ�к������롣*/

		//����Ϣ��ʱ��Ͻ������� ��Peek����TRUE��ʱ��
		if(PeekMessage(&msg,NULL,0U,0U,PM_REMOVE))
		{
			TranslateMessage (&msg);//������Ϣ�����͵�windows��Ϣ����
			DispatchMessage (&msg) ;//������Ϣ
		}
		else
			//��Ϣ����û���������� �Ǿ͸���ѭ��
		{
			MainLoop();
		};
	}
	Cleanup();//����
	UnregisterClass(WindowClassName, hInstance );
	return msg.wParam ;
}



BOOL InitWindowClass(WNDCLASS* wc,HINSTANCE hInstance)
{
	wc->style = CS_HREDRAW | CS_VREDRAW ; //��ʽ
	wc->cbClsExtra   = 0 ;
	wc->cbWndExtra   = 0 ;
	wc->hInstance = hInstance ;//����ʵ��������windows�Զ��ַ�
	wc->hIcon = LoadIcon (NULL, IDI_APPLICATION) ;//��ʾ�����ͼ��titlte
	wc->hCursor = LoadCursor (NULL, IDC_ARROW) ;//���ڹ��
	wc->hbrBackground= (HBRUSH) GetStockObject (WHITE_BRUSH) ;//����ˢ
	wc->lpszMenuName=NULL;
	wc->lpfnWndProc=WndProc;//���ô������windws��Ϣ����
	wc->lpszClassName= WindowClassName;//��������

	if (!RegisterClass (wc))//ע�ᴰ����
	{
		MessageBox ( NULL, TEXT ("This program requires Windows NT!"), TEXT("R"), MB_ICONERROR) ;
		return FALSE ;
	}
	else
	{
		return TRUE;
	};

};

HWND InitWindow(HINSTANCE hInstance)
{
	HWND hwnd;
	hwnd = CreateWindow(WindowClassName,      // window class name
		AppName,   // window caption
		WS_OVERLAPPEDWINDOW, // window style
		CW_USEDEFAULT,// initial x position
		CW_USEDEFAULT,// initial y position
		640,// initial x size
		480,// initial y size
		NULL, // parent window handle
		NULL, // window menu handle
		hInstance, // program instance handle
		NULL) ;

	if (hwnd == 0)
	{
		//����ʧ��
		return 0;
	}
	else
	{
		//�����ɹ�
		ShowWindow (hwnd,SW_RESTORE);//��ʾ����
		UpdateWindow (hwnd) ;//���´���
		return hwnd;
	};

};

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)//��Ϣ�Ĵ������
{
	/*HDC			hdc ;
	PAINTSTRUCT ps ;
	RECT        rect ;*/
	switch (message)
	{
	case WM_CREATE:
		return 0 ;

	case WM_DESTROY:
		PostQuitMessage (0) ;
		return 0 ;
	}

	return DefWindowProc (hwnd, message, wParam, lParam) ;

}


