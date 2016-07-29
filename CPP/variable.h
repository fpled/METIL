//
// C++ Interface: variable
//
// Description: 
//
//
// Author: LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef VARIABLE_H
#define VARIABLE_H

#include "nstring.h"
#include "bool.h"
#include "config.h"

struct TransientData;
struct Type;
struct TypeVariable;
struct Thread;
struct Definition;
struct Property;
struct VarArgs;

/**
	@author LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct Variable {
    static const unsigned CONST = 1;
    static const unsigned STATIC = 2;
    static const unsigned TYPE_OF_ONLY = 4; /// toto ~= Pouet
    static const unsigned DONT_USE_DESTROY = 8; /// do the free() on data without looking for __destroy__ in .met or in ccode
    static const unsigned CAN_BE_REDEFINED = 16;
    static const unsigned IS_PRIMARY_DEF = 32;
    static const unsigned VIRTUAL = 64;
    
    void init( Type *type, unsigned attributes );
    bool replace_by_method_named( Thread *th, Nstring name );

    // generic data
    Type *type;
    char *data;
    TransientData *transient_data;
    unsigned attributes;
    
    
    // parent
    Type *parent_type;
    char *parent_data;
    TransientData *parent_transient_data;
    int *cpt_use; /// times parent is used
    
    // name
    Nstring name; /// if named
    Variable *prev_named;

    // wrapper function
    void set_bit_offset(unsigned o) { attributes &= 0xFF; attributes |= (o << 8); }
    unsigned get_bit_offset() const { return attributes >> 8; }
    bool is_const()           const { return attributes & CONST; }
    bool is_static()          const { return attributes & STATIC; }
    bool can_be_redefined()   const { return attributes & CAN_BE_REDEFINED; }
    bool type_of_only()       const { return attributes & TYPE_OF_ONLY; }
    bool is_primary_def()     const { return attributes & IS_PRIMARY_DEF; }
    bool is_virtual()         const { return attributes & VIRTUAL; }
    void inc_ref() { ++ *cpt_use; }
    void dec_ref() { -- *cpt_use; }
    Definition *replace_by_method( Thread *th, Definition *method );
    Property *replace_by_property( Property *property );
    void replace_by_non_static_attr( TypeVariable *attr );
    
    operator bool() const { return bool( type ); }
};
    
const void *destroy_var( Thread *th, const void *tok, const void *next_tok, Variable * &sp, Variable *v );
void push_destroy_methods_rec( Thread *th, const void *tok, const void *next_tok, Variable *sp, Type *type, char *data, unsigned offset_in_bits, 
        TransientData *transient_data, unsigned &nb_calls_to_destroy, Type *parent_type, char *parent_data );

inline const void *rm_last_var_in_sp( Thread *th, const void *tok, const void *next_tok, Variable *&sp ) {
    Variable *v = --sp;
    return destroy_var( th, tok, next_tok, sp, v );
}

// v -> n
inline void make_copy(Variable *n,Variable *v) {
    // generic data
    n->type = v->type;
    n->data = v->data;
    n->transient_data = v->transient_data;
    n->attributes = v->attributes & ~Variable::IS_PRIMARY_DEF;
    
    // parent
    n->parent_type = v->parent_type;
    n->parent_data = v->parent_data;
    n->parent_transient_data = v->parent_transient_data;
    n->cpt_use = v->cpt_use;
}
// v -> n
inline Variable *assign_ref_on(Variable *n,Variable *v) {
    make_copy( n, v );
    v->inc_ref();
    return n;
}

// v -> n
inline Variable *assign_name_and_ref_on(Variable *n,Variable *v) {
    make_copy( n, v );
    n->name = v->name;
    v->inc_ref();
    return n;
}

// v -> n
inline void make_copy_with_name(Variable *n,Variable *v) {
    make_copy( n, v );
    n->name = v->name;
}

typedef Variable *BegScope;

void copy_bits( char *dest, unsigned dest_offset_in_bits, const char *src, unsigned src_offset_in_bits, unsigned data_size_in_bits );
const void *init_using_default_values( Thread *th, const void *tok, Variable *v, Variable * &sp );
const void *init_and_return_false_if_same_types( Thread *th, const void *tok, Variable *a, Variable *b, Bool &ret, Variable * &sp );
const void *reassign_and_return_false_if_same_types( Thread *th, const void *tok, Variable *a, Variable *b, Bool &ret, Variable * &sp );
const void *varargs_contains_only_named_variables_included_in_self_non_static_variables( Thread *th, const void *tok, Variable *a, VarArgs &va, Bool &ret, Variable * &sp );
const void *init_using_varargs( Thread *th, const void *tok, Variable *a, VarArgs &va, Variable * &sp );
void uninitialised_vec_using_partial_instanciation( Thread *th, const void *tok, Variable *&sp, Variable *return_var, Variable *a, Int32 b );
const void *__get_data_in_static_vec__( Thread *th, const void *tok, Variable *&sp, Variable *return_var, Variable *a, Int32 num_item );
const void *__get_data_in_dyn_vec__( Thread *th, const void *tok, Variable *&sp, Variable *return_var, UntypedPtr ptr, Variable *var_ptr, Variable *var_type, Int32 num_item );
void assign_string( Thread *th, const void *tok, Variable *v, const char *s, unsigned size, unsigned reserved = 0 );
const void *destroy_instanciated_attributes( Thread *th, const void *tok, Variable *a, Variable * &sp );
void make_symbol( Thread *th, const void *tok, Variable *return_var, Variable *partial_inst, Variable *cpp_name, Variable *tex_name );

std::string rm_spaces_at_beg_of_lines( Int32 a, Variable *b );
// void symbolic_graphviz_string(std::ostream &os, Variable *v);
// std::string symbolic_graphviz_string(Variable *v);
// std::string symbolic_tex_string(Variable *v);
// std::string symbolic_cpp_string(Variable *v);
// void symbolic_diff( Thread *th, const void *tok, Variable *return_var, Variable *a, Variable *b );
// void symbolic_subs( Thread *th, const void *tok, Variable *return_var, Variable *a, VarArgs &b, VarArgs &c );
// void symbolic_simplifications( Thread *th, const void *tok, Variable *return_var, Variable *a, Int32 level );

int system_( const char *str, unsigned si );
std::string absolute_filename( const char *str, unsigned si );
Int64 last_modification_time_or_zero_of_file_named( Variable *a );
void get_includes_of_cpp_named( Thread *th, const void *tok, VarArgs &va, const char *s, unsigned si, bool &cc, bool &cg );
bool str_eq( Variable *a, Variable *b );
void split_dir( Thread *th, const void *tok, const char *s, unsigned si, VarArgs &ret );
void getenv( Thread *th, const void *tok, const char *s, unsigned si, Variable *ret );
void getcwd( Thread *th, const void *tok, Variable *ret );
void get_os_type_( Thread *th, const void *tok, Variable *return_var );
void get_cpu_type_( Thread *th, const void *tok, Variable *return_var );
void get_type_md5_sum( Thread *th, const void *tok, Variable *a, Variable *return_var );
void get_next_line_( Thread *th, const void *tok, struct CFile &a, Int32 b, Variable *return_var );

void exec_ccode_function( void *f_ );
void exec_ccode_function( void *f_, Variable *a );
void exec_ccode_function( void *f_, Variable *a, Variable *b );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j, Variable *k );
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j, Variable *k, Variable *l );

void exec_untyped_ptr_function( void *f_ );
void exec_untyped_ptr_function( void *f_, void *a );
void exec_untyped_ptr_function( void *f_, void *a, void *b );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j, void *k );
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j, void *k, void *l );

void exec_asm( void *th, void *code );

void *mmap_file( const char *s, unsigned si, Int32 &length );
void munmap_( void *a, Int32 length );

#endif

