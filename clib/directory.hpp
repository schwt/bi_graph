#include <stdio.h>
#include <dirent.h>  
#include <string.h>  
#include <sys/stat.h>
#include <vector>
#include <string>

// 返回目录下所有文件（不包括子目录）
// 若本身为文件，返回本身
std::vector<std::string> filesOfPath(std::string path) {

    std::vector<std::string> ret;
    struct stat s_buf;
    stat(path.c_str(), &s_buf);

    if (S_ISREG(s_buf.st_mode)) {
        // 是文件
        ret.push_back(path);
    } else if(S_ISDIR(s_buf.st_mode)) {
        // 是目录
        struct dirent *filename;
        DIR *dp = opendir(path.c_str());
        while ((filename = readdir(dp))) {
            std::string sub_file = path + "/" + filename->d_name;
            stat(sub_file.c_str(), &s_buf);
            if (S_ISREG(s_buf.st_mode)) {  
                ret.push_back(sub_file);
            }
        }
    }
    return ret;
}

