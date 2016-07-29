# project name
PROJECT( metil )

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# add definitions, compiler switches, etc.
ADD_DEFINITIONS( -Wall )

# global variables
SET( CMAKE_METIL_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/lib/metil )
SET( METIL_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/CPP/metil ) # TODO -> find it from ADD_SUBDIRECTORY( CPP )

# subdir
ADD_SUBDIRECTORY( CPP )
ADD_SUBDIRECTORY( MET )
ADD_SUBDIRECTORY( TESTS )

MESSAGE( "Copy met.xml to " $ENV{HOME}/.kde/share/apps/katepart/syntax )
INSTALL( FILES met.xml DESTINATION $ENV{HOME}/.kde/share/apps/katepart/syntax )
