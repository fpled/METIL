//
// C++ Implementation: lexer
//
// Description: 
//
//
// Author: LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "errorlist.h"
#include "operators_hash.h"
#include "typechar.h"
#include "lexer.h"
#include "tok.h"
#include <assert.h>

inline bool is_num(char c) { return type_char(c) == TYPE_CHAR_number; }
inline bool is_letter(char c) { return type_char(c) == TYPE_CHAR_letter; }
inline bool is_operator(char c) { return type_char(c) == TYPE_CHAR_operator; }

Lexer::Lexer(const char *txt, const char *provenance_, ErrorList *error_list_ ) : provenance(provenance_), error_list(error_list_) {
    if ( not txt ) {
        first_tok.next = NULL;
        return;
    }
    s = txt + 1;
    //
    for(unsigned i=0;i<NB_OPERATORS+16;++i) {
        first_of_type[i] = NULL;
        last_of_type[i] = NULL;
    }
    first_tok.type = Lexem::CR;
    unsigned oe = error_list->data.size();
    
    // let's go
    make_flat_token_list();                     if (oe!=error_list->data.size()) { first_tok.next=NULL; return; }
    set_next_prev_fields();                     if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    parenthesis_hierarchisation();              if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    make_unit_hierarchy();                      if (oe!=error_list->data.size()) { first_tok.next=NULL; return; }
    assemble___string_assembly__();             if (oe!=error_list->data.size()) { first_tok.next=NULL; return; }
    assemble___calc_name__();                   if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    assemble_par___getattr__();                 if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    remove_cr();                                if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    assemble_op();                              if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    parse_definitions();                        if (oe!=error_list->data.size()) { first_tok.next=NULL; return; } 
    add_child_to_info( STRING___info___NUM );   if (oe!=error_list->data.size()) { first_tok.next=NULL; return; }
    add_child_to_info( STRING___infon___NUM );  if (oe!=error_list->data.size()) { first_tok.next=NULL; return; }
    //display_graph( root() );
}

Lexer::~Lexer() {
}

Lexem *Lexer::app_tok( int type, const char *s, unsigned si, int num ) {
    Lexem *t = tokens.new_elem();
    t->type = type;
    t->s = s;
    t->si = si;
    t->num = num;
    if ( last_of_type[ type+16 ] == NULL ) {
        first_of_type[ type+16 ] = t;
        last_of_type [ type+16 ] = t;
    }
    else {
        last_of_type [ type+16 ]->sibling = t;
        last_of_type [ type+16 ] = t;
    }
    return t;
}

void Lexer::read_dollar( const char *old_str, unsigned nb_spaces ) {
    // string before
    app_tok( Lexem::STRING, old_str, s-old_str, nb_spaces );
    
    // operator __string_assembly__
    app_tok( STRING___string_assembly___NUM, s, 1, behavior_of_operator( STRING___string_assembly___NUM ) );
    
    // tokens
    const char *e = ++s;
    if ( e[0]=='(' or e[0]=='[' or e[0]=='{' ) { // $(...)
        unsigned cpt = 1;
        while( (++e)[0] ) {
            if ( e[0]==')' or e[0]==']' or e[0]=='}' ) {
                if ( --cpt == 0 ) {
                    ++e;
                    break;
                }
            }
            else if ( e[0]=='(' or e[0]=='[' or e[0]=='{' )
                ++cpt;
        }
        if ( cpt ) {
            error_list->add( "laking ')', ']' or '}'", s, provenance );
            return;
        }
        //std::cout << std::string( s, e ) << std::endl;
    }
    else { // $toto
        while ( (++e)[0] and ( type_char(e[0])==TYPE_CHAR_letter or type_char(e[0])==TYPE_CHAR_number ) ) ;
    }
    read_s<true>( e );

    // operator __string_assembly__
    app_tok( STRING___string_assembly___NUM, s, 1, behavior_of_operator( STRING___string_assembly___NUM ) );
}

template<bool stop_condition> void Lexer::read_s(const char *limit) {
    while ( stop_condition==false or s < limit ) {
        switch ( type_char(*s) ) {
            case TYPE_CHAR_space:
                ++s;
                break;
            case TYPE_CHAR_letter: {
                const char *old_str = s;
                while ( is_num(*(++s)) or is_letter(*s) );
                unsigned si = s-old_str;
                int num_op = num_operator_le( old_str, si ); // look for an operator
                app_tok( num_op, old_str, si, behavior_of_operator( num_op ) );
                break;
            }
            case TYPE_CHAR_operator: {
                const char *old_str = s;
                while ( is_operator(*(++s)) );
                while ( s != old_str ) {
                    for( unsigned i=s-old_str; ; --i ) {
                        if ( i==0 ) {
                            error_list->add("Unknown operator (or operator set) '"+std::string(old_str,s)+"'.", old_str, provenance );
                            return;
                        }
                        int num_op = num_operator_op( old_str, i ); // look for an operator
                        if ( num_op >= 0 ) {
                            app_tok( num_op, old_str, i, behavior_of_operator( num_op ) );
                            old_str += i;
                            /// f <<<< ...
                            if ( num_op == STRING_shift_left_long_str_NUM ) {
                                // get nb_spaces from current line
                                unsigned nb_spaces = 0;
                                const char *b = s;
                                while ( *(--b) and *b!='\n' and *b!='\r' );
                                while ( *(++b)==' ' ) nb_spaces++;
                                
                                // skip spaces after <<<<
                                while ( *old_str == ' ' ) ++old_str;
                                if ( *old_str != '\n' and *old_str != '\r' ) {
                                    error_list->add("After a '<<<<' and in the same line, only spaces and carriage returns are allowed.", old_str, provenance );
                                    return;
                                }
                                old_str += ( *old_str == '\r' );
                                old_str += ( *old_str == '\n' );
                                
                                //
                                s = old_str;
                                while ( true ) {
                                    unsigned new_nb_spaces = 0;
                                    while ( *s==' ' ) { ++s; ++new_nb_spaces; }
//                                     std::cout << *s << " " << new_nb_spaces << std::endl;
                                    if ( new_nb_spaces < nb_spaces + 4 ) {
                                        while ( *s and *s!='\n' and *s!='\r' ) --s; // return to beginning of the line
                                        break;
                                    }
                                    // else
                                    while ( *s and *s!='\n' and *s!='\r' ) ++s; // next line
                                    s += ( *s == '\r' );
                                    s += ( *s == '\n' );
                                }
                                
                                // $...
                                const char *new_s = s;
                                s = old_str;
                                while ( s < new_s ) {
                                    if ( s[0] == '$' and s[-1] != '\\' and s[-1] != '$' and ( type_char(s[1])==TYPE_CHAR_letter or type_char(s[1])==TYPE_CHAR_parenthesis ) ) {
                                        read_dollar( old_str, nb_spaces + 4 );
                                        old_str = s - 1;
                                    }
                                    else
                                        s++;
                                }
                                
                                // app_tok
                                app_tok( Lexem::STRING, old_str, s-old_str, nb_spaces + 4 );
                                
                                // out loop
                                old_str = s;
                                
                            }
                            break;
                        }
                    }
                }
                break;
            }
            case TYPE_CHAR_cr: {
                unsigned num = 0;
                while ( *(++s) ) {
                    if ( s[0]=='\t' ) {
                        error_list->add( "Tabulations are not allowed at the beginning of a line. Please use spaces only.", s, provenance );
                        num += 3;
                    }
                    else if ( s[0]!=' ' ) {
                        app_tok( Lexem::CR, s, 1, num );
                        break;
                    }
                    ++num;
                }
                break;
            }
            case TYPE_CHAR_ccode: {
                const char *old_str = ++s;
                while ( *s and (*s != '`' or (s[-1] == '\\')) ) ++s;
                app_tok( Lexem::CCODE, old_str, s - old_str, -1 );
                if ( *s ) ++s;
                break;
            }
            case TYPE_CHAR_string: {
                const char *old_str = s;
                while ( *(++s) and (*s != '"' or (s[-1] == '\\' and s[-2] != '\\')) ) {
                    if ( s[0] == '$' and s[-1] != '$' and s[-1] != '\\' and ( type_char(s[1])==TYPE_CHAR_letter or type_char(s[1])==TYPE_CHAR_parenthesis ) ) {
                        read_dollar( old_str + 1 );
                        old_str = --s;
                    }
                }
                //
                if ( not *s )
                    error_list->add("Laking closing '\"'.", old_str, provenance, 1 );
                else {
                    app_tok( Lexem::STRING, old_str+1, s-old_str-1, 0 );
                    ++s;
                }
                break;
            }
            case TYPE_CHAR_number: {
                const char *old_str = s;

                LexemNumber *n = numbers.new_elem(); n->v.init(0);
                if ( s[0]=='0' and (s[1]=='x' or s[1]=='X') ) { // 0xFFE -> hexadecimal
                    s += 2;
                    for( ; ; ++s ) { // read value
                        if ( is_num(s[0]) ) { n->v *= 16; n->v += unsigned( s[0] - '0' ); }
                        else if ( s[0]>='a' and s[0]<='f' ) { n->v *= 16; n->v += 10 + unsigned( s[0] - 'a' ); } 
                        else if ( s[0]>='A' and s[0]<='F' ) { n->v *= 16; n->v += 10 + unsigned( s[0] - 'A' ); }
                        else
                            break;
                    }
                    n->expo = 0;
                    n->recquired_prec = 0;
                    n->attributes = 0;
                    app_tok( Lexem::NUMBER, old_str, s-old_str, 0 )->number = n;
                }
                else { // decimal number
                    n->v = unsigned( s[0] - '0' );
                    while ( is_num(*(++s)) ) { n->v *= 10; n->v += s[0] - '0'; } // first numbers, before . and [eE]
                    n->expo = 0;
                    if ( s[0]=='.' and is_num(s[1]) ) {
                        while ( is_num(*(++s)) ) { n->v *= 10; n->v += s[0] - '0'; --n->expo; } // numbers after .
                    }
                    if ( s[0]=='e' or s[0]=='E' ) { // numbers after [eE]
                        n->has_e = true;
                        int sgn = 1;
                        if ( s[1]=='-' ) { sgn = -1; ++s; }
                        else if ( s[1]=='+' ) { ++s; }
                        int ae = 0;
                        while ( is_num(*(++s)) ) { ae *= 10; ae += s[0] - '0'; }
                        n->expo += sgn * ae;
                    }
                    //
                    n->attributes = 0;
                    // completion of attributes (unsigned, float, ...)
                    for( ; ; ++s ) {
                        if ( s[0] == 'u' )
                            n->attributes |= Tok::Number::UNSIGNED;
                        else if ( s[0] == 'i' )
                            n->attributes |= Tok::Number::IMAGINARY;
                        else if ( type_char(s[0])==TYPE_CHAR_letter )
                            error_list->add( "Character '"+std::string(1,*s)+"' is not a known number attribute ('u' for unsigned, 'i' for imaginary ).", s, provenance );
                        else
                            break;
                    }
                    app_tok( Lexem::NUMBER, old_str, s-old_str, 0 )->number = n;
                }

                break;
            }
            case TYPE_CHAR_parenthesis: {
                if ( s[0]=='(' and s[1]=='*' and s[2]==')' ) {
                    app_tok( STRING_tensorial_product_NUM, s, 3, behavior_of_operator( STRING_tensorial_product_NUM ) );
                    s += 3;
                } else // (...
                    app_tok( Lexem::PAREN, s++, 1, 0 );
                break;
            }
            case TYPE_CHAR_comment: {
                const char *old_str = s; // used if error
                if ( s[1]=='#' ) { // ## pouet ##
                    for( s += 2; ; ++s ) {
                        if ( not *s ) {
                            error_list->add("Comment is not closed ( Ex: ## toto ## ).", old_str, provenance, 1 );
                            return;
                        }
                        if ( s[-1]=='#' and s[0]=='#' ) {
                            ++s;
                            break;
                        }
                    }
                } else if ( s[1]=='~' ) { // ## pouet ##
                    for( s += 2; ; ++s ) {
                        if ( not *s ) {
                            error_list->add("Comment is not closed ( Ex: #~ toto ~# ).", old_str, provenance, 1 );
                            return;
                        }
                        if ( s[-1]=='~' and s[0]=='#' ) {
                            ++s;
                            break;
                        }
                    }
                } else {
                    while( *(++s) and s[0] != '\n' ) ; // looking for a carriage return
                }
                break;
            }
            case TYPE_CHAR_dos_makes_me_sick:
                ++s;
                break;
            case TYPE_CHAR_end:
                return;
            default:
                error_list->add("Character is not allowed in this context.", s, provenance );
                ++s;
        }
    }
}

void Lexer::make_flat_token_list() {
    read_s<false>();
}

struct SetNextPrevFields {
    void operator()(Lexem &l) { l.prev = old; old->next = &l; old = &l; }
    Lexem *old;
};
    
void Lexer::set_next_prev_fields() {
    SetNextPrevFields sp;
    sp.old = &first_tok;
    tokens.apply( sp );
    sp.old->next = NULL;
}

struct ParenthesisHierachisation {
    void operator()(Lexem &i) {
        if ( i.type==Lexem::PAREN ) {
            if ( i.s[0]=='(' or i.s[0]=='[' or i.s[0]=='{' ) // opening
                ps_stack.push_back( &i );
            else { // closing parenthesis
                if ( ps_stack.size()==0 or ps_stack.back()->s[0] != ( i.s[0]==')'?'(':(i.s[0]==']'?'[':'{')) ) {
                    error_list->add( "Closing '"+std::string(1,i.s[0])+"' has no correspondance.", i.s, provenance );
                    return;
                }
                Lexem *o = ps_stack.back(); // assumed opening correspondance

                o->si += ( i.s - o->s );
                    
                ps_stack.pop_back();
                if ( i.prev == o ) { // [nothing]
                    o->next = i.next;
                    if ( i.next ) i.next->prev = o;
                }
                else { // [something ...]
                    if ( o->next ) {
                        o->next->parent = o;
                        o->next->prev = NULL;
                    }
                    o->children[0] = o->next;
                    o->next = i.next;
                        
                    if ( i.next ) i.next->prev = o;
                    if ( i.prev ) i.prev->next = NULL;
                }
                    
            }
        }
        else if ( i.type==Lexem::CR and ps_stack.size() )
            i.num = -1;
    }
    SplittedVec<Lexem *,64> ps_stack;
    const char *provenance;
    ErrorList *error_list;
};

void Lexer::parenthesis_hierarchisation() {
    ParenthesisHierachisation ph;
    ph.provenance = provenance;
    ph.error_list = error_list;
    tokens.apply( ph );
    
    for(unsigned i=0;i<ph.ps_stack.size();++i)
        error_list->add( "Opening '"+std::string(1,ph.ps_stack[i]->s[0])+"' has no correspondance.", ph.ps_stack[i]->s, provenance, true );
}

void Lexer::make_unit_hierarchy() {
    for(Lexem *t = first_of_type[ Lexem::NUMBER + 16 ]; t; t=t->sibling)
        if ( t->next and t->next->type == Lexem::STRING )
            assemble_right_arg(t);
}

void Lexer::remove_cr() {
    // remove unused CR ( void lines or CR with same num )
    for(Lexem *t = first_of_type[ Lexem::CR + 16 ]; t; t=t->sibling) {
        if ( t->next and t->next->type==Lexem::CR ) {
            t->num = -2;
            if ( t->prev )  t->prev->next = t->next;
            if ( t->next )  t->next->prev = t->prev;
        }
    }
    // then, we remove \n in a+\nb, a\n+b and so on...
    for(Lexem *t = first_of_type[ Lexem::CR + 16 ]; t; t=t->sibling) {
        if ( t->num >= -1 ) {
            if ( need_rarg(t->prev) and t->prev->children[0]==NULL and t->prev->type!=STRING___if___NUM and t->prev->type!=STRING___else___NUM and t->prev->type!=STRING___try___NUM ) {
                t->num = -2;
                t->prev->next = t->next;
                if ( t->next ) t->next->prev = t->prev;
            }
            else if ( need_larg(t->next) and t->next->children[0]==NULL and t->next->type!=STRING___if___NUM and t->next->type!=STRING___else___NUM ) {
                t->num = -2;
                t->next->prev = t->prev;
                if ( t->prev ) t->prev->next = t->next;
            }
        }
    }
    
    // after what, we make blocks
    int old_nb_spaces = 0;
    int nb_pending_spaces = 0;
    int pending_spaces_num[256];
    Lexem *pending_spaces_prev[256];
    for(Lexem *t = first_of_type[ Lexem::CR + 16 ]; t; t=t->sibling) {
        // if ( t-> ) continue;
        int tmp = t->num;
        if ( tmp >= 0 ) {
            if ( tmp == old_nb_spaces ) {
                if ( t->prev )  t->prev->next = t->next;
                if ( t->next )  t->next->prev = t->prev;
            }
            else if ( tmp > old_nb_spaces ) {
                t->type = Lexem::BLOCK;
                    
                if ( t->next ) {
                    t->children[0] = t->next;
                    if ( t->next ) {
                        t->next->parent = t;
                        t->next->prev = NULL;
                    }
                    t->next = NULL;
                    pending_spaces_num[nb_pending_spaces] = old_nb_spaces;
                    pending_spaces_prev[nb_pending_spaces] = t;
                    nb_pending_spaces += 1;
                    assert( nb_pending_spaces < 256 );
                }
                else { // a block with 0 instructions is not very interesting
                    if ( t->prev ) t->prev->next = t->next;
                    if ( t->next ) t->next->prev = t->prev;
                }
            }
            else {
                nb_pending_spaces -= 1;
                while ( tmp != pending_spaces_num[nb_pending_spaces] ) {
                    nb_pending_spaces -= 1;
                    if ( nb_pending_spaces < 0 ) {
                        error_list->add( "columns are not aligned.", t->s, provenance );
                        return;
                    }
                }
                if ( t->prev )
                    t->prev->next = NULL;
                if ( pending_spaces_prev[nb_pending_spaces] )
                    pending_spaces_prev[nb_pending_spaces]->next = t->next;
                if ( t->next )
                    t->next->prev = pending_spaces_prev[nb_pending_spaces];
            }
            old_nb_spaces = tmp;
        }
    }
}

void Lexer::assemble___calc_name__() {
    for(Lexem *o = first_of_type[STRING_calc_name_NUM+16]; o; o=o->sibling)
        assemble_right_arg( o );
}

void Lexer::assemble___string_assembly__() {
    for(Lexem *o = first_of_type[STRING___string_assembly___NUM+16]; o; o=o->sibling)
        assemble_2_args( o, true, true );
}

bool elligible_token_for_par(const Lexem *pr) { // return true if in something(), something should become a child of ()
    return ( pr and 
            ( pr->type<0 or 
                ( pr->type==Lexem::APPLY or
                  pr->type==Lexem::SELECT  or
                  pr->type==Lexem::CHANGE_BEHAVIOR  or
                  pr->type==STRING_get_attr_ptr_NUM or
                  pr->type==STRING_get_attr_NUM or
                  pr->type==STRING_doubledoubledot_NUM or
                  pr->type==STRING___self___NUM or
                  pr->type==STRING_calc_name_NUM
                )
            )
            and pr->type != Lexem::CCODE
            and pr->type != Lexem::CR
           );
}

///
void Lexer::assemble_par___getattr__() {
    SplittedVec<Lexem *,256> st;
    st.push_back( first_tok.next );
    while ( st.size() ) { // while stack is not empty
        for(Lexem *t=st.pop_back();t;t=t->next) {
            if ( t->children[0] )
                st.push_back( t->children[0] );
            if ( t->children[1] )
                st.push_back( t->children[1] );
            
            if ( t->type==Lexem::PAREN and elligible_token_for_par(t->prev) ) { 
                Lexem *pr = t->prev, *pa = pr->parent, *pp = pr->prev;
                
                if ( pa ) {
                    if ( pa->children[0]==pr )
                        pa->children[0] = t;
                    else
                        pa->children[1] = t;
                    t->parent = pa;
                    pr->parent = NULL;
                }
                if ( pp )
                    pp->next = t;
                t->prev = pp;
    
                if ( t->s[0] == '(' )      t->type = Lexem::APPLY;
                else if ( t->s[0] == '[' ) t->type = Lexem::SELECT;
                else                       t->type = Lexem::CHANGE_BEHAVIOR;
    
                t->children[1] = t->children[0];
                t->children[0] = pr;
                pr->parent = t;
                pr->next = NULL;
                pr->prev = NULL;
            }
            else if ( (t->type==STRING_get_attr_NUM or t->type==STRING_get_attr_ptr_NUM or t->type==STRING_doubledoubledot_NUM) and elligible_token_for_par( t->prev ) ) {
                assemble_2_args( t, true, true );
                if ( t->children[1] )
                    st.push_back( t->children[1] );
            }
        }
    }
}

bool Lexer::assemble_2_args(Lexem *o,int need_left,int need_right) {
    if ( need_right and o->next==NULL ) {
        error_list->add( "Operator "+std::string(o->s,o->s+o->si)+" needs a right expression.", o->s, provenance, is_in_main_block(o) );
        return false;
    }
    if ( need_left and o->prev==NULL ) {
        error_list->add( "Operator "+std::string(o->s,o->s+o->si)+" needs a left expression.", o->s, provenance );
        return false;
    }
    // prev
    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if (o->prev->parent->children[0]==o->prev)
            o->prev->parent->children[0] = o;
        else
            o->prev->parent->children[1] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->children[0] = o->prev;
    o->children[0]->parent = o;
    o->prev = o->prev->prev;

    o->children[0]->next = NULL;
    o->children[0]->prev = NULL;

    // next
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[1] = o->next;
    o->children[1]->parent = o;
    o->next = o->next->next;

    o->children[1]->prev = NULL;
    o->children[1]->next = NULL;

    return true;
}

bool Lexer::assemble_right_arg(Lexem *o) {
    if ( o->next == NULL ) {
        error_list->add( "Operator "+std::string(o->s,o->s+o->si)+" needs a right expression.", o->s, provenance, is_in_main_block(o));
        return false;
    }
    if ( o->next->next )
        o->next->next->prev = o;

    o->children[0] = o->next;
    o->children[0]->parent = o;
    o->next = o->next->next;

    o->children[0]->prev = NULL;
    o->children[0]->next = NULL;
    return true;
}

bool Lexer::assemble_left_arg(Lexem *o) {
    if ( o->prev == NULL ) {
        error_list->add( "Operator "+std::string(o->s,o->s+o->si)+" needs a left expression.", o->s, provenance );
        return false;
    }
    if ( o->prev->parent ) {
        o->parent = o->prev->parent;
        if (o->prev->parent->children[0]==o->prev)
            o->prev->parent->children[0] = o;
        else
            o->prev->parent->children[1] = o;
    }
    if ( o->prev->prev )
        o->prev->prev->next = o;

    o->children[0] = o->prev;
    o->children[0]->parent = o;
    o->prev = o->prev->prev;

    o->children[0]->next = NULL;
    o->children[0]->prev = NULL;
    return true;
}

/// take a,x. Should x be a right value for a tuple ?
bool tok_elligible_maylr(const Lexem *o,const Lexem *orig) {
    return o and not (
            o->type == Lexem::CR or
            o->type == STRING___pertinence___NUM or
            o->type == STRING___print___NUM or
            o->type == STRING___info___NUM or
            o->type == STRING___return___NUM or
            o->type == STRING___in___NUM or
            o->type == STRING___comma___NUM or
            o->type == STRING___comma_dot___NUM or
            o->type == STRING___pass___NUM or

            ( orig->type == STRING_sub_NUM and (
                o->type == STRING_assign_NUM or
                o->type == STRING_reassign_NUM or
                o->type == STRING_inferior_NUM or
                o->type == STRING_superior_NUM or
                o->type == STRING_inferior_equal_NUM or
                o->type == STRING_superior_equal_NUM or
                o->type == STRING_equal_NUM or
                o->type == STRING_not_equal_NUM or
                o->type == STRING_shift_left_NUM or
                o->type == STRING_shift_left_then_endl_NUM or
                
                o->type == STRING_self_or_NUM or
                o->type == STRING_self_xor_NUM or
                o->type == STRING_self_and_NUM or
                o->type == STRING_self_mod_NUM or
                o->type == STRING_self_shift_left_NUM or
                o->type == STRING_self_shift_right_NUM or
                o->type == STRING_self_div_NUM or
                o->type == STRING_self_mul_NUM or
                o->type == STRING_self_sub_NUM or
                o->type == STRING_self_add_NUM or
                o->type == STRING_self_div_int_NUM or
                o->type == STRING_self_concatenate_NUM
            ) )
    );
}
/// transform next and prev to children if necessary
void Lexer::assemble_op() {
    for(int num_op=STRING_calc_name_NUM-1;num_op>=0;--num_op) {
        for(Lexem *o = first_of_type[num_op+16]; o; o=o->sibling) {
            int behavior = o->num;
            bool need_left = behavior & 1;
            bool need_right = behavior & 2;
            bool may_need_left = behavior & 4;
            bool may_need_right = behavior & 8;
            bool need_right_right = behavior & 16;

            // both
            if (
                ( need_left  or ( may_need_left  and tok_elligible_maylr(o->prev,o) ) ) and
                ( need_right or ( may_need_right and tok_elligible_maylr(o->next,o) ) )
               ) {
                assemble_2_args( o, need_left, need_right );
            }
            // only right
            else if ( need_left==0 and need_right ) {
                assemble_right_arg( o );
            }
            // only left
            else if ( need_left and need_right==0 ) {
                assemble_left_arg( o );
            }
            // need two tokens at the right
            else if ( need_right_right ) {
                if ( ! o->next ) {
                    error_list->add( "Operator needs a right expression.", o->s, provenance, is_in_main_block(o));
                    return;
                }
                if ( ! o->next->next ) {
                    //display_graph( l );
                    error_list->add( "Operator needs TWO right expressions.", o->s, provenance, is_in_main_block(o));
                    return;
                }

                if ( o->next->next->next )
                    o->next->next->next->prev = o;

                o->children[0] = o->next;
                o->children[0]->parent = o;

                o->children[1] = o->next->next;
                o->children[1]->parent = o;
                o->next = o->next->next->next;

                o->children[0]->prev = NULL;
                o->children[0]->next = NULL;
                o->children[1]->prev = NULL;
                o->children[1]->next = NULL;
            }

        }
    }
}


void Lexer::parse_definitions() {
    for(Lexem *o = first_of_type[STRING___class___NUM+16]; o; o=o->sibling) {
        o->num = -1;
        o->def = defs.new_elem();
        o->def->init( o, provenance, error_list );
    }
    for(Lexem *o = first_of_type[STRING___def___NUM  +16]; o; o=o->sibling) {
        o->num = -1;
        o->def = defs.new_elem();
        o->def->init( o, provenance, error_list );
    }
}

void Lexer::add_child_to_info( int stn ) {
    for(Lexem *t = first_of_type[ stn + 16 ]; t; t=t->sibling) {
        // get str
        if ( not t->children[0] ) continue;
        const Lexem *l = leftmost_child( t->children[0] );
        const Lexem *r = rightmost_child( t->children[0] );
        
        const char *b = l->s - ( l->type == Lexem::STRING );
        unsigned si = (r->s-b) + r->si + ( r->type == Lexem::STRING );

        Lexem *res = app_tok( Lexem::STRING, b, si, 0 );
        res->prev = NULL;
        res->next = NULL;
        res->sibling = NULL;
        t->children[1] = res;
        res->parent = t;
    }
}


