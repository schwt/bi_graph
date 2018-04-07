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

#include "../../clib/Config.h"
#include "../../clib/Timer.h"
#include "../../clib/my_hash_map.h"
#include "../../clib/WinnerTree_Linux.h" //����
#include "../../clib/ReadStructFile.h"   // ��������
#include "../../clib/MyGloble.h"
#include "../../clib/Logger.h"

using namespace std;

typedef unsigned int UINT;

struct DataNode {
    int user_id;
    int item_id;
    int timestamp;
    float score;
};

struct MatrixIndex {
    int id;
    float norm;   // sum of scores in invert list
    int count;
    long long offset;
};
struct MatrixInvert {
    int id;
    int timestamp;
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

