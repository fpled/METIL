#include "tokheader.h"
#include "thread.h"
#include "scope.h"
#include "errorlist.h"
#include "sourcefile.h"
#include "metil_interactive.h"
#include "globaldata.h"
#include "autorestore.h"
#include "lexer.h"
#include "metil_qt_config.h"
#include "DisplayWindowCreator.h"
extern "C" {
    #include "mpi_wrap.h"
}
#include <locale.h>

#ifdef QT4_FOUND
    #include <QtGui/QApplication>
    #include <QtCore/QThread>
    #include <QtCore/QTimer>
#endif // QT4_FOUND

// #define WITHOUT_SIGNAL

#include <iostream>
#ifndef WITHOUT_SIGNAL
    #include <signal.h>
#endif

void display_help(std::ostream &os) {
    os <<   "  -h or --help : I know you know how to use it.\n"
            "  --qt ... : use qt.\n"
            "  --doc file1.sar file2.sar ... : make documentation of files specified as arguments.\n"
            "  -c file1.sar : compile 'file1.sar' into an executable by default named 'file1'.\n"
            "  -o file1 : specify name of the resulting executable.\n"
            "  -s : be silentious.\n"
            "  -v : be verbose.\n"
            "  -V : be very verbose.\n"
            "  -r : remake all '.sar.tok', '.sar.cpp' and '.sar.so' while reading.\n"
            "  -t : precompile files (file.met -> .file.met.cache/).\n"
            "  --prof : generate information for kcachegrind in $currentmetfile.log.\n"
            "  --disp_tok_graph file1.sar ... : display graphes of .met files\n"
            "  --disp_tok_data file1.sar ... : display data contained in .file1.sar.tok, ...\n";
}

Thread *main_thread;

int display_stack() { // for gdb
    main_thread->add_error( "display_stack", NULL );
    return 0;
}

#ifdef QT4_FOUND
struct MetilQtThread : public QThread {
    MetilQtThread() {
        connect( this, SIGNAL(finished()), qApp, SLOT(quit()) );
    }
    void run() {
        setlocale( LC_NUMERIC, "C" );
        thread_loop( main_thread );
    }
};
#endif // QT4_FOUND

#ifndef WITHOUT_SIGNAL
void signal_mgr(int sig) {
    switch ( sig ) {
        case SIGSEGV:
            signal( SIGSEGV, SIG_DFL );
            main_thread->add_error( "segmentation fault", NULL );
            *( (int *)NULL ) = 0;
            break;
        case SIGINT:
            // fprintf(stdout, "Interruption du programme. Nettoyage puis terminaison.n");
            main_thread->interpreter_behavior.prof_entries.make_output();
            main_thread->add_error( "interruption", NULL );
            exit( EXIT_FAILURE );
            break;
        case SIGUSR1:
            std::cout << "SIGUSR1" << std::endl;
            main_thread->display_stack( NULL );
            break;
        default:
            break;
    }
}
#endif


int main(int argc,char **argv) {
    #ifndef WITHOUT_SIGNAL
        signal( SIGINT , signal_mgr );
        signal( SIGSEGV, signal_mgr );
        signal( SIGUSR1, signal_mgr );
    #endif

    // initialisation of main_thread
    main_thread = new Thread( new Scope, new ErrorList );
    AutoDecRef<Thread> adc_thread( main_thread );
    main_thread->primitive_init();
    
    // mpi (or not)
    metil_mpi_init( argc, argv );
    
    // argument parsing
    const char *file_to_compile = NULL, *output_file = NULL, *file_to_interpret = NULL;
    bool with_qt = false;
    for(int i=1;i<argc;++i) {
        // options with only one '-'
        if ( argv[i][0]=='-' and argv[i][1]!='-' ) {
            for(const char *c=argv[i]+1;*c;++c) {
                if      ( *c=='v' ) main_thread->interpreter_behavior.verbose_level = 2;
                else if ( *c=='V' ) main_thread->interpreter_behavior.verbose_level = 3;
                else if ( *c=='s' ) main_thread->interpreter_behavior.verbose_level = 0;
                else if ( *c=='c' ) {
                    if ( file_to_compile ) { main_thread->error_list->add( "-c flag cannot be specified several times." ); return 1; }
                    if ( ++i >= argc ) { main_thread->error_list->add( "-c flag means that you wan't to compile a .met file. It must be followed by the name of that file." ); return 2; }
                    file_to_compile = argv[i];
                }
                else if ( *c=='o' ) {
                    if ( output_file ) { main_thread->error_list->add( "-o flag cannot be specified several times." ); return 3; }
                    if ( ++i >= argc ) { main_thread->error_list->add( "several output files is forbiden." ); return 4; }
                    output_file = argv[i];
                }
                else if ( *c=='d' ) main_thread->interpreter_behavior.debug = true;
                else if ( *c=='r' ) main_thread->interpreter_behavior.remake_all = true;
                else if ( *c=='h' ) { display_help(std::cout); return 0; }
                else if ( *c=='t' ) {
                    main_thread->interpreter_behavior.remake_all = true;
                    while ( ++i < argc ) {
                        global_data.get_sourcefile( argv[i], "/", main_thread->error_list, &main_thread->interpreter_behavior );
                    }
                    return 0;
                }
                else {
                    std::cerr << "-'" << *c << "' is not a known option.\n";
                    display_help(std::cerr);
                    return 0;
                }
            }
        }
        // help
        else if ( strcmp(argv[i],"--help")==0 ) {
            display_help(std::cout);
            return 0;
        }
        // no qt
        else if ( strcmp(argv[i],"--qt")==0 ) {
            with_qt = true;
        }
        // prof
        else if ( strcmp(argv[i],"--prof")==0 ) {
            main_thread->profile_mode = true;
        }
        // disp_tok_data
        else if ( strcmp(argv[i],"--disp_tok_data")==0 ) {
            while ( ++i < argc ) {
                SourceFile *sf = global_data.get_sourcefile( argv[i], "/", main_thread->error_list, &main_thread->interpreter_behavior );
                std::cout << *sf->tok_header() << std::endl;
            }
            return 0;
        }
        // disp_tok_graph
        else if ( strcmp(argv[i],"--disp_tok_graph")==0 ) {
            while ( ++i < argc ) {
                const char *provenance = argv[i];
                FILE *f = fopen( provenance, "r" );
                if ( ! f ) { std::cerr << "Unable to open file '" << provenance << "'." << std::endl; return 1; }
                // read text
                struct stat stat_sar;
                stat( provenance, &stat_sar );
                unsigned si = stat_sar.st_size;
                char *txt_ = (char *)malloc( si+2 );
                fread( txt_+1, 1, si, f );
                txt_[0] = 0;
                txt_[si+1] = 0;    
                fclose( f );
                //
                Lexer l( txt_, provenance, main_thread->error_list );
                display_graph( l.root() );
            }
            return 0;
        }
        // file
        else {
            if ( file_to_compile ) {
                main_thread->error_list->add( "In compilation mode, arguments that should be passed as parameters of executable (like '"+std::string(argv[i])+"' in this example) are not accepted." );
                return 1;
            }
            file_to_interpret = argv[i];
            main_thread->argc = argc - i; main_thread->argv = argv + i;
            break;
        }
    }
    
    // compilation
    if ( file_to_compile ) {
        return main_thread->error_list->data.size();
    }
    
    // else -> command line mode
    if ( output_file ) { main_thread->error_list->add( "Specifing an output file without anything to compile is weird." ); return 7; }
    
    //
    if ( main_thread->profile_mode )
        main_thread->interpreter_behavior.prof_entries.filename = "callgrind.out." + std::string( file_to_interpret ? file_to_interpret : "interp" );
        
    //
    SourceFile *base_sar_init_sf = global_data.get_sourcefile( "metil_base_init.met"                                             , "/", main_thread->error_list, &main_thread->interpreter_behavior );
    SourceFile *sf               = global_data.get_sourcefile( file_to_interpret ? file_to_interpret : "metil_base_interprer.met", ".", main_thread->error_list, &main_thread->interpreter_behavior );
    SourceFile *base_sar_dest_sf = global_data.get_sourcefile( "metil_base_dest.met"                                             , "/", main_thread->error_list, &main_thread->interpreter_behavior );
    
    if ( base_sar_dest_sf and sf and base_sar_init_sf ) {
        main_thread->push_prev_file_to_read( base_sar_dest_sf );
        main_thread->push_prev_file_to_read( sf               );
        main_thread->push_prev_file_to_read( base_sar_init_sf );
    }
    
    #ifdef QT4_FOUND
        if ( with_qt ) {
            QApplication qapp( argc, argv );
            qapp.setQuitOnLastWindowClosed( false );
            
            main_thread->display_window_creator = new DisplayWindowCreator;
            
            MetilQtThread mqt;
            QTimer::singleShot( 0, &mqt, SLOT(start()) );
            
            qapp.exec();
        } else {
    #endif
            main_thread->display_window_creator = NULL;
            thread_loop( main_thread );
    #ifdef QT4_FOUND
        }
    #endif
    
    if ( main_thread->profile_mode )
        main_thread->interpreter_behavior.prof_entries.make_output();
        
    #ifdef HAVE_MPI
        metil_mpi_finalize();
    #endif // HAVE_MPI
    
    return main_thread->return_value;
}

