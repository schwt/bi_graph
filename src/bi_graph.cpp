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
    F_ivt_user_ = "../data/temp/matrix_user.ivt";
    F_ivt_item_ = "../data/temp/matrix_item.ivt";
}

BiGraph::~BiGraph()
{
    cls_log_msg.log("done.")
}

// 初始化
bool BiGraph::Init(const char* s_f_config)
{
    cout << "config file path: " << s_f_config << endl;
    bool res = true;

    if (!cls_log_msg.SetLogFile("./log.txt", "Spear")) return false;
    if (!ReadConfigFile(s_f_config))  {
        cls_log_msg.log(__LINE__, false, "ReadConfigFile");
        return false;
    }
    else {
        cls_log_msg.log(__LINE__, true, "ReadConfigFile");
    }
    return res;
}

//************* 读配置文件 *************//
bool BiGraph::ReadConfigFile(const char* s_f_config, const char* s_f_log)
{
    cls_log_msg.log("<<<<<<<<<<<<<<<ReadConfigFile>>>>>>>>>>>>>>>");

    Config ReadConfig(s_f_config);

    bool res = true;

    if (!(res = ReadConfig.ReadInto("file", "train_data",  F_train_data_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "sim_item_txt",  F_output_idx_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "sim_item_bin",  F_output_ivt_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "user_id_map",   F_user_map_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "item_id_map",   F_item_map_))) return res;
    
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
    cls_log_msg.log(__LINE__, res, "SourceDataManage");
    if (!res) return res;

    res = Train();
    cls_log_msg.log(__LINE__, res, "Train");
    if (!res) return res;

    res = OutputBin();
    cls_log_msg.log(__LINE__, res, "OutputBin");
    if (!res) return res;

    res = OutputTxt();
    cls_log_msg.log(__LINE__, res, "OutputTxt");
    if (!res) return res;

    cout << endl;
    return res;
}

//////////// 数据预处理 ////////////
bool BiGraph::SourceDataManage()
{
    cls_log_msg.log("==================SourceDataManage==================");
    bool res = true;
    
    
    string f_temp_data        = "../data/temp/data.dat";
    string f_temp_data_sorted = "../data/temp/data.sorted.dat";

    res = LoadData(f_temp_data);
    cls_log_msg.log(__LINE__, res, "Loaddata");
    if (!res) return res;

    cout << "\nSorting by pid/score/uid..." << endl;
    if(K_MergeFile<DataNode>(f_temp_data.c_str(),
                             f_temp_data_sorted.c_str(), CMP_BY_PTU,
                             SORTMEMSIZE) == -1) res = false;
    cls_log_msg.log(__LINE__, res, "K_MergeFile " + f_temp_data 
                                  + " to " + f_temp_data_sorted);
    if (!res) return res;


    res = MakeMatrixP2U(f_temp_data_sorted);
    cls_log_msg.log(__LINE__, res, "MakeMatrixP2U");
    if (!res) return res;

    cout << endl;
    RemoveFile(f_temp_data_sorted);
 
    cout << "\nSorting by uid/score/pid..." << endl;
    if(K_MergeFile<DataNode>(f_temp_data.c_str(),
                             f_temp_data_sorted.c_str(),
                             CMP_BY_UTP, SORTMEMSIZE) == -1) res = false;
    cls_log_msg.log(__LINE__, res, "K_MergeFile " + f_temp_data
                                  + " to " + f_temp_data_sorted);
    if (!res) return res;


    res = MakeMatrixU2P(f_temp_data_sorted);
    cls_log_msg.log(__LINE__, res, "MakeMatrixU2P");
    if (!res) return res;

    cout << endl;
    RemoveFile(f_temp_data);
    RemoveFile(f_temp_data_sorted);

    return res;
}

// input text: "uid itemID score timestamp"
// output bin: DataNode
bool BiGraph::LoadData(const string& dst) {
    cls_log_msg.log("-------------LoadData-------------");
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
    hash_map<string, int> hm_item_map;
    hash_map<string, int>::iterator ith;

    while (getline (fin, line)) {
        stringUtils::split(line, " ", sep_vec);
        if (sep_vec.size() != 4) continue;
        if (atof(vec[2].c_str() ) <= 0.0) continue;
        string user = sep_vec[0];
        string item = sep_vec[1];

        ith = hm_user_map.find(user);
        if (ith == hm_user_map.end()) {
            mapped_uid = hm_user_map.size();
            hm_user_map.insert(make_pair(user, mapped_uid));
        } else
            mapped_uid = ith->second;
        ith = hm_item_map.find(item);
        if (ith == hm_item_map.end()) {
            mapped_pid = hm_item_map.size();
            hm_item_map.insert(make_pair(item, mapped_pid));
        } else
            mapped_pid = ith->second;

        buff[idc].user_id   = mapped_uid;
        buff[idc].item_id   = mapped_pid;
        buff[idc].score     = atof(vec[2].c_str());
        buff[idc].timestamp = atoi(vec[3].c_str());
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

    vec_user_id_map_.resize(num_user_);
    vec_item_id_map_.resize(num_item_);
    for (ith = hm_user_map.begin(); itm != hm_user_map.end(); itm++) {
        vec_user_id_map_[itm->second] = itm->first;
    }
    for (ith = hm_item_map.begin(); itm != hm_item_map.end(); itm++) {
        vec_item_id_map_[itm->second] = itm->first;
    }

    cls_log_msg.log("# user count: %ld\n", num_user_);
    cls_log_msg.log("# item count: %ld\n", num_item_);
    cls_log_msg.log("# input data: %d\n", cnt);
    return true;
}

void BiGraph::normalize(vector<MatrixInvert>& vec, float norm) {
    for (size_t i = 0; i < vec_invert_buf.size(); i++) {
        vec_invert_buf[i].score /= norm;
    }
}
    
// user -> [items ]
// input  bin: DataNode(sorted)
// output bin: idx, ivt
bool BiGraph::MakeMatrixU2P(const string& f_src) {
    cls_log_msg.log("-------------MakeMatrixU2P-------------");
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
    cls_log_msg.log("-------------MakeMatrixP2U-------------");
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
    cls_log_msg.log("-------------Train-------------");
    FILE* fp_ivt_item =  fopen(F_matrix_ivt_item_.c_str(),  "rb");
    FILE* fp_ivt_user =  fopen(F_matrix_ivt_user_.c_str(),  "rb");
    FILE* fp_output_ivt =  fopen(F_output_ivt_.c_str(),  "wb");
    if (!fp_ivt_item || !fp_ivt_user) { printf("ERROR open file!\n"); return false;}
     
    vector<MatrixInvert> vec_ivt_user;
    vector<MatrixInvert> vec_ivt_item;
    vector<MatrixInvert>::iterator iivt_user;
    vector<MatrixInvert>::iterator iivt_item;
    tNode ini_node;
    ini_node.id = -1, ini_node.score = -1.0;
    vector<tNode> vec_id_score;

    vec_id_score.resize(num_item_, ini_node);

    for (size_t pid = 0; pid < num_item_; pid ++) {
        vec_id_score.assign(num_item_, ini_node);
        ReadInvert(fp_ivt_item, vec_matrix_idx_item_[pid], vec_ivt_user);
        for (iivt_user = vec_ivt_user.begin(); iivt_user != vec_ivt_user.end(); iivt_user++) {
            int uid = iivt_user->user_id;
            ReadInvert(fp_ivt_item, vec_matrix_idx_user_[uid], vec_ivt_item);
            for (iivt_item = vec_ivt_item.begin(); iivt_item != vec_ivt_item.end(); iivt_item++) {
                int id = iivt_item->item_id;
                vec_id_score[id].id = id;
                vec_id_score[id].score += iivt_user->score * iivt_item->score * guassian(iivt_user->timestamp - iivt_item->timestamp);
            }
        }
        for (size_t i = 0; i < vec_id_score.size(); i++) {
            if (vec_id_score[i].id > 0)
                vec_id_score[i].score /= por(vec_matrix_idx_item_[i].norm, lambda_);
        }
        partial_sort(vec_id_score.begin(), vec_id_score.begin() + top_reserve_, vec_id_score.end() );
         
    }
    fclose(fp_ivt_item);
    fclose(fp_ivt_user);
    return true;
}

float BiGragh::guassian(int x) {
    return exp(-1.0 * (x * x) / 2.0 / sigma_/sigma_);
}
    
float BiGraph::calc_rse(const vector<float>& vec1, const vector<float>& vec2) {
    if (vec1.size() != vec2.size()) return -1.0;
    double sum = 0.0;
    for (size_t i = 0; i < vec1.size(); i++) {
        sum += (vec1[i] - vec2[i]) * (vec1[i] - vec2[i]);
    }
    return sqrt(sum);
}

bool BiGraph::WriteScore() {
    cls_log_msg.log("-------------WriteScore-------------");
    FILE *fp_user = fopen(F_score_user_.c_str(), "w");
    FILE *fp_item = fopen(F_score_item_.c_str(), "w");
    if (!fp_user || !fp_item) {
        printf("ERROR open files!\n");
        return false;
    }
    vector<tNode> vec_buff(num_user_);
    for (size_t uid = 0; uid < num_user_; uid++)
        fprintf(fp_user, "%d\t%f\n", vec_uid_[uid], vec_score_user_[uid]);
    for (size_t pid = 0; pid < num_item_; pid++)
        fprintf(fp_item, "%d\t%f\n", vec_pid_[pid], vec_score_item_[pid]);
    fclose(fp_user);
    fclose(fp_item);
    return true;
}

bool BiGraph::WriteTopUser() {
    cls_log_msg.log("-------------WriteTopUser-------------");
    
    vector<tNode> vec_buff;
    vec_buff.resize(num_user_);
    for (size_t i = 0; i < num_user_; i++) {
        vec_buff[i].id = vec_uid_[i];
        vec_buff[i].score = vec_score_user_[i];
    }
    sort(vec_buff.begin(), vec_buff.end());

    FILE* fp_dst = fopen(F_top_user_.c_str(), "w");
    if (!fp_dst) {
        printf("ERROR open file: %s\n", F_top_user_.c_str());
        return false;
    }
    for (size_t i = 0; i < Min(top_num_, num_user_); i++) {
        fprintf(fp_dst, "%d\t%.10f\n", vec_buff[i].id, vec_buff[i].score);
    }
    fclose(fp_dst);
    return true;
}

bool BiGraph::WriteTopItem() {
    cls_log_msg.log("-------------WriteTopItem-------------");
    
    vector<tNode> vec_buff;
    vec_buff.resize(num_item_);
    for (size_t i = 0; i < num_item_; i++) {
        vec_buff[i].id = vec_pid_[i];
        vec_buff[i].score = vec_score_item_[i];
    }
    sort(vec_buff.begin(), vec_buff.end());

    FILE* fp_dst = fopen(F_top_item_.c_str(), "w");
    if (!fp_dst) {
        printf("ERROR open file: %s\n", F_top_item_.c_str());
        return false;
    }
    for (size_t i = 0; i < Min(top_num_, num_item_); i++) {
        fprintf(fp_dst, "%d\t%.10f\n", vec_buff[i].id, vec_buff[i].score);
    }
    fclose(fp_dst);
    return true;
}


