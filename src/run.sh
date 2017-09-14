#!/bin/sh
# -*- coding: utf8 -*-
##Filename:    run.sh
 #Author:      wyb
 #Email:       yingbin.wang@corp.netease.com
 #Date:        2017-09-14 11:18:41
 #
dir=$(cd $(dirname $0); pwd)

cd $dir
./graph config.ini

