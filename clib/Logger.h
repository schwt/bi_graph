#ifndef MYLOGMESSAGE_H
#define MYLOGMESSAGE_H

#include <stdio.h>   
#include <stdlib.h>   
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
using namespace std;

class Logger
{ 
public: 
	Logger() {}
	Logger(string file);
	Logger(string file, string name);
	virtual ~Logger();

public:
	bool SetLogFile(string file);
	bool SetLogFile(string file, string name);
	bool log(string sMessages);
	bool log(bool bReturn, string sMessages);
	bool log(int nline, string sMessages);
	bool log(int nline, bool bReturn, string sMessages);

private:
	bool iwrite(string nline, string bReturn, string sMessages);//输出日志内容
	string m_log_file;
	string m_pro_name;
};
#endif

