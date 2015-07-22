
/***********************************************************************

							�ࣺNOISE TIMER

	�������߾��ȼ�ʱ������Ҫ��WINAPI��queryPerformanceCount��ʵ��    

***********************************************************************/
#include "Noise3D.h"

NoiseTimer::NoiseTimer(NOISE_TIMER_TIMEUNIT timeUnit = NOISE_TIMER_TIMEUNIT_MILLISECOND)
{
	//Ĭ���ú�����
	mTimeUnit				= timeUnit;
	mMilliSecondsPerCount	= 0.0;
	mDeltaTime				= 0.0;
	mTotalTime				= 0.0;
	mIsPaused				= FALSE;

	//ÿ����������ٴ�
	INT64 countsPerSecond;
	//��ȡ���������ʱ����Ƶ��
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	mMilliSecondsPerCount = (1000.0) /(double)countsPerSecond;//ÿһcount���ٺ���
	QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);
};

void NoiseTimer::Next()
{
	if(mIsPaused)
	{
		mDeltaTime = 0.0;
	}
	//���û����ͣ
	else
	{
		//����count
		mPrevCount = mCurrentCount;
		QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);

		//�����ʡ��ģʽ�£����л����������ܻᵼ��countsҲ�Ǹ���
		mDeltaCount = mCurrentCount - mPrevCount;
		BOOL isDeltaTimePositive = ((mDeltaCount) > 0);
		if(isDeltaTimePositive)
		{
			mDeltaTime =(double)(mDeltaCount * mMilliSecondsPerCount);
		}
		else
		{
			mDeltaTime = 0.0;
		};

		//û��ͣ�͸�����ʱ�� ��λ��ms
		mTotalTime += mDeltaTime;
	};
};

double NoiseTimer::GetTotalTime()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mTotalTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mTotalTime/1000); 
		break;
	};
	return 0;
};

double NoiseTimer::GetInterval()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mDeltaTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mDeltaTime/1000); 
		break;
	};
	return 0;
};

void NoiseTimer::SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit)
{
	if (timeUnit ==NOISE_TIMER_TIMEUNIT_SECOND||timeUnit==NOISE_TIMER_TIMEUNIT_MILLISECOND)
	{mTimeUnit = timeUnit;};
};

void NoiseTimer::Pause()
{
	mIsPaused = TRUE;
};

void NoiseTimer::Continue()
{
	mIsPaused = FALSE;
};

void NoiseTimer::ResetAll()
{
	mTotalTime	= 0.0;
	mDeltaTime	= 0.0;
	mIsPaused	= FALSE;
};

void NoiseTimer::ResetTotalTime()
{
	mTotalTime = 0;
};
