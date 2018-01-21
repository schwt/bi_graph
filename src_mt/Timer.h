#ifndef __TIMER_H__
#define __TIMER_H__

#include <time.h>
#include <string>
using namespace std;

class CTimer
{
public:
	void StartTiming();
	void EndTiming();
	void OutPutTimeSpan();
	string GetCurrentTime();
	static double GetTimeSpan(const char* cstrTimeEnd,const char* cstrTimeBegin);//unit is second
private:
	static time_t TimeStr2Time_t(const char* cstrTimeStr);
private:
	time_t m_tBeginTime;
	time_t m_tEndTime;
};

#endif //__TIMER_H__
