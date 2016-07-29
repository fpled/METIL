//
// C++ Implementation: variable
//
// Description: 
//
//
// Author: LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "variable.h"
#include "transientdata.h"
#include "type.h"
#include "definition.h"
#include "definitiondata.h"
#include "globaldata.h"
#include "thread.h"
#include "varargs.h"
#include <usual_strings.h>
#include "property.h"
#include "scope.h"
#include "system.h"
#include "md5.h"
#include "cfile.h"
#include <sstream>
#include <fstream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// #define INFO_ON_ALLOCATION

void Variable::init( Type *type_, unsigned attributes_ ) {
    #ifdef INFO_ON_ALLOCATION
        std::cerr << type_->name << std::endl;
    #endif
    
    unsigned room_for_transient_data = ( 1 + type_->nb_sub_objects ) * sizeof(TransientData);
    
    unsigned al = type_->needed_alignement - 1;
    unsigned item_size = ( type_->data_size_in_bits + al ) & ~al;
    unsigned data_size_in_bytes = ( item_size + 7 ) / 8;
        
    
    type = type_;
    type->inc_ref();
    
    data = (char *)malloc( data_size_in_bytes + room_for_transient_data + sizeof(int) /*cpt_use*/ );
    transient_data = reinterpret_cast<TransientData *>( data + data_size_in_bytes );
    
    // std::cout << type_->name << " " << type_->nb_sub_objects << std::endl;
    for(unsigned i=0;i<1 + type_->nb_sub_objects;++i)
        transient_data[i].init();
    attributes = attributes_;

    parent_type = type;
    parent_data = data;
    parent_transient_data = transient_data;
    
    // prev_named = NULL;
    
    cpt_use = reinterpret_cast<int *>( data + data_size_in_bytes + room_for_transient_data );
    *cpt_use = 0;
}
    
Definition *Variable::replace_by_method( Thread *th, Definition *method ) {
    #ifdef INFO_ON_ALLOCATION
        std::cerr << "Def" << std::endl;
    #endif
    
    Definition *def = (Definition *)malloc( sizeof(Definition) + sizeof(TransientData) + sizeof(int) );
    def->init( method );
    global_data.Def->inc_ref();
    
    // def->self
    make_copy( &def->self, this );
        
    //
    type = global_data.Def;
    data = reinterpret_cast<char *>( def );
    transient_data = reinterpret_cast<TransientData *>( data + sizeof(Definition) );
    transient_data->init();
    attributes = 0;
    
    parent_type = global_data.Def;
    parent_data = data;
    parent_transient_data = transient_data;
    
    cpt_use = reinterpret_cast<int *>( data + sizeof(Definition) + sizeof(TransientData) );
    *cpt_use = 0;
    
    return def;
}

Property *Variable::replace_by_property( Property *property ) {
    #ifdef INFO_ON_ALLOCATION
        std::cerr << "Property" << std::endl;
    #endif
    
    if ( property->self.type )
        std::cerr << "Internal weird things in " << __FILE__ << " " << __LINE__ << std::endl;
    Property *prop = (Property *)malloc( sizeof(Property) + sizeof(TransientData) + sizeof(int) );
    prop->init( property );
    global_data.Property->inc_ref();
    
    make_copy( &prop->self, this );
    //
    type = global_data.Property;
    data = reinterpret_cast<char *>( prop );
    transient_data = reinterpret_cast<TransientData *>( data + sizeof(Property) );
    transient_data->init();
    attributes = 0;
    
    parent_type = global_data.Property;
    parent_data = data;
    parent_transient_data = transient_data;
    
    cpt_use = reinterpret_cast<int *>( data + sizeof(Property) + sizeof(TransientData) );
    *cpt_use = 0;
    
    return prop;
}

bool Variable::replace_by_method_named( Thread *th, Nstring name ) {
    // look up for an attribute
    TypeVariable *attr = type->find_var( this, name );
    if ( attr and attr->v.type == global_data.Def ) {
        replace_by_method( th, reinterpret_cast<Definition *>( attr->v.data ) );
        return true;
    }
    return false;
}
void Variable::replace_by_non_static_attr( TypeVariable *attr ) {
    // an attribute
    type = attr->v.type;
    data += ( get_bit_offset() + attr->offset_in_bits ) / 8;
    set_bit_offset( ( get_bit_offset() + attr->offset_in_bits ) % 8 );
    transient_data += 1 + attr->num_in_transient_data;
    
    attributes |= attr->v.attributes & Variable::CONST;
    attributes &= ~Variable::TYPE_OF_ONLY;
}

void copy_bits( char *dest, unsigned dest_offset_in_bits, const char *src, unsigned src_offset_in_bits, unsigned data_size_in_bits ) {
    if ( dest_offset_in_bits==0 and src_offset_in_bits==0 ) {
        unsigned s = data_size_in_bits / 8;
        memcpy( dest, src, s );
        data_size_in_bits %= 8;
        if ( data_size_in_bits ) {
            char mask = ( 0xff >> 7-data_size_in_bits );
            dest[s] = ( dest[s] & ~mask ) | ( src[s] & mask );
        }
    }
    else if ( data_size_in_bits == 1 ) {
        char mask = ( 1 << 7-dest_offset_in_bits );
        if ( src[0] & ( 1 << 7-src_offset_in_bits ) )
            dest[0] |= mask;
        else 
            dest[0] &= ~mask;
    }
    else {
        std::cerr << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
    }
}


const void *init_using_default_values( Thread *th, const void *tok, Variable *v, Variable * &sp ) {
    // nb_init_call and reassign_using_mem_copy
    //     std::cout << v->type->name << std::endl;
    
    unsigned nb_init_call = 0;
    for(int num_variable=v->type->nb_variables-1;num_variable>=0;--num_variable) {
        TypeVariable *t = v->type->variables + num_variable;
        if ( t->num_attribute >= 0 and t->v.type_of_only()==false ) {
            if ( t->v.type->reassign_using_mem_copy )
                copy_bits( v->data + ( v->get_bit_offset() + t->offset_in_bits ) / 8, ( v->get_bit_offset() + t->offset_in_bits ) % 8,
                           t->v.data + t->v.get_bit_offset() / 8, t->v.get_bit_offset() % 8,
                           t->v.type->data_size_in_bits
                         );
            else        
                ++nb_init_call;
        }
    }
    //
    if ( nb_init_call ) {
        th->check_for_room_in_variable_stack( sp, 2 * nb_init_call, v );
        for(int num_variable=v->type->nb_variables-1;num_variable>=0;--num_variable) {
            TypeVariable *t = v->type->variables + num_variable;
            if ( t->num_attribute >= 0 and t->v.type->reassign_using_mem_copy==false and t->v.type_of_only()==false ) { // if in instance
                // push ref on variable to init, then to variable.init definition
                Variable *a = assign_ref_on( sp++, v ); // a
                a->replace_by_non_static_attr( t );  // a.toto
                if ( not a->replace_by_method_named( th, STRING_init_NUM ) ) // a.toto.init
                    if ( not th->replace_var_by_ext_method_named( a, STRING_init_NUM ) )
                        std::cerr << "init not found !!" << std::endl;;
                // push ref on value to copy
                assign_ref_on( sp++, &t->v ); // 16
            }
        }
        //
        if ( th->compile_mode )
            return exec_tok_apply( N<1>(), th, tok, 
                                   /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                   /*expect_res*/false, tok_end_def_block,
                                                 sp, nb_init_call );
        return exec_tok_apply( N<0>(), th, tok, 
                                   /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                   /*expect_res*/false, tok_end_def_block,
                                                 sp, nb_init_call );
    }
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}
const void *init_or_reassign_same_types( Thread *th, const void *tok, Variable *a, Variable *b, Bool &ret, Variable * &sp, Nstring name_proc ) {
    // nb_init_call and reassign_using_mem_copy
    unsigned nb_init_call = 0;
    for(int num_variable=a->type->nb_variables-1;num_variable>=0;--num_variable) {
        TypeVariable *t = a->type->variables + num_variable;
        if ( t->num_attribute >= 0 ) {
            if ( t->v.type->reassign_using_mem_copy ) {
                copy_bits( a->data + ( a->get_bit_offset() + t->offset_in_bits ) / 8, ( a->get_bit_offset() + t->offset_in_bits ) % 8,
                            b->data + ( b->get_bit_offset() + t->offset_in_bits ) / 8, ( b->get_bit_offset() + t->offset_in_bits ) % 8,
                            t->v.type->data_size_in_bits
                            );
                reassign_transient_data( th, a->transient_data + 1 + t->num_in_transient_data, b->transient_data + 1 + t->num_in_transient_data );
            }
            else
                ++nb_init_call;
        }
    }
    //
    if ( nb_init_call ) {
        th->check_for_room_in_variable_stack( sp, 2 * nb_init_call, a, b );
        for(int num_variable=a->type->nb_variables-1;num_variable>=0;--num_variable) {
            TypeVariable *t = a->type->variables + num_variable;
            if ( t->num_attribute >= 0 and t->v.type->reassign_using_mem_copy==false ) { // if in instance
                // push ref on variable to init, then to variable.init definition
                Variable *na = assign_ref_on( sp++, a ); // a
                na->replace_by_non_static_attr( t );  // a.toto
                if ( not na->replace_by_method_named( th, name_proc ) ) // a.toto.init
                    if ( not th->replace_var_by_ext_method_named( na, name_proc ) )
                        std::cerr << name_proc << " not found !!" << std::endl;;
                // push ref on value to copy
                Variable *nb = assign_ref_on( sp++, b ); // b
                nb->replace_by_non_static_attr( t );  // b.toto
            }
        }
        //
        if ( th->compile_mode )
            return exec_tok_apply( N<1>(), th, tok, 
                                /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                                sp, nb_init_call );
        return exec_tok_apply( N<0>(), th, tok, 
                                /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                                sp, nb_init_call );
    }
        
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

const void *init_and_return_false_if_same_types( Thread *th, const void *tok, Variable *a, Variable *b, Bool &ret, Variable * &sp ) {
    if ( a->type != b->type ) { ret = true; th->set_current_sourcefile( NULL ); return tok_end_def_block; }
    ret = false;
    return init_or_reassign_same_types( th, tok, a, b, ret, sp, STRING_init_NUM );
}
const void *reassign_and_return_false_if_same_types( Thread *th, const void *tok, Variable *a, Variable *b, Bool &ret, Variable * &sp ) {
    if ( a->type != b->type ) { ret = true; th->set_current_sourcefile( NULL ); return tok_end_def_block; }
    ret = false;
    return init_or_reassign_same_types( th, tok, a, b, ret, sp, STRING_reassign_NUM );
}

const void *varargs_contains_only_named_variables_included_in_self_non_static_variables( Thread *th, const void *tok, Variable *a, VarArgs &va, Bool &ret, Variable * &sp ) {
    if ( va.nb_uargs() ) {
        ret = false;
        th->set_current_sourcefile( NULL );
        return tok_end_def_block;
    }
    
    for(unsigned i=0;i<va.nb_nargs();++i) {
        TypeVariable *tv = a->type->find_var( a, va.name(i) );
        if ( tv==NULL or tv->num_attribute < 0 ) {
            ret = false;
            th->set_current_sourcefile( NULL );
            return tok_end_def_block;
        }
    }
    
    ret = true;
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

const void *init_using_varargs( Thread *th, const void *tok, Variable *var_to_init, VarArgs &va, Variable * &sp ) {
    Nstring name_proc = STRING_reassign_NUM;
    // nb_init_call and reassign_using_mem_copy
    unsigned nb_init_call = 0;
    for(unsigned i=0;i<va.nb_nargs();++i) {
        TypeVariable *t = var_to_init->type->find_var( var_to_init, va.name( i ) );
        nb_init_call += ( t and t->num_attribute >= 0 );
    }
    //
    if ( nb_init_call ) {
        th->check_for_room_in_variable_stack( sp, 2 * nb_init_call, var_to_init );
        for(unsigned i=0;i<va.nb_nargs();++i) {
            TypeVariable *t = var_to_init->type->find_var( var_to_init, va.name( i ) );
            if ( t and t->num_attribute >= 0 ) {
                // push ref on variable to init, then to variable.init definition
                Variable *na = assign_ref_on( sp++, var_to_init ); // a
                na->replace_by_non_static_attr( t );  // a.toto
                if ( not na->replace_by_method_named( th, name_proc ) ) // a.toto.init
                    if ( not th->replace_var_by_ext_method_named( na, name_proc ) )
                        std::cerr << name_proc << " not found !!" << std::endl;;
                // push ref on value to copy
                assign_ref_on( sp++, va.narg(i) ); // b
            }
        }
        //
        if ( th->compile_mode )
            return exec_tok_apply( N<1>(), th, tok, 
                                /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                              sp, nb_init_call );
        return exec_tok_apply( N<0>(), th, tok, 
                                /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                              sp, nb_init_call );
    }
    
    //
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

void push_destroy_methods_rec( Thread *th, const void *tok, const void *next_tok, Variable *sp, Type *type, char *data, unsigned offset_in_bits,
        TransientData *transient_data, unsigned &nb_calls_to_destroy, Type *parent_type, char *parent_data ) {
    if ( type->destroy_def_data ) {
        ++nb_calls_to_destroy;
        
        // get the def_trial made for...
        DefTrial *def_trial;
        if ( nb_calls_to_destroy == 1 ) {
            def_trial = (DefTrial *)th->info_stack.get_room_for( sizeof( DefTrial ) );
            def_trial->nb_uargs = 0;
            def_trial->nb_nargs = 0;
            def_trial->def_v = sp;
            def_trial->def = NULL;
            def_trial->names = NULL;
            //def_trial->min_pertinence_unconditionnal;
            def_trial->first_callable_to_try = NULL;
            def_trial->first_ok_callable = NULL;
            def_trial->current_callable = NULL;
            def_trial->first_default_variable = sp;
            def_trial->return_var = NULL;
            def_trial->old_tok = tok;
            def_trial->old_next_tok = next_tok;
            def_trial->old_sourcefile = th->current_sourcefile;
            def_trial->old_last_named_variable_in_current_scope = th->last_named_variable_in_current_scope; ///
            def_trial->old_current_self = th->current_self;
            def_trial->prev_def_trial = th->current_def_trial;
            def_trial->partial_instanciation = false;
            def_trial->nb_call_to_do_until_old_next_tok = 1;
            
            //
            th->current_def_trial = def_trial;
        }
        else {
            def_trial = th->current_def_trial;
            ++def_trial->nb_call_to_do_until_old_next_tok;
        }
        
        // Variable
        Variable *v = (Variable *)th->info_stack.get_room_for( sizeof( Variable ) + sizeof(int) );
        v->type = type;
        v->data = data;
        v->transient_data = transient_data;
        v->attributes = 0;
        v->set_bit_offset( offset_in_bits );
        v->parent_type = parent_type;
        v->parent_data = parent_data;
        v->cpt_use = (int *)( v + 1 );
        *v->cpt_use = 1;
        
        // Callable
        Callable *c = (Callable *)th->info_stack.get_room_for( sizeof( Callable ) );
        c->def_data = type->destroy_def_data;
        //if ( type->destroy_def_data->next_with_same_name )
        //    th->add_error( "TODO : multiple destroy functions", tok );
        c->cpp_thread_func = type->destroy_cpp_function;
        c->self = v;
        
        c->next = def_trial->first_ok_callable;
        def_trial->first_ok_callable = c;
    }
    else {
        // recursivity
        for(unsigned i=0;i<type->nb_variables;++i) {
            if ( type->variables[i].num_attribute >= 0 and type->variables[i].v.type_of_only() == false ) {
                unsigned o = offset_in_bits + type->variables[i].offset_in_bits;
                push_destroy_methods_rec( th, tok, next_tok, sp, type->variables[i].v.type, data + o/8, o%8, transient_data+1+type->variables[i].num_attribute, nb_calls_to_destroy, parent_type, parent_data );
            }
        }
    }
}

const void *destroy_var( Thread *th, const void *tok, const void *next_tok, Variable * &sp, Variable *v ) {
    if ( *v->cpt_use ) // referenced elsewhere...
        v->dec_ref();
    else { // have to look for destroy methods
        //std::cout << "destroy " << v->type->name << std::endl;
        // look recursively for destroy methods ( in v->parent and in children )
        if ( v->type_of_only() == false ) {
            unsigned nb_calls_to_destroy = 0;
            push_destroy_methods_rec( th, tok, next_tok, sp, v->parent_type, v->parent_data, 0, v->parent_transient_data, nb_calls_to_destroy, v->parent_type, v->parent_data );
            if ( nb_calls_to_destroy )
                return play_with_callables( th, th->current_def_trial, tok, sp );
        }
        
        // if no destroy method at all -> just free
        for(unsigned i=0;i<1 + v->parent_type->nb_sub_objects;++i)
            v->parent_transient_data[i].destroy( th );
        //dec_ref( th, v->parent_type );
        free( v->parent_data );
    }
    return NULL;
}

void uninitialised_vec_using_partial_instanciation( Thread *th, const void *tok, Variable *&sp, Variable *return_var, Variable *a, Int32 nb_items ) {
    if ( return_var ) {
        // calculation of data size
        unsigned room_for_transient_data = ( 1 + nb_items * ( 1 + a->type->nb_sub_objects ) ) * sizeof(TransientData);
    
        unsigned al = a->type->needed_alignement - 1;
        unsigned item_size = ( a->type->data_size_in_bits + al ) & ~al;
        unsigned data_size_in_bits = nb_items * item_size;
        unsigned data_size_in_bytes = ( data_size_in_bits + 7 ) / 8;
        
        //
        #ifdef INFO_ON_ALLOCATION
            std::cerr << "StatVec" << std::endl;
        #endif
        return_var->data = (char *)malloc( data_size_in_bytes + room_for_transient_data + sizeof(int) /*cpt_use*/ );
        return_var->transient_data = reinterpret_cast<TransientData *>( return_var->data + data_size_in_bytes );
        for(unsigned i=0;i<1 + nb_items * (a->type->nb_sub_objects + 1);++i)
            return_var->transient_data[i].init();
        return_var->attributes = 0;

        return_var->parent_data = return_var->data;
        return_var->parent_transient_data = return_var->transient_data;
    
        return_var->cpt_use = reinterpret_cast<int *>( return_var->data + data_size_in_bytes + room_for_transient_data );
        *return_var->cpt_use = 0;
    
        // type of return_var
        TypeAndSize ts( a->type, nb_items );
        std::map<TypeAndSize,Type>::iterator iter = global_data.static_vec_type_cache.find( ts );
        if ( iter != global_data.static_vec_type_cache.end() ) {
            return_var->type = &iter->second;
            return_var->parent_type = &iter->second;
        } else {
            Type *type = &global_data.static_vec_type_cache[ ts ];
            type->init( 0 );
            type->def_data = NULL;
            type->name = STRING_SimpleStaticVector_NUM;
            type->data_size_in_bits = data_size_in_bits;
            type->needed_alignement = a->type->needed_alignement;
            type->nb_sub_objects = ( a->type->nb_sub_objects + 1 ) * nb_items;
            type->reassign_using_mem_copy = a->type->reassign_using_mem_copy;
            
            type->sub_type_if_static_vec = a->type;
            a->type->inc_ref();
            
            type->nb_sub_value_if_static_vec = nb_items;
            // md5
            std::ostringstream ss;
            ss << "StaticVec_" << type->nb_sub_objects << "_" << md5_digest_to_string( type->sub_type_if_static_vec->md5_attributes_control_sum );
            md5_cs( ss.str().c_str(), ss.str().size(), type->md5_attributes_control_sum );
            
            //
            return_var->type = type;
            return_var->parent_type = type;
        }
        return_var->type->inc_ref();
    }
}

const void *__get_data_in_static_vec__( Thread *th, const void *tok, Variable *&sp, Variable *return_var, Variable *a, Int32 num_item ) {
    if ( num_item < 0 or num_item >= a->type->nb_sub_value_if_static_vec ) {
        if ( return_var )
            assign_error_var( return_var );
        std::ostringstream ss;
        ss << "index out of range (requesting number " << num_item << ", in a vector of size " << a->type->nb_sub_value_if_static_vec << ").";
        th->add_error( ss.str(), tok, std::vector<ErrorList::Provenance>(), 2 );
    }
    else if ( return_var ) {
        Type *type = a->type->sub_type_if_static_vec;
        unsigned al = type->needed_alignement - 1;
        unsigned item_size = ( type->data_size_in_bits + al ) & ~al;
        unsigned offset_in_bits = num_item * item_size;
        
        return_var->type = type;
        return_var->data = a->data + offset_in_bits / 8;
        return_var->transient_data = a->transient_data + 1 + ( a->type->sub_type_if_static_vec->nb_sub_objects + 1 ) * num_item;
        return_var->attributes = a->attributes;
        return_var->set_bit_offset( offset_in_bits % 8 );
        
        return_var->parent_type = a->type;
        // ++return_var->parent_type->cpt_use;
        return_var->parent_data = a->data;
        return_var->parent_transient_data = a->transient_data;
    
        return_var->cpt_use = a->cpt_use;
        a->inc_ref();
    }
    //
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

const void *__get_data_in_dyn_vec__( Thread *th, const void *tok, Variable *&sp, Variable *return_var, UntypedPtr ptr, Variable *var_ptr, Variable *var_type, Int32 num_item ) {
    if ( return_var ) {
        Type *type = var_type->type;
        unsigned al = type->needed_alignement - 1;
        unsigned item_size = ( type->data_size_in_bits + al ) & ~al;
        int offset_in_bits = num_item * item_size;
        
        char *data = (char *)ptr;
        return_var->type = type;
        return_var->data = data + offset_in_bits / 8;
        return_var->transient_data = NULL; // reinterpret_cast<TransientData *>( a->transient_data + 1 + ( a->type->nb_sub_objects + 1 ) * num_item );
        return_var->attributes = var_ptr->attributes;
        return_var->set_bit_offset( offset_in_bits % 8 );
        
        return_var->parent_type = var_ptr->type;
        // return_var->parent_type->inc_ref();
        
        return_var->parent_data = var_ptr->data;
        return_var->parent_transient_data = var_ptr->transient_data;
        
        static int toto; toto = 100000;
        return_var->cpt_use = &toto; // var_ptr->cpt_use; // hum
        //var_ptr->inc_ref();
    }
    //
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

void assign_string( Thread *th, const void *tok, Variable *v, const char *s, unsigned size, unsigned reserved ) {
    reserved = std::max( reserved, size );
    
    if ( not global_data.String ) {
        Variable *s_def_v = th->main_scope->find_var(STRING_String_NUM);
        if ( s_def_v->type != global_data.Def ) th->add_error( "String->type != global_data.Def !!!!", tok );
        global_data.String = reinterpret_cast<Definition *>( s_def_v->data )->def_data->type_cache[0];
    }
    v->init( global_data.String, 0 );
    //class String
    //    data ~= UntypedPtr
    //    size := 0
    //    reserved := 0
    //    encoding := 0
    if ( size ) {
        void *p = malloc( size );
        memcpy( p, s, size );
        *reinterpret_cast<void **>( v->data ) = p; // data
    }
    else
        *reinterpret_cast<void **>( v->data ) = NULL; // data
    reinterpret_cast<Int32 *>( reinterpret_cast<void **>( v->data ) + 1 )[0] = size; // size
    reinterpret_cast<Int32 *>( reinterpret_cast<void **>( v->data ) + 1 )[1] = reserved; // reserved
    reinterpret_cast<Int32 *>( reinterpret_cast<void **>( v->data ) + 1 )[2] = 0; // encoding
}

const void *destroy_instanciated_attributes( Thread *th, const void *tok, Variable *a, Variable * &sp ) {
    th->set_current_sourcefile( NULL );
    
    unsigned nb_calls = 0;
    for(unsigned i=0;i<a->type->nb_variables;++i)
        nb_calls += ( a->type->variables[i].num_attribute >= 0 and a->type->variables[i].v.type_of_only() == false );
    //
    if ( nb_calls ) {
        th->check_for_room_in_variable_stack( sp, nb_calls, a );
        for(unsigned i=0;i<a->type->nb_variables;++i) {
            if ( a->type->variables[i].num_attribute >= 0 and a->type->variables[i].v.type_of_only() == false ) {
                Variable *r = assign_ref_on( sp++, a );
                r->replace_by_non_static_attr( a->type->variables + i );  // a.toto
                if ( not r->replace_by_method_named( th, STRING_destroy_NUM ) ) // a.toto.init
                    if ( not th->replace_var_by_ext_method_named( r, STRING_destroy_NUM ) )
                        std::cerr << "destroy not found !!" << std::endl;;
            }
        }
        if ( th->compile_mode )
            return exec_tok_apply( N<1>(), th, tok, 
                                /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                              sp, nb_calls );
        return exec_tok_apply( N<0>(), th, tok, 
                                /*nb_uargs*/0, /*nb_nargs*/0, /*names*/NULL,
                                /*expect_res*/false, tok_end_def_block,
                                              sp, nb_calls );
    }
    //
    return tok_end_def_block;
}

int system_( const char *str, unsigned si ) {
    char *c_str = (char *)malloc( si + 1 );
    memcpy( c_str, str, si );
    c_str[si] = 0;
    int res = system( c_str );
    free( c_str );
    return res;
}

std::string rm_spaces_at_beg_of_lines( Int32 nb_to_rm, Variable *v ) {
    const char *s = *reinterpret_cast<char **>( v->data ); // data
    unsigned si = reinterpret_cast<Int32 *>( reinterpret_cast<char **>( v->data ) + 1 )[0]; // size
    std::string res; res.reserve( si );
    for(unsigned i=0;i<si;++i) {
        if ( s[i] == '\n' ) {
            res += '\n'; ++i; if ( i+1<si and s[i+1]=='\r' ) { res += '\r'; ++i; }
            for(int cpt=0;;++cpt,++i) {
                if ( cpt == nb_to_rm ) {
                    --i;
                    break;
                }
                if ( s[i]!=' ' ) {
                    res += s[i];
                    break;
                }
            }
        } else
            res += s[i];
    }
    return res;
}

std::string absolute_filename( const char *str, unsigned si ) {
    return absolute_filename( std::string( str, str+si ) );
}

Int64 last_modification_time_or_zero_of_file_named( Variable *v ) {
    const char *s = *reinterpret_cast<char **>( v->data ); // data
    unsigned si = reinterpret_cast<Int32 *>( reinterpret_cast<char **>( v->data ) + 1 )[0]; // size
    return last_modification_time_or_zero_of_file_named( std::string( s, s + si ) );
}

void get_includes_of_cpp_named( Thread *th, const void *tok, VarArgs &va, const char *s, unsigned si, bool &cc, bool &cg ) {
    va.init();
    
    std::vector<std::string> dep;
    get_includes_of_cpp_named( dep, s, si, cc, cg );
    for(unsigned i=0;i<dep.size();++i)
        assign_string( th, tok, va.variables.new_elem(), &dep[i][0], dep[i].size() );
}

bool str_eq( Variable *a, Variable *b ) {
    const char *s0 = *reinterpret_cast<char **>( a->data ); // data
    unsigned si0 = reinterpret_cast<Int32 *>( reinterpret_cast<char **>( a->data ) + 1 )[0]; // size
    const char *s1 = *reinterpret_cast<char **>( b->data ); // data
    unsigned si1 = reinterpret_cast<Int32 *>( reinterpret_cast<char **>( b->data ) + 1 )[0]; // size
    return si0==si1 and strncmp( s0, s1, si1 )==0;
}

void split_dir( Thread *th, const void *tok, const char *s, unsigned si, VarArgs &ret ) {
    ret.init();
    for(unsigned i=0,oi=0;;++i) {
        if ( i == si or s[i] == '/' or s[i] == '\\' ) {
            if ( oi != i )
                assign_string( th, tok, ret.variables.new_elem(), s+oi, i-oi );
            if ( i == si )
                break;
            oi = i + 1;
        }
    }
}

void getenv( Thread *th, const void *tok, const char *s, unsigned si, Variable *ret ) {
    if ( not ret )
        return;
    std::string str( s, s+si );
    const char *res_cstr = getenv( str.c_str() );
    if ( not res_cstr )
        res_cstr = "";
    assign_string( th, tok, ret, res_cstr, strlen(res_cstr) );
}

void getcwd( Thread *th, const void *tok, Variable *ret ) {
    for( unsigned st = 1024; ; st *= 2 ) {
        char *res = (char *)malloc( st );
        if ( getcwd( res, st ) ) {
            assign_string( th, tok, ret, res, strlen(res) );
            free( res );
            return;
        }
        free( res );
    }
}

void get_os_type_( Thread *th, const void *tok, Variable *return_var ) {
    if ( not return_var )
        return;
    std::string str = get_os_type();
    assign_string( th, tok, return_var, &str[0], str.size() );
}

void get_cpu_type_( Thread *th, const void *tok, Variable *return_var ) {
    if ( not return_var )
        return;
    std::string str = get_cpu_type();
    assign_string( th, tok, return_var, &str[0], str.size() );
}

void get_type_md5_sum( Thread *th, const void *tok, Variable *a, Variable *return_var ) {
    if ( not return_var )
        return;
    std::string res = md5_digest_to_string( a->type->md5_attributes_control_sum );
    assign_string( th, tok, return_var, &res[0], res.size() );
}

void get_next_line_( Thread *th, const void *tok, CFile &a, Int32 b, Variable *return_var ) {
    std::string res;
    if ( b < 0 )
        b = std::numeric_limits< Int32 >::max();
    for( ; b>=0; --b ) {
        char c = getc( a.f );
        if ( feof( a.f ) or c=='\n' )
            break;
        res += c;
    }
    if ( return_var )
        assign_string( th, tok, return_var, &res[0], res.size() );
}

typedef void CcodeFunction0 (  );
typedef void CcodeFunction1 ( Variable * );
typedef void CcodeFunction2 ( Variable *, Variable * );
typedef void CcodeFunction3 ( Variable *, Variable *, Variable * );
typedef void CcodeFunction4 ( Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction5 ( Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction6 ( Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction7 ( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction8 ( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction9 ( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction10( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction11( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );
typedef void CcodeFunction12( Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable *, Variable * );

void exec_ccode_function( void *f_ ) { ((CcodeFunction0 *)( f_ ))(  ); }
void exec_ccode_function( void *f_, Variable *a ) { ((CcodeFunction1 *)( f_ ))( a ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b ) { ((CcodeFunction2 *)( f_ ))( a, b ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c ) { ((CcodeFunction3 *)( f_ ))( a, b, c ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d ) { ((CcodeFunction4 *)( f_ ))( a, b, c, d ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e ) { ((CcodeFunction5 *)( f_ ))( a, b, c, d, e ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f ) { ((CcodeFunction6 *)( f_ ))( a, b, c, d, e, f ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g ) { ((CcodeFunction7 *)( f_ ))( a, b, c, d, e, f, g ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h ) { ((CcodeFunction8 *)( f_ ))( a, b, c, d, e, f, g, h ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i ) { ((CcodeFunction9 *)( f_ ))( a, b, c, d, e, f, g, h, i ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j ) { ((CcodeFunction10 *)( f_ ))( a, b, c, d, e, f, g, h, i, j ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j, Variable *k ) { ((CcodeFunction11 *)( f_ ))( a, b, c, d, e, f, g, h, i, j, k ); }
void exec_ccode_function( void *f_, Variable *a, Variable *b, Variable *c, Variable *d, Variable *e, Variable *f, Variable *g, Variable *h, Variable *i, Variable *j, Variable *k, Variable *l ) { ((CcodeFunction12 *)( f_ ))( a, b, c, d, e, f, g, h, i, j, k, l ); }

typedef void UntypedPtrFunction0 (  );
typedef void UntypedPtrFunction1 ( void * );
typedef void UntypedPtrFunction2 ( void *, void * );
typedef void UntypedPtrFunction3 ( void *, void *, void * );
typedef void UntypedPtrFunction4 ( void *, void *, void *, void * );
typedef void UntypedPtrFunction5 ( void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction6 ( void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction7 ( void *, void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction8 ( void *, void *, void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction9 ( void *, void *, void *, void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction10( void *, void *, void *, void *, void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction11( void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void * );
typedef void UntypedPtrFunction12( void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void * );


void exec_untyped_ptr_function( void *f_ ) { ((UntypedPtrFunction0 *)( f_ ))(); }
void exec_untyped_ptr_function( void *f_, void *a ) { ((UntypedPtrFunction1 *)( f_ ))( a ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b ) { ((UntypedPtrFunction2 *)( f_ ))( a, b ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c ) { ((UntypedPtrFunction3 *)( f_ ))( a, b, c ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d ) { ((UntypedPtrFunction4 *)( f_ ))( a, b, c, d ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e ) { ((UntypedPtrFunction5 *)( f_ ))( a, b, c, d, e ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f ) { ((UntypedPtrFunction6 *)( f_ ))( a, b, c, d, e, f ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g ) { ((UntypedPtrFunction7 *)( f_ ))( a, b, c, d, e, f, g ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h ) { ((UntypedPtrFunction8 *)( f_ ))( a, b, c, d, e, f, g, h ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i ) { ((UntypedPtrFunction9 *)( f_ ))( a, b, c, d, e, f, g, h, i ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j ) { ((UntypedPtrFunction10 *)( f_ ))( a, b, c, d, e, f, g, h, i, j ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j, void *k ) { ((UntypedPtrFunction11 *)( f_ ))( a, b, c, d, e, f, g, h, i, j, k ); }
void exec_untyped_ptr_function( void *f_, void *a, void *b, void *c, void *d, void *e, void *f, void *g, void *h, void *i, void *j, void *k, void *l ) { ((UntypedPtrFunction12 *)( f_ ))( a, b, c, d, e, f, g, h, i, j, k, l ); }


void *mmap_file( const char *s, unsigned si, Int32 &length ) {
    std::string str( s, s+si );
    int fd = open( str.c_str(), O_RDONLY );
    if ( fd < 0 )
        return NULL;
    
    struct stat stat_file;
    if ( fstat( fd, &stat_file ) )
        return NULL;
    length = stat_file.st_size;
    
    return mmap( /*start*/ 0, length, PROT_READ, MAP_PRIVATE, fd, /*offset*/ 0 );
}

void munmap_( void *a, Int32 length ) {
    munmap( a, length );
}


