#!/usr/bin/python
#encoding:utf-8
import sys

# (uid, pid, rate, time)  =>
# pid: (uid, rate, time)

idc_uid  = int(sys.argv[1])
idc_pid  = int(sys.argv[2])
idc_rate = int(sys.argv[3])
idc_time = int(sys.argv[4])
# 闭区间条件
rate_min = -1
rate_max = -1


def asInt(s):
    try: return int(s)
    except: return -1
def asFloat(s):
    try: return float(s)
    except: return -1

if len(sys.argv) > 5:
    rate_min = asFloat(sys.argv[5])
    rate_max = asFloat(sys.argv[6])

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) < 4:
            continue
        if info[idc_pid]=='' or info[idc_uid]=='':
            continue
        rate = asFloat(info[idc_rate])
        if rate == -1:
            continue
        if asInt(info[idc_time]) == -1:
            continue
        if rate_min != -1 and rate < rate_min:
            continue
        if rate_max != -1 and rate > rate_max:
            continue
        print "%s\t%s" % (info[idc_pid], info[idc_uid]+','+info[idc_rate]+','+info[idc_time])

