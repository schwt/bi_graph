#!/usr/bin/python
#encoding:utf-8
import sys
from math import exp

# uid: (pid, rate, time, sum_p)
#   =>
# uid: [(pid, rate,time, sum_p), (), ...]
#   =>
# pid: (pid, score)

Rho = float(sys.argv[1])
Tau = float(sys.argv[2])
buff = []
old_id = "aljsdkljg"

def guassian(t):
    return exp(-t*t / (2*Tau*Tau) )

def output_reduce(data):
    sum_u = sum(x[1] for x in data) ** Rho
    for i in range(len(data) - 1):
        for j in range(i+1, len(data), 1):
            if data[i][0] == data[j][0]:
                continue
            score = data[i][1] * data[j][1] / sum_u * guassian(data[i][2] - data[j][2])
            print "%s\t%s" % (data[i][0], "%s,%s" % (data[j][0], score/data[j][3]))
            print "%s\t%s" % (data[j][0], "%s,%s" % (data[i][0], score/data[i][3]))

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) != 2:
            continue
        uid = info[0]
        pid, rate, time, sum_p = info[1].split(',')
        rate  = float(rate)
        time  = int(time)
        sum_p = float(sum_p)
        if uid != old_id:
            if len(buff) > 1:
                output_reduce(buff)
            buff = []
            old_id = uid
        buff.append((pid, rate, time, sum_p))

    if len(buff) > 1:
        output_reduce(buff)

