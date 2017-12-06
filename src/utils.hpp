#include <fstream>    
#include <string>    
#include <vector>    
#include <iostream>    
using namespace std;    
    
string& trim(string &s)
{
    if (s.empty()) return s;
    s.erase(0,s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);

    s.erase(0,s.find_first_not_of("\n"));
    s.erase(s.find_last_not_of("\n") + 1);
    return s;
}

vector<string> GetAllFiles( string path ) {
    string cmd = "ls -l " + path + "| awk ' $NF~/txt/ {print $NF}'";
    FILE *ret = popen(cmd.c_str(), "r");

    if (ret == NULL) {
        printf("check dir %s error!\n", path.c_str());
        exit(1);
    }

    char buff[1024] = {0};
    vector<string> res;
    while (fgets(buff, sizeof(buff), ret)) {
        string s = string(buff);
        s = trim(s);
        if (s.length() > 0) 
            res.push_back(s);
    }
    pclose(ret);
    return res;
}
