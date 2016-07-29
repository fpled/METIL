#ifndef OPWITHSEQGENERATOR_H
#define OPWITHSEQGENERATOR_H

struct OpWithSeqGenerator {
    OpWithSeqGenerator( unsigned nb_sp, const char *T, const char *TI, const char *S, const char *SI ) : beg_line( nb_sp, ' ' ), T( T ), TI( TI ), S( S ), SI( SI ) {
        num_instr = 0;
        nb_ops = 0;
        nb_reg = 0;
    }
    
    //     std::string R( OpWithSeq *op ) {
    //         std::ostringstream ss;
    //         //         if ( op->type == OpWithSeq::NUMBER ) {
    //         //             if ( strcmp(T,"Ex") == 0 ) {
    //         //                 bool neg = op->val() < 0;
    //         //                 if ( op->den != 1 )
    //         //                     ss << "Rationnal(" << op->num << "," << op->den << ")";
    //         //                 else
    //         //                     ss << ( neg ? "(" : "" ) << op->val() << ( neg ? ")" : "" );
    //         //             }
    //         //             else {
    //         //                 bool neg = op->val() < 0, den = ( op->den != 1 );
    //         //                 if ( den )
    //         //                     ss << ( neg ? "(" : "" ) << op->num << "/" << T << "(" << op->den << ")" << ( neg ? ")" : "" );
    //         //                 else
    //         //                     ss << ( neg ? "(" : "" ) << op->num << ( neg ? ")" : "" );
    //         //             }
    //         //         }
    //         //         else
    //         ss << "R" << op->reg;
    //         return ss.str();
    //     }
    
    const char *ty( OpWithSeq *op ) const {
        if ( op->integer_type ) {
            if ( op->nb_simd_terms > 1 )
                return SI;
            return TI;
        }
        if ( op->nb_simd_terms > 1 )
            return S;
        return T;
    }
    
    void write_instr( OpWithSeq *op , bool is_parallele) {
        if ( op->type == OpWithSeq::SEQ )
            return;
        //if ( op->type == OpWithSeq::NUMBER )
        //    return;
            
        //
        if ( num_instr % 6 == 0 ) {
            if ( num_instr )
                os << "\n";
            os << beg_line;
        }
        ++num_instr;
        nb_ops += op->nb_instr();
        
        //
        if ( op->type == OpWithSeq::WRITE_ADD  and not ( is_parallele )   ) 
	    { os     << op->cpp_name_str << " += R" << op->children[0]->reg << "; "; return; }
        if ( op->type == OpWithSeq::WRITE_ADD  and ( is_parallele )   ) 
	    { os     <<      "pthread_mutex_lock(mutex);"     << op->cpp_name_str << " += R" << op->children[0]->reg << "; pthread_mutex_unlock(mutex);"; return; }	    
        if ( op->type == OpWithSeq::WRITE_REASSIGN ) { os                               << op->cpp_name_str <<  " = R" << op->children[0]->reg << "; "; return; }
        if ( op->type == OpWithSeq::WRITE_RET      ) { os << "return R"                                                << op->children[0]->reg << "; "; return; }
        if ( op->type == OpWithSeq::WRITE_INIT     ) { os << ty(op->children[0]) << " " << op->cpp_name_str <<  " = R" << op->children[0]->reg << "; "; return; }
        
        // we will need a register
        op->reg = find_reg();
        os << ty( op ) << " R" << op->reg << " = ";
        
        //
        if ( op->type == OpWithSeq::NUMBER ) {
            double n = op->num / op->den;
            if ( numbers.count( n ) )
                os << "R" << numbers[ n ];
            else {
                os.precision( 16 );
                os << n;
                numbers[ n ] = op->reg;
            }
        } else if ( op->type == OpWithSeq::SYMBOL ) {
            os << op->cpp_name_str;
        } else if ( op->type == OpWithSeq::INV ) {
            os << "1/R" << op->children[0]->reg;
        } else if ( op->type == OpWithSeq::NEG ) {
            os << "-R" << op->children[0]->reg;
        } else if ( op->type == STRING_add_NUM ) {
            for(unsigned i=0;i<op->children.size();++i)
                os << ( i ? "+R" : "R" ) << op->children[i]->reg;
        } else if ( op->type == STRING_sub_NUM ) {
            os << "R" << op->children[0]->reg << "-R" << op->children[1]->reg;
        } else if ( op->type == STRING_div_NUM ) {
            os << "R" << op->children[0]->reg << "/R" << op->children[1]->reg;
        } else if ( op->type == STRING_mul_NUM ) {
            for(unsigned i=0;i<op->children.size();++i)
                os << ( i ? "*R" : "R" ) << op->children[i]->reg;
        } else if ( op->type == STRING_select_symbolic_NUM ) {
            if ( op->children[1]->nb_simd_terms > 1 ) {
                os << S << "(";
                for(int i=0;i<op->children[1]->nb_simd_terms;++i)
                    os << (i?",":"") << op->children[0]->cpp_name_str << "[R" << op->children[1]->reg << "[" << i << "]]";
                os << ")";
            } else
                os << op->children[0]->cpp_name_str << "[R" << op->children[1]->reg << "]";
        } else {
            os << Nstring( op->type );
            os << "(";
            for(unsigned i=0;i<op->children.size();++i)
                os << ( i ? ",R" : "R" ) << op->children[i]->reg;
            os << ")";
        }
        os << "; ";
    }
    
    int find_reg() { return nb_reg++; }
    
    std::ostringstream os;
    std::string beg_line;
    const char *T, *TI, *S, *SI;
    unsigned num_instr, nb_ops;
    int nb_reg;
    std::map<double,int> numbers;
};

#endif // OPWITHSEQGENERATOR_H

