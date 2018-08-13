#!/bin/bash

HADOOP_DIR=""
HADOOP="${HADOOP_DIR}/bin/hadoop"
HADOOP_STREAM="${HADOOP_DIR}/share/hadoop/tools/lib/hadoop-streaming-2.7.3.jar"

job_name="bi_graph"
hdfs_tmp_dir=""
source_path=""
filter_recoIds=""                        # 如果结果需要过滤的，有效item集放此文件
output_file="./data/result.txt"

##### 使用数据天数
days=2

##### train parameters
rho=0.5
lambda=0.5
tau=4320000
confidence_rule=5
length=100
if_norm=1

##### 输入数据字段对应
idc_uid=0
idc_pid=1
idc_rate=2
idc_time=3


function timestamp {
    date +%s 
}
function timediff {
    old=$1
    new=`timestamp`
    dif=$[$new - $old]
    echo $dif
}
function datetime {
    date +"[%Y-%m-%d %H:%M:%S]"
}
function date_range {
    n=$1
    for i in `seq 1 $n`; do
        date -d "$i days ago" +"%Y-%m-%d"
    done
}

# 以 -input 连接的hdfs路径、日期
function path_list {
    path=$1
    days=$2
    dates=(`date_range ${days}`)
    echo -e "${path}${dates[0]} \c"
    new=(${dates[@]:1})
    for x in ${new[@]}; do
        echo -e "-input ${path}${x} \c"
    done
}

