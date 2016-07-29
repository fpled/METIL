#ifndef MET_CFILE_H
#define MET_CFILE_H

#include "typeconfig.h"
#include <string>
#include <stdio.h>

struct CFile {
    void init();
    int write( const void *p, Int32 si );
    int read( void *p, Int32 si );
    std::string read();
    
    void read_ascii( Int32 &a   );
    void read_ascii( Float32 &a );
    void read_ascii( Float64 &a );
    
    void close();
    void flush();
    
    int open_with_str( struct Variable *a, struct Variable *b, bool want_pipe );
    
    operator bool() const;
    
    bool eof() const;
    Int32 tell() const;
    void seek( Int64 v ) const;
    
    Int32 size() const;
    
    bool pipe;
    FILE *f;
};

inline bool operator==(const CFile &f0,const CFile &f1) { return f0.f==f1.f; }

#endif // MET_FILE_H
