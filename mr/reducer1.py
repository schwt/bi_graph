#!/usr/bin/python
#encoding:utf-8
import sys

# pid: (uid, rate, time)
#   =>
# pid: [(uid, rate, time), (), ...]
#   =>
# uid: (pid, rate, time, sum_p)

Lambda = float(sys.argv[1])
buff = []
old_id = "aljsdkljg"

def output_reduce(data, pid):
    sum_p = sum(x[1] for x in data) ** Lambda
    for uid, rate, time in data:
        print "%s\t%s,%s,%s,%s" % (uid, pid, rate, time, sum_p)

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) != 2:
            continue
        pid = info[0]
        uid, rate, time = info[1].split(',')
        rate = float(rate)
        if pid != old_id:
            if len(buff) > 0:
                output_reduce(buff, old_id)
            buff = []
            old_id = pid
        buff.append((uid, rate, time))

    if len(buff) > 0:
        output_reduce(buff, old_id)

