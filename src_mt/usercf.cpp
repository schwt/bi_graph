#include "usercf.h"

bool cmp_cid_cateid(const CidCateid& left, const CidCateid& right)
{
    return ((left.cid  < right.cid)
        || ((left.cid == right.cid) && (left.cate_id < right.cate_id)));
}

bool cmp_cateid_cid(const CidCateid& left, const CidCateid& right)
{
    return ((left.cate_id  < right.cate_id)
        || ((left.cate_id == right.cate_id) && (left.cid < right.cid)));
}

bool cmp_cache_count(const IndexNode& left, const IndexNode& right)
{
    return (left.count > right.count);
}

bool cmp_cache_id(const IndexNode& left, const IndexNode& right)
{
    return (left.id < right.id);
}

CUserCF::CUserCF()
{
    otl_connect::otl_initialize();
}


CUserCF::~CUserCF()
{
}

// init work path
bool CUserCF::Init(const char* s_f_config, const char* s_f_log)
{
    cout << "config file path: " << s_f_config << endl;
    cout << "log file path: "    << s_f_log    << endl;
    bool res = true;

    if (!ChangeWorkDir()) return false;
    if (!cls_log_msg.SetLogFilePath(s_f_log, "Test")) return false;
    
    if (!ReadConfigFile(s_f_config, s_f_log))
    {
        cls_log_msg.LogMessages("ReadConfigFile", __LINE__, false);
        return false;
    }
    else
    {
        cls_log_msg.LogMessages("ReadConfigFile", __LINE__, true);
    }
    
    if (!TEST)
    {
        printf("\n////////////// Testing environment \\\\\\\\\\\\\\\\\\\\\\\\\\\\ \n");
    }
    
    // write flag file
    FILE *fp_runflag = NULL;
    if ((fp_runflag=fopen(fpath_run_flag_.c_str(), "w")) == NULL) // result
    {
        res = false;
        cls_log_msg.LogMessages("open file:" + fpath_run_flag_, __LINE__, res);     
        return res;
    }
    fprintf(fp_runflag, "%d\n", 0);
    
    fclose(fp_runflag);
    return res;
}


bool CUserCF::ChangeWorkDir()
{
    char exec_path[1024] = {0};
    readlink("/proc/self/exe", exec_path, 1024);
    
    int i = 0;
    for (i = strlen(exec_path); i >= 0; --i)
    {
        if ('/' == exec_path[i]) break;
    }
    if (i < 0) return false;
    
    exec_path[++i] = '\0';
    if (chdir(exec_path) == -1) return false;
    printf("Working directory has been changed to %s.\n", exec_path);
    
    return true;
}

//*************** main function ***************//
bool CUserCF::Calc()
{
    bool res = true;
    
    res = SourceDataManage(fpath_cid_product_idx_, fpath_cid_product_ivt_,
                           fpath_similarity_idx_,  fpath_similarity_ivt_);
    cls_log_msg.LogMessages("SourceDataManage", __LINE__, res);
    if (!res) return res;
    
    
    res = MakeReco(fpath_similarity_ivt_,  fpath_cid_product_ivt_,
                   fpath_cid_recopro_idx_, fpath_cid_recopro_ivt_);
    cls_log_msg.LogMessages("MakeReco", __LINE__, res);
    if (!res) return res;
    
    
    // RemoveFile(fpath_similarity_idx_);
    // RemoveFile(fpath_similarity_ivt_);
    
    
    res = PrintReco(fpath_cid_product_ivt_,  fpath_cid_recopro_ivt_, 
                   fpath_printf_reco_);
    cls_log_msg.LogMessages("PrintReco", __LINE__, res);
    if (!res) return res;
    
    
    RemoveFile(fpath_cid_product_idx_);
    RemoveFile(fpath_cid_product_ivt_);
    
    
    if (res) // write success flag
    {
        FILE *fp_runflag = NULL;
        if ((fp_runflag = fopen(fpath_run_flag_.c_str(), "w")) == NULL) // check result
        {
            res = false;
            cls_log_msg.LogMessages("open file:" + fpath_run_flag_, __LINE__, res);     
            return res;
        }
        fprintf(fp_runflag, "%d\n", 1);
        fclose(fp_runflag);
    }
    
    return res;
}

//************* data manage *************//
bool CUserCF::SourceDataManage(const string& fpath_cid_product_idx_,
                               const string& fpath_cid_product_ivt_,
                               const string& fpath_similarity_idx_,
                               const string& fpath_similarity_ivt_)
{
    cls_log_msg.LogMessages("\n==================SourceDataManage==================\n");
    bool res = true;
    
    // 临时文件
    string fpath_order_info           = dir_temp_ + "order.dat";
    string fpath_usermap_product      = dir_temp_ + "usermap_product.dat";
    string fpath_usermap_catemap      = dir_temp_ + "usermap_catemap.dat";
    string fpath_usermap_catemap_sort = dir_temp_ + "usermap_catemap_sort.dat";
    string fpath_cid_cateid           = dir_temp_ + "cid_cateid.dat";
    string fpath_cateid_cid_sort      = dir_temp_ + "cateid_cid_sort.dat";
    
    set<int> set_black_list;                   // 用户黑名单
    set<int> set_trashy_products;              // 垃圾商品
    set<int> set_filt_user;                    // 订单过少，无推荐资格的用户
    
    vector<IndexNode>  vec_c2k_idx;
    vector<InvertNode> vec_c2k_ivt;
    vector<IndexNode>  vec_k2c_idx;
    vector<InvertNode> vec_k2c_ivt;
    
    hash_map<int, int> hm_product_category;    // product_id -> category
    hash_map<int, int> hm_category_kid;        // category   -> cateid
    
    
    res = ReadCategory(hm_product_category);
    cls_log_msg.LogMessages("ReadCategory", __LINE__, res);
    if (!res) return res;
    
    
    res = LoadBlackList(fpath_black_list_, set_black_list);
    cls_log_msg.LogMessages("LoadBlackList", __LINE__, res);
    if (!res) return res;
    
    
    res = LoadTrashyProduct(fpath_trashy_products_, set_trashy_products);
    cls_log_msg.LogMessages("LoadTrashyProduct", __LINE__, res);
    if (!res) return res;
    
    
    res = ReadOrderHistory(set_black_list,   set_trashy_products,
                           fpath_order_info, cid_count_);
    cls_log_msg.LogMessages("ReadOrderHistory", __LINE__, res);
    if (!res) return res;
    
    
    res = CreateCidProidCateid(hm_product_category,   fpath_order_info,
                               fpath_usermap_product, fpath_usermap_catemap,
                               vec_cid_map_, hm_category_kid);
    cls_log_msg.LogMessages("CreateCidProidCateid", __LINE__, res);
    if (!res) return res;
    
    
    res = MakeInvertCid2Product(fpath_usermap_product,
                                fpath_cid_product_idx_, fpath_cid_product_ivt_,
                                set_filt_user);
    cls_log_msg.LogMessages("MakeInvertCid2Product", __LINE__, res);
    if (!res) return res;
    
    
    RemoveFile(fpath_order_info);
    RemoveFile(fpath_usermap_product);
    
    
    cout << "\nSorting by cid-cateid..." << endl;
    if(K_MergeFile<CidCateid>(fpath_usermap_catemap.c_str(),
                              fpath_usermap_catemap_sort.c_str(), cmp_cid_cateid,
                              SORTMEMSIZE) == -1)
        res = false; 
    cls_log_msg.LogMessages("K_MergeFile " + fpath_usermap_catemap
                                  + " to " + fpath_usermap_catemap_sort, __LINE__, res);
    if (!res) return res;
    
    
    res = ShrinkCategory(fpath_usermap_catemap_sort, fpath_cid_cateid);
    cls_log_msg.LogMessages("ShrinkCategory", __LINE__, res);
    if (!res) return res;
        
    
    RemoveFile(fpath_usermap_catemap);
    RemoveFile(fpath_usermap_catemap_sort);
    
    
    res = MakeInvertC2K(fpath_cid_cateid, vec_c2k_idx, vec_c2k_ivt);
    cls_log_msg.LogMessages("MakeInvertC2K", __LINE__, res);
    if (!res) return res;
    
    
    cout << "\nSorting by cateid-cid..." << endl;
    if(K_MergeFile<CidCateid>(fpath_cid_cateid.c_str(),
                              fpath_cateid_cid_sort.c_str(), cmp_cateid_cid,
                              SORTMEMSIZE) == -1)
        res = false;
    cls_log_msg.LogMessages("K_MergeFile " + fpath_cid_cateid
                                  + " to " + fpath_cateid_cid_sort, __LINE__, res);
    if (!res) return res;
    
    
    res = MakeInvertK2C(fpath_cateid_cid_sort, vec_c2k_idx, set_filt_user,
                        vec_k2c_idx, vec_k2c_ivt);
    cls_log_msg.LogMessages("MakeInvertK2C", __LINE__, res);
    if (!res) return res;
    
    
    RemoveFile(fpath_cid_cateid);
    RemoveFile(fpath_cateid_cid_sort);
    
    
    res = ManageThreads(vec_c2k_idx, vec_c2k_ivt,
                        vec_k2c_idx, vec_k2c_ivt,
                        fpath_similarity_idx_, fpath_similarity_ivt_);
    cls_log_msg.LogMessages("ManageThreads", __LINE__, res);
    if (!res) return res;
    
    
    // res = ComputeSimilarity(vec_c2k_idx, vec_c2k_ivt,
    //                         vec_k2c_idx, vec_k2c_ivt,
    //                         fpath_similarity_idx_, fpath_similarity_ivt_);
    // cls_log_msg.LogMessages("ComputeSimilarity", __LINE__, res);
    // if (!res) return res;
    
    
    printf("Source Data Manage Done.\n");
    return res;
}

// read product category
bool CUserCF::ReadCategory(hash_map<int,int>& hm_product_category)
{
    cls_log_msg.LogMessages("\n-------------ReadCategory-------------n");
    bool res = false;
    
    FILE * fp_cate2path = fopen(fpath_cate2path_ivt_.c_str(), "rb");
    if (!fp_cate2path)
    {
        cls_log_msg.LogMessages("OpenFile:" + fpath_cate2path_ivt_, __LINE__, res);
        return res;
    }
    FILE * fp_pro2cate_ivt = fopen(fpath_pro_desp_ivt_.c_str(), "rb");
    if (!fp_pro2cate_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + fpath_pro_desp_ivt_, __LINE__, res);
        fclose(fp_cate2path);
        return res;
    }
    
    string s_cate_path = "";
    string s_cate_name = "";
    int category_id    = 0;
    struct CategoryMsg              str_cate;
    multimap<string, int>           hm_catpath2catid; // path -> id
    multimap<string, int>::iterator ith_path2id;
    multimap<int, string>           hm_catid2catPath; // id -> path
    multimap<int, string>::iterator ith_id2path;
    
    while (!feof(fp_cate2path))
    {
        fread(&str_cate, sizeof(CategoryMsg), 1, fp_cate2path);
        s_cate_path = str_cate.cate_path;
        category_id = str_cate.cate_id;
        
        hm_catid2catPath.insert(make_pair(category_id, s_cate_path));
        hm_catpath2catid.insert(make_pair(s_cate_path, category_id));
    }
    cout << "Read category_path done." << endl;
    cout << "cateid2catepath_cnt = "   << hm_catid2catPath.size() << endl;
    cout << "catepath2cateid_cnt = "   << hm_catpath2catid.size() << endl;
    
    ////////////// make hm_product_category //////////////
    size_t a              = 0;
    int product_id        = 0;
    char cat_path_des[18] = {0};
    
    map<int, int>           map_category;
    map<int, int>::iterator itm_catid;
    vector<Pro2cateInvert>  vec_pro2cate_ivt;
    struct Pro2cateInvert   str_invert;
    
    while (!feof(fp_pro2cate_ivt))
    {
        a = fread(&str_invert, sizeof(Pro2cateInvert), 1, fp_pro2cate_ivt);
        if (a != 1) continue;
        
        category_id = str_invert.cate_id;
        product_id  = str_invert.product_id;
        
        if (category_id < 0) continue;
        
        ith_id2path      = hm_catid2catPath.find(category_id);
        if (ith_id2path == hm_catid2catPath.end())
        {
            continue;
        }
        GetCategoryIndex((ith_id2path->second).c_str(), cat_path_des, 3); // 四级类
        
        ith_path2id      = hm_catpath2catid.find(cat_path_des);
        if (ith_path2id == hm_catpath2catid.end()) 
        {
            printf(" ####3 cannot find 3level [pid=%s]\n", cat_path_des);
            continue;
        }
        hm_product_category.insert(make_pair(product_id, ith_path2id->second));
    }
    printf("Read Category DONE.\n");
    printf("pro2cate map size: %d\n\n", (int)hm_product_category.size());
    
    fclose(fp_cate2path);
    fclose(fp_pro2cate_ivt);
    return true;
}

// read blacklist of custs
bool CUserCF::LoadBlackList(const string& f_src,
                            set<int>& set_black_list)
{
    cls_log_msg.LogMessages("\n-------------LoadBlackList-------------\n");
    bool res = true;
    
    ifstream fin(f_src.c_str());
    string s_line = "";
    
    while (getline(fin, s_line))
    {
        if (s_line == "") continue;
        set_black_list.insert(atoi(s_line.c_str()));
    }
    
    printf("Black list count: %lu\n", set_black_list.size());
    return res;
}

// read trashy products
bool CUserCF::LoadTrashyProduct(const string& f_src,
                                set<int>& set_rubbish_products)
{
    cls_log_msg.LogMessages("\n-------------LoadTrashyProduct-------------\n");
    bool res = true;
    
    ifstream fin(f_src.c_str());
    string   s_line = "";
    while (getline(fin, s_line))
    {
        if (s_line == "") continue;
        
        set_rubbish_products.insert(atoi(s_line.c_str()));
    }
    printf("Trashy Products Count: %lu\n", set_rubbish_products.size());
    return res;
}

// 读历史订单
bool CUserCF::ReadOrderHistory(const set<int>& set_black_list,
                               const set<int>& set_trashy_products,
                               const string& fpath_order_info,
                               int& cid_count_)
{
    cls_log_msg.LogMessages("\n-------------ReadOrderHistory-------------\n");
    bool res = false;
    
    FILE * fp_ivt = fopen(fpath_order_ivt_.c_str(), "rb");
    if (!fp_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + fpath_order_ivt_, __LINE__, res);
        return res;
    }
    FILE * fp_des = fopen(fpath_order_info.c_str(), "wb");
    if (!fp_des)
    {
        cls_log_msg.LogMessages("OpenFile:" + fpath_order_info, __LINE__, res);
        fclose(fp_ivt);
        return res;
    }
    
    vector<OrderIndex>  vec_order_idx;
    vector<OrderInvert> vec_order_ivt;
    vector<OrderInvert>::iterator itv_order_ivt;
    hash_map<int, int>::iterator  ith_pro2cate;
    
    LoadIndex_Vector(fpath_order_idx_, vec_order_idx);
    sort(vec_order_idx.begin(), vec_order_idx.end());
    
    int run_cnt     = 0;
    int old_cust_id = -1;
    int idx_cnt     = 0;
    vector<OrderInfo> vec_wrt_buf;
    vec_wrt_buf.resize(WRITE_BUF);
    cid_count_ = 0;
    
    for (vector<OrderIndex>::iterator itv_order_idx  = vec_order_idx.begin();
                                      itv_order_idx != vec_order_idx.end();
                                      itv_order_idx++)
    {
        if (!TEST)
        {
            if (run_cnt >= TEST_ORDER_CNT) break; // running in test environment
        }
        if (itv_order_idx->count > AGENT_ORDER) continue;    // filter agent order
        
        if (set_black_list.find(itv_order_idx->cust_id) !=
            set_black_list.end())  continue;                 // fliter blacklist
        
        ReadInvert(fp_ivt, *itv_order_idx, vec_order_ivt);
        
        for (itv_order_ivt  = vec_order_ivt.begin();
             itv_order_ivt != vec_order_ivt.end();
             itv_order_ivt++)
        {
            if (itv_order_ivt->time <= INIT_TIME_STAMP) continue; // 过滤05年之前订单
            if (set_trashy_products.find(itv_order_ivt->product_id) !=
                set_trashy_products.end()) continue;        // filter trashy products
            vec_wrt_buf[idx_cnt].cust_id    = itv_order_idx->cust_id;
            vec_wrt_buf[idx_cnt].product_id = itv_order_ivt->product_id;
            if (++idx_cnt == WRITE_BUF)
            {
                fwrite(&vec_wrt_buf[0], sizeof(OrderInfo), idx_cnt, fp_des);
                idx_cnt = 0;
            }
        }
        if (old_cust_id != itv_order_idx->cust_id)              // 统计 cust_id 数量
        {
            old_cust_id  = itv_order_idx->cust_id;
            cid_count_++;
        }
        if (++run_cnt % 10000 == 0)
        {
            printf("\rrun_cnt: %d", run_cnt);
            fflush(stdout);
        }
    }
    printf("\rrun_cnt: %d\n", run_cnt);
    printf("cid_count_ = %d\n", cid_count_);
    
    if (idx_cnt > 0)    // 扫尾
    {
        fwrite(&vec_wrt_buf[0], sizeof(OrderInfo), idx_cnt, fp_des);
    }
    
    fclose(fp_ivt);
    fclose(fp_des);
    return true;
}

// 映射cust_id、category_id, 生成 cid-productid、cid-cateid 两个文件
bool CUserCF::CreateCidProidCateid(hash_map<int, int>& hm_product_category,
                                   const string& f_src,
                                   const string& f_cid_pro,
                                   const string& f_cid_cateid,
                                   vector<int>& vec_cid_map_,
                                   hash_map<int,int>& hm_category_kid)
{
    cls_log_msg.LogMessages("\n-------------CreateCidProidCateid-------------\n");
    bool res = false;
    
    FILE * fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_src, __LINE__, res);
        return res;
    }
    FILE * fp_cid_cateid = fopen(f_cid_cateid.c_str(), "wb");
    if (!fp_cid_cateid)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_cateid, __LINE__, res);
        fclose(fp_src);
        return res;
    }
    FILE *fp_cid_proid = fopen(f_cid_pro.c_str(), "wb");
    if (!fp_cid_proid)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_cateid, __LINE__, res);
        fclose(fp_src);
        fclose(fp_cid_cateid);
        return res;
    }
    
    int i            = 0;
    int run_cnt      = 0;
    int old_cust_id  = -1;
    int map_cid      = -1;
    int map_cateid   = 0;
    int read_cnt     = 0;
    int wrt_cnt_cate = 0;
    int wrt_cnt_pro  = 0;
    
    hash_map<int, int>::iterator ith_pro2cate;
    hash_map<int, int>::iterator ith_cate_map;
    vector<OrderInfo> vec_read_buf;
    vector<CidCateid> vec_write_buf_cate;
    vector<CidProid>  vec_write_buf_pro;
    vec_read_buf.resize(READ_BUF);
    vec_write_buf_cate.resize(WRITE_BUF);
    vec_write_buf_pro.resize(WRITE_BUF);
    
    vec_cid_map_.resize(cid_count_);
    read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
    printf("Reading order info...\n");
    
    while(read_cnt)
    {
        for (i = 0; i < read_cnt; ++i)
        {
            ith_pro2cate      = hm_product_category.find(vec_read_buf[i].product_id);
            if (ith_pro2cate == hm_product_category.end()) continue; // 找不到分类的过滤掉
            
            ith_cate_map      = hm_category_kid.find(ith_pro2cate->second); // category 映射
            if (ith_cate_map == hm_category_kid.end())
            {
                map_cateid = hm_category_kid.size();
                hm_category_kid.insert(make_pair(ith_pro2cate->second, map_cateid));
            }
            else map_cateid = ith_cate_map->second;
            
            if (old_cust_id != vec_read_buf[i].cust_id)             // cust_id 映射
            {
                old_cust_id  = vec_read_buf[i].cust_id;
                vec_cid_map_[++map_cid] = old_cust_id;
            }
            vec_write_buf_cate[wrt_cnt_cate].cid      = map_cid;
            vec_write_buf_cate[wrt_cnt_cate].cate_id  = map_cateid;
            vec_write_buf_pro[wrt_cnt_pro].cid        = map_cid;
            vec_write_buf_pro[wrt_cnt_pro].product_id = vec_read_buf[i].product_id;
            
            if (++wrt_cnt_cate == WRITE_BUF)
            {
                fwrite(&vec_write_buf_cate[0], sizeof(CidCateid),
                       wrt_cnt_cate, fp_cid_cateid);
                wrt_cnt_cate = 0;
            }
            if (++wrt_cnt_pro == WRITE_BUF)
            {
                fwrite(&vec_write_buf_pro[0],  sizeof(CidProid), 
                       wrt_cnt_pro,  fp_cid_proid);
                wrt_cnt_pro = 0;
            }
            if (++run_cnt % 100000 == 0)
            {
                printf("\rrun_cnt: %d", run_cnt);
                fflush(stdout);
            }
        }
        read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
    }
    printf("\rrun_cnt: %d\n", run_cnt);
    if (wrt_cnt_cate > 0)  // É¨Î²
    {
        fwrite(&vec_write_buf_cate[0], sizeof(CidCateid), wrt_cnt_cate, fp_cid_cateid);
    }
    if (wrt_cnt_pro > 0)
    {
        fwrite(&vec_write_buf_pro[0],  sizeof(CidProid),  wrt_cnt_pro,  fp_cid_proid);
    }
    
    vec_cid_map_.assign(vec_cid_map_.begin(), vec_cid_map_.begin() + map_cid + 1);
    cid_count_    = vec_cid_map_.size();
    cateid_count_ = hm_category_kid.size();
    
    printf("max_cid   : %d\n", map_cid);
    printf("cid_count_: %d\n", cid_count_);
    
    fclose(fp_src);
    fclose(fp_cid_cateid);
    fclose(fp_cid_proid);
    return true;    
}

// 生成倒排索引 cid -> product_id
bool CUserCF::MakeInvertCid2Product(const string& f_src,
                                    const string& f_idx,
                                    const string& f_ivt,
                                    set<int>& set_filt_user)
{
    cls_log_msg.LogMessages("\n-------------MakeInvertCid2Product-------------\n");
    bool res = false;
    
    FILE * fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_src, __LINE__, res);
        return res;
    }
    FILE * fp_idx = fopen(f_idx.c_str(), "wb");
    if (!fp_idx)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_idx, __LINE__, res);
        fclose(fp_src);
        return res;
    }
    FILE * fp_ivt = fopen(f_ivt.c_str(), "wb");
    if (!fp_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_ivt, __LINE__, res);
        fclose(fp_src);
        fclose(fp_idx);
        return res;
    }
    
    int i          = 0;
    int empty_cid_cnt = 0;
    int run_cnt    = 0;
    int read_cnt   = 0;
    int old_cid    = 0;
    long long from = 0;
    vector<int>        vec_invert_list;
    vector<IndexNode>  vec_index_list;
    vector<CidProid>   vec_read_buf;
    vec_read_buf.resize(READ_BUF);
    vec_index_list.resize(cid_count_);
    
    read_cnt   = fread(&vec_read_buf[0], sizeof(CidProid), READ_BUF, fp_src);
    old_cid    = vec_read_buf[0].cid;
    while (read_cnt)
    {
        for (i = 0; i < read_cnt; ++i)
        {
            if (old_cid != vec_read_buf[i].cid)
            {
                if (!vec_invert_list.empty())
                {
                    if (vec_invert_list.size() < USER_ORDER_MIN)
                        set_filt_user.insert(old_cid);  // 订单过少，不作为推荐用户
                    vec_index_list[old_cid].id     = old_cid;
                    vec_index_list[old_cid].count  = vec_invert_list.size();
                    vec_index_list[old_cid].offset = from * sizeof(int);
                    
                    fwrite(&vec_invert_list[0], sizeof(int), vec_index_list[old_cid].count, fp_ivt);
                    from   += vec_index_list[old_cid].count;
                    vec_invert_list.clear();
                }
                else empty_cid_cnt++;
                old_cid = vec_read_buf[i].cid;
            }
            vec_invert_list.push_back(vec_read_buf[i].product_id);
        }
        read_cnt = fread(&vec_read_buf[0], sizeof(CidProid), READ_BUF, fp_src);
        
        if (++run_cnt % 1000 == 0)
        {
            printf("\rrun_cnt: %d", run_cnt);
            fflush(stdout);
        }
    }
    if (!vec_invert_list.empty())   // 扫尾
    {
        vec_index_list[old_cid].id     = old_cid;
        vec_index_list[old_cid].count  = vec_invert_list.size();
        vec_index_list[old_cid].offset = from * sizeof(int);
        fwrite(&vec_invert_list[0], sizeof(int), vec_index_list[old_cid].count, fp_ivt);
    }
    else empty_cid_cnt++;
    
    printf("\rrun_cnt: %d\n", run_cnt);
    fwrite(&vec_index_list[0], sizeof(IndexNode), vec_index_list.size(), fp_idx);
    
    if (empty_cid_cnt > 0)
    {
        printf("#### cid count with empty ivt: %d\n", empty_cid_cnt);
    }
    
    // 测试 cid 映射正确性
    for (i = 0; i < (int)vec_index_list.size(); ++i)
    {
        if (i != vec_index_list[i].id)
        {
            cout << "#### ";
            printf("i = %d; id = %d\n", i, vec_index_list[i].id);
        }
    }
    
    fclose(fp_src);
    fclose(fp_idx);
    fclose(fp_ivt);
    
    res = IdxIvtCheck<IndexNode, int>(f_idx, f_ivt);
    cls_log_msg.LogMessages("IdxIvtCheck", __LINE__, res);
    if (!res) printf("Check Invert: Error!\n");
    
    return res;
}

// 将每一用户的相同 category 合并
bool CUserCF::ShrinkCategory(const string& f_src, const string& f_des)
{
    cls_log_msg.LogMessages("\n-------------ShrinkCategory-------------\n");
    bool res = false;
    
    FILE * fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_src, __LINE__, res);
        return res;
    }
    FILE * fp_des = fopen(f_des.c_str(), "wb");
    if (!fp_des)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_des, __LINE__, res);
        fclose(fp_src);
        return res;
    }
    
    int run_cnt    = 0;
    int read_cnt   = 0;
    int write_cnt  = 0;
    int i          = 0;
    int old_cateid = -1;
    int old_cid    = -1;
    vector<CidCateid> vec_read_buf;
    vector<CidCateid> vec_write_buf;
    vec_read_buf.resize(READ_BUF);
    vec_write_buf.resize(WRITE_BUF);
    
    read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
    while (read_cnt)
    {
        for (i = 0; i < read_cnt; ++i)
        {
            if ((old_cateid == vec_read_buf[i].cate_id)
                && (old_cid == vec_read_buf[i].cid)) continue;
            old_cid    = vec_read_buf[i].cid;
            old_cateid = vec_read_buf[i].cate_id;
            vec_write_buf[write_cnt++] = vec_read_buf[i];
            
            if (write_cnt == WRITE_BUF)
            {
                fwrite(&vec_write_buf[0], sizeof(CidCateid), write_cnt, fp_des);
                write_cnt = 0;
            }
        }
        read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
        
        if (++run_cnt % 10000 == 0)
        {
            printf("\rrun_cnt: %d", run_cnt);
            fflush(stdout);
        }
    }
    printf("\rrun_cnt: %d\n", run_cnt);
    if (write_cnt > 0)   // 扫尾
        fwrite(&vec_write_buf[0], sizeof(CidCateid), write_cnt, fp_des);
        
    fclose(fp_src);
    fclose(fp_des);
    return true;
}

// 生成倒排索引 cid -> cateid
bool CUserCF::MakeInvertC2K(const string& f_src,
                            vector<IndexNode>&  vec_c2k_idx,
                            vector<InvertNode>& vec_c2k_ivt)
{
    cls_log_msg.LogMessages("\n-------------MakeInvertC2K-------------\n");
    bool res = false;
    
    FILE * fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_src, __LINE__, res);
        return res;
    }
    
    int empty_cid_cnt = 0;
    int run_cnt    = 0;
    int read_cnt   = 0;
    int i          = 0;
    int old_cid    = 0;
    long long from = 0;
    int count      = 0;
    struct InvertNode  strt_invert;
    vector<CidCateid>  vec_read_buf;
    vec_read_buf.resize(READ_BUF);
    vec_c2k_idx.resize(cid_count_);
    
    read_cnt   = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
    old_cid    = vec_read_buf[0].cid;
    while (read_cnt)
    {
        for (i = 0; i < read_cnt; ++i)
        {
            if (old_cid != vec_read_buf[i].cid)
            {
                if (count != 0)
                {
                    vec_c2k_idx[old_cid].id     = old_cid;
                    vec_c2k_idx[old_cid].count  = count;
                    vec_c2k_idx[old_cid].offset = from;
                    
                    from       += count;
                    count       = 0;
                }
                else empty_cid_cnt++;
                old_cid = vec_read_buf[i].cid;
            }
            
            strt_invert.id = vec_read_buf[i].cate_id;
            vec_c2k_ivt.push_back(strt_invert);
            count++;
        }
        read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
        
        if (++run_cnt % 100 == 0)
        {
            printf("\rrun_cnt: %d", run_cnt);
            fflush(stdout);
        }
    }
    if (count != 0)   // 扫尾
    {
        vec_c2k_idx[old_cid].id     = old_cid;
        vec_c2k_idx[old_cid].count  = count;
        vec_c2k_idx[old_cid].offset = from;
        
        from += vec_c2k_idx[old_cid].count;
    }
    else empty_cid_cnt++;
    printf("\rrun_cnt: %d\n", run_cnt);
    printf("idx: ivt_cnt = %lld\n", from);
    printf("invert_size  = %lu\n", vec_c2k_ivt.size());
    
    if (from != (int)vec_c2k_ivt.size())
    {
        printf("Invert Error!\n");
        return res;
    }
    
    fclose(fp_src);
    
    if (empty_cid_cnt > 0)
    {
        printf("#### cid count with empty ivt: %d\n", empty_cid_cnt);
    }    
    for (i = 0; i < (int)vec_c2k_idx.size(); ++i)   // 检验索引ID完备性
    {
        if (i != vec_c2k_idx[i].id)
        {
            printf("#### i = %d; id = %d\n", i, vec_c2k_idx[i].id);
            return false;
        }
    }
    
    return true;
}

// 生成倒排索引 cateid -> cid
bool CUserCF::MakeInvertK2C(const string& f_src,
                            const vector<IndexNode>& vec_c2k_idx,
                            const set<int>& set_filt_user,
                            vector<IndexNode>&  vec_k2c_idx,
                            vector<InvertNode>& vec_k2c_ivt)
{
    cls_log_msg.LogMessages("\n-------------MakeInvertK2C-------------\n");
    bool res = false;
    
    FILE * fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_src, __LINE__, res);
        return res;
    }
    
    int run_cnt    = 0;
    int read_cnt   = 0;
    int i          = 0;
    int old_cateid = 0;
    long long from = 0;
    struct InvertNode  strt_invert;
    struct IndexNode   strt_index;
    vector<CidCateid>  vec_read_buf;
    strt_index.id     = -1;
    strt_index.count  = 0;
    strt_index.offset = 0;
    
    vec_read_buf.resize(READ_BUF);
    vec_k2c_idx.resize(cateid_count_, strt_index);
    int count = 0;
    
    read_cnt   = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
    old_cateid = vec_read_buf[0].cate_id;
    while (read_cnt)
    {
        for (i = 0; i < read_cnt; ++i)
        {
            if (old_cateid != vec_read_buf[i].cate_id)
            {
                if (count != 0)
                {
                    vec_k2c_idx[old_cateid].id     = old_cateid;
                    vec_k2c_idx[old_cateid].count  = count;
                    vec_k2c_idx[old_cateid].offset = from;
                    
                    from       += count;
                    count       = 0;
                }
                old_cateid = vec_read_buf[i].cate_id;
            }
            strt_invert.id = vec_read_buf[i].cid;
            if (vec_c2k_idx[strt_invert.id].count > USER_INVERT_MAX)
                continue;                               // 购买太泛，过滤掉
            if (set_filt_user.find(strt_invert.id) != set_filt_user.end())
                continue;                               // 购买过少，过滤掉
            vec_k2c_ivt.push_back(strt_invert);
            count++;
        }
        read_cnt = fread(&vec_read_buf[0], sizeof(CidCateid), READ_BUF, fp_src);
        
        if (++run_cnt % 100 == 0)
        {
            printf("\rrun_cnt: %d", run_cnt);
            fflush(stdout);
        }
    }
    if (count != 0)   // 扫尾
    {
        vec_k2c_idx[old_cateid].id     = old_cateid;
        vec_k2c_idx[old_cateid].count  = count;
        vec_k2c_idx[old_cateid].offset = from;
        from += vec_k2c_idx[old_cateid].count;
    }
    printf("\rrun_cnt: %d\n", run_cnt);
    printf("idx: ivt_cnt = %lld\n", from);
    printf("invert_size  = %lu\n", vec_k2c_ivt.size());
    
    if (from != (int)vec_k2c_ivt.size())
    {
        printf("Invert Error!\n");
        return res;
    }
    fclose(fp_src);
    
    for (int i = 0; i < (int)vec_k2c_idx.size(); ++i)
    {
        if (i != vec_k2c_idx[i].id)
        {
            printf("Non invert kid!\n");
        }
    }
    return true;
}

//*************** 线程控制 ***************//
bool CUserCF::ManageThreads(vector<IndexNode>&  vec_c2k_idx,
                            vector<InvertNode>& vec_c2k_ivt,
                            vector<IndexNode>&  vec_k2c_idx,
                            vector<InvertNode>& vec_k2c_ivt,
                            const string& f_similiarity_idx,
                            const string& f_similiarity_ivt)
{
    cls_log_msg.LogMessages("\n-------------ManageThreads-------------\n");
    bool res = false;
    
    if (THREAD_NUM == 0)            // 不使用多线程
    {
        res = ComputeSimilarity(vec_c2k_idx, vec_c2k_ivt,
                                vec_k2c_idx, vec_k2c_ivt,
                                f_similiarity_idx, f_similiarity_ivt);
        return res;
    }
    void *ComputeSimilarity_Thread(void *arg);
    bool CombineSimiFile(int, const vector<string>&, 
                              const vector<string>&, string, string);
    
    // 把索引分配到各线程
    int thread_num  = -1;
    int index_cnt   = 0;     // 每个线程分配的索引个数（最后一个除外）
    int index_begin = 0;
    if (cid_count_ % THREAD_NUM == 0)
    {
        index_cnt = cid_count_ / THREAD_NUM;
    }
    else index_cnt = cid_count_ / THREAD_NUM + 1;
    
    vector<vector<IndexNode> > vec_thread_idx;
    vec_thread_idx.resize(THREAD_NUM);
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        if (thread_num != THREAD_NUM -1)
            vec_thread_idx[thread_num].assign(vec_c2k_idx.begin()+index_begin,
                                              vec_c2k_idx.begin()+index_begin+index_cnt);
        else
            vec_thread_idx[thread_num].assign(vec_c2k_idx.begin()+index_begin,
                                              vec_c2k_idx.end());
        index_begin += index_cnt;
    }
    
    // 配置各线程输出文件
    vector<string> vec_output_idx;
    vector<string> vec_output_ivt;
    vec_output_idx.resize(THREAD_NUM, dir_temp_ + "simi_thread_");
    vec_output_ivt.resize(THREAD_NUM, dir_temp_ + "simi_thread_");
    
    char c_temp[2];
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        snprintf(c_temp, sizeof(c_temp), "%d", thread_num);
        vec_output_idx[thread_num] += c_temp + (string)".idx";
        vec_output_ivt[thread_num] += c_temp + (string)".ivt";
    }
    
    // 配置各线程参数
    struct ThreadArg strt_arg;
    // strt_arg.c2k_idx    = &vec_c2k_idx;
    strt_arg.total_c2k_idx = &vec_c2k_idx;
    strt_arg.c2k_ivt    = &vec_c2k_ivt;
    strt_arg.k2c_idx    = &vec_k2c_idx;
    strt_arg.k2c_ivt    = &vec_k2c_ivt;
    strt_arg.MIN_SIM    = MIN_SIM;
    strt_arg.cid_count_ = cid_count_;
    // strt_arg.simi_idx = &output_idx;
    // strt_arg.simi_idx = &output_ivt;
    
    vector<ThreadArg> vec_arg;
    vec_arg.resize(THREAD_NUM, strt_arg);
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        vec_arg[thread_num].c2k_idx  = &vec_thread_idx[thread_num];
        vec_arg[thread_num].simi_idx = &vec_output_idx[thread_num];
        vec_arg[thread_num].simi_ivt = &vec_output_ivt[thread_num];
    }
    
    // 创建各线程 
    pthread_t tid[THREAD_NUM];
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        res = !pthread_create(&tid[thread_num], NULL, ComputeSimilarity_Thread,
                              (void *)&vec_arg[thread_num]);
        if (!res)
        {
            printf("Create threads failed!\n");
            return res;
        }
        else printf("Create threads: tid[%d]=%d\n",
                     thread_num, (int)tid[thread_num]);
    }
    
    // 等待各线程完成
    void * thread_res[THREAD_NUM];
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        pthread_join(tid[thread_num], &thread_res[thread_num]);
    }
    // 检查各线程返回值
    for (thread_num = 0; thread_num < THREAD_NUM; ++thread_num)
    {
        if (!(bool)thread_res[thread_num])
        {
            printf("Thread %d return false!\n", thread_num);
            return false;
        } 
    }
    // 合并输出结果
    res = CombineSimiFile(THREAD_NUM, vec_output_idx, vec_output_ivt,
                          f_similiarity_idx, f_similiarity_ivt);
    if (!res)
        cls_log_msg.LogMessages("CombineFile Error!", __LINE__, res);
    return res;
}

// 计算余弦值
inline void ComputeCosine(int denominator_a,
                          int MIN_SIM,
                          int& rule_flag_cnt,
                          vector<int>& vec_rule,
                          vector<int>& vec_rule_flag,
                          const vector<IndexNode>& vec_c2k_idx,
                          vector<SimiInvertNode>& vec_cid_score)
{
    for (int i = 0; i < rule_flag_cnt; ++i)
    {
        vec_cid_score[i].cid   = vec_rule_flag[i];
        vec_cid_score[i].score = vec_rule[vec_rule_flag[i]] 
            / sqrt(denominator_a * vec_c2k_idx[vec_rule_flag[i]].count);
        vec_rule[vec_rule_flag[i]] = 0;      // 归零
        vec_rule_flag[i]           = 0;
    }
    if (rule_flag_cnt > MIN_SIM)
    {
        partial_sort(vec_cid_score.begin(),
                     vec_cid_score.begin() + MIN_SIM,
                     vec_cid_score.begin() + rule_flag_cnt);
        rule_flag_cnt = MIN_SIM;
    }
    else
        sort(vec_cid_score.begin(), vec_cid_score.begin() + rule_flag_cnt);
}

// 计算相似用户的线程
void *ComputeSimilarity_Thread(void *arg)
{
    bool res = false;
    
    int tid = (int)pthread_self();
    struct ThreadArg *pstrt_arg = (ThreadArg *)arg;
    
    vector<IndexNode>  *vec_total_c2k_idx = pstrt_arg->total_c2k_idx;
    vector<IndexNode>  *vec_c2k_idx = pstrt_arg->c2k_idx;
    vector<InvertNode> *vec_c2k_ivt = pstrt_arg->c2k_ivt;
    vector<IndexNode>  *vec_k2c_idx = pstrt_arg->k2c_idx;
    vector<InvertNode> *vec_k2c_ivt = pstrt_arg->k2c_ivt;
    
    string f_similiarity_idx = *(pstrt_arg->simi_idx);
    string f_similiarity_ivt = *(pstrt_arg->simi_ivt);
    int cid_count_           =   pstrt_arg->cid_count_;
    int MIN_SIM              =   pstrt_arg->MIN_SIM;
    
    
    FILE * fp_similarity_idx = fopen(f_similiarity_idx.c_str(), "wb");
    if (!fp_similarity_idx)
    {
        pthread_exit(NULL);
    }
    FILE * fp_similarity_ivt = fopen(f_similiarity_ivt.c_str(), "wb");
    if (!fp_similarity_ivt)
    {
        fclose(fp_similarity_idx);
        pthread_exit(NULL);
    }
    int i   = 0;
    int k   = 0;
    int j   = 0;
    int cid = 0;
    int kid = 0;
    
    int offset     = 0;
    int count      = 0;
    int offset_ivt = 0;
    int count_ivt  = 0;
    vector<SimiInvertNode> vec_simi_ivt;
    vector<IndexNode>      vec_simi_idx;
    vector<IndexNode>      vec_cate_idx;
    vector<InvertNode>     vec_u2cate_ivt;
    vector<InvertNode>     vec_c2user_ivt;
    
    vector<int> vec_rule;
    vector<int> vec_rule_flag;
    vector<int>::iterator itv_flag;
    vec_rule.resize(cid_count_, 0);
    vec_rule_flag.resize(cid_count_, 0);
    int rule_flag_cnt = 0;
    
    long long from = 0;
    struct IndexNode       strt_index;
    vector<IndexNode>      vec_output_idx;
    vector<SimiInvertNode> vec_cid_score;
    vec_cid_score.resize(cid_count_);
    
    int index_size = (int)(*vec_c2k_idx).size();
    vec_output_idx.resize(index_size);
    
    for (i = 0; i < index_size; ++i)
    {
        offset = (*vec_c2k_idx)[i].offset;
        count  = (*vec_c2k_idx)[i].count;
        
        for (k = offset; k < offset + count; ++k)
        {
            kid        = (*vec_c2k_ivt)[k].id;
            offset_ivt = (*vec_k2c_idx)[kid].offset;
            count_ivt  = (*vec_k2c_idx)[kid].count;
            
            for (j = offset_ivt; j < offset_ivt + count_ivt; ++j)
            {
                cid = (*vec_k2c_ivt)[j].id;
                if (vec_rule[cid] == 0)
                    vec_rule_flag[rule_flag_cnt++] = cid;
                vec_rule[cid]++;
            }
        }
        ComputeCosine(count, MIN_SIM, rule_flag_cnt,        // 计算余弦
                      vec_rule, vec_rule_flag,
                      *vec_total_c2k_idx, vec_cid_score);
        strt_index.id     = (*vec_c2k_idx)[i].id;
        strt_index.count  = rule_flag_cnt;
        strt_index.offset = from * sizeof(SimiInvertNode);
        
        vec_output_idx[i] = strt_index;
        fwrite(&vec_cid_score[0], sizeof(SimiInvertNode), rule_flag_cnt, fp_similarity_ivt);
        
        from         += strt_index.count;
        rule_flag_cnt = 0;
        
        if (i % 100 == 0)
        {
            printf("\rtid: %d\trun_cnt: %d  %.2f%%", tid, i, 100.0 * i / index_size);
            fflush(stdout);
        }
    }
    printf("\rtid: %d\trun_cnt: %d  %.2f%%\n", tid, i, 100.0 * i / index_size);
    
    fwrite(&vec_output_idx[0], sizeof(IndexNode), vec_output_idx.size(), fp_similarity_idx);
    
    fclose(fp_similarity_ivt);
    fclose(fp_similarity_idx);
    
    res = IdxIvtCheck<IndexNode, SimiInvertNode>(f_similiarity_idx, f_similiarity_ivt);
    if (!res) printf("Check Invert: Error!\n");
    
    pthread_exit((void *)res);
}

// 合并相似各线程的用户结果
bool CombineSimiFile(const int THREAD_NUM,
                     const vector<string>& vec_output_idx,
                     const vector<string>& vec_output_ivt,
                     const string f_similiarity_idx,
                     const string f_similiarity_ivt)
{
    bool res = false;
    printf("\n-------------CombineSimiFile-------------\n");
    
    FILE * fp_des_idx = fopen(f_similiarity_idx.c_str(), "wb");
    if (!fp_des_idx)
    {
        printf("FILE %s open failed!\n", f_similiarity_idx.c_str());
        return res;
    }
    FILE * fp_des_ivt = fopen(f_similiarity_ivt.c_str(), "wb");
    if (!fp_des_ivt)
    {
        printf("FILE %s open failed!\n", f_similiarity_ivt.c_str());
        return res;
    }
    
    int i    = 0;
    UINT j   = 0;
    
    // 调出索引
    vector<IndexNode>      vec_thread_idx;
    vector<SimiInvertNode> vec_thread_ivt;
    vector<IndexNode> vec_index;
    vector<FILE*>     vec_fp_ivt;
    vec_fp_ivt.resize(THREAD_NUM);
    for (i = 0; i < THREAD_NUM; ++i)
    {
        vec_fp_ivt[i] = fopen(vec_output_ivt[i].c_str(), "rb");
        if (!vec_fp_ivt[i])
        {
            printf("FILE %s open failed!\n", vec_output_ivt[i].c_str());
            return res;
        }
    }
    
    size_t from = 0;
    for (i = 0; i < THREAD_NUM; ++i)
    {
        printf("\rthread_cnt: %d\n", i);
        printf("\tIndexFile : %s\n", vec_output_idx[i].c_str());
        printf("\tInvertFile: %s\n", vec_output_ivt[i].c_str());
        
        vec_thread_idx.clear();
        LoadIndex_Vector(vec_output_idx[i], vec_thread_idx);
        for (j = 0; j < vec_thread_idx.size(); ++j)
        {
            res = ReadInvert(vec_fp_ivt[i], vec_thread_idx[j], vec_thread_ivt);
            if (!res)
            {
                printf("Thread %d: ReadInvert Error!\n", i);
                return res;
            }
            fwrite(&vec_thread_ivt[0], sizeof(SimiInvertNode),
                    vec_thread_ivt.size(), fp_des_ivt);
            vec_thread_idx[j].offset = from * sizeof(SimiInvertNode);
            from += vec_thread_idx[j].count;
            vec_index.push_back(vec_thread_idx[j]);
        }
    }
    
    fwrite(&vec_index[0], sizeof(IndexNode), vec_index.size(), fp_des_idx);
    fclose(fp_des_idx);
    fclose(fp_des_ivt);
    for (i = 0; i < THREAD_NUM; ++i)
    {
        fclose(vec_fp_ivt[i]);
    }
    
    res = IdxIvtCheck<IndexNode, SimiInvertNode>(f_similiarity_idx, f_similiarity_ivt);
    if (!res)
    {
        printf("Check Invert: Error!\n");
        return res;
    }
    
    return true;
}

// 计算相似用户
bool CUserCF::ComputeSimilarity(const vector<IndexNode>&  vec_c2k_idx,
                                const vector<InvertNode>& vec_c2k_ivt,
                                const vector<IndexNode>&  vec_k2c_idx,
                                const vector<InvertNode>& vec_k2c_ivt,
                                const string& f_similiarity_idx,
                                const string& f_similiarity_ivt)
{
    cls_log_msg.LogMessages("\n-------------ComputeSimilarity-------------\n");
    bool res = false;
    
    FILE * fp_similarity_idx = fopen(f_similiarity_idx.c_str(), "wb");
    if (!fp_similarity_idx)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_similiarity_idx, __LINE__, res);
        return res;
    }
    FILE * fp_similarity_ivt = fopen(f_similiarity_ivt.c_str(), "wb");
    if (!fp_similarity_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_similiarity_ivt, __LINE__, res);
        fclose(fp_similarity_idx);
        return res;
    }
    int i   = 0;
    int k   = 0;
    int j   = 0;
    int cid = 0;
    int kid = 0;
    
    int offset     = 0;
    int count      = 0;
    int offset_ivt = 0;
    int count_ivt  = 0;
    vector<SimiInvertNode> vec_simi_ivt;
    vector<IndexNode>      vec_simi_idx;
    vector<IndexNode>      vec_cate_idx;
    vector<InvertNode>     vec_u2cate_ivt;
    vector<InvertNode>     vec_c2user_ivt;
    
    vector<int> vec_rule;
    vector<int> vec_rule_flag;
    vector<int>::iterator itv_flag;
    vec_rule.resize(cid_count_, 0);
    vec_rule_flag.resize(cid_count_, 0);
    int rule_flag_cnt = 0;
    
    long long from   = 0;
    struct IndexNode       strt_index;
    vector<IndexNode>      vec_output_idx;
    vector<SimiInvertNode> vec_cid_score;
    vec_cid_score.resize(cid_count_);
    
    int index_size = (int)vec_c2k_idx.size();
    vec_output_idx.resize(index_size);
    
    for (i = 0; i < index_size; ++i)
    {
        offset = vec_c2k_idx[i].offset;
        count  = vec_c2k_idx[i].count;
        
        if (count == 0) continue;
        
        for (k = offset; k < offset + count; ++k)
        {
            kid        = vec_c2k_ivt[k].id;
            offset_ivt = vec_k2c_idx[kid].offset;
            count_ivt  = vec_k2c_idx[kid].count;
            if (count_ivt == 0) continue;
            
            for (j = offset_ivt; j < offset_ivt + count_ivt; ++j)
            {
                cid = vec_k2c_ivt[j].id;
                if (vec_rule[cid] == 0)
                    vec_rule_flag[rule_flag_cnt++] = cid;
                vec_rule[cid]++;
            }
        }
        ComputeCosine(count, MIN_SIM, rule_flag_cnt,        // 计算余弦
                      vec_rule, vec_rule_flag,
                      vec_c2k_idx, vec_cid_score);
        strt_index.id     = vec_c2k_idx[i].id;
        strt_index.count  = rule_flag_cnt;
        strt_index.offset = from * sizeof(SimiInvertNode);
        
        vec_output_idx[i] = strt_index;
        fwrite(&vec_cid_score[0], sizeof(SimiInvertNode), rule_flag_cnt, fp_similarity_ivt);
        
        from         += strt_index.count;
        rule_flag_cnt = 0;
        
        if (i % 100 == 0)
        {
            printf("\rrun_cnt: %d  %.2f%%", i, 100.0 * i / vec_c2k_idx.size());
            fflush(stdout);
        }
    }
    printf("\rrun_cnt: %d  %.2f%%\n", i, 100.0 * i / vec_c2k_idx.size());
    
    fwrite(&vec_output_idx[0], sizeof(IndexNode), vec_output_idx.size(), fp_similarity_idx);
    
    fclose(fp_similarity_ivt);
    fclose(fp_similarity_idx);
    
    res = IdxIvtCheck<IndexNode, SimiInvertNode>(f_similiarity_idx, f_similiarity_ivt);
    cls_log_msg.LogMessages("IdxIvtCheck", __LINE__, res);
    if (!res) printf("Check Invert: Error!\n");
    
    return res;    
}

// 给每一用户推荐商品
bool CUserCF::MakeReco(const string& f_simi_user_ivt,
                       const string& f_cid_product_ivt,
                       const string& f_cid_recopro_idx,
                       const string& f_cid_recopro_ivt)
{
    cls_log_msg.LogMessages("\n-------------MakeReco-------------\n");
    bool res = false;
    
    FILE * fp_simi_user_ivt = fopen(f_simi_user_ivt.c_str(), "rb");
    if (!fp_simi_user_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_simi_user_ivt, __LINE__, res);
        return res;
    }
    FILE * fp_cid_product_ivt = fopen(f_cid_product_ivt.c_str(), "rb");
    if (!fp_cid_product_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_product_ivt, __LINE__, res);
        fclose(fp_simi_user_ivt);
        return res;
    }
    FILE * fp_cid_recopro_idx = fopen(f_cid_recopro_idx.c_str(), "wb");
    if (!fp_cid_recopro_idx)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_recopro_idx, __LINE__, res);
        fclose(fp_simi_user_ivt);
        fclose(fp_cid_product_ivt);
        return res;
    }
    FILE * fp_cid_recopro_ivt = fopen(f_cid_recopro_ivt.c_str(), "wb");
    if (!fp_cid_recopro_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_recopro_ivt, __LINE__, res);
        fclose(fp_simi_user_ivt);
        fclose(fp_cid_product_ivt);
        fclose(fp_cid_recopro_idx);
        return res;
    }
    
    vector<IndexNode>      vec_user_simi_idx;
    vector<IndexNode>      vec_user_order_idx;
    vector<SimiInvertNode> vec_user_simi_ivt;
    vector<SimiInvertNode>::iterator itv_simi_ivt;
    vector<int>           vec_user_order_ivt;
    vector<int>::iterator itv_order_ivt;
    set<int>    set_pro_bought;
    
    map<int, double> map_recopro_weight;
    map<int, double>::iterator itm_pro_weight;
    
    int i          = 0;
    int cid        = 0;
    int run_cnt    = 0;
    int reco_num   = 0;
    long long from = 0;
    struct IndexNode    strt_idx;
    vector<RecoProduct> vec_recopro_ivt;
    
    LoadIndex_Vector(fpath_similarity_idx_,  vec_user_simi_idx);
    LoadIndex_Vector(fpath_cid_product_idx_, vec_user_order_idx);
    
    for (vector<IndexNode>::iterator itv_simi_idx  = vec_user_simi_idx.begin();
                                     itv_simi_idx != vec_user_simi_idx.end();
                                     itv_simi_idx++)
    {
        // 处理已购买过的商品
        cid = itv_simi_idx->id;
        ReadInvert(fp_cid_product_ivt, vec_user_order_idx[cid],
                   vec_user_order_ivt);
        set_pro_bought.insert(vec_user_order_ivt.begin(),
                              vec_user_order_ivt.end());
        // 遍历相似用户
        ReadInvert(fp_simi_user_ivt, *itv_simi_idx, vec_user_simi_ivt);
        for (itv_simi_ivt  = vec_user_simi_ivt.begin();
             itv_simi_ivt != vec_user_simi_ivt.end();
             itv_simi_ivt++)
        {
            cid = itv_simi_ivt->cid;
            ReadInvert(fp_cid_product_ivt, vec_user_order_idx[cid],
                       vec_user_order_ivt);
            for (itv_order_ivt  = vec_user_order_ivt.begin();
                 itv_order_ivt != vec_user_order_ivt.end();
                 itv_order_ivt++)
            {
                if (set_pro_bought.find(*itv_order_ivt) !=
                    set_pro_bought.end()) continue;
                map_recopro_weight[*itv_order_ivt] += itv_simi_ivt->score;
            }
        }
        set_pro_bought.clear();
        // 统计被推荐商品
        reco_num = map_recopro_weight.size();
        vec_recopro_ivt.resize(reco_num);
        i = 0;
        for (itm_pro_weight  = map_recopro_weight.begin();
             itm_pro_weight != map_recopro_weight.end();
             itm_pro_weight++, i++)
        {
            vec_recopro_ivt[i].product_id = itm_pro_weight->first;
            vec_recopro_ivt[i].weight     = (int)(itm_pro_weight->second * 100);
        }
        if (reco_num <= MIN_RECO)
            sort(vec_recopro_ivt.begin(), vec_recopro_ivt.end());
        else
        {
            partial_sort(vec_recopro_ivt.begin(),
                         vec_recopro_ivt.begin() + reco_num,
                         vec_recopro_ivt.end());
            reco_num = MIN_RECO;
        }
        strt_idx.id     = cid;
        strt_idx.count  = reco_num;
        strt_idx.offset = from * sizeof(RecoProduct);
        fwrite(&strt_idx,           sizeof(IndexNode),   1, fp_cid_recopro_idx);
        fwrite(&vec_recopro_ivt[0], sizeof(RecoProduct), strt_idx.count, fp_cid_recopro_ivt);
        from += strt_idx.count;
        map_recopro_weight.clear();
        
        if (++run_cnt % 1000 == 0)
        {
            printf("\rrun_cnt: %d  %.2f%%", run_cnt, 100.0*run_cnt/vec_user_simi_idx.size());
            fflush(stdout);
        }
    }
    printf("\rrun_cnt: %d  %.2f%%\n", run_cnt, 100.0*run_cnt/vec_user_simi_idx.size());
    
    fclose(fp_simi_user_ivt);
    fclose(fp_cid_product_ivt);
    fclose(fp_cid_recopro_idx);
    fclose(fp_cid_recopro_ivt);
    
    return true;
}

inline bool GetProName(FILE * fp_proname_ivt,
                       struct IndexNode strt_index,
                       string& s_proname)
{
    vector<char> vec_invert;
    ReadInvert(fp_proname_ivt, strt_index, vec_invert);
    vec_invert.push_back('\0');
    s_proname = string(&vec_invert[0]);
    return true;
}

// 打印一部分用户的推荐结果
bool CUserCF::PrintReco(const string& f_cid_product_ivt,
                       const string& f_cid_recopro_ivt,
                       const string& f_printf_reco)
{
    cls_log_msg.LogMessages("\n-------------PrintReco-------------\n");
    bool res = false;
    
    FILE * fp_cid_product_ivt = fopen(f_cid_product_ivt.c_str(), "rb");
    if (!fp_cid_product_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_product_ivt, __LINE__, res);
        return res;
    }
    FILE * fp_cid_recopro_ivt = fopen(f_cid_recopro_ivt.c_str(), "rb");
    if (!fp_cid_recopro_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_cid_recopro_ivt, __LINE__, res);
        fclose(fp_cid_product_ivt);
        return res;
    }
    FILE * fp_printf_reco = fopen(f_printf_reco.c_str(), "wb");
    if (!fp_printf_reco)
    {
        cls_log_msg.LogMessages("OpenFile:" + f_printf_reco, __LINE__, res);
        fclose(fp_cid_product_ivt);
        fclose(fp_cid_recopro_ivt);
        return res;
    }
    FILE * fp_proname_ivt = fopen(fpath_pro_name_ivt_.c_str(), "rb");
    if (!fp_proname_ivt)
    {
        cls_log_msg.LogMessages("OpenFile:" + fpath_pro_name_ivt_, __LINE__, res);
        fclose(fp_cid_product_ivt);
        fclose(fp_cid_recopro_ivt);
        fclose(fp_printf_reco);
        return res;
    }
    
    vector<IndexNode>   vec_cid_order_index;
    vector<IndexNode>   vec_cid_reco_index;
    vector<IndexNode>::iterator itv_order_idx;
    vector<InvertNode>  vec_order_invert;
    vector<InvertNode>::iterator itv_invert;
    vector<IndexNode>   vec_proname_index;
    vector<char>        vec_proname_invert;
    map<int, IndexNode> map_proname_index;
    map<int, IndexNode>::iterator itm_proname;
    vector<RecoProduct> vec_reco_ivt;
    vector<RecoProduct>::iterator itv_reco_ivt;
    
    LoadIndex_Vector(fpath_cid_product_idx_, vec_cid_order_index);
    LoadIndex_Vector(fpath_cid_recopro_idx_, vec_cid_reco_index);
    LoadIndex_Vector(fpath_pro_name_idx_,    vec_proname_index);
    
    for (vector<IndexNode>::iterator itv_index  = vec_proname_index.begin();   // manage product name
                                     itv_index != vec_proname_index.end();
                                     itv_index++)
    {
        map_proname_index.insert(make_pair(itv_index->id, *itv_index));
    }
    vec_proname_index.clear();
    
    int i          = 0;
    int ivt_cnt    = 0;
    string s_line  = "\n=======================================================\n";
    string s_line2 = "\t___________________________________________________\n";
    string s_proname = "";
    for (i = 0; i < (int)vec_cid_order_index.size(); ++i)
    {
        ivt_cnt = 0;
        fprintf(fp_printf_reco, "%sNo.%2d;\tcid: %d;\tcnt: %d\n", 
                s_line.c_str(), i, vec_cid_order_index[i].id, vec_cid_order_index[i].count);
        ReadInvert(fp_cid_product_ivt, vec_cid_order_index[i], vec_order_invert);   // print order products
        for (itv_invert  = vec_order_invert.begin();
             itv_invert != vec_order_invert.end();
             itv_invert++)
        {
            itm_proname      = map_proname_index.find(itv_invert->id);
            if (itm_proname == map_proname_index.end()) continue;
            GetProName(fp_proname_ivt, itm_proname->second, s_proname);
            fprintf(fp_printf_reco, "\tNo.%2d\tpid:%9d\t\t\t\tpname:%s\n",
                    ++ivt_cnt, itv_invert->id, s_proname.c_str());
        }
        fprintf(fp_printf_reco, "%s", s_line2.c_str());
        
        ivt_cnt = 0;
        ReadInvert(fp_cid_recopro_ivt, vec_cid_reco_index[i], vec_reco_ivt);   // print reco products
        for (itv_reco_ivt  = vec_reco_ivt.begin();
             itv_reco_ivt != vec_reco_ivt.end();
             itv_reco_ivt++)
        {
            itm_proname      = map_proname_index.find(itv_reco_ivt->product_id);
            if (itm_proname == map_proname_index.end()) continue;
            GetProName(fp_proname_ivt, itm_proname->second, s_proname);
            fprintf(fp_printf_reco, "\tNo.%2d\tpid:%9d\twght:%4d\tpname:%s\n",
                    ++ivt_cnt, itv_reco_ivt->product_id, itv_reco_ivt->weight,
                    s_proname.c_str());
        }
        if (i % 10 == 0)
        {
            printf("\rrun_cnt: %d   %.2f%%", i, 100.0*i/vec_cid_order_index.size());
            fflush(stdout);
        }
        if (i > TEST_USER_CNT) break;
    }
    printf("\rrun_cnt: %d   %.2f%%\n", i, 100.0*i/vec_cid_order_index.size());
    
    fclose(fp_cid_product_ivt);
    fclose(fp_cid_recopro_ivt);
    fclose(fp_printf_reco);
    fclose(fp_proname_ivt);
    return true;
}

// 读配置文件
bool CUserCF::ReadConfigFile(const char* s_f_config, const char* s_f_log)
{
    cls_log_msg.LogMessages("\n<<<<<<<<<<<<<<<ReadConfigFile>>>>>>>>>>>>>>>\n");
    bool res      = true;

    string s_temp = "";
    CReadConfig ReadConfig(s_f_log);
    cout << "config file path: " << s_f_config << endl;
    
    // 程序主目录
    if (!(res = ReadConfig.GetConfigStr("filepath", "main_dir",        dir_main_,              1024, s_f_config))) return res;
    // 运行成功标志位
    if (!(res = ReadConfig.GetConfigStr("filepath", "run_flag",        fpath_run_flag_,        1024, s_f_config))) return res;
    // HuangJingang 数据
    if (!(res = ReadConfig.GetConfigStr("filepath", "db_dirpath",      dir_datapath_,          1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "pro_desp_ivt",    fpath_pro_desp_ivt_,    1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "cate2path_ivt",   fpath_cate2path_ivt_,   1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "pro_name_idx",    fpath_pro_name_idx_,   1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "pro_name_ivt",    fpath_pro_name_ivt_,   1024, s_f_config))) return res;
    // 用户、商品的黑名单
    if (!(res = ReadConfig.GetConfigStr("filepath", "black_list",      fpath_black_list_,      1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "trashy_products", fpath_trashy_products_, 1024, s_f_config))) return res;
    // 订单数据文件
    if (!(res = ReadConfig.GetConfigStr("filepath", "order_idx",       fpath_order_idx_,       1024, s_f_config))) return res;
    if (!(res = ReadConfig.GetConfigStr("filepath", "order_ivt",       fpath_order_ivt_,       1024, s_f_config))) return res;
    // 中间临时文件
    if (!(res = ReadConfig.GetConfigStr("filepath", "dir_temp",             dir_temp_,                   1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "cid_product_idx",       fpath_cid_product_idx_,       1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "cid_product_ivt",       fpath_cid_product_ivt_,       1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "similarity_idx",       fpath_similarity_idx_,       1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "similarity_ivt",       fpath_similarity_ivt_,       1024, s_f_config))) return res; // 
    // output
    if (!(res = ReadConfig.GetConfigStr("filepath", "cid_recopro_idx",       fpath_cid_recopro_idx_,      1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "cid_recopro_ivt",       fpath_cid_recopro_ivt_,      1024, s_f_config))) return res; // 
    if (!(res = ReadConfig.GetConfigStr("filepath", "printf_reco",           fpath_printf_reco_,          1024, s_f_config))) return res; // 
    
    // data
    if (!(res = ReadConfig.GetConfigStr("data", "AGENT_ORDER", s_temp,   1024, s_f_config))) return res;
    AGENT_ORDER = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "INIT_TIME_STAMP", s_temp,   1024, s_f_config))) return res;
    INIT_TIME_STAMP = atol(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "USER_ORDER_MIN", s_temp,   1024, s_f_config))) return res;
    USER_ORDER_MIN = atol(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "USER_INVERT_MAX", s_temp,   1024, s_f_config))) return res;
    USER_INVERT_MAX = atol(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "THREAD_NUM", s_temp,   1024, s_f_config))) return res;
    THREAD_NUM = atol(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "WRITE_BUF",   s_temp,   1024, s_f_config))) return res;
    WRITE_BUF = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "READ_BUF",    s_temp,   1024, s_f_config))) return res;
    READ_BUF  = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "TEST_ORDER_CNT",s_temp, 1024, s_f_config))) return res;
    TEST_ORDER_CNT = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "SORTMEMSIZE", s_temp,   1024, s_f_config))) return res;
    SORTMEMSIZE = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "MIN_SIM",     s_temp,   1024, s_f_config))) return res;
    MIN_SIM   = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "MIN_RECO",     s_temp,   1024, s_f_config))) return res;
    MIN_RECO   = atoi(s_temp.c_str());
    if (!(res = ReadConfig.GetConfigStr("data", "TEST_USER_CNT",s_temp,   1024, s_f_config))) return res;
    TEST_USER_CNT = atoi(s_temp.c_str());
    // switch
    if (!(res = ReadConfig.GetConfigStr("switch", "TEST",      s_temp,   1024, s_f_config))) return res;
    TEST = atoi(s_temp.c_str());
    
    dir_temp_              = dir_main_     + dir_temp_;
    fpath_run_flag_        = dir_main_     + fpath_run_flag_;
    fpath_cid_recopro_idx_ = dir_main_     + fpath_cid_recopro_idx_;
    fpath_cid_recopro_ivt_ = dir_main_     + fpath_cid_recopro_ivt_;
    fpath_printf_reco_     = dir_main_     + fpath_printf_reco_;
    
    fpath_pro_desp_ivt_    = dir_datapath_ + fpath_pro_desp_ivt_;
    fpath_cate2path_ivt_   = dir_datapath_ + fpath_cate2path_ivt_;
    fpath_pro_name_idx_    = dir_datapath_ + fpath_pro_name_idx_;
    fpath_pro_name_ivt_    = dir_datapath_ + fpath_pro_name_ivt_;
    
    fpath_cid_product_idx_ = dir_temp_ + fpath_cid_product_idx_;
    fpath_cid_product_ivt_ = dir_temp_ + fpath_cid_product_ivt_;
    fpath_similarity_idx_  = dir_temp_ + fpath_similarity_idx_;
    fpath_similarity_ivt_  = dir_temp_ + fpath_similarity_ivt_;
    return res;
}
