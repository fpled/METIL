//
// C++ Implementation: globaldata
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "globaldata.h"
#include "sourcefile.h"
#include "system.h"
#include "errorlist.h"
#include "type.h"
#include "definitiondata.h"
#include <metil_sys_dir.h>
#include <string.h>

GlobalData global_data;

GlobalData::GlobalData() {
    sourcefiles = (SourceFile *)NULL;
    
    Def = NULL;
    Bool = NULL;
    Unsigned8 = NULL;
    Unsigned16 = NULL;
    Unsigned32 = NULL;
    Unsigned64 = NULL;
    Int8 = NULL;
    Int16 = NULL;
    Int32 = NULL;
    Int64 = NULL;
    Float32 = NULL;
    Float64 = NULL;
    Float96 = NULL;
    Rationnal = NULL;
    
    String = NULL;
}

GlobalData::~GlobalData() {
    for(SourceFile *sf=sourcefiles;sf;) {
        SourceFile *osf = sf;
        sf = sf->prev_in_rc;
        if ( not osf->dont_free )
            delete osf;
    }
}
                              
SourceFile *GlobalData::get_sourcefile( char *complete_name, ErrorList *error_list, InterpreterBehavior *interpreter_behavior ) {
    // look if already imported somewhere (-> rc.sourcefiles)
    SourceFile *sf = sourcefiles;
    for( ; sf; sf = sf->prev_in_rc ) {
        if ( strcmp( sf->provenance, complete_name ) == 0 ) {
            free( complete_name );
            return sf;        
        }                     
    }                         
    // no ? Let"s create a new one
    sf = new SourceFile( complete_name, error_list, interpreter_behavior );
    if ( not sf->tok_header() ) {
        delete sf;
        return NULL;
    }
    sf->prev_in_rc = sourcefiles;
    sourcefiles = sf;
    return sf;
}

SourceFile *GlobalData::get_sourcefile( const char *name_file, const char *current_directory, ErrorList *error_list, InterpreterBehavior *interpreter_behavior ) {
    // find complete file name
    char *complete_name = find_path( name_file, current_directory );
    if ( not complete_name ) {
        char *cd = NULL;
        if ( not current_directory ) cd = cur_dir();
        error_list->add( "impossible to find file '"+std::string(name_file)+"' neither in current directory ('"+
                std::string( current_directory ? current_directory : cd )+ "') nor in directories specified in METILPATH, nor in '"+std::string( MET_INSTALL_DIR )+"'." );
        if ( not current_directory ) free(cd);
        return NULL;
    }
    return get_sourcefile( complete_name, error_list, interpreter_behavior );
}
    
SourceFile *GlobalData::get_sourcefile_from_line( char *line, ErrorList *error_list, InterpreterBehavior *interpreter_behavior ) {
    SourceFile *sf = new SourceFile( 0, line, error_list, interpreter_behavior );
    if ( not sf->tok_header() ) {
        sf->txt_ = NULL;
        delete sf;
        return NULL;
    }
    sf->prev_in_rc = sourcefiles;
    sourcefiles = sf;
    return sf;
}

