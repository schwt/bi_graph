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
    Logger cls_logger;

    //////////////////////////// new ///////////////////////////////
    string DIR_temp_;
    // files
    string F_train_data_;
    string F_output_idx_;
    string F_output_ivt_;
    string F_output_txt_;
    // tmp file
    string F_matrix_ivt_user_;
    string F_matrix_ivt_item_;

    vector<int> vec_item_id_map_;

    vector<MatrixIndex> vec_matrix_idx_user_;
    vector<MatrixIndex> vec_matrix_idx_item_;
    size_t num_user_;
    size_t num_item_;

    //////////////////////////// Defined in config.ini ///////////////////////////////
    
    int is_multifile_;
    int calc_in_mem_;
    int top_reserve_;
    float lambda_;
    float rho_;
    float sigma_;
    int progress_num_;
    // 读数据过滤的行为分数范围: score_min_ <= x <= score_max_
    int score_min_;         
    int score_max_;

    int BUFFERCNT;
    int SORTMEMSIZE;
    int DISK_CACHE;
    
///////////////////////////////////////////////////////////

private:
    bool SourceDataManage();

    bool LoadData(const string&);
    bool LoadMultiData(const string&);
    bool MakeMatrixP2U(const string&);
    bool MakeMatrixU2P(const string&);
    bool Train();
    bool TrainInMem();
    bool OutputTxt();
    bool OutputTxtFormat();
    float guassian(int);
    void normalize(vector<MatrixInvert>&, float);
};


