#include "functionnal.h"
#include "thread.h"
#include "varargs.h"
#include "globaldata.h"
#include <sstream>
    
void Functionnal::init() {
    var.type = NULL;
    variables.init();
    attribute.v = 0;;
    get_var_nb = NOT_A_PREDEFINED_FUNCTIONNAL;
    nb_named = 0;
}

void Functionnal::init( Functionnal *f, bool inc_refs ) {
    if ( f->var.type )
        assign_ref_on( &var, &f->var );
    else
        var.type = NULL;
    variables.init();
    attribute.v = 0;;
    for(unsigned i=0;i<f->variables.size();++i) make_copy_with_name( variables.new_elem(), &f->variables[i] );
    if ( inc_refs )
        for(unsigned i=0;i<f->variables.size();++i) variables[i].inc_ref();
    attribute = f->attribute;
    get_var_nb = f->get_var_nb;
    nb_named = f->nb_named;
}

void Functionnal::copy_data_from( Functionnal *f ) {
    variables.destroy();
    init( f );
}

const void *destroy( Functionnal &d, Thread *th, Variable * &sp ) {
    for(unsigned i=0;i<d.variables.size();++i)
        th->push_var_if_to_be_destroyed( &d.variables[i], sp );
    if ( d.var.type )
        th->push_var_if_to_be_destroyed( &d.var, sp );
        
    d.variables.destroy();
        
    th->set_current_sourcefile( NULL );
    return tok_end_def_block;
}


std::ostream &operator<<( std::ostream &os, const Functionnal &d ) {
    if ( d.get_var_nb == Functionnal::SELF )
        os << "_self";
    else if ( d.get_var_nb >= 0 )
        os << "_" << d.get_var_nb;
    else if ( d.attribute.v )
        os << "." << d.attribute;
    else if ( d.variables.size() ) {
        os << "(";
        for(unsigned i=0;i<d.variables.size();++i)
            os << d.variables[i].type->name << ",";
        os << ")";
    }
    else
        os << "VoidFunctionnal";
    return os;
}

void extract_arg_defined_in_functionnal( Thread *th, const void *tok, Variable *return_var, Variable *self_var, Functionnal *f, VarArgs *va ) {
    assert( f->get_var_nb != Functionnal::NOT_A_PREDEFINED_FUNCTIONNAL );
    
    if ( f->get_var_nb == Functionnal::SELF ) {
        if ( self_var )
            assign_ref_on( return_var, self_var );
        else {
            th->add_error( "there's no self var in current context.", tok );
            assign_error_var( return_var );
        }
    }
    else {
        if ( f->get_var_nb >= 0 ) {
            if ( (unsigned)f->get_var_nb >= va->variables.size() ) {
                std::ostringstream ss; ss << "not enough arguments (we want argument nb " << f->get_var_nb << " and we have only " << va->variables.size() << ").";
                th->add_error(ss.str(),tok);
                assign_error_var( return_var );
            }
            else
                assign_ref_on( return_var, &va->variables[ f->get_var_nb ] );
        }
        else { // a named arg
            for(unsigned i=0;i<va->nb_nargs();++i) {
                if ( va->name(i) == Functionnal::SELF - f->get_var_nb ) {
                    assign_ref_on( return_var, va->narg(i) );
                    return;
                }
            }
            th->add_error( "there's no variable named "+std::string(Nstring(Functionnal::SELF - f->get_var_nb))+" in arguments.", tok );
            assign_error_var( return_var );
        }
    }
}

void extract_var_defined_in_functionnal( Variable *return_var, Functionnal *f ) {
    return_var->init( global_data.InternalVariable, 0 );
    Variable *v = reinterpret_cast<Variable *>( return_var->data );
    assign_ref_on( v, &f->var );
}

void extract_var_nb_defined_in_functionnal( Variable *return_var, Functionnal *f, Int32 n ) {
    return_var->init( global_data.InternalVariable, 0 );
    Variable *v = reinterpret_cast<Variable *>( return_var->data );
    assign_ref_on( v, &f->variables[n] );
}



