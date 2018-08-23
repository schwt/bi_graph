#!/bin/bash
dir=$(cd $(dirname $0); pwd)
. `pwd`/config.sh
. ${dir}/utils.sh

cd $dir
name="${job_name}_job1"
INPUT=`path_list ${source_path} ${days}`
OUTPUT="${hdfs_tmp_dir}/output1"
mapper="mapper1.py"
reducer="reducer1.py"
LAMBDA="${lambda}"

echo "`datetime` start Job: ${name}"
echo "`datetime` dates:"
date_range_filter ${days} ${black_dates}

function main {

    javaOpt=" -Xms2012m -Xmx2012m -XX:MaxPermSize=256m -XX:-UseGCOverheadLimit -XX:+UseConcMarkSweepGC -XX:MaxDirectMemorySize=256m"

    ${HADOOP} fs -rm -r ${OUTPUT}

    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.maps=${task_m1}        \
            -D mapreduce.job.reduces=${task_r1}     \
            -D mapreduce.map.memory.mb=${mem_m1}    \
            -D mapreduce.reduce.memory.mb=${mem_r1} \
            -D mapreduce.jobtracker.maxreducememory.mb=8192 \
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapreduce.job.name="${name}"  \
            -mapper  "python ${mapper} ${idc_uid} ${idc_pid} ${idc_rate} ${idc_time} ${rate_min} ${rate_max}" \
            -reducer "python ${reducer} ${LAMBDA}" \
            -file "${mapper}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}


t0=`timestamp`
main
tt=`timediff $t0`
echo "`datetime` job1 time: `second2formated ${tt}` (${tt}s)"
echo

