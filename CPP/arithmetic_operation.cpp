#include <fstream>
#include <sstream>

struct Operation {
    const char *str;
    unsigned nb_args;
    std::string arg_str() const {
        std::ostringstream ss;
        for(unsigned i=0;i<nb_args;++i)
            ss << char('a'+i) << ( i<nb_args-1 ? ", " : "" );
        return ss.str();
    }
};


static Operation operations[] = {
    { "sub"           , 1 },
    { "exp"           , 1 },
    { "log"           , 1 }, 
    { "sin"           , 1 },
    { "cos"           , 1 },
    { "tan"           , 1 },
    { "asin"          , 1 },
    { "acos"          , 1 },
    { "atan"          , 1 },
    { "sinh"          , 1 },
    { "cosh"          , 1 },
    { "tanh"          , 1 },
    { "heaviside"     , 1 },
    { "abs"           , 1 },
    { "eqz"           , 1 },
    
    { "add"           , 2 },
    { "sub"           , 2 },
    { "mul"           , 2 },
    { "div"           , 2 },
    { "div_int"       , 2 },
    { "mod"           , 2 },
    { "pow"           , 2 },
    { "not_equal"     , 2 },
    { "equal"         , 2 },
    { "inferior"      , 2 },
    { "inferior_equal", 2 },
    { "superior"      , 2 },
    { "superior_equal", 2 },
    
    { NULL, 0 }
};

void lst_op( std::ofstream &metil_base_vecop, const Operation &op, const char *test, const char *type, bool multidim ) {
    // all arg check test
    metil_base_vecop << "def " << op.str << "( " << op.arg_str() << " ) when ";
    for(unsigned na=0;na<op.nb_args;++na)
        metil_base_vecop << char('a'+na) << "." << test << ( na<op.nb_args-1 ? " and " : "" );
    metil_base_vecop << "  return " << type << "Op( " << op.str << ", " << op.arg_str() << " )\n";
    
    // 2 args with only one which ckecks...
    if ( op.nb_args == 2 ) {
       metil_base_vecop << "def " << op.str << "( a, b ) when a." << test << "  return " << type << "Op( " << op.str << ", a, " << type << "Cst(a.size" << (multidim?"s":"") << ",b) )\n";
       metil_base_vecop << "def " << op.str << "( a, b ) when b." << test << "  return " << type << "Op( " << op.str << ", " << type << "Cst(b.size" << (multidim?"s":"") << ",a), b )\n";
    }
}

int main( int argc, char **argv ) {
    std::ofstream metil_base_vecop( argv[1] ); // metil_base_vecop.met
    
    for(unsigned i=0;operations[i].str;++i) {
        lst_op( metil_base_vecop, operations[i], "is_a_vector", "Vec", false );
        lst_op( metil_base_vecop, operations[i], "is_an_array", "Array", true );
        lst_op( metil_base_vecop, operations[i], "is_a_matrix", "Mat", true );

        if ( operations[i].nb_args == 1 )
            metil_base_vecop << "def " << operations[i].str << "( a ) when __is_a_Property__(a)  pertinence 9  toto := a; return " << operations[i].str << "( toto )\n";
        if ( operations[i].nb_args == 2 ) {
            metil_base_vecop << "def " << operations[i].str << "( a, b ) when __is_a_Property__(a)  pertinence 9  toto := a; return " << operations[i].str << "( toto, b )\n";
            metil_base_vecop << "def " << operations[i].str << "( a, b ) when __is_a_Property__(b)  pertinence 8  toto := b; return " << operations[i].str << "( a, toto )\n";
        
            // metil_base_vecop << "def " << operations[i].str << "( a, b ) pertinence 0  return a." << operations[i].str << "( b )\n";
            // metil_base_vecop << "def " << operations[i].str << "( self, b ) when are_of_different_type(self,b) pertinence 0  return op_with_ret_using_type_promote( self, b, " << operations[i].str << " )\n";
            metil_base_vecop << "def " << operations[i].str << "( a, b ) when are_of_different_type(a,b) pertinence 0  return op_with_ret_using_type_promote( a, b, " << operations[i].str << " )\n";
        }
        metil_base_vecop << "\n";
    }
}
