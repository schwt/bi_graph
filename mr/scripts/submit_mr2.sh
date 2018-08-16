#!/bin/bash
dir=$(cd $(dirname $0); pwd)
. `pwd`/config.sh
. ${dir}/utils.sh

cd ${dir}
name="${job_name}_job2"
INPUT="${hdfs_tmp_dir}/output1"
OUTPUT="${hdfs_tmp_dir}/output2"
reducer="reducer2.py"
RHO="${rho}"
Tau="${tau}"

echo "`datetime` start Job: ${name}"
function main {

    javaOpt=" -Xms2012m -Xmx2012m -XX:MaxPermSize=256m -XX:-UseGCOverheadLimit -XX:+UseConcMarkSweepGC -XX:MaxDirectMemorySize=256m"

    ${HADOOP} fs -rm -r ${OUTPUT}

    ${HADOOP} jar ${HADOOP_STREAM} \
            -D mapreduce.job.maps=${task_m2}        \
            -D mapreduce.job.reduces=${task_r2}     \
            -D mapreduce.map.memory.mb=${mem_m2}    \
            -D mapreduce.reduce.memory.mb=${mem_r2} \
            -D mapreduce.jobtracker.maxreducememory.mb=8192 \
            -D mapreduce.map.java.opts="$javaOpt" \
            -D mapreduce.reduce.java.opts="$javaOpt" \
            -D mapred.job.name="${name}"  \
            -mapper 'cat' \
            -reducer "python ${reducer} ${RHO} ${Tau}" \
            -file "${reducer}" \
            -input ${INPUT} \
            -output ${OUTPUT}
}


t0=`timestamp`
main
tt=`timediff $t0`
echo "`datetime` job2 time: `second2formated ${tt}` (${tt}s)"

