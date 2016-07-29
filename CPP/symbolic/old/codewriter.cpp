#include "codewriter.h"
#include "definition.h"
#include "definitiondata.h"
#include "transientdata.h"
// #include "simplification_op.h"
#include "globaldata.h"
#include <usual_strings.h>
#include "basic_string_manip.h"
#include <iostream>

void CodeWriter::init( const char *s, Int32 si ) {
    if ( si )
        basic_type = strdupp0( s, si );
    else
        basic_type = NULL;
    
    //
    op_to_write.init();
    nb_to_write.init();
    already_calculated.init();
    free_registers.init();
    has_init_methods = false;    
    
}

void CodeWriter::init( CodeWriter &c ) {
//     basic_type = strdup( c.basic_type );
//     op_to_write.init( c.op_to_write );
//     already_calculated.init( c.already_calculated );
//     has_init_methods = c.has_init_methods;
//             
//     free_registers = new std::map<Type *,RegList>;
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void CodeWriter::reassign(  Thread *th, const void *tok, CodeWriter &c ) {
    std::cout << "TODO " << __FILE__ << " " << __LINE__ << std::endl;
}

void CodeWriter::add_expr( Thread *th, const void *tok, Variable *str, Op *expr, Definition &b ) {
    const char *s = *reinterpret_cast<const char **>(str->data);
    Int32 si = *reinterpret_cast<const Int32 *>( reinterpret_cast<const char **>(str->data) + 1 );
    if ( b.def_data->name == STRING_init_NUM )
        has_init_methods = true;
        
    if ( expr->type == Op::NUMBER ) {
        NumberToWrite *ow = nb_to_write.new_elem();
        init_arithmetic( ow->n, expr->number_data()->val );
        ow->method = b.def_data->name;
        ow->name = strdupp0( s, si );
    } else {
        OpToWrite *ow = op_to_write.new_elem();
        ow->op = &expr->inc_ref();
        ow->method = b.def_data->name;
        ow->name = strdupp0( s, si );
    }
}

void CodeWriter::add_expr( Op *expr, Nstring method, char *name ) {
    if ( expr->type == Op::NUMBER ) {
        NumberToWrite *ow = nb_to_write.new_elem();
        init_arithmetic( ow->n, expr->number_data()->val );
        ow->method = method;
        ow->name = strdup( name );
    } else {
        OpToWrite *ow = op_to_write.new_elem();
        ow->op = &expr->inc_ref();
        ow->method = method;
        ow->name = strdup( name );
    }
}


void disp_number_to_write( std::ostream &os, CodeWriter::NumberToWrite &nb_to_write, unsigned &cpt_op, bool &put_a_cr, Int32 nb_spaces, const char *basic_type ) {
    switch ( nb_to_write.method.v ) {
        if ( (cpt_op++ % 4) == 3 ) { os << '\n' << std::string(nb_spaces,' '); put_a_cr=true; } else { os << ' '; put_a_cr=false; }
        case STRING_add_NUM:       os << nb_to_write.name << " += " << nb_to_write.n << ".0;";  break;
        case STRING_sub_NUM:       os << nb_to_write.name << " -= " << nb_to_write.n << ".0;";  break;
        case STRING_mul_NUM:       os << nb_to_write.name << " *= " << nb_to_write.n << ".0;";  break;
        case STRING_div_NUM:       os << nb_to_write.name << " /= " << nb_to_write.n << ".0;";  break;
        case STRING_reassign_NUM:  os << nb_to_write.name << " = "  << nb_to_write.n << ".0;";  break;
        case STRING_init_NUM:      if ( basic_type ) os << nb_to_write.name << " = "  << nb_to_write.n << ".0;"; else os << nb_to_write.name << " := "  << nb_to_write.n << ".0;"; break;
        case STRING___print___NUM: os << "printf( \"" << nb_to_write.name << " -> %f\\n\", " << nb_to_write.n << ".0 );";  break;
        default:
            std::cerr << "UNKNOWN RES METHOD '" << nb_to_write.method << "'" << std::endl;
    }
}

void disp_res( std::ostream &os, Op *op, CodeWriter::ParentsOpAndNumReg *ponr, unsigned &cpt_op, bool &put_a_cr, Int32 nb_spaces, const char *basic_type ) {
    for(unsigned i=0;i<ponr->res.size();++i) {
        if ( (cpt_op++ % 4) == 3 ) { os << '\n' << std::string(nb_spaces,' '); put_a_cr=true; } else { os << ' '; put_a_cr=false; }
        switch ( ponr->res[i]->method.v ) {
            case STRING_add_NUM:       os << ponr->res[i]->name << " += " << "R_" << ponr->num_reg << ";";  break;
            case STRING_sub_NUM:       os << ponr->res[i]->name << " -= " << "R_" << ponr->num_reg << ";";  break;
            case STRING_mul_NUM:       os << ponr->res[i]->name << " *= " << "R_" << ponr->num_reg << ";";  break;
            case STRING_div_NUM:       os << ponr->res[i]->name << " /= " << "R_" << ponr->num_reg << ";";  break;
            case STRING_reassign_NUM:  os << ponr->res[i]->name << " = "  << "R_" << ponr->num_reg << ";";  break;
            case STRING_init_NUM:  
                if ( basic_type ) os << ponr->res[i]->name << " = "  << "R_" << ponr->num_reg << ";";
                else              os << ponr->res[i]->name << " := "  << "R_" << ponr->num_reg << ";";
                break;
            case STRING___print___NUM: os << "printf( \"" << ponr->res[i]->name << " -> %f\\n\", R_"  << ponr->num_reg << " );";  break;
            default:
                std::cerr << "UNKNOWN RES METHOD '" << ponr->res[i]->method << "'" << std::endl;
        }
    }
}


void disp_codewriter_number( std::ostream &os, const Rationnal &r ) {
    if ( r.is_integer() and r.positive_or_null() )
        os << r;
    else if ( r.den.n==0 and ( r.den.val==2 or r.den.val==4 or r.den.val==8 ) )
        os << double( r );
    else if ( r.den != Rationnal::BI(1) )
        os << "(" << r.num << ".0/" << r.den << ".0)";
    else
        os << "(" << r << ".0)";
}

void disp_codewriter( std::ostream &os, Op *c ) {
    if ( c->type == Op::NUMBER ) {
        disp_codewriter_number( os, c->number_data()->val );
    } else {
        CodeWriter::ParentsOpAndNumReg *poc = reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( c->additional_info );
        os << "R_" << poc->num_reg;
    }
}

void CodeWriter::write_code( std::ostream &os, SplittedVec<Op *,256,1024> &front, Int32 nb_spaces, bool &put_a_cr ) {
    // 
    unsigned cpt_op = 0;
    while ( front.size() ) {
        // selection of operation
        Op *op = front.pop_back();
        ParentsOpAndNumReg *ponr = reinterpret_cast<ParentsOpAndNumReg *>( op->additional_info );
        
        // prevision of freed registers
        if ( op->type > 0 ) {
            for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
                Op *c = op->func_data()->children[i];
                if ( c->type != Op::NUMBER ) {
                    ParentsOpAndNumReg *poc = reinterpret_cast<ParentsOpAndNumReg *>( c->additional_info );
                    if ( ++poc->used == poc->to_be_used )
                        free_registers.free_ones.push_back( poc->num_reg );
                }
            }
        }
        
        // get num_reg
        RegList &fr = free_registers;
        if ( (cpt_op % 4) == 0 ) for(Int32 i=0;i<nb_spaces;++i) os << ' ';
        if ( fr.free_ones.size() ) {
            ponr->num_reg = fr.free_ones.pop_back();
            os << "R_" << ponr->num_reg << " = ";
        }
        else {
            ponr->num_reg = fr.nb_declared_registers++;
            if ( basic_type ) 
                os << basic_type << " R_" << ponr->num_reg << " = ";
            else
                os << "R_" << ponr->num_reg << " := ";
        }
        
        // no variables in front
        if ( op->type == Op::SYMBOL ) {
            os << op->symbol_data()->cpp_name_str;
        } else if ( op->type == STRING_add_NUM ) { // a + b
            //             if ( is_a_sub( *op->func_data()->children[1] ) ) { // a + ( -b )
            //                 disp_codewriter( os, op->func_data()->children[0] );
            //                 os << "-";
            //                 disp_codewriter( os, op->func_data()->children[1]->func_data()->children[1] );
            //             } else if ( is_a_sub( *op->func_data()->children[0] ) ) { // (-a) + b
            //                 disp_codewriter( os, op->func_data()->children[1] );
            //                 os << "-";
            //                 disp_codewriter( os, op->func_data()->children[0]->func_data()->children[1] );
            //             } else if ( op->func_data()->children[0]->type == Op::NUMBER and op->func_data()->children[0]->number_data()->val.num.is_negative() ) { // (-2) + b
            //                 disp_codewriter( os, op->func_data()->children[1] );
            //                 os << "-";
            //                 disp_codewriter_number( os, - op->func_data()->children[0]->number_data()->val );
            //             } else { // a + b
            //             }
            disp_codewriter( os, op->func_data()->children[0] );
            os << "+";
            disp_codewriter( os, op->func_data()->children[1] );
        } else if ( op->type == STRING_mul_NUM ) { // a * b
            //             if ( is_a_inv( *op->func_data()->children[1] ) ) { // a * ( 1/b )
            //                 disp_codewriter( os, op->func_data()->children[0] );
            //                 os << "/";
            //                 disp_codewriter( os, op->func_data()->children[1]->func_data()->children[1] );
            //             } else if ( is_a_inv( *op->func_data()->children[0] ) ) { // (1/a) * b
            //                 disp_codewriter( os, op->func_data()->children[1] );
            //                 os << "/";
            //                 disp_codewriter( os, op->func_data()->children[0]->func_data()->children[1] );
            //             } else { // a + b
            //             }
            disp_codewriter( os, op->func_data()->children[0] );
            os << "*";
            disp_codewriter( os, op->func_data()->children[1] );
        } else if ( op->type == STRING_pow_NUM and op->func_data()->children[1]->type == Op::NUMBER ) { // a ^ b
            Rationnal b = op->func_data()->children[1]->number_data()->val;
            if ( b.is_minus_one() ) {
                os << "1/";
                disp_codewriter( os, op->func_data()->children[0] );
            } else if ( b.num.is_one() and b.den.is_two() ) {
                os << "sqrt(";
                disp_codewriter( os, op->func_data()->children[0] );
                os << ")";
            } else if ( b.den.is_one() ) {
                os << "pow(";
                disp_codewriter( os, op->func_data()->children[0] );
                os << "," << b << ")";
            } else {
                os << "pow(";
                disp_codewriter( os, op->func_data()->children[0] );
                os << "," << b << ".0)";
            }
            
        } else { // function
            os << Nstring( op->type ) << "(";
            for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i) {
                disp_codewriter( os, op->func_data()->children[i] );
                if ( i+1<Op::FuncData::max_nb_children and op->func_data()->children[i+1] ) os << ",";
            }
            os << ")";
        }
        os << ";";
        
        //
        disp_res( os, op, ponr, cpt_op, put_a_cr, nb_spaces, basic_type );
        
        if ( (cpt_op++ % 4) == 3 ) { os << '\n'; put_a_cr=true; } else { os << ' '; put_a_cr=false; }

        // parents (necessarily a function or a sumseq)
        if ( ponr->parents.size() ) {
            for(unsigned i=0;i<ponr->parents.size();++i)
                for(unsigned j=0;j<i;++j)
                    assert( ponr->parents[i] != ponr->parents[j] );
                    
            for(unsigned i=0;i<ponr->parents.size();++i) {
                Op *p = ponr->parents[i];
                assert( p != op );
                // if all children are stored in registers, we can push it in front
                if ( p->type > 0 ) {
                    for(unsigned j=0;;++j) {
                        if ( j==Op::FuncData::max_nb_children or p->func_data()->children[j]==NULL ) {
                            front.push_back( p );
                            break;
                        }
                        Op *c = p->func_data()->children[j];
                        if ( c->type != Op::NUMBER and reinterpret_cast<ParentsOpAndNumReg *>( c->additional_info )->num_reg < 0 )
                            break;
                    }
                }
            }
        }
        else if ( ponr->used >= 0 )// if there's no parents -> we can free the register
            free_registers.free_ones.push_back( ponr->num_reg );
    }
}

void get_parents( Op *expr, SplittedVec<CodeWriter::ParentsOpAndNumReg,256,1024,true> &parents ) {
    if ( expr->additional_info ) // already done ?
        return;
        
    // creation of a new parent list
    CodeWriter::ParentsOpAndNumReg *po = parents.new_elem();
    po->num_reg = -1;
    po->to_be_used = 0;
    expr->additional_info = reinterpret_cast<Op *>( po );
        
    if ( expr->type > 0 ) {
        // recursivity
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i)
            get_parents( expr->func_data()->children[i], parents );
        // register expr as parent of its children
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i) {
            CodeWriter::ParentsOpAndNumReg *ponr = reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( expr->func_data()->children[i]->additional_info );
            ponr->parents.push_back_unique( expr );
            ++ponr->to_be_used;
        }
    }
}

void get_front( Op *expr, SplittedVec<Op *,256,1024> &front ) {
    CodeWriter::ParentsOpAndNumReg *ponr = reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( expr->additional_info );
    if ( ponr->test_front ) // already done ?
        return;
    ponr->test_front = true;
    if ( ponr->num_reg >= 0 ) // has a register
        return;
    
    // creation of a new parent list
    if ( expr->type == Op::SYMBOL ) 
        front.push_back( expr );
    else if ( expr->type > 0 ) {
        // recursivity
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i)
            get_front( expr->func_data()->children[i], front );
        // if all children have a register (or are variables)
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i) {
            Op *c = expr->func_data()->children[i];
            if ( c->type != Op::NUMBER and reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( c->additional_info )->num_reg < 0 )
                return;
        }
        front.push_back( expr );
    }
}

void get_heavisides_rec( Op *expr, SplittedVec<Op *,32,256> &heavisides ) {
    if ( expr->additional_info ) // already done ?
        return;
    expr->additional_info = expr;
    
    if ( expr->type > 0 ) {
        if ( expr->type == STRING_heaviside_NUM or expr->type == STRING_eqz_NUM )
            heavisides.push_back_unique( expr );
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i) // recursivity
            get_heavisides_rec( expr->func_data()->children[i], heavisides );
    }
}

void add_to_used_rec( Op *expr, int val, SimpleVector<CodeWriter::AlreadyCalculated> &already_calculated ) {
    if ( expr->additional_info ) {
        reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( expr->additional_info )->used += val;
        CodeWriter::AlreadyCalculated *ac = already_calculated.new_elem();
        ac->op = expr;
    }
    else if ( expr->type > 0 )
        for(unsigned i=0;i<Op::FuncData::max_nb_children and expr->func_data()->children[i];++i) // recursivity
            add_to_used_rec( expr->func_data()->children[i], val, already_calculated );
}
    
void CodeWriter::write_particular_cases_with_cond_0_and_1( Thread *th, const void *tok, std::ostream &os, SplittedVec<Op *,32> &subs_values, Int32 nb_spaces, SimpleVector<AlreadyCalculated> &already_calculated ) {
    CodeWriter *cw = (CodeWriter *)malloc( sizeof(CodeWriter) );
    cw->init( basic_type, basic_type ? strlen(basic_type) : 0 );
    cw->free_registers = free_registers;
    cw->already_calculated = already_calculated;
    for(unsigned i=0;i<op_to_write.size();++i)
        cw->add_expr( subs_values[i], op_to_write[i].method, op_to_write[i].name );
    // write code
    os << cw->to_string( th, tok, nb_spaces + 4 );
    
    destroy( th, tok, *cw );
    free(cw);
}

void CodeWriter::write_code_with_cond_0_and_1( Thread *th, const void *tok, std::ostream &os, Op *cond, Int32 nb_spaces, SplittedVec<Op *,1024,4096> &of ) {
    // substitutions op_to_write -> op_to_write.subs( cond == 0 or 1 )
    SplittedVec<Op *,32> subs_values[2];
    for(unsigned val_cond=0;val_cond<2;++val_cond) {
        for(unsigned i=0;i<op_to_write.size();++i) { // leaves and parents
            op_to_write[i].op->clear_additional_info_rec();
            cond->additional_info = &Op::new_number( val_cond );
            of.push_back( cond->additional_info );
            subs_values[val_cond].push_back( &subs_rec( th, tok, *op_to_write[i].op, of ) );
        }
    }
    for(unsigned val_cond=0;val_cond<2;++val_cond)
        for(unsigned i=0;i<op_to_write.size();++i)
            subs_values[val_cond][i]->clear_additional_info_rec();
    //
    cond->clear_additional_info_rec();
    for(unsigned i=0;i<already_calculated.size();++i)
        already_calculated[i].op->clear_additional_info_rec();
            
    // cond preparation
    SplittedVec<ParentsOpAndNumReg,256,1024,true> parents;
    get_parents( cond, parents );
    for(unsigned i=0;i<already_calculated.size();++i) // already_calculated
        if ( already_calculated[i].op->additional_info )
            reinterpret_cast<ParentsOpAndNumReg *>( already_calculated[i].op->additional_info )->num_reg = already_calculated[i].num_reg;
    // front
    SplittedVec<Op *,256,1024> front;
    get_front( cond, front );
    
    // say that some calculations may be re-used
    SimpleVector<AlreadyCalculated> already_calculated;
    for(unsigned v=0;v<2;++v) for(unsigned i=0;i<op_to_write.size();++i) add_to_used_rec( subs_values[v][i], -1, already_calculated );
    // write
    bool put_a_cr = false;
    write_code( os, front, nb_spaces, put_a_cr );
    if ( not put_a_cr ) os << '\n'; for(int k=0;k<nb_spaces;++k) os << ' ';
    for(unsigned i=0;i<already_calculated.size();++i)
        already_calculated[i].num_reg = reinterpret_cast<CodeWriter::ParentsOpAndNumReg *>( already_calculated[i].op->additional_info )->num_reg;
    
    // write each particular cases using new CodeWriter
    os << "if ( " << "R_" << reinterpret_cast<ParentsOpAndNumReg *>( cond->additional_info )->num_reg << " ) {\n";
    write_particular_cases_with_cond_0_and_1( th, tok, os, subs_values[1], nb_spaces, already_calculated );
    for(int k=0;k<nb_spaces;++k) os << ' '; os << "} else {\n"; put_a_cr = true;
    write_particular_cases_with_cond_0_and_1( th, tok, os, subs_values[0], nb_spaces, already_calculated );
    for(int k=0;k<nb_spaces;++k) os << ' '; os << "}\n"; put_a_cr = true;

    // dec_ref
    // for(unsigned v=0;v<2;++v) for(unsigned i=0;i<op_to_write.size();++i) dec_ref( subs_values[v][i] );
}

std::string CodeWriter::to_string( Thread *th, const void *tok, Int32 nb_spaces ) {
    if ( op_to_write.size() + nb_to_write.size() == 0 )
        return "";

    std::ostringstream ss;
    // results with method init -> declaration
    if ( has_init_methods ) {
        if ( basic_type ) {
            for(int k=0;k<nb_spaces;++k) ss << ' ';
            std::string cr( "\n" ); cr += std::string( nb_spaces, ' ' );
            for(unsigned i=0;i<op_to_write.size();++i) if ( op_to_write[i].method == STRING_init_NUM ) ss << basic_type << " " << op_to_write[i].name << ";" << ( i%4==3 ? cr.c_str() : " " );
            for(unsigned i=0;i<nb_to_write.size();++i) if ( nb_to_write[i].method == STRING_init_NUM ) ss << basic_type << " " << nb_to_write[i].name << ";" << ( i%4==3 ? cr.c_str() : " " );
            ss << "\n";
        }
    }
    
    if ( op_to_write.size() ) {
        SplittedVec<Op *,1024,4096> of;
        // preparation
        for(unsigned i=0;i<op_to_write.size();++i)
            op_to_write[i].op->clear_additional_info_rec();
        for(unsigned i=0;i<already_calculated.size();++i)
            already_calculated[i].op->clear_additional_info_rec();
        // get heavisides
        SplittedVec<Op *,32,256> heavisides;
        //         for(unsigned i=0;i<op_to_write.size();++i)
        //             get_heavisides_rec( op_to_write[i].op, heavisides );
            
        if ( heavisides.size() ) {
            write_code_with_cond_0_and_1( th, tok, ss, heavisides[0], nb_spaces, of );
        }
        else { // no heavisides
            // preparation
            for(unsigned i=0;i<op_to_write.size();++i)
                op_to_write[i].op->clear_additional_info_rec();
            for(unsigned i=0;i<already_calculated.size();++i)
                already_calculated[i].op->clear_additional_info_rec();
                
            // symbols
            SplittedVec<ParentsOpAndNumReg,256,1024,true> parents;
            for(unsigned i=0;i<op_to_write.size();++i) // leaves and parents
                get_parents( op_to_write[i].op, parents );
            for(unsigned i=0;i<already_calculated.size();++i) // already_calculated
                if ( already_calculated[i].op->additional_info )
                    reinterpret_cast<ParentsOpAndNumReg *>( already_calculated[i].op->additional_info )->num_reg = already_calculated[i].num_reg;
            for(unsigned i=0;i<op_to_write.size();++i) { // results in...
                ParentsOpAndNumReg *ponr = reinterpret_cast<ParentsOpAndNumReg *>( op_to_write[i].op->additional_info );
                ponr->res.push_back( &op_to_write[i] );
                unsigned cpt_op = 0;
                if ( ponr->num_reg >= 0 ) {
                    for(int k=1;k<nb_spaces;++k) ss << ' '; 
                    bool put_a_cr;
                    disp_res( ss, op_to_write[i].op, ponr, cpt_op, put_a_cr, nb_spaces, basic_type );
                }
            }
            // front
            SplittedVec<Op *,256,1024> front;
            for(unsigned i=0;i<op_to_write.size();++i) // leaves and parents
                get_front( op_to_write[i].op, front );
    
            // write_code
            bool put_a_cr = false;
            write_code( ss, front, nb_spaces, put_a_cr );
            if ( put_a_cr == false ) ss << "\n";
        }
    
        // feee
        for(unsigned i=0;i<of.size();++i) dec_ref( of[i] );
        while ( op_to_write.size() ) { free( op_to_write.back().name ); dec_ref( op_to_write.pop_back().op ); }
    }
    
    // constant variables (no dependencies)
    if ( nb_to_write.size() ) {
        for(int k=0;k<nb_spaces;++k) ss << ' ';
        unsigned cpt_op = 0; bool put_a_cr = false;
        for(unsigned i=0;i<nb_to_write.size();++i) {
            disp_number_to_write( ss, nb_to_write[i], cpt_op, put_a_cr, nb_spaces, basic_type );
            if ( i%4==3 ) { ss << "\n"; for(int k=0;k<nb_spaces;++k) ss << ' '; }
            else          ss << " ";
        }
        ss << "\n";
    }
    
    return ss.str();
}

void destroy( Thread *th, const void *tok, CodeWriter &c ) {
    for(unsigned i=0;i<c.op_to_write.size();++i) {
        free( c.op_to_write[i].name );
        dec_ref( c.op_to_write[i].op );
    }
    
    for(unsigned i=0;i<c.nb_to_write.size();++i) {
        free( c.nb_to_write[i].name );
        c.nb_to_write[i].n.destroy();
    }
    
    c.op_to_write.destroy();
    c.nb_to_write.destroy();
    c.already_calculated.destroy();
    if ( c.basic_type )
        free( c.basic_type );
    c.free_registers.free_ones.destroy();
}
    


