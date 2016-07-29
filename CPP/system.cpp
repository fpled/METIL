//
// C++ Implementation: system
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "system.h"
#include "basic_string_manip.h"
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include "typeconfig.h"
#include <sys/time.h>
#include <sys/utsname.h>
#include <metil_sys_dir.h>
#include <stdio.h>

char *cur_dir() {
    for( unsigned st = 1024; ; st *= 2 ) {
        char *res = (char *)malloc( st );
        if ( getcwd( res, st ) )
            return res;
        free( res );
    }
    return NULL;
}

char *find_path( const char *nf, const char *current_directory ) {
    if ( not current_directory ) {
        char *cd = cur_dir();
        char *res = find_path( nf, cd );
        free( cd );
        return res;
    }
    
    unsigned lnf = strlen( nf );
    
    // if absolute directory
    if ( nf[0] == '/' )
        return strdupp0( nf, lnf );
    
    //
    struct stat stat_file;
    
    // look in current_directory
    unsigned lc = strlen( current_directory );
    char *trial = (char *)malloc( lc + 1 + lnf + 1 );
    memcpy( trial, current_directory, lc );
    trial[lc] = '/';
    memcpy( trial+lc+1, nf, lnf+1 );
    if ( stat( trial, &stat_file ) == 0 )
        return trial;
    free(trial);
    
    // look in pwd
    char *cd = cur_dir();
    lc = strlen( cd );
    trial = (char *)malloc( lc + 1 + lnf + 1 );
    memcpy( trial, cd, lc );
    trial[lc] = '/';
    memcpy( trial+lc+1, nf, lnf+1 );
    if ( stat( trial, &stat_file ) == 0 )
        return trial;
    free(trial);

    // look in METILPATH
    const char *METILPATH = getenv("METILPATH");
    if ( METILPATH ) {
        for(const char *p = METILPATH, *op = METILPATH; ; ++p ) {
            if ( *p==0 || *p==':' ) {
                if ( op!=p ) {
                    char *trial = (char *)malloc( p-op + 1 + lnf + 1 );
                    memcpy( trial, op, p-op );
                    trial[p-op] = '/';
                    memcpy( trial+(p-op)+1, nf, lnf+1 );
                    if ( stat( trial, &stat_file ) == 0 )
                        return trial;
                    free(trial);
                }
                if ( *p==0 )
                    break;
                op = p+1;
            }
        }
    }
    
    // else, look in MET_INSTALL_DIR
    cd = MET_INSTALL_DIR;
    lc = strlen( cd );
    trial = (char *)malloc( lc + 1 + lnf + 1 );
    memcpy( trial, cd, lc );
    trial[lc] = '/';
    memcpy( trial+lc+1, nf, lnf+1 );
    if ( stat( trial, &stat_file ) == 0 )
        return trial;
    free(trial);
    
    return NULL;
}

// int compilation_to_so(const char *cpp, const char *so, bool disp_command ) {
//     std::ostringstream command;
//     const char *METILCXX = getenv("METILCXX");
//     if ( not METILCXX ) METILCXX = "g++";
//     const char *METILCXXFLAGS = getenv("METILCXXFLAGS");
//     if ( not METILCXXFLAGS ) METILCXXFLAGS = "-fPIC -shared -g3";
// 
//     // g++
//     command << METILCXX << " ";
//     // -I...
//     const char *METILPATH = getenv("METILPATH");
//     for(const char *p = METILPATH, *op = METILPATH; ; ++p ) {
//         if ( *p==0 || *p==':' ) {
//             if ( op!=p ) {
//                 command << "-I";
//                 command.write( op, p-op );
//                 command << " ";
//             }
//             if ( *p==0 ) break;
//             op = p+1;
//         }
//     }
//     // xx.cpp -o xx.so
//     command << METILCXXFLAGS << " -o " << so << " " << cpp;
//     if ( disp_command )
//         std::cerr << command.str() << std::endl;
//     return system( command.str().c_str() );
// }

bool term_supports_color() {
    const char *c_term = getenv("TERM");
    if ( not c_term )
        return false;
    std::string term = c_term;
    return term != "dumb";
}

std::string canonicalize_filename( const std::string &f ) {
    std::string res; res.reserve( f.size() );
    for(unsigned i=0;i<f.size();++i) {
        if ( f[i]=='.' and i+1<f.size() and ( f[i+1]=='/' or f[i+1]=='\\' ) ) {
            // ../toto
            if ( i and f[i-1]=='.' ) {
                for( unsigned cpt = 2, j = res.size()-1; j >= 0; --j ) {
                    if ( res[j]=='/' or res[j]=='\\' ) {
                        if ( --cpt == 0 ) {
                            res = std::string( res.begin(), res.begin() + j );
                            break;
                        }
                    }
                }
            } else
                ++i;
        } else if ( f[i] == '\\' )
            res += '/';
        else
            res += f[i];
    }
    return res;
}

std::string absolute_filename( const std::string &f ) {
    if ( f[0]=='/' or f[0]=='\\' )
        return canonicalize_filename( f );
    char *cd = cur_dir();
    std::string res = canonicalize_filename( cd + ( "/" + f ) );
    free( cd );
    return res;
}

Int64 last_modification_time_or_zero_of_file_named( const std::string &n ) {
    struct stat stat_file;
    if ( stat( n.c_str(), &stat_file ) )
        return 0;
    return stat_file.st_mtime;
}
            
void skip_spaces_and_look_up_for_an_include( std::vector<std::string> &dep, char * &s ) {
    while ( *s == '\n' or *s == '\r' or *s == '\t' or *s == ' ' ) ++s;
    if ( *s != '#' ) return;
    if ( strncmp( s, "#include", 8 ) == 0 ) {
        s += 8;
        while ( *s == '\t' or *s == ' ' ) ++s;
        if ( *s != '<' and *s != '"' ) return;
        const char *os = ++s;
        while ( *s != '>' and *s != '"' ) {
            if ( *s == '\n' ) return;
            ++s;
        }
        dep.push_back( std::string( os, (const char *)s ) );
    }
}

void get_includes_of_cpp_named( std::vector<std::string> &dep, const char *fs, unsigned fsi, bool &has_a_corresponding_cpp, bool &has_a_corresponding_generated_cpp ) {
    char *filename = strdupp0( fs, fsi );
    //
    struct stat stat_cpp;
    if ( stat( filename, &stat_cpp ) ) {
        free( filename );
        return;
    }
    
    //
    FILE *f = fopen( filename, "r" );
    free( filename );
    if ( not f )
        return;
    
    //
    unsigned si = stat_cpp.st_size;
    char *txt = (char *)malloc( si+1 );
    fread( txt, 1, si, f );
    txt[si] = 0;    
    fclose( f );
    
    //
    char *s = txt;
    skip_spaces_and_look_up_for_an_include( dep, s );
    for( ; *s; ++s ) {
        if ( *s == '\n' ) {
            skip_spaces_and_look_up_for_an_include( dep, s );
            if ( not *s )
                break;
        }
        if ( strncmp( s, "**has_a_corresponding_cpp**", 25 ) == 0 )
            has_a_corresponding_cpp = true;
        else if ( strncmp( s, "**has_a_corresponding_generated_cpp**", 37 ) == 0 )
            has_a_corresponding_generated_cpp = true;
    }
    
    free( txt );
}

double time_of_day_in_sec() {
    struct timeval st;
    struct timezone toto;
    gettimeofday( &st, &toto );
    return st.tv_sec + st.tv_usec / 1e6;
}

std::string directory_of( const char *s, int si ) {
    for(int i=si-1;i>=0;--i)
        if ( s[i]=='/' or s[i]=='\\' )
            return std::string( s, s + i );
    return ".";
}

std::string get_os_type() {
    struct utsname utsn;
    uname(&utsn);
    return utsn.sysname;
}

std::string get_cpu_type() {
    struct utsname utsn;
    uname(&utsn);
    return utsn.machine;
}

