#ifndef METIL_LABEL_H
#define METIL_LABEL_H


struct Label {
    void init() { tok = NULL; sourcefile = NULL; }
    void init(Label &a) { tok = a.tok; sourcefile = a.sourcefile; }
    void reassign(Label &a) { tok = a.tok; sourcefile = a.sourcefile; }
    void destroy() {}
    operator bool() const { return tok; }
    
    const void *go( Thread *th, Variable * &sp ) {
        OldScope *os = (OldScope *)th->info_stack.get_room_for( sizeof(OldScope) );
        os->last_named_variable = th->last_named_variable_in_current_scope;
        os->variable_sp = sp;
        os->prev = th->old_scope;
        os->nb_errors = th->error_list->data.size();
        th->old_scope = os;
        
        th->set_current_sourcefile( sourcefile );
        return tok;
    }
    
    const void *tok;
    SourceFile *sourcefile;
};

#endif // METIL_LABEL_H

