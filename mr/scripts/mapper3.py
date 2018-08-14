#!/usr/bin/python
#encoding:utf-8
import sys
from math import exp

# uid: [(p,r,t,k), (), ...]
# pid: {pid:score, ...}

Rho = float(sys.argv[1])
Tau = float(sys.argv[2])
F_filter = ""               # 结果中要保留的有效商品集,可选
if len(sys.argv) == 4:
    F_filter = sys.argv[3]

def decay(x):
    return guassian(x)
    # return half_decay(x)

def guassian(t):
    return exp(-t*t / (2*Tau*Tau) )
def half_decay(t):
    return pow(2, -abs(t)/Tau)

def trans_elem(s):
    p, r, t,k = s.strip().split(",")
    r = float(r)
    t = int(t)
    k = float(k)
    return (p,r,t,k)

def output(data, filters):
    sum_u = sum(x[1] for x in data) ** Rho
    for i in range(len(data)):
        p1, r1, t1, k1 = data[i]
        buff = {}
        for j in range(len(data)):
            if i == j:
                continue
            p2, r2, t2, k2 = data[j]
            if p1 == p2:
                continue
            if filters and p2 not in filters:
                continue
            score = r1 * r2 / k2 / sum_u * decay(t1 - t2)
            buff[p2] = score + buff.get(p2, 0)
        if len(buff) > 0:
            print "%s\t%s" % (p1, str(buff).replace(" ", ""))

def loadIds():
    ret = []
    if F_filter != '':
        for line in file(F_filter):
            id = line.strip()
            if id:
                ret.append(id)
    return set(ret)

if __name__ == '__main__':
    filters = loadIds()
    for line in sys.stdin:
        info = line.strip(' \n\r').split(';')
        if len(info) < 2:
            continue
        data = [trans_elem(s) for s in info]
        output(data, filters)

