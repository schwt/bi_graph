#!/bin/bash
. ./config.sh

name="${job_name}_job1"
INPUT="${source_data}"
OUTPUT="${hdfs_tmp_dir}/output1"
mapper="mapper1.py"
reducer="reducer1.py"
LAMBDA="${lambda}"


function main {

    javaOpt=" -Xms2012m -Xmx2012m -XX:MaxPermSize=256m -XX:-UseGCOverheadLimit -XX:+UseConcMarkSweepGC -XX:MaxDirectMemorySize=256m"

    ${HADOOP} fs -rm -r ${OUTPUT}

    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.maps=200 \
            -D mapreduce.job.reduces=100 \
            -D mapreduce.map.memory.mb=8192\
            -D mapreduce.reduce.memory.mb=8192\
            -D mapreduce.jobtracker.maxreducememory.mb=8192 \
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapreduce.job.name="${name}"  \
            -mapper "python ${mapper}" \
            -reducer "python ${reducer} ${LAMBDA}" \
            -file "${mapper}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}


t0=`timestamp`
main
datetime
tt=`timediff $t0`
echo "job1 time: ${tt}s"

