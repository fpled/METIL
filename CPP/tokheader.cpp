//
// C++ Implementation: tokheader
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "tokheader.h"

std::ostream &operator<<( std::ostream &os, const TokHeader &th ) {
    os << "Tok data\n";
    os << "  pos strings to rellocate: ";
    for(unsigned i=0;i<th.nb_pos_strings_to_rellocate;++i)
        os << th.data[i] << " ";
    os << std::endl;

    os << "  strings:\n";
    TokHeader::StringInfoIterator str = th.strings_begin();
    for(unsigned i=0;i<th.nb_strings;++i,++str) {
        os << "    num=" << str->num << " size=" << str->size << " hash=" << str->hash_val << " str='";
        os.write( str->str, str->size );
        os << "'" << std::endl;
    }
    
    os << "  tokens:\n";
    for( const void *tok = th.tok_data(); tok; tok = Tok::get_next( tok ) ) {
        os << "    " << tok << " ";
        Tok::disp( os, tok );
        os << "\n";
        if ( *reinterpret_cast<const unsigned *>(tok) == Tok::END_TOK_FILE )
            break;
    }
    
    return os;
}

void TokHeader::rellocate() {
    Nstring corr_[2048], *corr;
    // get corr size
    StringInfoIterator si = strings_begin();
    int corr_size = 0;
    for(unsigned i=0;i<nb_strings;++i,++si)
        corr_size = std::max( corr_size, si->num );
    ++corr_size;
    // corr allocation
    if ( corr_size > 2048 )
        corr = (Nstring *)malloc( sizeof(Nstring) * corr_size );
    else
        corr = corr_;

    // get correspondances
    si = strings_begin();
    for(unsigned i=0;i<nb_strings;++i,++si)
        corr[ si->num ] = Nstring( si->str, si->size, si->hash_val );
    
    // string rellocation
    char *b = (char *)tok_data();
    unsigned *sd = data + nb_pos_types_to_rellocate;
    for(unsigned i=0;i<nb_pos_strings_to_rellocate;++i) {
        unsigned *p = (unsigned *)( b + sd[i] );
        *p = corr[ *p ].v;
    }
    
    // property names
    int *pn = begin_property_names();
    for(unsigned i=0;i<nb_property_names;++i,++pn)
        *pn = corr[ *pn ].v;
    
    // free
    if ( corr_size > 2048 )
        free( corr );
}


