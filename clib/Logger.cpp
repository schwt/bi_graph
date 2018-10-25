#include "Logger.h"

Logger::~Logger() 
{
	log("\n********************************************************\n\n");	
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

    char time_s[64] = {0};
    time_t timep;
    time (&timep);
    strftime(time_s, sizeof(time_s), "%Y-%m-%d %H:%M:%S",localtime(&timep) );

	FILE *fpLog;
	if((fpLog=fopen(m_log_file.c_str(),"at"))==NULL)
	{	
		sprintf(cBuffer,"%s [ ERROR ] [Msg:Open Log.txt File] [Line:%d]\n", time_s, __LINE__);
				
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
	
    char time_s[64] = {0};
    time_t timep;
    time (&timep);
    strftime(time_s, sizeof(time_s), "%Y-%m-%d %H:%M:%S",localtime(&timep) );

	FILE *fpLog;
	if((fpLog=fopen(m_log_file.c_str(),"at"))==NULL)
	{	
		sprintf(cBuffer, "[%s][ERROR]%s  Open Log.txt File\n", time_s, nLine.c_str());
				
		printf("%s", cBuffer);
		fprintf(fpLog, "%s", cBuffer);
		return false;
	}

    sprintf(cBuffer,"[%s]%s%s %s\n", time_s, bReturn.c_str(), nLine.c_str(), sMessages.c_str());
    		
    printf("%s", cBuffer);
    fprintf(fpLog, "%s", cBuffer);

	fclose(fpLog);
	fpLog=NULL;

	return true;
}


