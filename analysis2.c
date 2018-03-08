/*
 *
 * Arm (thumb) instruction anaylyasis tools
 * author: L0phTg
 * data:   2018-02-20
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


#define REG_MAX 16

#define VM_GETW(x)       ((x << 1)&0x60) // 取OPCODE 位宽
#define VM_OPERAND_ISREG(x) (x & 0x40)   // 操作数是否为寄存器
#define VM_GETREG(x)     (x & 0x1f)      // 获取REGId
#define VM_GETIMMSIZE(x) (x & 0x1f)      // 获取imm长度
#define VM_IMM_EXT(x)    (x & 0x20)      // IMM后面是否有operand
#define VM_SET_FLAG(x)   (x & 0x10)      // 是否影响标志位

//#define len2hex(x)       (0xFF<<(8-x) & 0xFF)
#define VM_EXTEND(x)     (x & 0xF0 == 0xC0 || x & 0xF0 == 0x80)   //  指令是否扩展   
#define VM_GET_BIT_VALUE(x, i) (x>>i & 1)
#define VM_GET_OPCODE(x, len) (x & (0xFF<<(8-len) & 0xFF))         // 获取Opcode(依照len)
#define VM_GET_REG(x, start, len) (x>>start & 0xFF>>(8-len))
#define VM_UPDATE_FLAG(x) (x & 0x01)
//#define VM_GETREG(x)
#define IF_NEGATIVE(x, len) (x >> len-1)   // 是否是负数
#define IS_ZERO_BIT(x) (x == 0)

#define NOT(x)   (-x)

//#defin GEN_REGS_LIST(m, list) (


enum vm_error_code
{
    VM_ERROR_UNDEF,
    VM_ERROR_DIV,
    VM_ERROR_REG,
    VM_ERROR_MEM
};

enum OPCODES24
{
    CMP = 0x00,
    MOV = 0x02,
    ADD = 0x04,
    SUB = 0x06,
};

enum OPCODE16
{
    PUSH   = 0x04,
    POP    = 0x06,

    STM    = 0x10,
    LDM    = 0x18,

    rSTR   = 0x40,
    rSTRH  = 0x42,
    rSTRB  = 0x44,
    rLDRSB = 0x46,
    rLDR   = 0x48,
    rLDRH  = 0x4A,
    rLDRB  = 0x4C,
    rLDRSH = 0x4E,

    iSTR   = 0x50,
    iSTRH  = 0x70,
    iSTRB  = 0x60,
    iLDR   = 0x58,
    iLDRH  = 0x78,
    iLDRB  = 0x68,

    LDRPC  = 0x90,
    LDRSP  = 0xA0, 
    STRSP  = 0xB0,

    B      = 0x40,
    BX     = 0x50,
    BLX    = 0x50,

    RET    = 0xFF,
};

typedef struct vm_context_t
{
    uint32_t regs[REG_MAX];

    uint8_t *PC;
    uint32_t error_pc;
    vm_error_code error_code;
    //char *bytecode;
    uint32_t *SP;
    uint32_t LR;

    bool flag_z;
    bool flag_n;
    bool flag_c;
    bool flag_v;
} vm_context;


char BitCount(uint16_t registers)
{
    char result = 0;
    for (int i = 0; i < sizeof(registers)*8; i++)    
        if (registers & (1<<i))
            result = result + 1
    return result;
}

uint32_t Align(uint8_t *address, uint8_t align)
{
    return align * (address / align);
}

uint32_t vm_align_address(uint32_t address, uint8_t align)
{
    if (address % 4 == 0)
        return address;
    else 
        return (address - address%4);
}

void vm_set_result_flag(vm_context *vm, uint32_t value)
{
    int signed_value = (int)value; 

    vm->flag_z = value == 0;
    vm->flag_n = signed_value < 0;
}

void vm_add_with_carry(uint32_t a, uint32_t b, char carry1, uint32_t *result, char *carry_out, char *overflow_out)
{
    char carry = 0;
    char overflow = 0;
    long long a64, b64, result64;
    a64 = a;
    b64 = b;
    result64 = a64;
    result64 += b64;
    result64 += carry1;
    if (result64 & 0x100000000)
        carry1 = 1;
    uint32_t unsigned_result = a+b+carry1;
    int32_t signed_result = (int32_t)a + (int32_t)b + carry1;
    overflow = unsigned_result != signed_result;
    *result = unsigned_result;
    *carry_out = carry;
    *overflow_out = overflow;
}

//void vm_sub_with_carry(uint32_t a, uint3

void AddWithCarry()
{


}

void vm_mov_func(vm_context *proc)
{
    unsigned char dest = *(proc->PC + 1) & 0x0F;
    int8_t src  = (int8_t)*(proc->PC + 2);
    unsigned char flag = *proc->PC & 0x01;

    unsigned char rOri = *(proc->PC + 1) & 0x40;

    uint32_t result;

    if (rOri) {
        proc->regs[dest] = proc->regs[src];      // dest = src
        result = proc->regs[src];
    }
    else {
        proc->regs[dest] = src;
        result = src;
    }

    if (flag) {
        //proc->flag_n = result>>31;
        //proc->flag_z = IS_ZERO_BIT(result);
        vm_set_result_flag(proc, result);
    }
    //vm_set_result_flag(proc, result);
}

void vm_cmp_func(vm_context *proc)
{
    unsigned char dest = VM_GET_REG(*(proc->PC+1), 0, 4);
    int8_t src = VM_GET_REG(*(proc->PC+2), 0, 8);
    unsigned char rOri = *(proc->PC+1) &0x40;
    uint32_t result;

    if (rOri) {                    // cmp reg, reg
        result = proc->regs[dest] - proc->regs[src];
        vm_set_result_flag(proc, result); 
    }
    else {                         // cmp reg, imm8
        result = proc->regs[dest] - src;
        vm_set_result_flag(proc, result);
    }
}

// add操作
// add Rd, Rm                  Rd = Rd + Rm
// add Rd, #<imm8>              
void vm_add_func(vm_context *proc)
{
    unsigned char dest = VM_GET_REG(*(proc->PC+1), 0, 4);
    uint32_t src = VM_GET_REG(*(proc->PC+2), 0, 8);
    unsigned char rOri = *(proc->PC+1) &0x40;
    unsigned char flag = *proc->PC & 0x01;
    unsigned char flag_c, flag_v;

    if (rOri) {
        vm_add_with_carry(proc->regs[dest], proc->regs[src], proc->flag_c, &proc->regs[dest], &flag_c, &flag_v);
    }
    else {
        vm_add_with_carry(proc->regs[dest], src, proc->flag_c, &proc->regs[dest], &flag_c, &flag_v);
    }

    if (flag) {
        proc->flag_c = flag_c;
        proc->flag_v = flag_v;
        vm_set_result_flag(proc, proc->regs[dest]);
    }
}

void vm_sub_func(vm_context *proc)
{
    unsigned char dest = VM_GET_REG(*(proc->PC+1), 0, 4);
    uint32_t src = VM_GET_REG(*(proc->PC+2), 0, 8);
    unsigned char rOri = *(proc->+1) & 0x40;
    unsigned char flag = *proc->PC & 0x01;
    unsigned char flag_c, flag_v;

    if (rOri) {
        vm_add_with_carry(proc->regs[dest], NOT(proc->regs[src]), proc->flag_c, &proc->regs[dest], &flag_c, &flag_v);
    } 
    else {
        vm_add_with_carry(proc->regs[dest], NOT(src), proc->flag_c, &proc->regs[dest], &flag_c, &flag_v);
    }

    if (flag) {
        proc->flag_c = flag_c;
        proc->flag_v = flag_v;
        vm_set_result_flag(proc, proc->regs[dest]);
    }
}

void vm_mul_func(vm_context *proc)
{


}

void vm_div_func(vm_context *proc)
{

}

void vm_push_func(vm_context *proc)
{
    uint8_t register_list = *(proc->PC+1);
    uint16_t registers = (*proc->PC & 0x01)<<8 | register_list;
    uint32_t address = proc->SP - 4 *BitCount(registers);

    for(int i = 0; i < 15; i++) {
        if(VM_GET_BIT_VALUE(registers, i) == 1) {
            (uint32_t)*(address) = proc->regs[i];

            address = address + 4;
        }
    }
    proc->SP = proc->SP - 4 * BitCount(registers);
}

void vm_pop_func(vm_context *proc)
{
    uint8_t register_list = *(proc->PC+1);
    uint16_t registers = (*proc->PC & 0x01)<<7 | register_list;
    uint32_t address = proc->SP;
               
    for (int i = 0; i < 15; i++) {
        if(VM_GET_BIT_VALUE(registers, i) == 1) {          
            proc->regs[i] = *(address);
            address = address + 4
        }
    } 

    if (proc->regs[15]) {
        //LoadWritePC(*(address));
        proc->PC = *(address);
    }
}


void vm_stm_func(vm_context *proc)
{


}

void vm_ldm_func(vm_context *proc)
{


}

void vm_str_Reg_func(vm_context *proc)
{
    uint8_t Rt =  *(proc->PC) & 0x01 | *(proc->PC+1)&0xC0 >>6;
    uint8_t Rn = *(proc->PC+1)&0x38;
    uint8_t Rm = *(proc->PC+1)&0x07;
    switch(VM_GETOPCODE(*proc->PC, 7))
    {
    case rSTR:
        *(proc->regs[Rn] + proc->regs[Rm]) = proc->regs[Rt];
        break;
    case rSTRH:
        *(uint16_t*)(proc->regs[Rn] + proc->regs[Rm]) = (uint16_t)(proc->regs[Rt]);
        break;

    case rSTRB:
        *(uint8_t*)(proc->regs[Rn] + proc->regs[Rm]) = (uint8_t)proc->regs[Rt];
        break;
    }
}

void vm_str_imm_func(vm_context *proc)
{
    uint8_t Rt   = (*proc->PC)&0x03;
    uint8_t Rn   = (*proc->PC+1)>>5 & 0x03;
    uint32_t imm = (*proc->PC+1) & 0x1F;
    uint32_t offset_addr = proc->regs[Rn] + imm;
    uint32_t address = offset_addr;
     
    switch(VM_GETOPCODE(*proc->PC, 5))
    {
    case iSTR:
        *(uint32_t*)(address) = proc->regs[Rt];
        break;
    case iSTRH:
        *(uint16_t*)(address) = (uint16_t)proc->regs[Rt];
        break;
    case iSTRB:
        *(uint8_t*)(address)  = (uint8_t) proc->regs[Rt];
        break;
    }
}

void vm_ldr_Reg_func(vm_context *proc)
{
    uint8_t Rt =  *(proc->PC) & 0x01 | *(proc->PC+1)&0xC0 >>6;
    uint8_t Rn = *(proc->PC+1)&0x38;
    uint8_t Rm = *(proc->PC+1)&0x07;
    switch(VM_GETOPCODE(*proc->PC, 7))
    {
    case rLDR:
        uint32_t data = *(uint32_t*)(proc->regs[Rn]+proc->regs[Rm]);
        proc->regs[Rt] = data;
        break;

    case rLDRH:
        uint16_t data = *(uint16_t*)(proc->regs[Rn]+proc->regs[Rm]);
        proc->regs[Rt] = (uint32_t)data;
        break;

    case rLDRB:
        uint8_t data  = *(uint8_t*)(proc->regs[Rn]+proc->regs[Rm]);
        proc->regs[Rt] = (uint32_t)data;
        break;

    case rLDRSB:
        int8_t data   = *(int8_t*)(proc->regs[Rn]+proc->regs[Rm]);
        proc->regs[Rt] = (int32_t)data;
        break;

    case rLDRSH:
        int16_t data  = *(int16_t*)(proc->regs[Rn]+proc->regs[Rm]);
        proc->regs[Rt] = (int32_t)data;
        break;
    }
}

void vm_ldr_imm_func(vm_context *proc)
{
    uint8_t Rt   = (*proc->PC)&0x03;  
    uint8_t Rn   = (*proc->PC+1)>>5 & 0x03;
    uint32_t imm = ((*proc->PC+1) & 0x1F) * 4;
    uint32_t offset_addr = proc->regs[Rn] + imm;
    uint32_t address = offset_addr; 

    switch(VM_GETOPCODE(*proc->PC, 5))
    {
    case iLDR:
        uint32_t data = *(uint32_t*)(address);
        proc->regs[Rt] = data;
        break;

    case iLDRH:
        uint16_t data = *(uint16_t*)(address);
        proc->regs[Rt] = (uint32_t)data;
        break;

    case iLDRB:
        uint8_t  data = *(uint8_t*)(address);
        proc->regs[Rt] = (uint32_t)data;
        break;
    }
}

void vm_ldrsp_ldrpc_strsp_func(vm_context *proc)
{
    uint8_t Rt = (*proc->PC & 0F);
    uint32_t imm = (uint32_t)*(proc->PC+1)*4;

    switch(VM_GETOPCODE(*proc->PC, 4))
    {
    case LDRPC:
        uint32_t base = Align(proc->PC, 4)
        uint32_t address = base+imm;
        uint32_t data = *(uint32_t*)(address);
        proc->regs[Rt] = data;
        break;

    case LDRSP:
        uint8_t Rn = 13;
        uint32_t address = proc->regs[Rn] + imm;
        uint32_t data = *(uint32_t*)(address);
        proc->regs[Rt] = data;
        break;

    case STRSP:
        uint8_t Rn = 13; 
        uint32_t address = proc->regs[Rn] + imm; 
        (uint32_t*)(address) = proc->regs[Rt];
        break;
    }

}


void vm_xor_func(vm_context *proc)
{



}

void vm_and_func(vm_context *proc)
{


}

void vm_or_func(vm_context *proc)
{


}

void vm_lsl_func(vm_context *proc)
{


}

void vm_lsr_func(vm_context *proc)
{


}

void vm_asr_func(vm_context *proc)
{


}



/*
 * 执行字节码对应的handle
 */
void exec_Handle(vm_processor *proc)
{
    int flag = 0;
    int i = 0;

        // 24位指令
    if (VM_EXTEND(*proc->PC))
    {
        proc->PC = proc->PC + 1;
        switch(VM_GETOPCODE(*proc->PC, 7))
        {
        case CMP:
            vm_cmp_func(proc);
            break;

        case MOV:
            vm_mov_func(proc);
            break;

        case ADD:
            vm_add_func(proc);
            break;

        case SUB:
            vm_sub_func(proc);
            break;
        }
    }
    else {
    // 16位指令

        // 处理Opcode of 7 bit
        switch(VM_GETOPCODE(*proc->PC, 7))
        {
        case PUSH:
            vm_push_func(proc);
            break;

        case POP:
            vm_pop_func(proc);
            break;

        case rSTR:
        case rSTRH:
        case rSTRB:
            vm_str_Reg_func(proc);
            break;

        case rLDR:
        case rLDRH:
        case rLDRB:
        case rLDRSB:
        case rLDRSH:
            vm_ldr_Reg_func(proc);
            break;
        }

        // 处理Opcode of 5 bit
        switch(VM_GETOPCODE(*proc->PC, 5))
        {
        case STM:

        case LDM:

        case iSTR: 
        case iSTRH:
        case iSTRB:
            vm_str_imm_func(proc);
            break;
        case iLDR:
        case iLDRH:
        case iLDRB:
            vm_ldr_imm_func(proc);
            break;
        }

        // 处理Opcode of 4 bit
        switch(VM_GETOPCODE(*proc->PC, 4))
        {
        case LDRPC:
        case LDRSP:
        case STRSP:
            vm_ldrsp_ldrpc_strsp_func(proc);
            break;

        case B:
        }

        if (*proc->PC == 0x08)
        {
            // blx
            if (*(proc->PC - 1) >> 1) {

            }  // bx
            else {

                
            }
        }

    }
}


/*
 * 虚拟机的解释器
 */
void vm_CPU(vm_processor *proc, unsigned char* Vcode)
{
    /*
     * PC指向被保护代码的第一个字节
     */

    proc->PC = Vcode;

    while (*proc->PC != RET) {
        exec_Handle(proc);
    }
}

unsigned char* Vcode = {



};

void copy_regs(vm_context *proc)
{
    

}

void init_vm_enviroment()
{
    // 将全部的寄存器存储并放入processor中 
    vm_context proc;
    copy_regs(proc);
    vm_CPU(proc, Vcode);
}


int main()
{



    return 0;
}
