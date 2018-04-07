#!/usr/bin/env python
# -*- coding: utf8 -*-
##Filename:    generate_data.py
 #Author:      wyb
 #Email:       wangyingbin@corp.netease.com
 #Date:        2017-09-13 13:57:31
 #
import sys
import random as rd

output = "train.txt"
data_num = 1000000
max_uid  = 100000
max_pid  = 10000
max_score = 5
time_from = 1483200000
time_to   = 1505282614

def main():
    num = data_num
    wf = open(output, 'w')
    if len(sys.argv) > 1:
        num = int(sys.argv[1])
        print "generate num:", num
    else:
        print "use default num:", num
    if num > 1000000:
        print "it may use several minutes..."
    for x in xrange(num):
        wf.write("%d\t%d\t%d\t%s\n" % (rd.randint(1, max_uid), rd.randint(1, max_pid), rd.randint(1, max_score), rd.randint(time_from, time_to) ))
    wf.close()
    print "done."

if __name__ == '__main__':
    main()
