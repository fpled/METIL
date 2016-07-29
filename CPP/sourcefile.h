//
// C++ Interface: sourcefile
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SOURCEFILE_H
#define SOURCEFILE_H

#include <sys/stat.h>
#include "nstring.h"

struct TokHeader;
struct ErrorList;
struct InterpreterBehavior;
struct Thread;
struct Scope;

/**
	@author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
class SourceFile {
public:
    SourceFile();
    SourceFile(char *provenance,ErrorList *error_list,const InterpreterBehavior *interpreter_behavior);
    SourceFile(unsigned provenance,char *line,ErrorList *error_list,const InterpreterBehavior *interpreter_behavior);
    ~SourceFile();
    
    const char *sar_text();
    const TokHeader *tok_header() { return tok_data; }
    
    unsigned nb_property_names() const;
    const int *begin_property_names() const;
    void add_property_names( Scope *scope ) const;
    unsigned line_of_char_num( unsigned pos_char );
    
    char *provenance; /// absolute name of file (if it comes from a file)
    char *directory; ///
    SourceFile *prev_in_rc;
    bool dont_free; ///
    char *txt_;
private:
    void get_binary_data(ErrorList *error_list,const InterpreterBehavior *interpreter_behavior);
    void get_cache_directory(ErrorList *error_list,const InterpreterBehavior *interpreter_behavior);
    void get_dot_file_names();
    
    char *dir_of_source_file_caches; /// where cache files (.tok, .so...) are stored
    char *dot_tok_file_name; /// .tok file
    TokHeader *tok_data;
    
    struct stat stat_sar;
};

#endif
