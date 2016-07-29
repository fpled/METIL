#ifndef COMPILEDFUNCTIONSET_H
#define COMPILEDFUNCTIONSET_H

#include <map>
#include "dlloader.h"
#include "staticordyncontiguousvec.h"

struct CompiledFunctionSet_ {
    typedef void CompFunc( void ** );
    
    SplittedVec<DlLoader,256,1024> so_data;
    typedef std::map<std::string,CompFunc *> TMAP;
    
    void init() {
        so_data.init();
        function_map = new TMAP;
    }
    void destroy() {
        for(unsigned i=0;i<so_data.size();++i)
            so_data[i].destroy();
        so_data.destroy();
        delete function_map;
    }
    Int32 call( Thread *th, const void *tok, const char *s, unsigned si, VarArgs &b ) {
        TMAP::const_iterator iter = function_map->find( std::string( s, s+si ) );
        if ( iter == function_map->end() )
            return false;
        // list of arguments
        StaticOrDynContiguousVec<void *,8> v( b.nb_uargs() );
        for(unsigned i=0;i<v.size();++i)
            v[ i ] = reinterpret_cast<void *>( b.uarg( i )->data );
        // call
        iter->second( v.ptr() );
        return true;
    }
    void append_so( Thread *th, const void *tok, const char *s, unsigned si ) {
        std::string so_file_name( s, s+si );
        
        DlLoader *ld_handle = so_data.new_elem();
        if ( not ld_handle->init( so_file_name.c_str() ) ) {
            std::cerr << "LD error -> " << ld_handle->last_error() << std::endl;
            return;
        }
        //
        const char **metil_functions = reinterpret_cast<const char **>( ld_handle->find_symbol( "metil_functions" ) );
        if ( not metil_functions )
            return;
        //
//         for(unsigned i=0;metil_functions[ i ];++i)
//             function_map->operator[]( metil_functions[i] ) = reinterpret_cast<CompFunc *>( ld_handle->find_symbol( metil_functions[i] ) );
    }
    
    TMAP *function_map;
};

#endif // COMPILEDFUNCTIONSET_H
