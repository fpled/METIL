//
// C++ Implementation: lexem
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "lexem.h"
#include <fstream>

Lexem::Lexem() {
    children[0] = NULL;
    children[1] = NULL;
    parent = NULL;
    next = NULL;
    prev = NULL;
    sibling = NULL;
    def = NULL;
    number = NULL;
}


Lexem::~Lexem() {
}


const Lexem *leftmost_child(const Lexem *t) {
    if ( t->type==Lexem::APPLY or t->type==Lexem::SELECT or t->type==Lexem::CHANGE_BEHAVIOR )
        return leftmost_child( t->children[0] ); // a(15)
    if ( t->type > 0 )
        if ( ( t->num & 1 )/*need_left_arg*/ or ( ( t->num & 4 )/*MayNeedlarg*/ and t->children[0] ) )
            return leftmost_child( t->children[0] ); // a', a-b
    //     if ( t->type==Lexem::PAREN or t->children[0]==NULL or t->children[1]==NULL )
    return t;
    //     return leftmost_child( t->children[0] );
}

const Lexem *rightmost_child(const Lexem *t) {
    while ( t->next )
        t = t->next;
    
    if ( t->type==Lexem::PAREN or t->type==Lexem::APPLY or t->type==Lexem::SELECT or t->type==Lexem::CHANGE_BEHAVIOR )
        return t;
    
    if ( t->children[1] )
        return rightmost_child( t->children[1] );
    if ( t->children[0] and not ( t->type > 0 and (t->num & 1/*need_left_arg*/) ) ) // operators with 1 child are in left of there operand
        return rightmost_child( t->children[0] );
    return t;
}


void display_graph_rec( std::ostream &os, const Lexem *t, unsigned level, unsigned &max_level ) {
    unsigned j;
    max_level = std::max( max_level, level );
    const Lexem *old_t = t;
    // nodes
    for(;t;t=t->next) {
        os << "{ rank = same; " << level << " node" << t << " [label=\"";
        for(unsigned i=0;i<t->si;++i) {
            if ( t->s[i]=='\n' )
                os << "\\n";
            else if ( t->s[i]=='"' )
                os << "\\\"";
            else
                os << t->s[i];
        }
        os << '(' << t->type << ')' << "\"] }\n";
        
        // children
        for(j=0;j<2;++j) {
            if ( t->children[j] ) {
                display_graph_rec( os, t->children[j], level+1, max_level );
                os << "  node" << t << " -> node" << t->children[j] << ";\n";
            }
        }
        // ascending edge
        if ( t->parent )
            os << "  node" << t << " -> node" << t->parent << " [color=red];\n";
    }
    // edges
    t = old_t;
    for(;t;t=t->next) {
        if ( t->next )
            os << "  node" << t << " -> node" << t->next << " [color=green];\n";
        if ( t->prev ) //
            os << "  node" << t << " -> node" << t->prev << " [color=yellow];\n";
    }
}

void display_graph(const Lexem *t,const char *file_name) {
    std::ofstream f(file_name);
    f << "digraph popoterie {";
    unsigned max_level = 1;
    display_graph_rec( f, t, 1, max_level );

    for(unsigned i=1;i<=max_level;++i)
        f << "  " << i << " [ shape=plaintext ];\n  ";
    for(unsigned i=1;i<=max_level;++i)
        f << i << ( i<max_level ? " -> " : ";" );
    
    f << "}";
    f.close();
    
    system( ("dot -Tps "+std::string(file_name)+" > "+std::string(file_name)+".eps && gv "+std::string(file_name)+".eps").c_str() );
}

int offset_to_doc_of( const Lexem *l, const char *sar_txt ) {
    // look at the end of the line
    for( const char *b = l->s; *b and *b!='\n'; ++b ) 
        if ( *b=='#' )
            return b - sar_txt;
    // else, go to the beginning of the previous ones while there are #s
    //     const char *b = l->s;
    //     bool double_comment = false;
    //     for( ; *b and *b!='\n'; --b )
    //         if ( *b=='#' )
    //             double_comment = true;
    //     for( bool cont; cont; ) {
    //         cont = false;
    //         for( ; *b and *b!='\n'; --b )
    //             cont |= ( *b == '#' );
    //     }
    
    return -1;
}
