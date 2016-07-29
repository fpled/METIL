//
// C++ Implementation: dlloader
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
// partly stolen from VTK library

#include "dlloader.h"
#include <metil_dl_config.h>
#include <iostream>

#ifdef HAVE_LIBDL
    #include <dlfcn.h>
#else
    #include <windows.h>
#endif

void DlLoader::init() {
    lib = NULL;
}

bool DlLoader::init( const char *lib_name, int size_lib_name ) {
    char *str = (char *)malloc( size_lib_name + 8 );
    for(int i=0;i<size_lib_name;++i) // -> libdl read char by sets of 4
        str[i] = lib_name[i];
    for(int i=size_lib_name;i<size_lib_name+8;++i) // -> libdl read char by sets of 4
        str[i] = 0;
    bool res = this->init( str );
    free( str );
    return res;
}

bool DlLoader::init( const char *lib_name ) {
    #ifdef HAVE_LIBDL
        lib = dlopen( lib_name, RTLD_LAZY );
    #else
        lib = LoadLibrary( lib_name );
    #endif
    return lib;
}


void DlLoader::destroy() {
    #ifdef HAVE_LIBDL
        if ( lib ) dlclose( lib );
    #else
        FreeLibrary( static_cast<HMODULE>( lib ) );
    #endif
}

const char *DlLoader::last_error() const {
    #ifdef HAVE_LIBDL
        return dlerror(); 
    #else
        LPVOID lpMsgBuf;
        
        FormatMessage( 
                        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL 
                        );
        
        // Free the buffer.
        static char *str = 0;
        delete [] str;
        if (lpMsgBuf) {
            str = strcpy(new char[strlen((char*)lpMsgBuf)+1], (char*)lpMsgBuf);
            LocalFree( lpMsgBuf );
        }
        return str;
    #endif
    return NULL;
}

void *DlLoader::find_symbol( const char *symbol_name ) const {
    #ifdef HAVE_LIBDL
        if ( lib )
            return dlsym( lib, symbol_name ); 
        return NULL;
    #else
        return (void *)GetProcAddress( static_cast<HMODULE>(lib), symbol_name );
    #endif
}

void *DlLoader::find_symbol( const char *symbol_name, int size_symbol_name ) const {
    std::string s( symbol_name, symbol_name + size_symbol_name );
    return find_symbol( s.c_str() );
}

const char *DlLoader::std_prefix() {
    #ifdef HAVE_LIBDL
        return "lib";
    #else
        return "";
    #endif
}

const char *DlLoader::std_extension() {
    #ifdef HAVE_LIBDL
        return ".so";
    #else
        return "";
    #endif
}


