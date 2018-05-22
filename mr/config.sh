#!/bin/bash
source /home/idm/wyb/lib/lib_sh/environment.sh

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

job_name="bi_graph"
hdfs_tmp_dir="hdfs://xxxx/sim_item/temp"
source_data="hdfs://yyy"
output_file="./data/result.txt"

##### parameters
rho=0.5         # punish on hot user
lambda=0.7      # punish on hot item
tau=86400       # 1 day
length=30
if_norm=0

