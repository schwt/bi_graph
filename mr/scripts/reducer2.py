#!/usr/bin/python
#encoding:utf-8
import sys
from math import exp

# uid: (pid, rate, time, sum_p)
#   =>
# uid: [(pid, rate,time, sum_p), (), ...]  (hide uid)

buff = []
old_id = "aljsdkljg"

def output_reduce(data):
    print ";".join("%s,%s,%s,%s" % x for x in data)


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
        if pid == "\N": continue
        buff.append((pid, rate, time, sum_p))

    if len(buff) > 1:
        output_reduce(buff)

