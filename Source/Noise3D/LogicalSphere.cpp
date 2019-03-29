
/*******************************************************

						cpp ��Logical Sphere

********************************************************/

#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::D3D;

Noise3D::LogicalSphere::LogicalSphere():
	mRadius(1.0f)
{
}

Noise3D::LogicalSphere::~LogicalSphere()
{
}

void Noise3D::LogicalSphere::SetRadius(float r)
{
	mRadius = r > 0.0f ? r : 1.0f;
}

float Noise3D::LogicalSphere::GetRadius()
{
	return mRadius;
}

NOISE_SCENE_OBJECT_TYPE Noise3D::LogicalSphere::GetObjectType() const
{
	return NOISE_SCENE_OBJECT_TYPE::LOGICAL_SPHERE;
}

N_AABB Noise3D::LogicalSphere::GetLocalAABB()
{
	NVECTOR3 max = NVECTOR3(mRadius, mRadius, mRadius);
	NVECTOR3 min = NVECTOR3(-mRadius, -mRadius, -mRadius);
	return N_AABB(min, max);
}

N_AABB Noise3D::LogicalSphere::ComputeWorldAABB_Accurate()
{
	//(2019.3.27)Shameful, but, it's not so easy to solve for the accurate bounding box
	//of affine transformed sphere... so did the surface area, etc.
	//got some idea: perhaps i can use S to yield an ellipsoid equation
	// (x/s_x)^2+(y/s_y)^2+(z/s_z)^2 = 1
	// and use R to rotate a direction (1,0,0) or sth like that, 
	//calculate intersection point at rotated direction, and get its normal/tangent plane
	//then calculate signed distance between tangent plane and origin,
	//then add it to world space position to yield one bounding "slab"
	//Unfortunately, i don't have time for now because of graduation project.
	//(2019.3.27)so all 'SCALE' are ignored

	SceneNode* pNode = ISceneObject::GetAttachedSceneNode();
	if (pNode == nullptr)return N_AABB();

	AffineTransform t = pNode->EvalWorldTransform();
	NVECTOR3 worldCenterPos = t.GetPosition();
	NVECTOR3 offset = NVECTOR3(mRadius, mRadius, mRadius);
	return N_AABB(worldCenterPos - offset, worldCenterPos + offset);
}

float Noise3D::LogicalSphere::ComputeArea()
{
	//(2019.3.27)scale is ignore
	return 4.0f * Ut::PI * mRadius * mRadius ;
}

