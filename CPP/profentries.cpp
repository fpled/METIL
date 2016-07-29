//
// C++ Implementation: profentries
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "profentries.h"
#include "thread.h"
#include "deftrial.h"
#include "definition.h"
#include "sourcefile.h"
#include <fstream>
#include <sstream>

ProfEntries::ProfEntries() {
}

ProfEntries::~ProfEntries() {
}

void ProfEntries::add_tick( Thread *th ) {
    for( DefTrial *d = th->current_def_trial; d; d=d->prev_def_trial ) {
        if ( d and d->current_callable and d->current_callable->def_data ) {
            data[ d ].nb_ticks++;
            for( DefTrial *d2 = d->prev_def_trial; d2; d2=d2->prev_def_trial ) {
                if ( d2->current_callable and d2->current_callable->def_data ) {
                    data[ d2 ].sub_entries[ d ].call_count++;
                    break;
                }
            }
            break;
        }
    }
    // return ErrorList::Provenance( d->old_sourcefile->sar_text() + /*offset_in_sar_file*/ reinterpret_cast<const int *>(d->old_tok)[1], d->old_sourcefile->provenance );
}

void disp_entry( std::ostream &f, const ProfEntries::FuncData &fd, const ProfEntries::Entry &e, ProfEntries &pe ) {
    // fi -> file
    if ( fd.sf ) 
        f << "fl=" << fd.sf->provenance << "\n";
    else
        f << "fl=~\n";
        
    // fn -> function data
    f << "fn=" << fd.full_name() << " ";
    if ( fd.sf )
        f << fd.sf->provenance << ":" << fd.sf->line_of_char_num( fd.line ) << "\n";
    else
        f << "~\n";

    // inline time
    f << ( fd.sf ? fd.sf->line_of_char_num( fd.line ) : 0 ) << " " << e.nb_ticks << "\n";
    
    // sub entries
    for( std::map< ProfEntries::FuncData, ProfEntries::SubEntry, ProfEntries::FuncDataCmp >::const_iterator iter = e.sub_entries.begin(); iter != e.sub_entries.end(); ++iter ) {
        const ProfEntries::FuncData &fd2 = iter->first;
        if ( fd2.sf ) 
            f << "cfl=" << fd2.sf->provenance << "\n";
        else
            f << "cfl=~\n";
            
        // fn -> function data
        f << "cfn=" << fd2.full_name() << " ";
        if ( fd2.sf )
            f << fd2.sf->provenance << ":" << fd2.sf->line_of_char_num( fd2.line ) << "\n";
        else
            f << "~\n";
        // inline time
        f << "calls=" << iter->second.call_count << " " << ( fd2.sf ? fd2.sf->line_of_char_num( fd2.line ) : 0 ) << "\n";
        f << ( fd.sf ? fd.sf->line_of_char_num( fd.line ) : 0 ) << " " << iter->second.call_count /*pe.data[ fd2 ].nb_ticks*/ << "\n";
    }
    
    f << "\n";
}

void ProfEntries::make_output() {
    if ( not filename.size() )
        return;
    std::ofstream f( filename.c_str() );
    f << "events: Ticks\n";
    f << "summary: 26\n";
    for( std::map< FuncData, Entry, FuncDataCmp >::const_iterator iter = data.begin(); iter != data.end(); ++iter )
        disp_entry( f, iter->first, iter->second, *this );

}

ProfEntries::FuncData::FuncData( DefTrial *dt ) {
    DefinitionData *dd = dt->current_callable->def_data;
    sf = dd->sourcefile;
    names[0] = dd->name;
    nb_names = 1 + std::min( 15u, dt->nb_uargs + dt->nb_nargs );
    for(unsigned i=0;i<nb_names-1;++i)
        names[i+1] = dt->def_v[i+1].type->name;
    line = dd->offset_in_sar_file;
}

std::string ProfEntries::FuncData::full_name() const {
    std::ostringstream res;
    res << names[0] << "(";
    for(unsigned i=1;i<nb_names;++i)
        res << names[i] << ( i+1<nb_names ? "," : "" );
    res << ")";
    return res.str();
}
