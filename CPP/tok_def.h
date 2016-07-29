#ifndef TOK_DEF_H
#define TOK_DEF_H

#include "basic_string_manip.h"
#include "rectilinear_iterator.h"
#include <vector>
#include <sstream>

struct TokDef {
    struct Arg {
        Arg(std::string attribute_list="") : type(attribute_list) {}
    
        Arg(std::string t,std::string n,bool v=false) : type(t), name(n), variable_nb(v) {}
    
        std::string type;
        std::string name;
        bool variable_nb;
    };
    struct Parameter {
        Parameter(std::string t,std::vector<int> p) : name(t), possible_values(p) {
            if ( p.size() == 0 ) { possible_values.push_back(0); possible_values.push_back(1); }
        }
        std::string name;
        std::vector<int> possible_values;
    };
    std::string name;
    std::vector<Arg> args;
    std::vector<Parameter> parameters;
    bool may_want_return;
    std::string field_for_next_in_exception_mode;
    
    ///
    TokDef(std::string n,Arg arg0=Arg(),Arg arg1=Arg(),Arg arg2=Arg(),Arg arg3=Arg(),Arg arg4=Arg(),Arg arg5=Arg()):name(n) {
        if (arg0.type.size()) args.push_back( arg0 );
        if (arg1.type.size()) args.push_back( arg1 );
        if (arg2.type.size()) args.push_back( arg2 );
        if (arg3.type.size()) args.push_back( arg3 );
        if (arg4.type.size()) args.push_back( arg4 );
        if (arg5.type.size()) args.push_back( arg5 );
        may_want_return = false;
    }
    ///
    TokDef &A(std::string attribute_list) { args.push_back(Arg(attribute_list)); return *this; }
    ///
    TokDef &A(std::string t,std::string n,bool v=false) { args.push_back(Arg(t,n,v)); return *this; }
    ///
    TokDef &P(std::string t,std::vector<int> p=std::vector<int>()) { parameters.push_back(Parameter(t,p)); return *this; }
    
    ///
    TokDef &set_may_want_return() { may_want_return = true; return *this; }
    ///
    TokDef &set_field_for_next_in_exception_mode(std::string field_for_next_in_exception_mode_) { field_for_next_in_exception_mode = field_for_next_in_exception_mode_; return *this; }
    
    ///
    std::vector<std::vector<int> > possible_parameter_sets() const {
        std::vector<std::vector<int> > res;
        if ( parameters.size() ) {
            std::vector<int> b, e;
            for(unsigned j=0;j<parameters.size();++j) {
                b.push_back( 0 );
                e.push_back( parameters[j].possible_values.size() );
            }
            for( Rectilinear_iterator r(b,e); r; ++r ) {
                std::vector<int> values;
                for(unsigned j=0;j<r.pos.size();++j)
                    values.push_back( parameters[j].possible_values[r.pos[j]] );
                res.push_back( values );
            }
        } else
            res.push_back( std::vector<int>() );
        return res;
    }
    
    ///
    std::string name_with_params(const std::vector<int> &values) const {
        std::ostringstream ss;
        ss << name;
        for(unsigned i=0;i<values.size();++i)
            ss << "__" << parameters[i].name << "_" << values[i];
        return ss.str();
    }
};

inline void get_tok_defs( std::vector<TokDef> &tok_defs ) {
    tok_defs.push_back(TokDef("end_tok_file").set_may_want_return());
    
    tok_defs.push_back(TokDef("push_room_for_new_variable")); /// 
    tok_defs.push_back(TokDef("check_room_in_variable_stack").A("unsigned","nb_variables")); /// 
    
    tok_defs.push_back(TokDef("variable_named").A("Nstring","name"));
    tok_defs.push_back(TokDef("variable_in_stack").A("int","offset_in_nb_variables"));
    tok_defs.push_back(TokDef("variable_in_args").A("int","num_arg"));
    tok_defs.push_back(TokDef("get_attr").A("Nstring","name"));
    
    tok_defs.push_back(TokDef("self"));
    
    tok_defs.push_back(TokDef("interact"));
    
    tok_defs.push_back(TokDef("constify"));
    
    tok_defs.push_back(TokDef("assign").P("calculated_name").P("type").P("need_verification").
            A("static const virtual want_ref property").A("int","offset_to_doc").A("Nstring","name"));
    // tok_defs.push_back(TokDef("reassign"));
    
    tok_defs.push_back(TokDef("apply").A("expect_res").A("unsigned","nb_unnamed").A("Nstring","named",true));
    tok_defs.push_back(TokDef("select").A("expect_res").A("unsigned","nb_unnamed").A("Nstring","named",true));
    tok_defs.push_back(TokDef("change_behavior").A("expect_res").A("unsigned","nb_unnamed").A("Nstring","named",true));
    
    tok_defs.push_back(TokDef("partial_instanciation"));
    tok_defs.push_back(TokDef("append_inheritance_data"));
    tok_defs.push_back(TokDef("test_next_type"));
    
    tok_defs.push_back(TokDef("pass"));
    
    tok_defs.push_back(TokDef("import"));
    tok_defs.push_back(TokDef("exec"));
    
    tok_defs.push_back(TokDef("throw"));
    tok_defs.push_back(TokDef("try").A("tok_offset","offset_to_next_inst").set_field_for_next_in_exception_mode("offset_to_next_inst"));
    tok_defs.push_back(TokDef("catch").A("Nstring","arg").A("tok_offset","offset_to_next_inst").set_field_for_next_in_exception_mode("offset_to_next_inst"));
    tok_defs.push_back(TokDef("end_catch_block"));
    tok_defs.push_back(TokDef("end_try_exception_block"));

    tok_defs.push_back(TokDef("get_calculated_attr"));
    tok_defs.push_back(TokDef("get_value_prop"));
    
    tok_defs.push_back(TokDef("number").A("unsigned imaginary").A("int","expo").A("unsigned","values",true) );
    tok_defs.push_back(TokDef("number_int32").A("Int32","val") );
    tok_defs.push_back(TokDef("number_unsigned32").A("Unsigned32","val") );
    tok_defs.push_back(TokDef("string").A("char","characters",true) );
    
    tok_defs.push_back(TokDef("and_or_or").P("want_or").A("tok_offset","offset_if_dont_want_to_continue").set_field_for_next_in_exception_mode("offset_if_dont_want_to_continue") );
    tok_defs.push_back(TokDef("conversion_to_bool") );
    
    // if/while stuff
    tok_defs.push_back(TokDef("if_or_while").P("want_while").A("tok_offset","offset_if_fail").A("tok_offset","offset_if_not_executed").set_field_for_next_in_exception_mode("offset_if_not_executed") );
    
    tok_defs.push_back(TokDef("end_of_an_if_block"));
    tok_defs.push_back(TokDef("end_of_a_while_block").A("tok_offset","offset_to_condition").P("in_else_block") );
    tok_defs.push_back(TokDef("end_of_an_if_block_followed_by_an_else").A("tok_offset","offset_to_next_inst").set_field_for_next_in_exception_mode("offset_to_next_inst") );
    tok_defs.push_back(TokDef("end_of_a_while_block_followed_by_an_else").A("tok_offset","offset_to_condition").A("tok_offset","offset_to_next_inst").set_field_for_next_in_exception_mode("offset_to_next_inst") );

    // for stuff
    tok_defs.push_back(TokDef("for").P("want_tuple").A("tok_offset","offset_to_next_inst").A("Nstring","names",true).set_field_for_next_in_exception_mode("offset_to_next_inst"));
    tok_defs.push_back(TokDef("end_of_a_for_block"));
    
    tok_defs.push_back(TokDef("lambda").A("tok_offset","offset_to_next_inst").A("Nstring","arguments",true).set_field_for_next_in_exception_mode("offset_to_next_inst"));
    tok_defs.push_back(TokDef("label").A("tok_offset","offset_to_next_inst").A("Nstring","name").set_field_for_next_in_exception_mode("offset_to_next_inst"));
    tok_defs.push_back(TokDef("end_of_a_label_block"));
    
    tok_defs.push_back(TokDef("def_return"));
    tok_defs.push_back(TokDef("break_or_continue").P("want_continue"));
    
    tok_defs.push_back(TokDef("make_varargs").P("expect_result").A("unsigned","nb_unnamed_children").A("Nstring","names",true));

    //    
    tok_defs.push_back(TokDef("definition").
            A("class static virtual varargs self_as_arg abstract method").
            A("Nstring","name"). 
            A("int","offset_to_doc").
            A("unsigned","size_needed_in_stack").
            A("Float64","pertinence").
            A("unsigned","min_nb_args").
            A("Nstring","property_name").
            A("unsigned","type_property").
            A("tok_offset","offset_to_try_block").
            A("tok_offset","offset_to_block").
            A("tok_offset","offset_to_next_inst").
            A("Nstring","args",true).
            set_field_for_next_in_exception_mode("offset_to_next_inst")
    );
    
    tok_defs.push_back(TokDef("proposition_of_default_value").A("unsigned","num_arg").A("tok_offset","offset_to_next_inst"));
    tok_defs.push_back(TokDef("assign_to_arg_nb").A("unsigned","num_arg"));
    
    tok_defs.push_back(TokDef("end_class_block"));
    tok_defs.push_back(TokDef("end_def_block").set_may_want_return());
    
    tok_defs.push_back(TokDef("end_try_block"));
    tok_defs.push_back(TokDef("condition_in_try_block"));
    tok_defs.push_back(TokDef("pertinence_in_try_block"));

    tok_defs.push_back(TokDef("exec_in_prev_scope"));
    tok_defs.push_back(TokDef("end_exec_in_prev_scope"));
}

#endif // TOK_DEF_H
