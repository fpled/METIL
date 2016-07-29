#ifndef TOKNUMBER_H
#define TOKNUMBER_H

struct TokNumber {
    enum {
        INT = 1,
        RATIONAL = 2,
        DEFINED_PREC = 3,
        FLOAT = 4,
        DOUBLE = 5,

        UNSIGNED = 32,
        IMAGINARY = 64,
        LONG = 128,
    };
    unsigned attributes; /// IMAGINARY...
    int expo;
    unsigned recquired_prec; /// if DEFINED_PREC -> nb unsigned in mantissa.
    unsigned nb_values; /// nb unsigned to represent number...
    unsigned val[1]; /// value(s)

    unsigned type() const { return attributes & 31; }
    double to_double() const;
};

#endif // TOKNUMBER_H
