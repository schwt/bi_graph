#include "usercf.h"

int main(int argc, char* argv[])
{
    bool b_rt = true;
    if(argc < 3)
    {
        printf("[Error]:Para format error! Use 'exe config_file_path log_path'.\n");
        return 1;
    }

    CTimer timer;
    timer.StartTiming();

    CUserCF g_obj_usercf;

    b_rt = g_obj_usercf.Init(argv[1],argv[2]);
    if (!b_rt) return -1;
    
    b_rt = g_obj_usercf.Calc();
    if (!b_rt) return -1;
    timer.EndTiming();
    timer.OutPutTimeSpan();
    


    printf("OK!\n");
    return 0;
}
