//
// C++ Interface: splittedvec
//
// Description: 
//
//
// Author: Grover <hugo@grosnordi>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SPLITTEDVEC_H
#define SPLITTEDVEC_H

#define __STDC_LIMIT_MACROS
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
//ULONG_MAX
#ifndef SIZE_MAX
#define SIZE_MAX INT_MAX
#endif
#include <iostream>
#include <assert.h>

/**
    vector of values arranged in blocks of item_size_in_stack for the first block,
    and item_size_in_heap for the followings

    if want_new==true, allocation will be made using new, else, malloc will be used.
    In all the cases, the constructors will be called for items until size<=s_stack.
 */
template<class T, unsigned item_size_in_stack, unsigned item_size_in_heap=item_size_in_stack, bool want_new=false>
class SplittedVec {
public:
    // ------------------------------------------------------------------------------------------
    struct Item {
        Item *prev, *next;
        unsigned size, reserved;
    };
    template<unsigned s> struct ItemWithData : public Item {
        T data[ s ];
    };
    Item *last_item;
    ItemWithData<item_size_in_stack> first_item;
    
private:
    template<unsigned n> struct N { };

    void delete_item( Item *t, const N<0> &st  ) { }
    template<unsigned size_trial> void delete_item( Item *t, const N<size_trial> &st  ) {
        static const bool cont = ( size_trial <= SIZE_MAX/4/sizeof(T) );
        if ( t->reserved == size_trial )
            delete static_cast<ItemWithData<size_trial> *>(t);
        else
            delete_item( t, N< ( cont * 2 ) * size_trial >() );
    }
    
    Item *new_item( unsigned wanted_size, const N<0> &st ) { return NULL; }
    template<unsigned size_trial> Item *new_item( unsigned wanted_size, const N<size_trial> &st ) {
        if ( wanted_size <= size_trial ) {
            Item *res = new ItemWithData<size_trial>;
            res->reserved = size_trial;
            return res;
        }
        static const unsigned cont = ( size_trial <= SIZE_MAX/4/sizeof(T) );
        return new_item( wanted_size, N<cont*2*size_trial>() );
    }
    
    Item *new_copy( Item *i, const N<0> &st ) { return NULL; }
    template<unsigned size_trial> Item *new_copy( Item *i, const N<size_trial> &st ) {
        if ( i->reserved == size_trial ) {
            ItemWithData<size_trial> *res;
            if ( want_new ) res = new ItemWithData<size_trial>;
            else            res = (ItemWithData<size_trial> *)malloc( sizeof(ItemWithData<size_trial>) );
            res->reserved = size_trial;
            res->size = i->size;
            for(unsigned j=0;j<i->size;++j)
                res->data[j] = static_cast<ItemWithData<size_trial> *>(i)->data[j];
            return res;
        }
        static const unsigned cont = ( size_trial <= SIZE_MAX/4/sizeof(T) );
        return new_copy( i, N<cont*2*size_trial>() );
    }

public:
    // ------------------------------------------------------------------------------------------
    void init() {
        first_item.size = 0;
        first_item.reserved = item_size_in_stack;

        first_item.prev = NULL;
        first_item.next = NULL;
    
        last_item = &first_item;
    }
    
    void destroy() {
        for(Item *t=first_item.next;t;) {
            Item *old = t;
            t = t->next;
            if ( want_new ) delete_item( old, N<item_size_in_heap>() );
            else            free( old );
        }
    }
    
    SplittedVec() {
        init();
    }
    
    void operator=(const SplittedVec &s) {
        destroy();
        
        first_item.size = s.first_item.size;
        for(unsigned i=0;i<s.first_item.size;++i)
            first_item.data[i] = s.first_item.data[i];
        
        last_item = &first_item;
        for(Item *i=s.first_item.next;i;i=i->next) {
            last_item->next = new_copy(i,N<item_size_in_heap>());
            last_item->next->prev = last_item;
            last_item = last_item->next;
        }
        last_item->next = NULL;
    }
    
    SplittedVec(const SplittedVec &s) {
        init();
        *this = s;
    }
    
    ~SplittedVec() {
        destroy();
    }
    
    // ------------------------------------------------------------------------------------------
    T *get_room_for( unsigned nb_elements ) {
        // can we copy data to already reserved room ?
        if ( last_item->size + nb_elements <= last_item->reserved ) {
            T *ptr = static_cast<ItemWithData<1> *>(last_item)->data + last_item->size;
            last_item->size += nb_elements;
            return ptr;
        }
        // else, if an Item has already been reserved with enough room
        if ( last_item->next and nb_elements <= last_item->next->reserved ) {
            last_item = last_item->next;
            last_item->size = nb_elements;
            return static_cast<ItemWithData<1> *>(last_item)->data;
        }
        // else, we have to create a new item
        Item *old_item = last_item;

        if ( want_new )
            last_item = new_item( nb_elements, N<item_size_in_heap>() );
        else {
            unsigned r = ( nb_elements > item_size_in_heap ? nb_elements : item_size_in_heap );
            last_item = (Item *)malloc( sizeof(ItemWithData<1>) + ( r - 1 ) * sizeof(T) );
            last_item->reserved = r;
        }
        
        last_item->prev = old_item;
        last_item->next = old_item->next;
        old_item ->next = last_item;
        last_item->size = nb_elements;

        return static_cast<ItemWithData<1> *>(last_item)->data;
    }

    T *new_elem() { return get_room_for( 1 ); }
    
    void push_back(const T &val) { *new_elem() = val; } /// append a new element at the end of the list
    void push_back_unique(const T &val) { /// append a new element at the end of the list only if no already present
        for(unsigned i=0;i<size();++i)
            if ( this->operator[](i) == val )
                return;
        *new_elem() = val;
    }
    
    T &pop_back() { /// beware, this operator does not free the memory
        if ( last_item->size==0 )
            last_item = last_item->prev;
        return static_cast<ItemWithData<1> *>(last_item)->data[ --last_item->size ];
    }
    
    T &pop_back(unsigned size_to_remove) { /// beware, this operator does not free the memory
        //
        while ( size_to_remove > last_item->size ) {
            size_to_remove -= last_item->size;
            last_item->size = 0;
            last_item = last_item->prev;
        }
        //
        last_item->size -= size_to_remove;
        return static_cast<ItemWithData<1> *>(last_item)->data[ last_item->size ];
    }

    void clear() { for( Item *i = &first_item; i; i=i->next ) i->size = 0; }

    ///
    int find( const T &val ) const {
        for(unsigned i=0;i<size();++i)
            if ( operator[]( i ) == val )
                return i;
        return -1;
    }
    ///
    bool has( const T &val ) const {
        return find(val) >= 0;
    }
    
    ///
    void erase_elem_in_unordered_list( const T &val ) {
        int i = find( val );
        assert ( i >= 0 );
        erase_elem_nb_in_unordered_list( i );
    }
    
    /// erase element nb
    void erase_elem_nb(unsigned nb) {
        for(unsigned i=nb;i<size()-1;++i)
            operator[](i) = operator[](i+1);
        pop_back();
    }
    
    /// erase element nb
    void erase_elem_nb_in_unordered_list(unsigned nb) {
        if ( nb == size() - 1 )
            pop_back();
        else
            operator[]( nb ) = pop_back();
    }
    
    T &pop_contiguous(unsigned size_to_remove) { /// beware, this operator does not free the memory
        if ( last_item->size == 0 and size_to_remove )
            last_item = last_item->prev;
        last_item->size -= size_to_remove;
        return static_cast<ItemWithData<1> *>(last_item)->data[ last_item->size ];
    }
    
    void pop_until( const T *val ) {
        const T *beg = static_cast<ItemWithData<1> *>(last_item)->data;
        while ( val >= beg + last_item->reserved or val < beg ) {
            last_item->size = 0;
            last_item = last_item->prev;
            beg = static_cast<ItemWithData<1> *>(last_item)->data;
        }
        last_item->size = val - beg;
    }
    
    const T &back() const {
        Item *l = last_item; while ( not l->size ) l = l->prev;
        return static_cast<const ItemWithData<1> *>(l)->data[ l->size - 1 ];
    }
    T &back() {
        Item *l = last_item; while ( not l->size ) l = l->prev;
        return static_cast<ItemWithData<1> *>(l)->data[ l->size - 1 ];
    }
    
    unsigned size() const {
        unsigned res = first_item.size;
        for( Item *i=first_item.next; i; i=i->next )
            res += i->size;
        return res;
    }

    bool empty() const { if ( first_item.size ) return false; for( Item *i=first_item.next; i; i=i->next ) if ( i->size ) return false; return true; }
    
    T &operator[](unsigned val) {
        Item *item = &first_item;
        unsigned v;
        for( v = val; v >= item->size; item = item->next )
            v -= item->size;
        return static_cast<ItemWithData<1> *>(item)->data[ v ];
    }

    const T &operator[](unsigned val) const {
        const Item *item = &first_item;
        unsigned v;
        for( v = val; v >= item->size; item = item->next )
            v -= item->size;
        return static_cast<const ItemWithData<1> *>(item)->data[ v ];
    }

    void *append(const T *data,unsigned size) {
        T *ptr = get_room_for(size);
        for(unsigned i=0;i<size;++i)
            ptr[i]=data[i];
        return ptr;
    }

    /// s * sizeof(T2) % sizeof(T) must be == 0
    template<class T2> T2 *binary_write( const T2 *data, unsigned s ) { return (T2 *)append( (const T *)data, s * sizeof(T2) / sizeof(T) ); }
    
    template<class T2> T2 *binary_write( const T2 &data ) { return binary_write( &data, 1 ); }

    template<class Op> void apply(Op &op) {
        for(unsigned i=0;i<first_item.size;++i)
            op( first_item.data[i] );
        for(Item *t=first_item.next; t; t=t->next )
            for(unsigned i=0;i<t->size;++i)
                op( static_cast<ItemWithData<1> *>(t)->data[i] );
    }

    template<class Op,class TT> void apply(Op &op,TT &tt) {
        for(unsigned i=0;i<first_item.size;++i)
            op( first_item.data[i], tt );
        for(Item *t=first_item.next; t; t=t->next )
            for(unsigned i=0;i<t->size;++i)
                op( static_cast<ItemWithData<1> *>(t)->data[i], tt );
    }
    
    template<class Op> void apply_reverse(Op &op) {
        for(Item *t=last_item; t; t=t->prev )
            for(int i=t->size-1;i>=0;--i)
                op( static_cast<ItemWithData<1> *>(t)->data[i] );
    }

    template<class Op,class TT> void apply_reverse(Op &op,TT &tt) {
        for(Item *t=last_item; t; t=t->prev )
            for(int i=t->size-1;i>=0;--i)
                op( static_cast<ItemWithData<1> *>(t)->data[i], tt );
    }

    void copy_binary_data_to( char *res ) const {
        memcpy( res, first_item.data, first_item.size * sizeof(T) );
        res += first_item.size * sizeof(T);
        for(Item *t=first_item.next; t; t=t->next ) {
            memcpy( res, static_cast<ItemWithData<1> *>(t)->data, t->size * sizeof(T) );
            res += t->size * sizeof(T);
        }
    }
    char *get_binary_data() {
        char *res = (char *)malloc( size() );
        copy_binary_data_to( res );
        return res;
    }
    
    template<class T2,unsigned its2,unsigned ith2,bool wn2> bool operator==(const SplittedVec<T2,its2,ith2,wn2> &v) const {
        if ( size() != v.size() )
            return false;
        for(unsigned i=0;i<size();++i)
            if ( operator[](i) != v[i] )
                return false;
        return true;
    }
    
    template<class Op> void sort( const Op &op ) { // TODO -> optimize
        for(unsigned i=0;i<size();++i)
            for(unsigned j=i+1;j<size();++j)
                if ( op( operator[](i), operator[](j) ) )
                    std::swap( operator[](i), operator[](j) );
    }
};

#endif

