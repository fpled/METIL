#ifndef DEF_TRIAL_H
#define DEF_TRIAL_H

#include "config.h"
#include "variable.h"
#include "splittedvec.h"
#include "definitiondata.h"

struct Definition;
struct Scope;
struct SourceFile;

///
struct OldPC {
    const void *tok;
    const void *next_tok;
    SourceFile *sourcefile;
};

///
struct OldScope {
    Variable *last_named_variable;
    Variable *variable_sp;
    OldScope *prev;
    unsigned nb_errors; // because we want to know if nb errors has increased during while loops
};

///
struct Callable {
    DefinitionData *def_data;
    CppThreadFunc *cpp_thread_func;
    unsigned nb_args_including_varargs;
    
    Variable *self;
    Float64 pertinence;
    Callable *next;
    Variable *first_variable; /// first variable for execution of block
    Variable *condition; /// only if condition is not known at compile time
    unsigned testing_type_nb;
    Variable *arguments[1];
};

///
struct DefTrial {
    void rm_callables_without_enough_pertinence();
    void push_in_callable_to_try( Callable *c ); /// 
    void push_in_ok_callable( Callable *c, bool fully_asserted ); /// maintain ok_callable sorted

    std::string to_string() const;

    unsigned nb_uargs, nb_nargs;
    Variable *def_v; /// followed by arguments
    Definition *def;
    const Nstring *names;
    Float64 min_pertinence_unconditionnal;
    
    Callable *first_callable_to_try;
    Callable *first_ok_callable;
    Callable *current_callable;
    
    Variable *first_default_variable; /// made during test block or during fonction execution if no test block
    Variable *return_var;
    unsigned supplementary_attributes; /// for return_var
    
    const void *old_tok;
    const void *old_next_tok; /// we will use old_next_tok only if nb_call_to_do_until_old_next_tok == 1
    SourceFile *old_sourcefile;
    Variable *old_last_named_variable_in_current_scope; ///
    Variable *old_current_self;
    DefTrial *prev_def_trial;
    DefTrial *def_trial_of_creation_if_for;
    
    bool partial_instanciation;
    unsigned nb_call_to_do_until_old_next_tok;
};

const void *instantiate( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp, Type *type );
const void *play_with_ok_callables_after_all_trials_are_done( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp );
void fill_callable_lists( DefTrial *def_trial, Thread *th, const void *tok, Variable *&sp );
const void *end_of_a_try_block( Thread *th, bool ok, const void *tok, Variable *&sp );
const void *play_with_callables( Thread *th, DefTrial *def_trial, const void *tok, Variable *&sp );
const void *test_static_args( Thread *th, const void *tok, Callable *c, Variable *&sp );
void init_scope_for_callable( Thread *th, Callable *c );

#endif // DEF_TRIAL_H
