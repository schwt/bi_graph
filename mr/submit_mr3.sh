#!/bin/bash
. ./config.sh

name="${job_name}_job3"
INPUT="${hdfs_tmp_dir}/output2"
OUTPUT="${hdfs_tmp_dir}/output3"
reducer="reducer3.py"

Length="${length}"
If_norm="${if_norm}"

function main {

    javaOpt=" -Xms2012m -Xmx2012m -XX:MaxPermSize=256m -XX:-UseGCOverheadLimit -XX:+UseConcMarkSweepGC -XX:MaxDirectMemorySize=256m"

    ${HADOOP} fs -rm -r ${OUTPUT}

    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.maps=200 \
            -D mapreduce.job.reduces=1000 \
            -D mapreduce.reduce.memory.mb=8192\
            -D mapreduce.map.memory.mb=8192\
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapreduce.jobtracker.maxreducememory.mb=8192 \
            -D mapred.job.name="${name}"  \
            -mapper 'cat' \
            -reducer "python ${reducer} ${Length} ${If_norm}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}

main

