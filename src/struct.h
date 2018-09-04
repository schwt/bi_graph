#ifndef MYSTRUCT_H
#define MYSTRUCT_H

// #include <assert.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <utility>
#include <malloc.h>
#include <iconv.h>
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <time.h>
#include <algorithm>
#include <cstring>
#include <functional>
#include <vector>
#include <map>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "../clib/Config.h"
#include "../clib/Timer.h"
#include "../clib/my_hash_map.h"
#include "../clib/WinnerTree_Linux.h" //外排
#include "../clib/ReadStructFile.h"   // 倒排索引
#include "../clib/MyGloble.h"
#include "../clib/Logger.h"
#include "../clib/uSTL.h"

using namespace std;

typedef unsigned int UINT;
typedef unsigned short SHORT;

struct DataNode {
    int user_id;
    int item_id;
    SHORT timestamp;   // 转换距5年前的小时数：[x - t(5年前)] / 3600 
    SHORT score;
};

struct MatrixIndex {
    int id;
    float norm;   // sum of scores in invert list
    int count;
    long long offset;
};
struct MatrixInvert {
    int id;
    SHORT timestamp;
    float score;
};

struct SimIndex {
    int id;
    float norm;
    int count;
    long long offset;
};
struct SimInvert {
    int id;
    float score;
    bool operator < (const struct SimInvert& b) const {
        return (this->score > b.score);
    }
};

#endif

