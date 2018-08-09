#!/usr/bin/python
#encoding:utf-8
import sys

# (uid, pid, rate, time)  =>
# pid: (uid, rate, time)

idc_uid  = int(sys.argv[1])
idc_pid  = int(sys.argv[2])
idc_rate = int(sys.argv[3])
idc_time = int(sys.argv[4])

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) < 4:
            continue
        print "%s\t%s" % (info[idc_pid], info[idc_uid]+','+info[idc_rate]+','+info[idc_time])

