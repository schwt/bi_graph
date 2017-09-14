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
    // files
    string F_train_data_;
    string F_output_idx_;
    string F_output_ivt_;
    string F_output_txt_;
    // string F_user_map_;
    // tmp file
    string F_matrix_ivt_user_;
    string F_matrix_ivt_item_;

    // vector<string> vec_user_id_map_;
    // vector<string> vec_item_id_map_;
    vector<int> vec_item_id_map_;

    vector<MatrixIndex> vec_matrix_idx_user_;
    vector<MatrixIndex> vec_matrix_idx_item_;
    size_t num_user_;
    size_t num_item_;

    //////////////////////////// Defined in config.ini ///////////////////////////////
    int top_reserve_;
    float lambda_;
    float rho_;
    float sigma_;

    int BUFFERCNT;
    int SORTMEMSIZE;
    int DISK_CACHE;
    
///////////////////////////////////////////////////////////

private:
    bool SourceDataManage();

    bool LoadData(const string&);
    bool MakeMatrixP2U(const string&);
    bool MakeMatrixU2P(const string&);
    bool Train();
    bool OutputTxt();
    float guassian(int);
    void normalize(vector<MatrixInvert>&, float);
};


