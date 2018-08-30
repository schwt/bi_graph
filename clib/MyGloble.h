#ifndef MYGLOBLE_H_
#define MYGLOBLE_H_

#include <assert.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>  
#include <utility>
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <time.h>
#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>
#include <time.h>
#include <sys/types.h>   
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>

using namespace std;

typedef unsigned int UINT;
typedef unsigned short WORD;
typedef unsigned char  BYTE;

//文件删除
inline bool RemoveFile(const string& fpath)
{
	bool res = true;	
	if (remove(fpath.c_str()) < 0)//
	{
		res = false;
		cout << "[ERROR] remove file: "<< fpath << " line=" << endl;
	}
	else
	{
		res = true;
		cout << "[OK] remove file: " << fpath<<" line=" << endl;
	}	
	return res;
}
	
///////////////////time/////////////////////////////
inline int GetTimeStamp(const char *datetime)
{		
	int Y, m, d, H, M, S;
	Y = m = d = H = M = S = 0;
	sscanf(datetime, "%d-%d-%d %d:%d:%d", &Y, &m, &d, &H, &M, &S);	
	if (m <= 0)
	{
		sscanf(datetime, "%d年%d月%d日%d:%d:%d", &Y, &m, &d, &H, &M, &S);	
	}		
	return Y * 10000 + m * 100 + d;
}

inline void GetDateAndTime(const char * strTime, int& date, int& time)
{		
	int Y,m,d,H,M,S;
	Y=m=d=H=M=S=0;
	sscanf(strTime, "%d-%d-%d %d:%d:%d", &Y,&m,&d,&H,&M,&S);	
	if (m<=0)
	{
		sscanf(strTime, "%d年%d月%d日%d:%d:%d", &Y,&m,&d,&H,&M,&S);	
	}		
	date = Y*10000+m*100+d;
    time = (H+10)*10000+M*100+S;
}

//时间差 return天
inline int DiffDateTime(int datetime_now, int datetime_before)
{
	struct tm tm_cur, tm_before;
	time_t time_t_now, time_t_before;
	int res;

	tm_before.tm_year  = datetime_before / 10000 - 1900;
	tm_before.tm_mon   = datetime_before % 10000 / 100 - 1;
	tm_before.tm_mday  = datetime_before % 10000 % 100;
	tm_before.tm_hour  = 0;
	tm_before.tm_min   = 0;
	tm_before.tm_sec   = 1 ;
	tm_before.tm_isdst = -1;
	time_t_before      = mktime(&tm_before);

	tm_cur.tm_year  = datetime_now / 10000 - 1900;
	tm_cur.tm_mon   = datetime_now % 10000 / 100 - 1;
	tm_cur.tm_mday  = datetime_now % 10000 % 100;
	tm_cur.tm_hour  = 0;
	tm_cur.tm_min   = 0;
	tm_cur.tm_sec   = 1;
	tm_cur.tm_isdst = -1;
	time_t_now      = mktime(&tm_cur);

	res = (int)(difftime(time_t_now, time_t_before) / (24 * 60 * 60));
	//cout << res <<endl;
	
	return res;
}
//时间差 return分钟
inline int DiffDateTime(int tmNow_YMD, int tmNow_HMS, int tmBefore_YMD, int tmBefore_HMS)
{
	struct tm currentTime, beforeTime;
	time_t now,before;
	int rs;

	beforeTime.tm_year=tmBefore_YMD/10000-1900;
	beforeTime.tm_mon=tmBefore_YMD%10000/100-1;
	beforeTime.tm_mday=tmBefore_YMD%10000%100;
	beforeTime.tm_hour=tmBefore_HMS/10000-10;
	beforeTime.tm_min=tmBefore_HMS%10000/100;
	beforeTime.tm_sec=tmBefore_HMS%10000%100;
	beforeTime.tm_isdst=-1;
	before=mktime(&beforeTime);

	currentTime.tm_year=tmNow_YMD/10000-1900;
	currentTime.tm_mon=tmNow_YMD%10000/100-1;
	currentTime.tm_mday=tmNow_YMD%10000%100;
	currentTime.tm_hour=tmNow_HMS/10000-10;
	currentTime.tm_min=tmNow_HMS%10000/100;
	currentTime.tm_sec=tmNow_HMS%10000%100;
	currentTime.tm_isdst=-1;
	now=mktime(&currentTime);

	rs=static_cast<int> (difftime(now,before)/60);

	return rs;
}

// check idx-ivt
template<class IdxNode, class IvtNode>
inline bool IdxIvtCheck(const string& fpath_idx, const string& fpath_ivt)
{
	cout<<"@ IdxIvtCheck " << endl;
	size_t read_size = 0;
	
	// idx
	FILE *fp_idx = NULL;
	if ((fp_idx = fopen(fpath_idx.c_str(),"rb")) == NULL)
	{
		cout << "@ [Error] open file: " << fpath_idx << " " << endl;
		return false;
	}
	
	IdxNode *read_buffer_idx = new IdxNode[10240];
	int cnt_idx = 0;
	while (!feof(fp_idx))
	{
		read_size = fread(read_buffer_idx, sizeof(IdxNode), 10240,fp_idx);
		for (size_t i=0; i<read_size; ++i)
		{	
			cnt_idx += read_buffer_idx[i].count;
		}
		if (read_size < 10240) break;
		memset(read_buffer_idx, 0, 10240);//清零
	}
	fclose(fp_idx);
	delete []read_buffer_idx;
	read_buffer_idx = NULL;

	// ivt
	FILE *fp_ivt = NULL;
	if ((fp_ivt = fopen(fpath_ivt.c_str(),"rb")) == NULL)
	{
		cout << "@ [Error] open file: " << fpath_ivt << " " << endl;
		return false;
	}

	IvtNode *read_buffer_ivt = new IvtNode[10240];
	int cnt_ivt = 0;
	while (!feof(fp_ivt))
	{
		read_size = fread(read_buffer_ivt, sizeof(IvtNode), 10240, fp_ivt);
		cnt_ivt += read_size;
		if (read_size < 10240) break;
		memset(read_buffer_ivt, 0, 10240);//清零
	}
	fclose(fp_ivt);
	delete []read_buffer_ivt;
	read_buffer_ivt = NULL;

	char prt_buffer[1024] = {0};
	sprintf(prt_buffer, "@ cnt_idx: %d  cnt_ivt: %d\n", cnt_idx, cnt_ivt);
	cout << prt_buffer << endl;

	if (cnt_idx != cnt_ivt)	return false;
	else return true;
}

template<class IdxNode, class IvtNode>
inline bool IdxIvtCheck(const vector<IdxNode>& vec_idx, const string& fpath_ivt)
{
	cout << "@ IdxIvtCheck" << endl;

	// idx_cnt
	int cnt_idx = 0;
	for (UINT i = 0; i < vec_idx.size(); ++i)
	{
		cnt_idx += vec_idx[i].count;		
	}
	
	// ivt_cnt
	FILE *fp_ivt = NULL;
	if ((fp_ivt = fopen(fpath_ivt.c_str(), "rb"))==NULL)
	{
		cout << "[Error] open file: " << fpath_ivt << " " << endl;
		return false;
	}

	IvtNode *read_buffer = new IvtNode[10240];
	int cnt_ivt      = 0;
	size_t read_size = 0;
	
	while (!feof(fp_ivt))
	{
		read_size = fread(read_buffer, sizeof(IvtNode), 10240, fp_ivt);
		cnt_ivt += read_size;
		if (read_size < 10240) break;		
		memset(read_buffer, 0, 10240);		
	}
	
	fclose(fp_ivt);
	delete []read_buffer;
	read_buffer = NULL;

	char prt_buffer[1024] = {0};
	sprintf(prt_buffer, "@ cnt_idx: %d  cnt_ivt: %d\n", cnt_idx, cnt_ivt);
	cout << prt_buffer << endl;
	
	if (cnt_idx != cnt_ivt)	return false;
	else return true;
}

#endif
