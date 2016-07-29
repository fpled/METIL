//
// C++ Implementation: nstring
//
// Description: 
//
//
// Author: LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "nstring.h"
#include "hashstring.h"
#include "splittedvec.h"
#include <usual_strings.h>
#include <sstream>
#include <string.h>

struct NstringStr {
    const char *s;
    unsigned si;
    unsigned num;
    NstringStr *next_in_hash_table;

    bool operator==(const NstringStr &str) const { return si==str.si and strncmp(s,str.s,si)==0; }
    unsigned hash() const { return hashstring(s,si); }
};

#include <usual_strings_cpp.h>

struct NstringTable {
    NstringTable() {
        // firsts elements in hash_table
        for(unsigned i=0;i<NB_PREALLOCATED_STRINGS;++i) {
            usual_string_str[i].next_in_hash_table = hash_table[ usual_string_str[i].num ];
            hash_table[ usual_string_str[i].num ] = &usual_string_str[i];
            usual_string_str[i].num = i;
        }
    }
    int operator()( const char *s, unsigned si, unsigned hash_val, bool use_buf=false ) {
        hash_val %= size_hash_nstring;
        for( NstringStr *i = hash_table[hash_val]; i; i = i->next_in_hash_table )
            if ( i->si == si and strncmp( i->s, s, si )==0 )
                return i->num;
        // not found ? -> create a new one
        int n = data.size() + NB_PREALLOCATED_STRINGS;
        NstringStr *i = data.new_elem();
        if ( use_buf ) {
            char *t = buf.get_room_for( si );
            memcpy( t, s, si );
            i->s = t;
        }
        else
            i->s = s;
        i->si = si;
        i->num = n;
        i->next_in_hash_table = hash_table[ hash_val ];
        hash_table[ hash_val ] = i;
        
        return n;
    }
    
    NstringStr *hash_table[ size_hash_nstring ];
    SplittedVec<NstringStr,256> data; /// strings as they appear during execution (not in usual_strings)
    SplittedVec<char,1024> buf; /// string data that may be deleted during use of Nstring hash_table
};
static NstringTable nstring_table;
        
Nstring::Nstring(const char *s) { unsigned si = strlen(s); v = nstring_table( s, si, hashstring(s,si) ); }

Nstring::Nstring(const char *s,unsigned si) { v = nstring_table( s, si, hashstring(s,si) ); }

Nstring::Nstring(const char *s,unsigned si,unsigned hash_val,bool use_buf) { v = nstring_table( s, si, hash_val, use_buf ); }
    
std::ostream &operator<<( std::ostream &os, Nstring s ) {
    if ( s.v < 0 ) {
        os << s.v;
    }
    else if ( s.v >= NB_PREALLOCATED_STRINGS ) {
        const NstringStr &str = nstring_table.data[s.v - NB_PREALLOCATED_STRINGS];
        os.write( str.s, str.si );
    }
    else {
        const NstringStr &str = usual_string_str[s.v];
        os.write( str.s, str.si );
    }
    return os;
}

Nstring::operator std::string() const {
    std::ostringstream os;
    os << *this;
    return os.str();
}
