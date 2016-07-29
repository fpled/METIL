#ifndef THREAD_H
#define THREAD_H

#include "interpreterbehavior.h"
#include "ref.h"
#include "errorlist.h"
#include "splittedvec.h"
#include "deftrial.h"
#include "n.h"
#include <fstream>

struct SourceFile;
struct Scope;
struct Variable;
struct Type;

///
class Thread {
public:
    // methods
    Thread( Scope *main_scope, ErrorList *error_list, Thread *parent=0, unsigned prereserved_room_in_variable_stack = 4096*4 );
    ~Thread();
    void push_prev_file_to_read( SourceFile *sf );
    void primitive_init();
    const void *pop_pc(); /// use old_pc_list
    const void *pop_def_trial(); ///
    const void *go_to_next_trial();
    Variable *find_var_in_current_scope( Nstring name );
    Variable *find_var( Nstring name );
    Variable *find_var_but_not_a_property_name_in_current_scope( Nstring name );
    int nb_accessible_variables() const; /// return nb accessible named variables
    Nstring accessible_variables_nb(unsigned n) const;
    void register_var_in_current_scope( const void *tok, Variable *v, Nstring name, bool need_verification );
    void set_current_sourcefile( SourceFile *sf ) { current_sourcefile = sf; }
    void add_error( const std::string &msg, const void *tok, const std::vector<ErrorList::Provenance> &possibilities=std::vector<ErrorList::Provenance>(), unsigned nb_def_trial_to_skip=0 );
    
    SignedSize check_for_room_in_variable_stack( Variable * &sp, int nb_variables );
    SignedSize check_for_room_in_variable_stack( Variable * &sp, int nb_variables, Variable *&v0 );
    SignedSize check_for_room_in_variable_stack( Variable * &sp, int nb_variables, Variable *&v0, Variable *&v1 );
    void push_var_if_to_be_destroyed( Variable *v, Variable * &sp ); /// @see destroy( Definition * ... )
    void push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( Variable *v ); ///
    bool currently_in_main_scope() const;
    void fill_first_callable_to_try( DefTrial *def_trial, Variable *&sp );
    bool replace_var_by_ext_method_named( Variable *v, Nstring name );
    bool replace_var_by_ext_property_named( Variable *v, Nstring name );
    void rm_var_in_current_scope( Variable *v );
    unsigned nb_def_trial() const;
    int display_stack(const void *tok);
    //
    
    Variable *beg_variable_stack, *end_variable_stack; /// nearly all the variable data are stored here
    SplittedVec<Variable,64,64> variables_to_del_at_next_end_block; /// nearly all the data are stored here
    SplittedVec<Type,1024,2048> *types; /// type cache
    SplittedVec<char,16096,65536> info_stack; /// for DefTrial, Callable...
    // SplittedVec<Thread *,8> child_threads; /// 
    
    Variable *current_variable_sp; /// nearly all the data are stored here
    const void *current_tok; /// not up to date only during calls to thread_loop
    SourceFile *current_sourcefile;
    DefTrial *current_def_trial;
    Variable *current_self;
    Variable *last_named_variable_in_current_scope;  /// not used if in main_scope
    OldScope *old_scope;
        
    Scope *main_scope;  /// 
    Thread *parent; ///
    int argc;
    char **argv;
    
    ErrorList *error_list; ///
    InterpreterBehavior interpreter_behavior; ///
    SplittedVec<OldPC,4> old_pc_list;
    unsigned cpt_use;
    int return_value;
    bool compile_mode, profile_mode;
    unsigned max_recursive_depth;
    struct DisplayWindowCreator *display_window_creator;
//     std::ofstream display_stack_file;
private:
    void append_def_data_if_basic_conditions_are_checked( DefTrial *def_trial, Variable *&sp, DefinitionData *dd );
    void push_destroy_methods_rec( unsigned &nb_calls_to_destroy, Variable * &sp, Type *type, char *data, TransientData *transient_data );
};
    
extern unsigned tok_end_def_block[], tok_test_next_type[];

void assign_error_var( Variable *v );
void assign_void_var( Variable *v );

bool thread_loop( Thread *th ); /// 

template<int compile_mode> const void *exec_tok_apply( const N<compile_mode> &n__compile_mode, Thread *th, const void *tok,
        unsigned nb_unnamed, unsigned nb_named, const Nstring *named, bool expect_res, const void *next_tok,Variable * &sp,
        unsigned nb_calls=1, bool partial_instanciation=false, Variable *return_var=NULL, unsigned supplementary_attributes=0 );

template<int compile_mode> const void *exec_tok_select( const N<compile_mode> &n__compile_mode, Thread *th, const void *tok,
        unsigned nb_unnamed, unsigned nb_named, const Nstring *named, bool expect_res, const void *next_tok, Variable * &sp, Variable *return_var );


Int32 __nb_stack_level__( const Thread *th, const void *tok );
void __file_at_stack_level__( Thread *th, const void *tok, Int32 level, Variable *return_var );
Int32 __line_at_stack_level__( const Thread *th, const void *tok, Int32 level );
Int32 __col_at_stack_level__( const Thread *th, const void *tok, Int32 level );
void __line_str_at_stack_level__( Thread *th, const void *tok, Int32 level, Int32 nb_col_max, Variable *return_var );
void calc_name( Thread *th, const void *tok, Variable *return_var, const char *s, unsigned si );
int direct_conv_to_bool( Variable *v );

#endif // THREAD_H
