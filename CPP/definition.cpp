//
// C++ Implementation: definition
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "definition.h"
#include "definitiondata.h"
#include "ref.h"
#include "thread.h"
#include <usual_strings.h>
#include "globaldata.h"
#include "scope.h"

void Definition::init( DefinitionData *def_data_ ) {
    def_data = def_data_;
    nb_uargs = 0;
    nb_nargs = 0;
    args = NULL;
    self.type = NULL;
    max_pertinence = FLOAT64_MAX_VAL;
    want_self = false;
    cached_type = NULL;
}
    
void Definition::reassign( Definition *d, Thread *th, Variable * &sp ) {
    //std::cout << "reassign " << __FILE__ << std::endl;
    Definition old = *this;
    init( d );
    destroy( old, th, sp );
}

void Definition::init( Definition *def, unsigned nb_supplementary_uargs, unsigned nb_supplementary_nargs ) {
    def->def_data->inc_this_and_children_cpt_use();
    
    def_data = def->def_data;
    nb_uargs = def->nb_uargs;
    nb_nargs = def->nb_nargs;
    args = NULL;
    if ( def->args or nb_supplementary_uargs or nb_supplementary_nargs ) {
        args = (Variable *)malloc( sizeof(Variable) * ( def->nb_nargs + def->nb_uargs + nb_supplementary_uargs + nb_supplementary_nargs ) );
        for(unsigned i=0;i<def->nb_uargs;++i) {
            make_copy( args + i, def->args + i );
            args[ i ].inc_ref();
        }
        for(unsigned i=0;i<def->nb_nargs;++i) {
            make_copy_with_name( args + def->nb_uargs + i + nb_supplementary_uargs, def->args + def->nb_uargs + i );
            args[ def->nb_uargs + i + nb_supplementary_uargs ].inc_ref();
        }
    }
    if ( def->self.type ) {
        make_copy( &self, &def->self );
        self.inc_ref();
    }
    else
        self.type = NULL;
   
    max_pertinence = def->max_pertinence;
    if ( nb_supplementary_uargs + nb_supplementary_nargs )
        cached_type = NULL;
    else
        cached_type = def->cached_type;
}

const void *destroy( Definition &d, Thread *th, Variable * &sp ) {
    // end by destroying types 
    for(DefinitionData *dd=d.def_data; dd; ) {
        DefinitionData *odd = dd;
        dd = dd->next_with_same_name;
        
        if ( odd->cpt_use==0 ) {
            odd->destroy( th );
            free( odd );
        }
        else
            --odd->cpt_use;
    }
    
    //
    if ( d.self.type )
        th->push_var_if_to_be_destroyed( &d.self, sp );
    for(unsigned i=0;i<d.nb_uargs;++i)
        th->push_var_if_to_be_destroyed( d.args + i, sp );
    for(unsigned i=0;i<d.nb_nargs;++i)
        th->push_var_if_to_be_destroyed( d.args + d.nb_uargs + i, sp );
    
    if ( d.args )
        free( d.args );
    
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

std::ostream &operator<<( std::ostream &ss, const Definition &def ) {
    if ( def.def_data ) {
        ss << def.def_data->name;
        if ( def.nb_uargs + def.nb_nargs ) {
            ss << '[';
            for(unsigned i=0;i<def.nb_uargs;++i)
                ss << def.args[i].type->to_string() << ( i+1 < def.nb_uargs+def.nb_nargs ? "," : "" );
            for(unsigned i=0;i<def.nb_nargs;++i)
                ss << def.args[i+def.nb_uargs].name << "=" << def.args[i+def.nb_uargs].type->to_string() << ( i+1 < def.nb_nargs ? "," : "" );
            ss << ']';
        }
    }
    else {
        ss << "uninitialised Def";
    }
    return ss;
}

const void *get_def_from_type( Thread *th, const void *tok, Variable *&sp, Definition &ret, Type *type ) {
    if ( not type )
        return NULL;
    //     Type *type = a->type;
    DefinitionData *dd = type->def_data;
    unsigned nb_args = dd->nb_args + dd->has_varargs();
    
    ret.cached_type = type;
    ret.def_data = dd; dd->inc_this_and_children_cpt_use();
    ret.nb_uargs = nb_args;
    ret.nb_nargs = 0;
    ret.args = NULL;
    ret.self.type = NULL;
    ret.max_pertinence = FLOAT64_MAX_VAL;
    ret.want_self = false;
    
    if ( nb_args ) {
        ret.args = (Variable *)malloc( sizeof(Variable) * nb_args );
        th->check_for_room_in_variable_stack( sp, nb_args * 2 );
        // partial instanciations
        for(unsigned i=0;i<nb_args;++i) {
            TypeVariable *tv = type->find_var( NULL, dd->args[i] );
            if ( tv ) {
                // partial instanciation
                ret.args[i].init( tv->v.type, 0 );
                //
                Variable *attr = assign_ref_on( sp++, ret.args + i );
                if ( not attr->replace_by_method_named( th, STRING_init_NUM ) ) // a.init
                    if ( not th->replace_var_by_ext_method_named( attr, STRING_init_NUM ) )
                        std::cerr << "init not found !!" << std::endl;
                assign_ref_on( sp++, &tv->v );
            }
            else
                std::cerr << "Very weird !! " << __FILE__ << " " << __LINE__ << std::endl;
        }        
        //
        if ( th->compile_mode )
            return exec_tok_apply( N<1>(), th, tok, /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/false, tok_end_def_block, sp, nb_args );
        return exec_tok_apply( N<0>(), th, tok, /*nb_uargs*/1, /*nb_nargs*/0, /*names*/NULL, /*expect_res*/false, tok_end_def_block, sp, nb_args );
    }
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}

bool are_radically_different( Definition &a, Definition &b ) {
    if ( a.def_data->name != b.def_data->name )
        return true;
    if ( a.cached_type and b.cached_type and a.cached_type != b.cached_type )
        return true;
    for(unsigned i=0;i<std::min(a.nb_uargs,b.nb_uargs);++i) {
        if ( a.args[i].type != b.args[i].type )
            return true;
        //
        if ( a.args[i].type == global_data.Int32 ) {
            if ( *reinterpret_cast<Int32 *>(a.args[i].data) != *reinterpret_cast<Int32 *>(b.args[i].data) )
                return true;
        } else if ( a.args[i].type == global_data.Def ) {
            if ( are_radically_different( *reinterpret_cast<Definition *>(a.args[i].data), *reinterpret_cast<Definition *>(b.args[i].data) ) )
                return true;
        } else // unnmanaged type -> don't know
            return false;
    }
    return false;
}


bool are_similar_for_sure( Definition &a, Definition &b ) {
    if ( a.cached_type and b.cached_type and a.cached_type == b.cached_type )
        return true;
    if ( a.def_data == b.def_data and a.nb_nargs==0 and b.nb_nargs==0 and a.nb_uargs==b.nb_uargs ) {
        for(unsigned i=0;i<b.nb_uargs;++i) {
            if ( a.args[i].type != b.args[i].type )
                return false;
            //
            if ( a.args[i].type == global_data.Int32 ) {
                if ( *reinterpret_cast<Int32 *>(a.args[i].data) != *reinterpret_cast<Int32 *>(b.args[i].data) )
                    return false;
            } else if ( a.args[i].type == global_data.Def ) {
                if ( not are_similar_for_sure( *reinterpret_cast<Definition *>(a.args[i].data), *reinterpret_cast<Definition *>(b.args[i].data) ) )
                    return false;
            } else // unnmanaged type
                return false;
        }
        return true;
    }
    return false;
}



