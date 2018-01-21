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
#include <pthread.h>

#include "Timer.h"

#include "/d2/wangyingbin/mylib/ReadConfig.h"
#include "/d2/wangyingbin/mylib/MyHash_map.h"
#include "/d2/wangyingbin/mylib/MyLogMessage.h"
#include "/d2/wangyingbin/mylib/WinnerTree_Linux.h" //外排

#include "/d2/wangyingbin/mylib/ReadStructFile_new.h" // 倒排索引
// #include "/d2/wangyingbin/mylib/StringProcess.h"

#include "/d2/wangyingbin/mylib/MyGloble.h" // !

using namespace std;

#ifdef __GNUC__
#define OTL_ODBC_UNIX
#endif
 
#define OTL_ODBC  //  Compile OTL 4.0/ODBC
#define OTL_STL
#define OTL_ODBC_SELECT_STM_EXECUTE_BEFORE_DESCRIBE

typedef unsigned int UINT;

struct CategoryMsg      // 类别信息（HuangJingang)
{
    int  cate_id;
    char cate_path[18];
    char cate_name[128];
};

struct Pro2cateInvert   // 商品描述信息（HuangJingang）
{
    int  product_id;
    int  sale_volume;
    int  sale_price;
    int  in_price;
    int  gross;
    int  gross_rate;
    int  shop_id;
    int  cate_id;
};

// 历史订单格式
struct OrderIndex
{
    int cust_id;        // custome_id
    long long offset;   // offset
    int count;          // cnt
    bool operator < (const struct OrderIndex &b) const
    {
        if (this->cust_id < b.cust_id) return true;
        else return false;
    }
};
struct OrderInvert
{
    int product_id;     //
    int time;           // unix time
};

// 订单读取后的保存格式
struct OrderInfo
{
    int cust_id;
    int product_id;
};

// 重写订单数据
struct CidCateid        // user and category
{
    int cid;
    int cate_id;        // mapped category
};
struct CidProid         // user and product
{
    int cid;
    int product_id;
};

struct IndexNode        // cust product/category 间的索引倒排
{
    int id;
    int count;
    long long offset;
};
struct InvertNode
{
    int id;
};

struct SimiInvertNode
{
    int cid;
    double score;
    bool operator < (const struct SimiInvertNode &b) const
    {
        if (this->score > b.score)
            return true;
        else
            return false;
    }
};

struct RecoProduct      // 输出结果：待推荐商品
{
    int product_id;
    int weight;
    bool operator < (const struct RecoProduct &b) const
    {
        if (this->weight > b.weight)
            return true;
        else
            return false;
    }
};


struct ThreadArg
{
    vector<IndexNode>*  total_c2k_idx;
    vector<IndexNode>*  c2k_idx;
    vector<InvertNode>* c2k_ivt;
    vector<IndexNode>*  k2c_idx;
    vector<InvertNode>* k2c_ivt;
    string* simi_idx;
    string* simi_ivt;
    int cid_count_;
    int MIN_SIM;
    ThreadArg()
    {
        c2k_idx  = NULL;
        c2k_ivt  = NULL;
        k2c_idx  = NULL;
        k2c_ivt  = NULL;
        simi_idx = NULL;
        simi_ivt = NULL;
        cid_count_ = 0;
        MIN_SIM    = 0;
    }
};

#endif
