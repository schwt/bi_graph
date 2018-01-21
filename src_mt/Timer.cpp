#include "Timer.h"
#include <stdio.h>
void CTimer::StartTiming()
{
	time(&m_tBeginTime);
}
void CTimer::EndTiming()
{
	time(&m_tEndTime);
}
void CTimer::OutPutTimeSpan()
{
	if(m_tEndTime<m_tBeginTime)
		printf("Error!\n");
	long timespan=m_tEndTime-m_tBeginTime;
	long hours=timespan/3600;
	timespan%=3600;
	long minutes=timespan/60;
	long seconds=timespan%60;
	printf("It takes %ld hours,%ld minutes,%ld seconds.\n",hours,minutes,seconds);
}

string CTimer::GetCurrentTime()
{
	time_t cur_time;
	time(&cur_time);
	struct tm *p = gmtime(&cur_time);
	char str[1024];
	sprintf(str,"%d-%d-%d_%d-%d-%d",p->tm_year+1900,p->tm_mon+1,p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);
	return (string)str;
}

time_t CTimer::TimeStr2Time_t(const char* cstrTimeStr)
{
	struct tm t;
	if(!cstrTimeStr)
		return 0;
	memset(&t, 0, sizeof(t));
	sscanf(cstrTimeStr, "%04d%02d%02d-%02d:%02d:%02d", &(t.tm_year), &(t.tm_mon), &(t.tm_mday), &(t.tm_hour), &(t.tm_min), &(t.tm_sec));
	t.tm_year -= 1900;
	--t.tm_mon;
	return mktime(&t);
}

double CTimer::GetTimeSpan(const char* cstrTimeEnd,const char* cstrTimeBegin)
{
	time_t tBegin = TimeStr2Time_t(cstrTimeBegin);
	time_t tEnd = TimeStr2Time_t(cstrTimeEnd);
	return difftime(tEnd,tBegin);
}
