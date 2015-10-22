

/***********************************************************************

                           h����

************************************************************************/

//��COM�Ѿ��������˾Ͳ����ͷ���
//������һ���ո�����Ҫ�滻���ı�
#pragma once

#define MATH_PI 3.1415926f
//the first element in Mat Mgr is default value (means we dont accept invalid material)
//but texture can be invalid
#define NOISE_MACRO_INVALID_MESH_ID				UINT_MAX
#define	 NOISE_MACRO_INVALID_TEXTURE_ID			UINT_MAX
#define NOISE_MACRO_INVALID_GRAPHICOBJ_ID	UINT_MAX
#define NOISE_MACRO_DEFAULT_MATERIAL_ID 0U
#define NOISE_MACRO_DEFAULT_COLOR_BYTESIZE 16U //DXGI_FORMAT_R32G32B32A32_FLOAT 128bit 16bytes
#define NOISE_MACRO_FUNCTION_EXTERN_CALL   //A mark that indicates this function is called by external friend class


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
				DEBUG_MSG1(MsgText);\
				return FALSE;\
				};\


//���ԣ�������Ϣ
#define DEBUG_MSG3(msg1,msg2,msg3)\
				g_Debug_MsgString<<msg1<<msg2<<msg3<<std::endl;\
				g_Debug_MsgString<<"file: "<<__FILE__<<std::endl;\
				g_Debug_MsgString<<"line: "<<__LINE__<<std::endl;\
				MessageBoxA(0,g_Debug_MsgString.str().c_str(),0,0);\
				g_Debug_MsgString.clear()\

#define DEBUG_MSG1(msg)\
				g_Debug_MsgString<<msg<<std::endl;\
				g_Debug_MsgString<<"file: "<<__FILE__<<std::endl;\
				g_Debug_MsgString<<"line: "<<__LINE__<<std::endl;\
				MessageBoxA(0,g_Debug_MsgString.str().c_str(),0,0);\
				g_Debug_MsgString.clear()\

;//�ļ�β



