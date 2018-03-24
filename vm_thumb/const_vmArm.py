#!/usr/bin/env python
#!-*- coding:utf-8 -*-

VM_OP_PUSH    = "0000010"
VM_OP_POP     = "0000011"   


 
DATA_OP    =    {
                        "cmp"  : "VM_OP_CMP",  # 24位指令
                        "mov"  : "VM_OP_MOV",
                        "add"  : "VM_OP_ADD",
                        "sub"  : "VM_OP_SUB",
                        "div"  : "VM_OP_DIV",
                        "mul"  : "VM_OP_MUL",
                        "adc"  : "VM_OP_ADC",

                        "lsl"  : "VM_OP_LSL",
                        "lsr"  : "VM_OP_LSR", 
                        "asr"  : "VM_OP_ASR",
    
                        "and"  : "VM_OP_AND",
                        "or"   : "VM_OP_OR",
                        "eor"  : "VM_OP_EOR",
                        "sbc"  : "VM_OP_SBC",
                        "ror"  : "VM_OP_ROR",

                        "push"  : "VM_OP_PUSH", # 16位指令
                        "pop"   : "VM_OP_POP",

                        "ldrliteral" : "VM_OP_LDRLITERAL",

                        "str"   : "VM_OP_STR",  
                        "strh"  : "VM_OP_STRH",
                        "strb"  : "VM_OP_STRB",
                        "ldr"   : "VM_OP_LDR",
                        "ldrh"  : "VM_OP_LDRH",
                        "ldrb"  : "VM_OP_LDRB",
                        "ldrsb" : "VM_OP_LDRSB",
                        
                        "b"     : "VM_OP_JUMP",
                        "bx"    : "VM_OP_BX",
                        "bl"    : "VM_OP_BL",
                        "blx"   : "VM_OP_BLX",


                        "stm"   : "VM_OP_STM",
                        "ldm"   : "VM_OP_LDM",


                        "it"    : "VM_OP_IT",
      #                  "nop"   : "VM_OP_NOP",
      #                  "yield" : "VM_OP_YIELD",
      #                  "wfe"   : "VM_OP_WFE",
      #                  "wfi"   : "VM_OP_WFI",
      #                  "sev"   : "VM_OP_SEV",
      #                  "sevl"  : "VM_OP_SEVL",
                    }

DATA_OP_BYTE = {
        "VM_OP_CMP"    :  "0000000",
        "VM_OP_MOV"    :  "0000001",
                    
        "VM_OP_ADD"    :  "0000010",
        "VM_OP_SUB"    :  "0000011",

        "VM_OP_ADC"    :  "0000100",
        "VM_OP_SBC"    :  "0000101",

        "VM_OP_DIV"    :  "0000110",
        "VM_OP_MUL"    :  "0000111",
}

#当高位为'1100'或'1000'时, 扩展为24位
#        '01xx'时为 str/ldr
DATA_OP_BYTE16 = {
        "VM_OP_PUSH"   :  "0000010",
        "VM_OP_POP"    :  "0000011",   

        "VM_OP_JUMP"   :  "0010",
        "VM_OP_B"      :  "0010",
        "VM_OP_BL"     :  "",
        "VM_OP_BX"     :  "000000000",
        "VM_OP_BLX"    :  "000000001",
        
        "VM_OP_LDRLITERAL" : "00001",
        "VM_OP_STM"    :  "00010",
        "VM_OP_LDM"    :  "00011",


        "VM_OP_IT"     :  "11100000",
        #"VM_OP_NOP"    :  "11100001"
}



#01xx
LS_REG = {
                "VM_OP_STR"   : "0100000",          # Rt + Rn + Rm    str Rt, [<Rn>, <Rm>]
                "VM_OP_STRH"  : "0100001",          # Rt + Rn + Rm    strh <Rt>, [<Rn>, <Rm>]
                "VM_OP_STRB"  : "0100010",          # 
                "VM_OP_LDRSB" : "0100011",
                "VM_OP_LDR"   : "0100100",
                "VM_OP_LDRH"  : "0100101",
                "VM_OP_LDRB"  : "0100110",
                "VM_OP_LDRSH" : "0100111",
                }

LS_IMM = {      
                "VM_OP_STR"   : "01010",     # + imm5 + Rn + Rt
                "VM_OP_STRH"  : "01110",
                "VM_OP_STRB"  : "01100",     # + imm5 + Rn + Rt
                "VM_OP_LDR"   : "01011",
                "VM_OP_LDRH"  : "01111",     # + imm5 + Rn + Rt
                "VM_OP_LDRB"  : "01101",
                }

LS_MIS = {
                "ldrpc"       : "1001",
                "ldrsp"       : "1010",
                "strsp"       : "1011",
                }



ARM_CC_INVALID = 0
ARM_CC_EQ = 1
ARM_CC_NE = 2
ARM_CC_HS = 3
ARM_CC_LO = 4
ARM_CC_MI = 5
ARM_CC_PL = 6
ARM_CC_VS = 7
ARM_CC_VC = 8
ARM_CC_HI = 9
ARM_CC_LS = 10
ARM_CC_GE = 11
ARM_CC_LT = 12
ARM_CC_GT = 13
ARM_CC_LE = 14

VM_COND_INVALID = "1111"
VM_COND_EQ = "0000"
VM_COND_NE = "0001"                 
VM_COND_HS = "0010"
VM_COND_LO = "0011"
VM_COND_MI = "0100"
VM_COND_PL = "0101"      
VM_COND_VS = "0110"
VM_COND_VC = "0111"
VM_COND_HI = "1000"
VM_COND_LS = "1001"
VM_COND_GE = "1010"
VM_COND_LT = "1011"
VM_COND_GT = "1100"
VM_COND_LE = "1101"
VM_COND_AL = "1110"


Cond_Collection =      (
                VM_COND_INVALID, VM_COND_EQ, VM_COND_NE,
                VM_COND_HS, VM_COND_LO, VM_COND_MI,
                VM_COND_PL, VM_COND_VS, VM_COND_VC,
                VM_COND_HI, VM_COND_LS, VM_COND_GE, 
                VM_COND_LT, VM_COND_GT, VM_COND_LE
                )


#VM_OP_JMP = "0010"                        # cond + imm8
#VM_OP_BX  = ""
#VM_OP_BLX = ""

