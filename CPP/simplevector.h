//
// C++ Interface: sharedvector
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SIMPLE_VECTOR_H
#define SIMPLE_VECTOR_H


/**
    similar as std::vector but support apply(...), find(...) and is_a_ref_of 
 */
template<class T_>
class SimpleVector {
public:
    typedef T_ T;
    typedef T *iterator;
    typedef const T *const_iterator;
    
    void init() { s=0; r=0; vec=(T *)0; }
    SimpleVector() { init(); }
    SimpleVector(unsigned size) { s=size; r=size; vec=new T[size]; }
    SimpleVector(unsigned size,const T &val) {
        s=size; r=size; vec=new T[size];
        for(unsigned i=0;i<size;++i) vec[i] = val;
    }
    SimpleVector(const SimpleVector &v) {
        r = 0; s = 0; vec = (T *)0;
        resize(v.size());
        for(unsigned i=0;i<v.size();++i)
            vec[i] = v[i];
    }
    SimpleVector &operator=(const SimpleVector &v) {
        clear(); r = 0; vec = (T *)0;
        resize(v.size());
        for(unsigned i=0;i<v.size();++i)
            vec[i] = v[i];
        return *this;
    }
    void destroy() { delete [] vec; }
    ~SimpleVector() { destroy(); }
    
    unsigned size() const { return s; }
    unsigned reserved() const { return r; }
    
    void resize(unsigned size) {
        if ( size==0 ) { r=0; vec=(T *)0; }
        else if ( size > r ) reserve(size);
        s = size;
    }
    void resize(unsigned size,const T &val) {
        unsigned os = s;
        resize(size);
        for(unsigned i=os;i<size;++i) vec[i] = val;
    }
    void pop_back() { --s; }
    void reserve(unsigned size) {
        if ( size == r ) return;
        assert( size >= s );
        T *nvec = new T[size];
        for(unsigned i=0;i<(s>size?size:s);++i)
            nvec[i] = vec[i];
        delete [] vec;
        vec = nvec;
        r = size;
    }
    void clear() {
        delete [] vec;
        r = 0;
        vec = (T *)0;
        s = 0;
    }
    void push_back(const T &val) {
        if ( r==0 ) reserve(1);
        else if ( s==r ) reserve(2*r);
        vec[s++] = val;
    }
    void push_back_unique(const T &val) {
        for(int i=0;i<s;++i)
            if ( vec[i] == val )
                return;
        push_back( val );
    }
    /// assumes that room for val is already reserved
    void push_back_in_reserved(const T &val) { vec[s++] = val; }
    ///
    T *new_elem() {
        if ( r==0 ) reserve(1);
        else if ( s==r ) reserve(2*r);
        return &vec[s++];
    }
    ///
    void erase_first(const T &val) {
        for(unsigned i=0;i<s;++i) {
            if ( vec[i]==val ) {
                vec[i] = back();
                pop_back();
                return;
            }
        }
    }
    
    const T &operator[](unsigned i) const {
        #ifdef DEBUG
            assert( i < s );
        #endif
        return vec[i];
    }
    T &operator[](unsigned i) {
        #ifdef DEBUG
            assert( i < s );
        #endif
        return vec[i];
    }

    T *begin() { return vec; }
    T *end() { return vec+s; }
    const T *begin() const { return vec; }
    const T *end() const { return vec+s; }
    
    T &back() { return vec[s-1]; }
    const T &back() const { return vec[s-1]; }

private:
    T *vec;
    unsigned s,r;
    bool owning;
};

template<>
class SimpleVector<void> {
public:
};


#endif // SIMPLE_VECTOR_H

