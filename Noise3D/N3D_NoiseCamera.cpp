/************************************************************************

							CPP:  	Noise Engine
				
1������YawPitchRoll�����Ƕ��壺����ʵ������ϵ���ǵĶ��壩
			Yaw:����ͼ˳ʱ��ת
			Pitch:y�����ķ���ת����ͷ��
			Roll:��ʱ��ת


************************************************************************/

#pragma once
#include "Noise3D.h"

NoiseCamera::NoiseCamera()
{
	m_pFatherScene = NULL;
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
	mCanUpdateProjMatrix = TRUE;

	m_pMatrixProjection = new D3DXMATRIX();
	m_pMatrixView = new D3DXMATRIX();
	D3DXMatrixPerspectiveFovLH(m_pMatrixProjection,mViewAngleY,mAspectRatio,mNearPlane,mFarPlane);
	D3DXMatrixIdentity(m_pMatrixView);
};

NoiseCamera::~NoiseCamera()
{
	delete m_pMatrixView;
	delete m_pMatrixProjection;
	delete m_pDirection;
	delete m_pPosition;
	delete m_pLookat;
};


void	NoiseCamera::SetLookAt(NVECTOR3* vLookat)
{
	*m_pLookat=*vLookat;
	mFunction_UpdateRotation();
};

void	NoiseCamera::SetLookAt(float x,float y,float z)
{
	NVECTOR3 tmpLookat(x,y,z);
	*m_pLookat=tmpLookat;
	mFunction_UpdateRotation();
};

void	NoiseCamera::GetLookAt(NVECTOR3& out_vLookat)
{
	out_vLookat = *m_pLookat;
};

void	NoiseCamera::SetPosition(NVECTOR3* vPos)
{
	//lookat��λ�ò����غϰ�
		*m_pPosition=*vPos;
		mFunction_UpdateRotation();
}

void	NoiseCamera::SetPosition(float x,float y,float z)
{
	NVECTOR3 tmpPos(x,y,z);
	*m_pPosition=tmpPos;
	mFunction_UpdateRotation();
};

void NoiseCamera::GetPosition(NVECTOR3& out_vPos)
{
	out_vPos=*m_pPosition;
};

void	NoiseCamera::Move(NVECTOR3* vRelativePos)
{
	D3DXVec3Add(m_pPosition,m_pPosition,vRelativePos);
	SetPosition(m_pPosition);
};

void	NoiseCamera::Move(float relativeX,float relativeY,float relativeZ)
{
	NVECTOR3 tmpRelativePos(relativeX,relativeY,relativeZ);
	D3DXVec3Add(m_pPosition,m_pPosition,&tmpRelativePos);
	SetPosition(m_pPosition);
};

void	NoiseCamera::SetRotation(float RX_Pitch,float RY_Yaw,float RZ_Roll)//Ҫ����Lookat
{
	mRotateX_Pitch = RX_Pitch;
	mRotateY_Yaw = RY_Yaw;
	mRotateZ_Roll = RZ_Roll;
	mFunction_UpdateDirection();
};

void	NoiseCamera::SetRotationY_Yaw(float angleY)
{
	mRotateY_Yaw = angleY;
	mFunction_UpdateDirection();
};

void	NoiseCamera::SetRotationX_Pitch(float AngleX)
{
		mRotateX_Pitch = AngleX;
		mFunction_UpdateDirection();
};

void	NoiseCamera::SetRotationZ_Roll(float AngleZ)
{
	//roll��������Ҫ����lookat
	mRotateZ_Roll = AngleZ;
};

void	NoiseCamera::SetViewFrustumPlane(float iNearPlaneZ,float iFarPlaneZ)
{
	if ( (iNearPlaneZ >0) && (iFarPlaneZ>iNearPlaneZ))
	{
		mNearPlane	= iNearPlaneZ;
		mFarPlane	=	iFarPlaneZ;
		mCanUpdateProjMatrix =TRUE;
	}

};

void NoiseCamera::SetViewAngle(float iViewAngleY,float iAspectRatio)
{
	if(iViewAngleY>0 && (mViewAngleY <(MATH_PI/2))){mViewAngleY	=	iViewAngleY;	}
	if(iAspectRatio>0){mAspectRatio	= iAspectRatio;}
	mCanUpdateProjMatrix =TRUE;
};


/************************************************************************
											PRIVATE	
************************************************************************/

void	NoiseCamera::mFunction_UpdateProjMatrix()
{
	D3DXMatrixPerspectiveFovLH(
		m_pMatrixProjection,
		mViewAngleY,
		mAspectRatio,
		mNearPlane,
		mFarPlane);
	mCanUpdateProjMatrix = FALSE;
	//Ҫ���µ�GPU��TM��ȻҪ��ת��
	D3DXMatrixTranspose(m_pMatrixProjection,m_pMatrixProjection);
};

void	NoiseCamera::mFunction_UpdateViewMatrix()
{

	D3DXMATRIX	tmpMatrixTranslation;
	D3DXMATRIX	tmpMatrixRotation;
	//�ȶ���ԭ��
	D3DXMatrixTranslation(&tmpMatrixTranslation, -m_pPosition->x, -m_pPosition->y, -m_pPosition->z);
	//Ȼ����yawpitchroll������ת��view�ռ�
	D3DXMatrixRotationYawPitchRoll(&tmpMatrixRotation, mRotateY_Yaw, mRotateX_Pitch, mRotateZ_Roll);
	//���������ת������
	D3DXMatrixTranspose(&tmpMatrixRotation,&tmpMatrixRotation);
	//��ƽ�ƣ�����ת
	D3DXMatrixMultiply(m_pMatrixView,&tmpMatrixTranslation,&tmpMatrixRotation);
	//Ҫ���µ�GPU��TM��ȻҪ��ת��
	D3DXMatrixTranspose(m_pMatrixView,m_pMatrixView);

};

void	NoiseCamera::mFunction_UpdateRotation()
{
	//��Ҫ���ܣ����������Ҫ��Ϊ�˴���Direction�ı��������̬�Ǳ仯

	//����direction
	D3DXVECTOR3	tmpDirection;
	//�����direction�Ƿ�Ϊ0
	D3DXVec3Subtract(&tmpDirection,m_pLookat,m_pPosition);
	float mLength = D3DXVec3Length(&tmpDirection);
	//ע�⸡�������ӵ��λ�ò����غ�
	if (mLength<0.001)
	{
		//����������� ���� ������set�ó�������ȥ����
		mRotateX_Pitch = 0;
		mRotateY_Yaw = 0;
		mRotateZ_Roll = 0;
		*m_pDirection = NVECTOR3(1.0f, 0, 0);
		*m_pLookat = *m_pPosition + *m_pDirection;
		return;
	}
	else
	//�ӵ��λ�ò��غ� ���ٸ�ֵ
	{ *m_pDirection = tmpDirection; }
	;

	//��ʱ����������ı�ֵ����arctan���Pitch�Ƕ�
	float tmpRatio;
	//pitch�ǣ� tan = y/sqr(x^2+z^2))
	/*	ע�⣺	atanȡֵ��Χ�� [-pi/2,pi/2]  
					atan2ȡֵ��Χ�� [-pi,pi] 		*/
	if((m_pDirection->x==0) && (m_pDirection->z==0))
	{
		//�Ƿ���ԭ��������·�
		if(m_pDirection->y>=0)
		{mRotateX_Pitch=-MATH_PI/2;}
		else
		{mRotateX_Pitch=MATH_PI/2;}
	}
	else
	{
		//ȥ��� �ѵ�y����ʱ�������ǡ��������Ե�û��DX������ϵ������ϵ��ת�����������־���
		//�ô�Ĵָָ����ת��������ָָ������ת����
		tmpRatio =-m_pDirection->y /  sqrt(pow(m_pDirection->x,2.0f)+pow(m_pDirection->z,2.0f));
		mRotateX_Pitch = atan(tmpRatio);//����[-0.5pi,0.5pi]
	}

	//yaw�ǣ� tan = -x/z
		mRotateY_Yaw = atan2(m_pDirection->x,m_pDirection->z);//����ͼyaw��˳ʱ������

	//roll�ǣ�����direction����ı�roll�� 
	//roll��ʱ��ת������
};

void	NoiseCamera::mFunction_UpdateDirection()
{
	//��Ҫ���ܣ����������Ҫ��Ϊ�˴�����̬�Ǹı����������Direction�仯

	//Ҫ����Lookat
	float tmpDirectionLength = D3DXVec3Length(m_pDirection);
	//ֱ�������Ǻ������Direction	3dscanner������任һ��������
	m_pDirection->x =- tmpDirectionLength* sin(mRotateY_Yaw)* cos(mRotateX_Pitch);
	m_pDirection->z =tmpDirectionLength* cos(mRotateY_Yaw)*cos(mRotateX_Pitch);
	m_pDirection->y =tmpDirectionLength* sin(mRotateX_Pitch);
	D3DXVec3Add(m_pLookat,m_pPosition,m_pDirection);
};
