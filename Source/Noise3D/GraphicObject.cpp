
/***********************************************************************

						class�� Graphic Object

	desc: store data of simple graphic object like line/point/triangle 
			on mem and GpuMem

************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::D3D;

//#define ASSIGN_VERTEX_VALUE(a,b) if(a!=b){a=b;canUpdate=true;}
#define ASSIGN_VERTEX_VALUE(a,b) {a=b;canUpdate=true;}

GraphicObject::GraphicObject()
{
	//there are several variables needed for one graphic object type
	//and for the time being ,there are NOISE_GRAPHIC_OBJECT_BUFFER_COUNT graphic object types
	//see enumerations in ".h" file
	for (UINT i = 0;i < NOISE_GRAPHIC_OBJECT_BUFFER_COUNT;i++)
	{
		mVB_ByteSize_GPU[i]		=0;

		m_pVB_GPU[i]					=nullptr;

		mCanUpdateToGpu[i]		= false;

		m_pVB_Mem[i]					= new std::vector<N_SimpleVertex>;
	}

	m_pBaseScreenSpacePosOffset = new Vec2(0, 0);
	m_pTextureUidList_Rect = new std::vector<N_UID>;
	m_pRectSubsetInfoList= new std::vector<N_GraphicObject_SubsetInfo>;
}

GraphicObject::~GraphicObject()
{
	for (UINT i = 0;i < NOISE_GRAPHIC_OBJECT_BUFFER_COUNT;i++)
	{
		ReleaseCOM(m_pVB_GPU[i]);
	}
	delete m_pBaseScreenSpacePosOffset;
	delete m_pTextureUidList_Rect;
	delete m_pRectSubsetInfoList;
}

void GraphicObject::SetBasePosOffset(Vec2 pixelOffset)
{
	// --------->
	// |		
	// |	PIXEL SPACE
	// |

	//				  ^
	//				   |
	//		--------|--------->
	//				   |		SCR SPACE
	//				   |
	
	Renderer* pRenderer = GetScene()->GetRenderer();
	UINT mainWidth = pRenderer->GetBackBufferWidth();
	UINT mainHeight = pRenderer->GetBackBufferHeight();

	pixelOffset.x = pixelOffset.x * 2.0f / float(mainWidth);
	pixelOffset.y = -pixelOffset.y*2.0f  / float(mainHeight);

	//vector difference
	Vec2 offsetV = pixelOffset - (*m_pBaseScreenSpacePosOffset);

	if (offsetV.x ==0.0f && offsetV.y == 0.0f)return;


	*m_pBaseScreenSpacePosOffset = pixelOffset;

	//update vertices, deviate with the upper difference
	for (UINT i = 0;i < NOISE_GRAPHIC_OBJECT_BUFFER_COUNT;i++)
	{
		if (i == NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D || i == NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D)continue;
		for (UINT j = 0;j < m_pVB_Mem[i]->size();++j)
		//for (auto &v : *m_pVB_Mem[i])
		{
			m_pVB_Mem[i]->at(j).Pos.x += offsetV.x;
			m_pVB_Mem[i]->at(j).Pos.y += offsetV.y;
		}
		mCanUpdateToGpu[i] = true;
	}
}

Vec2 GraphicObject::GetBasePosOffset()
{
	Renderer* pRenderer = GetScene()->GetRenderer();
	UINT mainWidth = pRenderer->GetBackBufferWidth();
	UINT mainHeight = pRenderer->GetBackBufferHeight();

	//Position Offset
	Vec2 outBaseTopLeftPixel = *m_pBaseScreenSpacePosOffset;
	//mFunction_ConvertFloatVec2PixelVec(outBaseTopLeftPixel);
	outBaseTopLeftPixel.x = outBaseTopLeftPixel.x *float(mainWidth) / 2.0f;
	outBaseTopLeftPixel.y = - outBaseTopLeftPixel.y * float(mainHeight) / 2.0f;
	return outBaseTopLeftPixel;
}

uint32_t GraphicObject::AddLine3D(Vec3 v1, Vec3 v2, Vec4 color1, Vec4 color2)
{
	mFunction_AddVertices3D(
		NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D, 
		{ v1,v2 },
		{ color1,color2 }, 
		{ Vec2(0,0),Vec2(0,0) }
	);
	return GetLine3DCount() - 1;
}

uint32_t Noise3D::GraphicObject::AddLine3D_AABB(Vec3 min, Vec3 max, Vec4 color1)
{
	Vec3 v[8] = {
		{ min}, //0
		{ max.x,min.y, min.z},//1
		{ min.x,max.y, min.z },//2
		{ min.x,min.y, max.z },//3
		{ max.x,max.y, min.z },//4
		{ min.x,max.y, max.z },//5
		{ max.x,min.y, max.z },//6
		{max}//7
	};

	uint32_t edge[12][2]=
	{
		{0,1}, {0,3}, {1,6},{3,6},
		{0,2}, {1,4}, {6,7}, {3,5},
		{2,4}, {4,7}, {2,5}, {5,7}
	};

	for (int i = 0; i < 12; ++i)
	{
		mFunction_AddVertices3D(NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D,
		{ v[edge[i][0]],v[edge[i][1]] }, { color1,color1 }, { Vec2(0,0),Vec2(0,0) });
	}

	return GraphicObject::GetLine3DCount() - 1;
}

uint32_t GraphicObject::AddLine2D(Vec2 v1, Vec2 v2, Vec4 color1, Vec4 color2)
{
	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(v1);
	mFunction_ConvertPixelVec2FloatVec(v2);

	mFunction_AddVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D,
		{ v1,v2 },
		{ color1,color2 },
		{ Vec2(0,0),Vec2(0,0) }
	);

	return GetLine2DCount()-1;
}

uint32_t GraphicObject::AddPoint3D(Vec3 v, Vec4 color)
{
	mFunction_AddVertices3D(
		NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D,
		{ v },
		{ color },
		{ Vec2(0,0) }
	);
	return GetPoint3DCount() - 1;
}

uint32_t GraphicObject::AddPoint2D(Vec2 v, Vec4 color)
{
	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(v);

	mFunction_AddVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D,
		{ v },
		{ color },
		{ Vec2(0,0) }
	);

	return GetPoint2DCount() - 1;
}

uint32_t GraphicObject::AddTriangle2D(Vec2 v1, Vec2 v2, Vec2 v3, Vec4 color1, Vec4 color2, Vec4 color3)
{
	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(v1);
	mFunction_ConvertPixelVec2FloatVec(v2);
	mFunction_ConvertPixelVec2FloatVec(v3);

	mFunction_AddVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D,
		{ v1,v2,v3 },
		{ color1,color2,color3 },
		{ Vec2(0,0),Vec2(0, 0),Vec2(0, 0) }
	);

	return GetTriangle2DCount() - 1;
}

uint32_t GraphicObject::AddRectangle(Vec2 vTopLeft,Vec2 vBottomRight,Vec4 color, const N_UID& texName)
{
	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(vTopLeft);
	mFunction_ConvertPixelVec2FloatVec(vBottomRight);


	Vec2 vTopRight = Vec2(vBottomRight.x, vTopLeft.y);
	Vec2 vBottomLeft = Vec2(vTopLeft.x, vBottomRight.y);

	//....initializer_list nit
	mFunction_AddVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D,
		{ vTopLeft,vTopRight,vBottomLeft,vTopRight,vBottomRight,vBottomLeft },
		{ color,color ,color ,color ,color ,color },
		{ Vec2(0,0),Vec2(1,0),Vec2(0, 1),Vec2(1, 0),Vec2(1, 1),Vec2(0, 1) }
	);

	//add TextureID
	m_pTextureUidList_Rect->push_back(texName);

	//return ID of Rectangle
	return GetRectCount()-1;
}

uint32_t GraphicObject::AddRectangle(Vec2 vCenter, float fWidth, float fHeight, Vec4 color, const N_UID& texName)
{
	//dont use coord conversion here , because in the other overload , conversion will be applied
	UINT newRectID = NOISE_MACRO_INVALID_ID;
	newRectID = AddRectangle(vCenter - Vec2(fWidth / 2, fHeight / 2), vCenter + Vec2(fWidth / 2, fHeight / 2), color, texName);
	return newRectID;
}

void GraphicObject::AdjustElementCount(NOISE_GRAPHIC_OBJECT_TYPE objType, UINT newElementCount)
{
	switch (objType)
	{
	case NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D:
		mFunction_AdjustElementCount(newElementCount, GetLine2DCount(), NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D);
		break;

	case NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D:
		mFunction_AdjustElementCount(newElementCount, GetLine3DCount(), NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D);
		break;

	case NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D:
		mFunction_AdjustElementCount(newElementCount, GetPoint2DCount(), NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D);
		break;

	case NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D:
		mFunction_AdjustElementCount(newElementCount, GetPoint3DCount(), NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D);
		break;

	case NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D:
		mFunction_AdjustElementCount(newElementCount, GetTriangle2DCount(), NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D);
		break;

	case NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D:
		mFunction_AdjustElementCount(newElementCount, GetRectCount(), NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D);
		break;

	default:
		break;
	}
}

void	GraphicObject::SetLine3D(UINT index, Vec3 v1, Vec3 v2, Vec4 color1, Vec4 color2)
{
	if (index > GetLine3DCount())
	{
		ERROR_MSG("Line 3D index invalid!!");
		return;
	}

	mFunction_SetVertices3D(
		NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D,
		index*2,
		{ v1,v2 },
		{ color1,color2 },
		{ Vec2(0,0),Vec2(0,0) }
	);

}

void	GraphicObject::SetLine2D(UINT index, Vec2 v1, Vec2 v2, Vec4 color1, Vec4 color2)
{
	if (index >=GetLine2DCount())
	{
		ERROR_MSG("Line 2D index invalid!!");
		return;
	}

	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(v1);
	mFunction_ConvertPixelVec2FloatVec(v2);

	mFunction_SetVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D,
		index*2,
		{ v1,v2 },
		{ color1,color2 },
		{ Vec2(0,0),Vec2(0,0) }
	);
}

void	GraphicObject::SetPoint3D(UINT index, Vec3 v, Vec4 color)
{
	if (index > GetPoint3DCount())
	{
		ERROR_MSG("Point 3D index invalid!!");
		return;
	}

	mFunction_SetVertices3D(
		NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D,
		index,
		{ v },
		{ color },
		{ Vec2(0,0) }
	);

}

void	GraphicObject::SetPoint2D(UINT index, Vec2 v, Vec4 color)
{
	if (index > GetPoint2DCount())
	{
		ERROR_MSG("Point 2D index invalid!!");
		return;
	}

	mFunction_ConvertPixelVec2FloatVec(v);

	mFunction_SetVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D,
		index,
		{ v },
		{ color },
		{ Vec2(0,0) }
	);
}

void	GraphicObject::SetTriangle2D(UINT index, Vec2 v1, Vec2 v2, Vec2 v3, Vec4 color1, Vec4 color2, Vec4 color3)
{
	if (index >= GetTriangle2DCount())
	{
		ERROR_MSG("Rectangle Index Invalid !!");
		return;
	}

	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(v1);
	mFunction_ConvertPixelVec2FloatVec(v2);
	mFunction_ConvertPixelVec2FloatVec(v3);

	mFunction_SetVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D,
		index*3,
		{ v1,v2,v3 },
		{ color1,color2,color3 },
		{ Vec2(0,0),Vec2(0, 0),Vec2(0, 0) }
	);
}

void	GraphicObject::SetRectangle(UINT index, Vec2 vTopLeft, Vec2 vBottomRight, Vec4 color, const N_UID& texName)
{
	//index mean the 'index'th rectangle

	if (index >= GetRectCount())
	{
		ERROR_MSG("Rectangle Index Invalid !!");
		return;
	}

	//coord unit conversion
	mFunction_ConvertPixelVec2FloatVec(vTopLeft);
	mFunction_ConvertPixelVec2FloatVec(vBottomRight);

	//.............
	auto pList = m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D];
	Vec2 vTopRight = Vec2(vBottomRight.x, vTopLeft.y);
	Vec2 vBottomLeft = Vec2(vTopLeft.x, vBottomRight.y);

	//modify TextureID
	if (texName != m_pTextureUidList_Rect->at(index))
	{
		m_pTextureUidList_Rect->at(index) = texName;
		mCanUpdateToGpu[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D] = true;
	}

	//skip if nth need to change,avoid unnecessary update subresource
	/*if (vTopLeft == m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(index * 6).Pos &&//top left
		vBottomRight == m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(index * 6 + 4).Pos &&//bottom right
		color == m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(index * 6).Color)//
		return;*/

	//....initializer_list nit
	mFunction_SetVertices2D(
		NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D,
		index*6,
		{ vTopLeft,vTopRight,vBottomLeft,vTopRight,vBottomRight,vBottomLeft },
		{ color,color ,color ,color ,color ,color },
		{ pList->at(index * 6).TexCoord,pList->at(index * 6 +1).TexCoord, pList->at(index * 6 +2).TexCoord,
		pList->at(index * 6 +3).TexCoord,pList->at(index * 6 + 4).TexCoord, pList->at(index * 6 + 5).TexCoord }
	);


}

void	GraphicObject::SetRectangle(UINT index, Vec2 vCenter, float fWidth, float fHeight, Vec4 color, const N_UID& texName)
{
	//dont use coord conversion here , because in the other overload , conversion will be applied
	SetRectangle(index,vCenter - Vec2(fWidth/2,fHeight/2),vCenter+ Vec2(fWidth / 2, fHeight / 2),color,texName);
}

void	GraphicObject::SetRectangleTexCoord(UINT index, Vec2 texCoordTopLeft,Vec2 texCoordBottomRight)
{


	//index mean the 'index'th rectangle

	if (index >=GetRectCount())
	{
		ERROR_MSG("Rectangle Index Invalid !!");
		return;
	}

	//after getting triangle ID (in global buffer) , compute 6 vertex ID of these 2 tri (in global buffer)
	UINT vertexID[6] =
	{
		index *6,
		index * 6	+ 1,
		index * 6	+ 2,
		index * 6	+3,
		index * 6	+ 4,
		index * 6	+ 5
	};

	//m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[0]).TexCoord = texCoordTopLeft;
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[0]).TexCoord =  texCoordTopLeft;
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[1]).TexCoord =  Vec2(texCoordBottomRight.x, texCoordTopLeft.y);
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[2]).TexCoord =  Vec2(texCoordTopLeft.x, texCoordBottomRight.y);
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[3]).TexCoord =  Vec2(texCoordBottomRight.x, texCoordTopLeft.y);
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[4]).TexCoord =  texCoordBottomRight;
	m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(vertexID[5]).TexCoord =  Vec2(texCoordTopLeft.x, texCoordBottomRight.y);


	mCanUpdateToGpu[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D] = true;	

}

void	GraphicObject::SetRectangleDepth(UINT index, float posZ)
{
	//index mean the 'index'th rectangle

	if (index >= GetRectCount())
	{ERROR_MSG("Rectangle Index Invalid !!");return;}

	//depth unchanged
	float& tmpPosZ = m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(index * 6).Pos.z;
	if (tmpPosZ == posZ)return;

	//after getting triangle ID (in global buffer) , compute 6 vertex ID of these 2 tri (in global buffer)
	for (UINT i = 0;i < 6;++i)
		m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->at(index * 6+i).Pos.z = posZ;

	//now it is allowed to update because of modification
	mCanUpdateToGpu[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D] = true;
}

void	GraphicObject::DeleteLine3D(UINT index)
{
	UINT vertexStartIndex = index * 2;
	UINT vertexCount = 2;	//1 line consist of 2 vertices
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D, vertexStartIndex, vertexCount);

}

void GraphicObject::DeleteLine3D(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	UINT vertexStartIndex = startID * 2;
	UINT vertexCount = (endID-startID+1)*2;	//1 line consist of 2 vertices
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D, vertexStartIndex, vertexCount);
}

void	GraphicObject::DeleteLine2D(UINT index)
{
	UINT vertexStartIndex = index * 2;
	UINT vertexCount = 2;	//1 line consist of 2 vertices
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D, vertexStartIndex, vertexCount);
}

void GraphicObject::DeleteLine2D(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	UINT vertexStartIndex = startID * 2;
	UINT vertexCount = (endID - startID+1) * 2;	//1 line consist of 2 vertices
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D, vertexStartIndex, vertexCount);
}

void	GraphicObject::DeletePoint3D(UINT index)
{
	UINT vertexStartIndex = index;
	UINT vertexCount = 1;
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D, vertexStartIndex, vertexCount);
}

void GraphicObject::DeletePoint3D(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	UINT vertexStartIndex = startID;
	UINT vertexCount = (endID - startID+1);
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D, vertexStartIndex, vertexCount);
}

void	GraphicObject::DeletePoint2D(UINT index)
{
	UINT vertexStartIndex = index;
	UINT vertexCount = 1;	
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D, vertexStartIndex, vertexCount);
}

void GraphicObject::DeletePoint2D(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	UINT vertexStartIndex = startID;
	UINT vertexCount = endID-startID+1;	
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D, vertexStartIndex, vertexCount);
}

void	GraphicObject::DeleteTriangle2D(UINT index)
{
	UINT vertexStartIndex = index* 3;
	UINT vertexCount = 3;
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D, vertexStartIndex, vertexCount);
}

void GraphicObject::DeleteTriangle2D(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	UINT vertexStartIndex = startID * 3;
	UINT vertexCount = (endID-startID+1)*3;
	mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D, vertexStartIndex, vertexCount);
}

void	GraphicObject::DeleteRectangle(UINT index)
{
	//delete the index_th Rectangle

	UINT vertexStartIndex = 0;
	UINT	 vertexCount = 0;

	if (index < GetRectCount())
	{
		vertexStartIndex = index * 6;
		vertexCount = 6;
		//erase vertices
 		mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D, vertexStartIndex, vertexCount);

		//erase texture info
		auto tmpIter = m_pTextureUidList_Rect->begin()+index;
		m_pTextureUidList_Rect->erase(tmpIter);
	}
	else
	{
		ERROR_MSG("delete rectangle : Rectangle Index Invalid!!");
		return;
	}

}

void GraphicObject::DeleteRectangle(UINT startID, UINT endID)
{
	if (startID > endID)std::swap(startID, endID);
	//delete the index_th Rectangle

	UINT vertexStartIndex = 0;
	UINT	 vertexCount = 0;

	if (endID < GetRectCount())
	{
		vertexStartIndex = startID * 6;
		vertexCount = (endID-startID+1)*6;
		//erase vertices
		mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D, vertexStartIndex, vertexCount);

		//erase texture info
		auto iterStart = m_pTextureUidList_Rect->begin() + startID;
		auto iterEnd = m_pTextureUidList_Rect->begin() + endID;
		m_pTextureUidList_Rect->erase(iterStart,iterEnd);
	}
	else
	{
		ERROR_MSG("delete rectangle : Rectangle Index Invalid!!");
		return;
	}
}

UINT GraphicObject::GetLine3DCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D]->size()/2;
};

UINT GraphicObject::GetLine2DCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D]->size() / 2;
};

UINT GraphicObject::GetPoint3DCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D]->size();
};

UINT GraphicObject::GetPoint2DCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D]->size();
};

UINT GraphicObject::GetTriangle2DCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D]->size()/3;
}

UINT GraphicObject::GetRectCount()
{
	return m_pVB_Mem[NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D]->size()/6;
}

N_AABB Noise3D::GraphicObject::GetLocalAABB()
{
	return N_AABB();
}

N_AABB Noise3D::GraphicObject::ComputeWorldAABB_Accurate()
{
	return N_AABB();
}

N_AABB Noise3D::GraphicObject::ComputeWorldAABB_Fast()
{
	return N_AABB();
}

N_BoundingSphere Noise3D::GraphicObject::ComputeWorldBoundingSphere_Accurate()
{
	return N_BoundingSphere();
}

NOISE_SCENE_OBJECT_TYPE Noise3D::GraphicObject::GetObjectType()const
{
	return NOISE_SCENE_OBJECT_TYPE::GRAPHIC_OBJECT;
}


/***********************************************************************
										P R I V A T E
***********************************************************************/

//When needed (like when updating data to gpu), this CreateVB function will be 
//invoked to dynamically resize (re-create) ID3DVB to fit the data

bool	GraphicObject::mFunction_CreateVB(UINT objType_ID)
{
	//get VB (in memory) byte size of corresponding graphic object type
	UINT vertexCount = m_pVB_Mem[objType_ID]->size();

	//Create VERTEX BUFFER
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = vertexCount * sizeof(N_SimpleVertex);
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0; 
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;


	D3D11_SUBRESOURCE_DATA tmpInitData;
	tmpInitData.pSysMem = vertexCount>0?&m_pVB_Mem[objType_ID]->at(0):nullptr;
	tmpInitData.SysMemPitch = 0;
	tmpInitData.SysMemSlicePitch = 0;

	//update the byte size of GPU vertex buffer
	mVB_ByteSize_GPU[objType_ID] = vbd.ByteWidth;

	//Create Buffers
	HRESULT hr = 0;
	hr = g_pd3dDevice11->CreateBuffer(&vbd, &tmpInitData, &m_pVB_GPU[objType_ID]);

	HR_DEBUG(hr, "Fail to create vertex buffer.");

	return true;
}

void		GraphicObject::mFunction_UpdateVerticesToGpu(UINT objType_ID)
{
	//nothing to update ,so exit (in InitVB() , create a  
	if (m_pVB_Mem[objType_ID]->size() == 0)return;


	//if a GPU buffer has not been created then create a new one
	if (m_pVB_GPU[objType_ID] == NULL)
	{
		mFunction_CreateVB(objType_ID);
		return;
	}


	//calculate byte size of ALL VERTEX BUFFER
	UINT mVB_Byte_Size_Memory =	m_pVB_Mem[objType_ID]->size()*	sizeof(N_SimpleVertex);

	//decide whether we should expand the capacity of GPU Vertex Buffer
	if (mVB_Byte_Size_Memory  > mVB_ByteSize_GPU[objType_ID])
	{
		//If VB was once created , destroy it to remake a new one
		if (m_pVB_GPU[objType_ID])
		{
			m_pVB_GPU[objType_ID]->Release();
			
			//create a new VB
			mFunction_CreateVB(objType_ID);
		}
		else
		{
			mFunction_CreateVB(objType_ID);
		}
	}


	//update new data to GPU
	g_pImmediateContext->UpdateSubresource(m_pVB_GPU[objType_ID], 0, 0, &m_pVB_Mem[objType_ID]->at(0), 0, 0);

}

void		GraphicObject::mFunction_AddVertices2D(NOISE_GRAPHIC_OBJECT_TYPE buffType, std::initializer_list<Vec2> vertexList, std::initializer_list<Vec4> colorList, std::initializer_list<Vec2> texcoordList)
{

	N_SimpleVertex tmpVertex;
	auto vertexIter = vertexList.begin();
	auto colorIter = colorList.begin();
	auto texcoordIter = texcoordList.begin();

	//construct a N_SimpleVertex
	for (UINT i = 0;i < vertexList.size();i++)
	{

		tmpVertex = 
		{
			Vec3(vertexIter->x+m_pBaseScreenSpacePosOffset->x,vertexIter->y + m_pBaseScreenSpacePosOffset->y,0.0f),
			*colorIter++,
			*texcoordIter++
		};
		vertexIter++;

		m_pVB_Mem[buffType]->push_back(tmpVertex);
	}

	//now it is allowed to update because of modification
	mCanUpdateToGpu[buffType] = true;
}

void		GraphicObject::mFunction_AddVertices3D(NOISE_GRAPHIC_OBJECT_TYPE buffType, std::initializer_list<Vec3> vertexList, std::initializer_list<Vec4> colorList, std::initializer_list<Vec2> texcoordList)
{

	N_SimpleVertex tmpVertex;
	auto vertexIter = vertexList.begin();
	auto colorIter = colorList.begin();
	auto texcoordIter = texcoordList.begin();

	//construct a N_SimpleVertex
	for (UINT i = 0;i < vertexList.size();i++)
	{
		tmpVertex =
		{
			*vertexIter++,
			*colorIter++,
			*texcoordIter++
		};
		m_pVB_Mem[buffType]->push_back(tmpVertex);
	}

	//now it is allowed to update because of modification
	mCanUpdateToGpu[buffType] = true;
}

void		GraphicObject::mFunction_SetVertices2D(NOISE_GRAPHIC_OBJECT_TYPE buffType, UINT iVertexStartID, std::initializer_list<Vec2> vertexList, std::initializer_list<Vec4> colorList, std::initializer_list<Vec2> texcoordList)
{
	//check boundary (check the tail ,if the tail is within boundary , then it's valid
	if (iVertexStartID + vertexList.size() >m_pVB_Mem[buffType]->size())
	{
		ERROR_MSG("Noise Graphic Object : Vertex ID Out of boundary!");
		return;
	}
	
	std::initializer_list<Vec2>::iterator vertexIter = vertexList.begin();
	std::initializer_list<Vec4>::iterator colorIter = colorList.begin();
	std::initializer_list<Vec2>::iterator texcoordIter = texcoordList.begin();

	bool canUpdate = false;

	//construct a N_SimpleVertex
	for (UINT i = 0;i < vertexList.size();i++)
	{
		// --------->
		// |		
		// |	PIXEL SPACE
		// |

		//				  ^
		//				   |
		//		--------|--------->
		//				   |		SCR SPACE
		//				   |

		ASSIGN_VERTEX_VALUE(m_pVB_Mem[buffType]->at(iVertexStartID + i).Pos.x, vertexIter->x + m_pBaseScreenSpacePosOffset->x);
		ASSIGN_VERTEX_VALUE(m_pVB_Mem[buffType]->at(iVertexStartID + i).Pos.y, vertexIter->y + m_pBaseScreenSpacePosOffset->y);
		ASSIGN_VERTEX_VALUE(m_pVB_Mem[buffType]->at(iVertexStartID + i).Color, *colorIter);
		ASSIGN_VERTEX_VALUE(m_pVB_Mem[buffType]->at(iVertexStartID + i).TexCoord,*texcoordIter);
		++colorIter;
		++texcoordIter;
		++vertexIter;

	}

	//now it is allowed to update because of modification
	mCanUpdateToGpu[buffType] = true;//(mCanUpdateToGpu[buffType] || canUpdate);

}

void		GraphicObject::mFunction_SetVertices3D(NOISE_GRAPHIC_OBJECT_TYPE buffType, UINT iVertexStartID, std::initializer_list<Vec3> vertexList, std::initializer_list<Vec4> colorList, std::initializer_list<Vec2> texcoordList)
{
	//check boundary (check the tail ,if the tail is within boundary , then it's valid
	if (iVertexStartID + vertexList.size() >m_pVB_Mem[buffType]->size())
	{
		ERROR_MSG("Noise Graphic Object : Vertex ID Out of boundary!");
		return;
	}

	N_SimpleVertex tmpVertex;
	auto vertexIter = vertexList.begin();
	auto colorIter = colorList.begin();
	auto texcoordIter = texcoordList.begin();

	bool canUpdate = false;

	//construct a N_SimpleVertex
	for (UINT i = 0;i < vertexList.size();i++)
	{

		tmpVertex =
		{
			*vertexIter++,
			*colorIter++,
			*texcoordIter++
		};

		ASSIGN_VERTEX_VALUE(m_pVB_Mem[buffType]->at(iVertexStartID + i), tmpVertex);
		//m_pVB_Mem[buffType]->at(iVertexStartID + i) = tmpVertex;
	}

	//now it is allowed to update because of modification
	mCanUpdateToGpu[buffType] = canUpdate;

}

void		GraphicObject::mFunction_EraseVertices(NOISE_GRAPHIC_OBJECT_TYPE buffType, UINT iVertexStartID, UINT iVertexCount)
{
	auto pList			= m_pVB_Mem[buffType];
	auto iter_Start	= pList->begin();
	auto iter_End	= pList->begin();


	//check boundary (check the tail ,if the tail is within boundary , then it's valid
	if ((iVertexStartID + iVertexCount) > pList->size())
	{
		ERROR_MSG("Noise Graphic Object : Vertex ID Out of boundary!");
		return;
	}


	//vector :: erase  ---------  delete elements in [a,b) 
	//(note that if I apply vector.erase(begin()+1,begin()+3) , then the 2nd & 3rd elements will be erased
	iter_Start += iVertexStartID;
	iter_End = iter_Start + iVertexCount;

	//delte elements
	pList->erase(iter_Start, iter_End);

	mCanUpdateToGpu[buffType] = true;
}

void		GraphicObject::mFunction_ConvertFloatVec2PixelVec(Vec2 & in_out_vec)
{
	// --------->
	// |		
	// |	PIXEL SPACE
	// |

	//				  ^
	//				   |
	//		--------|--------->
	//				   |		SCR SPACE
	//				   |
	Renderer* pRenderer = GetScene()->GetRenderer();
	UINT mainWidth = pRenderer->GetBackBufferWidth();
	UINT mainHeight = pRenderer->GetBackBufferHeight();

	in_out_vec.x = float(mainWidth) *((in_out_vec.x + 1.0f) / 2.0f);
	in_out_vec.y = float(mainHeight)*((1.0f - in_out_vec.y) / 2.0f);
};

inline void  GraphicObject::mFunction_ConvertPixelVec2FloatVec(Vec2& vec)
{
	// --------->
	// |		
	// |	PIXEL SPACE
	// |

	//				  ^
	//				   |
	//		--------|--------->
	//				   |		SCR SPACE
	//				   |
	Renderer* pRenderer = GetScene()->GetRenderer();
	UINT mainWidth = pRenderer->GetBackBufferWidth();
	UINT mainHeight = pRenderer->GetBackBufferHeight();

	//PIXEL SPACE TO [-1,1] SCR SPACE
	float halfW= (float)mainWidth / 2.0f;
	float halfH = (float)mainHeight / 2.0f;
	vec.x = (vec.x / halfW)- 1.0f;	
	vec.y=1.0f - (vec.y / halfH);
	//vec -= *m_pBaseScreenSpacePosOffset;
}

inline float GraphicObject::mFunction_ConvertPixelLength2FloatLength(float pxLen, bool isWidth)
{
	Renderer* pRenderer = GetScene()->GetRenderer();
	UINT mainWidth = pRenderer->GetBackBufferWidth();
	UINT mainHeight = pRenderer->GetBackBufferHeight();

	float outLength = 0;
	//PIXEL SPACE TO [-1,1] SCR SPACE
	if (isWidth)
	{
		outLength = pxLen*2.0f / (float)mainWidth;
	}
	else
	{
		outLength = pxLen * 2.0f / (float)mainHeight;
	}
	return outLength;
}

void	GraphicObject::mFunction_AdjustElementCount(UINT newCount, UINT currentObjCount, NOISE_GRAPHIC_OBJECT_TYPE objType)
{

	if (newCount<0 || currentObjCount == newCount)
	{
		return;
	}

	if (currentObjCount < newCount)
	{
		for (UINT i = 0;i < newCount - currentObjCount;i++)
		{
			switch (objType)
			{
			case NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D:
				AddLine2D(Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f));
				break;

			case NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D:
				AddLine3D(Vec3(0, 0, 0), Vec3(0, 0, 0));
				break;

			case NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D:
				AddPoint2D(Vec2(0, 0));
				break;
			case NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D:
				AddPoint3D(Vec3(0, 0,0));
				break;

			case NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D:
				AddTriangle2D(Vec2(0, 0), Vec2(0, 0), Vec2(0, 0));
				break;

			case NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D:			
				AddRectangle(Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f), Vec4(1.0f, 1.0f, 1.0f, 1.0f));
				break;
			}

		}
		return;
	}

	if (currentObjCount > newCount)
	{
		switch (objType)
		{
		case NOISE_GRAPHIC_OBJECT_TYPE_LINE_2D:
			DeleteLine2D(newCount,currentObjCount-1);
			break;

		case NOISE_GRAPHIC_OBJECT_TYPE_LINE_3D:
			DeleteLine3D(newCount, currentObjCount - 1);
			break;

		case NOISE_GRAPHIC_OBJECT_TYPE_POINT_2D:
			DeletePoint2D( newCount, currentObjCount - 1);
			break;

		case NOISE_GRAPHIC_OBJECT_TYPE_POINT_3D:
			DeletePoint3D( newCount, currentObjCount - 1);
			break;

		case NOISE_GRAPHIC_OBJECT_TYPE_TRIANGLE_2D:
			DeleteTriangle2D(newCount, currentObjCount - 1);
			break;

		case NOISE_GRAPHIC_OBJECT_TYPE_RECT_2D:
			DeleteRectangle(newCount, currentObjCount - 1);
			break;
		}
		return;
	}

	return;
}

void		GraphicObject::mFunction_GenerateRectSubsetInfo()
{
	//clear old record
	m_pRectSubsetInfoList->clear();

	//ignore empty vertex buffer
	if (m_pTextureUidList_Rect->size() == 0)return;


	//init the first subset (at least 1 element)
	N_GraphicObject_SubsetInfo newSubset;
	newSubset.vertexCount = 6;//6 ver for 1 rect
	newSubset.startID = 0;
	newSubset.texName = m_pTextureUidList_Rect->at(0);
	m_pRectSubsetInfoList->push_back(newSubset);

	//1 UINT for 1 Rect,6 vertices
	for (UINT i = 1;i < m_pTextureUidList_Rect->size();i++)
	{
		//region growing
		if (m_pTextureUidList_Rect->at(i) == m_pRectSubsetInfoList->back().texName)
		{
			m_pRectSubsetInfoList->back().vertexCount+=6;
		}
		else
		{
			newSubset.startID = i*6;
			newSubset.vertexCount = 6;
			newSubset.texName = m_pTextureUidList_Rect->at(i);
			m_pRectSubsetInfoList->push_back(newSubset);
		}


	}

}
