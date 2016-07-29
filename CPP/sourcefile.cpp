//
// C++ Implementation: sourcefile
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "sourcefile.h"

#include "system.h"
#include "errorlist.h"
#include "make_dot_tok_data.h"
#include "interpreterbehavior.h"
#include "globaldata.h"
#include "scope.h"
#include <sys/utsname.h>
#include <sstream>
#include <stdio.h>

SourceFile::SourceFile(char *provenance_,ErrorList *error_list,const InterpreterBehavior *interpreter_behavior) : provenance(provenance_) {
    dont_free = false;
    txt_ = NULL;
    tok_data = NULL;
    dir_of_source_file_caches = NULL;
    dot_tok_file_name = NULL;
    
    directory = (char *)malloc( strlen( provenance_ ) + 1 );
    memcpy( directory, provenance_, strlen( provenance_ ) + 1 );
    for(int i=strlen( provenance_ );i>=0;--i) if ( directory[i]=='/' or directory[i]=='\\' ) { directory[i]=0; break; }
    
    if ( stat( provenance, &stat_sar ) )
        return;
    get_binary_data( error_list, interpreter_behavior );
    // if ( tok_data ) std::cout << *tok_data << std::endl;
}
    
SourceFile::SourceFile(unsigned provenance_,char *line,ErrorList *error_list,const InterpreterBehavior *interpreter_behavior) {
    dont_free = false;
    txt_ = line;
    tok_data = NULL;
    dir_of_source_file_caches = NULL;
    dot_tok_file_name = NULL;
    
    directory = NULL;
    
    std::ostringstream ss; ss << "interpreted line nb " << provenance_;
    provenance = strdup( ss.str().c_str() );
    
    TokData td = make_tok_data( txt_, provenance, error_list );
    tok_data = ( TokHeader * )td.data;
}

SourceFile::SourceFile() {
}

SourceFile::~SourceFile() {
    if ( dont_free )
        return;
    
    if ( directory ) free( directory );
    if ( provenance ) free( provenance );
    if ( txt_ ) free( txt_ );
    if ( tok_data ) free( tok_data );
    if ( dir_of_source_file_caches ) free( dir_of_source_file_caches );
    if ( dot_tok_file_name ) free( dot_tok_file_name );
}

unsigned SourceFile::nb_property_names() const {
    return tok_data->nb_property_names;
}

const int *SourceFile::begin_property_names() const {
    return tok_data->begin_property_names();
}
    
void SourceFile::add_property_names( Scope *scope ) const {
    for(unsigned i=0;i<nb_property_names();++i)
        scope->add_property( begin_property_names()[i] );
}

void SourceFile::get_binary_data(ErrorList *error_list,const InterpreterBehavior *interpreter_behavior) {
    // cache directory
    get_cache_directory(error_list,interpreter_behavior);
    get_dot_file_names();

    // look for an up to date tok.xx file
    struct stat stat_tok;
    if ( interpreter_behavior->remake_all==false and dot_tok_file_name and stat( dot_tok_file_name, &stat_tok )==0 and stat_tok.st_mtime >= stat_sar.st_mtime ) {
        tok_data = (TokHeader *)malloc( stat_tok.st_size );
        FILE *f = fopen( dot_tok_file_name, "r" );
        if ( f==NULL or fread( (void *)tok_data, 1, stat_tok.st_size, f ) != (size_t)stat_tok.st_size ) {
            if ( interpreter_behavior->verbose_level >= 1 )
                std::cerr << "File '" << dot_tok_file_name << "' exists but can't be opened for reading." << std::endl;
            free( tok_data );
            tok_data = NULL;
        }
        else {
            if ( strncmp( tok_data->md5_ctrl_sum, USUAL_STRINGS_MD5, 32 ) == 0 )
                tok_data->rellocate();
            else {
                std::cout << "!= md5" << std::endl;
                tok_data = NULL;
            }
        }
        
        if ( f )
            fclose( f );
    }
    
    // not tok.xx file ? Let's create and save it.
    if ( not tok_data ) {
        if ( interpreter_behavior->verbose_level >= 2 )
            std::cout << "Tokenization of " << provenance << std::endl;
        TokData td = make_tok_data( sar_text(), provenance, error_list );
        if ( td.data ) {
            FILE *f = fopen( dot_tok_file_name, "w" );
            if ( f ) {
                fwrite( (void *)td.data, 1, td.size, f );
                fclose( f );
            }
        }
        tok_data = ( TokHeader * )td.data;
    }
}

void SourceFile::get_cache_directory(ErrorList *error_list,const InterpreterBehavior *interpreter_behavior) {
    char *res;
    int sp = strlen(provenance);
    if ( provenance[0]=='/' or provenance[0]=='\\' ) {
        // /tata/toto.sar -> /tata/.toto.sar.cache/ -> 8+1 characters more
        res = (char *)malloc( sp+9 );
        int d = sp - 1; // find last /
        while ( provenance[d] != '/' and provenance[d] != '\\' ) --d;
        ++d;
        // 
        memcpy( res, provenance, d ); // '/tata/'
        res[ d ] = '.'; // '.'
        memcpy( res + d + 1, provenance + d, sp - d ); // toto.sar
        memcpy( res + sp + 1, ".cache/", 8 ); // .cache
    }
    else {
        // relative path
        char *tmp = cur_dir(); // get_current_dir_name();
        int st = strlen(tmp);
        res = (char *)malloc( sp+st+10 );
        for( int d = sp - 1; ; --d ) { // look for the last '/' in provenance
            if ( d < 0 ) {
                memcpy( res, tmp, st );                    // '/cwd'
                memcpy( res + st, "/.", 2 );               // '/.'
                memcpy( res + st + 2, provenance, sp );    // 'toto.sar'
                memcpy( res + st + 2 + sp, ".cache/", 8 ); // '.cache'
                break;
            }
            if ( provenance[d] == '/' or provenance[d] == '\\' ) {
                ++d;
                memcpy( res, tmp, st );                    // '/cwd'
                res[st] = '/';                             // '/'
                memcpy( res + st + 1, provenance, d + 1 ); // 'tata/'
                res[st+d+1] = '.';                         // '.'
                memcpy( res + st + d + 2, provenance + d, sp - d ); // 'toto.sar'
                memcpy( res + st + sp + 2, ".cache/", 8 ); // 'toto.sar'
                break;
            }
        }
        free(tmp);
    }
    dir_of_source_file_caches = res;

    // check for directory existence. Else, create it.
    struct stat stat_dir_of_source_file_caches;
    if ( stat( dir_of_source_file_caches, &stat_dir_of_source_file_caches ) ) { // if there's no directory, we create one
        if ( mkdir( dir_of_source_file_caches, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH ) ) {
            if ( interpreter_behavior->verbose_level >= 1 )
                std::cerr << "Unable to create directory '" << dir_of_source_file_caches << "'." << std::endl;
            dir_of_source_file_caches = NULL;
        }
    }
    else if ( ( stat_dir_of_source_file_caches.st_mode & S_IFDIR ) == 0 ) { // assert that it is a directory
        if ( interpreter_behavior->verbose_level >= 1 )
            std::cerr << "File '" << dir_of_source_file_caches << "' already exists and is not a directory." << std::endl;
        dir_of_source_file_caches = NULL;
    }
}

void SourceFile::get_dot_file_names() {
    if ( not dir_of_source_file_caches ) { // error occured
        dot_tok_file_name = NULL;
        return;
    }    
    struct utsname name;
    uname(&name);
    unsigned sc = strlen(dir_of_source_file_caches);
    unsigned su = strlen(name.machine);
    
    // xx.tok
    dot_tok_file_name = (char *)malloc( sc + su + 5 ); // 'dir/' + 'uname' + '.tok'
    memcpy( dot_tok_file_name, dir_of_source_file_caches, sc );
    memcpy( dot_tok_file_name + sc, name.machine, su );
    memcpy( dot_tok_file_name + sc + su, ".tok", 5 );
}

const char *SourceFile::sar_text() {
    if ( txt_ )
        return txt_;
    
    // open
    FILE *f = fopen( provenance, "r" );
    if ( ! f ) {
        std::cerr << "Unable to open file '" << provenance << "'." << std::endl;
        return NULL;
    }

    // read text
    unsigned si = stat_sar.st_size;
    txt_ = (char *)malloc( si+2 );
    fread( txt_+1, 1, si, f );
    txt_[0] = 0;
    txt_[si+1] = 0;    
    fclose( f );
    
    return txt_;
}

unsigned SourceFile::line_of_char_num( unsigned pos_char ) {
    const char *pos = sar_text() + pos_char;

    const char *b = pos, *e = pos;
    while ( *b and *b != '\n' and *b != '\r' )
        --b;
    ++b;
    while ( *e and *e != '\n' and *e != '\r' )
        ++e;
    if ( b > e )
        b = e;

    unsigned line = 1;
    for ( b = pos; *b; --b )
        line += ( *b == '\n' );
    return line;
}




