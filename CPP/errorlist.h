//
// C++ Interface: errorlist
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <vector>
#include <string>

#include "ref.h"

/**
    @author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
class ErrorList {
public:
    ErrorList();
    
    struct Provenance {
        Provenance( const char *pos, const char *provenance );
        bool operator==(const Provenance &p) const;
        const char *provenance; /// name of file
        const char *complete_line;
        unsigned length_complete_line;
        int line;
        int col;
    };
    
    struct Error {
        bool due_to_not_ended_expr, display_col, display_escape_sequences, warn;
        std::string msg;
        std::vector<Provenance> caller_stack; /// "copy" of caller stack
        std::vector<Provenance> possibilities; /// if ambiguous overload, list of possible functions
    };
    
    void add( const Error &e );
    void add( const std::string &msg, const char *pos=NULL, const char *provenance=NULL, bool due_to_not_ended_expr=false );
    bool line_not_finished() const; /// return true if errors are due to not ended expression

    std::vector<Error> data; /// errors

    bool display_escape_sequences;
    std::ostream *error_stream;
    unsigned cpt_use;
};

std::ostream &operator<<( std::ostream &os, const ErrorList::Error &error );

#endif
