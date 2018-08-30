#ifndef __TIMER_H__
#define __TIMER_H__

#include <time.h>
#include <string>
using namespace std;

// 标准字符串格式： 
//     "2014-08-10 19:30:00"
class CTimer
{
public:
    void StartTiming();             // 开始时间打点
    void EndTiming();               // 结束时间打点
    long UsedSeconds();             // 返回耗时，秒数
    void OutPutTimeSpan();          // 打印耗时（"小时数:分:秒")
    string GetCurrentTime();        // 获取当前时间, 格式化
    static double GetTimeSpan(const char* cstrTimeEnd,const char* cstrTimeBegin); //unit: second
private:
    static time_t TimeStr2Time_t(const char* cstrTimeStr);
private:
    time_t m_tBeginTime;    // 开始时间戳
    time_t m_tEndTime;      // 结束时间戳
    time_t m_tDiffTime;     // 时间差，秒
};

#endif //__TIMER_H__

