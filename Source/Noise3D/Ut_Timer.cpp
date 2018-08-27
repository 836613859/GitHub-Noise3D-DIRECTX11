
/***********************************************************************

							�ࣺNOISE TIMER

	�������߾��ȼ�ʱ������Ҫ��WINAPI��queryPerformanceCount��ʵ��    

***********************************************************************/
#include "Noise3D.h"

using namespace Noise3D;
using namespace Noise3D::Ut;

Timer::Timer()
	:mTimeUnit(NOISE_TIMER_TIMEUNIT_MILLISECOND),
	mMilliSecondsPerCount(0.0),
	mDeltaTime(0.0),
	mTotalTime(0.0),
	mMaxInterval(100000.0f),
	mTimeScaleFactor(1.0f),
	mIsPaused(false),
	mFPS(0)
{
	//ÿ����������ٴ�
	INT64 countsPerSecond = 0;
	//get the FREQUENCY of this counter
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	mMilliSecondsPerCount = (1000.0) / (double)countsPerSecond;//ÿһcount���ٺ���
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mPrevCount);
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);
}

Timer::Timer(NOISE_TIMER_TIMEUNIT timeUnit = NOISE_TIMER_TIMEUNIT_MILLISECOND)
	:mTimeUnit(timeUnit),
	mMilliSecondsPerCount(0.0),
	mDeltaTime(0.0),
	mTotalTime(0.0),
	mMaxInterval(100000.0f),
	mTimeScaleFactor(1.0f),
	mIsPaused(false),
	mFPS(0)
{
	//ÿ����������ٴ�
	INT64 countsPerSecond=0;
	//get the FREQUENCY of this counter
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	mMilliSecondsPerCount = (1000.0) /(double)countsPerSecond;//ÿһcount���ٺ���
		NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mPrevCount);
	NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);
}

//elapse time . and time interval will be scaled
void Timer::NextTick()
{
	//used to indicate that it's time to update FPS
	static double normalTotalTime = 0.0;

	if(mIsPaused)
	{
		mDeltaTime = 0.0;
	}
	//���û����ͣ
	else
	{
		//����count
		mPrevCount = mCurrentCount;
		NOISE_MACRO_FUNCTION_WINAPI QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);

		//�����ʡ��ģʽ�£����л����������ܻᵼ��countsҲ�Ǹ���
		mDeltaCount = mCurrentCount - mPrevCount;
		bool isDeltaTimePositive = ((mDeltaCount) > 0);
		if(isDeltaTimePositive)
		{
			mDeltaTime =(double)(mDeltaCount * mMilliSecondsPerCount);
		}
		else
		{
			mDeltaTime = 0.0;
		};

		//Update Scaled Total Time/Normal Total Time if not pause
		mTotalTime += mDeltaTime *mTimeScaleFactor;
		normalTotalTime += mDeltaTime;

		//accumulate ticks count within one sec
		++mCurrentSecondTickCount;

		//check if total time had round down to a bigger integer,then compute FPS of last second
		//but we dont use scaled time (but normalTotalTime), because FPS is based on real time elapsed
		if (mCurrentSecondInteger != UINT(normalTotalTime /1000.0))
		{
			mFPS = mCurrentSecondTickCount;
			mCurrentSecondTickCount = 0;//reset
			//current second integer = total fractional time round down to integer
			mCurrentSecondInteger = UINT(normalTotalTime / 1000.0);
		};
		
	};
}

UINT Timer::GetFPS() const
{
	return mFPS;
};

//scaled elapsed time is counted in
double Timer::GetTotalTimeElapsed()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return mTotalTime; 
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return (mTotalTime/1000.0); 
		break;
	};
	return 0;
};

//return time is scaled
double Timer::GetInterval()const
{
	switch(mTimeUnit)
	{
	case NOISE_TIMER_TIMEUNIT_MILLISECOND:
		return Clamp(mDeltaTime,0.0,mMaxInterval)*mTimeScaleFactor;
		break;
	case NOISE_TIMER_TIMEUNIT_SECOND:
		return Clamp(mDeltaTime/1000.0, 0.0, mMaxInterval/1000.0)*mTimeScaleFactor;
		break;
	};
	return 0;
};

//select milli-sec or second
void Timer::SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit)
{
	if (timeUnit ==NOISE_TIMER_TIMEUNIT_SECOND||timeUnit==NOISE_TIMER_TIMEUNIT_MILLISECOND)
	{mTimeUnit = timeUnit;};
};

void Timer::Pause()
{
	mIsPaused = true;
};

void Timer::Continue()
{
	mIsPaused = false;
};

void Timer::ResetAll()
{
	mDeltaTime = 0.0;
	mTotalTime = 0.0;
	mMaxInterval = 100000.0f;
	mTimeScaleFactor = 1.0f;
	mIsPaused = false;
	mFPS = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&mPrevCount);
	QueryPerformanceCounter((LARGE_INTEGER*)&mCurrentCount);
};

void Timer::ResetTotalTime()
{
	mTotalTime = 0;
}

void Timer::SetTimeIntervalClamp(double maxInterval)
{
	if (maxInterval > 0.0f)
	{
		mMaxInterval = maxInterval;
	}
}

void Timer::SetTimeScale(double scaleFactor)
{
	mTimeScaleFactor = scaleFactor;//could be neg/0/pos
}

double Timer::GetTimeScale() const
{
	return mTimeScaleFactor;
}
;

