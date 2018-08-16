#!/bin/bash

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
function date_range_filter {
    days=$1
    black_list=$2
    for i in `seq 1 $days`; do
        d=`date -d "$i days ago" +"%Y-%m-%d"`
        if ! [[ $black_list =~ $d ]]; then
            echo $d
        fi  
    done
}

# 以 -input 连接的hdfs路径、日期
function path_list {
    path=$1
    days=$2
    dates=(`date_range_filter ${days} ${black_dates}`)
    echo -e "${path}${dates[0]} \c"
    new=(${dates[@]:1})
    for x in ${new[@]}; do
        echo -e "-input ${path}${x} \c"
    done
}

function second2formated {
    seconds=$1
    h=$[seconds/3600]
    seconds=$[seconds-(h*3600)]
    m=$[seconds/60]
    s=$[seconds%60]
    echo "$h:$m:$s"
}

## test
# date_range_filter ${days} ${black_dates}
# second2formated 3699

