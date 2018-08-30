#include "stringUtils.h"

// Remove leading and trailing whitespace  
void stringUtils::Trim( std::string& inout_s )  
{  
    static const char whitespace[] = " \n\t\v\r\f";  
    inout_s.erase( 0, inout_s.find_first_not_of(whitespace) );  
    inout_s.erase( inout_s.find_last_not_of(whitespace) + 1U );  
}  
 
void stringUtils::split(const std::string& src, const std::string& delimiter, std::vector<std::string>& v) {
    std::string::size_type pos1, pos2;
    pos2 = src.find(delimiter);
    pos1 = 0;
    v.clear();
    while(std::string::npos != pos2) {
      v.push_back(src.substr(pos1, pos2-pos1));
      pos1 = pos2 + delimiter.size();
      pos2 = src.find(delimiter, pos1);
    }
    if(pos1 != src.length())
        v.push_back(src.substr(pos1));
}

std::vector<std::string> stringUtils::split(const std::string& src, const std::string& delimiter) {
    std::string::size_type pos1, pos2;
    pos2 = src.find(delimiter);
    pos1 = 0;
    std::vector<std::string> v;
    while(std::string::npos != pos2) {
      v.push_back(src.substr(pos1, pos2-pos1));
      pos1 = pos2 + delimiter.size();
      pos2 = src.find(delimiter, pos1);
    }
    if(pos1 != src.length())
        v.push_back(src.substr(pos1));
    return v;
}

/*
template<class T>  
std::string stringUtils::asString( const T& t )   
{  
    // Type T must support << operator  
    std::ostringstream ost;  
    ost << t;  
    return ost.str();  
}
*/

