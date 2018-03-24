#!/usr/bin/env python
#-*- coding: utf-8 -*-

from capstone import *
from capstone.arm import *

from const_vmArm import *
#import 
from xprint import *
#import capstone 
#from capstone.arm impo
#import const_vmArm
#import xprint


__all__ = [
    'Vs',
    'VsInsn',
    'CS_ARCH_ARM',
    'CS_ARCH_ARM64',
    'CS_ARCH_MIPS',
    'CS_ARCH_X86',
    'CS_ARCH_PPC',
    'CS_ARCH_SPARC',
    'CS_ARCH_SYSZ',
    'CS_ARCH_XCORE',
    'CS_ARCH_ALL',

    'CS_MODE_LITTLE_ENDIAN',
    'CS_MODE_BIG_ENDIAN',
    'CS_MODE_16',
    'CS_MODE_32',
    'CS_MODE_64',
    'CS_MODE_ARM',
    'CS_MODE_THUMB',
    'CS_MODE_MCLASS',
    'CS_MODE_MICRO',
    'CS_MODE_MIPS3',
    'CS_MODE_MIPS32R6',
    'CS_MODE_MIPSGP64',
    'CS_MODE_V8',
    'CS_MODE_V9',
    'CS_MODE_MIPS32',
    'CS_MODE_MIPS64',

]

# architectures
CS_ARCH_ARM = 0
CS_ARCH_ARM64 = 1
CS_ARCH_MIPS = 2
CS_ARCH_X86 = 3
CS_ARCH_PPC = 4
CS_ARCH_SPARC = 5
CS_ARCH_SYSZ = 6
CS_ARCH_XCORE = 7
CS_ARCH_MAX = 8
CS_ARCH_ALL = 0xFFFF

# disasm mode                   反汇编模式
CS_MODE_LITTLE_ENDIAN = 0      # little-endian mode (default mode)
CS_MODE_ARM = 0                # ARM mode
CS_MODE_16 = (1 << 1)          # 16-bit mode (for X86)
CS_MODE_32 = (1 << 2)          # 32-bit mode (for X86)
CS_MODE_64 = (1 << 3)          # 64-bit mode (for X86, PPC)
CS_MODE_THUMB = (1 << 4)       # ARM's Thumb mode, including Thumb-2
CS_MODE_MCLASS = (1 << 5)      # ARM's Cortex-M series
CS_MODE_V8 = (1 << 6)          # ARMv8 A32 encodings for ARM
CS_MODE_MICRO = (1 << 4)       # MicroMips mode (MIPS architecture)
CS_MODE_MIPS3 = (1 << 5)       # Mips III ISA
CS_MODE_MIPS32R6 = (1 << 6)    # Mips32r6 ISA
CS_MODE_MIPSGP64 = (1 << 7)    # General Purpose Registers are 64-bit wide (MIPS arch)
CS_MODE_V9 = (1 << 4)          # Sparc V9 mode (for Sparc)
CS_MODE_BIG_ENDIAN = (1 << 31) # big-endian mode
CS_MODE_MIPS32 = CS_MODE_32    # Mips32 ISA
CS_MODE_MIPS64 = CS_MODE_64    # Mips64 ISA



def copy_pyField(src):
    return src




class VsInsn(object):
    def __init__(self, vs, all_info):
        self._raw = copy_pyField(all_info)
    
    @property
    def id(self):                   # 需要处理
        return self.id

    @property
    def address(self):              # 需要处理的
        return

    @property
    def size(self):                 # 需要处理
        return

    @property
    def bytes(self):
        result = []
        _rawInsnName = self._raw.insn_name()
        _rawUpdateFlags = self._raw.update_flags

        if _rawInsnName == 'push':
            if 'lr' in self._raw.op_str:
                insn_byte = bin8toByte(VM_OP_PUSH+'1')
            else:
                insn_byte = bin8toByte(VM_OP_PUSH+'0')

            result =  [insn_byte, self._raw.bytes[0]]           # return a list
        elif _rawInsnName == 'pop':
            if 'pc' in self._raw.op_str:
                insn_byte = bin8toByte(VM_OP_POP+'1')
            else:
                insn_byte = bin8toByte(VM_OP_POP+'0')
            result =  [insn_byte, self._raw.bytes[0]]

        #elif  _rawInsnName in ['xor', 'and', 'or',  'eor']:

        #elif  _rawInsnName in ['lsl', 'lsr', 'asr']:


        elif  _rawInsnName in ['mov', 'add', 'sub', 'cmp']:     # 有两个操作数

            vm_op = DATA_OP[_rawInsnName]                            
            s = "0"
            if _rawUpdateFlags:
                s = "1"      
            Rn = self._raw.reg_name(self._raw.operands[0].reg)
      
            if self._raw.operands[1].type == ARM_OP_REG:       # all is reg

                Rm = self._raw.reg_name(self._raw.operands[1].reg)
                result =  bin24toByte(DATA_OP_BYTE[vm_op] + s + "1100" + reg2bin(Rn, 4) + "0000" + reg2bin(Rm, 4))
            else:                                              # imm exist
                imm8 = hex2int(to_x_32(self._raw.operands[1].imm))
                result =  bin24toByte(DATA_OP_BYTE[vm_op] + s + "1000" + reg2bin(Rn, 4) + int2formatBin(imm8, 8))

        elif _rawInsnName in ['str', 'strh', 'strb', 'ldr', 'ldrb', 'ldrh',
                'ldrsh', 'ldrsb', 'ldrpc', 'ldrsp', 'strsp']:          # 数据存储加载指令
            vm_op = DATA_OP[_rawInsnName]
            Rt = self._raw.reg_name(self._raw.operands[0].reg)
            Rn = self._raw.reg_name(self._raw.operands[1].mem.base)

            if self._raw.operands[1].mem.disp == 0:            # [reg, {reg}]
                if self._raw.operands[1].mem.index != 0:
                    Rm = reg2bin(self._raw.reg_name(self._raw.operands[1].mem.index), 3) 
                else: 
                    Rm = "000"
                result = bin16toByte(LS_REG[vm_op] + reg2bin(Rt, 3) + reg2bin(Rn, 3) + Rm)  
            else:                                              # [reg, imm]
                if self._raw.operands[1].mem.disp != 0:
                    #imm5 = hex2int(to_x_32(self._raw.operands[1].mem.disp))
                    imm5 = self._raw.operands[1].mem.disp
                    #sys.stdout.write(to_x_32(self._raw.operands[1].mem.disp))
                    #sys.stdout.write()
                    result = bin16toByte(LS_IMM[vm_op] + reg2bin(Rt, 3) + reg2bin(Rn, 3) + int2formatBin(imm5/4, 5)) 
                else:
                    imm8 = hex2int(to_x_32(self._raw.operands[1].mem.disp))
                    result = bin16toByte(LS_MIS[_rawInsnName+Rn] + reg2bin(Rt, 4) + int2formatBin(imm8/4, 8))
        elif _rawInsnName in ['stm', 'ldm']:
            vm_op = DATA_OP[_rawInsnName]
            Rm = self._raw.reg_name(self._raw.operands[0].reg)
            result = bin16toByte(DATA_OP_BYTE16[vm_op] + reg2bin(Rm, 3) + int2formatBin(self._raw.bytes[1], 8))
         
        elif _rawInsnName in ['it']:
            vm_op = DATA_OP[_rawInsnName]  
            #result = [bin8toByte[DATA_OP_BYTE16[vm_op]], self._raw.bytes[1]] 
            result = bin16toByte(DATA_OP_BYTE16[vm_op] + int2formatBin(self._raw.bytes[1], 8))
        elif _rawInsnName in ['b', 'bx', 'blx']:
            vm_op = DATA_OP[_rawInsnName]
            if not self._raw.cc in [ARM_CC_AL, ARM_CC_INVALID]:
                my_cond = Cond_Collection[self._raw.cc]
                imm8 =  hex2int(to_x_32(self._raw.operands[0].imm))
                result = bin16toByte(DATA_OP_BYTE16[vm_op] + my_cond + int2formatBin(imm8/4, 8))
            if _rawInsnName is 'b':
                imm8 = hex2int(to_x_32(self._raw.operands[0].imm))
                result = bin16toByte(DATA_OP_BYTE16[vm_op] + "1111" + int2formatBin(imm8/4, 8))

            elif _rawInsnName == 'bx':
                Rm = self._raw.reg_name(self._raw.operands[0].reg)
                result = bin16toByte(DATA_OP_BYTE16[vm_op] + reg2bin(Rm, 4) + "000") 

            elif _rawInsnName == 'blx':
                if self._raw.operands[0].type == ARM_OP_REG:
                    Rm = self._raw.reg_name(self._raw.operands[0].reg)
                    result = bin16toByte(DATA_OP_BYTE16[vm_op] + reg2bin(Rm, 4) + "000") 
        
        # return result 
        return result[-1::-1]        # 指令编码逆序输出, 地址从低->高地址: OpNum(低地址) ->OpCode(高地址)
                                     # 方便解析, "PC++

    @property
    def mnemonic(self):
        return "vm_"+self._raw.insn_name()+"s" if self._raw.update_flags else "vm_" + self._raw.insn_name() 

    @property
    def op_str(self):
        return self._raw.op_str



class Vs(object):
    def __init__(self, arch, mode):
        self.arch, self.mode = arch, mode

    def disasm(self, code, offset, count=0):
        md = Cs(self.arch, self.mode)
        md.detail = True
        for insn in md.disasm(code, offset):
            yield VsInsn(self, insn)





def int7bin(*numbers):    # 7 ret '111'            0~7 ----- '000' ~ '111'    
    str = ""
    for i in numbers:
        str += format(i, '03b')
    return str

def int2formatBin(oldInt, objlength):
    if oldInt >= 0:
        return  format(oldInt, 'b').zfill(objlength)
    else:
        return '1'+format(-oldInt, 'b').zfill(objlength-1)

def bin8toByte(binNumber):          
    return int(binNumber, 2)

def bin16toByte(binNumber):             # return a list
    return [int(binNumber[0:8], 2), int(binNumber[8:], 2)]

def bin24toByte(binNumber):
    return [int(binNumber[0:8], 2), int(binNumber[8:16], 2), int(binNumber[16:], 2)]


_reg2num = {            
            "r0" : 0,
            "r1" : 1,     "r4" : 4,     "r7" : 7,   "r10" : 10,   "r13" : 13,
            "r2" : 2,     "r5" : 5,     "r8" : 8,   "r11" : 11,   "r14" : 14,
            "r3" : 3,     "r6" : 6,     "r9" : 9,   "r12" : 12,   "r15" : 15,

            "sl" : 10,    "fp" : 11,    "ip" : 12,  "sp"  : 13,   "lr"  : 14,
            "pc" : 15,
}


def reg2bin(reg, bits):
    return format(_reg2num[reg], 'b').zfill(bits)

def hex2int(hexstr):
    return int(hexstr, 16)

