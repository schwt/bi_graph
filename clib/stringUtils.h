#ifndef MY_STRING_UTILS_H
#define MY_STRING_UTILS_H

#include <string>  
#include <iostream>  
#include <fstream>  
#include <sstream> 
#include <vector> 

namespace stringUtils { 
    
    // Remove leading and trailing whitespace  
    void Trim( std::string& inout_s );
    void split(const std::string& src, const std::string& delimiter, std::vector<std::string>& v);
    std::vector<std::string> split(const std::string& src, const std::string& delimiter);

    template<class T> 
    std::string asString( const T& t ) {
        // Type T must support << operator  
        std::ostringstream ost;         
        ost << t;                       
        return ost.str();
    }

}
#endif

