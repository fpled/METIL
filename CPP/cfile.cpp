#include <errno.h>
#include <iostream>

#include "cfile.h"
#include "variable.h"

void CFile::init() {
    f = NULL;
    pipe = false;
}

int CFile::write( const void *p, Int32 si ) {
    return fwrite( (const char *)p, si, 1, f );
}
int CFile::read( void *p, Int32 si ) {
    return fread( (char *)p, si, 1, f );
}

std::string CFile::read() { // hum
    std::string res;
    char buf[256];
    while ( true ) {
        int n = fread( buf, 1, 256, f );
        if ( n <= 0 ) break;
        res += std::string( buf, buf + n );
    }
    return res;
}

void CFile::read_ascii( Int32 &a   ) {
    fscanf( f, "%i", &a  );
}

void CFile::read_ascii( Float32 &a ) {
    fscanf( f, "%f", &a  );
}

void CFile::read_ascii( Float64 &a ) {
    fscanf( f, "%lf", &a );
}

void CFile::close() {
    if ( f and f != stdout and f != stderr and f != stdin ) {
        if ( pipe ) pclose( f );
        else        fclose( f );
        f = NULL;
    }
}
void CFile::flush() {
    fflush( f );
}

int CFile::open_with_str( Variable *a, Variable *b, bool want_pipe ) {
    const char *file_name_str = *reinterpret_cast<const char **>(a->data);
    Int32 file_name_si = *reinterpret_cast<Int32 *>(reinterpret_cast<const char **>(a->data)+1);
    const char *mode_name_str = *reinterpret_cast<const char **>(b->data);
    Int32 mode_name_si = *reinterpret_cast<Int32 *>(reinterpret_cast<const char **>(b->data)+1);
    
    std::string file_name( file_name_str, file_name_str + file_name_si );
    std::string mode_name( mode_name_str, mode_name_str + mode_name_si );
    
    pipe = want_pipe;
    if ( want_pipe )
        f = popen( file_name.c_str(), mode_name.c_str() );
    else
        f = fopen( file_name.c_str(), mode_name.c_str() );
    
    if ( not f )
        return errno;
    return 0;
}

CFile::operator bool() const { return not feof(f); }

bool CFile::eof() const { return feof(f); }
Int32 CFile::tell() const { return ftell(f); }
void CFile::seek( Int64 v ) const { fseek( f, v, SEEK_SET ); }

Int32 CFile::size() const {
    Int32 t = tell();
    fseek( f, 0, SEEK_END );
    Int32 res = tell() - t;
    fseek( f, t, SEEK_SET );
    return res;
}

