from string import *

type_char = split("unknown cr dos_makes_me_sick space operator string comment parenthesis number letter ccode end")
m_char = dict( zip(type_char,range(len(type_char))) )

val = [m_char['end']] + [m_char['unknown']] * 255
for c in map(ord,ascii_letters+'_'):        val[c] = m_char['letter']
for c in map(ord,digits):                   val[c] = m_char['number']
for c in map(ord,r"$+-*/=<>&'!^:;.,%|\~@"): val[c] = m_char['operator']
for c in map(ord,' \t'):                    val[c] = m_char['space']
for c in map(ord,'\n'):                     val[c] = m_char['cr']
for c in map(ord,'\r'):                     val[c] = m_char['dos_makes_me_sick']
for c in map(ord,'#'):                      val[c] = m_char['comment']
for c in map(ord,'`'):                      val[c] = m_char['ccode']
for c in map(ord,'"'):                      val[c] = m_char['string']
for c in map(ord,'([{}])'):                 val[c] = m_char['parenthesis']
    
print "#ifndef TYPECHAR_H"
print "#define TYPECHAR_H"
    
print 'inline unsigned char type_char(char c) {'
print '    static const unsigned char type_char_[] = {'
for i in range(32): print '        '+join(map(str,val[i*8:i*8+8]),', ')+','
print '    };'
print '    return type_char_[ unsigned(c) ];'
print '};'

for t,n in m_char.items(): print '#define TYPE_CHAR_%s %i' % ( t, n );

print "#endif // TYPECHAR_H"
