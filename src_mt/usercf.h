#include "mystruct.h"

class CUserCF
{
public:
    CUserCF();
    ~CUserCF();
    
public:
    bool Init(const char* cstr_config_file, const char* cstr_log_file);
    bool Calc();
    
private:
    bool ChangeWorkDir();             // 改动
    bool ReadConfigFile(const char* cstr_config_file, const char* cstr_log_file);
private:
    CMyLogMessages cls_log_msg;         // 日志对象
    
//////////////////////////// Defined in config.ini ///////////////////////////////
    int BUFFERCNT;
    int SORTMEMSIZE;
    int AGENT_ORDER;
    int WRITE_BUF;
    int READ_BUF;
    int MIN_SIM;
    int MIN_RECO;
    int TEST_USER_CNT;
    long long INIT_TIME_STAMP;
    size_t USER_ORDER_MIN;    // 有推荐资格用户的最低倒排数量
    int USER_INVERT_MAX;    // 有推荐资格用户的最高倒排(类别)数量
    int THREAD_NUM;
    
    bool TEST;
    int TEST_ORDER_CNT;
    
    string dir_main_;                       // 程序主目录
    string fpath_run_flag_;                 // 程序状态标志位

    //********* HuangJingang data *********//
    string dir_datapath_;                   // 数据路径
    string fpath_pro_desp_ivt_;             // 商品描述、分类信息
    string fpath_cate2path_ivt_;            // 分类 ID 与 path
    string fpath_pro_name_idx_;             // 商品名称
    string fpath_pro_name_ivt_;
    
    string fpath_black_list_;               // 用户黑名单
    string fpath_trashy_products_;          // 垃圾商品
    
    string fpath_order_idx_;                // 历史订单数据
    string fpath_order_ivt_;
    
    //********* 中间临时文件 **********//
    string dir_temp_;                       // 临时文件目录
    
    //********* 预处理文件 *********//
    string fpath_cid_product_idx_;          // 用户与购买商品
    string fpath_cid_product_ivt_;
    
    string fpath_similarity_idx_;           // 用户相似性的倒排索引
    string fpath_similarity_ivt_;
    
    //************* 输出 *************//
    string fpath_cid_recopro_idx_;          // 用户 -> 待推荐商品
    string fpath_cid_recopro_ivt_;
    
    string fpath_printf_reco_;              // 测试推荐结果
    
///////////////////////////////////////////////////////////
    
    int cid_count_;                         // cust id count
    int cateid_count_;                      // category id count
    vector<int> vec_cid_map_;               // cust id map
    
private:
    bool SourceDataManage(const string& fpath_cid_product_idx_,
                          const string& fpath_cid_product_ivt_,
                          const string& fpath_similarity_idx_,
                          const string& fpath_similarity_ivt_);
        bool ReadCategory(hash_map<int, int>& hm_product_category);
        bool LoadBlackList(const string& f_src,
                           set<int>& set_black_list);
        bool LoadTrashyProduct(const string& f_src,
                               set<int>& set_rubbish_products);
        bool ReadOrderHistory(const set<int>& set_black_list,
                              const set<int>& set_trashy_products,
                              const string& fpath_order_info,
                              int& cid_count_);
        bool CreateCidProidCateid(hash_map<int, int>& hm_product_category,
                                  const string& f_src,
                                  const string& f_cid_pro,
                                  const string& f_cid_cateid,
                                  vector<int>& vec_cid_map_,
                                  hash_map<int,int>& hm_category_kid);
        bool MakeInvertCid2Product(const string& f_src,
                                   const string& f_idx,
                                   const string& f_ivt,
                                   set<int>& set_filt_user);
        bool ShrinkCategory(const string& f_src,
                            const string& f_des);
        bool MakeInvertC2K(const string& f_src,
                           vector<IndexNode>&  vec_c2k_idx,
                           vector<InvertNode>& vec_c2k_ivt);
        bool MakeInvertK2C(const string& f_src,
                           const vector<IndexNode>& vec_c2k_idx,
                           const set<int>& set_filt_user,
                           vector<IndexNode>&  vec_k2c_idx,
                           vector<InvertNode>& vec_k2c_ivt);
        bool ManageThreads(vector<IndexNode>&  vec_c2k_idx,
                           vector<InvertNode>& vec_c2k_ivt,
                           vector<IndexNode>&  vec_k2c_idx,
                           vector<InvertNode>& vec_k2c_ivt,
                           const string& f_similiarity_idx,
                           const string& f_similiarity_ivt);
            // void *ComputeSimilarity_Thread(void*);
        bool ComputeSimilarity(const vector<IndexNode>&  vec_c2k_idx,
                               const vector<InvertNode>& vec_c2k_ivt,
                               const vector<IndexNode>&  vec_k2c_idx,
                               const vector<InvertNode>& vec_k2c_ivt,
                               const string& f_similiarity_idx,
                               const string& f_similiarity_ivt);
    bool MakeReco(const string& f_simi_user_ivt,
                  const string& f_cid_product_ivt,
                  const string& f_cid_recopro_idx,
                  const string& f_cid_recopro_ivt);
    bool PrintReco(const string& f_cid_product_ivt,
                   const string& f_cid_recopro_ivt,
                   const string& f_printf_reco);

};

