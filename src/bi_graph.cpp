#include <limits.h>
#include <cmath>
#include "bi_graph.h"
#include "utils.hpp"
#include "../clib/directory.hpp"

// #define DEBUG

#define Min(a, b) ( (a)<(b) ? (a):(b) )
#define Max(a, b) ( (a)>(b) ? (a):(b) )

bool CMP_PTU(const DataNode& left, const DataNode& right) {
    return ((left.item_id  < right.item_id) ||
            (left.item_id == right.item_id  &&  left.timestamp  < right.timestamp) ||
            (left.item_id == right.item_id  &&  left.timestamp == right.timestamp && left.user_id < right.user_id));
}
bool CMP_UTP(const DataNode& left, const DataNode& right) {
    return ((left.user_id  < right.user_id) ||
            (left.user_id == right.user_id  &&  left.timestamp  < right.timestamp) ||
            (left.user_id == right.user_id  &&  left.timestamp == right.timestamp && left.item_id < right.item_id));
}

BiGraph::BiGraph() {
}

BiGraph::~BiGraph()
{
    logger.log("done.");
}

// 初始化
bool BiGraph::Init(const char* s_f_config)
{
    cout << "config file path: " << s_f_config << endl;
    bool res = true;

    res = logger.SetLogFile("./log.txt", "Bi_Graph");
    if (!res) return false;

    res = ReadConfigFile(s_f_config);
    if (!res) logger.log(__LINE__, false, "ReadConfigFile");

    return res;
}

//************* 读配置文件 *************//
bool BiGraph::ReadConfigFile(string s_f_config)
{
    logger.log("<<<<<<<<<<<<<<<ReadConfigFile>>>>>>>>>>>>>>>");

    Config ReadConfig(s_f_config);

    bool res = true;
    string snull = "";

    if (!(res = ReadConfig.ReadInto("file", "data_dir",         DIR_data_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "train_data",       F_train_data_))) return res;
    if (!(res = ReadConfig.ReadInto("file", "train_data_right", F_train_data_right_, snull))) return res;
    if (!(res = ReadConfig.ReadInto("file", "valid_reco_id",    F_valid_reco_id_, snull))) return res;
    if (!(res = ReadConfig.ReadInto("file", "invalid_reco_id",  F_invalid_reco_id_, snull))) return res;
    
    if (!(res = ReadConfig.ReadInto("switch", "delimiter",      delimiter_, 1))) return res;
    if (!(res = ReadConfig.ReadInto("switch", "calc_in_mem",    calc_in_mem_, 1))) return res;
    if (!(res = ReadConfig.ReadInto("switch", "if_norm_result", if_norm_result_, 1))) return res;
    if (!(res = ReadConfig.ReadInto("switch", "only_read_bin",  only_read_bin_, 0))) return res;
    if (!(res = ReadConfig.ReadInto("switch", "time_decay_type",time_decay_type_, 1))) return res;

    if (!(res = ReadConfig.ReadInto("data", "idc_user", idc_user_, 0))) return res;
    if (!(res = ReadConfig.ReadInto("data", "idc_item", idc_item_, 1))) return res;
    if (!(res = ReadConfig.ReadInto("data", "idc_rate", idc_rate_, 2))) return res;
    if (!(res = ReadConfig.ReadInto("data", "idc_time", idc_time_, 3))) return res;
    if (!(res = ReadConfig.ReadInto("data", "score_min", score_min_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "score_max", score_max_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "score_threshold", score_threshold_, 0.0f))) return res;
    if (!(res = ReadConfig.ReadInto("data", "lambda", lambda_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "rho",    rho_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "tau",  tau_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "top_reserve",  top_reserve_))) return res;
    if (!(res = ReadConfig.ReadInto("data", "BUFFERCNT",    BUFFERCNT))) return res;
    if (!(res = ReadConfig.ReadInto("data", "SORTMEMSIZE",  SORTMEMSIZE))) return res;

    tau_ *= 3600;  // 小时转秒
    F_output_idx_      = DIR_data_ + "/sim_item.idx";
    F_output_ivt_      = DIR_data_ + "/sim_item.ivt";
    F_output_txt_      = DIR_data_ + "/sim_item.txt";
    F_matrix_ivt_user_ = DIR_data_ + "/matrix_user.ivt";
    F_matrix_ivt_item_ = DIR_data_ + "/matrix_item.ivt";

    logger.log("paremeters");
    logger.log("\ttrain_data: "   + F_train_data_);
    logger.log("\tdata_dir: "     + DIR_data_);
    logger.log("\tscore_thresh: " + stringUtils::asString(score_threshold_));
    logger.log("\tlambda: "       + stringUtils::asString(lambda_));
    logger.log("\trho: "          + stringUtils::asString(rho_));
    logger.log("\tsigma: "        + stringUtils::asString(tau_));
    logger.log("\ttop_reserve: "  + stringUtils::asString(top_reserve_));
    logger.log("\tscore_min: "    + stringUtils::asString(score_min_));
    logger.log("\tscore_max: "    + stringUtils::asString(score_max_));
    return res;
}

/////////// 主函数 /////////////
bool BiGraph::Calc()
{
    bool res = true;

    // 只从旧二进制倒排索引数据转成文本
    if (only_read_bin_) {
        res = OutputTxt();
        if (!res) {
            logger.log(__LINE__, res, "OutputTxt");
        }
        return res;
    }

    res = SourceDataManage();
    if (!res) {
        logger.log(__LINE__, res, "SourceDataManage");
        return res;
    }

    if (calc_in_mem_ == 0)
        res = Train();
    else
        res = TrainInMem();
    if (!res) {
        logger.log(__LINE__, res, "Train");
        return res;
    }

    res = OutputTxt();
    if (!res) {
        logger.log(__LINE__, res, "OutputTxt");
        return res;
    }

    cout << endl;
    return res;
}

//////////// 数据预处理 ////////////
bool BiGraph::SourceDataManage()
{
    logger.log("==================SourceDataManage==================");
    bool res = true;
    
    
    string f_temp              = DIR_data_ + "/data.dat";
    string f_temp_right        = DIR_data_ + "/data_right.dat";
    string f_temp_sorted       = DIR_data_ + "/data.sorted.dat";
    string f_temp_right_sorted = DIR_data_ + "/data_right.sorted.dat";

    is_double_behavior_ = (F_train_data_right_ != "");

    res = LoadData(f_temp, f_temp_right);
    if (!res) {
        logger.log(__LINE__, res, "Loaddata");
        return res;
    }

    int ret = 0;
    logger.log("Sorting by pid/score/uid...");
    ret = K_MergeFile<DataNode>(f_temp.c_str(), f_temp_sorted.c_str(), CMP_PTU, SORTMEMSIZE);
    res = (ret != -1);
    if (!res) {
        logger.log(res, "K_MergeFile"+f_temp_sorted);
        return res;
    }

    res = MakeMatrixP2U(f_temp_sorted);
    if (!res) {
        logger.log(__LINE__, res, "MakeMatrixP2U");
        return res;
    }

    cout << endl;
    RemoveFile(f_temp_sorted);
 
    string f_sort_from = is_double_behavior_? f_temp_right: f_temp;

    logger.log("Sorting by uid/score/pid...");
    ret = K_MergeFile<DataNode>(f_sort_from.c_str(), f_temp_sorted.c_str(), CMP_UTP, SORTMEMSIZE);
    res = (ret != -1);
    if (!res) {
        logger.log(__LINE__, res, "K_MergeFile "+f_sort_from+ " -> " +f_temp_sorted);
        return res;
    }


    res = MakeMatrixU2P(f_temp_sorted);
    if (!res) {
        logger.log(__LINE__, res, "MakeMatrixU2P");
        return res;
    }

    cout << endl;
    // RemoveFile(f_temp);
    // RemoveFile(f_temp_sorted);

    return res;
}

// output bin: DataNode
bool BiGraph::LoadData(string dst, string dst_right) {
    logger.log("-------------LoadData-------------");
    bool res;

    if (!is_double_behavior_) {
        res = LoadData_(F_train_data_, dst, vec_item_id_left_, true);
        if (!res) {
            logger.log(__LINE__, res, "LoadData_ file 1/1");
            return res;
        }

        vec_item_id_right_.assign(vec_item_id_left_.begin(), vec_item_id_left_.end());
    } else {
        res = LoadData_(F_train_data_, dst, vec_item_id_left_, false);
        if (!res) {
            logger.log(__LINE__, res, "LoadData_ file 1/2");
            return res;
        }

        res = LoadData_(F_train_data_right_, dst_right, vec_item_id_right_, true);
        if (!res) {
            logger.log(__LINE__, res, "LoadData_ file 2/2");
            return res;
        }
    }

    // 加载可推荐结果集
    if (F_valid_reco_id_ != "") {
        res = LoadIds(F_valid_reco_id_, vec_item_id_right_, set_valid_reco_id_);
        if (!res) {
            logger.log(__LINE__, res, "LoadIds");
            return res;
        }
    }

    // 加载不可推荐结果集
    if (F_invalid_reco_id_ != "") {
        res = LoadIds(F_invalid_reco_id_, vec_item_id_right_, set_invalid_reco_id_);
        if (!res) {
            logger.log(__LINE__, res, "LoadIds");
            return res;
        }
    }


    num_item_left_  = vec_item_id_left_.size();
    num_item_right_ = vec_item_id_right_.size();
    num_user_       = hm_user_map_.size();
    hm_user_map_.clear();
    return res;
}

bool BiGraph::LoadData_(string src, string dst, vector<int>& vec_item_id, bool if_stat_item_norm) {
    logger.log("load data from: " + src);
    FILE *fp_dst = fopen(dst.c_str(), "wb");
    if (!fp_dst) {
        logger.log(__LINE__, false, "error open file " + dst);
        return false;
    }
    string line;
    vector<string> sep_vec;
    vector<DataNode> buff;
    buff.resize(BUFFERCNT);
    int idc = 0;
    int cnt1 = 0, cnt2 = 0;
    int mapped_uid = 0;
    int mapped_pid = 0;
    hash_map<string, int>::iterator itu;
    hash_map<int, int> hm_item_map;
    hash_map<int, int>::iterator itp;
    string s_delimiter;
    if (delimiter_ == 1) s_delimiter = ",";
    else if (delimiter_ == 2) s_delimiter = " ";
    else if (delimiter_ == 3) s_delimiter = "\t";
    else {
        logger.log(__LINE__, false, "error delimiter setting: " + stringUtils::asString(delimiter_));
        return false;
    }

    time_t now;
    time(&now);
    // int time_begin = now - 5*365*24*3600; // 5 years ago

    vector<string> vec_files = filesOfPath(src);
    sort(vec_files.begin(), vec_files.end());
    for (size_t i = 0; i < vec_files.size(); i++) {
        printf("\t%s\n", vec_files[i].c_str());
        ifstream fin(vec_files[i].c_str());
        while (getline (fin, line)) {
            cnt1++;
            stringUtils::split(line, s_delimiter, sep_vec);
            if (sep_vec.size() < 4) continue;
            string user   = sep_vec[idc_user_];
            int item      = atoi(sep_vec[idc_item_].c_str());
            int score     = atoi(sep_vec[idc_rate_].c_str());
            int timestamp = atoi(sep_vec[idc_time_].c_str());
            if (score < score_min_ || score > score_max_) continue;

            mapped_uid = ustl.get(hm_user_map_, user, (int)hm_user_map_.size());
            if (mapped_uid == (int)hm_user_map_.size()) {
                hm_user_map_.insert(make_pair(user, mapped_uid));
            }

            mapped_pid = ustl.get(hm_item_map, item, (int)hm_item_map.size());
            if (mapped_pid == (int)hm_item_map.size()) {
                hm_item_map.insert(make_pair(item, mapped_pid));
            }

            if (if_stat_item_norm) {
                if (mapped_pid+1 > (int)vec_item_right_norm_.size()) {
                    vec_item_right_norm_.resize(mapped_pid+1, 0);
                }
                vec_item_right_norm_[mapped_pid] += score;
            }

            buff[idc].user_id   = mapped_uid;
            buff[idc].item_id   = mapped_pid;
            buff[idc].score     = score;
            buff[idc].timestamp = timestamp;
            cnt2++;

            if (++idc >= BUFFERCNT) {
                fwrite(&buff[0], sizeof(DataNode), idc, fp_dst);
                idc = 0;
            }
        }
        // fin.close();
    }
    if (idc > 0) {
        fwrite(&buff[0], sizeof(DataNode), idc, fp_dst);
    }
    fclose(fp_dst);

    vec_item_id.resize(hm_item_map.size());
    for (itp = hm_item_map.begin(); itp != hm_item_map.end(); itp++) {
        vec_item_id[itp->second] = itp->first;
    }

    logger.log("# user count: " + stringUtils::asString(hm_user_map_.size()));
    logger.log("# item count: " + stringUtils::asString(hm_item_map.size()));
    logger.log("# input data: " + stringUtils::asString(cnt2) + "/" + stringUtils::asString(cnt1));
    hm_item_map.clear();
    if (cnt2 == 0) return false;
    return true;
}

bool BiGraph::LoadIds(string f_src, vector<int>& vec_item_id, set<int>& set_dst) {
    logger.log("------------- LoadIds -------------");

    int cnt     = 0;
    string line = "";
    hash_map<int, int> map_id;
    for (size_t i = 0; i < vec_item_id.size(); i++) {
        map_id[vec_item_id[i]] = i;
    }
    ifstream fin(f_src.c_str());
    while (getline (fin, line)) {
        cnt++;
        int item_id = atoi(line.c_str());
        int id = ustl.get(map_id, item_id, -1);
        if (id > 0) {
            set_dst.insert(id);
        }
    }
    logger.log("# read line : " + stringUtils::asString(cnt));
    logger.log("# res lt len: " + stringUtils::asString(set_dst.size()));
    if (set_dst.size() == 0)
        return false;
    return true;
}

void BiGraph::normalize(T_v_ivt& vec, float norm) {
    for (size_t i = 0; i < vec.size(); i++) {
        vec[i].score /= pow(norm, rho_);
    }
}
    
// user -> [items ]
// input  bin: DataNode(sorted)
// output bin: idx, ivt
bool BiGraph::MakeMatrixU2P(string f_src) {
    logger.log("-------------MakeMatrixU2P-------------");
    FILE* fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src) {
        logger.log(__LINE__, false, "error open file "+f_src+"!");
        return false;
    }
    FILE* fp_ivt = fopen(F_matrix_ivt_item_.c_str(), "wb");
    if (!fp_ivt) {
        logger.log(__LINE__, false, "error open file "+F_matrix_ivt_item_+"!");
        return false;
    }
    int from   = 0;
    float norm = 0.0;
    bool if_filter   = set_valid_reco_id_.size() > 0;
    bool if_infilter = set_invalid_reco_id_.size() > 0;
    DataNode     node;
    MatrixIndex   strt_index;
    MatrixInvert  strt_invert;
    T_v_ivt vec_invert_buf;
    strt_index.count  = 0;
    strt_index.offset = 0;
    vec_matrix_idx_user_.resize(num_user_, strt_index);

    fread(&node, sizeof(DataNode), 1, fp_src);
    int uid_old           = node.user_id;
    strt_invert.id        = node.item_id;
    strt_invert.score     = node.score;
    strt_invert.timestamp = node.timestamp;
    vec_invert_buf.push_back(strt_invert);
    norm += node.score;
    DataNode* readbuf = new struct DataNode[BUFFERCNT];
    if (!CheckMemAlloc(readbuf, BUFFERCNT)) {
        logger.log(__LINE__, false, "error allocate mem!");
        return false;
    }

    while(!feof(fp_src))
    {
        int size = fread(readbuf, sizeof(DataNode), BUFFERCNT, fp_src);

        for (int i = 0; i < size; ++i) {
            if (if_filter && !ustl.contain(set_valid_reco_id_, readbuf[i].item_id))
                continue;
            if (if_infilter && ustl.contain(set_invalid_reco_id_, readbuf[i].item_id))
                continue;
            if (uid_old != readbuf[i].user_id) {
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
    if (!res) logger.log(__LINE__, false, "Check Invert: Error!");

    logger.log("user idx: " + stringUtils::asString(vec_matrix_idx_user_.size()));
    return res;
}

// item -> [user list]
// input  bin: DataNode(sorted)
// output bin: idx, ivt
bool BiGraph::MakeMatrixP2U(string f_src) {
    logger.log("-------------MakeMatrixP2U-------------");
    FILE* fp_src = fopen(f_src.c_str(), "rb");
    if (!fp_src) {
        logger.log(__LINE__, false, "error open file "+f_src+"!");
        return false;
    }
    FILE* fp_ivt = fopen(F_matrix_ivt_user_.c_str(), "wb");
    if (!fp_ivt) {
        logger.log(__LINE__, false, "error open file "+F_matrix_ivt_user_+"!");
        return false;
    }
    int from = 0;
    float norm = 0.0;
    DataNode     node;
    MatrixIndex   strt_index;
    MatrixInvert  strt_invert;
    T_v_ivt vec_invert_buf;
    strt_index.count  = 0;
    strt_index.offset = 0;
    vec_matrix_idx_item_.resize(num_item_left_, strt_index);

    fread(&node, sizeof(DataNode), 1, fp_src);
    int pid_old           = node.item_id;
    strt_invert.id        = node.user_id;
    strt_invert.score     = node.score;
    strt_invert.timestamp = node.timestamp;
    vec_invert_buf.push_back(strt_invert);
    norm += node.score;
    DataNode* readbuf = new struct DataNode[BUFFERCNT];
    if (!CheckMemAlloc(readbuf, BUFFERCNT)) {
        logger.log(__LINE__, false, "error allocate mem!");
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
    if (!res) logger.log(__LINE__, false, "Check Invert: Error!\n");
    logger.log("item idx: " + stringUtils::asString(vec_matrix_idx_item_.size()));
    return res;
}

void printIdx(string s, int i, MatrixIndex node) {
#ifdef DEBUG
    printf("%sid------%d\n", s.c_str(), i);
    printf("%snorm  : %.1f\n", s.c_str(), node.norm);
    printf("%scount : %d\n", s.c_str(), node.count);
    printf("%soffset: %lld\n", s.c_str(), node.offset);
#endif
}
void printIvt(string s, MatrixInvert node) {
#ifdef DEBUG
    printf("%sid-----%d\n", s.c_str(), node.id);
    printf("%sscore: %.1f\n", s.c_str(), node.score);
#endif
}

double BiGraph::get_score(T_v_ivt::iterator node1, T_v_ivt::iterator node2) {
    return node1->score * node2->score * decay(node1->timestamp - node2->timestamp);
}

bool BiGraph::Train() {
    logger.log("-------------Train-------------");
    FILE* fp_ivt_user =  fopen(F_matrix_ivt_user_.c_str(),  "rb");
    if (!fp_ivt_user) {
        logger.log(__LINE__, false, "error open file "+F_matrix_ivt_user_+"!");
        return false;
    }
    FILE* fp_ivt_item =  fopen(F_matrix_ivt_item_.c_str(),  "rb");
    if (!fp_ivt_item) {
        logger.log(__LINE__, false, "error open file "+F_matrix_ivt_item_+"!");
        return false;
    }
    FILE* fp_output_ivt =  fopen(F_output_ivt_.c_str(),  "wb");
    if (!fp_output_ivt) {
        logger.log(__LINE__, false, "error open file "+F_output_ivt_+"!");
        return false;
    }
     
    T_v_ivt vec_ivt_user;
    T_v_ivt vec_ivt_item;
    T_v_ivt::iterator iivt_user;
    T_v_ivt::iterator iivt_item;

    vector<SimIndex> vec_output_idx(num_item_left_);
    vector<SimInvert> vec_ivt_score;
    vector<float> vec_collector_score(num_item_right_);
    vector<int> vec_collector_id(num_item_right_);
    int progress_num = Max(num_item_left_/10000, 1);
    int collector_id = 0;
    int from = 0;

    for (size_t i = 0; i < vec_item_right_norm_.size(); i++) {
        vec_item_right_norm_[i] = pow(vec_item_right_norm_[i], lambda_);
    }
    for (size_t pid = 0; pid < num_item_left_; pid ++) {
        if (vec_matrix_idx_item_[pid].count == 0) continue;
        ReadInvert(fp_ivt_user, vec_matrix_idx_item_[pid], vec_ivt_user);
        for (iivt_user = vec_ivt_user.begin(); iivt_user != vec_ivt_user.end(); iivt_user++) {
            if (vec_matrix_idx_user_[iivt_user->id].count == 0) continue;
            ReadInvert(fp_ivt_item, vec_matrix_idx_user_[iivt_user->id], vec_ivt_item);
            for (iivt_item = vec_ivt_item.begin(); iivt_item != vec_ivt_item.end(); iivt_item++) {
                size_t item_id = iivt_item->id;
                if (vec_item_id_right_[item_id] == vec_item_id_left_[pid]) continue;
                if (vec_collector_score[item_id] == 0)
                    vec_collector_id[collector_id++] = item_id;
                vec_collector_score[item_id] += get_score(iivt_user, iivt_item);
            }
        }
        if (collector_id== 0)
            continue;
        float sum = 0.0;
        vec_ivt_score.resize(collector_id);
        for (int i = 0; i < collector_id; i++) {
            int id = vec_collector_id[i];
            vec_ivt_score[i].id    = vec_item_id_right_[id];
            vec_ivt_score[i].score = vec_collector_score[id] / vec_item_right_norm_[id];
            sum += vec_ivt_score[i].score;
            vec_collector_id[i] = 0;
            vec_collector_score[id] = 0;
        }
        collector_id = 0;
        int limit_len =  Min(top_reserve_, (int)vec_ivt_score.size());
        partial_sort(vec_ivt_score.begin(), vec_ivt_score.begin() + limit_len, vec_ivt_score.end() );

        vec_output_idx[pid].id     = vec_item_id_right_[pid];
        vec_output_idx[pid].norm   = sum;
        vec_output_idx[pid].count  = limit_len;
        vec_output_idx[pid].offset = (long long)from * sizeof(SimInvert);
        fwrite(&vec_ivt_score[0], sizeof(SimInvert), limit_len, fp_output_ivt);
        from += vec_output_idx[pid].count;
        if  (pid % progress_num == 0)  {
            printf("progress: %.2f%% (%ld)\r", 100.0 * pid / num_item_left_, pid);
            fflush(stdout);
        }
    }
    printf("progress: 100.00%% (%ld)\n", num_item_left_);
    fclose(fp_ivt_item);
    fclose(fp_ivt_user);
    fclose(fp_output_ivt); 
    FILE* fp_output_idx  = fopen(F_output_idx_.c_str(),  "wb");
    if (!fp_output_idx) {
        logger.log(__LINE__, false, "error open file " + F_output_idx_);
        return false;
    }
    fwrite(&vec_output_idx[0], sizeof(SimIndex), num_item_left_, fp_output_idx);

    fclose(fp_output_idx); 

    bool res = IdxIvtCheck<SimIndex, SimInvert>(F_output_idx_, F_output_ivt_);
    if (!res) logger.log("check idx-ivt ERROR!");
    return res;
}

bool BiGraph::TrainInMem() {
    logger.log("-------------TrainInMem-------------");
    FILE* fp_output_ivt =  fopen(F_output_ivt_.c_str(),  "wb");
    if (!fp_output_ivt) {
        logger.log(__LINE__, false, "error open file " + F_output_ivt_);
        return false;
    }
     
    T_v_ivt vec_ivt_user;
    T_v_ivt vec_ivt_item;
    T_v_ivt::iterator iivt_user;
    T_v_ivt::iterator iivt_item;

    LoadIndex_Vector(F_matrix_ivt_user_, vec_ivt_user);
    LoadIndex_Vector(F_matrix_ivt_item_, vec_ivt_item);

    SimIndex index_node;
    vector<SimIndex> vec_output_idx;
    vector<SimInvert> vec_ivt_score(num_item_right_);
    vector<int>   vec_collector_id(num_item_right_);
    vector<float> vec_collector_score(num_item_right_, -1);
    int from = 0;

    CTimer timer;
    timer.StartTiming();

    int progress_num = Max(num_item_left_/10000, 1);

    for (size_t i = 0; i < vec_item_right_norm_.size(); i++) {
        vec_item_right_norm_[i] = pow(vec_item_right_norm_[i], lambda_);
    }
    for (size_t pid = 0; pid < num_item_left_; pid ++) {
        int from_u = vec_matrix_idx_item_[pid].offset / sizeof(MatrixInvert);
        int to_u   = vec_matrix_idx_item_[pid].count + from_u;
        int collector_id = 0;
        for (iivt_user = vec_ivt_user.begin() + from_u; iivt_user < vec_ivt_user.begin() + to_u; iivt_user++) {

            int user_id = iivt_user->id;

            int from_i = vec_matrix_idx_user_[user_id].offset / sizeof(MatrixInvert);
            int to_i   = vec_matrix_idx_user_[user_id].count + from_i;
            for (iivt_item = vec_ivt_item.begin() + from_i; iivt_item < vec_ivt_item.begin() + to_i; iivt_item++) {

                size_t item_id = iivt_item->id;
                if (vec_item_id_right_[item_id] == vec_item_id_left_[pid]) continue;

                if (vec_collector_score[item_id] == -1) {
                    vec_collector_id[collector_id++] = item_id;
                    vec_collector_score[item_id] = 0;
                }
                vec_collector_score[item_id] += get_score(iivt_user, iivt_item);
            }
        }
        if (collector_id == 0)
            continue;
        float sum = 0.0;
        for (int i = 0; i < collector_id; i++) {
            int id = vec_collector_id[i];
            vec_ivt_score[i].id = vec_item_id_right_[id];
            vec_ivt_score[i].score = vec_collector_score[id] / vec_item_right_norm_[id];
            sum += vec_ivt_score[i].score;
            vec_collector_score[id] = -1;
        }
        int len =  Min(top_reserve_, collector_id);
        partial_sort(vec_ivt_score.begin(), vec_ivt_score.begin() + len, vec_ivt_score.begin() + collector_id);
        if (if_norm_result_ == 1) {
            double max = vec_ivt_score[0].score != 0? vec_ivt_score[0].score: 1;
            for (int i = 0; i < len; i++) {
                vec_ivt_score[i].score /= max;
            }
        }

        index_node.id     = vec_item_id_left_[pid];
        index_node.norm   = sum;
        index_node.count  = len;
        index_node.offset = (long long)from * sizeof(SimInvert);
        vec_output_idx.push_back(index_node);
        fwrite(&vec_ivt_score[0], sizeof(SimInvert), len, fp_output_ivt);
        from += len;
        if (pid % progress_num == 0) {
            printf("progress: %.2f%% (%ld)\r", 100.0 * pid / num_item_left_, pid);
            fflush(stdout);
        }
    }
    printf("progress: 100.00%% (%ld)\n", num_item_left_);
    fclose(fp_output_ivt); 
    FILE* fp_output_idx  = fopen(F_output_idx_.c_str(),  "wb");
    if (!fp_output_idx) {
        logger.log(__LINE__, false, "error open file " + F_output_idx_);
        return false;
    }
    fwrite(&vec_output_idx[0], sizeof(SimIndex), vec_output_idx.size(), fp_output_idx);

    fclose(fp_output_idx); 

    timer.EndTiming();
    long used_seconds = timer.UsedSeconds();
    logger.log("# train time: " + stringUtils::asString(used_seconds));
    logger.log("#  mean time: " + stringUtils::asString(1.0 * used_seconds / num_item_left_));
    logger.log("#  reco rate: " + stringUtils::asString(100.*vec_output_idx.size()/num_item_left_) 
               + "% (" + stringUtils::asString(vec_output_idx.size()) 
               + "/"   + stringUtils::asString(num_item_left_) + ")"); 

    bool res = IdxIvtCheck<SimIndex, SimInvert>(F_output_idx_, F_output_ivt_);
    if (!res) logger.log(__LINE__, false, "check idx-ivt ERROR!");
    return res;
}

float BiGraph::decay(int x) {
    if (time_decay_type_ == 1) return half_decay(x);
    if (time_decay_type_ == 2) return guassian(x);
    return half_decay(x);
}
float BiGraph::half_decay(int x) {
    return exp2(-1.0 * abs(x) /  tau_) * 0.9 + 0.1;
}
float BiGraph::guassian(int x) {
    // return exp(-1.0 * x * x / 2.0 / tau_/tau_) * 0.9 + 0.1;
    return exp(-1.0 * x * x / 2.0 / tau_/tau_);
}

bool BiGraph::OutputTxt() {
    logger.log("-------------OutputTxt-------------");
    FILE *fp_ivt = fopen(F_output_ivt_.c_str(), "rb");
    if (!fp_ivt) {
        logger.log(__LINE__, false, "ERROR open files:"+F_output_ivt_);
        return false;
    }
    FILE *fp_txt = fopen(F_output_txt_.c_str(), "w");
    if (!fp_txt) {
        logger.log(__LINE__, false, "ERROR open files:"+F_output_txt_);
        return false;
    }
    vector<SimIndex>  vec_idx;
    vector<SimInvert> vec_ivt;
    LoadIndex_Vector(F_output_idx_, vec_idx);
    for (size_t i = 0; i < vec_idx.size(); i++) {
        if (vec_idx[i].count == 0) continue;
        fprintf(fp_txt, "%d\t", vec_idx[i].id);
        ReadInvert(fp_ivt, vec_idx[i], vec_ivt);
        int wcnt = 0;
        for (size_t j = 0; j < vec_ivt.size(); j++) {
            // if (vec_ivt[j].score <= score_threshold_)
            //     continue;
            if (wcnt++ > 0) fprintf(fp_txt, ",");
            fprintf(fp_txt, "%d:%g", vec_ivt[j].id, vec_ivt[j].score);
        }
        if (i != vec_idx.size()-1) {
            fprintf(fp_txt, "\n");
        }
    }
    fclose(fp_ivt);
    fclose(fp_txt);
    return true;
}
bool BiGraph::OutputTxtFormat() {
    logger.log("-------------OutputTxtFormat-------------");
    FILE *fp_ivt = fopen(F_output_ivt_.c_str(), "rb");
    if (!fp_ivt) {
        logger.log(__LINE__, false, "ERROR open files:"+F_output_ivt_);
        return false;
    }
    FILE *fp_txt = fopen(F_output_txt_.c_str(), "w");
    if (fp_txt) {
        logger.log(__LINE__, false, "ERROR open files:"+F_output_txt_);
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
            fprintf(fp_txt, "\t[%ld]\tid=%d\tscore=%g\n", 
                    j, vec_ivt[j].id, vec_ivt[j].score);
        }
    }
    fclose(fp_ivt);
    fclose(fp_txt);
    return true;
}


