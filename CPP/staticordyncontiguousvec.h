#ifndef STATICORDYNCONTIGUOUSVEC_H
#define STATICORDYNCONTIGUOUSVEC_H

/**
 */
template<class T,unsigned nb_in_stack>
struct StaticOrDynContiguousVec {
    StaticOrDynContiguousVec( unsigned s = 0, unsigned r = 0 ) {
        init( s, r );
    }
    
    ~StaticOrDynContiguousVec() {
        destroy();
    }
    
    void init( unsigned s = 0, unsigned r = 0 ) {
        size_ = s;
        //
        reservation_ = std::max( nb_in_stack, std::max( s, r ) );
        if ( reservation_ > nb_in_stack )
            data_ = new T[ reservation_ ];
        else
            data_ = sta_data_;
    }
    
    void destroy() {
        if ( data_ != sta_data_ )
            delete [] data_;
    }
    
    const T *ptr() const { return data_; }
    T *ptr() { return data_; }
    
    const T &operator[]( unsigned i ) const { return data_[ i ]; }
    T &operator[]( unsigned i ) { return data_[ i ]; }
    
    unsigned size() const { return size_; }
    unsigned reservation() const { return reservation_; }
    
    void reserve( unsigned wanted_size ) {
        if ( wanted_size > reservation_ ) {
            reservation_ *= 2;
            //
            T *old_data_ = data_;
            data_ = new T[ reservation_ ];
            for(unsigned i=0;i<size_;++i)
                data_[i] = old_data_[i];
            if ( old_data_ != sta_data_ )
                delete [] old_data_;
        }
    }
    
    T *push_back() {
        reserve( size_ + 1 );
        return data_ + (size_++);
    }
    
    template<class T2>
    T *push_back( const T2 &v ) {
        T *res = push_back();
        *res = v;
        return res;
    }
    
    ///
    template<class T2>
    int find( const T2 &val ) const {
        for(unsigned i=0;i<size();++i)
            if ( operator[]( i ) == val )
                return i;
        return -1;
    }
    ///
    template<class T2>
    bool has( const T2 &val ) const {
        return find( val ) >= 0;
    }
    
    ///
    template<class T2>
    void erase_elem_in_unordered_list( const T2 &val ) {
        int i = find( val );
        assert ( i >= 0 );
        //
        if ( i == size() - 1 )
            pop_back();
        else
            operator[]( i ) = pop_back();
    }
    
    T &pop_back() {
        return data_[ --size_ ];
    }
    
private:
    unsigned size_, reservation_;
    T *data_;
    T sta_data_[ nb_in_stack ];
};

#endif // STATICORDYNCONTIGUOUSVEC_H
