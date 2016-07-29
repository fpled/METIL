#ifndef REF_H
#define REF_H

template<class T> void inc_ref(T *val) {
    ++val->cpt_use;
}

template<class T> void dec_ref(T *val) {
    if ( not val->cpt_use )
        delete val;
    else
        --val->cpt_use;
}

#endif // REF_H
