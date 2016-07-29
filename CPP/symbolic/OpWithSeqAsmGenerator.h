#ifndef OPWITHSEQASMGENERATOR_H
#define OPWITHSEQASMGENERATOR_H

#include "splittedvec.h"
#include <unistd.h>
#include <sys/mman.h>
// #define DEBUG_ASM

struct OpWithSeqAsmGenerator {
    enum AsmType {
        X86,
        X86_64
    };
    static const int T_size = sizeof( Float64 ); //
    
    OpWithSeqAsmGenerator( AsmType asm_type ) : asm_type( asm_type ) {
        num_instr = 0;
        nb_ops = 0;
        nb_regs = 8 + 8 * ( asm_type == X86_64 );
        for(unsigned i=0;i<nb_regs;++i)
            regs[ i ] = NULL;
        stack_size = NULL;
    }
    
    void push_header() {
        write_save_registers_from_stack();
        stack_size = write_sub_stack_pointer();
    }

    void push_footer() {
        #ifdef DEBUG_ASM
            std::cout << "    ; stack size = " << stack.size() * T_size << std::endl;
        #endif
        
        *stack_size = stack.size() * T_size;
        *write_add_stack_pointer() = stack.size() * T_size;
        
        write_load_registers_from_stack();
        write_ret_instr();
        
        //         #ifdef DEBUG_ASM
        //             for(unsigned i=0;i<os.size();++i) {
        //                 if ( i % 8 == 0 )
        //                     std::cout << std::endl << "    ;";
        //                 std::cout << " ";
        //                 std::cout.width( 2 );
        //                 std::cout.fill( '0' );
        //                 std::cout << std::hex << (int)os[i];
        //             }
        //         #endif
    }
    
    int get_free_room_in_stack() {
        int room_in_stack = -1;
        for(unsigned i=0;i<stack.size();++i) {
            if ( not stack[i] ) {
                room_in_stack = i;
                break;
            }
        }
        if ( room_in_stack < 0 ) {
            room_in_stack = stack.size();
            stack.push_back( NULL );
        }
        return room_in_stack;
    }
    
    int get_free_reg( int forbidden_register = -1 ) {
        // if there's already a free reg
        for(unsigned i=0;i<nb_regs;++i)
            if ( i != forbidden_register and not regs[ i ] )
                return i;
        
        // else, if there's already a reg saved in stack
        int best_parent_date = -1, best_reg = -1;
        for(unsigned i=0;i<nb_regs;++i) {
            if ( i != forbidden_register and regs[ i ]->stack >= 0 and best_parent_date < regs[ i ]->min_parent_date() ) {
                best_parent_date = regs[ i ]->min_parent_date();
                best_reg = i;
            }
        }
        if ( best_reg >= 0 )  {
            regs[ best_reg ]->reg = -1;
            return best_reg;
        }
        
        
        // else, select a given register to be saved in stack and...
        best_parent_date = -1;
        best_reg = 0;
        for(unsigned i=1;i<nb_regs;++i) {
            if ( i != forbidden_register and best_parent_date < regs[ i ]->min_parent_date() ) {
                best_parent_date = regs[ i ]->min_parent_date();
                best_reg = i;
            }
        }
        // get a room to save it, and save it
        int room_in_stack = get_free_room_in_stack();
        stack[ room_in_stack ] = regs[ best_reg ];
        regs[ best_reg ]->stack = room_in_stack;
        regs[ best_reg ]->reg = -1;
        write_save_xmm_in_stack_plus_offset( best_reg, room_in_stack * T_size );
        return best_reg;
    }
    
    void write_instr( Thread *th, const void *tok, OpWithSeq *op ) {
        if ( op->type == OpWithSeq::SEQ )
            return;
        if ( op->ptr_val ) // somewhere in memory
            return;
        
        ++num_instr;
        nb_ops += op->nb_instr();
        
        #ifdef DEBUG_ASM
            if ( op->type == OpWithSeq::WRITE_REASSIGN )
                std::cout << "    ; SAVE" << std::endl;
            else if ( op->type == OpWithSeq::NUMBER )
                std::cout << "    ; " << op->num / op->den << std::endl;
            else if ( op->type == OpWithSeq::NUMBER_M1 )
                std::cout << "    ; ~( 1L << 63 )" << std::endl;
            else if ( op->type == OpWithSeq::SYMBOL )
                std::cout << "    ; " << op->cpp_name_str << std::endl;
            else
                std::cout << "    ; " << Nstring( op->type ) << std::endl;
        #endif
        
        for(unsigned i=0;i<op->children.size();++i)
            ++op->children[i]->nb_times_used;
        
        //
        if ( op->type == OpWithSeq::WRITE_REASSIGN ) {
            write_save_op_in_memory( op->ptr_res, op->children[0] );
            op->children[0]->ptr_val = op->ptr_res;
        } else if ( op->type == OpWithSeq::WRITE_ADD ) {
            assert( 0 /*TODO*/ );
        } else if ( op->type == OpWithSeq::WRITE_RET ) {
            assert( 0 /*TODO*/ );
        } else if ( op->type == OpWithSeq::WRITE_INIT ) {
            assert( 0 /*TODO*/ );
        } else if ( op->type == OpWithSeq::NUMBER ) {
            // -> put number in stack
            op->stack = get_free_room_in_stack(); stack[ op->stack ] = op;
            write_assign_number_to_offset_from_stack_pointer( op->stack * T_size, op->num / op->den );
        } else if ( op->type == OpWithSeq::NUMBER_M1 ) {
            op->stack = get_free_room_in_stack(); stack[ op->stack ] = op;
            write_assign_number_M1_to_offset_from_stack_pointer( op->stack * T_size );
        } else if ( op->type == OpWithSeq::SYMBOL ) {
            th->add_error( "There's a remaining non associated symbol (named '" + std::string( op->cpp_name_str ) + "') in expression.", tok );
        } else if ( op->type == STRING_add_NUM or op->type == STRING_mul_NUM ) {
            unsigned char sse2_op = ( op->type == STRING_add_NUM ? 0x58 : 0x59 );
            // look for a child with a register that will be freed after
            for(unsigned i=0;i<op->children.size();++i) {
                if ( op->children[i]->reg >= 0 and op->children[i]->nb_times_used == op->children[i]->parents.size() ) { // if children[i] has a register which can be reused
                    op->reg = op->children[i]->reg; regs[ op->reg ] = op;
                    write_self_sse2_op_xmm( op->reg, op->children[ 1 - i ], sse2_op );
                    break;
                }
            }
            // else, look for a child with a register... to be saved
            if ( op->reg < 0 ) {
                for(unsigned i=0;i<op->children.size();++i) {
                    if ( op->children[i]->reg >= 0 ) { // if children[i] has a register which can be used but has to be saved before
                        op->reg = op->children[i]->reg; regs[ op->reg ] = op;
                        write_save_op_somewhere_and_clear_reg( op->children[i] );
                        write_self_sse2_op_xmm( op->reg, op->children[ 1 - i ], sse2_op );
                        break;
                    }
                }
            }
            // else, everything in stack or in memory -> save children[0] in a free reg and use it
            if ( op->reg < 0 ) {
                op->reg = get_free_reg(); regs[ op->reg ] = op;
                write_get_op_in_reg( op->reg, op->children[0] );
                write_self_sse2_op_xmm( op->reg, op->children[ 1 ], sse2_op );
            }
        } else if ( op->type == STRING_sub_NUM or op->type == STRING_div_NUM ) {
            unsigned char sse2_op = ( op->type == STRING_sub_NUM ? 0x5c : 0x5e );
            if ( op->children[ 0 ]->reg >= 0 ) {
                op->reg = op->children[0]->reg; regs[ op->reg ] = op;
                if ( op->children[0]->nb_times_used < op->children[0]->parents.size() ) // need to save children[0]
                    write_save_op_somewhere_and_clear_reg( op->children[0] );
            } else {
                op->reg = get_free_reg(); regs[ op->reg ] = op;
                write_get_op_in_reg( op->reg, op->children[0] );
            }
            write_self_sse2_op_xmm( op->reg, op->children[ 1 ], sse2_op );
        } else if ( op->type == STRING_select_symbolic_NUM ) {
            op->reg = get_free_reg(); regs[ op->reg ] = op;
            write_select_symbolic( op->reg, op );
        } else if ( op->type == STRING_pow_NUM ) {
            assert( 0 );
            write_pow_instr( op );
        } else if ( op->type == STRING_sqrt_NUM ) {
            assert( 0 );
            write_self_sse2_op_xmm( op->reg, op->children[ 0 ], 0x51 ); // particular case : op->reg does not have to be initialized
        } else if ( op->type == STRING_heaviside_NUM ) {
            write_heaviside_or_eqz( op, 5 ); // >= 0
        } else if ( op->type == STRING_eqz_NUM ) {
            write_heaviside_or_eqz( op, 0 ); // == 0
        } else if ( op->type == STRING_pos_part_NUM ) {
            assert( 0 );
            write_pos_part( op->reg, op ); // x_+
        } else if ( op->type == STRING_abs_NUM ) {
            assert( 0 );
            write_save_op_in_xmm( op->reg, op->children[ 0 ] );
            write_andsd( op->reg, op->children[ 1 ] );
        } else {
            std::cout << "TODO: " << Nstring( op->type ) << std::endl;
            assert( 0 );
        }
        
        // free operands with only one remaining parent
        for(unsigned i=0;i<op->children.size();++i) {
            if ( op->children[i]->nb_times_used == op->children[i]->parents.size() ) {
                if ( op->children[i]->stack >= 0 )
                    stack[ op->children[i]->stack ] = NULL;
                if ( op->children[i]->reg >= 0 and op->reg != op->children[i]->reg )
                    regs[ op->children[i]->reg ] = NULL;
            }
        }
    }
    
    void *add_ret_and_make_code_as_new_contiguous_data() const {
        //
        unsigned code_size = os.size();
        void *res = malloc( code_size );
        
        os.copy_binary_data_to( (char *)res );
        
        size_t gm1 = getpagesize() - 1;
        size_t beg = (size_t)res & ~gm1;
        size_t end = ( (size_t)res + code_size + gm1 ) & ~gm1;
        
        mprotect( (void *)beg, end - beg, PROT_READ | PROT_WRITE | PROT_EXEC );
        return res;
    }
    
    AsmType asm_type;
    SplittedVec<unsigned char,1024,1024> os;
    unsigned num_instr, nb_ops;
    
    OpWithSeq *regs[16];
    unsigned nb_regs;
    
    SplittedVec<OpWithSeq *,1024,1024> stack;
private:
    void write_save_op_somewhere_and_clear_reg( OpWithSeq *op ) {
        assert( op->reg >= 0 );
        if ( op->stack >= 0 or op->ptr_val ) { // already saved
            op->reg = -1;
            return;
        }
        // there's a free register
        for(unsigned i=0;i<nb_regs;++i) {
            if ( not regs[ i ] ) {
                write_copy_xmm_regs( i, op->reg );
                op->reg = i; regs[ i ] = op;
                return;
            }
        }
        // else put in stack
        op->stack = get_free_room_in_stack(); stack[ op->stack ] = op;
        write_save_xmm_in_stack_plus_offset( op->reg, op->stack * T_size );
        op->reg = -1;
    }
    void write_pow_instr( OpWithSeq *op ) {
        std::cout << "TODO : pow(x,y)... in asm" << std::endl;
        assert( 0 );
    }
    void write_get_op_in_reg( int reg, OpWithSeq *op ) {
        if ( op->reg >= 0 )
            write_copy_xmm_regs( reg, op->reg );
        else if ( op->stack >= 0 )
            write_save_stack_in_xmm( reg, op->stack * T_size );
        else {
            assert( op->ptr_val );
            write_save_memory_in_xmm( reg, op->ptr_val, op );
        }
    }

    void write_select_symbolic( int reg, OpWithSeq *op ) {
        assert( op->children[0]->ptr_val );
        assert( op->children[1]->type == Op::NUMBER or op->children[1]->ptr_val );
        #ifdef DEBUG_ASM
            std::cout << "    mov   rax, " << op->children[0]->ptr_val << std::endl;
            if ( op->children[1]->type == Op::NUMBER ) {
                std::cout << "    add   rax, " << op->children[1]->val() * T_size << std::endl;
                std::cout << "    movsd xmm" << reg << ", [ rax ]" << std::endl;
            } else {
                std::cout << "    push  rbx" << std::endl;
                std::cout << "    mov   rbx, " << op->children[1]->ptr_val << std::endl;
                std::cout << "    mov   ebx, [ rbx ]" << std::endl;
                std::cout << "    movsd xmm" << reg << ", [ rax + 8 * rbx ]" << std::endl;
                std::cout << "    pop   rbx" << std::endl;
            }
        #endif
        // mov rax, ...
        os.push_back( 0x48 );
        os.push_back( 0xb8 );
        *reinterpret_cast<void **>( os.get_room_for( sizeof(void *) ) ) = op->children[0]->ptr_val;
        
        if ( op->children[1]->type == Op::NUMBER ) {
            // add rax, ...
            os.push_back( 0x48 );
            os.push_back( 0x05 );
            *reinterpret_cast<size_t *>( os.get_room_for( sizeof(size_t) ) ) = size_t(op->children[1]->val()) * T_size;
            
            // mov xmm., [ rax ]
            os.push_back( 0xf2 );
            if ( reg >= 8 )
                os.push_back( 0x44 );
            os.push_back( 0x0f );
            os.push_back( 0x10 );
            os.push_back( 0x00 + 8 * ( reg % 8 ) );
        } else {
            assert( op->children[1]->ptr_val );
            assert( op->children[1]->nstring_type == STRING_Int32_NUM );
            
            // push rbx
            os.push_back( 0x53 );
            
            // mov rbx, ...
            os.push_back( 0x48 );
            os.push_back( 0xbb );
            *reinterpret_cast<void **>( os.get_room_for( sizeof(void *) ) ) = op->children[1]->ptr_val;
            
            // mov ebx, [ rbx ]
            os.push_back( 0x8b );
            os.push_back( 0x1b );
            
            // mov xmm., [ rax + 8 * rbx ]
            os.push_back( 0xf2 );
            if ( reg >= 8 )
                os.push_back( 0x44 );
            os.push_back( 0x0f );
            os.push_back( 0x10 );
            os.push_back( 0x04 + 8 * ( reg % 8 ) );
            os.push_back( 0xd8 );
            
            // pop rbx
            os.push_back( 0x5b );
        }
        
    }
    
    void write_heaviside_or_eqz( OpWithSeq *op, unsigned char cmp_op ) {
        if ( op->children[ 0 ]->reg >= 0 ) {
            op->reg = op->children[ 0 ]->reg; regs[ op->reg ] = op;
            if ( op->children[ 0 ]->nb_times_used != op->children[ 0 ]->parents.size() )
                write_save_op_somewhere_and_clear_reg( op->children[0] );
        } else {
            op->reg = get_free_reg(); regs[ op->reg ] = op;
            write_get_op_in_reg( op->reg, op->children[0] );
        }
        write_cmpsd( op->reg, op->children[1], cmp_op );
        write_andsd( op->reg, op->children[2] );
    }
    
    void write_pos_part( int reg, OpWithSeq *op ) {
        write_save_op_in_xmm( reg, op->children[ 0 ] );
        OpWithSeq *ch = op->children[ 1 ];
        //
        #ifdef DEBUG_ASM
            std::cout << "    maxsd   xmm" << reg << ", " << ch->asm_str() << std::endl;
        #endif
        os.push_back( 0xf2 );
        if ( reg >= 8 or ch->reg >= 8 )
            os.push_back( 0x40 | 4 * ( reg >= 8 ) | ( ch->reg >= 8 ) );
        os.push_back( 0x0f );
        os.push_back( 0x5f );
        if ( ch->reg >= 0 ) {
            os.push_back( 0xc0 | 8 * ( reg % 8 ) | ( ch->reg % 8 ) );
        } else {
            assert( ch->stack >= 0 );
            os.push_back( 0x84 | 8 * ( reg % 8 ) );
            os.push_back( 0x24 );
            *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = ch->stack * T_size;
        }
    }
    
    void write_cmpsd( int reg, OpWithSeq *ch, unsigned char cmp_op ) {
        #ifdef DEBUG_ASM
            std::cout << "    cmpsd   xmm" << reg << ", " << ch->asm_str() << ", " << int(cmp_op) << std::endl;
        #endif
        os.push_back( 0xf2 );
        if ( reg >= 8 or ch->reg >= 8 )
            os.push_back( 0x40 | 4 * ( reg >= 8 ) | ( ch->reg >= 8 ) );
        os.push_back( 0x0f );
        os.push_back( 0xc2 );
        if ( ch->reg >= 0 ) {
            os.push_back( 0xc0 | 8 * ( reg % 8 ) | ( ch->reg % 8 ) );
        } else {
            assert( ch->stack >= 0 );
            os.push_back( 0x84 | 8 * ( reg % 8 ) );
            os.push_back( 0x24 );
            *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = ch->stack * T_size;
        }
        os.push_back( cmp_op );
    }
    
    void write_andsd( int reg, OpWithSeq *ch ) {
        if ( ch->reg < 0 and ch->stack & 1 ) {
            assert( not ch->ptr_val );
            int n_reg = get_free_reg( reg );
            write_save_op_in_xmm( n_reg, ch );
            ch->reg = n_reg;
        }
        
        //
        #ifdef DEBUG_ASM
            std::cout << "    andpd   xmm" << reg << ", " << ch->asm_str() << std::endl;
        #endif
        
        //
        os.push_back( 0x66 );
        if ( reg >= 8 or ch->reg >= 8 )
            os.push_back( 0x40 | 4 * ( reg >= 8 ) | ( ch->reg >= 8 ) );
        os.push_back( 0x0f );
        os.push_back( 0x54 );
        if ( ch->reg >= 0 ) {
            os.push_back( 0xc0 | 8 * ( reg % 8 ) | ( ch->reg % 8 ) );
        } else {
            assert( ch->stack >= 0 );
            os.push_back( 0x84 | 8 * ( reg % 8 ) );
            os.push_back( 0x24 );
            *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = ch->stack * T_size;
        }
    }
    
    void write_assign_number_M1_to_offset_from_stack_pointer( int offset_in_stack ) {
        #ifdef DEBUG_ASM
            std::cout << "    mov   rax, ~( 1L << 63 )" << std::endl;
            std::cout << "    mov   [ rsp + " << offset_in_stack << " ], rax" << std::endl;
        #endif
        os.push_back( 0x48 );
        os.push_back( 0xb8 );
        *reinterpret_cast<long long *>( os.get_room_for( 8 ) ) = ~( 1L << 63 );
        
        os.push_back( 0x48 );
        os.push_back( 0x89 );
        os.push_back( 0x84 );
        os.push_back( 0x24 );
        *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = offset_in_stack;
    }
    
    void write_assign_number_to_offset_from_stack_pointer( int offset_in_stack, Float64 val ) {
        #ifdef DEBUG_ASM
            std::cout << "    mov   rax, " << val << std::endl;
            std::cout << "    mov   [ rsp + " << offset_in_stack << " ], rax" << std::endl;
        #endif
        os.push_back( 0x48 );
        os.push_back( 0xb8 );
        *reinterpret_cast<Float64 *>( os.get_room_for( sizeof(Float64) ) ) = val;
        
        os.push_back( 0x48 );
        os.push_back( 0x89 );
        os.push_back( 0x84 );
        os.push_back( 0x24 );
        *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = offset_in_stack;
    }
    
    void write_assign_to_reg_from_offset_from_stack_pointer( int reg, int offset_in_stack ) {
        #ifdef DEBUG_ASM
            std::cout << "    movsd xmm" << reg << ", [ rsp + " << offset_in_stack << " ]" << std::endl;
        #endif
        os.push_back( 0xf2 );
        if ( reg >= 8 )
            os.push_back( 0x44 );
        os.push_back( 0x0f );
        os.push_back( 0x10 );
        os.push_back( 0x84 | ( 8 * ( reg % 8 ) ) );
        os.push_back( 0x24 );
        *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = offset_in_stack;
    }
    
    void write_save_registers_from_stack() {
        static const unsigned char data[] = {
            0x48, 0x81, 0xec, 0x88, 0x00, 0x00, 0x00,      // sub   rsp, 8 * 16
            0xf2, 0x0f, 0x11, 0x04, 0x24,                  // movsd [ rsp + 0 * 8 ], xmm0
            0xf2, 0x0f, 0x11, 0x4c, 0x24, 0x08,            // movsd [ rsp + 1 * 8 ], xmm1
            0xf2, 0x0f, 0x11, 0x54, 0x24, 0x10,            // movsd [ rsp + 2 * 8 ], xmm2
            0xf2, 0x0f, 0x11, 0x5c, 0x24, 0x18,            // movsd [ rsp + 3 * 8 ], xmm3
            0xf2, 0x0f, 0x11, 0x64, 0x24, 0x20,            // movsd [ rsp + 4 * 8 ], xmm4
            0xf2, 0x0f, 0x11, 0x6c, 0x24, 0x28,            // movsd [ rsp + 5 * 8 ], xmm5
            0xf2, 0x0f, 0x11, 0x74, 0x24, 0x30,            // movsd [ rsp + 6 * 8 ], xmm6
            0xf2, 0x0f, 0x11, 0x7c, 0x24, 0x38,            // movsd [ rsp + 7 * 8 ], xmm7
            0xf2, 0x44, 0x0f, 0x11, 0x44, 0x24, 0x40,      // movsd [ rsp + 8 * 8 ], xmm8
            0xf2, 0x44, 0x0f, 0x11, 0x4c, 0x24, 0x48,      // movsd [ rsp + 9 * 8 ], xmm9
            0xf2, 0x44, 0x0f, 0x11, 0x54, 0x24, 0x50,      // movsd [ rsp + 10* 8 ], xmm10
            0xf2, 0x44, 0x0f, 0x11, 0x5c, 0x24, 0x58,      // movsd [ rsp + 11* 8 ], xmm11
            0xf2, 0x44, 0x0f, 0x11, 0x64, 0x24, 0x60,      // movsd [ rsp + 12* 8 ], xmm12
            0xf2, 0x44, 0x0f, 0x11, 0x6c, 0x24, 0x68,      // movsd [ rsp + 13* 8 ], xmm13
            0xf2, 0x44, 0x0f, 0x11, 0x74, 0x24, 0x70,      // movsd [ rsp + 14* 8 ], xmm14
            0xf2, 0x44, 0x0f, 0x11, 0x7c, 0x24, 0x78,      // movsd [ rsp + 15* 8 ], xmm15
            0x48, 0x89, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00 // mov    %rax,0x80(%rsp)
        };
        const unsigned data_size = 0x76;
        for(unsigned i=0;i<data_size;++i)
            os.push_back( data[ i ] );
    }
    void write_load_registers_from_stack() {
        static const unsigned char data[] = {
            0xf2, 0x0f, 0x10, 0x04, 0x24,                   // movsd xmm0 , [ rsp + 0 * 8 ]
            0xf2, 0x0f, 0x10, 0x4c, 0x24, 0x08,             // movsd xmm1 , [ rsp + 1 * 8 ]
            0xf2, 0x0f, 0x10, 0x54, 0x24, 0x10,             // movsd xmm2 , [ rsp + 2 * 8 ]
            0xf2, 0x0f, 0x10, 0x5c, 0x24, 0x18,             // movsd xmm3 , [ rsp + 3 * 8 ]
            0xf2, 0x0f, 0x10, 0x64, 0x24, 0x20,             // movsd xmm4 , [ rsp + 4 * 8 ]
            0xf2, 0x0f, 0x10, 0x6c, 0x24, 0x28,             // movsd xmm5 , [ rsp + 5 * 8 ]
            0xf2, 0x0f, 0x10, 0x74, 0x24, 0x30,             // movsd xmm6 , [ rsp + 6 * 8 ]
            0xf2, 0x0f, 0x10, 0x7c, 0x24, 0x38,             // movsd xmm7 , [ rsp + 7 * 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x44, 0x24, 0x40,       // movsd xmm8 , [ rsp + 8 * 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x4c, 0x24, 0x48,       // movsd xmm9 , [ rsp + 9 * 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x54, 0x24, 0x50,       // movsd xmm10, [ rsp + 10* 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x5c, 0x24, 0x58,       // movsd xmm11, [ rsp + 11* 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x64, 0x24, 0x60,       // movsd xmm12, [ rsp + 12* 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x6c, 0x24, 0x68,       // movsd xmm13, [ rsp + 13* 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x74, 0x24, 0x70,       // movsd xmm14, [ rsp + 14* 8 ]
            0xf2, 0x44, 0x0f, 0x10, 0x7c, 0x24, 0x78,       // movsd xmm15, [ rsp + 15* 8 ]
            0x48, 0x8b, 0x84, 0x24, 0x80, 0x00, 0x00, 0x00, // mov    0x80(%rsp),%rax
            0x48, 0x81, 0xc4, 0x88, 0x00, 0x00, 0x00        // add   rsp, 8 * 16
        };
        const unsigned data_size = 0x76;
        for(unsigned i=0;i<data_size;++i)
            os.push_back( data[ i ] );
    }
    
    int *write_sub_stack_pointer() {
        // sub rsp, stack_size
        os.push_back( 0x48 ); os.push_back( 0x81 ); os.push_back( 0xec );
        return reinterpret_cast<int *>( os.get_room_for( 4 ) );
    }
    
    int *write_add_stack_pointer() {
        // add rsp, ...
        os.push_back( 0x48 ); os.push_back( 0x81 ); os.push_back( 0xc4 );
        return reinterpret_cast<int *>( os.get_room_for( 4 ) );
    }
    
    void write_ret_instr() {
        os.push_back( 0xc3 );
    }
    
    void write_exit_code() {
        os.push_back( 0x31 ); os.push_back( 0xff );
        os.push_back( 0xb8 ); os.push_back( 0x3c ); os.push_back( 0x00 ); os.push_back( 0x00 ); os.push_back( 0x00 );
        os.push_back( 0x0f ); os.push_back( 0x05 );
    }
    
    
    void write_save_op_in_memory( void *ptr_res, OpWithSeq *ch ) {
        #ifdef DEBUG_ASM
            std::cout << "    mov   rax, " << ptr_res << std::endl;
            if ( ch->reg >= 0 ) {
                std::cout << "    movsd [ rax ], xmm" << ch->reg << std::endl;
            } else if ( ch->stack >= 0 ) {
                std::cout << "    movsd [ rsp - 8 ], xmm0" << std::endl;
                std::cout << "    movsd xmm0, [ rsp + " << ch->stack * T_size << " ]"<< std::endl;
                std::cout << "    movsd [ rax ], xmm0" << std::endl;
                std::cout << "    movsd xmm0, [ rsp - 8 ]"<< std::endl;
            } else
                assert( 0 );
        #endif
        
        // mov rax, ptr_re
        os.push_back( 0x48 );
        os.push_back( 0xb8 );
        *reinterpret_cast<void **>( os.get_room_for( sizeof(void *) ) ) = ptr_res;
        
        //
        if ( ch->reg >= 0 ) {
            // movsd [ rax ], xmm.
            os.push_back( 0xf2 );
            if ( ch->reg >= 8 )
                os.push_back( 0x44 );
            os.push_back( 0x0f );
            os.push_back( 0x11 );
            os.push_back( 8 * ( ch->reg % 8 ) );
        } else if ( ch->stack >= 0 ) {
            // movsd [ rsp - 8 ], xmm0
            os.push_back( 0xf2 ); os.push_back( 0x0f ); os.push_back( 0x11 ); os.push_back( 0x44 ); os.push_back( 0x24 ); os.push_back( 0xf8 );
            // movsd xmm0, [ rsp + ch->stack * T_size ]
            os.push_back( 0xf2 ); os.push_back( 0x0f ); os.push_back( 0x10 ); os.push_back( 0x84 ); os.push_back( 0x24 );
            *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = ch->stack * T_size;
            // movsd [ rax ], xmm0
            os.push_back( 0xf2 ); os.push_back( 0x0f ); os.push_back( 0x11 ); os.push_back( 0x00 );
            // movsd xmm0, [ rsp - 8 ]
            os.push_back( 0xf2 ); os.push_back( 0x0f ); os.push_back( 0x10 ); os.push_back( 0x44 ); os.push_back( 0x24 ); os.push_back( 0xf8 );
        } else {
            assert( 0 );
        }
        
    }
    
    void write_save_xmm_in_stack_plus_offset( int reg, int offset ) {
        #ifdef DEBUG_ASM
            std::cout << "    movsd  [ rsp + " << offset << " ], xmm" << reg << std::endl;
        #endif
        os.push_back( 0xf2 );
        if ( reg >= 8 )
            os.push_back( 0x44 );
        os.push_back( 0x0f );
        os.push_back( 0x11 );
        os.push_back( 0x84 | ( 8 * ( reg % 8 ) ) );
        os.push_back( 0x24 );
        *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = offset;
    }
    
    void write_save_memory_in_xmm( int reg, void *ptr_val, OpWithSeq *orig = NULL /*for debug purpose*/ ) {
        #ifdef DEBUG_ASM
            std::cout << "    mov   rax, " << ptr_val << " ; ";
            if ( orig )
                std::cout << *orig;
            std::cout << std::endl;
            std::cout << "    movsd xmm" << reg << ", [ rax ]" << std::endl;
        #endif
        
        os.push_back( 0x48 );
        os.push_back( 0xb8 );
        *reinterpret_cast<void **>( os.get_room_for( sizeof(void *) ) ) = ptr_val;
        
        os.push_back( 0xf2 );
        if ( reg >= 8 )
            os.push_back( 0x44 );
        os.push_back( 0x0f );
        os.push_back( 0x10 );
        os.push_back( 8 * ( reg % 8 ) );
    }
    
    void write_copy_xmm_regs( int reg0, int reg1 ) {
        if ( reg0 == reg1 )
            return;
        #ifdef DEBUG_ASM
            std::cout << "    movsd  xmm" << reg0 << ", xmm" << reg1 << std::endl;
        #endif
        /*
            f2 0f 10 c0             movsd  xmm0, xmm0
            f2 41 0f 10 c0          movsd  xmm0, xmm8
            f2 44 0f 10 c0          movsd  xmm8, xmm0
            f2 45 0f 10 c0          movsd  xmm8, xmm8
        */
        os.push_back( 0xf2 );
        if ( reg0 >= 8 or reg1 >= 8 )
            os.push_back( 0x40 | 4 * ( reg0 >= 8 ) | ( reg1 >= 8 ) );
        os.push_back( 0x0f );
        os.push_back( 0x10 );
        os.push_back( 0xc0 | 8 * ( reg0 % 8 ) | ( reg1 % 8 ) );
    }
    
    void write_save_stack_in_xmm( int reg, int stack_offset ) {
        #ifdef DEBUG_ASM
            std::cout << "    movsd  xmm" << reg << ", [ rsp + " << stack_offset << " ]" << std::endl;
        #endif
        os.push_back( 0xf2 );
        if ( reg >= 8 )
            os.push_back( 0x44 );
        os.push_back( 0x0f );
        os.push_back( 0x10 );
        os.push_back( 0x84 | ( 8 * ( reg % 8 ) ) );
        os.push_back( 0x24 );
        *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = stack_offset;
    }
    
    void write_save_op_in_xmm( int reg, OpWithSeq *ch ) {
        if ( reg == ch->reg )
            return;
        //
        if ( ch->reg >= 0 )
            write_copy_xmm_regs( reg, ch->reg );
        else {
            assert( not ch->ptr_val );
            write_save_stack_in_xmm( reg, ch->stack * T_size );
        }
    }

    void write_self_sse2_op_xmm( int reg, OpWithSeq *ch, unsigned char sse2_op ) {
        #ifdef DEBUG_ASM
            if ( ch->reg < 0 and ch->stack < 0 )
                std::cout << "    mov    rax, " << ch->ptr_val << " ; " << *ch << std::endl;
            switch ( sse2_op ) {
                case 0x58: std::cout << "    add"; break;
                case 0x59: std::cout << "    mul"; break;
                case 0x5c: std::cout << "    sub"; break;
                case 0x5e: std::cout << "    div"; break;
                case 0x51: std::cout << "    sqrt"; break;
            }
            std::cout << "sd  xmm" << reg << ", ";
            if ( ch->reg >= 0 )         std::cout << "xmm" << ch->reg << std::endl;
            else if ( ch->stack >= 0 )  std::cout << "[ rsp + " << ch->stack * T_size << " ]" << std::endl;
            else if ( ch->ptr_val )     std::cout << "[ rax ] " << std::endl;
        #endif
        /*
             f2 0f 58 c0                 addsd  xmm0, xmm0
             f2 0f 58 c1                 addsd  xmm0, xmm1
             f2 0f 58 c9                 addsd  xmm1, xmm1
             f2 41 0f 58 c0              addsd  xmm0, xmm8
             f2 44 0f 58 c0              addsd  xmm8, xmm0
             f2 45 0f 58 c0              addsd  xmm8, xmm8
             f2 0f 58 c0                 addsd  xmm0, xmm0
             f2 0f 58 84 24 11 11 00 00  addsd  xmm0, [ rsp + 0x1111 ]
             f2 0f 58 8c 24 11 11 00 00  addsd  xmm1, [ rsp + 0x1111 ]
             f2 44 0f 58 84 24 11        addsd  xmm8, [ rsp + 0x1111 ]
             
             f2 44 0f 58 04 25 11 ...    addsd  xmm8, [ rax ]
        */
        if ( ch->reg < 0 and ch->stack < 0 ) {
            assert( ch->ptr_val );
            os.push_back( 0x48 );
            os.push_back( 0xb8 );
            *reinterpret_cast<void **>( os.get_room_for( sizeof(void *) ) ) = ch->ptr_val;
        }
        
        os.push_back( 0xf2 );
        if ( reg >= 8 or ch->reg >= 8 )
            os.push_back( 0x40 | 4 * ( reg >= 8 ) | ( ch->reg >= 8 ) );
        os.push_back( 0x0f );
        os.push_back( sse2_op );
        if ( ch->reg >= 0 )
            os.push_back( 0xc0 | 8 * ( reg % 8 ) | ( ch->reg % 8 ) );
        else if ( ch->stack >= 0 ) {
            os.push_back( 0x84 | 8 * ( reg % 8 ) );
            os.push_back( 0x24 );
            *reinterpret_cast<int *>( os.get_room_for( 4 ) ) = ch->stack * T_size;
        } else {
            assert( ch->ptr_val );
            os.push_back( 0x00 | 8 * ( reg % 8 ) );
        }
    }
    
    int *stack_size; // sub rsp, $stack_size at the beginning of the code
};

#endif // OPWITHSEQASMGENERATOR_H

