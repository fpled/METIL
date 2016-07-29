//
// C++ Interface: globaldata
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef GLOBALDATA_H
#define GLOBALDATA_H

#include <map>
#include "typeandsize.h"

struct SourceFile;
struct ErrorList;
struct InterpreterBehavior;

/**
	@author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct GlobalData {
    GlobalData();
    ~GlobalData();
    
    SourceFile *get_sourcefile( char *complete_name, ErrorList *error_list, InterpreterBehavior *interpreter_behavior );
    SourceFile *get_sourcefile( const char *name_file, const char *current_directory, ErrorList *error_list, InterpreterBehavior *interpreter_behavior );
    SourceFile *get_sourcefile_from_line( char *line, ErrorList *error_list, InterpreterBehavior *interpreter_behavior ); // line[0] must be 0
    
    #include "generated_globaldata_attributes.h"
    
    SourceFile *sourcefiles;
    Type *String;
    
    std::map<TypeAndSize,Type> static_vec_type_cache;
};

extern GlobalData global_data;

#endif
