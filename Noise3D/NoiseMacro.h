

/***********************************************************************

                           h����

************************************************************************/

//��COM�Ѿ��������˾Ͳ����ͷ���
//������һ���ո�����Ҫ�滻���ı�
#pragma once

#define MATH_PI 3.1415926f

//�ͷ�һ��COM����
#define ReleaseCOM(ComPointer)\
				if(ComPointer!=0)\
				{\
				ComPointer->Release();\
				}\


//���ԣ����Ե���
#define HR_DEBUG(hr,MsgText)\
				if(FAILED(hr)) \
				{\
				MessageBoxA(0,MsgText,0,0);\
				return FALSE;\
				};\


//���ԣ�������Ϣ
#define DEBUG_MSG(msg1,msg2,msg3)\
				g_Debug_MsgString<<msg1<<msg2<<msg3;\
				MessageBoxA(0,g_Debug_MsgString.str().c_str(),0,0);\
				g_Debug_MsgString.clear()\


;//�ļ�β



