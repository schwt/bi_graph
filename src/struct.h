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
#include <pthread.h>

#include "../clib/Config.h"
#include "../clib/Timer.h"
#include "../clib/my_hash_map.h"
#include "../clib/WinnerTree_Linux.h" //Õ‚≈≈
#include "../clib/ReadStructFile.h"   // µπ≈≈À˜“˝
#include "../clib/MyGloble.h"
#include "../clib/Logger.h"
#include "../clib/uSTL.h"

using namespace std;

typedef unsigned int UINT;
typedef unsigned short SHORT;

struct DataNode {
    int user_id;
    int item_id;
    int timestamp;
    int score;
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

struct ThreadArgs {
    int thread_id;
    int num_threads;
    int time_decay_type;
    int top_reserve;
    int if_norm_result;
    size_t num_item_right;
    float tau;
    float score_threshold;
    vector<int>* vec_item_id_left;
    vector<int>* vec_item_id_right;
    vector<double>* item_right_norm;
    vector<MatrixIndex>* i2u_idx;
    vector<MatrixIndex>* u2i_idx;
    vector<MatrixInvert>* i2u_ivt;
    vector<MatrixInvert>* u2i_ivt;
    vector<pair<int, vector<SimInvert> > >* th_result;
    vector<float>* vec_progression;

    ThreadArgs(int _num_threads,
            int _time_decay_type,
            int _top_reserve,
            int _if_norm_result,
            int _num_item_right,
            double _tau,
            double _score_threshold,
            vector<int>* _vec_item_id_left,
            vector<int>* _vec_item_id_right,
            vector<double>* _item_right_norm,
            vector<MatrixIndex>*  _i2u_idx,
            vector<MatrixIndex>*  _u2i_idx,
            vector<MatrixInvert>* _i2u_ivt,
            vector<MatrixInvert>* _u2i_ivt,
            vector<float>* _vec_progression) {
        thread_id = 0;
        num_threads = _num_threads;
        time_decay_type = _time_decay_type;
        top_reserve = _top_reserve;
        if_norm_result = _if_norm_result;
        num_item_right = _num_item_right;
        tau = _tau;
        score_threshold = _score_threshold;
        vec_item_id_left  = _vec_item_id_left;
        vec_item_id_right = _vec_item_id_right;
        item_right_norm = _item_right_norm;
        i2u_idx = _i2u_idx;
        u2i_idx = _u2i_idx;
        i2u_ivt = _i2u_ivt;
        u2i_ivt = _u2i_ivt;
        vec_progression = _vec_progression;
        th_result = NULL;
    }
};

#endif

