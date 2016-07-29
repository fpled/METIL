//
// C++ Implementation: errorlist
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "errorlist.h"
#include "system.h"
#include <iostream>

ErrorList::ErrorList() {
    display_escape_sequences = true;
    error_stream = &std::cerr;
    cpt_use = 0;
    display_escape_sequences = term_supports_color();
}

void ErrorList::add( const Error &e ) {
    if ( error_stream )
        *error_stream << e;
    data.push_back(e);
}

void ErrorList::add( const std::string &msg, const char *pos, const char *provenance, bool due_to_not_ended_expr ) {
    Error e;
    e.warn = false;
    e.display_escape_sequences = display_escape_sequences;
    e.due_to_not_ended_expr = due_to_not_ended_expr;
    e.display_col = true;
    e.msg = msg;

    e.caller_stack.push_back( Provenance(pos, provenance) );
    
    add( e );
}

ErrorList::Provenance::Provenance( const char *pos, const char *provenance_ ) {
    provenance = provenance_;

    if ( not pos ) {
        col = 0;
        line = 0;
        length_complete_line = 0;
        return;
    }

    const char *b = pos, *e = pos;
    while ( *b and *b != '\n' and *b != '\r' )
        --b;
    ++b;
    while ( *e and *e != '\n' and *e != '\r' )
        ++e;
    if ( b > e )
        b = e;

    complete_line = b;
    length_complete_line = e - b;

    col = pos - b + 1;
    line = 1;
    for ( b = pos;*b;--b )
        line += ( *b == '\n' );
}

void display_line( std::ostream &os, const char *complete_line, unsigned length_complete_line, int col, bool display_col ) {
    if ( display_col )
        os << "  ";
    if ( length_complete_line < 64 ) {
        os.write(complete_line,length_complete_line);
        if ( display_col ) {
            os << "\n";
            for ( int i = 1;i < 2 + col;++i )
                os << " ";
        }
    } else {
        if ( length_complete_line - col < 64 ) { // only the ending
            os << "...";
            os.write( complete_line + length_complete_line - 64, 64 );
            if ( display_col ) {
                os << "\n";
                for ( unsigned i = 1;i < 2 + 64 + 3 - length_complete_line + col;++i )
                    os << " ";
            }
        } else if ( col < 64 ) { // only the beginning
            os.write( complete_line, 64 );
            os << "...";
            if ( display_col ) {
                os << "\n";
                for ( int i = 1;i < 2 + col;++i )
                    os << " ";
            }
        } else { // middle
            os << "...";
            os.write( complete_line + col - 32, 61 );
            os << "...";
            if ( display_col ) {
                os << "\n";
                for ( int i = 1;i < 2 + 32 + 3;++i )
                    os << " ";
            }
        }
    }
    if ( display_col )
        os << "^";
}

std::ostream &operator<<( std::ostream &os, const ErrorList::Error &error ) {
    // last item in caller stack
    if ( error.caller_stack.size() ) {
        if ( error.display_escape_sequences )
            os << "\033[1m";
        const ErrorList::Provenance &po = error.caller_stack.front();
        if ( po.provenance )
            os << po.provenance << ":";
        if ( po.line )
            os << po.line << ":" << po.col << ":";
        os << "error: " << error.msg << ( error.display_col ? "\n" : "" );
        if ( po.length_complete_line )
            display_line( os, po.complete_line, po.length_complete_line, po.col, error.display_col );
        if ( error.display_escape_sequences )
            os << "\033[0m";
        os << "\n";
    }
    else {
        if ( error.display_escape_sequences )
            os << "\033[1m";
        os << "error: " << error.msg << "\n";
        if ( error.display_escape_sequences )
            os << "\033[0m";
    }
    
    // caller_stack
    for(unsigned num_prov=1;num_prov<error.caller_stack.size();++num_prov) {
        const ErrorList::Provenance & po = error.caller_stack[ num_prov ];
        if ( po.provenance )
            os << po.provenance << ":";
        if ( po.line ) {
            os << po.line << ":" << po.col;
            //while ( num_prov>0 and error.caller_stack[ num_prov-1 ].line==po.line ) os << "," << error.caller_stack[ --num_prov ].col;
            os << ":";
        }
        os << "instantiated from: ";
        display_line( os, po.complete_line, po.length_complete_line, po.col, false );
        os << "\n";
    }

    // possibilities
    if ( error.possibilities.size() ) {
        os << "Possibilities are:" << std::endl;
        for ( unsigned i = 0;i < error.possibilities.size();++i ) {
            for(unsigned j=0;;++j) {
                if ( j==i ) {
                    const ErrorList::Provenance & po = error.possibilities[ i ];
                    if ( po.provenance and po.line )
                        os << "  " << po.provenance << ":" << po.line << ":" << po.col << ":";
                    else
                        os << "  (in primitive functions or classes)";
                    display_line( os, po.complete_line, po.length_complete_line, po.col, false );
                    os << "\n";
                    break;
                }
                if ( error.possibilities[i].provenance==error.possibilities[j].provenance and 
                     error.possibilities[i].line==error.possibilities[j].line and 
                     error.possibilities[i].col==error.possibilities[j].col )
                    break;
            }
        }
    }

    return os;
}

bool ErrorList::line_not_finished() const {
    if ( data.size() == 0 )
        return false;
    for(unsigned i=0;i<data.size();++i)
        if ( data[i].due_to_not_ended_expr == false )
            return false;
    return true;
}
        
bool ErrorList::Provenance::operator==(const Provenance &p) const {
    return (
            provenance == p.provenance and
            complete_line == p.complete_line and
            length_complete_line == p.length_complete_line and
            line == p.line and
            col == p.col
           );
}

