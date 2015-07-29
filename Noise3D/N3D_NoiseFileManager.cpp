
/***********************************************************************

                           �ࣺNOISE File Manager

			�����������ļ�������import export ����decode encode

************************************************************************/
#include "Noise3D.h"


NoiseFileManager::NoiseFileManager()
{

}

BOOL NoiseFileManager::ImportFile_PURE(char * pFilePath, std::vector<char>* pFileBuffer)
{
	if (!pFileBuffer)
	{
		return FALSE;
	}

	//�ļ�������
	std::ifstream fileIn(pFilePath, std::ios::binary);

	//�ļ������ھ�return
	if (!fileIn)
	{
		assert(FALSE : "File Path Not Exist !!");
		return FALSE;
	}

	//ָ���Ƶ��ļ�β
	fileIn.seekg(0, std::ios_base::end);

	//ָ��ָ���ļ�β����ǰλ�þ��Ǵ�С
	int fileSize = (int)fileIn.tellg();


	//ָ���Ƶ��ļ�ͷ
	fileIn.seekg(0, std::ios_base::beg);

	//...........
	int i = 0;char tmpC =0;
	while (!fileIn.eof())
	{
		//���ֽڶ�ȡ
		fileIn.get(tmpC);
		pFileBuffer->push_back(tmpC);
	}


	//�ر��ļ�
	fileIn.close();


	return TRUE;
}

BOOL NoiseFileManager::ImportFile_STL(char* pFilePath,std::vector<NVECTOR3>* pVertexBuffer,std::vector<UINT>* pIndexBuffer,std::vector<NVECTOR3>* pNormalBuffer, std::vector<char>* pFileInfo )
{
	
	//����һ����ʱfile buffer
	std::vector<char> tmpFileBuffer;

	//�����ļ�
	if (!ImportFile_PURE(pFilePath, &tmpFileBuffer))
	{
		assert (FALSE : "Load Pure File Failed !! ");
		return FALSE;
	}

		
	/*STL: Baidu encyclopedia
	������STL�ļ��ù̶����ֽ���������������Ƭ�ļ�����Ϣ��
		�ļ���ʼ��80���ֽ����ļ�ͷ�����ڴ����ļ�����
		�������� 4 ���ֽڵ�����������ģ�͵�������Ƭ������
		�����������ÿ��������Ƭ�ļ�����Ϣ��ÿ��������Ƭռ�ù̶���50���ֽڣ������� :

		3��4�ֽڸ�����(����Ƭ�ķ�ʸ��)
		3��4�ֽڸ�����(1���������)
		3��4�ֽڸ�����(2���������)
		3��4�ֽڸ�����(3���������)

		������Ƭ�����2���ֽ���������������Ƭ��������Ϣ��
		һ������������STL�ļ��Ĵ�СΪ��������Ƭ������ 50�ټ���84���ֽڡ�*/


		//��ʼ��������
		std::vector<char>::iterator tmp_iterStart = tmpFileBuffer.begin();
		std::vector<char>::iterator tmp_iterEnd	 = tmpFileBuffer.begin();


		//����80�ֽ�STL�ļ���Ϣ ,�õ�������Ϊvector�����ʾ��
		tmp_iterEnd += 80;


		//vector.assign()
		//������[first, last)��Ԫ�ظ�ֵ����ǰ��vector�����У����߸�n��ֵΪx��Ԫ�ص�vector�����У�
		//��������������vector��������ǰ�����ݡ�
		if(pFileInfo)
		{
			pFileInfo->assign(tmp_iterStart, tmp_iterEnd);
		}


		//׼����4�ֽ������θ���
		int triangleCount = 0;

		//.................�����θ���
		triangleCount = mFunction_Combine4CharIntoInt(
			tmpFileBuffer.at(80), tmpFileBuffer.at(81), tmpFileBuffer.at(82), tmpFileBuffer.at(83));


		//��ʼ������ȡÿ������ķ���(float)
		int i = 0, j = 0;
		float tmpF = 0.0f;
		NVECTOR3 tmpVec3(0,0,0);
		char c1 = 0, c2 = 0, c3 = 0, c4 = 0;
		const int baseOffset = 84;
		//�����εı���
		for ( i = 0;i < triangleCount;i++)
		{
			//һ���������ڶ���ı���
			for (j = 0;j < 4; j++)
			{
					//50 * i����Ϊÿ��������ռ��50���ֽ�, j*12 ����Ϊһ������ռ12�ֽ�
					//ע��3dmax���굽dx�����ת��
					c1 = tmpFileBuffer[baseOffset + 50 * i + j * 12 +  0];
					c2 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 1];
					c3 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 2];
					c4 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 3];
					tmpVec3.x = mFunction_Combine4CharIntoFloat(c1, c2, c3, c4);

					c1 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 4];
					c2 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 5];
					c3 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 6];
					c4 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 7];
					tmpVec3.z = mFunction_Combine4CharIntoFloat(c1, c2, c3, c4);

					c1 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 8];
					c2 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 9];
					c3 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 10];
					c4 = tmpFileBuffer[baseOffset + 50 * i + j * 12 + 11];
					tmpVec3.y = mFunction_Combine4CharIntoFloat(c1, c2, c3, c4);

					//�ж����ڼ��ص��Ƕ��㻹�Ƿ���
					switch ( j )
					{

					// j =0 ���ص��Ƿ��ߣ���һ��12�ֽڣ�
					case 0:
						if (pNormalBuffer)
						{
							pNormalBuffer->push_back(tmpVec3);
						}
						break;

					// j = 1��2��3���ص��Ƕ��㣨��2��3��4��12�ֽڣ�
					default:
						pVertexBuffer->push_back(tmpVec3);
						pIndexBuffer->push_back(3 * i + j - 1);
						break;
					}
			}//�㶨һ�������� next����һ��50�ֽ�blobk
		}

	return TRUE;
}




/***********************************************************************
											PRIVATE
***********************************************************************/


float NoiseFileManager::mFunction_Combine4CharIntoFloat(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//�����и��޿ӵģ���˵����������ִ��棬�Ǵӵ�λ��ʼ��
	//Ҳ����˵��С��λ����ߣ����λ���ұ�
	int tmpInt = static_cast<int>(c1) | static_cast<int>(c2) << 8 | static_cast<int>(c3) << 16 | static_cast<int>(c4) << 24;
	float tmpFloat =0.0f	;

	//intתfloat��Ҫ����....�Ҷ�������֪��Static_cast��û����ʲô�ֽţ�reinterpret��֪��Ϊʲô�ò���
	memcpy(&tmpFloat, &tmpInt, 4);
	return tmpFloat;
}

int NoiseFileManager::mFunction_Combine4CharIntoInt(unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4)
{

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//�����и��޿ӵģ���˵����������ִ��棬�Ǵӵ�λ��ʼ��
	//Ҳ����˵��С��λ����ߣ����λ���ұ�
	int tmpInt = static_cast<int>(c1) | static_cast<int>(c2) << 8 | static_cast<int>(c3) << 16 | static_cast<int>(c4) << 24;
	return tmpInt;
}
;
