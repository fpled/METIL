#ifndef AUTO_RESTORE_H
#define AUTO_RESTORE_H

template<class T>
struct AutoRestore {
    AutoRestore(T &val):v(val),old_v(val) {}
    AutoRestore(T &val,const T &new_val):v(val),old_v(val) { val = new_val; }
    
    ~AutoRestore() { restore(); }
    void restore() { v = old_v; }
    
    T &v;
    T old_v;
};

template<class T>
struct AutoDecRef {
    AutoDecRef(T *p) { ptr = p; }
    ~AutoDecRef() { dec_ref(ptr); }
    T *ptr;
};

#endif // AUTO_RESTORE_H
