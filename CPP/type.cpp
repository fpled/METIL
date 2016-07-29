//
// C++ Implementation: type
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "type.h"
#include "definitiondata.h"
#include "definition.h"
#include <usual_strings.h>
#include "globaldata.h"
#include "splittedset.h"
#include "thread.h"
#include "md5.h"
#include "property.h"

void Type::init( unsigned prevision_of_nb_variables ) {
    // zero_var = NULL; one_var = NULL; two_var = NULL; zero_op = NULL; one_op = NULL; two_op = NULL;
    for(unsigned i=0;i<size_hash_table;++i)
        hash_table[ i ] = NULL;
    
    destroy_def_data = NULL;
    destroy_cpp_function = NULL;
    
    variables = NULL;
    nb_sub_objects = 0;
    nb_variables = 0;
    cpt_use = 0;
    sub_type_if_static_vec = NULL;
    cant_be_converted_to_bool = false;
    contains_virtual_methods = false;
    
    if ( prevision_of_nb_variables )
        variables = (TypeVariable *)malloc( sizeof(TypeVariable) * prevision_of_nb_variables );
        
}
void Type::init_md5_attributes_control_sum(unsigned char c) {
    md5_attributes_control_sum[0] = c;
    for(unsigned i=1;i<16;++i)
        md5_attributes_control_sum[i] = 0;
}

bool store_in_inst( Variable &v ) {
    return v.is_static()==false and v.type!=global_data.Def and v.type!=global_data.Property;
}

bool is_static_attr( Variable &v ) {
    return v.is_static() and not v.is_primary_def();
}

extern Thread *main_thread;

void Type::init( DefinitionData *dd, Variable *last_named_variable ) {
    // basic init
    for(unsigned i=0;i<size_hash_table;++i)
        hash_table[ i ] = NULL;
    
    def_data = dd; /// the def_data from which *this comes
    name = dd->name; /// copied from def_data
    
    nb_variables = 0;
    variables = NULL;
    cpt_use = 0;
    sub_type_if_static_vec = NULL;
    
    destroy_def_data = NULL;
    destroy_cpp_function = NULL;

    data_size_in_bits = 0;
    needed_alignement = 1;
    nb_sub_objects = 0; ///
    reassign_using_mem_copy = false;
    cant_be_converted_to_bool = false;
    
    // get nb_variables
    for( Variable *v=last_named_variable; v; v=v->prev_named, ++nb_variables ) ;
    if ( nb_variables ) {
        // fill 'variables'
        variables = (TypeVariable *)malloc( sizeof(TypeVariable) * nb_variables );
        Variable *v = last_named_variable;
        for(int i=nb_variables-1;i>=0;--i,v=v->prev_named) {
            make_copy( &variables[i].v, v );
            if ( v->is_primary_def() )
                variables[i].v.attributes |= Variable::IS_PRIMARY_DEF;
            v->inc_ref();
            register_var( variables + i, v->name );
            variables[i].num_attribute = -1; // by default
            variables[i].num_in_transient_data = -1; // by default
            //
            //             if ( v->is_primary_def() ) {
            //             }
            //             varistore_in_instables[i].
        }
        /*
        // get data size and alignements
        SplittedSet<unsigned,32> alignement_set;
        for( unsigned i=0; i<nb_variables; ++i )
            if ( store_in_inst( variables[i].v ) )
                alignement_set.insert( variables[i].v.type->needed_alignement );
    
        if ( alignement_set.size() ) {
            // get offset_in_bits
            needed_alignement = alignement_set[0];
            data_size_in_bits = 0;
            for(unsigned nai=0,cpt_var=0;nai<alignement_set.size();++nai) {
                unsigned al = alignement_set[nai];
                for( unsigned i=0; i<nb_variables; ++i ) {
                    if ( store_in_inst(variables[i].v) and variables[i].v.type->needed_alignement==al ) {
                        if ( al )
                            data_size_in_bits = ( data_size_in_bits + al - 1 ) & ~( al - 1 );
                        variables[i].offset_in_bits = data_size_in_bits;
                        variables[i].num_attribute = cpt_var++;
                        variables[i].num_in_transient_data = nb_sub_objects;
                        data_size_in_bits += variables[i].v.type->data_size_in_bits;
                        nb_sub_objects += 1 + variables[i].v.type->nb_sub_objects;
                    }
                }
            }
            // data_size_in_bytes is used for the allocation of Variable
            //static const unsigned al_by = sizeof(int) * 8 - 1;
            //data_size_in_bytes = ( (data_size_in_bits + al_by ) & ~al_by ) / 8;
        }
        else {
            needed_alignement = 0;
            data_size_in_bits = 0;
            //data_size_in_bytes = 0;
        }
        */
        needed_alignement = 0;
        data_size_in_bits = 0;
        for(unsigned nai=0,cpt_var=0;nai<nb_variables;++nai) {
            unsigned al = variables[nai].v.type->needed_alignement;
            if ( store_in_inst(variables[nai].v) ) {
                if ( al )
                    data_size_in_bits = ( data_size_in_bits + al - 1 ) & ~( al - 1 );
                needed_alignement = std::max( needed_alignement, al );
                variables[nai].offset_in_bits = data_size_in_bits;
                variables[nai].num_attribute = cpt_var++;
                variables[nai].num_in_transient_data = nb_sub_objects;
                data_size_in_bits += variables[nai].v.type->data_size_in_bits;
                nb_sub_objects += 1 + variables[nai].v.type->nb_sub_objects;
            }
        }
        
        // destroy ?
        for( unsigned i=0; i<nb_variables; ++i )
            if ( variables[i].v.name == STRING_destroy_NUM and variables[i].v.type == global_data.Def )
                destroy_def_data = reinterpret_cast<Definition *>( variables[i].v.data )->def_data;
    }
    
    // md5_attributes_control_sum
    std::ostringstream ss;
    std::string ns( name );
    ss << ns.size() << "_" << ns << "_" << nb_sub_objects << "_";
    for(unsigned i=0;i<nb_variables;++i) {
         if ( variables[i].num_attribute >= 0 ) {
            std::string ns( variables[i].v.name );
            ss << "_" << ns.size() << "_" << ns << "_";
            ss.write( (const char *)variables[i].v.type->md5_attributes_control_sum, 16 );
         }
    }
    md5_cs( ss.str().c_str(), ss.str().size(), md5_attributes_control_sum );
    
    // cant_be_converted_to_bool
    for(unsigned i=0;i<nb_variables;++i) {
        if ( variables[i].v.name == STRING_cant_be_converted_to_bool_NUM ) {
            int cond = direct_conv_to_bool( &variables[i].v );
            cant_be_converted_to_bool |= ( cond == 1 );
        }
    }
    
    // virtual
    contains_virtual_methods = false;
    for(unsigned i=0;i<nb_variables;++i) {
        if ( variables[i].v.is_virtual() ) { // and variables[i].v.is_primary_def()
            contains_virtual_methods = true;
            //
            data_size_in_bits += sizeof(void *) * 8;
            needed_alignement = std::max( needed_alignement, unsigned( sizeof(int) * 8 ) );
            for(unsigned j=0;j<nb_variables;++j)
                if ( variables[j].offset_in_bits >= 0 )
                    variables[j].offset_in_bits += sizeof(void *) * 8;
            break;
        }
    }
    
    // 
    for(unsigned i=0;i<nb_variables;++i) {
        if ( variables[i].v.is_virtual() ) { // and variables[i].v.is_primary_def()
            if ( variables[i].v.type == global_data.Def ) {
                Definition *def = reinterpret_cast<Definition *>( variables[i].v.data );
                def->def_data->parent_type = this;
            }
        }
    }
    
    // check that template args are not properties
    for(unsigned i=0;i<def_data->nb_args;++i) {
        Variable *v = &find_var( NULL, def_data->args[i] )->v;
        if ( v->type == global_data.Property ) {
            std::cout << reinterpret_cast<Property *>( v->data )->get_() << std::endl;
            main_thread->add_error( "property in template args is forbiden.", NULL );
        }
    }
    
}

void Type::destroy() {
    //    nb_variables = 0; // to destroy Def
    if ( variables )
        free( variables );
}


TypeVariable *Type::find_var( Variable *self, Nstring n ) {
    unsigned h = n.v % size_hash_table;
    //     std::cout << name << " " << n << std::endl;
    //     std::cout << hash_table[h] << std::endl;
    for(TypeVariable *tv = hash_table[h]; tv; tv=tv->prev_in_hash_table) {
        if ( tv->v.name == n ) {
            if ( tv->v.is_virtual() and self and not self->type_of_only() ) {
                //                 main_thread->display_stack_file << self->type->name << " " << n << std::endl;
                //                 main_thread->display_stack(0);
                Type *nt = *reinterpret_cast<Type **>( self->data );
                TypeVariable *tmp = nt->find_var( NULL, n );
                if ( tmp )
                    return tmp;
            }
            return tv;
        }
    }
    return NULL;
}

void Type::register_var( TypeVariable *tv, Nstring n ) {
    unsigned h = n.v % size_hash_table;
    tv->v.name = n ;
    tv->prev_in_hash_table = hash_table[ h ];
    hash_table[ h ] = tv;
}
    
std::string Type::to_string() const {
    return name;
}

unsigned Type::nb_attributes_in_inst() const {
    unsigned res = 0;
    for( unsigned i=0; i<nb_variables; ++i )
        res += store_in_inst( variables[i].v );
    return res;
}

unsigned Type::nb_static_attributes_in_inst() const {
    unsigned res = 0;
    for( unsigned i=0; i<nb_variables; ++i )
        res += is_static_attr( variables[i].v );
    return res;
}

Nstring Type::name_attributes_in_inst(unsigned n) const {
    for( unsigned i=0; i<nb_variables; ++i )
        if ( store_in_inst( variables[i].v ) )
            if ( n-- == 0 )
                return variables[i].v.name;
    return 0;
}

Nstring Type::name_static_attributes_in_inst(unsigned n) const {
    for( unsigned i=0; i<nb_variables; ++i )
        if ( is_static_attr( variables[i].v ) )
            if ( n-- == 0 )
                return variables[i].v.name;
    return 0;
}

TypeVariable *Type::attr_in_inst_nb(unsigned n) {
    for( unsigned i=0; i<nb_variables; ++i )
        if ( store_in_inst( variables[i].v ) )
            if ( n-- == 0 )
                return variables + i;
    return NULL;
}
TypeVariable *Type::static_attr_in_inst_nb(unsigned n) {
    for( unsigned i=0; i<nb_variables; ++i )
        if ( is_static_attr( variables[i].v ) )
            if ( n-- == 0 )
                return variables + i;
    return NULL;
}

Int32 Type::nb_template_parameters() const {
    return def_data->nb_args;
}

Nstring Type::name_template_parameter_nb(Int32 n) const {
    return def_data->args[n];
}

Variable *Type::template_parameter_nb(Int32 n) {
    return &find_var( NULL, def_data->args[n] )->v;
}


void dec_ref( Thread *th, Type *t ) {
    if ( t->sub_type_if_static_vec )
        dec_ref( th, t->sub_type_if_static_vec );
    //
    if ( t->cpt_use == 0 ) {
        for(unsigned i=0;i<t->nb_variables;++i)
            th->push_var_in_variables_to_del_at_next_end_block_if_to_be_destroyed( &t->variables[i].v );
        // t->nb_variables = 0;
        // t->destroy();
    }
    //     else if ( t->cpt_use < 0 )
    //         std::cerr << "sur-deleting " << t->name << std::endl;
    --t->cpt_use;
}

bool inheritance( Type *a, Type *b ) {
    if ( a == b )
        return true;
    for( unsigned i=0; i<a->nb_variables; ++i )
        if ( a->variables[i].v.name == STRING_0_inheritance_NUM and a->variables[i].v.type == b )
            return true;
    return false;
}

