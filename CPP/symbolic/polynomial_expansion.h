#include "ex.h"
#include "op.h"
#include "thread.h"

// ------------------------------------------------------------------------------------------------------------
struct PolynomialExpansion {
    PolynomialExpansion( Thread *th, const void *tok, int order ) : th(th), tok(tok), zero(0), one(1), nb_elements(order+1) {}
    
    const Ex *complete_if_necessary( const Ex *r_, Op *ch );
    void exp_rec_add( Op *a );
    void exp_rec_mul( Op *a );
    void exp_rec_abs( Op *a );
    void exp_rec_ppa( Op *a );
    void exp_rec_pow( Op *a );
    void exp_rec_log( Op *a );
    void exp_rec_exp( Op *a );
    void exp_rec_sin( Op *a );
    void exp_rec_cos( Op *a );
    void exp_rec_tan( Op *a );
    void exp_rec_asin( Op *a );
    void exp_rec_acos( Op *a );
    void exp_rec_atan( Op *a );
    void exp_rec_atan2( Op *a);
    
    void exp_rec( Op *a );
    
    Thread *th;
    const void *tok;
    Ex zero, one;
    unsigned nb_elements;
    SplittedVec<Ex,2048,2048,true> tmp_vec;
};

// void polynomial_expansion( Thread *th, const void *tok, const SEX &expressions, const Ex &var, int order, SEX &res, Ex center );
// void polynomial_expansion( Thread *th, const void *tok, const VarArgs &expressions, const Ex &var, int order, VarArgs &res, Ex center );
