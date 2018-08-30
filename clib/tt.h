#include <iostream>
#include <set>
using namespace std ;

template <class T>
class Base {
public:
    T a ;
    Base(T b) {
        a = b ;  
    }
    T getA(){return a ;} //类内定义
    void setA(T c);  
    bool contain(set<T>&, T);
};

template <class T>   //模板在类外的定义
void  Base<T>::setA(T c) {
   a = c ;          
}

template <class T>   //模板在类外的定义
bool Base<T>::contain(set<T>& sets, T value) {
    set<T>::iterator iter = sets.find(value);
    return (iter != sets.end());
}
