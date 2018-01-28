#include "bi_graph.h"

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

    BiGraph obj_bi_graph;

    b_rt = obj_bi_graph.Init(argv[1]);
    if (!b_rt) return -1;
    
    b_rt = obj_bi_graph.Calc();
    if (!b_rt) return -1;
    timer.EndTiming();
    timer.OutPutTimeSpan();

    return 0;
}
