
/***********************************************************************

                           h��NoiseTimer

************************************************************************/

#pragma once


public class _declspec(dllexport) NoiseUtTimer
{
public:

	//���캯��
	NoiseUtTimer(NOISE_TIMER_TIMEUNIT timeUnit);

	//�ں��������const��ʾ�˺��������޸����Ա
	//��ͣ���ټ�
	double GetTotalTime()const;

	//��ȡ���ʱ��
	double GetInterval()const;

	//����ʱ�䵥λ
	void SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit);

	//������һ֡
	void Next();

	//��ͣ��ʱ
	void Pause();

	void Continue();	

	//������������
	void ResetAll();

	void ResetTotalTime();

private:
	//���º����ʱ��
	double					mTotalTime;
	//����INTERVAL ��֡���ʱ����
	double					mDeltaTime;				
	//ÿһcountռ�˶��ٺ���
	double					mMilliSecondsPerCount;	
	//ʱ����
	NOISE_TIMER_TIMEUNIT	mTimeUnit; 
	//
	BOOL					mIsPaused;
	//queryPerformanceò�Ʒ��ص��Ǿ���count����������query���count
	INT64					mPrevCount	;		
	//
	INT64					mCurrentCount;
	//
	INT64					mDeltaCount;
};
