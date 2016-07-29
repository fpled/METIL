#ifndef MAKE_DOT_TOK_DATA_H
#define MAKE_DOT_TOK_DATA_H

#include "tokheader.h"

struct ErrorList;
struct Lexem;

TokData make_tok_data( const char *txt, const char *provenance, ErrorList *error_list, bool one_var=false ); ///

#ifdef MAKE_DOT_TOK_DATA_CPP

/**
    Convert a set of lexem to data that must reside in corresponding .tok file.
 */
class DotSarToDotTok {
public:
    DotSarToDotTok( const char *sar_txt_, const char *provenance, ErrorList *error_list, bool one_var );
    ~DotSarToDotTok();
    
    TokData contiguous_bin_data(); /// must be freed outside
private:
    struct RelData { /// string rellocation data
        RelData() {}
        RelData(const char *s_,unsigned si_): s(s_), si(si_), n( s, si ) { }
        RelData(const char *s_,unsigned si_,int n_): s(s_), si(si_), n( n_ ) { }
        bool operator==(const RelData &red) const { return n == red.n; }

        const char *s;
        unsigned si;
        Nstring n;
    };
    struct StackPrev {
        typedef enum {
            UNNAMED_VARIABLE,
            NAMED_VARIABLE,
            SOMETHING_UNKNOWN,
            //DEF_TRIAL,
            //BEG_SCOPE
        } TypeVariable;
        
        struct Item {
            bool var_with_calculated_name() const { return type==NAMED_VARIABLE and var < 0; }
            TypeVariable type;
            int var; /// name of variable if VARIABLE, < 0 means calculated name...
        };
        
        ///
        StackPrev(StackPrev *parent_=NULL) : parent(parent_) {
            size = 0;
            needed_size = 0;
            main_one = false;
            of_class = 0;
        }
        ///
        int find_var_in_scope( int n ) const {
            //             if ( n <= 0 ) return 0;
            //             
            //             int res = 0, cont = true;
            //             for(const StackPrev *s = this; s and cont; s=s->parent ) {
            //                 for(int i = s->items.size()-1; i>=0; --i ) {
            //                     --res;
            //                     
            //                     if ( s->items[i].var_with_calculated_name() ) cont = false; // there's a variable be we don't known its name
            //                     if ( s->items[i].type == SOMETHING_UNKNOWN ) return 0; // scope is invalidated
            //                     
            //                     if ( s->items[i].var == n and (s->items[i].type==UNNAMED_VARIABLE or s->items[i].type==NAMED_VARIABLE) )
            //                         return res; // found
            //                 }
            //             }
            return 0;
        }
        ///
        int find_var_in_args( int n ) const {
            if ( n <= 0 ) return -1;
            for(unsigned i=0;i<args.size();++i)
                if ( args[i] == n and i < max_num_arg_that_can_be_used )
                    return i;
            return -1;
        }
        bool has_unknown_var() const {
            for(int i = items.size()-1; i>=0; --i )
                if ( items[i].type == SOMETHING_UNKNOWN or items[i].var_with_calculated_name() )
                    return true;
            return false;
        }
        bool assign_name_to_last_var( int n ) {
            if ( n > 0 ) {
                for(int i = items.size()-1; i>=0; --i ) // look for variable with same name in scope
                    if ( items[i].type == NAMED_VARIABLE and items[i].var == n )
                        return false;
                for(unsigned i=0; i<args.size(); ++i ) // look for variable with same name in args
                    if ( args[i] == n )
                        return false;
            }
            items.back().var = n;
            items.back().type = NAMED_VARIABLE;
            return true;
        }
        void push_unnamed_var() {
            Item *i = items.new_elem();
            i->type = UNNAMED_VARIABLE;
            i->var = 0;
            ++size;
            needed_size = std::max( needed_size, size );
        }
        bool push_named_var( int n ) {
            push_unnamed_var();
            return assign_name_to_last_var( n );
        }
        void push_something_unknown() {
            Item *i = items.new_elem();
            i->type = SOMETHING_UNKNOWN;
            i->var = 0;
        }
        void push_arg(int n) {
            args.push_back( n );
            max_num_arg_that_can_be_used = args.size();
        }
        void pop() { --size; items.pop_back(); }
                        
        StackPrev *parent;
        SplittedVec<Item,256> items;
        SplittedVec<int,16> args;
        unsigned needed_size, size;
        bool main_one;
        int of_class;
        unsigned max_num_arg_that_can_be_used;
    };
    
    void push_type_and_offsets( const Lexem *l, unsigned type, int nb_var_created_or_destroyed_in_stack );
    int push_string( const char *s, unsigned si ); /// push number of string in bin and necessary info in strings_to_rellocate and pos_strings_rellocations...
    void push_block( const Lexem *l ); /// convert lexems to TokInBlock data
    void push_tok( const Lexem *l, bool expect_result ); /// add individual lexem
    void push_tok_if_calc( const Lexem *n, int &nb_calculated_names, int nstring_conv );
    int push_string_or_get_calc( const Lexem *n );
    void push_tok_apply( const Lexem *l, unsigned nb_unnamed, bool expect_result );
    void push_tok_make_vec( const Lexem *l );
    void get_array_data( const Lexem *t, SplittedVec<const Lexem *,32> &ch, SplittedVec<unsigned,4> &dim );
    
    void push_tok_generic_apply( const Lexem *l, bool expect_result );
    void push_to_and_or_or( const Lexem *l, bool expect_result );
    void push_tok_assign( const Lexem *n );
    void push_tok_get_attr( const Lexem *l, bool ptr, bool partial_instanciation );
    void push_tok_definition( const Lexem *l );
    void push_tok_definition_try_block( const Lexem *l );
    void push_tok_for( const Lexem *l );
    void push_tok_lambda( const Lexem *l );
    void push_tok_label( const Lexem *l );
    void push_tok_if( const Lexem *l );
    void push_tok_string( const Lexem *l );
    void push_tok_operator( const Lexem *l, bool expect_result );
    void push_tok_new( const Lexem *l, bool expect_result );
    void push_tok_variable_( const Lexem *l, const RelData &red );
    void push_tok_variable( const Lexem *l, int n );
    void push_tok_variable( const Lexem *l, const char *s, unsigned si );
    void push_tok_if_rec( const Lexem *l, const SplittedVec<const Lexem *,8> &ch, const SplittedVec<const Lexem *,8> &conditions, unsigned n, unsigned &max_needed_size_in_stack );
    void push_default_arguments_values( const Lexem *l );
    void push_tok_try( const Lexem *l );
    void push_tok_catch( const Lexem *l );
    void push_tok_doubledoubledot( const Lexem *l );
    void push_tok_get_value_prop( const Lexem *l );
    
    
    SplittedVec<unsigned,1024,16384> pos_types_to_rellocate;
    SplittedVec<unsigned,1024,16384> pos_strings_to_rellocate; /// pos in tok.xx file
    SplittedVec<int,64,64> property_names; /// property is_someting, ...
    HashSet<RelData,1024,1024,16384> strings_to_rellocate;
    SplittedVec<char,4096,16384> bin; /// binary data

    const char *sar_txt, *provenance;
    ErrorList *error_list;
    
    bool static_, const_, virtual_;
    StackPrev *stack_prev;
    
    friend struct GetRoomForStringInfo;
    friend struct WriteStringInfo;
};


#endif

#endif // MAKE_DOT_TOK_DATA_H
