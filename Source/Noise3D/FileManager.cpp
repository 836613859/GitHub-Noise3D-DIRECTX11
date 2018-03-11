
/***********************************************************************

                           �ࣺNOISE File Manager

			�����������ļ���������ͬ�ļ����Ƿ��ڲ�ͬ��cppʵ�ְɣ�
			��Զ���һ��

************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

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

IFileIO::IFileIO()
{

};

bool IFileIO::ImportFile_PURE(NFilePath pFilePath, std::vector<char>& byteBuffer)
{
	std::ifstream fileIn(pFilePath, std::ios::binary);

	if (!fileIn.is_open()) 
	{
		ERROR_MSG("FileManager : Cannot Open File !!");
		return false;
	}

	//file pointer move to eof
	fileIn.seekg(0, std::ios_base::end);

	//get file size
	int static_fileSize = (int)fileIn.tellg();

	//file pointer move to the beginning
	fileIn.seekg(0, std::ios_base::beg);

	//...........
	int i = 0;char tmpC =0;

	//allocate new memory block , initialized with 0
	byteBuffer.resize(static_fileSize,0);
	while (!fileIn.eof())
	{
		fileIn.read(&byteBuffer.at(0), static_fileSize);
	}

	fileIn.close();

	return true;
}

bool IFileIO::ExportFile_PURE(NFilePath pFilePath, std::vector<char>* pFileBuffer, bool canOverlapOld)
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
		ERROR_MSG("FileManager : Cannot Open File !!");
		return false;
	}

	//...........
	UINT i = 0;char tmpC = 0;
	for (i = 0;i < pFileBuffer->size();i++)
	{
		fileOut.put(pFileBuffer->at(i));
	}

	//�ر��ļ�
	fileOut.close();

	return true;
}
