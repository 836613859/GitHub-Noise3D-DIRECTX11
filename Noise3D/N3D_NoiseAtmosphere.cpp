
/***********************************************************************

							类：NOISE Atmosphere

							简述：天空盒天空球雾 等


************************************************************************/

#include "Noise3D.h"

NoiseAtmosphere::NoiseAtmosphere()
{
	mFogEnabled = FALSE;
	mCanUpdateAmtosphere = FALSE;
	m_pFogColor = new NVECTOR3(1.0f, 1.0f, 1.0f);
	mFogNear = 10;
	mFogFar = 100;
	m_pVB_Mem_Sky	= new std::vector<N_SimpleVertex>;
	m_pIB_Mem_Sky = new std::vector<UINT>;
	m_pVB_Gpu_Sky	= nullptr;
	m_pIB_Gpu_Sky		= nullptr;
	mSkyDomeRadiusXZ =100;
	mSkyDomeHeight = 100;
	mSkyDomeTextureID = NOISE_MACRO_INVALID_TEXTURE_ID;

}

void NoiseAtmosphere::SelfDestruction()
{

};

BOOL NoiseAtmosphere::AddToRenderList()
{
	if (m_pFatherScene == NULL)
	{
		DEBUG_MSG1("NoiseScene Has Not Been Created!");
		return FALSE;
	}
	
	//scene是它的友元类；往Scene里管理的RenderList加上自己的指针
	m_pFatherScene->m_pChildRenderer->m_pRenderList_Atmosphere->push_back(this);

	//this sentence is to unify render command, that fog color will only be rendered after ADDTORENDERLIST();
	mFogHasBeenAddedToRenderList = TRUE;

	return TRUE;
}

void NoiseAtmosphere::SetFogEnabled(BOOL isEnabled)
{
	mFogEnabled = isEnabled;
	mCanUpdateAmtosphere = TRUE;
}

void NoiseAtmosphere::SetFogParameter(float fogNear, float fogFar, NVECTOR3 color)
{
	//perhaps i can skip checking the size comparison between NEAR & FAR

	if(fogFar > 1)mFogFar = fogFar;
	if (fogNear > 1)	mFogNear = fogNear;

	//validate distances
	if (fogNear >= fogFar)
	{
		mFogNear	= 10;
		mFogFar		= 100;
		DEBUG_MSG1("SetFog : fog Near/Far invalid");
	};

	//set color
	*m_pFogColor = color;

	mCanUpdateAmtosphere = TRUE;
}

BOOL NoiseAtmosphere::CreateSkyDome(float fRadiusXZ, float fHeight,UINT texID)
{
	//check if the input "Step Count" is illegal
	UINT iColumnCount		= 25;
	UINT iRingCount			= 30;
	UINT tmpVertexCount	= 0;
	UINT tmpIndexCount	= 0;

	//release existed gpu buffers
	if (m_pVB_Gpu_Sky != NULL) 
	{ 
		ReleaseCOM(m_pVB_Gpu_Sky);
		m_pVB_Mem_Sky->clear();
	}


	if (m_pIB_Gpu_Sky != NULL)
	{
		ReleaseCOM(m_pIB_Gpu_Sky);
		m_pIB_Mem_Sky->clear();
	}


	mSkyDomeTextureID = texID;

#pragma region GenerateVertex

	//iColunmCount : Slices of Columns (Cut up the ball Vertically)
	//iRingCount: Slices of Horizontal Rings (Cut up the ball Horizontally)
	//the "+2" refers to the TOP/BOTTOM vertex
	//the TOP/BOTTOM vertex will be restored in the last 2 position in this array
	//the first column will be duplicated to achieve adequate texture mapping
	NVECTOR3* tmpV;
	NVECTOR2* tmpTexCoord;
	tmpVertexCount = (iColumnCount + 1) * iRingCount + 2;
	tmpV = new NVECTOR3[tmpVertexCount];
	tmpTexCoord = new NVECTOR2[tmpVertexCount];
	tmpV[tmpVertexCount - 2] = NVECTOR3(NVECTOR3(0, fHeight, 0));			//TOP vertex
	tmpV[tmpVertexCount - 1] = NVECTOR3(NVECTOR3(0, -fHeight, 0));		//BOTTOM vertex
	tmpTexCoord[tmpVertexCount - 2] = NVECTOR2(0.5f, 0);			//TOP vertex
	tmpTexCoord[tmpVertexCount - 1] = NVECTOR2(0.5f, 1.0f);			//BOTTOM vertex

	//i,j will be used for iterating , and k will be the subscript
	UINT 	i = 0, j = 0, k = 0;
	float	tmpX, tmpY, tmpZ, tmpRingRadius;


	//Calculate the Step length (步长)
	float	StepLength_AngleY = MATH_PI / (iRingCount + 1); // distances between each level (ring)
	float StepLength_AngleXZ = 2 * MATH_PI / iColumnCount;


	//start to iterate
	for (i = 0;i < iRingCount;i++)
	{
		//Generate Vertices ring By ring ( from top to down )
		//the first column will be duplicated to achieve adequate texture mapping
		for (j = 0; j < iColumnCount + 1; j++)
		{
			//the Y coord of  current ring 
			tmpY = fHeight *sin(MATH_PI / 2 - (i + 1) *StepLength_AngleY);

			////Pythagoras theorem(勾股定理)
			tmpRingRadius = sqrtf(fRadiusXZ*fRadiusXZ - tmpY * tmpY);

			////trigonometric function(三角函数)
			tmpX = tmpRingRadius * cos(j*StepLength_AngleXZ);

			//...
			tmpZ = tmpRingRadius * sin(j*StepLength_AngleXZ);

			//...
			tmpV[k] = NVECTOR3(tmpX, tmpY, tmpZ);

			//map the i,j to closed interval [0,1] respectively , to proceed a spheric texture wrapping
			tmpTexCoord[k] = NVECTOR2((float)j / (iColumnCount), (float)i / (iRingCount - 1));

			k++;
		}
	}


	//add to Memory
	N_SimpleVertex tmpCompleteV;
	for (i = 0;i<tmpVertexCount;i++)
	{
		tmpCompleteV.Pos = tmpV[i];
		tmpCompleteV.Color = NVECTOR4(tmpV[i].x / fRadiusXZ, tmpV[i].y / fHeight, tmpV[i].z / fRadiusXZ, 1.0f);
		tmpCompleteV.TexCoord = tmpTexCoord[i];
		m_pVB_Mem_Sky->push_back(tmpCompleteV);
	}

	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &m_pVB_Mem_Sky->at(0);


#pragma endregion GenerateVertex

#pragma region GenerateIndex

	//Generate Indices of a ball
	//deal with the middle
	//every Ring grows a triangle net with lower level ring
	for (i = 0; i<iRingCount - 1; i++)
	{
		for (j = 0; j<iColumnCount; j++)
		{
			/*
			k	_____ k+1
			|    /
			|  /
			|/		k+2

			*/
			//+1是因为复制了第一列，比原来设好的列数多出一列
			m_pIB_Mem_Sky->push_back(i*			(iColumnCount + 1) + j + 0);
			m_pIB_Mem_Sky->push_back(i*			(iColumnCount + 1) + j + 1);
			m_pIB_Mem_Sky->push_back((i + 1)*	(iColumnCount + 1) + j + 0);

			/*
			k+3
			/|
			/  |
			k+5	/___|	k+4

			*/
			m_pIB_Mem_Sky->push_back(i*			(iColumnCount + 1) + j + 1);
			m_pIB_Mem_Sky->push_back((i + 1)*	(iColumnCount + 1) + j + 1);
			m_pIB_Mem_Sky->push_back((i + 1)*	(iColumnCount + 1) + j + 0);

		}
	}


	//deal with the TOP/BOTTOM

	for (j = 0;j<iColumnCount;j++)
	{
		m_pIB_Mem_Sky->push_back(j + 1);
		m_pIB_Mem_Sky->push_back(j);
		m_pIB_Mem_Sky->push_back(tmpVertexCount - 2);	//index of top vertex

		m_pIB_Mem_Sky->push_back((iColumnCount + 1)* (iRingCount - 1) + j);
		m_pIB_Mem_Sky->push_back((iColumnCount + 1) * (iRingCount - 1) + j + 1);
		m_pIB_Mem_Sky->push_back(tmpVertexCount - 1); //index of bottom vertex
	}


	D3D11_SUBRESOURCE_DATA tmpInitData_Index;
	ZeroMemory(&tmpInitData_Index, sizeof(tmpInitData_Index));
	tmpInitData_Index.pSysMem = &m_pIB_Mem_Sky->at(0);
	//a single Triangle
	tmpIndexCount = m_pIB_Mem_Sky->size();//(iColumnCount+1) * iRingCount * 2 *3

#pragma endregion GenerateIndex



	 //Create VERTEX BUFFER (GPU)
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = sizeof(N_DefaultVertex)* tmpVertexCount;
	vbd.Usage = D3D11_USAGE_DEFAULT;//这个是GPU能对其读写,IMMUTABLE是GPU只读
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0; //CPU啥都干不了  D3D_USAGE
	vbd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE 具体查MSDN
	vbd.StructureByteStride = 0;
	int hr = 0;
	hr = g_pd3dDevice->CreateBuffer(&vbd, &tmpInitData_Vertex, &m_pVB_Gpu_Sky);
	HR_DEBUG(hr, "Noise Atmosphere : creating VERTEX BUFFER failed!!");

	//create index buffer
	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(int) * tmpIndexCount;
	ibd.Usage = D3D11_USAGE_DEFAULT;//这个是GPU能对其读写,IMMUTABLE是GPU只读
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0; //CPU啥都干不了  D3D_USAGE
	ibd.MiscFlags = 0;//D3D11_RESOURCE_MISC_RESOURCE 具体查MSDN
	ibd.StructureByteStride = 0;
	hr = g_pd3dDevice->CreateBuffer(&ibd, &tmpInitData_Index, &m_pIB_Gpu_Sky);
	HR_DEBUG(hr, "INDEX BUFFER创建失败");


	return TRUE;
};




