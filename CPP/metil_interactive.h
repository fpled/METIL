//
// C++ Interface: sartiinteractive
//
// Description: 
//
//
// Author: Grover <hugo@gronordi>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef METILINTERACTIVE_H
#define METILINTERACTIVE_H

class Thread;
class Variable;

const void *metil_interactive(Thread *thread,const void *tok,const void *next_tok,Variable *&sp,const char *prompt1=">>> ",const char *prompt2="... ");

#endif
