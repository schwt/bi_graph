#!/bin/bash


source /home/xxx/lib/lib_sh/environment.sh
source /home/xxx/lib/lib_sh/utils.sh

job_name="bi_graph"
hdfs_tmp_dir="hdfs://xxxx/temp"
source_data="hdfs://yyyyy/day=2018-05-13"
output_file="./data/result.txt"

##### parameters
rho=0.5
lambda=0.5
tau=864000
length=20
if_norm=1

