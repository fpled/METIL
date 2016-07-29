#include <fstream>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <string.h>
#include "hashstring.h"
#include "md5.h"
#include "typechar.h"
#include "splittedvec.h"
#include "primitive.h"

const unsigned Needlarg = 1;
const unsigned Needrarg = 2;
const unsigned Need2args = 3;
const unsigned MayNeedlarg = 4;
const unsigned MayNeedrarg = 8;
const unsigned Needrrargs = 16;

struct OpInfo {
    const char *met_name;
    const char *cpp_name;
    unsigned behavior;
};

OpInfo operators[] = {
    { "0"          , "0"                     , 0          },
    { "init_using_default_values", "__init_using_default_values__", Needrarg   },
    { "import_c_decl", "__import_c_decl__"   , Needrarg   },
    { "continue"   , "__continue__"          , 0          },
    { "continue_n" , "__continue_n__"        , Needrarg   },
    { "break"      , "__break__"             , 0          },
    { "break_n"    , "__break_n__"           , Needrarg   },
    { "pass"       , "__pass__"              , 0          },
    { "self"       , "__self__"              , 0          },
    { "this"       , "__this__"              , 0          },
    { "try"        , "__try__"               , Needrarg   },
    { "catch"      , "__catch__"             , Needrrargs },
    { "interact"   , "__interact__"          , 0          },
    { "abstract"   , "__abstract__"          , 0          },
    
    { "delete"     , "__delete__"            , Needrarg   },
    
    { "transient"  , "__transient__"         , Needrarg   },
    { "virtual"    , "__virtual__"           , Needrarg   },
    { "static"     , "__static__"            , Needrarg   },
    { "property"   , "__property__"          , Needrarg   },
    
    { "label"      , "__label__"             , Needrrargs },
    { "class"      , "__class__"             , Needrrargs },
    { "def"        , "__def__"               , Needrrargs },
    { ";"          , "__comma_dot__"         , Need2args  },
    
    { "import"     , "__import__"            , Needrarg   },
    { "include"    , "__include__"           , Needrarg   },
    { "exec"       , "__exec__"              , Needrarg   },
    
    { "inherits"   , "__inherits__"          , Need2args  },
    { "pertinence" , "__pertinence__"        , Need2args  },
    { "when"       , "__when__"              , Need2args  },
    { "or_else"    , "__or_else__"           , Need2args  },
    
    { "else"       , "__else__"              , Need2args  },
    
    { "while"      , "__while__"             , Needrrargs },
    { "for"        , "__for__"               , Needrrargs },

    { "if"         , "__if__"                , Needrrargs },

    { "return"     , "__return__"            , Needrarg   },
    { "return_n"   , "__return_n__"          , Needrrargs },
    
    { "const"      , "__const__"             , Needrarg   },
    
    { "throw"      , "__throw__"             , Needrarg   },

    { "infon"      , "__infon__"             , Needrarg   },
    { "info"       , "__info__"              , Needrarg   },
    { "print"      , "__print__"             , Needrarg   },

    { "in"         , "__in__"                , Need2args  },

    { "?"          , "__alternative__"       , Needrarg   },
    { ","          , "__comma__"             , Need2args  },
    

    { "="          , "reassign"              , Need2args  },
    { ":="         , "assign"                , Need2args  },
    { "~="         , "assign_type"         , Need2args  },
    
    { "=>"         , "__lambda__"            , Need2args  },
    
    { "ref"        , "__ref__"               , Needrarg   },
    { "partial_instanciation", "partial_instanciation", Needrarg   },

    { "|="         , "self_or"               , Need2args  },
    { "^="         , "self_xor"              , Need2args  },
    { "&="         , "self_and"              , Need2args  },
    { "%="         , "self_mod"              , Need2args  },
    { "<<="        , "self_shift_left"       , Need2args  },
    { ">>="        , "self_shift_right"      , Need2args  },
    { "/="         , "self_div"              , Need2args  },
    { "*="         , "self_mul"              , Need2args  },
    { "-="         , "self_sub"              , Need2args  },
    { "+="         , "self_add"              , Need2args  },
    { "//="        , "self_div_int"          , Need2args  },
    { "++="        , "self_concatenate"      , Need2args  },
                                           
    { "<<<<"       , "shift_left_long_str"   , Need2args  },
    { "<<<"        , "shift_left_then_endl"  , Need2args  },
    { "<<"         , "shift_left"            , Need2args  },
    { ">>"         , "shift_right"           , Need2args  },

    { "exec_in_prev_scope",
                     "exec_in_prev_scope"    , Needrarg  },
                                        
    
    { "or"         , "or_boolean"            , Need2args  },
    { "and"        , "and_boolean"           , Need2args  },
    { "xor"        , "xor_boolean"           , Need2args  },

    //     { "|"          , "or_bitwise"            , Need2args  },
                                         
    { "not"        , "not_boolean"           , Needrarg   },
                                             
    { "!="         , "not_equal"             , Need2args  },
    { "=="         , "equal"                 , Need2args  },
    { ">="         , "superior_equal"        , Need2args  },
    { "<="         , "inferior_equal"        , Need2args  },
    { ">"          , "superior"              , Need2args  },
    { "<"          , "inferior"              , Need2args  },
                                             
    { "is_a"       , "__is_a__"              , Need2args  },

    { "+++"         , "concatenate"           , Need2args  },

    { ".."         , "range"                 , Need2args  },
    { "..."        , "triple_dots"           , Needlarg   },
                                             
    { "+"          , "add"                   , Need2args  },
    { "-"          , "sub"                   , MayNeedlarg+Needrarg },
    { "%"          , "mod"                   , Need2args  },
    { "*"          , "mul"                   , Need2args  },
    { "/"          , "div"                   , Need2args  },
    { "//"         , "div_int"               , Need2args  },
    { "\\"         , "ml_div"                , Need2args  },
    { "**"         , "pow"                   , Need2args  },
    { "^"          , "__pow__"               , Need2args  },
    { "~"          , "not_bitwise"           , Needrarg   },
    { "'"          , "trans"                 , Needlarg   },
    
    { "***"        , "tensorial_product"     , Need2args  },
        
    { "--"         , "post_dec"              , Needlarg   },
    { "++"         , "post_inc"              , Needlarg   },

    { "&"          , "pointer_on"            , Needrarg   },
    { "@"          , "pointed_value"         , Needrarg   },
    
    { ":"          , "doubledot"             , Need2args  },
    { ":."         , "doubledotdot"          , Need2args  },
    
    { "get_value_prop", "__get_value_prop__" , Needrarg   },
    { "new"        , "__new__"               , Needrarg   },
    
    { "$"          , "calc_name"             , Needrarg   },

    { "::"         , "doubledoubledot"       , Need2args  },
    { "->"         , "get_attr_ptr"          , Need2args  },
    { "."          , "get_attr"              , Need2args  },
        
    { "1"          , "__string_assembly__"   , Need2args  },
    { NULL         , NULL                    , 0          },
};

// we don't us HashSet because data is a contiguous block of chars
struct UsualStrings {
    UsualStrings() {
        for(unsigned i=0;i<hash_size;++i)
            hash_table[i] = NULL;
    }
    struct ResString {
        char *s;
        unsigned si;
        unsigned pos_in_prelim;
        unsigned hash_val;
        ResString *next_in_hash_table;
    };
    void append( const char *s ) { append( s, strlen(s) ); }
    void append( const char *s, unsigned si ) {
        unsigned hash_val = hashstring(s,si) % hash_size;
        for( ResString *i = hash_table[hash_val]; i; i = i->next_in_hash_table )
            if ( i->si == si and strncmp( i->s, s, si )==0 )
                return;
        // not found ? -> create a new one
        ResString *i = usual_strings.new_elem();
        i->si = si;
        i->pos_in_prelim = data.size();
        i->s = data.get_room_for( si );
        memcpy( i->s, s, si );
        i->next_in_hash_table = hash_table[ hash_val ];
        hash_table[ hash_val ] = i;
        i->hash_val = hash_val;
    }
    
    std::string to_contiguous_string() const {
        std::string res; res.resize( data.size() );
        data.copy_binary_data_to( &res[0] );
        return res;
    }
    
    static const unsigned hash_size = 1024;
    SplittedVec<ResString,128> usual_strings;
    SplittedVec<char,1024,2048> data;
    ResString *hash_table[hash_size];
};

std::ostream &operator<<(std::ostream &os,const UsualStrings::ResString &rs) {
    os.write( rs.s, rs.si );
    return os;
}
        
void append_string_of_sar_file( char *sar_file, UsualStrings &usual_strings ) {
    //
    struct stat fileStat;
    if ( stat( sar_file, &fileStat ) ) { std::cerr << "impossible to stat " << sar_file << std::endl; return; }
    char *s = (char *)malloc( fileStat.st_size + 1 ), *sf = s;
    //
    FILE *f = fopen( sar_file, "r" );
    if ( ! f ) { std::cerr << "impossible to open " << sar_file << std::endl; return; }
    fread( s, 1, fileStat.st_size, f );
    s[fileStat.st_size] = 0;
    fclose( f );
    
    // parse
    unsigned char old_type = TYPE_CHAR_comment;
    for( char *os = s; ; ++s ) {
        unsigned char new_type = type_char(s[0]);

        if ( new_type == TYPE_CHAR_letter ) {
            if ( old_type != TYPE_CHAR_letter ) // beginning of a word
                os = s;
        }
        else {
            if ( new_type == TYPE_CHAR_comment ) {
                if ( s[1]=='#' ) { // double comment
                    s += 2;
                    while ( s[0]!=0 and ( s[-1]!='#' or s[0]!='#' ) ) ++s;
                }
                else { // simple comment
                    ++s;
                    while ( s[0]!='\n' and s[0]!=0 ) ++s;
                }
                continue;
            }
            if ( new_type == TYPE_CHAR_ccode ) {
                while ( (++s)[0]!='`' and s[0]!=0 );
                continue;
            }
            if ( new_type == TYPE_CHAR_string ) {
                while ( (++s)[0]!='"' and s[0]!=0 );
                continue;
            }
            
            if ( old_type == TYPE_CHAR_letter ) // end of a word
                usual_strings.append( os, s-os );
            if ( new_type == TYPE_CHAR_end )
                break;
        }
        old_type = new_type;
    }
    
    //
    free( sf );
}

int main(int argc,char **argv) {
    UsualStrings usual_strings;

    std::ofstream fh( argv[1] );
    std::ofstream fc( argv[2] );
    std::ofstream operators_hash_file( argv[3] );

    // operators
    unsigned nb_operators = 0;
    for( ; operators[nb_operators].cpp_name; ++nb_operators )
        usual_strings.append( operators[nb_operators].cpp_name );
    // std files
    for(int i=5;i<argc;++i)
        append_string_of_sar_file( argv[i], usual_strings );
    // primitives
    std::vector<PrimitiveClass> primitive_classes;
    std::vector<PrimitiveFunction> primitive_functions;
    get_primitives( primitive_classes, primitive_functions );
    for(unsigned i=0;i<primitive_classes.size();++i)
        usual_strings.append( primitive_classes[i].met_name.c_str() );
    for(unsigned i=0;i<primitive_functions.size();++i)
        usual_strings.append( primitive_functions[i].name.c_str() );
    //
    const char *additional_usual_strings[] = { "select","change_behavior","tuple","init","varargs","throw_failed_assertion","assert","zip",
        "_1_andreassign_2__eq__3","SimpleStaticVector","T","size","get_size","String","__property_init__","__property_reassign__",
        "__property_call__","__property_call_with_return__","__property_call_partial_inst__","__property_call_partial_inst_with_return__",
        "make_little_heterogeneous_array","size","apply_functionnal","apply_functionnal_with_return","_self","apply","true","false",
        "_0","_1","_2","_3","_4","_5","_6","_7","_8","_9","_10","_11","_12","_13","_14","_15","_16","_17","_18","_19",
        "cant_be_converted_to_bool","0_inheritance","sqrt",NULL };
    for(const char **a = additional_usual_strings; *a; ++a)
        usual_strings.append( *a );
    
    // fh
    fh << "#ifndef USUAL_STRINGS_H\n";
    fh << "#define USUAL_STRINGS_H\n";
    fh << "#define NB_OPERATORS " << nb_operators << "\n";
    fh << "#define NB_PREALLOCATED_STRINGS " << usual_strings.usual_strings.size() << "\n";
    for(unsigned i=0;i<usual_strings.usual_strings.size();++i)
        fh << "#define STRING_" << usual_strings.usual_strings[i] << "_NUM " << i << "\n";
    
    fh << "inline unsigned behavior_of_operator(int num_op) {\n";
    fh << "    static const unsigned tab[] = {";
    for(unsigned i=0;i<nb_operators;++i) fh << operators[i].behavior << ",";
    fh << "};\n";
    fh << "    return tab[num_op];\n";
    fh << "}\n";
    fh << "inline const char *name_of_operator(int num_op) {\n";
    fh << "    static const char *tab[] = {";
    for(unsigned i=0;i<nb_operators;++i)
        if ( operators[i].met_name[0]=='\\' )
            fh << '"' << "\\\\" << "\",";
        else
            fh << '"' << operators[i].met_name << "\",";
    fh << "};\n";
    fh << "    return tab[num_op];\n";
    fh << "}\n";
    fh << "inline unsigned name_size_of_operator(int num_op) {\n";
    fh << "    static const unsigned tab[] = {";
    for(unsigned i=0;i<nb_operators;++i) fh << strlen(operators[i].met_name) << ",";
    fh << "};\n";
    fh << "    return tab[num_op];\n";
    fh << "}\n";
    fh << "inline const char *cpp_name_of_operator(int num_op) {\n";
    fh << "    static const char *tab[] = {";
    for(unsigned i=0;i<nb_operators;++i) fh << '"' << operators[i].cpp_name << "\",";
    fh << "};\n";
    fh << "    return tab[num_op];\n";
    fh << "}\n";
    fh << "inline unsigned cpp_name_size_of_operator(int num_op) {\n";
    fh << "    static const unsigned tab[] = {";
    for(unsigned i=0;i<nb_operators;++i) fh << strlen(operators[i].cpp_name) << ",";
    fh << "};\n";
    fh << "    return tab[num_op];\n";
    fh << "}\n";

    fh << "#define USUAL_STRINGS_MD5 \"" << md5_str( usual_strings.to_contiguous_string() ) << "\"\n";
    
    fh << "#endif // USUAL_STRINGS_H\n";

    // fc
    *usual_strings.data.new_elem() = 0;
    char *d = usual_strings.data.get_binary_data();
    fc << "static const char *usual_string_char = \"" << d << "\";\n";
    free( d );
    fc << "static const unsigned size_hash_nstring = " << UsualStrings::hash_size << ";\n";
    fc << "static NstringStr usual_string_str[] = {\n";
    for(unsigned i=0;i<usual_strings.usual_strings.size();++i)
        fc << "    { usual_string_char+" << usual_strings.usual_strings[i].pos_in_prelim << ", " << usual_strings.usual_strings[i].si
                << ", " << usual_strings.usual_strings[i].hash_val << ", NULL },\n";
    fc << "};\n";

    // operators_hash_file
    operators_hash_file << "#ifndef OPERATORHASH_H\n";
    operators_hash_file << "#define OPERATORHASH_H\n";
    operators_hash_file << "#include \"hashstring.h\"\n";
    operators_hash_file << "#include \"usual_strings.h\"\n";
    operators_hash_file << "#include <string.h>\n";
    
    for(unsigned want_le=0;want_le<2;++want_le) {
        operators_hash_file << "inline int num_operator_" << (want_le?"le":"op") << "(const char *s,unsigned si) {\n";
        unsigned hash_size_trial = nb_operators;
        std::vector<int> nb; nb.resize( hash_size_trial, -1 );
        for(int i=0;i<(int)nb_operators;++i) {
            if ( want_le xor ( type_char(operators[i].met_name[0])!=TYPE_CHAR_letter ) ) {
                unsigned hash_val = hashstring( operators[i].met_name, strlen(operators[i].met_name) ) % hash_size_trial;
                if ( nb[hash_val] >= 0 ) { // there's already an operator in this bucket
                    nb.clear();
                    nb.resize( ++hash_size_trial, -1 );
                    i = -1;
                }
                else
                    nb[hash_val] = i;
            }
        }
        operators_hash_file << "    static const int tab[] = {";
        for(unsigned i=0;i<hash_size_trial;++i)
            operators_hash_file << nb[i] << ",";
        operators_hash_file << "};\n";
        operators_hash_file << "    int n = tab[ hashstring(s,si) % " << hash_size_trial << " ];\n";
        operators_hash_file << "    if ( n>=0 and name_size_of_operator(n)==si and strncmp(s,name_of_operator(n),si)==0 )\n";
        operators_hash_file << "        return n;\n";
        operators_hash_file << "    return -1;\n";
        operators_hash_file << "}\n";
    }
    operators_hash_file << "#endif // OPERATORHASH_H\n";
    return 0;
}



