#include <fstream>    
#include <string>    
#include <vector>    
#include <iostream>    
using namespace std;    
    

vector<string> GetAllFiles( string path ) {
    string cmd = "ls -l " + path + "| awk ' $NF~/txt/ {print $NF}'";
    FILE *ret = popen(cmd.c_str(), "r");

    if (ret == NULL) {
        printf("check dir %s error!\n", path.c_str());
        exit(1);
    }

    char buff[1024] = {0};
    while (fgets(buff, sizeof(buff), ret)) {
        printf("file: %s\n", buff);
    }
    vector<string> res;
    return res;
}

