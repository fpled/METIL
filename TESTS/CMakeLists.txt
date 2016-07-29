
# ADD_TEST( BasicInterpreter ${METIL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_basic.met )

ADD_TEST( ProgramCompilation ${METIL_EXECUTABLE} -V ${CMAKE_CURRENT_SOURCE_DIR}/test_program.met )

