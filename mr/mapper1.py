#!/usr/bin/python
#encoding:utf-8
import sys

# (uid, pid, rate, time)  =>
# pid: (uid, rate, time)

if __name__ == '__main__':
    for line in sys.stdin:
        info = line.strip(' \n\r').split('\t')
        if len(info) != 4:
            continue
        print "%s\t%s" % (info[1], info[0]+','+info[2]+','+info[3])

