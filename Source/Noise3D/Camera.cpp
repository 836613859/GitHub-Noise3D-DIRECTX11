/************************************************************************

							CPP:  	Noise Camera
				
1������YawPitchRoll�����Ƕ��壺����ʵ������ϵ���ǵĶ��壩
			Yaw:����ͼ˳ʱ��ת
			Pitch:y�����ķ���ת����ͷ��
			Roll:��ʱ��ת


************************************************************************/

#pragma once
#include "Noise3D.h"

using namespace Noise3D;

ICamera::ICamera()
{
	mRotateX_Pitch=0;
	mRotateY_Yaw=0;
	mRotateZ_Roll=0;
	mViewAngleY = (float)60/180 * MATH_PI;
	mAspectRatio = 1.5;
	m_pPosition = new D3DXVECTOR3(0,0,0);
	m_pLookat = new D3DXVECTOR3(1,0,0);
	m_pDirection = new D3DXVECTOR3(1,0,0);
	mNearPlane=1;
	mFarPlane= 1000;

	m_pMatrixProjection = new D3DXMATRIX();
	m_pMatrixView = new D3DXMATRIX();
	D3DXMatrixPerspectiveFovLH(m_pMatrixProjection,mViewAngleY,mAspectRatio,mNearPlane,mFarPlane);
	D3DXMatrixIdentity(m_pMatrixView);
}

ICamera::~ICamera()
{
};


void ICamera::Destroy()
{
	 m_pMatrixView=nullptr;
	 m_pMatrixProjection = nullptr;
	 m_pDirection = nullptr;
	 m_pPosition = nullptr;
	 m_pLookat = nullptr;

};

void	ICamera::SetLookAt(NVECTOR3 vLookat)
{
	//pos and lookat can't superpose each other
	NVECTOR3 dir = vLookat - *m_pPosition;
	if (D3DXVec3Length(&dir) < 0.0001f)return;

	*m_pLookat=vLookat;
	mFunction_UpdateRotation();
};

void	ICamera::SetLookAt(float x,float y,float z)
{
	NVECTOR3 tmpLookat(x,y,z);
	//pos and lookat can't superpose each other
	NVECTOR3 dir = tmpLookat - *m_pPosition;
	if (D3DXVec3Length(&dir) < 0.0001f)return;

	*m_pLookat=tmpLookat;
	mFunction_UpdateRotation();
};

NVECTOR3	ICamera::GetLookAt()
{
	return  *m_pLookat;
};

NVECTOR3 	ICamera::GetDirection()
{
	return *m_pLookat-*m_pPosition;
}

void	ICamera::SetPosition(NVECTOR3 vPos)
{
	//pos and lookat can't superpose each other
	NVECTOR3 dir = *m_pLookat - vPos;
	if (D3DXVec3Length(&dir) < 0.0001f)return;

	*m_pPosition=vPos;
	mFunction_UpdateRotation();
}

void	ICamera::SetPosition(float x,float y,float z)
{
	NVECTOR3 tmpPos(x,y,z);
	NVECTOR3 dir = *m_pLookat - tmpPos;
	if (D3DXVec3Length(&dir) < 0.0001f)return;

	*m_pPosition=tmpPos;
	mFunction_UpdateRotation();
};

NVECTOR3 ICamera::GetPosition()
{
	return *m_pPosition;
};

void	ICamera::Move(NVECTOR3 vRelativePos)
{
	D3DXVec3Add(m_pPosition,m_pPosition,&vRelativePos);
	D3DXVec3Add(m_pLookat, m_pLookat, &vRelativePos);

};

void	ICamera::Move(float relativeX,float relativeY,float relativeZ)
{
	*m_pPosition += NVECTOR3(relativeX, relativeY, relativeZ);
};

void	ICamera::SetRotation(float RX_Pitch,float RY_Yaw,float RZ_Roll)//Ҫ����Lookat
{
	SetRotationX_Pitch(RX_Pitch);
	SetRotationY_Yaw(RY_Yaw);
	SetRotationZ_Roll(RZ_Roll);
	mFunction_UpdateDirection();
};

void	ICamera::SetRotationY_Yaw(float angleY)
{
	mRotateY_Yaw = angleY;
	mFunction_UpdateDirection();
};

void	ICamera::SetRotationX_Pitch(float AngleX)
{
	//clamp to [-pi/2,pi/2]
	mRotateX_Pitch = Clamp(AngleX, -MATH_PI / 2.0f, MATH_PI / 2.0f);

	mFunction_UpdateDirection();
};

void	ICamera::SetRotationZ_Roll(float AngleZ)
{
	//roll��������Ҫ����lookat
	mRotateZ_Roll = AngleZ;
}

float ICamera::GetRotationY_Yaw()
{
	return mRotateY_Yaw;
}

float ICamera::GetRotationX_Pitch()
{
	return mRotateX_Pitch;
}

float ICamera::GetRotationZ_Roll()
{
	return mRotateZ_Roll;
}

void ICamera::RotateY_Yaw(float angleY)
{
	SetRotationY_Yaw(mRotateY_Yaw + angleY);
	mFunction_UpdateDirection();
};

void ICamera::RotateX_Pitch(float angleX)
{
	float newAngle = mRotateX_Pitch + angleX;
	SetRotationX_Pitch(newAngle);
	mFunction_UpdateDirection();
};

void ICamera::RotateZ_Roll(float angleZ)
{
	SetRotationZ_Roll(mRotateZ_Roll + angleZ);
	mFunction_UpdateDirection();
};

void ICamera::fps_MoveForward(float fSignedDistance, BOOL enableYAxisMovement)
{
	//...Yaw Angle Starts at Z axis ( left-handed system) 
	/*		Z
			|    / A
			|_ /   
		O	|/__________ X		

			angle AOZ is the yaw angle
	
	*/

	NVECTOR3 relativePos;

	//generate a direction first (later multiply it with fDist)
	if(enableYAxisMovement)
	{ 
		relativePos.x = sinf(mRotateY_Yaw) *cosf(mRotateX_Pitch);
		relativePos.z = cosf(mRotateY_Yaw) *cosf(mRotateX_Pitch);
		relativePos.y = sinf(mRotateX_Pitch);
	}
	else
	{
		relativePos.x = sinf(mRotateY_Yaw);
		relativePos.z = cosf(mRotateY_Yaw);
		relativePos.y = 0;
	}

	// length * unit_dir
	relativePos *= fSignedDistance;

	//relative movement
	Move(relativePos);

}

void ICamera::fps_MoveRight(float fSignedDistance, BOOL enableYAxisMovement)
{
	//...Yaw Angle Starts at Z axis ( left-handed system) 
	/*		
		Z
		|    / A
		|_ /
	O	|/__________ X

	angle AOZ is the yaw angle

	*/

		NVECTOR3 relativePos;

		//generate a direction first (later multiply it with fDist)
		if (enableYAxisMovement)
		{
			relativePos.x = cosf(mRotateY_Yaw) *cosf(mRotateX_Pitch);
			relativePos.z = -sinf(mRotateY_Yaw) *cosf(mRotateX_Pitch);
			relativePos.y = sinf(mRotateX_Pitch);
		}
		else
		{
			relativePos.x = cosf(mRotateY_Yaw);
			relativePos.z = -sinf(mRotateY_Yaw);
			relativePos.y = 0;
		}

		// length * unit_dir
		relativePos *= fSignedDistance;

		//relative movement
		Move(relativePos);
}

void ICamera::fps_MoveUp(float fSignedDistance)
{
	*m_pPosition += NVECTOR3(0, fSignedDistance, 0);
	*m_pLookat += NVECTOR3(0, fSignedDistance, 0);
}

void ICamera::GetViewMatrix(NMATRIX & outMat)
{
	mFunction_UpdateViewMatrix();
	outMat = *m_pMatrixView;
}

void ICamera::GetProjMatrix(NMATRIX & outMat)
{
	mFunction_UpdateProjMatrix();
	outMat = *m_pMatrixProjection;
}

void Noise3D::ICamera::GetInvViewMatrix(NMATRIX & outMat)
{
	mFunction_UpdateViewMatrix();
	auto invPtr = D3DXMatrixInverse(&outMat, nullptr, m_pMatrixView);
	if(invPtr==nullptr)ERROR_MSG("Camera : Inverse of View Matrix not exist!")
}

void Noise3D::ICamera::GetInvProjMatrix(NMATRIX & outMat)
{
	mFunction_UpdateProjMatrix();
	auto invPtr = D3DXMatrixInverse(&outMat, nullptr, m_pMatrixProjection);
	if (invPtr == nullptr)ERROR_MSG("Camera : Inverse of Proj Matrix not exist!")
};






void	ICamera::SetViewFrustumPlane(float iNearPlaneZ,float iFarPlaneZ)
{
	if ( (iNearPlaneZ >0) && (iFarPlaneZ>iNearPlaneZ))
	{
		mNearPlane	= iNearPlaneZ;
		mFarPlane	=	iFarPlaneZ;
	}

};

void ICamera::SetViewAngle(float iViewAngleY,float iAspectRatio)
{
	if(iViewAngleY>0 && (mViewAngleY <(MATH_PI/2))){mViewAngleY	=	iViewAngleY;	}
	if(iAspectRatio>0){mAspectRatio	= iAspectRatio;}
};


/************************************************************************
											PRIVATE	
************************************************************************/

void	ICamera::mFunction_UpdateProjMatrix()
{
	D3DXMatrixPerspectiveFovLH(
		m_pMatrixProjection,
		mViewAngleY,
		mAspectRatio,
		mNearPlane,
		mFarPlane);

	//Ҫ���µ�GPU��TM��ȻҪ��ת��
	D3DXMatrixTranspose(m_pMatrixProjection,m_pMatrixProjection);
};

void	ICamera::mFunction_UpdateViewMatrix()
{

	NMATRIX	tmpMatrixTranslation;
	NMATRIX	tmpMatrixRotation;
	//�ȶ���ԭ��
	D3DXMatrixTranslation(&tmpMatrixTranslation, -m_pPosition->x, -m_pPosition->y, -m_pPosition->z);
	//Ȼ����yawpitchroll������ת��view�ռ�
	D3DXMatrixRotationYawPitchRoll(&tmpMatrixRotation, mRotateY_Yaw, mRotateX_Pitch, mRotateZ_Roll);
	//���������ת������
	D3DXMatrixTranspose(&tmpMatrixRotation,&tmpMatrixRotation);
	//��ƽ�ƣ�����ת
	D3DXMatrixMultiply(m_pMatrixView,&tmpMatrixTranslation,&tmpMatrixRotation);
	//Ҫ���µ�GPU��TM��ȻҪ��ת��
	//(2016.4.11)¥��ò���е㲻�԰��������shader��Ȼһֱд���Ǿ����ҳˣ�������
	//һֱ����������������������ֵ���
	D3DXMatrixTranspose(m_pMatrixView,m_pMatrixView);

};

void	ICamera::mFunction_UpdateRotation()
{
	//main function: cope with rotation change after the direction changes

	//update Direction
	D3DXVec3Subtract(m_pDirection,m_pLookat,m_pPosition);

	//pitch�ǣ� tan = y/sqr(x^2+z^2))
	/*	ע�⣺	atan ranged [-pi/2,pi/2]  
					atan2 ranged [-pi,pi] 		*/

	NVECTOR3 radiusDir(m_pDirection->x, 0, m_pDirection->z);
	//always positive
	float radiusLength = D3DXVec3Length(&radiusDir);
	//atan2(y,x) , radiusLength is constantly positive, pitch angle will range [-pi/2,pi/2] 
	//pitch : rotate downside is positive (the definition of rotation in left-handed frame)
	mRotateX_Pitch = atan2(-m_pDirection->y,radiusLength );

	//yaw�ǣ� tan = -x/z
	mRotateY_Yaw = atan2(m_pDirection->x,m_pDirection->z);//����ͼyaw��˳ʱ������

	//roll�ǣ�����direction����ı�roll�� 
	//roll��ʱ��ת������
};

void	ICamera::mFunction_UpdateDirection()
{
	//��Ҫ���ܣ����������Ҫ��Ϊ�˴�����̬�Ǹı����������Direction�仯

	//Ҫ����Lookat
	float tmpDirectionLength = D3DXVec3Length(m_pDirection);
	//ֱ�������Ǻ������Direction	3dscanner������任һ��������
	//!!!!!!!!!!!!!!!!!!!! (2016.7.21)���Ż᲻����-z��+x�أ����������󼦹ֵ�camera����һ��
	m_pDirection->x =- tmpDirectionLength* sin(mRotateY_Yaw)* cos(mRotateX_Pitch);
	m_pDirection->z =tmpDirectionLength* cos(mRotateY_Yaw)*cos(mRotateX_Pitch);
	m_pDirection->y =tmpDirectionLength* sin(mRotateX_Pitch);
	D3DXVec3Add(m_pLookat,m_pPosition,m_pDirection);
};
