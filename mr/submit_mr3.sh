#!/bin/bash
. ./config.sh

name="${job_name}_job3"
INPUT="${hdfs_tmp_dir}/output2"
OUTPUT="${hdfs_tmp_dir}/output3"
FILTER="${hdfs_tmp_dir}/filter_recoIds.txt"
FILTER_NAME="filterIds"
mapper="mapper3.py"
reducer="reducer3.py"

RHO="${rho}"
Tau="${tau}"
Length="${length}"
If_norm="${if_norm}"

echo "`datetime` start Job: ${name}"
function main {

    javaOpt=" -Xms2012m -Xmx2012m -XX:MaxPermSize=256m -XX:-UseGCOverheadLimit -XX:+UseConcMarkSweepGC -XX:MaxDirectMemorySize=256m"

    ${HADOOP} fs -rm -r ${OUTPUT}
    if [ -f ${filter_recoIds} ]; then
        ${HADOOP} fs -rm -r ${FILTER}
        ${HADOOP} fs -put ${filter_recoIds} ${FILTER}
    fi

    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.reduce.input.buffer.percent=0.3 \
            -D mapreduce.job.maps=500 \
            -D mapreduce.job.reduces=300 \
            -D mapreduce.map.memory.mb=3072 \
            -D mapreduce.reduce.memory.mb=3072 \
            -D mapreduce.jobtracker.maxreducememory.mb=8192\
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapred.job.name="${name}"  \
            -cacheFile "${FILTER}#${FILTER_NAME}"    \
            -mapper "python ${mapper} ${RHO} ${Tau} ${FILTER_NAME}" \
            -reducer "python ${reducer} ${Length} ${If_norm}" \
            -file "${mapper}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}


t0=`timestamp`
main
tt=`timediff $t0`
echo "`datetime` job3 time: ${tt}s"

