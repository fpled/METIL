//
// C++ Interface: system
//
// Description: 
//
//
// Author: Hugo LECLERC <leclerc@lmt.ens-cachan.fr>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include "typeconfig.h"

char *cur_dir(); /// get current directory
char *find_path( const char *nf, const char *current_directory ); /// Look for nf in METILPATH. Returns a malloc'ed filename.
bool term_supports_color(); /// return true if terminal supports escape color sequences
// int compilation_to_so(const char *cpp, const char *so, bool disp_command );
std::string absolute_filename( const std::string &f ); // ./grot//toto -> /pouet/grot/toto
Int64 last_modification_time_or_zero_of_file_named( const std::string &n );
void get_includes_of_cpp_named( std::vector<std::string> &dep, const char *s, unsigned si, bool &has_a_corresponding_cpp, bool &has_a_corresponding_generated_cpp ); // return true if has a correponding cpp

double time_of_day_in_sec();
std::string directory_of( const char *s, int si );
std::string get_os_type();
std::string get_cpu_type();

#endif
