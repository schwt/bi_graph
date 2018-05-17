#include "struct.h"

class BiGraph 
{
public:
    BiGraph();
    ~BiGraph();

public:
    bool Init(const char* cstr_config_file);
    bool Calc();

private:
    bool ReadConfigFile(string file_config);
private:
    Logger logger;
    uSTL ustl;

    //////////////////////////// new ///////////////////////////////
    string DIR_temp_;
    // files
    string F_train_data_;
    string F_train_data_right_;
    string F_valid_reco_id_;
    string F_output_idx_;
    string F_output_ivt_;
    string F_output_txt_;
    // tmp file
    string F_matrix_ivt_user_;
    string F_matrix_ivt_item_;

    set<int> set_valid_reco_id_;
    hash_map<string, int> hm_user_map_;
    vector<int> vec_item_id_left_;
    vector<int> vec_item_id_right_;
    vector<double> vec_item_right_norm_;

    vector<MatrixIndex> vec_matrix_idx_user_;
    vector<MatrixIndex> vec_matrix_idx_item_;
    size_t num_user_;
    size_t num_item_left_;
    size_t num_item_right_;

    //////////////////////////// Defined in config.ini ///////////////////////////////
    
    int calc_in_mem_;
    int if_norm_result_;
    int top_reserve_;
    float lambda_;
    float rho_;
    float tau_;
    int progress_num_;
    float score_threshold_;

    // 读数据过滤的行为分数范围: score_min_ <= x <= score_max_
    int score_min_;
    int score_max_;
    // 数据源分隔符
    int delimiter_;

    int BUFFERCNT;
    int SORTMEMSIZE;
    int DISK_CACHE;
    bool is_double_behavior_;
    
///////////////////////////////////////////////////////////

    typedef vector<MatrixInvert> T_v_ivt;
private:
    bool SourceDataManage();

    bool LoadIds(string, vector<int>&, set<int>&);
    bool LoadData(string, string);
    bool LoadData_(string, string, vector<int>&, bool);
    bool MakeMatrixP2U(string);
    bool MakeMatrixU2P(string);
    bool Train();
    bool TrainInMem();
    bool OutputTxt();
    bool OutputTxtFormat();
    float guassian(int);
    void normalize(T_v_ivt&, float);
    double get_score(T_v_ivt::iterator, T_v_ivt::iterator);
};

