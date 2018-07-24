
/***********************************************************************

								h��SweepingTrail

	A Sweeping trail several main characteristic:
	1. a trail path is driven by a sequence of spatial line segments
	2. adjacent line segments pairs can build up 2 triangles ( or a spatial 'quad')
	3. the header of the trail (the head line segment) must be updated in every frame
	4. the header of the trail should be "cooled down" when reached a given time duration limit.
			A new "free" header line trail shall be generated by then.
	5. the tail of the trail(the last line segment) must be updated in every frame, and
			approaching to the second last line segment over time to reduce the area 
			of the last quad (Meanwhile, the u coord of the tail vertices is maintained to 1)
	6. (2018.7.23)the headers' vertices u coord should be 0, while the tail should be 1.

************************************************************************/

#include "Noise3D.h"

using namespace Noise3D;

Noise3D::ISweepingTrail::ISweepingTrail() :
	mIsHeaderActive(false),
	mIsTailActive(false),
	//mHeaderCoolDownDistanceThreshold(1.0f),
	mHeaderCoolDownTimer(0.0f),
	mTailQuadCollapsingTimer(0.0f),
	mHeaderCoolDownTimeThreshold(20.0f),
	mTailQuadCollapseDuration(20.0f)
{
}

Noise3D::ISweepingTrail::~ISweepingTrail()
{
	ReleaseCOM(m_pVB_Gpu);
}

void Noise3D::ISweepingTrail::SetHeaderLineSegment(N_LineSegment lineSeg)
{
	mFreeHeader = lineSeg;
	if (!mIsHeaderActive)mIsHeaderActive = true;
}

void Noise3D::ISweepingTrail::SetHeaderCoolDownTimeThreshold(float duration)
{
	mHeaderCoolDownTimeThreshold = duration;
}

/*void Noise3D::ISweepingTrail::SetHeaderCoolDownDistance(float distance)
{
	mHeaderCoolDownDistanceThreshold = distance;
}*/

void Noise3D::ISweepingTrail::SetTailCollapsedTime(float duration)
{
	mTailQuadCollapseDuration = duration;
}

void Noise3D::ISweepingTrail::Update(float deltaTime)
{
	//timer add
	mTailQuadCollapsingTimer += deltaTime;

	//ensure that there is at least one cooled down line segment
	if (mLineSegments.empty())
	{
		mLineSegments.push_back(mFreeHeader);
	}

	//..
	if (!mIsTailActive)
	{
		mFreeTail_Current = mFreeHeader;
	}

	mFunction_CoolDownHeader();
	mFunction_MoveAndCollapseTail();
	mFunction_UpdateVertexBufferInMem();
}

/*****************************************************************


*****************************************************************/
bool NOISE_MACRO_FUNCTION_EXTERN_CALL Noise3D::ISweepingTrail::mFunction_InitGpuBuffer(UINT maxVertexCount)
{
	D3D11_SUBRESOURCE_DATA tmpInitData_Vertex;
	ZeroMemory(&tmpInitData_Vertex, sizeof(tmpInitData_Vertex));
	tmpInitData_Vertex.pSysMem = &mVB_Mem.at(0);
	mVB_Mem.resize(maxVertexCount);
	mGpuVertexPoolCapacity = sizeof(N_SweepingTrailVertexType)* maxVertexCount;

	//Simple Vertex!
	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = mGpuVertexPoolCapacity;
	vbd.Usage = D3D11_USAGE_DYNAMIC;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	//create gpu vertex buffer
	int hr = 0;
	hr = D3D::g_pd3dDevice11->CreateBuffer(&vbd, &tmpInitData_Vertex, &m_pVB_Gpu);
	HR_DEBUG(hr, "SweepingTrail : Failed to create vertex pool ! ");
}

void Noise3D::ISweepingTrail::mFunction_CoolDownHeader()
{
	//if fixed line segment exist
	if (mLineSegments.size() > 0)
	{
		//"distance between lines"
		//vector.back() is the second front line segment
		N_LineSegment& line1 = mFreeHeader;
		N_LineSegment& line2 = mLineSegments.back();
		float vertexDist1 = (line1.vert1 - line2.vert1).Length();
		float vertexDist2 = (line1.vert2 - line2.vert2).Length();
		float lineDist = max(vertexDist1, vertexDist2);

		//cool down current header, and GENERATE a NEW free segment (push to back)
		//note that when a new line segment cool down, we use "push back"
		//thus vector.back() is right after the header of the line sequence
		if (lineDist >= mHeaderCoolDownDistanceThreshold)
		{
			mLineSegments.push_back(mFreeHeader);
		}
	}
}

void Noise3D::ISweepingTrail::mFunction_MoveAndCollapseTail()
{
	//if at least one fixed line segment exists
	if (mLineSegments.size() > 0)
	{
		
	}
}

void Noise3D::ISweepingTrail::mFunction_UpdateVertexBufferInMem()
{

}

void Noise3D::ISweepingTrail::mFunction_UpdateToGpuBuffer()
{
	//not all of the vertices can be updated
	uint32_t updateByteSize = min(mVB_Mem.size() * sizeof(N_SweepingTrailVertexType) , mGpuVertexPoolCapacity);

	//update to gpu
	//(2018.7.24)if the vertex pool capacity is exceeded, then some of the front vertices won't be uploaded.
	D3D11_MAPPED_SUBRESOURCE mappedRes;
	D3D::g_pImmediateContext->Map(m_pVB_Gpu, 0, D3D11_MAP_WRITE, NULL, &mappedRes);
	memcpy_s(mappedRes.pData, updateByteSize, &mVB_Mem.at(0), updateByteSize);
	D3D::g_pImmediateContext->Unmap(m_pVB_Gpu, 0);
}

float Noise3D::ISweepingTrail::mFunction_Util_DistanceBetweenLine(N_LineSegment & line1, N_LineSegment & line2)
{
	float vertexDist1 = (line1.vert1 - line2.vert1).Length();
	float vertexDist2 = (line1.vert2 - line2.vert2).Length();
	float lineDist = max(vertexDist1, vertexDist2);
	return lineDist;
}
