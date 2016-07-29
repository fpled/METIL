#ifndef TYPECHAR_H
#define TYPECHAR_H
inline unsigned char type_char(char c) {
    static const unsigned char type_char_[] = {
        11, 0, 0, 0, 0, 0, 0, 0,
        0, 3, 1, 0, 0, 2, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        3, 4, 5, 6, 4, 4, 4, 4,
        7, 7, 4, 4, 4, 4, 4, 4,
        8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 4, 4, 4, 4, 4, 0,
        4, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 7, 4, 7, 4, 9,
        10, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 7, 4, 7, 4, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };
    return type_char_[ unsigned(c) ];
};
#define TYPE_CHAR_comment 6
#define TYPE_CHAR_dos_makes_me_sick 2
#define TYPE_CHAR_end 11
#define TYPE_CHAR_string 5
#define TYPE_CHAR_space 3
#define TYPE_CHAR_unknown 0
#define TYPE_CHAR_number 8
#define TYPE_CHAR_letter 9
#define TYPE_CHAR_parenthesis 7
#define TYPE_CHAR_operator 4
#define TYPE_CHAR_cr 1
#define TYPE_CHAR_ccode 10
#endif // TYPECHAR_H
