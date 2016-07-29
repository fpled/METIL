#include "codewriteralt.h"
#include "definition.h"
#include "definitiondata.h"
#include "transientdata.h"
#include "globaldata.h"
#include "op.h"
#include <usual_strings.h>
#include "basic_string_manip.h"
#include "OpWithSeq.h"
#include "OpWithSeqGenerator.h"
#include <iostream>

void CodeWriterAlt::init( const char *a, Int32 ai, const char *b, Int32 bi, const char *c, Int32 ci, const char *d, Int32 di) {
    if ( ai )
        basic_type = strdupp0( a, ai );
    else
        basic_type = NULL;
    
    if ( bi )
        basic_integer_type = strdupp0( b, bi );
    else
        basic_integer_type = NULL;
        
    if ( ci )
        basic_simd = strdupp0( c, ci );
    else
        basic_simd = strdup( basic_type );
        
    if ( di )
        basic_integer_simd = strdupp0( d, di );
    else
        basic_integer_simd = strdup( basic_integer_type );
    
    //
    op_to_write.init();
    nb_to_write.init();
    has_init_methods = false;
    is_parallele = false;
    
    want_float = ( basic_type ? strcmp( basic_type, "float" ) == 0 : 0 );
}

void CodeWriterAlt::init( CodeWriterAlt &c ) {
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void CodeWriterAlt::reassign(  Thread *th, const void *tok, CodeWriterAlt &c ) {
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void CodeWriterAlt::make_cw_parallele( bool is_para ) {
    is_parallele = is_para;
}

void CodeWriterAlt::add_expr( Thread *th, const void *tok, Variable *str, const Ex &expr, Definition &b ) {
    const char *s = *reinterpret_cast<const char **>(str->data);
    Int32 si = *reinterpret_cast<const Int32 *>( reinterpret_cast<const char **>(str->data) + 1 );
    if ( b.def_data->name == STRING_init_NUM )
        has_init_methods = true;
        
    if ( expr.op->type == Op::NUMBER ) {
        NumberToWrite *ow = nb_to_write.new_elem();
        init_arithmetic( ow->n, expr.op->number_data()->val );
        ow->method = b.def_data->name;
        ow->name = strdupp0( s, si );
    } else {
        OpToWrite *ow = op_to_write.new_elem();
        ow->ex.init( expr );
        ow->method = b.def_data->name;
        ow->name = strdupp0( s, si );
    }
}

void CodeWriterAlt::add_expr( const Ex &expr, Nstring method, char *name ) {
    if ( expr.op->type == Op::NUMBER ) {
        NumberToWrite *ow = nb_to_write.new_elem();
        init_arithmetic( ow->n, expr.op->number_data()->val );
        ow->method = method;
        ow->name = strdup( name );
    } else {
        OpToWrite *ow = op_to_write.new_elem();
        ow->ex = expr;
        ow->method = method;
        ow->name = strdup( name );
    }
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------
std::string CodeWriterAlt::invariant( Thread *th, const void *tok, Int32 nb_spaces, const VarArgs &variables ) {
    return "";
}


// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

OpWithSeq *CodeWriterAlt::make_seq() {
    OpWithSeq::clear_number_set();
    OpWithSeq *seq = new OpWithSeq( OpWithSeq::SEQ );
    ++Op::current_op;
    
    for(unsigned i=0;i<op_to_write.size();++i) {
        seq->add_child( 
            new OpWithSeq( 
                op_to_write[i].method.v,
                op_to_write[i].name,
                make_OpWithSeq_rec( op_to_write[i].ex.op )
            )
        );
    }
    
    for(unsigned i=0;i<nb_to_write.size();++i) {
        seq->add_child( 
            new OpWithSeq( 
                nb_to_write[i].method.v,
                nb_to_write[i].name,
                OpWithSeq::new_number( nb_to_write[i].n )
            )
        );
    }
    
    //
    simplifications( seq );
    
    //
    return seq;
}

std::string CodeWriterAlt::to_string( Thread *th, const void *tok, Int32 nb_spaces) {
    if ( op_to_write.size() + nb_to_write.size() == 0 )
        return "";
    
    //
    OpWithSeq *seq = make_seq();
    
    //
    std::vector<OpWithSeq *> ordering;
    make_OpWithSeq_simple_ordering( seq, ordering );
    
    //
    ++OpWithSeq::current_id;
    update_nb_simd_terms_rec( seq );
    
    //
    OpWithSeqGenerator g( nb_spaces, basic_type, basic_integer_type, basic_simd, basic_integer_simd );
    for(unsigned i=0;i<ordering.size();++i)
        g.write_instr( ordering[i] , is_parallele);
    g.os << " /* " << g.nb_ops << " instructions */";
    
    //
    delete seq;
    return g.os.str();
}

std::string CodeWriterAlt::to_graphviz( Thread *th, const void *tok ) {
    std::ostringstream ss;
    OpWithSeq *seq = make_seq();
    //
    ++OpWithSeq::current_id;
    update_cost_access_rec( seq );
    //
    ++OpWithSeq::current_id;
    seq->graphviz_rec( ss );
    return ss.str();
}


void destroy( Thread *th, const void *tok, CodeWriterAlt &c ) {
    for(unsigned i=0;i<c.op_to_write.size();++i) {
        free( c.op_to_write[i].name );
        c.op_to_write[i].ex.destroy();
    }
    
    for(unsigned i=0;i<c.nb_to_write.size();++i) {
        free( c.nb_to_write[i].name );
        c.nb_to_write[i].n.destroy();
    }
    
    c.op_to_write.destroy();
    c.nb_to_write.destroy();
    if ( c.basic_type )
        free( c.basic_type );
    if ( c.basic_simd )
        free( c.basic_simd );
}
    


