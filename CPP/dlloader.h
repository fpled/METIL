//
// C++ Interface: dlloader
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DLLOADER_H
#define DLLOADER_H

/**
    @author Hugo LECLERC <leclerc@lmt.ens-cachan.fr>
*/
class DlLoader {
public:
    typedef void* DL_Handle;
    
    void init(); // returns true if OK
    bool init( const char *lib_name, int size_lib_name ); // returns true if OK
    bool init( const char *lib_name ); // returns true if OK
    void destroy();
    void *find_symbol( const char *symbol_name ) const;
    void *find_symbol( const char *symbol_name, int size_symbol_name ) const;
    const char *last_error() const;
    operator bool() const { return lib; }
    
    static const char *std_prefix();
    static const char *std_extension();
    
    DL_Handle lib;
};

#endif
