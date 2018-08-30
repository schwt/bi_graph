#ifndef MYUSTL_H
#define MYUSTL_H

#include <stdio.h>   
#include <stdlib.h>   
#include <string>
#include <iostream>
#include <fstream>
#include <time.h>
#include <set>
#include <map>
#include "my_hash_map.h"

using namespace std;

class uSTL
{ 
public: 
    uSTL() {}
    ~uSTL() {};

public:
    template <class V> 
        bool contain(set<V>&, V);

    template <class K, class V> 
        V get(map<K, V>&, K, V);

    template <class K, class V> 
        V get(hash_map<K, V>&, K, V);


private:

};

template <class V>
bool uSTL::contain(set<V>& setter, V value) {
    typename set<V>::iterator iter = setter.find(value);
    return (iter != setter.end());
}


template <class K, class V> 
V uSTL::get(map<K, V>& mapper, K key, V def_value) {
    typename map<K, V>::iterator iter = mapper.find(key);
    if (iter == mapper.end())
        return def_value;
    return iter->second;
}

template <class K, class V> 
V uSTL::get(hash_map<K, V>& mapper, K key, V def_value) {
    typename hash_map<K, V>::iterator iter = mapper.find(key);
    if (iter == mapper.end())
        return def_value;
    return iter->second;
}


#endif

