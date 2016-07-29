#include <fstream>
#include <set>
#include "primitive.h"
#include "tok_def.h"

using namespace std;

///
std::string internal_to_cpp_type(std::string t) {
    //if ( t=="Nstring" ) return "int";
    if ( t=="tok_offset" ) return "int";
    return t;
}
///
std::string conv_from_cpp_type(std::string t) {
    //if ( t=="Nstring" ) return "Nstring(";
    if ( t=="tok_offset" ) return "(int *)((const char *)tok + ";
    return "(";
}
///
void print_disp( std::ostream &os, const TokDef &td, const std::vector<int> &ps ) {
    os << "os << \"" << capitalize( td.name_with_params(ps) ) << "\";";
    std::string C( "Tok::" + capitalize_first(td.name) );
    if ( td.args.size() ) {
        os << " { const " << C << " *d = reinterpret_cast<const " << C << " *>( tok );";
        for(unsigned i=0;i<td.args.size();++i) {
            if ( td.args[i].name.size() ) {
                if ( td.args[i].variable_nb )
                    os << " os << \" " << td.args[i].name << "=\"; for(unsigned i=0;i<d->nb_" << td.args[i].name
                            << ";++i) os << " << conv_from_cpp_type(td.args[i].type) << "d->get_first_" << td.args[i].name << "()[i])" << (td.args[i].type=="char"?"":" << ' '") << ";";
                else
                    os << " os << \" " << td.args[i].name << "=\" << " << conv_from_cpp_type(td.args[i].type) << "d->" << td.args[i].name << ");";
            }
            else {
                std::vector<std::string> sp = split( td.args[i].type );
                for(unsigned j=0,n=1;j<sp.size();++j,n*=2)
                    os << " os << \" " << sp[j] << "=\" << bool(d->attributes&" << n << ");";
            }
        }
        os << " } ";
    }
    
}
///
void print_struct( std::ostream &os, const TokDef &td ) {
    std::string T = capitalize_first(td.name);
    os << "    struct " << T << " {\n";
    os << "        unsigned type; /// VARIABLE, DEFINITION...\n";
    os << "        unsigned offset_in_sar_file; /// pos in original source file\n";
    //
    for(unsigned i=0;i<td.args.size();++i) {
        if ( td.args[i].name.size() ) {
            //
            if ( td.args[i].variable_nb )
                os << "        unsigned nb_" << td.args[i].name << ";\n";
            else
                os << "        " << internal_to_cpp_type(td.args[i].type) << " " << td.args[i].name << ";\n";
        } else {
            std::vector<std::string> sp = split( td.args[i].type );
            for(unsigned j=0,n=1;j<sp.size();++j,n*=2)
                os << "        static const unsigned " << capitalize(sp[j]) << " = " << n << "; " << " bool has_attr_" << sp[j] << "() const { return attributes & " << n << "; }\n";
            os << "\n        unsigned attributes;\n";
        }
    }
    //
    for(unsigned i=0;i<td.args.size();++i)
        if ( td.args[i].name.size() and td.args[i].variable_nb )
            os << "        " << internal_to_cpp_type( td.args[i].type ) << " " << td.args[i].name << "[1];\n";
    // get_first_...
    const TokDef::Arg *old_multi_arg = NULL;
    for(unsigned i=0;i<td.args.size();++i) {
        if ( td.args[i].name.size() and td.args[i].variable_nb ) {
            std::string t = internal_to_cpp_type(td.args[i].type);
            if ( old_multi_arg )
                os << "        const " << t << " *get_first_" << td.args[i].name << "() const { return reinterpret_cast<const " << t << " *>( get_first_" << old_multi_arg->name << "() + nb_" << old_multi_arg->name << " ); }\n";
            else
                os << "\n        const " << t << " *get_first_" << td.args[i].name << "() const { return reinterpret_cast<const " << t << " *>(" << td.args[i].name << "); }\n";
            old_multi_arg = &td.args[i];
        }
    }
    // next_tok()
    if ( old_multi_arg )
        os << "        const void *next() const { const unsigned o = (sizeof(unsigned)-1)/sizeof(*" << old_multi_arg->name << "); return reinterpret_cast<const void *>( &"
                << old_multi_arg->name << " + ( (nb_" << old_multi_arg->name << "+o) & ~o ) ); }\n";
    else
        os << "        const void *next() const { return reinterpret_cast<const void *>( this + 1 ); }\n";
    
    if ( td.field_for_next_in_exception_mode.size() )
        os << "        const void *next_in_exception_mode() const { return reinterpret_cast<const void *>(reinterpret_cast<const char *>(this) + " << td.field_for_next_in_exception_mode << "); }\n";
    else
        os << "        const void *next_in_exception_mode() const { return next(); }\n";

    //
    os << "    };\n";
}


int main(int argc,char **argv) {
    ofstream fh( argv[1] ); // tok.h file
    ofstream fc( argv[2] ); // tok.cpp file

    // get_tok_defs
    vector<TokDef> tok_defs;
    get_tok_defs(tok_defs);

    // add tok for primitives
    vector<PrimitiveClass> primitive_classes;
    vector<PrimitiveFunction> primitive_functions;
    get_primitives( primitive_classes, primitive_functions );
    //
    //set<string> name_of_primitive_functions;
    //for(unsigned i=0;i<primitive_functions.size();++i)
    //    name_of_primitive_functions.insert( primitive_functions[i].name );
    //for(set<string>::const_iterator iter=name_of_primitive_functions.begin();iter!=name_of_primitive_functions.end();++iter)
    //    tok_defs.push_back(TokDef( "primitive_" + *iter ));

    // ----------------------------------------------------------------------------------------------- tok.h
    fh << "#ifndef TOK_H\n";
    fh << "#define TOK_H\n";
    fh << "#include \"nstring.h\"\n\n";
    fh << "#include \"config.h\"\n\n";

    fh << "namespace Tok {\n";
    // enum
    fh << "    enum {\n";
    for(unsigned i=0;i<tok_defs.size();++i) {
        std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
        for(unsigned j=0;j<ps.size();++j)
            fh << "        " << capitalize( tok_defs[i].name_with_params( ps[j] ) ) << ",\n";
    }
    fh << "    };\n";
    for(unsigned i=0;i<tok_defs.size();++i) {
        if ( tok_defs[i].parameters.size() ) {
            fh << "    inline int get_" << capitalize(tok_defs[i].name) << "_type(";
            for(unsigned j=0;j<tok_defs[i].parameters.size();++j)
                fh << "int " << tok_defs[i].parameters[j].name << ( j<tok_defs[i].parameters.size()-1 ? "," : "" );
            fh << ") {\n";
            std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
            for(unsigned j=0;j<ps.size();++j) {
                fh << "        if ( ";
                for(unsigned k=0;k<tok_defs[i].parameters.size();++k)
                    fh << tok_defs[i].parameters[k].name << "==" << ps[j][k] << ( k<tok_defs[i].parameters.size()-1 ? " and " : "" );
                fh << " ) return " << capitalize(tok_defs[i].name);
                for(unsigned k=0;k<tok_defs[i].parameters.size();++k)
                    fh << "__" << capitalize(tok_defs[i].parameters[k].name) << "_" << ps[j][k];
                fh << ";\n";
            }
            fh << "        return 0;\n";
            fh << "    }\n";
        }
    }
    // struct
    for(unsigned i=0;i<tok_defs.size();++i)
        print_struct( fh, tok_defs[i] );
    //
    fh << "    void disp(std::ostream &os,const void *tok);\n";
    fh << "    const void *get_next(const void *tok);\n";
    fh << "    const void *get_next_in_exception_mode(const void *tok);\n";
    fh << "} // namespace\n\n";
    
    fh << "inline unsigned num_primitive(const std::string &type) {\n";
    for(unsigned i=0;i<primitive_classes.size();++i)
        fh << "    if ( type == \"" << primitive_classes[i].met_name << "\" ) return " << i << ";\n";
    fh << "    std::cerr << \"internal error : not a primitive \" << type << std::endl; return 0;\n";
    fh << "}\n\n";

    fh << "#endif // TOK_H\n";


    // ----------------------------------------------------------------------------------------------- tok.cpp
    fc << "#include \"tok.h\"\n\n";
    fc << "namespace Tok {\n\n";

    fc << "void disp(std::ostream &os,const void *tok) {\n";
    fc << "    switch ( *reinterpret_cast<const unsigned *>(tok) ) {\n";
    for(unsigned i=0;i<tok_defs.size();++i) {
        std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
        for(unsigned j=0;j<ps.size();++j) {
            fc << "        case " << capitalize( tok_defs[i].name_with_params( ps[j] ) ) << ": ";
            print_disp( fc, tok_defs[i], ps[j] );
            fc << " break;\n";
        }
    }
    fc << "    }\n";
    fc << "}\n";
    fc << "\n";
    fc << "const void *get_next(const void *tok) {\n";
    fc << "    switch ( *reinterpret_cast<const unsigned *>(tok) ) {\n";
    for(unsigned i=0;i<tok_defs.size();++i) {
        std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
        for(unsigned j=0;j<ps.size();++j)
            fc << "        case " << capitalize( tok_defs[i].name_with_params( ps[j] ) ) << ": return reinterpret_cast<const " << capitalize_first(tok_defs[i].name) << " *>( tok )->next();\n";
    }
    fc << "    }\n";
    fc << "    return NULL;\n";
    fc << "}\n";
    
    fc << "const void *get_next_in_exception_mode(const void *tok) {\n";
    fc << "    switch ( *reinterpret_cast<const unsigned *>(tok) ) {\n";
    for(unsigned i=0;i<tok_defs.size();++i) {
        std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
        for(unsigned j=0;j<ps.size();++j)
            fc << "        case " << capitalize( tok_defs[i].name_with_params( ps[j] ) ) << ": return reinterpret_cast<const " << capitalize_first(tok_defs[i].name) << " *>( tok )->next_in_exception_mode();\n";
    }
    fc << "    }\n";
    fc << "    return NULL;\n";
    fc << "}\n";
    
    fc << "} // namespace Tok\n";
}



