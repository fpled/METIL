#ifndef HASH_SET_H
#define HASH_SET_H

#include "splittedvec.h"

template<class T,unsigned hash_size,unsigned size_stack,unsigned size_heap>
struct HashSet {
    struct Item {
        T data;
        Item *next_in_hash_table;
    };
    
    HashSet() {
        for(unsigned i=0;i<hash_size;++i)
            hash_table[i] = NULL;
    }
    
    void insert( const T &val, unsigned hash_val ) {
        hash_val %= hash_size;
        for( Item *i = hash_table[hash_val]; i; i = i->next_in_hash_table )
            if ( i->data == val )
                return;

        // not found ? -> create a new one
        Item *i = data.new_elem();
        i->data = val;
        i->next_in_hash_table = hash_table[ hash_val ];
        hash_table[ hash_val ] = i;
    }
    
    bool has( const T &val, unsigned hash_val ) {
        hash_val %= hash_size;
        for( Item *i = hash_table[hash_val]; i; i = i->next_in_hash_table )
            if ( i->data == val )
                return true;
        return false;
    }
    
    unsigned size() const { return data.size(); }
    
    
    template<class Op> struct Apply {
        Apply(Op &o):op(o) {}
        void operator()(Item &v) { op( v.data ); }
        Op &op;
    };
    template<class Op> void apply(Op &op) {
        Apply<Op> ap(op);
        data.apply(ap);
    }
    
    SplittedVec<Item,size_stack,size_heap> data;
    Item *hash_table[ hash_size ];
};

#endif // HASH_SET_H
