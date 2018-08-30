#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <string>  
#include <map>  
#include <iostream>  
#include <fstream>  
#include <sstream>
#include "stringUtils.h"
  
class Config {  

public:  
    bool m_error(std::string s) const {
        std::cout << "Config: ERROR: " << s << std::endl;
        return false;
    }
    int ReadInt( const std::string& section, const std::string& key) const {
        return Read<int>( section, key);}
    float ReadFloat( const std::string& section, const std::string& key) const {
        return Read<float>( section, key); }
    double ReadDouble( const std::string& section, const std::string& key) const {
        return Read<double>( section, key); }
    std::string ReadString( const std::string& section, const std::string& key) const {
        return Read<std::string>( section, key); }
  
    template<class T> T Read( const std::string& section, const std::string& in_key) const;
    template<class T> bool ReadInto ( const std::string& section, const std::string& in_key, T& out_var) const;
    template<class T> void ReadIntoN( const std::string& section, const std::string& in_key, T& out_var) const;
    template<class T> bool ReadInto ( const std::string& section, const std::string& in_key, T& out_var, T default_val) const;

    // Write or read configuration  
    friend std::ostream& operator<<( std::ostream& os, const Config& cf);
    friend std::istream& operator>>( std::istream& is, Config& cf);

public:
    Config();
    Config( std::string filename, std::string delimiter = "=", std::string comment = "#" );  
  
protected:  
    std::string m_Delimiter;  //!< separator between key and value  
    std::string m_Comment;    //!< separator between value and comments  
    std::map<std::string, std::map<std::string, std::string> > m_Contents;  // section: key: value
  
    typedef std::map<std::string, std::string>::iterator mapi;  
    typedef std::map<std::string, std::string>::const_iterator mapci;  
    typedef std::map<std::string, std::map<std::string, std::string> >::iterator mmapi;  
    typedef std::map<std::string, std::map<std::string, std::string> >::const_iterator mmapci;  

protected:  
    template<class T> static std::string T_as_string( const T& t );  
    template<class T> static T string_as_T( const std::string& s );  
  
public:  
    // Exception types  
    struct File_not_found {  
        std::string filename;  
        File_not_found( const std::string& filename_ = std::string() )  
            : filename(filename_) {} 
    };  
    struct Key_not_found {
        std::string key;  
        Key_not_found( const std::string& key_ = std::string() ) : key(key_) {} 
    };  
    struct Section_not_found {
        std::string key;  
        Section_not_found( const std::string& key_ = std::string() ) : key(key_) {} 
    };  
};  
  
  
/* static */  
template<class T>  
std::string Config::T_as_string( const T& t )  
{  
    // Type T must support << operator  
    std::ostringstream ost;  
    ost << t;  
    return ost.str();  
}  
  
  
/* static */  
template<class T>  
T Config::string_as_T( const std::string& s )  
{  
    // Type T must support >> operator  
    T t;
    std::istringstream ist(s);  
    ist >> t;  
    return t;  
}
  
  
/* static */  
template<>  
inline std::string Config::string_as_T<std::string>( const std::string& s )  
{  
    // Convert from a string to a string  
    // In other words, do nothing  
    return s;  
}  
  
  
/* static */  
template<>  
inline bool Config::string_as_T<bool>( const std::string& s )  
{  
    // Convert from a string to a bool  
    // Interpret "false", "F", "no", "n", "0" as false  
    // Interpret "true", "T", "yes", "y", "1", "-1", or anything else as true  
    bool b = true;  
    std::string sup = s;  
    for( std::string::iterator p = sup.begin(); p != sup.end(); ++p )  
        *p = toupper(*p);  // make string all caps  
    if( sup==std::string("FALSE") || sup==std::string("F") ||  
        sup==std::string("NO") || sup==std::string("N") ||  
        sup==std::string("0") || sup==std::string("NONE") )  
        b = false;  
    return b;  
}  
  
template<class T>  
void Config::ReadIntoN( const std::string& section, const std::string& key, T& var ) const  
{  
    // Get the value corresponding to key and store in var  
    // Return true if key is found  
    // Otherwise leave var untouched
    mmapci p = m_Contents.find(section);
    if ( p == m_Contents.end() ) throw Section_not_found(section);
    mapci pp = p->second.find(key);
    if ( pp == p->second.end() ) throw Key_not_found(key);
    var = string_as_T<T>( pp->second );
}
  
  
template<class T>  
bool Config::ReadInto( const std::string& section, const std::string& key, T& var ) const  
{  
    // Get the value corresponding to key and store in var  
    // Return true if key is found  
    // Otherwise set var to given default  
    mmapci p = m_Contents.find(section);  
    if ( p == m_Contents.end() ) return Config::m_error("section not found: " + section);
    mapci pp = p->second.find(key);
    if ( pp == p->second.end())  return Config::m_error("key not found: " + key);
    var = string_as_T<T>( pp->second );  
    return true;;  
}  

template<class T>  
bool Config::ReadInto( const std::string& section, const std::string& key, T& var, T default_val ) const
{
    // Get the value corresponding to key and store in var
    // Return true if key is found
    // Otherwise set var to given default
    var = default_val;
    mmapci p = m_Contents.find(section);
    if ( p == m_Contents.end() ) return true;
    mapci pp = p->second.find(key);
    if ( pp == p->second.end())  return true;
    var = string_as_T<T>( pp->second );
    return true;
}  

  
template<class T>  
T Config::Read( const std::string& section, const std::string& key) const  
{  
    mmapci p = m_Contents.find(section);
    if ( p == m_Contents.end() ) throw Section_not_found(section);
    mapci pp = p->second.find(key);
    if ( pp == p->second.end() ) throw Key_not_found(key);
    return string_as_T<T>( pp->second );
}
  
#endif

