//
// C++ Implementation: sartiinteractive
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <string.h>
#include "metil_interactive.h"
#include "thread.h"
#include "autorestore.h"
#include "globaldata.h"
#include "tokheader.h"
#include "sourcefile.h"
#include "typechar.h"
#include <metil_readline_config.h>
#ifdef HAVE_READLINE
    #include <readline/readline.h>
    #include <readline/history.h>
#endif
#include <iostream>
#include <fstream>
#include <string.h>

static Thread *current_thread;
#ifdef HAVE_READLINE
inline bool is_num_or_letter(char c) { return type_char(c) == TYPE_CHAR_number or type_char(c) == TYPE_CHAR_letter; }

const char *operators[] = {
    "init_using_default_values",
    "continue",
    "continue_n",
    "break",
    "break_n",
    "pass",
    "self",
    "try",
    "catch",
    "interact",
    "transient",
    "static",
    "const",
    "class",
    "def",
    "import",
    "inherits",
    "pertinence",
    "when",
    "else",
    "while",
    "for",
    "if",
    "return",
    "return_n",
    "throw",
    "info",
    "print",
    "in",
    "ref",
    "partial_instanciation",
    NULL
};

char *find_var_generator(const char *text,int state) {
    static int list_index = 0;
    static int nb_operators = 0;
    if ( state == 0 ) {
        nb_operators = 0; while ( operators[nb_operators] ) ++nb_operators;
        list_index = nb_operators + current_thread->nb_accessible_variables() - 1; // rc.named_variable_stack.size()-1;
    }
    while ( list_index >= 0 ) {
        std::string str;
        if ( list_index < nb_operators )
            str = operators[ list_index-- ];
        else
            str = current_thread->accessible_variables_nb( list_index-- - nb_operators );
        if ( str.size() >= strlen(text) and strncmp( &str[0], text, strlen(text) ) == 0 )
            return strdup( str.c_str() );
    }
    return NULL;
}
char *find_attr_generator(const char *text,int state) {
    static TypeVariable *attributes = NULL;
    static int num_attributes = -1;
    static unsigned e;
    if ( state == 0 ) {
        e = strlen(text);
        while ( --e and text[e]!='.' and text[e]!='>' );

        if ( text[e]=='.' ) {
            char *lhs_str = (char *)malloc( e+2 );
            memcpy( lhs_str+1, text, e );
            lhs_str[0] = 0;
            lhs_str[e+1] = 0;
            
            Variable *v = current_thread->find_var( Nstring( lhs_str + 1 ) );
            if ( v ) {
                attributes = v->type->variables;
                num_attributes = v->type->nb_variables - 1;
            }
            else {
                for(unsigned i=1;lhs_str[i];++i) {
                    if ( not is_num_or_letter( lhs_str[i] ) ) {
                        std::cout << "TODO : calculate composed names" << std::endl;
                        return NULL;
                    }
                }
                std::cout << "\nUnable to find any variable named '" << lhs_str + 1 << "'." << std::endl;
                //Scope *scope = ( rc.current_scope ? rc.current_scope : &rc.main_scope );
                //AutoRestore<std::ostream *> aer( rc.error_stream ); rc.error_stream = &std::cerr;
                //Variable lhs = scope->calc_string( lhs_str ); // parse
                //if ( lhs.type != rc.type_cache.Error )
                //    tv = lhs.type->attributes;
                //destroy_variable( lhs );
                //attributes = NULL;
                //num_attributes = -1;
                return NULL;
            }
            
        }
        else {
            std::cout << "last chars in text for autocompletion are weird." << std::endl;
            return NULL;
        }
    }
    while ( num_attributes >= 0 ) {
        std::string str( attributes[num_attributes--].v.name );
        if ( str.size() >= strlen(text)-e-1 and strncmp( &str[0], text+e+1, strlen(text)-e-1 ) == 0 ) {
            char *res = (char *)malloc( e + 1 + str.size() + 1 );
            memcpy( res, text, e+1 ); res[e+1] = 0;
            strcat( res, str.c_str() );
            return res;
        }
    }
    return NULL;
}
char **sarti_completion(const char *text, int beg, int end) {
    bool want_lhs = false;
    int b = strlen(text);
    if ( b ) {
        while ( --b ) {
            if ( text[b]==')' or text[b]=='}' or text[b]==']' ) { // skip (...)
                if ( want_lhs==false )
                    return NULL;
            }
            if ( text[b]=='.' or (b>1 and text[b-1]=='-' and text[b]=='>') ) {
                want_lhs = true;
                continue;
            }
            if ( is_num_or_letter(text[b]) == false ) {
                ++b;
                break;
            }
        }
    }
    
    if ( want_lhs ) // lhs.something
        return rl_completion_matches( text+b, find_attr_generator );
    //
    return rl_completion_matches( text+b, find_var_generator );
}
void initialize_readline() {
    rl_attempted_completion_function = sarti_completion;
}
static bool readline_initialised = false;
std::string old_in_hist;
std::ofstream hist_file;
#endif

const void *metil_interactive(Thread *th,const void *tok,const void *next_tok,Variable *&sp,const char *prompt1,const char *prompt2) {
    current_thread = th;
    th->interpreter_behavior.interpretation = true;
        
    th->error_list->data.clear();
    AutoRestore<std::ostream *> aes( th->error_list->error_stream, NULL ); // delayed display of errors
    char *old_line = NULL; // if line not ended...
    
    #ifdef HAVE_READLINE
    if ( not readline_initialised ) {
        initialize_readline();
        readline_initialised = true;
        
        // history
        std::ifstream f( ".metil_history" );
        std::vector<std::string> ol;
        if ( f ) {
            while ( true ) {
                std::string tmp;
                std::getline( f, tmp );
                if ( not f )
                    break;
                add_history(tmp.c_str());
                ol.push_back( tmp );
            }
            f.close();
        }
        //
        hist_file.open( ".metil_history" );
        for(unsigned i=std::max(size_t(0),ol.size()-1000);i<ol.size();++i)
            hist_file << ol[i] << std::endl;
    }
    #endif
            
    while ( true ) {
        const char *prompt = ( old_line ? prompt2 : prompt1 );
        #ifdef HAVE_READLINE
            char *tmp = readline(prompt);
            if ( not tmp ) { std::cout << std::endl; return next_tok; }
            for(unsigned i=0;tmp[i];++i) {
                if ( type_char( tmp[i] ) != TYPE_CHAR_space  ) {
                    if ( tmp != old_in_hist ) {
                        old_in_hist = tmp;
                        add_history(tmp);
                        hist_file << tmp << "\n";
                    }
                    break; 
                }
            }
            if ( strcmp(tmp,"exit")==0 ) { std::cout << std::endl; return next_tok; }
            if ( strcmp(tmp,"quit")==0 ) { std::cout << std::endl; return next_tok; }
            char sp = tmp[0]; // first char of the line
    
            char *line;
            if ( old_line ) {
                line = (char *)malloc( strlen(old_line+1) + strlen(tmp) + 3 );
                strcpy( line+1, old_line+1 );
                free( old_line );
                old_line = NULL;
                strcat( line+1, tmp );
            } else {
                line = (char *)malloc( strlen(tmp) + 3 );
                strcpy( line+1, tmp );
            }
            strcat( line+1, "\n" ); // useful if line not finished
            line[0] = 0;
            free(tmp);
        #else
            std::cout << prompt;
            std::string sline;
            std::getline(std::cin,sline);
            if ( not std::cin ) { std::cout << std::endl; return next_tok; }
            if ( sline=="exit" ) { std::cout << std::endl; return next_tok; }
            if ( sline=="quit" ) { std::cout << std::endl; return next_tok; }
            char sp = sline[0]; // first char of the line
            
            char *line;
            if ( old_line ) {
                line = (char *)malloc( strlen(old_line+1) + sline.size() + 3 );
                strcpy( line+1, old_line+1 );
                free( old_line );
                old_line = NULL;
                strcat( line+1, sline.c_str() );
            } else {
                line = (char *)malloc( sline.size() + 3 );
                strcpy( line+1, sline.c_str() );
            }
            strcat( line+1, "\n" ); // useful if line not finished
            line[0] = 0;
        #endif

        // that is the block is not finished
        if ( sp==' ' ) {
            old_line = line;
            continue;
        }

        // parse
        SourceFile *sf = global_data.get_sourcefile_from_line( line, th->error_list, &th->interpreter_behavior );
        if ( not sf ) {
            // line is finished ?
            if ( th->error_list->line_not_finished() )
                old_line = line;
            else {
                for(unsigned i=0;i<th->error_list->data.size();++i)
                    std::cerr << th->error_list->data[i] << std::endl;
                free(line);
            }
            th->error_list->data.clear();
        }
        else {
            for(unsigned i=0;i<th->error_list->data.size();++i)
                std::cerr << th->error_list->data[i] << std::endl;
            th->error_list->data.clear();
            //
            OldPC *pc = th->old_pc_list.new_elem();
            pc->sourcefile = th->current_sourcefile;
            pc->tok = tok;
            pc->next_tok = tok;
        
            th->set_current_sourcefile( sf );
            sf->add_property_names( th->main_scope );
            return sf->tok_header()->tok_data();
        }
    }
    
    return next_tok;
}


