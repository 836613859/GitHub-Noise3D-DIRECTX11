#pragma once
#include <Windows.h>
#include "Noise3D.h"

const TCHAR AppName[] = "Render Window" ; //������
const TCHAR WindowClassName[] = "RENDER";//"��������

//���壺
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM); //������Ϣ������(����windows�ͽ���windows��Ϣ)
BOOL InitWindowClass(WNDCLASS* wc,HINSTANCE hInstance);
HWND InitWindow(HINSTANCE hInstance);

//3D��
BOOL Init3D(HWND hwnd);
void MainLoop();
void Cleanup();