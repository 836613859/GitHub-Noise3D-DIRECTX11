
/***********************************************************************

                           h��NoiseTimer

************************************************************************/

#pragma once


namespace Noise3D
{
	namespace Ut
	{
		class /*_declspec(dllexport)*/ ITimer
		{
		public:

			//���캯��
			ITimer(NOISE_TIMER_TIMEUNIT timeUnit);

			void		SelfDestruction();

			//�ں��������const��ʾ�˺��������޸����Ա
			//��ͣ���ټ�
			double GetTotalTime()const;

			//��ȡ���ʱ��
			double GetInterval()const;

			//����ʱ�䵥λ
			void SetTimeUnit(NOISE_TIMER_TIMEUNIT timeUnit = NOISE_TIMER_TIMEUNIT_MILLISECOND);

			//������һ֡
			void NextTick();

			//....
			UINT	 GetFPS() const;

			//��ͣ��ʱ
			void Pause();

			void Continue();

			//������������
			void ResetAll();

			void ResetTotalTime();

		private:
			//���º����ʱ��(ms)
			double					mTotalTime;
			//����INTERVAL ��֡���ʱ����
			double					mDeltaTime;
			//ÿһcountռ�˶��ٺ���
			double					mMilliSecondsPerCount;
			//how many ticks are in current second
			UINT						mCurrentSecondTickCount;
			//a rounded total time
			UINT						mCurrentSecondInteger;
			//frames per second
			UINT						mFPS;

			//ʱ����
			NOISE_TIMER_TIMEUNIT	mTimeUnit;
			//
			BOOL					mIsPaused;
			//queryPerformanceò�Ʒ��ص��Ǿ���count����������query���count
			INT64					mPrevCount;
			//
			INT64					mCurrentCount;
			//
			INT64					mDeltaCount;
		};
	}
}