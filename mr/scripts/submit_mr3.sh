#!/bin/bash
dir=$(cd $(dirname $0); pwd)
. `pwd`/config.sh
. ${dir}/utils.sh

cd ${dir}
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
        echo "filter file: ${filter_recoIds}"
        ${HADOOP} fs -rm -r ${FILTER}
        ${HADOOP} fs -put ${filter_recoIds} ${FILTER}
        submit_with_filter
    else
        submit_no_filter
    fi
}

function submit_with_filter {
    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.reduce.input.buffer.percent=0.3 \
            -D mapreduce.job.maps=${task_m3}        \
            -D mapreduce.job.reduces=${task_r3}     \
            -D mapreduce.map.memory.mb=${mem_m3}    \
            -D mapreduce.reduce.memory.mb=${mem_r3} \
            -D mapreduce.jobtracker.maxreducememory.mb=8192\
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapred.job.name="${name}"  \
            -cacheFile "${FILTER}#${FILTER_NAME}"    \
            -mapper "python ${mapper} ${RHO} ${Tau} ${FILTER_NAME}" \
            -reducer "python ${reducer} ${Length} ${If_norm} ${confidence_rule}" \
            -file "${mapper}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}
function submit_no_filter {
    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.reduce.input.buffer.percent=0.3 \
            -D mapreduce.job.maps=700 \
            -D mapreduce.job.reduces=300 \
            -D mapreduce.map.memory.mb=5120 \
            -D mapreduce.reduce.memory.mb=3072 \
            -D mapreduce.jobtracker.maxreducememory.mb=8192\
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapred.job.name="${name}"  \
            -mapper "python ${mapper} ${RHO} ${Tau}" \
            -reducer "python ${reducer} ${Length} ${If_norm} ${confidence_rule}" \
            -file "${mapper}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}


t0=`timestamp`
main
tt=`timediff $t0`
echo "`datetime` job3 time: `second2formated ${tt}` (${tt}s)"
echo

