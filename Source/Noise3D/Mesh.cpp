
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

	m_pVB_Gpu = nullptr;
	m_pIB_Gpu = nullptr;
};

IMesh::~IMesh()
{
	ReleaseCOM(m_pVB_Gpu);
	ReleaseCOM(m_pIB_Gpu);
}

void IMesh::SetMaterial(N_UID matName)
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

void IMesh::SetPosition(const NVECTOR3 & pos)
{
	*m_pPosition = pos;
}

NVECTOR3 IMesh::GetPosition()
{
	return *m_pPosition;
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

void IMesh::RotateX_Pitch(float angleX)
{
	mRotationX_Pitch += angleX;
}

void IMesh::RotateY_Yaw(float angleY)
{
	mRotationY_Yaw += angleY;
}

void IMesh::RotateZ_Roll(float angleZ)
{
	mRotationZ_Roll += angleZ;
}

float IMesh::GetRotationX_Pitch()
{
	return mRotationX_Pitch;
}

float IMesh::GetRotationY_Yaw()
{
	return mRotationY_Yaw;
}

float IMesh::GetRotationZ_Roll()
{
	return mRotationZ_Roll;
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
//this function could be externally invoked by ModelLoader..etc
BOOL IMesh::mFunction_UpdateDataToVideoMem(const std::vector<N_DefaultVertex>& targetVB, const std::vector<UINT>& targetIB)
{
	//check if buffers have been created
	ReleaseCOM(m_pVB_Gpu);
	m_pVB_Mem->clear();
	ReleaseCOM(m_pIB_Gpu);
	m_pIB_Mem->clear();

	//this function could be externally invoked by ModelLoader..etc
	*m_pVB_Mem = std::move(targetVB);
	*m_pIB_Mem = std::move(targetIB);


#pragma region CreateGpuBuffers
	//Prepare to update to video memory, fill in SUBRESOURCE description structure
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem->at(0);
	mVertexCount = m_pVB_Mem->size();

	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem->at(0);
	mIndexCount = m_pIB_Mem->size();

	//------Create VERTEX BUFFER
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = sizeof(N_DefaultVertex)* mVertexCount;
	vbd.Usage = D3D11_USAGE_DEFAULT;//�����GPU�ܶ����д,IMMUTABLE��GPUֻ��
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0; //CPUɶ���ɲ���  D3D_USAGE
	vbd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE �����MSDN
	vbd.StructureByteStride = 0;

	//Create Buffers
	int hr = 0;
	hr = g_pd3dDevice11->CreateBuffer(&vbd, &tmpInitData_Vertex, &m_pVB_Gpu);
	HR_DEBUG(hr, "VERTEX BUFFER����ʧ��");


	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(int) * mIndexCount;
	ibd.Usage = D3D11_USAGE_DEFAULT;//�����GPU�ܶ����д,IMMUTABLE��GPUֻ��
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0; //CPUɶ���ɲ���  D3D_USAGE
	ibd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE �����MSDN
	ibd.StructureByteStride = 0;

	//Create Buffers
	hr = g_pd3dDevice11->CreateBuffer(&ibd, &tmpInitData_Index, &m_pIB_Gpu);
	HR_DEBUG(hr, "INDEX BUFFER����ʧ��");

#pragma endregion CreateGpuBuffers

	return TRUE;
};

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
