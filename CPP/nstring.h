//
// C++ Interface: nstring
//
// Description: 
//
//
// Author: LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef NSTRING_H
#define NSTRING_H

#include <iostream>
#include <string>

/**
    String stored by a number, with a correspondance in a static list

	@author LECLERC <leclerc@lmt.ens-cachan.fr>
*/
class Nstring {
public:
    Nstring():v(0) {}
    Nstring(int n):v(n) {}
    Nstring(const char *s); /// construction from a C string. Register it if not in hash table.
    Nstring(const char *s,unsigned si); /// construction from a char * and a size. Register it if not in hash table.
    Nstring(const char *s,unsigned si,unsigned hash,bool use_buf=false); /// construction from a char * and a size. Register it if not in hash table. if not present and use buf, copy in a buffer deleted only at the en of the program
    bool operator==(Nstring s) const { return v == s.v; }
    bool operator!=(Nstring s) const { return v != s.v; }
    operator std::string() const;
    
    int v;
};

std::ostream &operator<<( std::ostream &os, Nstring s );

#endif
