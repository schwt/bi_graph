#include "Logger.h"

Logger::~Logger() 
{
	log("done\n********************************************************\n\n");	
}

Logger::Logger(string file)
{
    SetLogFile(file, "");
}
Logger::Logger(string file, string name)
{
    SetLogFile(file, name);
}

bool Logger::SetLogFile(string file) 
{
    return SetLogFile(file, "");
}
bool Logger::SetLogFile(string file, string name) 
{
    m_log_file = file;
    m_pro_name = name;
	char cBuffer[1024]={0};
	
	time_t t_Time;
	struct tm *tm_time;
	t_Time=time(NULL);
	tm_time=localtime(&t_Time);		

	FILE *fpLog;
	if((fpLog=fopen(m_log_file.c_str(),"at"))==NULL)
	{	
		sprintf(cBuffer,"%02d-%02d-%02d_%02d:%02d:%02d [ ERROR ] [Msg:Open Log.txt File] [Line:%d]\n",
		        tm_time->tm_year+1900, tm_time->tm_mon+1, tm_time->tm_mday, 
				tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, __LINE__);
				
		printf("%s", cBuffer);
		fprintf(fpLog, "%s", cBuffer);
        return false;
	} else {

	    if (name!="")
	    {
	    	printf("\n\n*************************************************************\n");
	    	printf("**************************** %s ****************************\n", name.c_str());
	    	fprintf(fpLog,"\n\n*************************************************************\n");
	    	fprintf(fpLog,"**************************** %s ****************************\n", name.c_str());
	    }

	    fclose(fpLog);
	    fpLog=NULL;
    }
    return true;
}


bool Logger::log(string sMessages) {
    return iwrite("", "", sMessages);
}
bool Logger::log(int nLine, string sMessages) {
    char cBuffer[1024] = {0};
    snprintf(cBuffer, 1024, "[line:%d]", nLine);
    return iwrite(cBuffer, "", sMessages);
}
bool Logger::log(bool bReturn, string sMessages) {
    string flag = "[OK]";
    if (!bReturn) flag = "[ERROR]";
    return iwrite("", flag, sMessages);
}
bool Logger::log(int nLine, bool bReturn, string sMessages) {
    char cBuffer[1024] = {0};
    snprintf(cBuffer, 1024, "[line:%d]", nLine);
    string flag = "[OK]";
    if (!bReturn) flag = "[ERROR]";
    return iwrite(cBuffer, flag, sMessages);
}
    
//日志信息
bool Logger::iwrite(string nLine, string bReturn, string sMessages)
{
	char cBuffer[1024]={0};
	
	time_t t_Time;
	struct tm *tm_time;
	t_Time=time(NULL);
	tm_time=localtime(&t_Time);	

	FILE *fpLog;
	if((fpLog=fopen(m_log_file.c_str(),"at"))==NULL)
	{	
		sprintf(cBuffer, "[%02d-%02d-%02d %02d:%02d:%02d][ERROR]%s  Open Log.txt File\n", 
		        tm_time->tm_year+1900, tm_time->tm_mon+1, tm_time->tm_mday, 
				tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, nLine.c_str());
				
		printf("%s", cBuffer);
		fprintf(fpLog, "%s", cBuffer);
		return false;
	}

    sprintf(cBuffer,"[%02d-%02d-%02d %02d:%02d:%02d]%s%s %s\n", 
            tm_time->tm_year+1900, tm_time->tm_mon+1, tm_time->tm_mday, 
    		tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec, 
    		bReturn.c_str(), nLine.c_str(), sMessages.c_str());
    		
    printf("%s", cBuffer);
    fprintf(fpLog, "%s", cBuffer);

	fclose(fpLog);
	fpLog=NULL;

	return true;
}


