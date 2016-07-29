#ifndef SIZE_ALIGNEMENT_H
#define SIZE_ALIGNEMENT_H

///
template<class T> struct GetSizeInBitsOf {
    static const unsigned res = sizeof(T) * 8;
};

///
template<class T> struct NeededAlignementOf {
    static const unsigned m = sizeof(unsigned) * 8;
    static const unsigned s = GetSizeInBitsOf<T>::res;
    static const unsigned res = ( s >= m ? m : s );
};
        
/// used 
template<class T> struct GetSizeInBytesForLonelyVariableOf {
    static const unsigned al = sizeof(int) * 8 - 1;
    static const unsigned res = ( ( GetSizeInBitsOf<T>::res + al ) & ~al ) / 8;
};

        
#endif // SIZE_ALIGNEMENT_H

