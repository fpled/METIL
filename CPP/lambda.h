#ifndef METIL_LAMBDA_H
#define METIL_LAMBDA_H


struct Lambda {
    void init() { names.init(); tok = NULL; sourcefile = NULL; }
    void init(Lambda &a) { names.init(); names = a.names; tok = a.tok; sourcefile = a.sourcefile; }
    void reassign(Lambda &a) { names = a.names; tok = a.tok; sourcefile = a.sourcefile; }
    void destroy() { names.destroy(); }
    
    SplittedVec<Nstring,4> names;
    const void *tok;
    SourceFile *sourcefile;
};

#endif // METIL_LAMBDA_H

