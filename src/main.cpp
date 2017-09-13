#include "spear.h"

int main(int argc, char* argv[])
{
    bool b_rt = true;
    if(argc < 2)
    {
        printf("[Error]:Para format error! Use 'exe config_file_path log_path'.\n");
        return 1;
    }

    CTimer timer;
    timer.StartTiming();

    CSpear g_obj_spear;

    b_rt = g_obj_spear.Init(argv[1]);
    if (!b_rt) return -1;
    
    b_rt = g_obj_spear.Calc();
    if (!b_rt) return -1;
    timer.EndTiming();
    timer.OutPutTimeSpan();
    


    printf("OK!\n");
    return 0;
}
