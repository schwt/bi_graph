#!/usr/bin/python
#encoding:utf-8
import sys
from math import exp

# pid: {(pid:score), (), ...}
#   =>
# pid: [(pid, sum_score), (), ...]

length  = int(sys.argv[1])
if_norm = int(sys.argv[2])
confidence_rule = int(sys.argv[3])

buff = {}                # recoID: (sum_score, concurrence_cnt)
old_id = "aljsdkljg"

def output_reduce(data):
    data_filtered = [(k, v[0]) for k,v in data.iteritems() if v[1] >= confidence_rule]
    if not data_filtered:
        return
    arr = sorted(data_filtered, key = lambda x: -x[1])[:length]
    maxer = 1
    if if_norm:
        maxer = arr[0][1]
    return ",".join("%s:%s" % (k, round(v/maxer, 6)) for k,v in arr)

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) != 2:
            continue
        mid = info[0]
        indata = eval(info[1])
        if mid != old_id:
            if len(buff) > 0:
                recos = output_reduce(buff)
                if recos:
                    print "%s\t%s" % (old_id, recos)
            buff = {}
            old_id = mid
        for p, s in indata.iteritems():
            old_record = buff.get(p, (0, 0))
            buff[p] = (s + old_record[0], 1 + old_record[1])
            # buff[p] = s + buff.get(p, 0)

    if len(buff) > 0:
        recos = output_reduce(buff)
        if recos:
            print "%s\t%s" % (old_id, recos)

