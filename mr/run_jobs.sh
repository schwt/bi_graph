#!/bin/bash
#!encoding:utf-8

dir0=$(cd $(dirname $0); pwd)
cd $dir0

. ./config.sh
. ./scripts/utils.sh

t0=`timestamp`
datetime
if ! ./scripts/submit_mr1.sh; then
    exit 1; fi

if ! ./scripts/submit_mr2.sh; then
    exit 1; fi

if ! ./scripts/submit_mr3.sh; then
    exit 1; fi

tt=`timediff $t0`
echo "`datetime` total time: `second2formated ${tt}` (${tt}s)"

mv ${output_file} ${output_file}.bak
hadoop fs -getmerge ${hdfs_tmp_dir}/output3 ${output_file}


