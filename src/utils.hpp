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
    // string cmd = "ls -l " + path + "| awk ' $NF~/txt/ {print $NF}'"; // 限制.txt后缀
    string cmd = "ls -l " + path + "| awk '{if (NF > 5) {print $NF}}'";
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

// murmur2
unsigned int murMurHash(const void *key, int len) {
    const unsigned int m = 0x5bd1e995;
    const int r = 24;
    const int seed = 97;
    unsigned int h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char *data = (const unsigned char *)key;
    while(len >= 4)
    {
        unsigned int k = *(unsigned int *)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch(len)
    {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
        h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    return h;
}

template<typename T>
unsigned int myHash(T arg) {
    stringstream stream;
    stream << arg;
    string s_arg = stream.str();
    return murMurHash(s_arg.c_str(), s_arg.size());
}

