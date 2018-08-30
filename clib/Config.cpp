#include "Config.h"  
// #include "stringUtils.h"
  
using namespace std;  
  
Config::Config(string filename, string delimiter,  
               string comment)  
               : m_Delimiter(delimiter), m_Comment(comment)  
{  
    // Construct a Config, getting keys and values from given file  
    std::ifstream in( filename.c_str() );  
    if( !in ) throw File_not_found( filename );   
    in >> (*this);  
}
  
Config::Config()  
: m_Delimiter( string(1,'=') ), m_Comment( string(1,'#') )  
{  
    // Construct a Config without a file; empty  
}


std::ostream& operator<<( std::ostream& os, const Config& cf )  
{  
    // Save a Config to os  
    for( Config::mmapci p = cf.m_Contents.begin();  p != cf.m_Contents.end(); ++p )  
    {
        for (Config::mapci pp = p->second.begin(); pp != p->second.end(); pp++) {
            os << p->first << ": " << pp->first << ": " << pp->second << endl;
        }
    }  
    return os;  
}  
  
std::istream& operator>>( std::istream& is, Config& cf )  
{  
    // Load a Config from is  
    // Read in keys and values, keeping internal whitespace  
    typedef string::size_type pos;  
    const string& delim  = cf.m_Delimiter;  // separator  
    const string& comm   = cf.m_Comment;    // comment  
    const pos skip = delim.length();        // length of separator  
    string section = "";
  
    string nextline = "";  // might need to read ahead to see where value ends  
  
    while( is || nextline.length() > 0 )  
    {  
        // Read an entire line at a time  
        string line;  
        if( nextline.length() > 0 )  
        {  
            line = nextline;  // we read ahead; use it now  
            nextline = "";  
        }  
        else  
        {  
            std::getline( is, line );  
        }  
  
        // Ignore comments  
        line = line.substr( 0, line.find(comm) );  
  
        // Parse the line if it contains a delimiter  
        pos delimPos = line.find( delim );  
        if( delimPos < string::npos )  
        {  
            // Extract the key  
            string key = line.substr( 0, delimPos );  
            line.replace( 0, delimPos+skip, "" );  
  
            // See if value continues on the next line  
            // Stop at blank line, next line with a key, end of stream,  
            // or end of file sentry 
/*
            bool terminate = false;  
            while( !terminate && is )  
            {  
                std::getline( is, nextline );  
                terminate = true;  
  
                string nlcopy = nextline;  
                stringUtils::Trim(nlcopy);  
                if( nlcopy == "" ) continue;  
  
                nextline = nextline.substr( 0, nextline.find(comm) );  
                if( nextline.find(delim) != string::npos )  
                    continue;  
  
                nlcopy = nextline;  
                stringUtils::Trim(nlcopy);  
                if( nlcopy != "" ) line += "\n";  
                line += nextline;  
                terminate = false;  
            } */

            // Store key and value  
            stringUtils::Trim(key);  
            stringUtils::Trim(line); 
            cf.m_Contents[section][key] = line;  // overwrites if key is repeated  
        } // END parse key value
        else {
            // parse section
            stringUtils::Trim(line);
            if (line.size() > 2 && line[0] == '[' && line[line.size()-1] == ']') {
                section = line.substr(1, line.size()-2);
                stringUtils::Trim(section);
            }
        } // END parse section
    }
  
    return is;  
}



