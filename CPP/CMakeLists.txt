INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} )
CONFIGURE_FILE( metil_sys_dir.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_sys_dir.h )

# libraries config
INCLUDE( CheckLibraryExists )
# readline library
FIND_PATH( READLINE_INCLUDE_DIR readline/history.h )
FIND_LIBRARY( READLINE_LIBRARY NAMES readline libreadline )
IF ( READLINE_LIBRARY AND READLINE_INCLUDE_DIR )
    SET( HAVE_READLINE "YES" )
ENDIF ( READLINE_LIBRARY AND READLINE_INCLUDE_DIR )
CONFIGURE_FILE( metil_readline_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_readline_config.h )
# dl library
CHECK_LIBRARY_EXISTS( dl dlopen "" HAVE_LIBDL )
CONFIGURE_FILE( metil_dl_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_dl_config.h )
MESSAGE( HAVE_LIBDL=${HAVE_LIBDL} )

# pthread library
INCLUDE( FindThreads )
# FIND_LIBRARY( PTHREAD_LIBRARY NAMES pthread libpthread )
# MESSAGE( ${CMAKE_USE_PTHREADS_INIT} )

# mpi library
INCLUDE( FindMPI )
CONFIGURE_FILE( metil_mpi_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_mpi_config.h )
IF ( MPI_FOUND )
    INCLUDE_DIRECTORIES(
        ${MPI_INCLUDE_PATH}
    )
ENDIF ( MPI_FOUND )


# Qt4 library
SET(QT_USE_QTSVG 1)
SET (QT_MIN_VERSION "4.4.0")
FIND_PACKAGE(Qt4)
INCLUDE(${QT_USE_FILE})
CONFIGURE_FILE( metil_qt_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_qt_config.h )
SET( MOC_CPP_FILES  )
MESSAGE( QT ${QT4_FOUND} )
IF(QT4_FOUND)
    QT4_GENERATE_MOC( ${CMAKE_CURRENT_SOURCE_DIR}/DisplayWindow.h  ${CMAKE_CURRENT_BINARY_DIR}/DisplayWindow.moc.cpp )
    QT4_GENERATE_MOC( ${CMAKE_CURRENT_SOURCE_DIR}/DisplayWindowCreator.h  ${CMAKE_CURRENT_BINARY_DIR}/DisplayWindowCreator.moc.cpp )
    SET( MOC_CPP_FILES ${CMAKE_CURRENT_BINARY_DIR}/DisplayWindow.moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/DisplayWindowCreator.moc.cpp )
ENDIF(QT4_FOUND)

# ieee754 + BigEndian
INCLUDE( TestBigEndian )
TEST_BIG_ENDIAN( WORDS_BIGENDIAN )
CONFIGURE_FILE( metil_ieee754_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/metil_ieee754_config.h )


# generation using make_usual_strings.cpp
ADD_EXECUTABLE( make_usual_strings make_usual_strings.cpp md5.cpp )
GET_TARGET_PROPERTY( MAKE_USUAL_STRINGS_EXE make_usual_strings LOCATION )
SET( MAKE_USUAL_STRINGS_OUT ${CMAKE_CURRENT_BINARY_DIR}/usual_strings.h ${CMAKE_CURRENT_BINARY_DIR}/usual_strings_cpp.h ${CMAKE_CURRENT_BINARY_DIR}/operators_hash.h )
ADD_CUSTOM_COMMAND( OUTPUT ${MAKE_USUAL_STRINGS_OUT} COMMAND ${MAKE_USUAL_STRINGS_EXE} ARGS ${MAKE_USUAL_STRINGS_OUT} DEPENDS ${MAKE_USUAL_STRINGS_EXE} )

# generation using make_tok_data.cpp
ADD_EXECUTABLE( make_tok_data make_tok_data.cpp )
GET_TARGET_PROPERTY( MAKE_TOK_DATA_EXE make_tok_data LOCATION )
SET( MAKE_TOK_CPP_DATA_OUT ${CMAKE_CURRENT_BINARY_DIR}/tok.cpp )
SET( MAKE_TOK_H_DATA_OUT   ${CMAKE_CURRENT_BINARY_DIR}/tok.h   )
SET( MAKE_TOK_DATA_OUT ${MAKE_TOK_H_DATA_OUT} ${MAKE_TOK_CPP_DATA_OUT} )
ADD_CUSTOM_COMMAND( OUTPUT ${MAKE_TOK_DATA_OUT} COMMAND ${MAKE_TOK_DATA_EXE} ARGS ${MAKE_TOK_DATA_OUT} DEPENDS ${MAKE_TOK_DATA_EXE} )

# generation using make_thread_data.cpp
ADD_EXECUTABLE( make_thread_data make_thread_data.cpp )
GET_TARGET_PROPERTY( MAKE_THREAD_DATA_EXE make_thread_data LOCATION )
SET( MAKE_THREAD_DATA_OUT ${CMAKE_CURRENT_BINARY_DIR}/cases_for_thread_loop.h ${CMAKE_CURRENT_BINARY_DIR}/generated_tok_methods.h ${CMAKE_CURRENT_BINARY_DIR}/generated_globaldata_attributes.h )
ADD_CUSTOM_COMMAND( OUTPUT ${MAKE_THREAD_DATA_OUT} COMMAND ${MAKE_THREAD_DATA_EXE} ARGS ${MAKE_THREAD_DATA_OUT} DEPENDS ${MAKE_THREAD_DATA_EXE} )

# generation using arithmetic_operation
ADD_EXECUTABLE( arithmetic_operation arithmetic_operation.cpp )
GET_TARGET_PROPERTY( ARITHMETIC_OPERATION_EXE arithmetic_operation LOCATION )
SET( ARITHMETIC_OPERATION_OUT ${CMAKE_CURRENT_SOURCE_DIR}/../MET/metil_base_vecop.met )
ADD_CUSTOM_COMMAND( OUTPUT ${ARITHMETIC_OPERATION_OUT} COMMAND ${ARITHMETIC_OPERATION_EXE} ARGS ${ARITHMETIC_OPERATION_OUT} DEPENDS ${ARITHMETIC_OPERATION_EXE} )
ADD_CUSTOM_TARGET( arithmetic_op DEPENDS  ${ARITHMETIC_OPERATION_OUT} )

# lib_lexer
ADD_LIBRARY( metil_lexer STATIC
    ${MAKE_TOK_CPP_DATA_OUT}
    ${MAKE_USUAL_STRINGS_OUT}

    errorlist.cpp
    nstring.cpp
    system.cpp
    
    lexer.cpp
    lexem.cpp
    lexemdefinition.cpp
    
    tokheader.cpp
    make_dot_tok_data.cpp
)


# lib_thread
ADD_LIBRARY( metil_thread STATIC
    ${MAKE_USUAL_STRINGS_OUT}
    ${MAKE_TOK_H_DATA_OUT}
    ${MAKE_THREAD_DATA_OUT}

    mutex.cpp
    dlloader.cpp
    cfile.cpp
    symbolic/ex.cpp
    symbolic/quadratic_expansion.cpp
    symbolic/polynomial_expansion.cpp
    symbolic/op.cpp
    symbolic/codewriter.cpp
    symbolic/OpWithSeq.cpp
    symbolic/codewriteralt.cpp
    symbolic/AsmWriter.cpp
    symbolic/series_func_1_acos.cpp
    symbolic/series_func_1_atan.cpp
    symbolic/series_func_1_log.cpp
    symbolic/series_func_1_asin.cpp
    symbolic/series_func_1_cos.cpp
    symbolic/series_func_1_sin.cpp
    symbolic/series_func_1_atan2.cpp
    symbolic/series_func_1_exp.cpp
    symbolic/series_func_1_tan.cpp
    symbolic/series_pow.cpp
    thread.cpp
    variable.cpp
    type.cpp
    definitiondata.cpp
    definition.cpp
    globaldata.cpp
    sourcefile.cpp
    deftrial.cpp
    property.cpp
    profentries.cpp
    varargs.cpp
    block.cpp
    functionnal.cpp
    metil_interactive.cpp
    transientdata.cpp
    md5.cpp
    DisplayWindowCreator.cpp
    DisplayWindow.cpp
    DisplayWidget.cpp
    DisplayPainter.cpp
    ${MOC_CPP_FILES}
)
ADD_DEPENDENCIES( metil_thread ${ARITHMETIC_OPERATION_OUT} )

# lib_mpi_wrap
IF ( MPI_FOUND )
    ADD_LIBRARY( metil_mpi_wrap STATIC mpi_wrap.c )
ENDIF ( MPI_FOUND )
ADD_LIBRARY( metil_mpi_wrap_no STATIC mpi_wrap_no.c )

# main executable
ADD_EXECUTABLE( metil metil.cpp )
TARGET_LINK_LIBRARIES( metil metil_thread metil_lexer metil_mpi_wrap_no ${QT_LIBRARIES} ${READLINE_LIBRARY} -ldl ${CMAKE_THREAD_LIBS_INIT} ) # -ltermcap 

# main mpi executable
IF ( MPI_FOUND )
    ADD_EXECUTABLE( metil_mpi metil.cpp )
    TARGET_LINK_LIBRARIES( metil_mpi metil_thread metil_lexer metil_mpi_wrap ${QT_LIBRARIES} ${READLINE_LIBRARY} -ldl ${CMAKE_THREAD_LIBS_INIT} ${MPI_LIBRARIES} ) #  -ltermcap
ENDIF ( MPI_FOUND )

# INSTALL
INSTALL( TARGETS metil RUNTIME DESTINATION bin ) #   LIBRARY DESTINATION lib #   ARCHIVE DESTINATION lib

IF ( MPI_FOUND )
    INSTALL( TARGETS metil_mpi RUNTIME DESTINATION bin ) #   LIBRARY DESTINATION lib #   ARCHIVE DESTINATION lib
ENDIF ( MPI_FOUND )

INSTALL( FILES DisplayPainter.h DESTINATION ${CMAKE_METIL_INSTALL_PREFIX} )
INSTALL( FILES DisplayPainterContext.h DESTINATION ${CMAKE_METIL_INSTALL_PREFIX} )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/metil_qt_config.h DESTINATION ${CMAKE_METIL_INSTALL_PREFIX} )
