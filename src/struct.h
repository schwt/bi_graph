#ifndef MYSTRUCT_H
#define MYSTRUCT_H

#include <assert.h>
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
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>

#include "../../clib/Config.h"
#include "../../clib/Timer.h"
#include "../../clib/my_hash_map.h"
#include "../../clib/WinnerTree_Linux.h" //Õ‚≈≈
#include "../../clib/ReadStructFile.h"   // µπ≈≈À˜“˝
#include "../../clib/MyGloble.h"
#include "../../clib/Logger.h"
#include "../../clib/stringUtils.h"

using namespace std;

#ifdef __GNUC__
#define OTL_ODBC_UNIX
#endif

#define OTL_ODBC  //  Compile OTL 4.0/ODBC
#define OTL_STL
#define OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE

typedef unsigned int UINT;

struct DataNode {
    int user_id;
    int item_id;
    int timestamp;
    float score;
};

struct MatrixIndex {
    float norm;   // sum of scores in invert list
    int count;
    long long offset;
};
struct MatrixInvert {
    int id;
    int timestamp;
    double score;
};

/*
struct tNode {
    int id;
    float score;
    bool operator < (const struct tNode& b) const {
        return (this->score > b.score);
    }
};
*/
#endif

