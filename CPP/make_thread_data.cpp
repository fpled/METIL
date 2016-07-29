#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <map>
#include <vector>
#include <sstream>

#include "tok_def.h"
#include "primitive.h"

using namespace std;

template<class T> std::string to_string( T val ) {
    std::ostringstream ss; ss << val;
    return ss.str();
}

void write_cases_for_thread_loop( const char *file_name, const vector<TokDef> &tok_defs ) {
    ofstream ftc( file_name );
    for(unsigned i=0;i<tok_defs.size();++i) {
        const TokDef &td = tok_defs[i];
        std::string C( "Tok::" + capitalize_first(td.name) );
        std::vector<std::vector<int> > ps = tok_defs[i].possible_parameter_sets();
        for(unsigned j=0;j<ps.size();++j) {
            ftc << "         case Tok::" << capitalize( tok_defs[i].name_with_params( ps[j] ) ) << ": tok = exec_tok_" << tok_defs[i].name << "( n__compile_mode, th, reinterpret_cast<const " << C << " *>(tok), sp";
            for(unsigned k=0;k<ps[j].size();++k)
                ftc << ", N<" << ps[j][k] << ">()";
            ftc << " ); ";
            if ( td.may_want_return )
                ftc << "if ( not tok ) { th->current_variable_sp = sp; th->current_tok = NULL; return false; } ";
            ftc << "break;\n";
        }
    }
}

void write_primitive_code( ostream &ftc, const map<string,vector<PrimitiveFunction *> > &name_of_primitive_functions, map<string,PrimitiveClass *> &met_name_to_class ) {
    // exec functions
    for(map<string,vector<PrimitiveFunction *> >::const_iterator iter=name_of_primitive_functions.begin();iter!=name_of_primitive_functions.end();++iter) {
        const std::string &name = iter->first;
        //
        map<string,vector<PrimitiveFunction *> > functions_by_parent;
        for(unsigned i=0;i<iter->second.size();++i)
            functions_by_parent[ iter->second[i]->method_of ].push_back( iter->second[i] );
        //
        for(map<string,vector<PrimitiveFunction *> >::const_iterator iter3=functions_by_parent.begin();iter3!=functions_by_parent.end();++iter3) {
            const std::string &class_name = iter3->first;
            for(unsigned ai=0;ai<iter3->second.size();++ai) {
                PrimitiveFunction *pf = iter3->second[ai];
                ftc << "const void *exec_primitive_" << class_name << "__" << name;
                for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg)
                    ftc << "_" << pf->args[num_arg].type;
                ftc << "( Thread *th, const void *tok, Variable *return_var, Variable **args, Variable *self_var, Variable * &sp ) {\n";
                // args retrieving
                for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg) {
                    string an( 1, 'a' + num_arg );
                    if ( pf->args[num_arg].name.size() ) an = pf->args[num_arg].name;
                    if ( pf->args[num_arg].type=="any" )
                        ftc << "    Variable *" << an << " = args[" << num_arg << "];\n";
                    else if ( pf->args[num_arg].type=="Bool" )
                        ftc << "    Bool " << an << "( args[" << num_arg << "]->data, 1 << (7-args[" << num_arg << "]->get_bit_offset()) );\n";
                    else
                        ftc << "    " << met_name_to_class[pf->args[num_arg].type]->cpp_name << " &" << an << " = *reinterpret_cast<" << met_name_to_class[pf->args[num_arg].type]->cpp_name << " *>( args[" << num_arg << "]->data );\n";
                }
                // self 
                if ( class_name.size() ) {
                    std::string cpp_name = met_name_to_class[class_name]->cpp_name;
                    if ( class_name=="Bool" )
                        ftc << "    Bool self( self_var->data, 1 << (7-self_var->get_bit_offset()) );\n";
                    else
                        ftc << "    " << cpp_name << " &self = *reinterpret_cast<" << cpp_name << " *>( self_var->data );\n";
                }
                // trying to modify const objects
                if ( pf->cond_of_self_modification.size() ) {
                    ftc << "    if ( self_var->is_const() and " << pf->cond_of_self_modification << " ) {\n";
                    ftc << "        th->add_error( \"Modifying a const object is forbidden.\", tok );\n";
                    ftc << "        return tok_end_def_block;\n";
                    ftc << "    }\n";
                }
                
                // allocation of return data
                if ( pf->ret.size() ) {
                    if ( pf->ret != "manual" ) {
                        ftc << "    char *ret_data;\n";
                        ftc << "    if ( return_var ) {\n";
                        ftc << "        const unsigned size_ret = GetSizeInBytesForLonelyVariableOf<" << met_name_to_class[pf->ret]->cpp_name << ">::res;\n";
                        ftc << "        ret_data = (char *)malloc( size_ret + sizeof(TransientData) + sizeof(int) );\n";
                        ftc << "        return_var->data = ret_data;\n";
                        ftc << "        return_var->type = global_data." << pf->ret << ";\n";
                        ftc << "        global_data." << pf->ret << "->inc_ref();\n";
                        ftc << "        return_var->transient_data = reinterpret_cast<TransientData *>( return_var->data + size_ret );\n";
                        ftc << "        return_var->transient_data->init();\n";
                        ftc << "        return_var->attributes = 0;\n";
                        ftc << "        return_var->parent_type = global_data." << pf->ret << ";\n";
                        ftc << "        return_var->parent_data = return_var->data;\n";
                        ftc << "        return_var->parent_transient_data = return_var->transient_data;\n";
                        ftc << "        return_var->prev_named = NULL;\n";
                        ftc << "        return_var->cpt_use = reinterpret_cast<int *>( return_var->transient_data + 1 );\n";
                        ftc << "        *return_var->cpt_use = 0;\n";
                        ftc << "    }\n";
                        ftc << "    else {\n";
                        ftc << "        char tmp[ GetSizeInBytesForLonelyVariableOf<" << met_name_to_class[pf->ret]->cpp_name << ">::res ];\n";
                        ftc << "        ret_data = tmp;\n";
                        ftc << "    }\n";
                        // cpp return variable
                        if ( pf->ret=="Bool" )
                            ftc << "    Bool ret( ret_data, 1 << 7 );\n";
                        else
                            ftc << "    " << met_name_to_class[pf->ret]->cpp_name << " &ret = *reinterpret_cast<" << met_name_to_class[pf->ret]->cpp_name << " *>( ret_data );\n";
                    }
                }
                else {
                    ftc << "    if ( return_var ) {\n";
                    ftc << "        th->add_error(\"Function '" << name << "(";
                    for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg)
                        ftc << pf->args[num_arg].type << ( num_arg+1<pf->args.size() ? "," : "" );
                    ftc << ")' does not return anything.\",tok);\n";
                    ftc << "        assign_error_var( return_var );\n";
                    ftc << "    }\n";
                }
                
                // code
                ftc << "    //\n";
                ftc << "    " << pf->code << "\n";
                
                //
                ftc << "    th->set_current_sourcefile( NULL );\n";
                ftc << "    return tok_end_def_block;\n";
                ftc << "}\n";
            }
        }        
    }
    
    // try functions
    for(map<string,vector<PrimitiveFunction *> >::const_iterator iter=name_of_primitive_functions.begin();iter!=name_of_primitive_functions.end();++iter) {
        map<string,vector<PrimitiveFunction *> > functions_by_parent;
        for(unsigned i=0;i<iter->second.size();++i)
            functions_by_parent[ iter->second[i]->method_of ].push_back( iter->second[i] );
        //
        for(map<string,vector<PrimitiveFunction *> >::const_iterator iter3=functions_by_parent.begin();iter3!=functions_by_parent.end();++iter3) {
            const std::string &class_name = iter3->first;
            ftc << "void cpp_thread_try__" << class_name << "__" << iter->first << "( Thread *th, DefTrial *def_trial, DefinitionData *dd, Variable * &sp ) {\n";
            if ( iter3->second[0]->varargs == false ) {
                ftc << "    if ( def_trial->def->nb_nargs + def_trial->nb_nargs ) { return; }\n";
                ftc << "    unsigned nb_args = ( def_trial->def->nb_uargs + def_trial->nb_uargs );\n";
            }
            if ( class_name.size() ) {
                ftc << "    Variable *self = ( def_trial->def->self.type ? &def_trial->def->self : th->current_self );\n";
                ftc << "    if ( not self ) return;\n";
            }
            
            // sort by nb args
            map<unsigned,vector<PrimitiveFunction *> > primitive_functions_by_nb_args;
            for(unsigned ai=0;ai<iter3->second.size();++ai)
                primitive_functions_by_nb_args[ iter3->second[ai]->args.size() ].push_back( iter3->second[ai] );
            //
            for(map<unsigned,vector<PrimitiveFunction *> >::const_iterator iter2=primitive_functions_by_nb_args.begin();iter2!=primitive_functions_by_nb_args.end();++iter2) {
                unsigned nb_args = iter2->first;
                if ( iter3->second[0]->varargs == false )
                    ftc << ( iter2!=primitive_functions_by_nb_args.begin() ? "    else " : "    " ) << "if ( nb_args == " << nb_args << " ) {\n";
                // ftc << "        std::cout << \"Testing " << iter->first << "\" << std::endl;\n";
                if ( iter3->second[0]->varargs == false ) {
                    ftc << "        Variable *args[" << nb_args << "];\n";
                    ftc << "        for(unsigned i=0;i<def_trial->def->nb_uargs;++i) args[i] = def_trial->def->args + i;\n";
                    ftc << "        for(unsigned i=0;i<def_trial->     nb_uargs;++i) args[def_trial->def->nb_uargs+i] = def_trial->def_v + 1 + i;\n";
                }
                for(unsigned ai=0;ai<iter2->second.size();++ai) {
                    PrimitiveFunction *pf = iter2->second[ai];
                    if ( pf->args.size() ) {
                        ftc << "        if ( ";
                        for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg) {
                            if ( pf->args[num_arg].type == "any" ) ftc << "true";
                            else ftc << "args[" << num_arg << "]->type==global_data." << pf->args[num_arg].type;
                            ftc << ( num_arg+1<pf->args.size() ? " and " : "" );
                        }
                        ftc << " ) {\n";
                    }
                    std::vector<std::string> supplementary_conds;
                    if ( pf->supplementary_cond.size() )
                        supplementary_conds.push_back( pf->supplementary_cond );
                    for(unsigned k=0;k<pf->args.size();++k) {
                        if ( pf->args[k].type == "any" and pf->dont_mind_about_prop==false ) {
                            std::ostringstream ss;
                            ss << "args[" << k << "]->type != global_data.Property";
                            supplementary_conds.push_back( ss.str() );
                        }
                    }
                    if ( supplementary_conds.size() ) {
                        ftc << "          if ( ";
                        for(unsigned k=0;k<supplementary_conds.size();++k)
                            ftc << supplementary_conds[k] << (k<supplementary_conds.size()-1?" and ":"");
                        ftc << " ) {\n";
                    }
                    
                    if ( pf->varargs == false )
                        ftc << "            int room_for_callable = sizeof(Callable) - sizeof(Variable *) + sizeof(Variable *) * " << nb_args << ";\n";
                    else {
                        ftc << "            unsigned nb_args = ( def_trial->def->nb_uargs + def_trial->def->nb_nargs + def_trial->nb_uargs + def_trial->nb_nargs );\n";
                        ftc << "            int room_for_callable = sizeof(Callable) - sizeof(Variable *) + sizeof(Variable *) * ( nb_args + 1 ) + sizeof(Nstring) * (def_trial->def->nb_nargs + def_trial->nb_nargs);\n";
                    }
                    ftc << "            def_trial = th->current_def_trial;\n";
                    ftc << "            Callable *c = (Callable *)th->info_stack.get_room_for( room_for_callable );\n";
                    ftc << "            c->def_data = dd;\n";
                    if ( class_name.size() )
                        ftc << "            c->self = self;\n";
                    else
                        ftc << "            c->self = NULL;\n";
                    ftc << "            c->pertinence = dd->pertinence;\n";
                    ftc << "            c->nb_args_including_varargs = " << pf->args.size() + pf->varargs << ";\n";
                    ftc << "            c->cpp_thread_func = exec_primitive_" << class_name << "__" << pf->name;
                    for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg)
                        ftc << "_" << pf->args[num_arg].type;
                    ftc << ";\n";
                    if ( pf->varargs == false ) {
                        for(unsigned num_arg=0;num_arg<pf->args.size();++num_arg)
                            ftc << "            c->arguments[" << num_arg << "] = args[" << num_arg << "];\n";
                    }
                    else {
                        ftc << "            unsigned cpt_arg = 0, cpt_narg = 0;\n";
                        ftc << "            Nstring *names = (Nstring *)( c->arguments + nb_args + 1 );\n";
                        ftc << "            for(unsigned i=0;i<def_trial->def->nb_uargs;++i,++cpt_arg) { c->arguments[cpt_arg] = def_trial->def->args + i; c->arguments[cpt_arg]->name.v = 0; }\n";
                        ftc << "            for(unsigned i=0;i<def_trial->     nb_uargs;++i,++cpt_arg) { c->arguments[cpt_arg] = def_trial->def_v + 1 + i; c->arguments[cpt_arg]->name.v = 0; }\n";
                        ftc << "            for(unsigned i=0;i<def_trial->def->nb_nargs;++i,++cpt_arg) { c->arguments[cpt_arg] = def_trial->def->args + def_trial->def->nb_uargs + i; names[cpt_narg++] = c->arguments[cpt_arg]->name; }\n";
                        ftc << "            for(unsigned i=0;i<def_trial->     nb_nargs;++i,++cpt_arg) { c->arguments[cpt_arg] = def_trial->def_v + 1 + def_trial->     nb_uargs + i; names[cpt_narg++] = def_trial->names[i]; }\n";
                        ftc << "            c->arguments[cpt_arg] = NULL;\n";
                    }
                    ftc << "            def_trial->min_pertinence_unconditionnal = std::max( def_trial->min_pertinence_unconditionnal, dd->pertinence );\n";
                    ftc << "            c->next = def_trial->first_ok_callable;\n";
                    ftc << "            def_trial->first_ok_callable = c;\n";
                    ftc << "            return;\n";
                    if ( supplementary_conds.size() )
                        ftc << "          }\n";
                    if ( pf->args.size() )
                        ftc << "        }\n";
                }
                if ( iter3->second[0]->varargs == false )
                    ftc << "    }\n";
            }
            ftc << "}\n";
        }
    }    
}

void write_function_decl( ostream &ftc, const vector<TokDef> &tok_defs, unsigned nb_prim_funcs ) {
    ftc << "/*\n";
    for(unsigned i=1;i<tok_defs.size();++i) {
        if ( tok_defs[i].parameters.size() ) {
            ftc << "template<";
            for(unsigned j=0;j<tok_defs[i].parameters.size();++j)
                ftc << "int " << tok_defs[i].parameters[j].name << ( j<tok_defs[i].parameters.size()-1 ? "," : "" );
            ftc << "> ";
        }
        else
            ftc << "inline ";
        ftc << "const void *exec_tok_" << tok_defs[i].name << "(Thread *th,const Tok::" << capitalize_first(tok_defs[i].name) << " *tok,char * &sp";
        for(unsigned j=0;j<tok_defs[i].parameters.size();++j) ftc << ",const N<" << tok_defs[i].parameters[j].name << "> &n__" << tok_defs[i].parameters[j].name;
        ftc << ") {\n";
        ftc << "    return tok->next();\n";
        ftc << "}\n";
    }
    ftc << "*/\n";
}

void write_register_primitive_classes( ostream &ftc, const vector<PrimitiveClass> &primitive_classes, const vector<PrimitiveFunction> &primitive_functions, map<string,PrimitiveClass *> &met_name_to_class ) {
    unsigned cpt_prim = 0;
    for(vector<PrimitiveClass>::const_iterator iter=primitive_classes.begin();iter!=primitive_classes.end();++iter,++cpt_prim) {
        unsigned nb_methods = 0;
        for(unsigned i=0;i<primitive_functions.size();++i)
            nb_methods += ( primitive_functions[i].method_of == iter->met_name );
        ftc << "    // ----------------------------- CLASS -----------------------------\n";
        // DefinitionData
        ftc << "    dd = DefinitionData::make_new(0,Tok::Definition::CLASS + Tok::Definition::STATIC );\n";
        ftc << "    dd->offset_in_sar_file = 0; /// pos in original source file (from Tok::)\n";
        ftc << "    dd->doc = NULL;\n";
        ftc << "    dd->cpp_thread_try_func = NULL;\n";
        ftc << "    dd->try_block = NULL;\n";
        ftc << "    dd->block = NULL;\n";
        ftc << "    dd->sourcefile = NULL;\n";
        ftc << "    dd->name = Nstring(\"" << iter->met_name << "\");\n";
        ftc << "    dd->pertinence = FLOAT64_MAX_VAL;\n";
        ftc << "    dd->min_nb_args = 0;\n";
        ftc << "    dd->nb_args = 0;\n";
        ftc << "    dd->next_with_same_name = NULL;\n\n";
        // type init
        ftc << "    global_data." << iter->met_name << " = types->new_elem();\n";
        ftc << "    *dd->type_cache.new_elem() = global_data." << iter->met_name << ";\n";
        ftc << "    global_data." << iter->met_name << "->init( " << nb_methods + iter->attributes.size() << " );\n";
        ftc << "    global_data." << iter->met_name << "->init_md5_attributes_control_sum( " << cpt_prim << " );\n";
        ftc << "    global_data." << iter->met_name << "->def_data = dd;\n";
        ftc << "    global_data." << iter->met_name << "->name = dd->name;\n";
        ftc << "    global_data." << iter->met_name << "->data_size_in_bits = GetSizeInBitsOf<" << iter->cpp_name << ">::res;\n";
        ftc << "    global_data." << iter->met_name << "->needed_alignement = NeededAlignementOf<" << iter->cpp_name << ">::res;\n";
        ftc << "    global_data." << iter->met_name << "->nb_sub_objects = 0;\n";
        ftc << "    global_data." << iter->met_name << "->reassign_using_mem_copy = " << iter->reassign_using_mem_copy << ";\n";
        ftc << "    global_data.primitive_classes[" << cpt_prim << "] = global_data." << iter->met_name << ";\n\n";
        
        // register Definition
        ftc << "    def_v = current_variable_sp++; def_v->init( global_data.Def, Variable::CONST | Variable::IS_PRIMARY_DEF );\n";
        ftc << "    def = reinterpret_cast<Definition *>( def_v->data );\n";
        ftc << "    def->init( dd );\n";
        ftc << "    main_scope->register_var( def_v, dd->name );\n\n";
    }
}
void write_register_primitive_classes_attr( ostream &ftc, const vector<PrimitiveClass> &primitive_classes, const vector<PrimitiveFunction> &primitive_functions, map<string,PrimitiveClass *> &met_name_to_class ) {
    // attributes
    for(vector<PrimitiveClass>::const_iterator iter=primitive_classes.begin();iter!=primitive_classes.end();++iter) {
        ftc << "    tmp_type = global_data." << iter->met_name << ";\n";
        for(unsigned i=0;i<iter->attributes.size();++i) {
            ftc << "    t_def_v = tmp_type->variables + tmp_type->nb_variables++;\n";
            ftc << "    t_def_v->num_attribute = -1;\n";
            ftc << "    t_def_v->num_in_transient_data = -1;\n";
            ftc << "    t_def_v->v.init( global_data." << iter->attributes[i].type << ", Variable::CONST | Variable::STATIC );\n";
            if ( iter->attributes[i].type=="Bool" ) ftc << "    Bool( t_def_v->v.data, 1 << 7 ) = " << iter->attributes[i].value << ";\n";
            else ftc << "    *reinterpret_cast<" << met_name_to_class[iter->attributes[i].type]->cpp_name << " *>( t_def_v->v.data ) = " << iter->attributes[i].value << ";\n";
            ftc << "    tmp_type->register_var( t_def_v, Nstring(\"" << iter->attributes[i].name << "\") );\n";
        }
    }
}
void write_register_primitive_functions( ostream &ftc, const map<string,vector<PrimitiveFunction *> > &name_of_primitive_functions ) {
    for(map<string,vector<PrimitiveFunction *> >::const_iterator iter=name_of_primitive_functions.begin();iter!=name_of_primitive_functions.end();++iter) {
        map<string,vector<PrimitiveFunction *> > functions_by_parent;
        for(unsigned i=0;i<iter->second.size();++i)
            functions_by_parent[ iter->second[i]->method_of ].push_back( iter->second[i] );
        //
        for(map<string,vector<PrimitiveFunction *> >::const_iterator iter2=functions_by_parent.begin();iter2!=functions_by_parent.end();++iter2) {
            const std::string &class_name = iter2->first;
            // arguments preparation -> if 
            unsigned nb_args = iter2->second[0]->args.size();
            for(unsigned i=1;i<iter2->second.size();++i)
                if ( nb_args and iter2->second[i]->args.size() != nb_args )
                    nb_args = 0;
            std::vector<std::string> args( nb_args );
            for(unsigned i=0;i<iter2->second.size();++i)
                for(unsigned j=0;j<nb_args;++j)
                    args[j] = iter2->second[i]->args[j].name;
            //
            ftc << "    // ----------------------------- DEF -----------------------------\n";
            ftc << "    dd = DefinitionData::make_new(" << nb_args << ",Tok::Definition::STATIC" << (class_name.size()?"+Tok::Definition::METHOD":"") << ");\n";
            // DefinitionData
            ftc << "    dd->offset_in_sar_file = 0; /// pos in original source file (from Tok::)\n";
            ftc << "    dd->doc = NULL;\n";
            ftc << "    dd->cpp_thread_try_func = cpp_thread_try__" << class_name << "__" << iter->first << ";\n";
            ftc << "    dd->sourcefile = NULL;\n";
            ftc << "    dd->name = Nstring(\"" << iter->first << "\");\n";
            ftc << "    dd->pertinence = FLOAT64_MAX_VAL;\n";
            ftc << "    dd->next_with_same_name = NULL;\n";
            for(unsigned i=0;i<nb_args;++i)
                ftc << "    dd->args[" << i << "] = Nstring(\"" << ( args[i].size() ? args[i] : std::string(1,char('a'+i)) ) << "\");\n";
        
            // register Definition
            if ( class_name.size() ) {
                ftc << "    t_def_v = global_data." << class_name << "->variables + global_data." << class_name << "->nb_variables++; // variables already allocated\n";
                ftc << "    t_def_v->v.init( global_data.Def, Variable::CONST | Variable::IS_PRIMARY_DEF );\n";
                ftc << "    t_def_v->num_attribute = -1;\n";
                ftc << "    def = reinterpret_cast<Definition *>( t_def_v->v.data );\n";
                ftc << "    def->init( dd );\n";
                ftc << "    global_data." << class_name << "->register_var( t_def_v, dd->name );\n";
                if ( iter->first == "destroy" ) {
                    ftc << "    global_data." << class_name << "->destroy_def_data = dd;\n";
                    ftc << "    global_data." << class_name << "->destroy_cpp_function = exec_primitive_" << class_name << "__destroy;\n";
                }
            }
            else {
                ftc << "    def_v = current_variable_sp++; def_v->init( global_data.Def, Variable::CONST | Variable::IS_PRIMARY_DEF );\n";
                ftc << "    def = reinterpret_cast<Definition *>( def_v->data );\n";
                ftc << "    def->init( dd );\n";
                ftc << "    main_scope->register_var( def_v, dd->name );\n";
            }
        }
    }
}


void write_direct_conv_to_bool( std::ostream &ftc, const vector<PrimitiveClass> &primitive_classes ) {
    ftc << "int direct_conv_to_bool( Variable *v ) {\n";
    ftc << "    if ( v->type == global_data.Bool ) return bool( Bool( v->data, 1 << (7-v->get_bit_offset()) ) );\n";
    for(unsigned i=0;i<primitive_classes.size();++i)
        if ( primitive_classes[i].met_name!="Bool" and 
             primitive_classes[i].met_name!="Def" and 
             primitive_classes[i].met_name!="Property" and
             primitive_classes[i].met_name!="Error" and
             primitive_classes[i].met_name!="Void" and
             primitive_classes[i].met_name!="Lambda" and
             primitive_classes[i].met_name!="CompiledFunctionSet_" and
             primitive_classes[i].met_name!="Op" and
             primitive_classes[i].met_name!="Pthread" and
             primitive_classes[i].met_name!="Semaphore" and
             primitive_classes[i].met_name!="DisplayWindow_" and
             primitive_classes[i].met_name!="BlockOfTok" )
            ftc << "    if ( v->type == global_data." << primitive_classes[i].met_name << " ) return bool( *reinterpret_cast<const " << primitive_classes[i].cpp_name << " *>(v->data) );\n";
    ftc << "    return -1;\n";
    ftc << "}\n";
}

void write_direct_conv_to_Float64( std::ostream &ftc, const vector<PrimitiveClass> &primitive_classes ) {
    ftc << "inline bool direct_conv_to_Float64( Variable *v, Float64 &res ) {\n";
    for(unsigned i=0;i<primitive_classes.size();++i)
        if ( primitive_classes[i].met_name!="Def" and
             primitive_classes[i].met_name!="Property" and
             primitive_classes[i].met_name!="Error" and
             primitive_classes[i].met_name!="Void" and
             primitive_classes[i].met_name!="Lambda" and
             primitive_classes[i].met_name!="CompiledFunctionSet_" and
             primitive_classes[i].met_name!="UntypedPtr" and
             primitive_classes[i].met_name!="CFile" and
             primitive_classes[i].met_name!="Op" and
             primitive_classes[i].met_name!="Pthread" and
             primitive_classes[i].met_name!="Semaphore" and
             primitive_classes[i].met_name!="DisplayWindow_" and
             primitive_classes[i].met_name!="BlockOfTok" )
            ftc << "    if ( v->type == global_data." << primitive_classes[i].met_name << " ) { res = Float64( *reinterpret_cast<const " << primitive_classes[i].cpp_name << " *>(v->data) ); return true; }\n";
    ftc << "    return false;\n";
    ftc << "}\n";
}

void write_create_functionnal( std::ostream &ftc ) {
    ftc << "    Variable *_fv; Functionnal *_f;\n";
    ftc << "    _fv = current_variable_sp++; _fv->init( global_data.Functionnal, Variable::CONST + Variable::STATIC ); _f = reinterpret_cast<Functionnal *>( _fv->data );\n";
    ftc << "    _f->init(); _f->get_var_nb = Functionnal::SELF;\n";
    ftc << "    main_scope->register_var( _fv, STRING__self_NUM );\n\n";
    for(unsigned i=0;i<20;++i) {
        ftc << "    _fv = current_variable_sp++; _fv->init( global_data.Functionnal, Variable::CONST + Variable::STATIC ); _f = reinterpret_cast<Functionnal *>( _fv->data );\n";
        ftc << "    _f->init(); _f->get_var_nb = " << i << ";\n";
        ftc << "    main_scope->register_var( _fv, STRING__" << i << "_NUM );\n\n";
    }
}

// void write_create_zero_one_buff( std::ostream &ftc, vector<PrimitiveClass> &primitive_classes ) {
//     for(unsigned i=0;i<primitive_classes.size();++i) {
//         PrimitiveClass *pc = &primitive_classes[ i ];
//         if ( arithmetic_class( *pc ) ) {
//             const char *lst[] = {"zero","one","two"};
//             for(int n=0;n<3;++n) {
//                 // op
//                 ftc << "    global_data." << pc->met_name << "->" << lst[n] << "_op = Op::new_variable_op( global_data." << pc->met_name << " );\n";
//                 ftc << "    def_v = current_variable_sp++; def_v->init( global_data." << pc->met_name << ", Variable::CONST ); def_v->transient_data->op = global_data." << pc->met_name << "->" << lst[n] << "_op;\n";
//                 
//                 // variables
//                 ftc << "    global_data." << pc->met_name << "->" << lst[n] << "_var = global_data." << pc->met_name << "->" << lst[n] << "_op->variable_data();\n";
//                 ftc << "    global_data." << pc->met_name << "->" << lst[n] << "_var->init( global_data." << pc->met_name << ", Variable::CONST );\n";
//                 
//                 if ( pc->met_name=="Rationnal" ) ftc << "    reinterpret_cast<" << pc->cpp_name << " *>(global_data." << pc->met_name << "->" << lst[n] << "_var->data )->init(" << n << ");\n";
//                 else if ( pc->met_name=="Bool" ) ftc << "    Bool( global_data." << pc->met_name << "->" << lst[n] << "_var->data, 1 << 7 ) = " << n << ";\n";
//                 else ftc << "    *reinterpret_cast<" << pc->cpp_name << " *>(global_data." << pc->met_name << "->" << lst[n] << "_var->data ) = " << n << ";\n";
//             }
//         }
//     }
// }

// void write_primitive_code_for_op( ostream &ftc, const PrimitiveFunction &pc, map<string,PrimitiveClass *> &met_name_to_class ) {
//     if ( pc.ret.size() == 0 )
//         return;
//     for(unsigned i=0;i<pc.args.size();++i)
//         if (  )
//     ftc << "void exec_tok_for_op_" << primitive_functions[i].decl_name() << "(Variable *return_var,Variable *a,Variable *b,Variable *c,Variable *d) {\n";
//     ftc << "    " << primitive_functions[i].code;
//     ftc << "}\n";
// }

// void write_primitive_code_for_op( ostream &ftc, const vector<PrimitiveFunction> &primitive_functions, map<string,PrimitiveClass *> &met_name_to_class ) {
//     for(unsigned i=0;i<primitive_functions.size();++i) {
//         write_primitive_code_for_op( ftc, primitive_functions[i], met_name_to_class );
//     }
// }

void write_generated_tok_methods_cpp( const char *file_name, const vector<TokDef> &tok_defs, vector<PrimitiveClass> &primitive_classes, vector<PrimitiveFunction> &primitive_functions ) {
    ofstream ftc( file_name );
    
    // maps
    map<string,vector<PrimitiveFunction *> > name_of_primitive_functions;
    for(unsigned i=0;i<primitive_functions.size();++i)
        name_of_primitive_functions[ primitive_functions[i].name ].push_back( &primitive_functions[i] );
    map<string,PrimitiveClass *> met_name_to_class;
    for(unsigned i=0;i<primitive_classes.size();++i)
        met_name_to_class[ primitive_classes[i].met_name ] = &primitive_classes[i];
    
    ftc << "\n";
    
    // primitive exec and try code
    write_primitive_code( ftc, name_of_primitive_functions, met_name_to_class );
    
    // register_primitives
    ftc << "void Thread::primitive_init() {\n";
    ftc << "    DefinitionData *dd;\n";
    ftc << "    Definition *def;\n";
    ftc << "    Variable *def_v;\n\n";
    ftc << "    TypeVariable *t_def_v;\n\n";
    ftc << "    Type *tmp_type;\n\n";
    write_register_primitive_classes( ftc, primitive_classes, primitive_functions, met_name_to_class );
    write_register_primitive_functions( ftc, name_of_primitive_functions );
    write_create_functionnal( ftc );
    // write_create_zero_one_buff( ftc, primitive_classes );
    write_register_primitive_classes_attr( ftc, primitive_classes, primitive_functions, met_name_to_class );
    ftc << "}\n";
    
    // thread loop
    write_function_decl( ftc, tok_defs, name_of_primitive_functions.size() );
    
    // direct_conv_to_...
    write_direct_conv_to_bool( ftc, primitive_classes );
    write_direct_conv_to_Float64( ftc, primitive_classes );
}

void write_generated_globaldata_attributes_h( const char *file_name, const vector<PrimitiveClass> &primitive_classes ) {
    ofstream fgp( file_name );
    
    // primitive_classes, global_data.Float64, ...
    fgp << "    Type *primitive_classes[" << primitive_classes.size() << "];\n";
    for(vector<PrimitiveClass>::const_iterator iter=primitive_classes.begin();iter!=primitive_classes.end();++iter)
        fgp << "    Type *" << iter->met_name << ";\n";
}

int main(int argc, char **argv) {
    vector<TokDef> tok_defs;
    get_tok_defs(tok_defs);
    //
    vector<PrimitiveClass> primitive_classes;
    vector<PrimitiveFunction> primitive_functions;
    get_primitives( primitive_classes, primitive_functions );
    //
    
    //
    write_cases_for_thread_loop( argv[1], tok_defs );
    write_generated_tok_methods_cpp( argv[2], tok_defs, primitive_classes, primitive_functions );
    write_generated_globaldata_attributes_h( argv[3], primitive_classes );
}
