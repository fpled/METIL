#include "AsmWriter.h"
#include "definition.h"
#include "definitiondata.h"
#include "transientdata.h"
#include "globaldata.h"
#include "op.h"
#include <usual_strings.h>
#include "basic_string_manip.h"
#include "OpWithSeq.h"
#include "OpWithSeqAsmGenerator.h"
#include <iostream>

void AsmWriter::init(  ) {
    op_to_write.init();
    nb_to_write.init();
    associations.init();
    has_init_methods = false;
    want_float = false;
}

void AsmWriter::init( AsmWriter &c ) {
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void AsmWriter::reassign(  Thread *th, const void *tok, AsmWriter &c ) {
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void AsmWriter::add_expr( Thread *th, const void *tok, void *ptr_res, const Ex &expr, Definition &b ) {
    if ( b.def_data->name == STRING_init_NUM )
        has_init_methods = true;
        
    if ( expr.op->type == Op::NUMBER ) {
        NumberToWrite *ow = nb_to_write.new_elem();
        init_arithmetic( ow->n, expr.op->number_data()->val );
        ow->method = b.def_data->name;
        ow->ptr_res = ptr_res;
    } else {
        OpToWrite *ow = op_to_write.new_elem();
        ow->ex.init( expr );
        ow->method = b.def_data->name;
        ow->ptr_res = ptr_res;
    }
}

void AsmWriter::add_association( Thread *th, const void *tok, const Ex &expr, void *ptr_val, Definition &b ) {
    Association *as = associations.new_elem();
    as->ex.init( expr );
    as->ptr_val = ptr_val;
    as->nstring_type = b.def_data->name.v;
}

// --------------------------------------------------------------------------------------------------------------------------------------------------------------------

OpWithSeq *AsmWriter::make_seq() {
    OpWithSeq::clear_number_set();
    OpWithSeq *seq = new OpWithSeq( OpWithSeq::SEQ );
    ++Op::current_op;
    
    for(unsigned i=0;i<op_to_write.size();++i) {
        seq->add_child( 
            new OpWithSeq( 
                op_to_write[i].method.v,
                op_to_write[i].ptr_res,
                make_OpWithSeq_rec( op_to_write[i].ex.op )
            )
        );
    }
    
    for(unsigned i=0;i<nb_to_write.size();++i) {
        seq->add_child( 
            new OpWithSeq( 
                nb_to_write[i].method.v,
                nb_to_write[i].ptr_res,
                OpWithSeq::new_number( nb_to_write[i].n )
            )
        );
    }
    
    //
    for(unsigned i=0;i<associations.size();++i) {
        Op *op = associations[i].ex.op;
        if ( op->op_id == Op::current_op ) {
            reinterpret_cast<OpWithSeq *>( op->additional_info )->ptr_val      = associations[i].ptr_val;
            reinterpret_cast<OpWithSeq *>( op->additional_info )->nstring_type = associations[i].nstring_type;
        }
    }
    
    //
    asm_simplifications_prep( seq );
    simplifications( seq );
    make_binary_ops( seq );
    asm_simplifications_post( seq );
    
    //
    return seq;
}

void *AsmWriter::to_code( Thread *th, const void *tok ) {
    //
    OpWithSeq *seq = make_seq();
    
    //
    std::vector<OpWithSeq *> ordering;
    make_OpWithSeq_simple_ordering( seq, ordering, true );
    
    //
    ++OpWithSeq::current_id;
    update_nb_simd_terms_rec( seq );
    
    //
    OpWithSeqAsmGenerator g( OpWithSeqAsmGenerator::X86_64 );
    g.push_header();
    for(unsigned i=0;i<ordering.size();++i)
        g.write_instr( th, tok, ordering[i] );
    g.push_footer();
    
    //
    delete seq;
    
    //
    return g.add_ret_and_make_code_as_new_contiguous_data();
}

std::string AsmWriter::to_graphviz( Thread *th, const void *tok ) {
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


void destroy( Thread *th, const void *tok, AsmWriter &c ) {
    for(unsigned i=0;i<c.op_to_write.size();++i)
        c.op_to_write[i].ex.destroy();
    
    for(unsigned i=0;i<c.nb_to_write.size();++i)
        c.nb_to_write[i].n.destroy();
    
    c.op_to_write.destroy();
    c.nb_to_write.destroy();
    c.associations.destroy();
}
    


