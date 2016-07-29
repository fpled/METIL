//
// C++ Interface: mutex
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <semaphore.h>
typedef pthread_t Pthread;
typedef sem_t Semaphore;

struct Thread;
struct Variable;
struct Definition;
struct VarArgs;

void pthread_create_( Thread *th, const void *tok, Variable *sp, Pthread &res, Variable *def, VarArgs &args );
void pthread_join_( Pthread &a );

void sem_init_( Thread *th, const void *tok, Semaphore &a, int value );
void sem_wait_( Thread *th, const void *tok, Semaphore &a );
void sem_post_( Thread *th, const void *tok, Semaphore &a );
void sem_destroy_( Thread *th, const void *tok, Semaphore &a );

void save_and_post( Thread *th, const void *tok, Semaphore &sem, VarArgs &a );

#endif
