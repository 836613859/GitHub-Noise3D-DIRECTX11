
/***********************************************************************

                           �ࣺNOISE File Manager

			�����������ļ�������import export ����decode encode

************************************************************************/
#include "Noise3D.h"
/*
ios::in 		��
ios::out		д
ios::app		���ļ�ĩβ��ʼд
ios::binary       ������ģʽ
ios::nocreate	��һ���ļ�ʱ������ļ������ڣ��������ļ���
ios::noreplace	��һ���ļ�ʱ������ļ������ڣ��������ļ�
ios::trunc		��һ���ļ���Ȼ���������
ios::ate		��һ���ļ�ʱ����λ���ƶ����ļ�β
*/

NoiseFileManager::NoiseFileManager()
{

};

BOOL NoiseFileManager::ImportFile_PURE(char * pFilePath, std::vector<char>* pFileBuffer)
{
	if (!pFileBuffer)
	{
		return FALSE;
	}

	//�ļ�������
	std::ifstream fileIn(pFilePath, std::ios::binary);

	//�ļ������ھ�return
	if (!fileIn.is_open()) 
	{
		DEBUG_MSG1("NoiseFileManager : Cannot Open File !!");
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

	//allocate new memory block , initialized with 0
	pFileBuffer->resize(fileSize,0);
	while (!fileIn.eof())
	{
		
		fileIn.read(&pFileBuffer->at(0), fileSize);
		//���ֽڶ�ȡ
		//fileIn.get(tmpC);
		//pFileBuffer->push_back(tmpC);
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
		DEBUG_MSG1( "Noise File Manager :  Load Pure File Failed !! ");
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

BOOL NoiseFileManager::ImportFile_NOISELAYER(char * pFilePath, std::vector<N_LineStrip>* pLineStripBuffer)
{
	if (!pLineStripBuffer)
	{
		return FALSE;
	}

	//�ļ�������
	std::ifstream fileIn(pFilePath, std::ios::binary);

	//�ļ������ھ�return

	if (!fileIn.is_open())
	{
		DEBUG_MSG1("NoiseFileManager : Cannot Open File !!");
		return FALSE;
	}

	//ָ���Ƶ��ļ�β
	fileIn.seekg(0, std::ios_base::end);

	//ָ��ָ���ļ�β����ǰλ�þ��Ǵ�С
	int fileSize = (int)fileIn.tellg();


	//ָ���Ƶ��ļ�ͷ
	fileIn.seekg(0, std::ios_base::beg);


	//some  check before importing file
	if (!pLineStripBuffer)
	{
		return FALSE;
	}


	//first import the count data of line strip
	UINT magicNum = 0;
	UINT versionID = 0;
	UINT lineStripCount=0;
	UINT currLineStripPointCount = 0;
	UINT currLIneStripNormalCount = 0;
	UINT layerID = 0;
	NVECTOR3 tmpV;
	N_LineStrip  emptyLineStrip;
	UINT i = 0, j = 0;
	

#define STREAM_READ(STREAM,OBJECT) STREAM.read((char*)&(OBJECT),sizeof(OBJECT));

	//file head
	STREAM_READ(fileIn, magicNum);

	STREAM_READ(fileIn, versionID);
	//.........how many line strips
	STREAM_READ(fileIn, lineStripCount);


	//start to read line strip
	for (i = 0;i < lineStripCount;i++)
	{
		//we can push an empty line strip at the back , but didn't specify a layerID
		//because we no longer need (it's used for optimization)
		pLineStripBuffer->push_back(emptyLineStrip);

		STREAM_READ(fileIn, layerID);
		pLineStripBuffer->at(i).LayerID = layerID;

		STREAM_READ(fileIn, currLineStripPointCount);

		STREAM_READ(fileIn, currLIneStripNormalCount);

	
		//input Points of a line strip
		for (j = 0;j < currLineStripPointCount;j++)
		{
			STREAM_READ(fileIn, tmpV);
			pLineStripBuffer->at(i).pointList.push_back(tmpV);
		}

		//input normals of line segments
		for (j = 0;j < currLIneStripNormalCount;j++)
		{
			STREAM_READ(fileIn, tmpV)
			pLineStripBuffer->at(i).normalList.push_back(tmpV);
		}

	}

	return TRUE;
}

BOOL NoiseFileManager::ExportFile_PURE(char * pFilePath, std::vector<char>* pFileBuffer, BOOL canOverlapOld)
{

	std::ofstream fileOut;

	//can we overlap the old file??
	if(canOverlapOld)
	{
		fileOut.open(pFilePath, std::ios::binary | std::ios::trunc);
	}
	else
	{
		fileOut.open(pFilePath, std::ios::binary | std::ios::app);
	}

	//check if we have successfully opened the file
	if (!fileOut.good())
	{
		DEBUG_MSG1("NoiseFileManager : Cannot Open File !!");
		return FALSE;
	}

	//...........
	UINT i = 0;char tmpC = 0;
	for (i = 0;i < pFileBuffer->size();i++)
	{
		fileOut.put(pFileBuffer->at(i));
	}

	//�ر��ļ�
	fileOut.close();

	return TRUE;
}

BOOL NoiseFileManager::ExportFile_NOISELAYER(char * pFilePath, std::vector<N_LineStrip>* pLineStripBuffer, BOOL canOverlapOld)
{
	std::ofstream fileOut;

	//can we overlap the old file??
	if (canOverlapOld)
	{
		fileOut.open(pFilePath, std::ios::binary | std::ios::trunc);
	}
	else
	{
		fileOut.open(pFilePath, std::ios::binary | std::ios::app);
	}

	//check if we have successfully opened the file
	if (!fileOut.good())
	{
		DEBUG_MSG1("NoiseFileManager : Cannot Open File !!");
		return FALSE;
	}

	//prepare to output,tmp var to store number
	UINT i = 0, j = 0;

	/*
	FORMAT: 
	4 byte magicNum
	4 byte versionID
	4 byte to store Line Strip Count
	and for every Line Strip :
		first		4 byte for pointList.size()
		then		4 byte for normalList.size(), but it's actually  normalList.size()-1 ,and this is a reminder that
					there are normal data to be read 
		then		4 (float) * 3 (vec3 component) *( n + n-1) byte for a whole line strip(vertex + normal)

		keep writing data until all line strip are traversed
	*/

	//convert variables into char* to directly write in a file
#define STREAM_WRITE(STREAM,OBJECT)  {(STREAM).write((char *)&(OBJECT),sizeof(OBJECT));}

	//	first 4 byte for magic number
	char magicNum[] = { 'k','A','s','T' };
	STREAM_WRITE(fileOut, magicNum);

	//	4 byte for version
	UINT32 version = 0xffffff01;
	STREAM_WRITE(fileOut, version);

	//	 4 byte for line strip count
	UINT32 lineStripCount = pLineStripBuffer->size();
	STREAM_WRITE(fileOut, lineStripCount);



	//for every line strip
	for (i = 0;i < pLineStripBuffer->size();i++)
	{
		UINT layerID = pLineStripBuffer->at(i).LayerID;
		STREAM_WRITE(fileOut, layerID);

		//first output points count of current line strip 
		UINT pointListSize = pLineStripBuffer->at(i).pointList.size();
		STREAM_WRITE(fileOut, pointListSize);

		//then normals
		UINT normalListSize = pLineStripBuffer->at(i).normalList.size();
		STREAM_WRITE(fileOut, normalListSize);

		//and traverse every vertices
		for (j = 0;j < pLineStripBuffer->at(i).pointList.size(); j++)
		{
			NVECTOR3 tmpVertex = pLineStripBuffer->at(i).pointList.at(j);
			STREAM_WRITE(fileOut, tmpVertex);
		}

		//and traverse every normal
		for (j = 0;j < pLineStripBuffer->at(i).normalList.size(); j++)
		{
			NVECTOR3 tmpNormal = pLineStripBuffer->at(i).normalList.at(j);
			STREAM_WRITE(fileOut, tmpNormal);
		}
	}

	fileOut.close();

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

void NoiseFileManager::mFunction_SplitFloatInto4Char(float fValue, unsigned char & c1, unsigned char & c2, unsigned char & c3, unsigned char & c4)
{
	int tmpI = 0;
	memcpy(&tmpI, &fValue, 4);
	c1 =	(char)	((tmpI & 0x000000FF) );
	c2 = (char)	((tmpI & 0x0000FF00) >> 8);
	c3 = (char)	((tmpI & 0x00FF0000) >> 16);
	c4 = (char)	((tmpI & 0xFF000000)>>24);
}

void NoiseFileManager::mFunction_SplitIntInto4Char(int iValue, unsigned char & c1, unsigned char & c2, unsigned char & c3, unsigned char & c4)
{
	c1 = (char)((iValue & 0x000000FF));
	c2 = (char)((iValue & 0x0000FF00) >> 8);
	c3 = (char)((iValue & 0x00FF0000) >> 16);
	c4 = (char)((iValue & 0xFF000000) >> 24);
}


