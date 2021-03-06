//
// C++ Interface: type
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TYPE_H
#define TYPE_H

#include "nstring.h"
#include "variable.h"

struct DefinitionData;
struct Definition;
struct Thread;
struct Scope;
struct DefTrial;
class SourceFile;

typedef const void *CppThreadFunc( Thread *th, const void *tok, Variable *return_var, Variable **args, Variable *self_var, Variable * &sp );
typedef void CppThreadTryFunc( Thread *th, DefTrial *def_trial, DefinitionData *dd, Variable * &sp ); // push Callable if OK

/**
 */
struct TypeVariable {
    Variable v;
    unsigned offset_in_bits;
    int num_attribute; /// -1 means not stored in instanciations
    int num_in_transient_data; /// first attribute -> 0
    TypeVariable *prev_in_hash_table;
};

/**
	@author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
struct Type {
    static const unsigned size_hash_table = 16;
            
    void init( unsigned prevision_of_nb_variables );
    void init( DefinitionData *dd, Variable *last_named_variable );
    void destroy();
    TypeVariable *find_var( Variable *self, Nstring n );
    std::string to_string() const;
    void register_var( TypeVariable *tv, Nstring n );
    unsigned nb_attributes_in_inst() const;
    unsigned nb_static_attributes_in_inst() const;
    Nstring name_attributes_in_inst(unsigned n) const;
    Nstring name_static_attributes_in_inst(unsigned n) const;
    TypeVariable *attr_in_inst_nb(unsigned n);
    TypeVariable *static_attr_in_inst_nb(unsigned n);
    void init_md5_attributes_control_sum(unsigned char c);
    unsigned size_of_in_in_vec() const { unsigned al = needed_alignement - 1; return ( data_size_in_bits + al ) & ~al; }
    
    Int32 nb_template_parameters() const;
    Nstring name_template_parameter_nb(Int32 n) const;
    Variable *template_parameter_nb(Int32 n);

    // Variable *zero_var; Variable *one_var; Variable *two_var;
    // Op *zero_op; Op *one_op; Op *two_op;

    DefinitionData *def_data; /// the def_data from which *this comes
    Nstring name; /// copied from def_data
    char char_prim;
    char *c_type; // only for code generation
    
    unsigned nb_variables;
    TypeVariable *variables;
    TypeVariable *hash_table[ size_hash_table ];
    
    DefinitionData *destroy_def_data; ///
    CppThreadFunc *destroy_cpp_function; ///
    
    unsigned char md5_attributes_control_sum[16];
    
    // unsigned data_size_in_bytes; /// 
    unsigned data_size_in_bits;
    unsigned needed_alignement;
    int cpt_use;
    inline void inc_ref() {
        ++cpt_use;
        if ( sub_type_if_static_vec )
            ++sub_type_if_static_vec->cpt_use;
    }
    
    unsigned nb_sub_objects; ///
    bool reassign_using_mem_copy; /// true only for some primitives
    bool cant_be_converted_to_bool; /// things like Op, TensorialProperties::i > TensorialProperties::j
    bool contains_virtual_methods;
    
    Type *sub_type_if_static_vec;
    Int32 nb_sub_value_if_static_vec;
};

bool store_in_inst( Variable &v );

void dec_ref( Thread *th, Type *t );

bool inheritance( Type *a, Type *b ); // check if a inherits from b

#endif

