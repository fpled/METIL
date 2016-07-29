//
// C++ Interface: profentries
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PROFENTRIES_H
#define PROFENTRIES_H

#include <map>
#include <string>
#include "nstring.h"

struct Thread;
struct DefTrial;
struct SourceFile;

/**
	@author 
*/
struct ProfEntries {
    struct FuncData {
        FuncData( DefTrial *dt );
        std::string full_name() const;
        
        SourceFile *sf;
        Nstring names[16];
        unsigned nb_names;
        unsigned line;
    };
    struct FuncDataCmp {
        bool operator()( const FuncData &fd0, const FuncData &fd1 ) const {
            if ( fd0.nb_names < fd1.nb_names ) return true;
            if ( fd0.nb_names > fd1.nb_names ) return false;
            for(unsigned i=0;i<fd0.nb_names;++i) {
                if ( fd0.names[i].v < fd1.names[i].v ) return true;
                if ( fd0.names[i].v > fd1.names[i].v ) return false;
            }
            if ( fd0.sf     < fd1.sf     ) return true;
            if ( fd0.sf     > fd1.sf     ) return false;
            return fd0.line < fd1.line;
        }
    };
    struct SubEntry {
        SubEntry() : call_count(0) {}
        unsigned call_count; // unsigned total_time;
    };
    struct Entry {
        Entry() {
            nb_ticks = 0;
        }
        unsigned nb_ticks;
        std::map< FuncData, SubEntry, FuncDataCmp > sub_entries;
    };
    
    ProfEntries();
    ~ProfEntries();
    void add_tick( Thread *th );
    void make_output();

    std::map< FuncData, Entry, FuncDataCmp > data;
    std::string filename;
};

#endif
