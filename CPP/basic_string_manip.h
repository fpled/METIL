#ifndef BASIC_STRING_MANIP_H
#define BASIC_STRING_MANIP_H

#include <string>
#include <vector>
#include <stdlib.h>
#include <string.h>

inline std::string capitalize_first(std::string a) {
    char n = a[0];
    if ( n != '_' and (n<'0' or n>'9') ) n += 'A' - 'a';
    return n + std::string(a.begin()+1,a.end());
}

inline std::string capitalize(std::string a) {
    for(unsigned i=0;i<a.size();++i)
        if ( a[i] != '_' and (a[i]<'0' or a[i]>'9') ) a[i] += 'A' - 'a';
    return a;
}

inline std::vector<std::string> split(const std::string &al) {
    std::vector<std::string> res;
    for(unsigned j=0,oj=0;;++j) {
        if ( j == al.size() or al[j]==' ' ) {
            res.push_back( std::string(al.begin()+oj,al.begin()+j) );
            if ( j == al.size() )
                break;
            oj = ++j;
        }
    }
    return res;
}

inline char *strdupp0( const char *p, unsigned si ) {
    char *res = (char *)malloc( si + 1 );
    memcpy( res, p, si );
    res[si] = 0;
    return res;
}

#endif // BASIC_STRING_MANIP_H
