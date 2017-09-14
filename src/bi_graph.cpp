#include <limits.h>
#include <cmath>
#include "bi_graph.h"

#define Min(a, b) ( (a)<(b) ? (a):(b) )
#define Max(a, b) ( (a)>(b) ? (a):(b) )

bool CMP_BY_PTU(const DataNode& left, const DataNode& right) {
    return ((left.item_id  < right.item_id) ||
            (left.item_id == right.item_id  &&  left.timestamp  < right.timestamp) ||
            (left.item_id == right.item_id  &&  left.timestamp == right.timestamp && left.user_id < right.user_id));
}
bool CMP_BY_UTP(const DataNode& left, const DataNode& right) {
    return ((left.user_id  < right.user_id) ||
            (left.user_id == right.user_id  &&  left.timestamp  < right.timestamp) ||
            (left.user_id == right.user_id  &&  left.timestamp == right.timestamp && left.item_id < right.item_id));
}

BiGraph::BiGraph() {
    F_matrix_ivt_user_ = "../data/temp/matrix_user.ivt";
    F_matrix_ivt_item_ = "../data/temp/matrix_item.ivt";
}

BiGraph::~BiGraph()
{
    cls_logger.log("done.");
}

// 初始化
bool BiGraph::Init(const char* s_f_config)
{
    cout << "config file path: " << s_f_config << endl;
    bool res = true;

    if (!cls_logger.SetLogFile("./log.txt", "Spear")) return false;
    if (!ReadConfigFile(s_f_config))  {
        cls_logger.log(__LINE__, false, "ReadConfigFile");
        return false;
    }
    else {
        cls_logger.log(__LINE__, true, "ReadConfigFile");
    }
    return res;
}

//************* 读配置文件 *************//
bool BiGraph::ReadConfigFile(string s_f_config)
{
    cls_logger.log("<<<<<<<<<<<<<<<ReadConfigFile>>>>>>>>>>>>>>>");

    Config ReadConfig(s_f_config);

    bool res = true;

    if (!(res = ReadConfig.ReadInto("file", "train_data",  F_train_data_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "sim_item_bin",  F_output_ivt_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "sim_item_txt",  F_output_txt_))) return res;
    
    if (!(res = ReadConfig.ReadInto("data", "BUFFERCNT",   BUFFERCNT))) return res;
    if (!(res = ReadConfig.ReadInto("data", "SORTMEMSIZE", SORTMEMSIZE))) return res;

    if (!(res = ReadConfig.ReadInto("data", "lambda", lambda_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "rho",    rho_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "sigma",  sigma_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "top_reserve", top_reserve_))) return res;
    
    return res;
}

/////////// 主函数 /////////////
bool BiGraph::Calc()
{
    bool res = true;

    res = SourceDataManage();
    cls_logger.log(__LINE__, res, "SourceDataManage");
    if (!res) return res;

    res = Train();
    cls_logger.log(__LINE__, res, "Train");
    if (!res) return res;

    res = OutputTxt();
    cls_logger.log(__LINE__, res, "OutputTxt");
    if (!res) return res;

    cout << endl;
    return res;
}

//////////// 数据预处理 ////////////
bool BiGraph::SourceDataManage()
{
    cls_logger.log("==================SourceDataManage==================");
    bool res = true;
    
    
    string f_temp_data        = "../data/temp/data.dat";
    string f_temp_data_sorted = "../data/temp/data.sorted.dat";

    res = LoadData(f_temp_data);
    cls_logger.log(__LINE__, res, "Loaddata");
    if (!res) return res;

    cout << "\nSorting by pid/score/uid..." << endl;
    if(K_MergeFile<DataNode>(f_temp_data.c_str(),
                             f_temp_data_sorted.c_str(), CMP_BY_PTU,
                             SORTMEMSIZE) == -1) res = false;
    cls_logger.log(__LINE__, res, "K_MergeFile " + f_temp_data 
                                  + " to " + f_temp_data_sorted);
    if (!res) return res;


    res = MakeMatrixP2U(f_temp_data_sorted);
    cls_logger.log(__LINE__, res, "MakeMatrixP2U");
    if (!res) return res;

    cout << endl;
    RemoveFile(f_temp_data_sorted);
 
    cout << "\nSorting by uid/score/pid..." << endl;
    if(K_MergeFile<DataNode>(f_temp_data.c_str(),
                             f_temp_data_sorted.c_str(),
                             CMP_BY_UTP, SORTMEMSIZE) == -1) res = false;
    cls_logger.log(__LINE__, res, "K_MergeFile " + f_temp_data
                                  + " to " + f_temp_data_sorted);
    if (!res) return res;


    res = MakeMatrixU2P(f_temp_data_sorted);
    cls_logger.log(__LINE__, res, "MakeMatrixU2P");
    if (!res) return res;

    cout << endl;
    RemoveFile(f_temp_data);
    RemoveFile(f_temp_data_sorted);

    return res;
}

// input text: "uid itemID score timestamp"
// output bin: DataNode
bool BiGraph::LoadData(const string& dst) {
    cls_logger.log("-------------LoadData-------------");
    FILE *fp_dst = fopen(dst.c_str(), "wb");
    if (!fp_dst) {
        printf("error open file %s!\n", dst.c_str());
        return false;
    }
    ifstream fin(F_train_data_.c_str());
    string line;
    vector<string> sep_vec;
    vector<DataNode> buff;
    buff.resize(BUFFERCNT);
    int idc = 0;
    int cnt = 0;
    int mapped_uid = 0;
    int mapped_pid = 0;
    hash_map<string, int> hm_user_map;
    hash_map<string, int>::iterator its;
    hash_map<int, int> hm_item_map;
    hash_map<int, int>::iterator iti;

    while (getline (fin, line)) {
        stringUtils::split(line, " ", sep_vec);
        if (sep_vec.size() != 4) continue;
        if (atof(sep_vec[2].c_str() ) <= 0.0) continue;
        string user = sep_vec[0];
        int item = atoi(sep_vec[1].c_str());

        its = hm_user_map.find(user);
        if (its == hm_user_map.end()) {
            mapped_uid = hm_user_map.size();
            hm_user_map.insert(make_pair(user, mapped_uid));
        } else
            mapped_uid = its->second;
        iti = hm_item_map.find(item);
        if (iti == hm_item_map.end()) {
            mapped_pid = hm_item_map.size();
            hm_item_map.insert(make_pair(item, mapped_pid));
        } else
            mapped_pid = iti->second;

        buff[idc].user_id   = mapped_uid;
        buff[idc].item_id   = mapped_pid;
        buff[idc].score     = atof(sep_vec[2].c_str());
        buff[idc].timestamp = atoi(sep_vec[3].c_str());
        cnt++;

        if (++idc >= BUFFERCNT) {
            fwrite(&buff[0], sizeof(DataNode), idc, fp_dst);
            idc = 0;
        }
    }
    if (idc > 0) {
        fwrite(&buff[0], sizeof(DataNode), idc, fp_dst);
    }
    fclose(fp_dst);
    num_item_ = hm_item_map.size();
    num_user_ = hm_user_map.size();

    vec_item_id_map_.resize(num_item_);
    for (iti = hm_item_map.begin(); iti != hm_item_map.end(); iti++) {
        vec_item_id_map_[iti->second] = iti->first;
    }

    cls_logger.log("# user count: " + stringUtils::asString(num_user_));
    // cls_logger.log("# item count: " + stringUtils::asString(num_item_));
    // cls_logger.log("# input data: " + stringUtils::asString(cnt));
    return true;
}

void BiGraph::normalize(vector<MatrixInvert>& vec, float norm) {
    for (size_t i = 0; i < vec.size(); i++) {
        vec[i].score /= norm;
    }
}
    
// user -> [items ]
// input  bin: DataNode(sorted)
// output bin: idx, ivt
bool BiGraph::MakeMatrixU2P(const string& f_src) {
    cls_logger.log("-------------MakeMatrixU2P-------------");
    FILE* fp_src = fopen(f_src.c_str(), "rb");
    FILE* fp_ivt= fopen(F_matrix_ivt_item_.c_str(), "wb");
    int from = 0;
    float norm = 0.0;
    struct DataNode     node;
    struct MatrixIndex   strt_index;
    struct MatrixInvert  strt_invert;
    vector<MatrixInvert> vec_invert_buf;
    strt_index.count  = 0;
    strt_index.offset = 0;
    vec_matrix_idx_user_.resize(num_user_, strt_index);

    fread(&node, sizeof(DataNode), 1, fp_src);
    int uid_old           = node.user_id;
    strt_invert.id        = node.item_id;
    strt_invert.score     = node.score;
    strt_invert.timestamp = node.timestamp;
    vec_invert_buf.push_back(strt_invert);
    struct DataNode* readbuf = new struct DataNode[BUFFERCNT];
    if (!CheckMemAlloc(readbuf, BUFFERCNT)) {
        printf("error allocate mem!\n");
        return false;
    }

    while(!feof(fp_src))
    {
        int size = fread(readbuf, sizeof(DataNode), BUFFERCNT, fp_src);

        for (int i = 0; i < size; ++i) {
            if (uid_old != readbuf[i].user_id) {
                if (norm > 0.0) {
                    strt_index.norm   = norm;
                    strt_index.count  = (int)vec_invert_buf.size();
                    strt_index.offset = (long long)from * sizeof(MatrixInvert);

                    vec_matrix_idx_user_[uid_old] = strt_index;
                    normalize(vec_invert_buf, norm);
                    fwrite(&vec_invert_buf[0], sizeof(MatrixInvert), strt_index.count, fp_ivt);

                    vec_invert_buf.clear();
                    uid_old = readbuf[i].user_id;
                    from   += strt_index.count;
                    norm = 0.0;
                } else {
                    vec_invert_buf.clear();
                    uid_old = readbuf[i].user_id;
                    norm = 0;
                    printf("error norm: %f  (user: %d)", norm, uid_old);
                }
            }
            strt_invert.id        = readbuf[i].item_id;
            strt_invert.score     = readbuf[i].score;
            strt_invert.timestamp = readbuf[i].timestamp;
            vec_invert_buf.push_back(strt_invert);
            norm += readbuf[i].score;
        }
        if (size < BUFFERCNT) break; 
        memset(readbuf, 0, BUFFERCNT);
    }
    if (vec_invert_buf.size() > 0) {
        strt_index.norm   = norm;
        strt_index.count  = (int)vec_invert_buf.size();
        strt_index.offset = (long long)from * sizeof(MatrixInvert);
        vec_matrix_idx_user_[uid_old] = strt_index;
        normalize(vec_invert_buf, norm);
        fwrite(&vec_invert_buf[0], sizeof(MatrixInvert), strt_index.count, fp_ivt);
    }
    delete []readbuf;
    readbuf = NULL;
    fclose(fp_src);
    fclose(fp_ivt);
    bool res = IdxIvtCheck<MatrixIndex, MatrixInvert>(vec_matrix_idx_user_, F_matrix_ivt_item_);
    if (!res) printf("Check Invert: Error!\n");

    return res;
}

// item -> [user list]
// input  bin: DataNode(sorted)
// output bin: idx, ivt
bool BiGraph::MakeMatrixP2U(const string& f_src) {
    cls_logger.log("-------------MakeMatrixP2U-------------");
    FILE* fp_src = fopen(f_src.c_str(), "rb");
    FILE* fp_ivt= fopen(F_matrix_ivt_user_.c_str(), "wb");
    int from = 0;
    float norm = 0.0;
    struct DataNode     node;
    struct MatrixIndex   strt_index;
    struct MatrixInvert  strt_invert;
    vector<MatrixInvert> vec_invert_buf;
    strt_index.count  = 0;
    strt_index.offset = 0;
    vec_matrix_idx_item_.resize(num_item_, strt_index);

    fread(&node, sizeof(DataNode), 1, fp_src);
    int pid_old           = node.item_id;
    strt_invert.id        = node.user_id;
    strt_invert.score     = node.score;
    strt_invert.timestamp = node.timestamp;
    vec_invert_buf.push_back(strt_invert);
    struct DataNode* readbuf = new struct DataNode[BUFFERCNT];
    if (!CheckMemAlloc(readbuf, BUFFERCNT)) {
        printf("error allocate mem!\n");
        return false;
    }

    while(!feof(fp_src))
    {
        int size = fread(readbuf, sizeof(DataNode), BUFFERCNT, fp_src);

        for (int i = 0; i < size; ++i) {
            if (pid_old != readbuf[i].item_id) {
                strt_index.norm   = norm;
                strt_index.count  = (int)vec_invert_buf.size();
                strt_index.offset = (long long)from * sizeof(MatrixInvert);

                vec_matrix_idx_item_[pid_old] = strt_index;
                fwrite(&vec_invert_buf[0], sizeof(MatrixInvert), strt_index.count, fp_ivt);

                vec_invert_buf.clear();
                pid_old = readbuf[i].item_id;
                from   += strt_index.count;
                norm = 0.0;
            }
            strt_invert.id        = readbuf[i].user_id;
            strt_invert.score     = readbuf[i].score;
            strt_invert.timestamp = readbuf[i].timestamp;
            vec_invert_buf.push_back(strt_invert);
            norm += readbuf[i].score;
        }
        if (size < BUFFERCNT) break; 
        memset(readbuf, 0, BUFFERCNT);
    }
    if (vec_invert_buf.size() > 0) {
        strt_index.norm   = norm;
        strt_index.count  = (int)vec_invert_buf.size();
        strt_index.offset = (long long)from * sizeof(MatrixInvert);
        vec_matrix_idx_item_[pid_old] = strt_index;
        fwrite(&vec_invert_buf[0], sizeof(MatrixInvert),
               strt_index.count, fp_ivt);
    }
    delete []readbuf;
    readbuf = NULL;
    fclose(fp_src);
    fclose(fp_ivt);
    bool res = IdxIvtCheck<MatrixIndex, MatrixInvert>(vec_matrix_idx_item_, F_matrix_ivt_user_);
    if (!res) printf("Check Invert: Error!\n");

    return res;
}

bool BiGraph::Train() {
    cls_logger.log("-------------Train-------------");
    FILE* fp_ivt_item =  fopen(F_matrix_ivt_item_.c_str(),  "rb");
    FILE* fp_ivt_user =  fopen(F_matrix_ivt_user_.c_str(),  "rb");
    FILE* fp_output_ivt =  fopen(F_output_ivt_.c_str(),  "wb");
    if (!fp_ivt_item || !fp_ivt_user) { printf("ERROR open file!\n"); return false;}
     
    vector<MatrixInvert> vec_ivt_user;
    vector<MatrixInvert> vec_ivt_item;
    vector<MatrixInvert>::iterator iivt_user;
    vector<MatrixInvert>::iterator iivt_item;
    vector<SimIndex> vec_output_idx(num_item_);
    vector<SimInvert> vec_ivt_score(num_item_);
    SimInvert ini_node;
    int from = 0;
    ini_node.score = 0.0;

    for (size_t pid = 0; pid < num_item_; pid ++) {
        vec_ivt_score.assign(num_item_, ini_node);
        ReadInvert(fp_ivt_item, vec_matrix_idx_item_[pid], vec_ivt_user);
        for (iivt_user = vec_ivt_user.begin(); iivt_user != vec_ivt_user.end(); iivt_user++) {
            int uid = iivt_user->id;
            ReadInvert(fp_ivt_item, vec_matrix_idx_user_[uid], vec_ivt_item);
            for (iivt_item = vec_ivt_item.begin(); iivt_item != vec_ivt_item.end(); iivt_item++) {
                vec_ivt_score[iivt_item->id].score += iivt_user->score * iivt_item->score * guassian(iivt_user->timestamp - iivt_item->timestamp);
            }
        }
        float sum = 0.0;
        for (size_t i = 0; i < vec_ivt_score.size(); i++) {
            if (vec_ivt_score[i].score > 0.0) {
                vec_ivt_score[i].score /= pow(vec_matrix_idx_item_[i].norm, lambda_);
                vec_ivt_score[i].id = vec_item_id_map_[i];
                sum += vec_ivt_score[i].score;
            }
        }
        partial_sort(vec_ivt_score.begin(), vec_ivt_score.begin() + top_reserve_, vec_ivt_score.end() );
        int count = top_reserve_;
        for (int i = 0; i < top_reserve_; i++) {
            if (vec_ivt_score[i].score <= 0.0) {
                count = i;
                break;
            }
        }
        vec_output_idx[pid].id     = vec_item_id_map_[pid];
        vec_output_idx[pid].norm   = sum;
        vec_output_idx[pid].count  = count;
        vec_output_idx[pid].offset = (long long)from * sizeof(SimInvert);
        fwrite(&vec_ivt_score[0], sizeof(SimInvert), count, fp_output_ivt);
    }
    fclose(fp_ivt_item);
    fclose(fp_ivt_user);
    fclose(fp_output_ivt); 
    FILE* fp_output_idx  = fopen(F_output_idx_.c_str(),  "wb");
    fwrite(&vec_output_idx[0], sizeof(SimIndex), num_item_, fp_output_idx);
    fclose(fp_output_idx); 

    bool res = IdxIvtCheck<SimIndex, SimInvert>(F_output_idx_, F_output_ivt_);
    if (!res) cls_logger.log("check idx-ivt ERROR!");
    return res;
}

float BiGraph::guassian(int x) {
    return exp(-1.0 * (x * x) / 2.0 / sigma_/sigma_);
}
    
bool BiGraph::OutputTxt() {
    cls_logger.log("-------------OutputTxt-------------");
    FILE *fp_ivt = fopen(F_output_ivt_.c_str(), "rb");
    FILE *fp_txt = fopen(F_output_txt_.c_str(), "w");
    if (!fp_ivt || !fp_txt) {
        cls_logger.log(__LINE__, false, "ERROR open files!\n");
        return false;
    }
    vector<SimIndex>  vec_idx;
    vector<SimInvert> vec_ivt;
    LoadIndex_Vector(F_output_idx_, vec_idx);
    for (size_t i = 0; i < vec_idx.size(); i++) {
        fprintf(fp_txt, "[%ld]\tmainID=%d\tnorm=%f\tcount=%d\toffset=%lld\n", 
                i, vec_idx[i].id, vec_idx[i].norm, vec_idx[i].count, vec_idx[i].offset);
        ReadInvert(fp_ivt, vec_idx[i], vec_ivt);
        for (size_t j = 0; j < vec_ivt.size(); j++) {
            fprintf(fp_txt, "\t[%ld]\tid=%d\tscore=%f\n", 
                    j, vec_ivt[j].id, vec_ivt[j].score);
        }
    }
    fclose(fp_ivt);
    fclose(fp_txt);
    return true;
}

