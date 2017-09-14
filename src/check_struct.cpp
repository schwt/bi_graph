#include "struct.h"

using namespace std;

string f_idx1 = "../data/temp/matrix_item.ivt";
string f_idx2 = "../data/temp/matrix_user.ivt";

int LEN = 10;

bool check_ivt(string f_idx) {

    vector<MatrixInvert> vec_buff(LEN);

    FILE* fp = fopen(f_idx.c_str(), "rb");
    if (!fp) {
        printf("error open file: %s\n", f_idx.c_str());
        return false;
    }
    printf("read file: %s\n", f_idx.c_str());

    int cnt = fread(&vec_buff[0], sizeof(MatrixInvert), LEN, fp);

    for (int i = 0; i < cnt; i++) {
        printf("[%d] id: %d\n", i, vec_buff[i].id);
        printf("   \ttm: %d\n",   vec_buff[i].timestamp);
        printf("   \tsc: %.2f\n",  vec_buff[i].score);
    }
    fclose(fp);
    return true;
}
        
int main(int argc, char** argv) {
    check_ivt(f_idx1);
    check_ivt(f_idx2);
    return 0;
}

