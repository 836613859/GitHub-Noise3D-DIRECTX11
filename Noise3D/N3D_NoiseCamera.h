
/***********************************************************************

                           h��NoiseCamera

************************************************************************/

#pragma once

public class _declspec(dllexport) NoiseCamera
{
public:

	friend class NoiseScene;
	friend class NoiseRenderer;

	NoiseCamera();

	void				SelfDestruction();

	void				SetLookAt(NVECTOR3 vLookat);//Ҫ������ת��

	void				SetLookAt(float x,float y,float z);//Ҫ������ת��

	NVECTOR3	GetLookAt();

	void				SetPosition(NVECTOR3 vPos);

	void				SetPosition(float x,float y,float z);

	NVECTOR3	GetPosition();

	void				Move(NVECTOR3 vRelativePos);//pos and lookat

	void				Move(float relativeX,float relativeY,float relativeZ);

	void				SetViewFrustumPlane(float iNearPlaneZ,float iFarPlaneZ);

	void				SetViewAngle(float iViewAngleY,float iAspectRatio);

	void				SetRotation(float RX_Pitch,float RY_Yaw,float RZ_Roll);//Ҫ����Lookat

	void				SetRotationY_Yaw(float AngleX);//��setRotation��

	void				SetRotationX_Pitch(float AngleY);

	void				SetRotationZ_Roll(float AngleZ);

	float				GetRotationY_Yaw();

	float				GetRotationX_Pitch();

	float				GetRotationZ_Roll();

	void				RotateY_Yaw(float angleY);

	void				RotateX_Pitch(float angleX);

	void				RotateZ_Roll(float angleZ);

	void				fps_MoveForward(float fSignedDistance,BOOL enableYAxisMovement=FALSE);

	void				fps_MoveRight(float fSignedDistance, BOOL enableYAxisMovement=FALSE);

	void				fps_MoveUp(float fSignedDistance);

	/*
	void				sm_Update();
	void				sm_LinearMoveTo();
	void				sm_SineMoveTo();
	void				sm_RotateAroundAxis();*/


private:
	NoiseScene*			m_pFatherScene;

	void						mFunction_UpdateProjMatrix();
	void						mFunction_UpdateViewMatrix();
	void						mFunction_UpdateRotation();
	void						mFunction_UpdateDirection();
	float						mViewAngleY;
	float						mAspectRatio;
	float						mNearPlane;
	float						mFarPlane;
	NVECTOR3*			m_pPosition;
	NVECTOR3*			m_pLookat;
	NVECTOR3*			m_pDirection;

	NMATRIX*			m_pMatrixView;
	NMATRIX*			m_pMatrixProjection;

	float						mRotateX_Pitch;
	float						mRotateY_Yaw;
	float						mRotateZ_Roll;

};




