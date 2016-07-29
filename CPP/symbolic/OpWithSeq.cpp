#include "config.h"
#include "OpWithSeq.h"
#include "basic_string_manip.h"
#include "usual_strings.h"
#include "op.h"
#include "nstring.h"
#include <set>
#include <map>
#include <algorithm>

unsigned OpWithSeq::current_id = 0;

static std::map<double,OpWithSeq *> numbers;
static OpWithSeq *number_M1 = NULL;

void OpWithSeq::clear_number_set() {
    numbers.clear();
}

OpWithSeq *OpWithSeq::new_number( double n ) {
    OpWithSeq *&r = numbers[ n ];
    if ( not r ) {
        r = new OpWithSeq( OpWithSeq::NUMBER );
        r->init_gen();
        r->num = n;
        r->den = 1;
    }
    return r;
}
OpWithSeq *OpWithSeq::new_number( const Rationnal &n ) {
    OpWithSeq *&r = numbers[ double( n ) ];
    if ( not r ) {
        r = new OpWithSeq( OpWithSeq::NUMBER );
        r->num = n.num;
        r->den = n.den;
    }
    return r;
}

OpWithSeq *OpWithSeq::new_number_M1() {
    if ( not number_M1 )
        number_M1 = new OpWithSeq( OpWithSeq::NUMBER_M1 );
    return number_M1;
}

OpWithSeq::OpWithSeq( int t ) : type( t ) {
    init_gen();
}

OpWithSeq::OpWithSeq( const char *cpp_name ) : type( SYMBOL ) {
    init_gen();
    cpp_name_str = strdup( cpp_name );
}

OpWithSeq::OpWithSeq( int method, const char *name, OpWithSeq *ch ) { // WRITE_...
    init_gen();
    switch ( method ) {
        case STRING_add_NUM :       type = WRITE_ADD;      break;
        case STRING_init_NUM:       type = WRITE_INIT;     break;
        case STRING_reassign_NUM:   type = WRITE_REASSIGN; break;
        case STRING___return___NUM: type = WRITE_RET;      break;
        default: assert(0);
    }
    cpp_name_str = strdup( name );
    add_child( ch );
}

OpWithSeq::OpWithSeq( int method, void *ptr_res, OpWithSeq *ch ) { // WRITE_...
    init_gen();
    switch ( method ) {
        case STRING_add_NUM :       type = WRITE_ADD;      break;
        case STRING_init_NUM:       type = WRITE_INIT;     break;
        case STRING_reassign_NUM:   type = WRITE_REASSIGN; break;
        case STRING___return___NUM: type = WRITE_RET;      break;
        default: assert(0);
    }
    this->ptr_res = ptr_res;
    add_child( ch );
}

void OpWithSeq::init_gen() {
    reg = -1;
    stack = -1;
    ordering = -1;
    id = 0;
    access_cost = 0;
    nb_simd_terms = 0;
    integer_type = 0;
    ptr_res = NULL;
    nb_times_used = 0;
    ptr_val = NULL;
}

OpWithSeq::~OpWithSeq() {
    for(unsigned i=0;i<children.size();++i) {
        OpWithSeq *ch = children[i];
        ch->parents.erase( std::find( ch->parents.begin(), ch->parents.end(), this ) );
        if ( ch->parents.size() == 0 and ch->type != NUMBER )
            delete ch;
    }
}

std::string OpWithSeq::asm_str() const {
    std::ostringstream ss;
    if ( reg >= 0 )
        ss << "xmm" << reg;
    else
        ss << "[ rsp + " << stack << " ]";
    return ss.str();
}

void OpWithSeq::add_child( OpWithSeq *c ) {
    children.push_back( c );
    c->parents.push_back( this );
}

OpWithSeq *make_OpWithSeq_rec( const Op *op ) {
    if ( op->op_id == Op::current_op )
        return reinterpret_cast<OpWithSeq *>( op->additional_info );
    op->op_id = Op::current_op;
    //
    if ( op->type == Op::SYMBOL ) {
        OpWithSeq *res = new OpWithSeq( op->symbol_data()->cpp_name_str );
        op->additional_info = reinterpret_cast<Op *>( res );
        res->access_cost = op->symbol_data()->access_cost;
        res->nb_simd_terms = op->symbol_data()->nb_simd_terms;
        res->integer_type = op->integer_type;
        return res;
    }
    //
    if ( op->type == Op::NUMBER ) {
        OpWithSeq *res = OpWithSeq::new_number( op->number_data()->val );
        op->additional_info = reinterpret_cast<Op *>( res );
        res->integer_type = op->integer_type;
        return res;
    }
    //
    if ( op->type == STRING_add_NUM ) {
        SplittedVec<const Op *,32> sum;
        get_child_not_of_type_add( op, sum );
        OpWithSeq *res = new OpWithSeq( op->type );
        for(unsigned i=0;i<sum.size();++i)
            res->add_child( make_OpWithSeq_rec( sum[i] ) );
        op->additional_info = reinterpret_cast<Op *>( res );
        res->integer_type = op->integer_type;
        return res;
    }
    //
    if ( op->type == STRING_mul_NUM ) {
        SplittedVec<const Op *,32> mul;
        get_child_not_of_type_mul( op, mul );
        bool want_neg = mul[0]->is_minus_one();
        //
        std::vector<OpWithSeq *> ch;
        for(unsigned i=want_neg;i<mul.size();++i) {
            const Op *c = mul[i];
            if ( c->type == STRING_pow_NUM and is_a_number( c->func_data()->children[1] ) and c->func_data()->children[1]->number_data()->val.is_integer() and c->func_data()->children[1]->number_data()->val.is_pos() ) {
                int a = int( c->func_data()->children[1]->number_data()->val );
                for(int i=0;i<abs(a);++i)
                    ch.push_back( make_OpWithSeq_rec( c->func_data()->children[0] ) );
            } else 
                ch.push_back( make_OpWithSeq_rec( mul[i] ) );
        }
        //
        OpWithSeq *res = new_add_or_mul( STRING_mul_NUM, ch );
        if ( want_neg )
            res = new_neg( res );
        op->additional_info = reinterpret_cast<Op *>( res );
        res->integer_type = op->integer_type;
        return res;
    }
    //
    if ( op->type == STRING_pow_NUM and is_a_number( op->func_data()->children[1] ) and op->func_data()->children[1]->number_data()->val.is_integer() ) {
        OpWithSeq *res;
        int a = int( op->func_data()->children[1]->number_data()->val );
        if ( a == -1 )
            res = make_OpWithSeq_rec( op->func_data()->children[0] );
        else {
            res = new OpWithSeq( STRING_mul_NUM );
            for(int i=0;i<abs(a);++i)
                res->add_child( make_OpWithSeq_rec( op->func_data()->children[0] ) );
        }
        //
        if ( a < 0 )
            res = new_inv( res );
        op->additional_info = reinterpret_cast<Op *>( res );
        res->integer_type = op->integer_type;
        return res;
    }

    //
    //     if ( op->type == STRING_select_symbolic_NUM ) {
    //         OpWithSeq *res = new OpWithSeq( op->type );
    //         res->add_child( make_OpWithSeq_rec( op->func_data()->children[1] ) );
    //         op->additional_info = reinterpret_cast<Op *>( res );
    //         res->integer_type = op->integer_type;
    //         return res;
    //     }

    //
    OpWithSeq *res = new OpWithSeq( op->type );
    for(unsigned i=0;i<Op::FuncData::max_nb_children and op->func_data()->children[i];++i)
        res->add_child( make_OpWithSeq_rec( op->func_data()->children[i] ) );
    op->additional_info = reinterpret_cast<Op *>( res );
    res->integer_type = op->integer_type;
    return res;
}

OpWithSeq *new_inv( OpWithSeq *ch ) {
    for(unsigned i=0;i<ch->parents.size();++i)
        if ( ch->parents[i]->type == OpWithSeq::INV and ch->parents[i]->children[0] == ch )
            return ch->parents[i];
    //
    OpWithSeq *res = new OpWithSeq( OpWithSeq::INV );
    res->add_child( ch );
    return res;
}

OpWithSeq *new_neg( OpWithSeq *ch ) {
    for(unsigned i=0;i<ch->parents.size();++i)
        if ( ch->parents[i]->type == OpWithSeq::NEG and ch->parents[i]->children[0] == ch )
            return ch->parents[i];
    //
    OpWithSeq *res = new OpWithSeq( OpWithSeq::NEG );
    res->add_child( ch );
    res->integer_type = ch->integer_type;
    return res;
}

bool same( const OpWithSeq *a, const OpWithSeq *b ) {
    return a == b;
    //     return ( a == b or ( a->type == OpWithSeq::NUMBER and b->type == OpWithSeq::NUMBER and a->val() == b->val() ) );
}

bool same_children( OpWithSeq *p, const std::vector<OpWithSeq *> &l ) {
    if ( l.size() != p->children.size() )
        return false;
    //
    unsigned nb_common = 0;
    static std::vector<bool> allow; if ( allow.size() < l.size() ) allow.resize( l.size() );
    for(unsigned i=0;i<l.size();++i) allow[i] = true;
    //
    for(unsigned i=0;i<l.size();++i) {
        for(unsigned j=0;j<l.size();++j) {
            if ( allow[j] and same( p->children[i], l[j] ) ) {
                ++nb_common;
                allow[j] = false;
                break;
            }
        }
    }
    return nb_common == l.size();
}

OpWithSeq *new_add_or_mul( int type, const std::vector<OpWithSeq *> &l ) {
    if ( l.size() == 0 )
        return NULL;
    if ( l.size() == 1 )
        return l[ 0 ];
    // look for similar...
    for(unsigned num_child=0;num_child<l.size();++num_child) {
        for(unsigned num_par=0;num_par<l[num_child]->parents.size();++num_par) {
            OpWithSeq *p = l[num_child]->parents[num_par];
            if ( p->type == type and same_children( p, l ) )
                return p;
        }
    }
    //
    OpWithSeq *res = new OpWithSeq( type );
    for(unsigned i=0;i<l.size();++i)
        res->add_child( l[i] );
    return res;
}

OpWithSeq *new_sub_or_div( int type, OpWithSeq *p, OpWithSeq *n ) {
    if ( not p ) return ( type == STRING_sub_NUM ? new_neg( n ) : new_inv( n ) );
    if ( not n ) return p;
    // look for similar...
    //
    OpWithSeq *res = new OpWithSeq( type );
    res->add_child( p );
    res->add_child( n );
    return res;
}

OpWithSeq *replace_op_by( OpWithSeq *o, OpWithSeq *n ) {
    if ( o == n )
        return o;
    //
    for(unsigned i=0;i<o->parents.size();++i) {
        OpWithSeq *p = o->parents[i];
        for(unsigned j=0;j<p->children.size();++j)
            if ( p->children[j] == o )
                p->children[j] = n;
        n->parents.push_back( p );
    }
    // o->parents.clear();
    delete o;
    return n;
}

void factorisation( OpWithSeq *op ) {
    //     std::map<OpWithSeq *op,int> nb_ops;
}

unsigned OpWithSeq::nb_instr() const {
    if ( type == NEG or type == INV )
        return 1;
    if ( type > 0 )
        return children.size() - 1 + ( children.size() == 1 );
    return 0;
}

void OpWithSeq::graphviz_rec( std::ostream &os ) const {
    if ( id == current_id ) // already outputted ?
        return;
    id = current_id;
    
    const char *colors[] = { "black", "blue", "red" };
    const char *color = colors[ ( access_cost > 1 ) + ( access_cost > 10 ) ];
    
    if ( type == SEQ )
        os << "    node" << this << " [label=\"SEQ\",color=\"" << color << "\"];\n";
    else if ( type == INV )
        os << "    node" << this << " [label=\"INV\",color=\"" << color << "\"];\n";
    else if ( type == NEG )
        os << "    node" << this << " [label=\"NEG\",color=\"" << color << "\"];\n";
    else if ( type == WRITE_ADD )
        os << "    node" << this << " [label=\"W+\",color=\"" << color << "\"];\n";
    else if ( type == WRITE_REASSIGN )
        os << "    node" << this << " [label=\"W\",color=\"" << color << "\"];\n";
    else if ( type == NUMBER )
        os << "    node" << this << " [label=\"" << num / den << "\",color=\"" << color << "\"];\n";
    else if ( type == SYMBOL )
        os << "    node" << this << " [label=\"" << cpp_name_str << "\",color=\"" << color << "\"];\n";
    else {
        os << "    node" << this << " [label=\"" << Nstring( type ) << "\",color=\"" << color << "\"];\n";
    }
    for(unsigned i=0;i<children.size();++i) {
        children[i]->graphviz_rec( os );
        os << "    node" << this << " -> node" << children[i] << ";\n";
    }
}

// a + ( -1 ) * b + c + ( - 1 ) * d -> ( a + c ) - ( b + d )
bool sep_neg_inv_rec_op( OpWithSeq *op, int base_type, int inv_type, int cmp_type ) {
    if ( op->type == base_type ) {
        unsigned nb_neg = 0;
        for(unsigned i=0;i<op->children.size();++i)
            nb_neg += op->children[i]->type == cmp_type;
        if ( nb_neg /*and nb_neg < op->children.size()*/ ) {
            std::vector<OpWithSeq *> pl, nl;
            for(unsigned i=0;i<op->children.size();++i) {
                if ( op->children[i]->type == cmp_type )
                    nl.push_back( op->children[i]->children[0] ); // - b
                else
                    pl.push_back( op->children[i] );
            }
            //
            OpWithSeq *p = new_add_or_mul( base_type, pl );
            OpWithSeq *n = new_add_or_mul( base_type, nl );
            OpWithSeq *r = new_sub_or_div( inv_type, p, n );
            //
            op = replace_op_by( op, r );
            return true;
        }
    }
    return false;
}

void sep_neg_inv_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    // recursivity
    for(unsigned i=0;i<op->children.size();++i)
        sep_neg_inv_rec( op->children[i] );
        
    if ( sep_neg_inv_rec_op( op, STRING_add_NUM, STRING_sub_NUM, OpWithSeq::NEG ) ) return;
    if ( sep_neg_inv_rec_op( op, STRING_mul_NUM, STRING_div_NUM, OpWithSeq::INV ) ) return;
}

void find_all_type_rec( OpWithSeq *op, int type, std::vector<OpWithSeq *> &l ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    // recursivity
    for(unsigned i=0;i<op->children.size();++i)
        find_all_type_rec( op->children[i], type, l );
    if ( op->type == type )
        l.push_back( op );
}

bool OpWithSeq::has_couple( const OpWithSeq *a, const OpWithSeq *b ) const {
    if ( a == b ) {
        unsigned count = 0;
        for(unsigned i=0;i<children.size();++i)
            count += same( children[i], a );
        return count >= 2;
    }
    //
    for(unsigned i=0;;++i) {
        if ( i == children.size()   ) return false;
        if ( same( children[i], a ) ) break;
    }
    for(unsigned i=0;;++i) {
        if ( i == children.size()   ) return false;
        if ( same( children[i], b ) ) break;
    }
    return true;
}

// struct CmpOpWithSeqPtr {
//     bool operator()( const std::pair<OpWithSeq *,OpWithSeq *> &a, const std::pair<OpWithSeq *,OpWithSeq *> &b ) const {
//         if ( same( a.first, b.first ) and same( a.second, b.second ) )
//             return false;
//         if ( same( a.second, b.first ) and same( a.first, b.second ) )
//             return false;
//         if ( a.first == b.first )
//             return a.second < b.second;
//         return a.first < b.first;
//     }
// };

struct CoupleOp {
    CoupleOp( OpWithSeq *a, OpWithSeq *b ) : a(a), b(b), cpt(0) {}
    
    static OpWithSeq *id_for_inf(OpWithSeq *c) {
        return c;
        //         if ( c->type == OpWithSeq::NUMBER )
        //             return c->val();
        //         return (int)c;
    }
    
    bool operator<( const CoupleOp &c ) const {
        if ( same( c.a, a ) and same( c.b, b ) )
            return false;
        if ( same( c.a, b ) and same( c.b, a ) )
            return false;
        if ( c.a == a )
            return id_for_inf(c.b) < id_for_inf(b);
        return id_for_inf(c.a) < id_for_inf(a);
    }
    bool operator==( const CoupleOp &c ) const {
        if ( same( c.a, a ) and same( c.b, b ) )
            return true;
        if ( same( c.a, b ) and same( c.b, a ) )
            return true;
        return false;
    }
    
    OpWithSeq *a;
    OpWithSeq *b;
    int cpt;
};

template<class TLO,class TLC>
void get_couple_lst( const TLO &l, TLC &couples ) {
    typedef std::map<CoupleOp,int> TS;
    // get all couples
    TS couple_set;
    for(unsigned k=0;k<l.size();++k)
        for(unsigned i=1;i<l[k]->children.size();++i)
            for(unsigned j=0;j<i;++j)
                couple_set[ CoupleOp( l[k]->children[i], l[k]->children[j] ) ]++;
    // copy those with cpt > 1
    couples.reserve( couple_set.size() );
    for( TS::iterator iter = couple_set.begin(); iter != couple_set.end(); ++iter ) {
        if ( iter->second > 1 ) {
            CoupleOp r = iter->first;
            r.cpt = iter->second;
            couples.push_back( r );
        }
    }
}

void inter_expr_factorisation( OpWithSeq *op, int type ) {
    typedef std::vector<CoupleOp   > TLC;
    typedef std::vector<OpWithSeq *> TLO;
    
    // nodes of type $type
    TLO l;
    ++OpWithSeq::current_id;
    find_all_type_rec( op, type, l );
    
    // set of couples
    TLC couples;
    get_couple_lst( l, couples );
    
    // remove nodes with only two children
    for(unsigned i=0;i<l.size();++i) {
        if ( l[i]->children.size() == 2 ) {
            l[i] = l.back();
            l.pop_back();
        }
    }
                
    // try each couples
    while ( true ) {
        int best_score = 0;
        CoupleOp *best_couple = NULL;
        for(unsigned i=0;i<couples.size();++i) {
            if ( best_score < couples[i].cpt ) {
                best_couple = &couples[i];
                best_score  = couples[i].cpt ;
            }
        }
        if ( best_score <= 1 )
            break;
            
        // intermediate expr
        TLO l2;
        l2.push_back( best_couple->a );
        l2.push_back( best_couple->b );
        OpWithSeq *tmp_reg = new_add_or_mul( type, l2 );
        
        // factorization
        TLC new_couples;
        for(unsigned i=0;i<l.size();++i) {
            OpWithSeq *c = l[i];
            //std::cerr << c->type << std::endl;
            //std::cerr << best_couple->a->type << std::endl;
            //std::cerr << best_couple->b->type << std::endl;
            if ( c->children.size() > 2 and c->has_couple( best_couple->a, best_couple->b ) ) {
                TLO remaining; bool a_removed = false, b_removed = false;
                for(unsigned j=0;j<c->children.size();++j) {
                    if ( same( c->children[j], best_couple->a ) and not a_removed ) { a_removed = true; continue; }
                    if ( same( c->children[j], best_couple->b ) and not b_removed ) { b_removed = true; continue; }
                    remaining.push_back( c->children[j] );
                }
                
                // new couples
                for(unsigned j=0;j<remaining.size();++j) {
                    CoupleOp co( remaining[j], tmp_reg );
                    TLC::iterator iter = std::find( new_couples.begin(), new_couples.end(), co );
                    if ( iter != new_couples.end() ) {
                        ++iter->cpt;
                    } else {
                        co.cpt = 1;
                        new_couples.push_back( co );
                    }
                }
                
                // new operand
                remaining.push_back( tmp_reg );
                    
                OpWithSeq *res = replace_op_by( c, new_add_or_mul( type, remaining ) );
                
                // register res, remove c from list of op to visit
                if ( res->type == type and res->children.size() > 2 and std::find( l.begin(), l.end(), res ) == l.end() ) {
                    l[i--] = res;
                } else {
                    l[i--] = l.back();
                    l.pop_back();
                }
            }
        }
        
        // remove couple
        *best_couple = couples.back();
        couples.pop_back();
        for(unsigned i=0;i<new_couples.size();++i)
            couples.push_back( new_couples[i] );
    }
}

void sep_neg_inv( OpWithSeq *op ) {
    ++OpWithSeq::current_id;
    sep_neg_inv_rec( op );
}

// a/b + sin( c/b ) -> a*(1/b) + sin( c*(1/b) )
void several_div_give_mul_inv( OpWithSeq *op ) {
    ++OpWithSeq::current_id;
    std::vector<OpWithSeq *> l;
    find_all_type_rec( op, STRING_div_NUM, l );
    std::set<OpWithSeq *> cs;
    for(unsigned i=0;i<l.size();++i)
        cs.insert( l[i]->children[1] );
    //
    for( std::set<OpWithSeq *>::iterator pc = cs.begin(); pc != cs.end(); ++pc ) {
        OpWithSeq *c = *pc;
        unsigned nb_div_parents = 0;
        for(unsigned i=0;i<c->parents.size();++i)
            nb_div_parents += c->parents[i]->type == STRING_div_NUM;
        //
        if ( nb_div_parents > 1 ) {
            std::vector<OpWithSeq *> ch_mul;
            ch_mul.push_back( NULL );
            ch_mul.push_back( new_inv( c ) );
            std::vector<OpWithSeq *> c_parents = c->parents;
            for(unsigned i=0;i<c_parents.size();++i) {
                OpWithSeq *p = c_parents[i];
                if ( p->type == STRING_div_NUM ) {
                    ch_mul[0] = p->children[0];
                    replace_op_by( p, new_add_or_mul( STRING_mul_NUM, ch_mul ) );
                }
            }
        }
    }
    //
}

void simplifications( OpWithSeq *op ) {
    sep_neg_inv( op );
    inter_expr_factorisation( op, STRING_mul_NUM );
    inter_expr_factorisation( op, STRING_add_NUM );
    several_div_give_mul_inv( op );
}

void OpWithSeq::remove_parent( OpWithSeq *parent ) {
    parents.erase( std::find( parents.begin(), parents.end(), parent ) );
}

void make_binary_ops_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    
    // 
    while ( op->children.size() > 2 ) {
        OpWithSeq *c0 = op->children[ op->children.size() - 2 ];
        OpWithSeq *c1 = op->children[ op->children.size() - 1 ];
        c0->remove_parent( op );
        c1->remove_parent( op );
        op->children.pop_back();
        op->children.pop_back();
        //
        OpWithSeq *n = new OpWithSeq( op->type );
        n->add_child( c0 );
        n->add_child( c1 );
        op->add_child( n );
    }
    
    // recursivity
    for(unsigned i=0;i<op->children.size();++i)
        make_binary_ops_rec( op->children[i] );
}

void make_binary_ops( OpWithSeq *op ) {
    ++OpWithSeq::current_id;
    make_binary_ops_rec( op );
}

OpWithSeq *new_pow( OpWithSeq *m, double e ) {
    if ( e == 1 )
        return m;
    //
    if ( int( e ) == e ) {
        OpWithSeq *res = new OpWithSeq( STRING_mul_NUM );
        for(int i=0;i<abs( e );++i)
            res->add_child( m );
        if ( e < 0 )
            res = new_inv( res );
        return res;
    }
    //
    OpWithSeq *res = new OpWithSeq( STRING_pow_NUM );
    res->add_child( m );
    res->add_child( OpWithSeq::new_number( e ) );
    return res;
}

void asm_simplifications_prep_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    
    if ( op->type == STRING_pow_NUM and op->children[1]->type == OpWithSeq::NUMBER ) {
        double v = op->children[1]->val();
        int n = int( 2 * v );
        if ( n == 2 * v and ( n & 1 ) ) { // ^ n/2
            OpWithSeq *ch = new_pow( op->children[0], 2 * v );
            //
            op->type = STRING_sqrt_NUM;
            op->children[0]->remove_parent( op );
            op->children[1]->remove_parent( op );
            op->children.clear();
            op->add_child( ch );
        }
    }
    
    // recursivity
    for(unsigned i=0;i<op->children.size();++i)
        asm_simplifications_prep_rec( op->children[i] );
}

void asm_simplifications_prep( OpWithSeq *op ) {
    ++OpWithSeq::current_id;
    asm_simplifications_prep_rec( op );
}

void asm_simplifications_post_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    
    if ( op->type == OpWithSeq::INV ) {
        op->type = STRING_div_NUM;
        op->add_child( OpWithSeq::new_number( Rationnal( 1 ) ) );
        std::swap( op->children[0], op->children[1] );
    } else if ( op->type == OpWithSeq::NEG ) {
        op->type = STRING_sub_NUM;
        op->add_child( OpWithSeq::new_number( Rationnal( 0 ) ) );
        std::swap( op->children[0], op->children[1] );
    } else if ( op->type == STRING_heaviside_NUM or op->type == STRING_eqz_NUM ) {
        op->add_child( OpWithSeq::new_number( Rationnal( 0 ) ) );
        op->add_child( OpWithSeq::new_number( Rationnal( 1 ) ) );
    } else if ( op->type == STRING_pos_part_NUM ) {
        op->add_child( OpWithSeq::new_number( Rationnal( 0 ) ) );
    } else if ( op->type == STRING_abs_NUM ) {
        op->add_child( OpWithSeq::new_number_M1() );
    }
    
    // recursivity
    for(unsigned i=0;i<op->children.size();++i)
        asm_simplifications_post_rec( op->children[i] );
}

void asm_simplifications_post( OpWithSeq *op ) {
    ++OpWithSeq::current_id;
    asm_simplifications_post_rec( op );
}

void update_cost_access_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    // recursivity
    for(unsigned i=0;i<op->children.size();++i) {
        update_cost_access_rec( op->children[i] );
        op->access_cost = std::max( op->access_cost, op->children[i]->access_cost );
    }
}

void update_nb_simd_terms_rec( OpWithSeq *op ) {
    if ( op->id == OpWithSeq::current_id )
        return;
    op->id = OpWithSeq::current_id;
    // recursivity
    for(unsigned i=0;i<op->children.size();++i) {
        update_nb_simd_terms_rec( op->children[i] );
        op->nb_simd_terms = std::max( op->nb_simd_terms, op->children[i]->nb_simd_terms );
    }
}

void make_OpWithSeq_simple_ordering( OpWithSeq *seq, std::vector<OpWithSeq *> &ordering, bool want_asm ) {
    if ( seq->ordering >= 0 )
        return;
    if ( seq->type == STRING_select_symbolic_NUM ) {
        if ( not want_asm )
           make_OpWithSeq_simple_ordering( seq->children[1], ordering, want_asm );
    } else {
        for(unsigned i=0;i<seq->children.size();++i)
            make_OpWithSeq_simple_ordering( seq->children[i], ordering, want_asm );
    }
    seq->ordering = ordering.size();
    ordering.push_back( seq );
}

std::ostream &operator<<( std::ostream &os, const OpWithSeq &op ) {
    if ( op.type == OpWithSeq::SEQ ) {
        for(unsigned i=0;i<op.children.size();++i)
            os << ( i ? ", " : "" ) << *op.children[i];
    }
    else if ( op.type == OpWithSeq::INV ) {
        os << "inv(" << *op.children[0] << ")";
    }
    else if ( op.type == OpWithSeq::WRITE_ADD ) {
        for(unsigned i=0;i<op.children.size();++i)
            os << "\n -> " << *op.children[i];
    }
    else if ( op.type == OpWithSeq::NUMBER ) {
        os << op.num / op.den;
    }
    else if ( op.type == OpWithSeq::SYMBOL ) {
        os << op.cpp_name_str;
    }
    else {
        os << Nstring( op.type );
        os << "(";
        for(unsigned i=0;i<op.children.size();++i)
            os << ( i ? "," : "" ) << *op.children[i];
        os << ")";
    }
    return os;
}

