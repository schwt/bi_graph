#!/bin/bash


source /home/idm/wyb/lib/lib_sh/environment.sh
source /home/idm/wyb/lib/lib_sh/utils.sh

job_name="bi_graph"
hdfs_tmp_dir="hdfs://hz-cluster6/user/kaolarec/wyb/mr/sim_item/temp"
source_data="hdfs://hz-cluster6/user/kaolarec/hive_db/kaola_rec_algo.db/wyb_action_goods/day=2018-05-13"
output_file="./data/result.txt"

##### parameters
rho=0.5
lambda=0.5
tau=864000
length=20
if_norm=1

