#!/usr/bin/env python
# -*- coding: utf8 -*-
##Filename:    generate_data.py
 #Author:      wyb
 #Email:       wangyingbin@corp.netease.com
 #Date:        2017-09-13 13:57:31
 #
import sys
from random import randint

"""
生成demo数据，仅供速度测试
"""

f_output = sys.argv[1]
num_sample = int(sys.argv[2])
num_user   = int(sys.argv[3])
num_item   = int(sys.argv[4])

wf = open(f_output, 'w')
for x in xrange(num_sample):
    uid = randint(0, num_user)
    iid = randint(0, num_item)
    time = randint(0, 3600 * 24 * 10) + 1500000000  # 10days range
    rate = randint(0, 5)
    wf.write("%s\t%s\t%s\t%s\n" % (uid, iid, rate, time))
    if x % 10000 == 0:
        print "\t" + str(x) + "\r",
        sys.stdout.flush()

print
wf.close()

