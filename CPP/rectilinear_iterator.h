/***************************************************************************
                          Rectilinear_iterator.h  -  description
                             -------------------
    begin                : lun oct 21 2002
    copyright            : (C) 2002 by hleclerc
    email                : hleclerc@lmamef23
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RECTILINEAR_ITERATOR_H
#define RECTILINEAR_ITERATOR_H

#include <vector>

/// allows to sweep a multidimensional box, Pt \in [beg,end[
class Rectilinear_iterator {
public:
    typedef std::vector<int> Tvec;

    Rectilinear_iterator(Tvec beg,Tvec end):lower(beg),upper(end) { ///
        pos = beg;
        dim = beg.size();
    }

    operator bool() const {
        return pos[dim-1] < upper[dim-1];
    } ///

    Rectilinear_iterator &operator++() { ///
        pos[0] += 1;
        for(int d=0;d<(int)dim-1;d++) {
            if ( pos[d] >= upper[d] ) {
                pos[d] = lower[d];
                pos[d+1] += 1;
            } else
                return *this;
        }
        return *this;
    }

    unsigned dim;
    Tvec pos; /// THE datum
    Tvec lower; /// first corner of the box
    Tvec upper; /// second corner of the box
};

#endif // RECTILINEAR_ITERATOR_H
