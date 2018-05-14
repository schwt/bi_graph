#!/usr/bin/python
#encoding:utf-8
import sys

s_sep = "\t"

count = 0
f_src    = sys.argv[1]
f_dst    = sys.argv[2]
f_name   = sys.argv[3]
idc_id   = int(sys.argv[4])
idc_name = int(sys.argv[5])
if len(sys.argv) >= 7:
    count = int(sys.argv[6])



d_name = {}
for line in file(f_name):
    sep = line.strip().split(s_sep)
    if len(sep) < max(idc_id, idc_name):
        continue
    id = int(sep[idc_id])
    d_name[id] = sep[idc_name]

print "# item name:", len(d_name)
wf = open(f_dst, "w")

cnt0 = 0
cnt1 = 1
for line in file(f_src):
    cnt0 += 1
    try:
        id, recos = line.strip(" ,\t\n\r").split("\t")
        sep = recos.split(",")
        id = int(id)
        wf.write("[%d] \t m=%d \t len=%d \t\t %s\n" % (cnt1, id, len(sep), d_name.get(id, "null")))
        cnt2 = 1
        for kv in sep:
            k,v = kv.split(":")
            rid = int(k)
            wf.write("\t[%2d]\t %-9d\t %-8s\t %s\n" % (cnt2, rid, v, d_name.get(rid, "null")))
            cnt2 += 1
        cnt1 += 1
        if count > 0 and cnt1 > count:
            break
        wf.write("\n")
    except:
        print "error line: (%s)" % line.strip()
        continue
wf.close()
print "# src line:", cnt0
print "# dst line:", cnt1 -1
