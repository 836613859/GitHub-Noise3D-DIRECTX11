
/***********************************************************************

							class��NoiseMesh

				Desc: encapsule a Mesh class that can be 

***********************************************************************/


#include "Noise3D.h"

using namespace Noise3D;

static UINT c_VBstride_Default = sizeof(N_DefaultVertex);		//VertexBuffer��ÿ��Ԫ�ص��ֽڿ��
static UINT c_VBoffset = 0;				//VertexBuffer�������ƫ�� ��Ϊ��ͷ��ʼ����offset��0

IMesh::IMesh()
{
	mVertexCount	= 0;
	mIndexCount	= 0;
	mRotationX_Pitch = 0.0f;
	mRotationY_Yaw = 0.0f;
	mRotationZ_Roll = 0.0f;
	mScaleX = 1.0f;
	mScaleY = 1.0f;
	mScaleZ = 1.0f;

	m_pMatrixWorld = new NMATRIX;
	m_pMatrixWorldInvTranspose = new NMATRIX;
	m_pPosition = new NVECTOR3(0, 0, 0);
	mBoundingBox.min = NVECTOR3(0, 0, 0);
	mBoundingBox.max = NVECTOR3(0, 0, 0);
	D3DXMatrixIdentity(m_pMatrixWorld);
	D3DXMatrixIdentity(m_pMatrixWorldInvTranspose);

	m_pVB_Mem		= new std::vector<N_DefaultVertex>;
	m_pIB_Mem			= new std::vector<UINT>;
	m_pSubsetInfoList		= new std::vector<N_MeshSubsetInfo>;//store [a,b] of a subset
};

IMesh::~IMesh()
{
	ReleaseCOM(m_pVB_Gpu);
	ReleaseCOM(m_pIB_Gpu);
};

void	IMesh::CreatePlane(float fWidth,float fDepth,UINT iRowCount,UINT iColumnCount)
{
	//check if the input "Step Count" is illegal
	if(iColumnCount <= 2)	{iColumnCount =2;}
	if(iRowCount <= 2)		{iRowCount = 2;}

	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	//delegate vert/idx creation duty to MeshGenerator 
	mMeshGenerator.CreatePlane(fWidth, fDepth, iRowCount, iColumnCount, *m_pVB_Mem, *m_pIB_Mem);

	//Prepare to update to GPU
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex,sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index,sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//���
	mFunction_CreateGpuBuffers( &tmpInitData_Vertex ,mVertexCount,&tmpInitData_Index,mIndexCount);

	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);
};

void IMesh::CreateBox(float fWidth,float fHeight,float fDepth,UINT iDepthStep,UINT iWidthStep,UINT iHeightStep)
{
	//If the user has create sth before,then we will destroy the former
	//VB in order to create a new size buffer
	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	//mesh creation delegate to MeshGenerator
	mMeshGenerator.CreateBox(fWidth, fHeight, fDepth, iDepthStep, iWidthStep, iHeightStep, *m_pVB_Mem, *m_pIB_Mem);

	//Prepare to Create Gpu Buffers
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex,sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index,sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//transmit to gpu
	mFunction_CreateGpuBuffers( &tmpInitData_Vertex ,mVertexCount,&tmpInitData_Index,mIndexCount);

	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);
}

void	IMesh::CreateSphere(float fRadius,UINT iColumnCount, UINT iRingCount)
{
	//check if the input "Step Count" is illegal
	if(iColumnCount <= 3)	{iColumnCount =3;}
	if(iRingCount <= 1)		{iRingCount = 1;}

	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	//mesh creation delegate to MeshGenerator
	mMeshGenerator.CreateSphere(fRadius, iColumnCount, iRingCount, *m_pVB_Mem, *m_pIB_Mem);

	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();//(iColumnCount+1) * iRingCount * 2 *3

	//���
	mFunction_CreateGpuBuffers( &tmpInitData_Vertex ,mVertexCount,&tmpInitData_Index,mIndexCount);
	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);

};

void IMesh::CreateCylinder(float fRadius,float fHeight,UINT iColumnCount,UINT iRingCount)
{
		//check if the input "Step Count" is illegal
	if(iColumnCount <= 3)	{iColumnCount =3;}
	if(iRingCount <= 2)		{iRingCount = 2;}

	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	//mesh creation delegate to MeshGenerator
	mMeshGenerator.CreateCylinder(fRadius,fHeight, iColumnCount, iRingCount, *m_pVB_Mem, *m_pIB_Mem);

	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();//(iColumnCount+1) * iRingCount * 2 *3


	//...
	mFunction_CreateGpuBuffers( &tmpInitData_Vertex ,mVertexCount,&tmpInitData_Index,mIndexCount);
	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);

};

BOOL IMesh::LoadFile_STL(NFilePath pFilePath)
{
	//check if buffers have been created
	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();


	std::vector<NVECTOR3> tmpVertexList;
	std::vector<NVECTOR3> tmpNormalList;
	std::string				tmpInfo;
	N_DefaultVertex	tmpCompleteV;
	NVECTOR3			tmpBoundingBoxCenter(0, 0, 0);

	//����STL
	BOOL fileLoadSucceeded = FALSE;
	fileLoadSucceeded=IFileManager::ImportFile_STL(pFilePath, tmpVertexList, *m_pIB_Mem, tmpNormalList, tmpInfo);
	if (!fileLoadSucceeded)
	{
		ERROR_MSG("Noise Mesh : Load STL failed!");
		return FALSE;
	}

	//�ȼ����Χ�У����������������ĵ㣨��һ����Mesh Space��ԭ�㣩
	mFunction_ComputeBoundingBox(&tmpVertexList);

	//�����Χ�����ĵ�
	tmpBoundingBoxCenter = NVECTOR3(
		(mBoundingBox.max.x + mBoundingBox.min.x) / 2.0f,
		(mBoundingBox.max.y + mBoundingBox.min.y) / 2.0f,
		(mBoundingBox.max.z + mBoundingBox.min.z) / 2.0f);



	UINT i = 0;UINT k = 0;
	for (i = 0;i < tmpVertexList.size();i++)
	{
		tmpCompleteV.Color = NVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		tmpCompleteV.Pos = tmpVertexList.at(i);
		tmpCompleteV.Normal = tmpNormalList.at(k);
		tmpCompleteV.Tangent = NVECTOR3(-tmpCompleteV.Normal.z, 0, tmpCompleteV.Normal.x);//mighty tangent algorithm= =
		tmpCompleteV.TexCoord = mFunction_ComputeTexCoord_SphericalWrap(tmpBoundingBoxCenter, tmpCompleteV.Pos);
		m_pVB_Mem->push_back(tmpCompleteV);

		//ÿ������һ��������3������ ��Ҫ�ֵ��¸������εķ�����
		if (i % 3 == 2) { k++; }
	}


	//Prepare to update to GPU
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//���
	mFunction_CreateGpuBuffers(&tmpInitData_Vertex, mVertexCount, &tmpInitData_Index, mIndexCount);

	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);

	return TRUE;
}

BOOL IMesh::LoadFile_OBJ(NFilePath pFilePath)
{
	//check if buffers have been created
	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu); 
	m_pIB_Mem->clear();


	std::vector<N_DefaultVertex> tmpCompleteVertexList;
	std::vector<UINT> tmpNormalList;

	//����STL
	BOOL fileLoadSucceeded = FALSE;
	fileLoadSucceeded = IFileManager::ImportFile_OBJ(pFilePath, *m_pVB_Mem, *m_pIB_Mem);
	if (!fileLoadSucceeded)
	{
		ERROR_MSG("Noise Mesh : Load OBJ failed!");
		return FALSE;
	}


	//Prepare to update to GPU
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//���
	mFunction_CreateGpuBuffers(&tmpInitData_Vertex, mVertexCount, &tmpInitData_Index, mIndexCount);

	//user-set material
	SetMaterial(NOISE_MACRO_DEFAULT_MATERIAL_NAME);

	return TRUE;
}

BOOL IMesh::LoadFile_3DS(NFilePath pFilePath)
{
	//check if buffers have been created
	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	std::vector<NVECTOR3>		verticesList;
	std::vector<NVECTOR2>		texCoordList;
	std::vector<UINT>				indicesList;
	std::vector<N_MeshSubsetInfo> subsetList;
	std::vector<N_Material>		materialList;
	std::unordered_map<std::string, std::string> texName2FilePathMap;

	//import data
	BOOL importSucceeded = FALSE;
	importSucceeded= IFileManager::ImportFile_3DS(
		pFilePath,
		verticesList, 
		texCoordList,
		indicesList, 
		subsetList, 
		materialList,
		texName2FilePathMap);

	if (!importSucceeded)
	{
		ERROR_MSG("Load 3ds : Import Operation Failed!!");
		return FALSE;
	}

#pragma region Index/VertexData

	//to compute vertex normal ,we should generate adjacent information of vertices first.
	//thus "vertexNormalList" holds the sum of face normal (the triangle is adjacent to corresponding vertex)
	std::vector<NVECTOR3>	vertexNormalList(verticesList.size(),NVECTOR3(0,0,0));
	std::vector<NVECTOR3>	vertexTangentList;

	//1. compute vertex normal of faces
	for (UINT i = 0;i < indicesList.size();i += 3)
	{
		//compute face normal
		uint16_t idx1 = indicesList.at(i);
		uint16_t idx2 = indicesList.at(i + 1);
		uint16_t idx3 = indicesList.at(i + 2);
		NVECTOR3 v1 = verticesList.at(idx1);
		NVECTOR3 v2 = verticesList.at(idx2);
		NVECTOR3 v3 = verticesList.at(idx3);
		NVECTOR3 vec1 = v2 - v1;
		NVECTOR3 vec2 = v3 - v1;
		NVECTOR3 faceNormal(0,0.0f,0);
		//D3DXVec3Normalize(&vec1, &vec1);
		//D3DXVec3Normalize(&vec2, &vec2);
		D3DXVec3Cross(&faceNormal, &vec1, &vec2);

		//add face normal to vertex normal
		vertexNormalList.at(idx1) += faceNormal;
		vertexNormalList.at(idx2) += faceNormal;
		vertexNormalList.at(idx3) += faceNormal;
	}

	//2.normalize all vertex normal
	//3.by the way, compute vertex tangent
	vertexTangentList.reserve(verticesList.size());
	for (auto& vn:vertexNormalList)
	{
		D3DXVec3Normalize(&vn, &vn);

		//compute Tangent
		NVECTOR3 tmpTangent(1, 0, 0);
		NVECTOR3 tmpVec(-vn.z, 0, vn.x);
		D3DXVec3Cross(&tmpTangent, &vn, &tmpVec);
		D3DXVec3Normalize(&tmpTangent, &tmpTangent);

		//one vertex for one tangent
		vertexTangentList.push_back(tmpTangent);
	}
	
	//generate complete vertices
	std::vector<N_DefaultVertex> completeVertexList;
	completeVertexList.reserve(verticesList.size());
	for (UINT i = 0;i < verticesList.size();i++)
	{
		N_DefaultVertex tmpCompleteVertex;

		try
		{
			tmpCompleteVertex.Color = NVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
			tmpCompleteVertex.Normal = vertexNormalList.at(i);
			tmpCompleteVertex.Pos = verticesList.at(i);
			tmpCompleteVertex.Tangent = vertexTangentList.at(i);
			tmpCompleteVertex.TexCoord = texCoordList.at(i);

			completeVertexList.push_back(tmpCompleteVertex);
		}
		catch(std::out_of_range)
		{
			//theoretically , all buffer should have the same total elements count
			//but if shit happens,we must stop this.
			ERROR_MSG("Load File 3DS: WARNING : data could be damaged!!");
			break;
		}
	}

	//.....................
	*m_pIB_Mem = std::move(indicesList);
	*m_pVB_Mem = std::move(completeVertexList);

#pragma endregion

#pragma region material&TextureData
	//subsets...
	*m_pSubsetInfoList = std::move(subsetList);

	//!!!!!materials have not been created. The Creation of materials will be done
	//by current Scene Tex&Mat Manager	
	IMaterialManager* pMatMgr = GetRoot()->GetScenePtr()->GetMaterialMgr(); //m_pChildMaterialMgr;
	ITextureManager* pTexMgr = GetRoot()->GetScenePtr()->GetTextureMgr();//m_pFatherScene->m_pChildTextureMgr;

	//Get the directory where the file locates
	std::string modelFileDirectory = GetFileDirectory(pFilePath);

	if (pTexMgr!=nullptr && pMatMgr!=nullptr)
	{
		for (UINT i = 0;i < materialList.size();i++)
		{
			//----------Create Material-----------
			UINT matReturnID = pMatMgr->CreateMaterial(materialList.at(i));
			if(matReturnID==NOISE_MACRO_INVALID_MATERIAL_ID)
				ERROR_MSG("WARNING : Load 3ds : Material Creation Failed!!");

			//----------Create Texture Maps----------
			std::string& diffMapName = materialList.at(i).diffuseMapName;
			std::string& normalMapName = materialList.at(i).normalMapName;
			std::string& specMapName = materialList.at(i).specularMapName;

			//Define a temp lambda function for texture creations
			auto lambdaFunc_CreateTexture = [&](std::string& texName)
			{
				if (pTexMgr->GetTextureID(texName) == NOISE_MACRO_INVALID_TEXTURE_ID)
				{
					if (texName != "")
					{
						UINT textureReturnID = NOISE_MACRO_INVALID_TEXTURE_ID;

						//locate the directory to generate file path
						std::string mapPath = modelFileDirectory + texName2FilePathMap.at(texName);

						//Try to create texture
						textureReturnID = pTexMgr->CreateTextureFromFile(
							mapPath.c_str(),
							texName,
							TRUE, 0, 0, FALSE);

						if (textureReturnID == NOISE_MACRO_INVALID_TEXTURE_ID)
							ERROR_MSG("WARNING : Load 3ds : texture Creation Failed!!");
					}
				}
			};

			//DIFFUSE MAP
			lambdaFunc_CreateTexture(diffMapName);

			//NORMAL MAP
			lambdaFunc_CreateTexture(normalMapName);

			//SPECULAR MAP
			lambdaFunc_CreateTexture(specMapName);
		}
	}

#pragma endregion

#pragma region CreateGpuBuffer
	//Prepare to update to GPU
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//���
	mFunction_CreateGpuBuffers(&tmpInitData_Vertex, mVertexCount, &tmpInitData_Index, mIndexCount);

#pragma endregion

	return TRUE;
}

void IMesh::SetMaterial(std::string matName)
{
	N_MeshSubsetInfo tmpSubset;
	tmpSubset.startPrimitiveID = 0;
	tmpSubset.primitiveCount = mIndexCount/3;//count of triangles
	tmpSubset.matName = matName;

	//because this SetMaterial aim to the entire mesh (all primitives) ,so
	//previously-defined material will be wiped,and set to this material
	m_pSubsetInfoList->clear();
	m_pSubsetInfoList->push_back(tmpSubset);
}

void IMesh::SetPosition(float x,float y,float z)
{
	m_pPosition->x =x;
	m_pPosition->y =y;
	m_pPosition->z =z;
}

void Noise3D::IMesh::SetPosition(const NVECTOR3 & pos)
{
	*m_pPosition = pos;
}

void IMesh::SetRotation(float angleX, float angleY, float angleZ)
{
	mRotationX_Pitch	= angleX;
	mRotationY_Yaw		= angleY;
	mRotationZ_Roll		= angleZ;
}

void IMesh::SetRotationX_Pitch(float angleX)
{
	mRotationX_Pitch = angleX;
};

void IMesh::SetRotationY_Yaw(float angleY)
{
	mRotationY_Yaw = angleY;
};

void IMesh::SetRotationZ_Roll(float angleZ)
{
	mRotationZ_Roll = angleZ;
}

void IMesh::SetScale(float scaleX, float scaleY, float scaleZ)
{
	mScaleX = scaleX;
	mScaleY = scaleY;
	mScaleZ = scaleZ;
}

void IMesh::SetScaleX(float scaleX)
{
	mScaleX = scaleX;
}

void IMesh::SetScaleY(float scaleY)
{
	mScaleY = scaleY;
}

void IMesh::SetScaleZ(float scaleZ)
{
	mScaleZ = scaleZ;
}

UINT IMesh::GetVertexCount()
{
	return m_pVB_Mem->size();
}

void IMesh::GetVertex(UINT iIndex, N_DefaultVertex& outVertex)
{
	if (iIndex < m_pVB_Mem->size())
	{
		outVertex = m_pVB_Mem->at(iIndex);
	}
}

void IMesh::GetVertexBuffer(std::vector<N_DefaultVertex>& outBuff)
{
	std::vector<N_DefaultVertex>::iterator iterBegin, iterLast;
	iterBegin = m_pVB_Mem->begin();
	iterLast = m_pVB_Mem->end();
	outBuff.assign(iterBegin,iterLast);
}

void IMesh::GetWorldMatrix(NMATRIX & outWorldMat, NMATRIX& outWorldInvTMat)
{
	mFunction_UpdateWorldMatrix();
	outWorldMat = *m_pMatrixWorld;
	outWorldInvTMat = *m_pMatrixWorldInvTranspose;
}

N_Box IMesh::ComputeBoundingBox()
{
	mFunction_ComputeBoundingBox();

	return mBoundingBox;
};

/***********************************************************************
								PRIVATE					                    
***********************************************************************/

BOOL IMesh::mFunction_CreateGpuBuffers
	(	D3D11_SUBRESOURCE_DATA* pVertexDataInMem,int iVertexCount,D3D11_SUBRESOURCE_DATA* pIndexDataInMem,int iIndexCount)
{
	//Create VERTEX BUFFER
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth =  sizeof(N_DefaultVertex)* iVertexCount;
	vbd.Usage = D3D11_USAGE_DEFAULT;//�����GPU�ܶ����д,IMMUTABLE��GPUֻ��
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0; //CPUɶ���ɲ���  D3D_USAGE
	vbd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE �����MSDN
	vbd.StructureByteStride = 0;

	//Create Buffers
	int hr =0;
	hr = g_pd3dDevice11->CreateBuffer(&vbd,pVertexDataInMem,&m_pVB_Gpu);
	HR_DEBUG(hr,"VERTEX BUFFER����ʧ��");


	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(int) * iIndexCount;
	ibd.Usage = D3D11_USAGE_DEFAULT;//�����GPU�ܶ����д,IMMUTABLE��GPUֻ��
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0; //CPUɶ���ɲ���  D3D_USAGE
	ibd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE �����MSDN
	ibd.StructureByteStride = 0;

	//Create Buffers
	hr = g_pd3dDevice11->CreateBuffer(&ibd,pIndexDataInMem,&m_pIB_Gpu);
	HR_DEBUG(hr,"INDEX BUFFER����ʧ��");

	//ReleaseCOM(g_pd3dDevice11);
	return TRUE;
}
	
void	IMesh::mFunction_UpdateWorldMatrix()
{

	D3DXMATRIX	tmpMatrixScaling;
	D3DXMATRIX	tmpMatrixTranslation;
	D3DXMATRIX	tmpMatrixRotation;
	D3DXMATRIX	tmpMatrix;
	float tmpDeterminant;

	//��ʼ���������
	D3DXMatrixIdentity(&tmpMatrix);
		
	//���ž���
	D3DXMatrixScaling(&tmpMatrixScaling, mScaleX, mScaleY, mScaleZ);

	//��ת����(��meshҲ�ú����hhhhhh��
	D3DXMatrixRotationYawPitchRoll(&tmpMatrixRotation, mRotationY_Yaw, mRotationX_Pitch, mRotationZ_Roll);

	//ƽ�ƾ���
	D3DXMatrixTranslation(&tmpMatrixTranslation, m_pPosition->x, m_pPosition->y, m_pPosition->z);

	//�����ţ�����ת����ƽ�ƣ���viewMatrix�е�����
	D3DXMatrixMultiply(&tmpMatrix, &tmpMatrix, &tmpMatrixScaling);
	D3DXMatrixMultiply(&tmpMatrix,&tmpMatrix,&tmpMatrixRotation);
	D3DXMatrixMultiply(&tmpMatrix, &tmpMatrix, &tmpMatrixTranslation);
	*m_pMatrixWorld = tmpMatrix;

	//������ת��Normal��InvTranspose	��ΪҪTrans ֮������һ��Trans���ܸ��� ���ԾͿ���ʡ��
	D3DXMatrixInverse(m_pMatrixWorldInvTranspose,&tmpDeterminant,m_pMatrixWorld);

	//Update��GPUǰҪ��ת��
	D3DXMatrixTranspose(m_pMatrixWorld,m_pMatrixWorld);

	//WorldInvTranspose
	//D3DXMatrixTranspose(m_pMatrixWorldInvTranspose,&tmpMatrix);

}

void IMesh::mFunction_ComputeBoundingBox()
{
	//�����Χ��.......����1

	UINT i = 0;
	NVECTOR3 tmpV;

	//�������ж��㣬�����Χ��3�������� С/�� ����������
	for (i = 0;i < m_pVB_Mem->size();i++)
	{
		if (i == 0)
		{
			//initialization
			mBoundingBox.min = m_pVB_Mem->at(0).Pos;
			mBoundingBox.max = m_pVB_Mem->at(0).Pos;
		}
		//N_DEFAULT_VERTEX
		tmpV = m_pVB_Mem->at(i).Pos;
		if (tmpV.x <( mBoundingBox.min.x)) { mBoundingBox.min.x = tmpV.x; }
		if (tmpV.y <(mBoundingBox.min.y)) { mBoundingBox.min.y = tmpV.y; }
		if (tmpV.z <(mBoundingBox.min.z)) { mBoundingBox.min.z = tmpV.z; }

		if (tmpV.x >(mBoundingBox.max.x)) { mBoundingBox.max.x = tmpV.x; }
		if (tmpV.y >(mBoundingBox.max.y)) { mBoundingBox.max.y = tmpV.y; }
		if (tmpV.z >(mBoundingBox.max.z)) { mBoundingBox.max.z = tmpV.z; }
	}
	D3DXVec3Add(&mBoundingBox.max, &mBoundingBox.max, m_pPosition);
	D3DXVec3Add(&mBoundingBox.min, &mBoundingBox.min, m_pPosition);
}

void IMesh::mFunction_ComputeBoundingBox(std::vector<NVECTOR3>* pVertexBuffer)
{
	//�����Χ��.......����2

	UINT i = 0;
	NVECTOR3 tmpV;
	//�������ж��㣬�����Χ��3�������� С/�� ����������
	for (i = 0;i < pVertexBuffer->size();i++)
	{
		if (i == 0)
		{
			//initialization
			mBoundingBox.min = m_pVB_Mem->at(0).Pos;
			mBoundingBox.max = m_pVB_Mem->at(0).Pos;
		}
		tmpV = pVertexBuffer->at(i);
		//N_DEFAULT_VERTEX
		tmpV = m_pVB_Mem->at(i).Pos;
		if (tmpV.x <(mBoundingBox.min.x)) { mBoundingBox.min.x = tmpV.x; }
		if (tmpV.y <(mBoundingBox.min.y)) { mBoundingBox.min.y = tmpV.y; }
		if (tmpV.z <(mBoundingBox.min.z)) { mBoundingBox.min.z = tmpV.z; }

		if (tmpV.x >(mBoundingBox.max.x)) { mBoundingBox.max.x = tmpV.x; }
		if (tmpV.y >(mBoundingBox.max.y)) { mBoundingBox.max.y = tmpV.y; }
		if (tmpV.z >(mBoundingBox.max.z)) { mBoundingBox.max.z = tmpV.z; }
	}
	D3DXVec3Add(&mBoundingBox.max, &mBoundingBox.max, m_pPosition);
	D3DXVec3Add(&mBoundingBox.min, &mBoundingBox.min, m_pPosition);
}

inline NVECTOR2 IMesh::mFunction_ComputeTexCoord_SphericalWrap(NVECTOR3 vBoxCenter, NVECTOR3 vPoint)
{
	//��...����������򵥵��������ΰ���

	NVECTOR2 outTexCoord(0,0);
	NVECTOR3 tmpP= vPoint - vBoxCenter;

	//ͶӰ����λ����
	D3DXVec3Normalize(&tmpP, &tmpP);

	//�����Ǻ�����������ϵ���꣬Ȼ��Ƕ�ֵӳ�䵽[0,1]
	float angleYaw = 0.0f;
	float anglePitch = 0.0f;
	float tmpLength = sqrtf(tmpP.x*tmpP.x + tmpP.z*tmpP.z);

	// [ -PI/2 , PI/2 ]
	anglePitch = atan2(tmpP.y,tmpLength);

	// [ -PI	, PI ]
	angleYaw =	atan2(tmpP.z, tmpP.x);	

	//map to [0,1]
	outTexCoord.x = (angleYaw +  MATH_PI) / (2.0f * MATH_PI);
	outTexCoord.y = (anglePitch + (MATH_PI/2.0f) ) / MATH_PI;

	return outTexCoord;
};
