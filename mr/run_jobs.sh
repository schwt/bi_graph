#!/bin/bash
#!encoding:utf-8


dir0=$(cd $(dirname $0); pwd)
cd $dir0

. ./config.sh

datetime

./submit_mr1.sh
datetime

./submit_mr2.sh
datetime

./submit_mr3.sh
datetime


hadoop fs -getmerge ${hdfs_tmp_dir}/output3 ${output_file}
datetime


