//
// C++ Implementation: mutex
//
// Description: 
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "mutex.h"
#include "definition.h"
#include "varargs.h"
#include "thread.h"
#include "variable.h"
#include "scope.h"
#include "thread.h"
#include <sstream>
#include <fstream>


void *metil_pthread_loop( void *data ) {
    Thread *th = reinterpret_cast<Thread *>( data );
    
    unsigned nb_uargs = th->current_variable_sp - th->beg_variable_stack - 1;
    th->current_tok = exec_tok_apply( N<0>(), th, /*tok*/th->current_tok, nb_uargs, 0, /*names*/NULL, /*expect_res*/false, /*next_tok*/NULL, th->current_variable_sp );
    
    thread_loop( th );
    delete th;
    
    return NULL;
}


void pthread_create_( Thread *th, const void *tok, Variable *sp, Pthread &res, Variable *def, VarArgs &args ) {
    //
    th->main_scope->cpt_use++; th->error_list->cpt_use++;
    Thread *new_thread = new Thread( th->main_scope, th->error_list, th );
    
    new_thread->set_current_sourcefile( th->current_sourcefile );
    new_thread->current_tok = tok;
    assign_ref_on( new_thread->current_variable_sp++, def );
    for(unsigned i=0;i<args.nb_args();++i)
        assign_ref_on( new_thread->current_variable_sp++, &args.variables[i] );
    args.init_names();
    
    if ( pthread_create( &res, NULL, metil_pthread_loop, new_thread ) )
        th->add_error( "Impossible to create a new thread", tok );
    std::cerr << "creat " << res << std::endl;
}

void pthread_join_( Pthread &a ) {
    void *tmp = NULL;
    std::cerr << "join " << a << std::endl;
    pthread_join( a, &tmp );
    std::cerr << "end of join " << a << std::endl;
}

void sem_init_( Thread *th, const void *tok, Semaphore &a, int value ) {
    assert( value >= 0 );
    if ( sem_init( &a, 0, (unsigned)value ) )
        th->add_error( "Impossible to create a new semaphore", tok );

}

void sem_wait_( Thread *th, const void *tok, Semaphore &a ) {
    if ( sem_wait( &a ) )
        th->add_error( "pb with sem_wait", tok );
}

void sem_post_( Thread *th, const void *tok, Semaphore &a ) {
    if ( sem_post( &a ) )
        th->add_error( "pb with sem_post", tok );
}

void sem_destroy_( Thread *th, const void *tok, Semaphore &a ) {
    if ( sem_destroy( &a ) )
        th->add_error( "pb with sem_destroy", tok );
}

struct SaveAndPost {
    Semaphore *sem;
    void *ptr;
    std::vector<void **> data;
    std::vector<int> sizes;
};

void *save_and_post_thread( void *d ) {
    SaveAndPost *sp = reinterpret_cast<SaveAndPost *>( d );
    std::ostringstream ss;
    ss << "/scratch/leclerc/tmp/tmp_" << sp->ptr;
    std::ofstream f( ss.str().c_str() );
    
    for(unsigned i=0;i<sp->sizes.size();++i) {
        f.write( (char *)*sp->data[i], sp->sizes[i] );
        *sp->data[i] = NULL;
    }

    sem_post( sp->sem );
    return NULL;
}

void save_and_post( Thread *th, const void *tok, Semaphore &sem, VarArgs &a ) {
    SaveAndPost *sp = new SaveAndPost;
    sp->sem = &sem;
    
    sp->ptr = *reinterpret_cast<void **>( a.uarg( 0 )->data );
    for(unsigned i=1;i<a.nb_uargs();i+=2) {
        Variable *ptr = a.uarg( i );
        Variable *siz = a.uarg( i + 1 );
        
        sp->data.push_back( reinterpret_cast<void **>( ptr->data ) ); 
        sp->sizes.push_back( *reinterpret_cast<int *>( siz->data ) ); 
    }
    Pthread pth;
    pthread_create( &pth, NULL, save_and_post_thread, (void *)sp );
}




