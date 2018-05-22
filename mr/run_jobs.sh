#!/bin/bash
#!encoding:utf-8

dir0=$(cd $(dirname $0); pwd)
cd $dir0

. ./config.sh

t0=`timestamp`
datetime
./submit_mr1.sh

./submit_mr2.sh

./submit_mr3.sh

tt=`timediff $t0`
echo "total time: ${tt}s"

hadoop fs -getmerge ${hdfs_tmp_dir}/output3 ${output_file}


