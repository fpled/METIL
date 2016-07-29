#ifndef SPLITTED_SET_H
#define SPLITTED_SET_H

#include "splittedvec.h"

/**
 */
template<class T,unsigned n>
class SplittedSet {
public:
    void insert(T val) {
        unsigned ds = data.size();
        for(unsigned i=0;i<ds;++i) {
            T d = data[i];
            if ( d == val )
                return;
            if ( data[i] < val ) {
                data.new_elem();
                for(unsigned j=ds;j>i;--j)
                    data[j] = data[j-1];
                data[i] = val;
                return;
            } 
        }
        data.push_back( val );
    }
    
    T operator[](unsigned i) const { return data[i]; }
    
    unsigned size() const { return data.size(); }
    
private:
    SplittedVec<T,n,n,true> data;
};

#endif
