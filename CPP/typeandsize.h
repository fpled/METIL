#ifndef TYPE_AND_SIZE_H
#define TYPE_AND_SIZE_H

#include "config.h"

struct Type;

struct TypeAndSize {
    TypeAndSize( Type *type_, Int32 size_ ) : type(type_), size(size_) {}
    bool operator<(const TypeAndSize &t) const {
        if ( type == t.type )
            return size < t.size;
        return type < t.type;
    }
    
    Type *type;
    Int32 size;
};

#endif // TYPE_AND_SIZE_H
