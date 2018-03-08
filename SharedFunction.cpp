G3.3

Shared/debug
 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Shared Abort handling
 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // IsFault()
 // =========
 // Return true if a fault is associated with an address descriptor

boolean IsFault(AddressDescriptor addrdesc)
{
     return addrdesc.fault.type != Fault_None;
}
 

 // IPAValid()
 // ==========
 // Return TRUE if the IPA is reported for the abort
 boolean IPAValid(FaultRecord fault)
{
     assert fault.type != Fault_None;
     if fault.s2fs1walk then
         return fault.type IN {Fault_AccessFlag, Fault_Permission, Fault_Translation,
                 Fault_AddressSize};
     else if fault.secondstage then
         return fault.type IN {Fault_AccessFlag, Fault_Translation, Fault_AddressSize};
     else
         return FALSE;
}

 // IsExternalAbort()
 // =================
 boolean IsExternalAbort(Fault type)
{
     assert type != Fault_None;

     return (type IN {Fault_SyncExternal, Fault_SyncParity, Fault_AsyncExternal, Fault_AsyncParity,
             Fault_SyncExternalOnWalk, Fault_SyncParityOnWalk});
}


 // IsExternalAbort()
 // =================
 boolean IsExternalAbort( FaultRecord fault)
{
     return IsExternalAbort(fault.type);
}

 // IsDebugException()
 // ==================
 boolean IsDebugException(FaultRecord fault)
{
     assert fault.type != Fault_None;
     return fault.type == Fault_Debug;
} 


 // IsSecondStage()
 // ===============
 boolean IsSecondStage(FaultRecord fault)
{

     assert fault.type != Fault_None;
     return fault.secondstage;
}

 // IsAsyncAbort()
 // ==============
 boolean IsAsyncAbort(Fault type)
{
     assert type != Fault_None;
     return (type IN {Fault_AsyncExternal, Fault_AsyncParity});
}

 // IsAsyncAbort()
 // ==============
 boolean IsAsyncAbort( FaultRecord fault)
{
     return IsAsyncAbort(fault.type);
}

 // EncodeLDFSC()
 // =============
 // Function that gives the Long-descriptor FSC code for types of Fault

 bits(6) EncodeLDFSC(Fault type, integer level)
{
     bits(6) result;

     case type of
         when Fault_AddressSize result = '0000':level<1:0>;
         when Fault_AccessFlag result = '0010':level<1:0>;
         when Fault_Permission result = '0011':level<1:0>;
         when Fault_Translation result = '0001':level<1:0>;
         when Fault_SyncExternal result = '010000';
         when Fault_SyncExternalOnWalk result = '0101':level<1:0>;
         when Fault_SyncParity result = '011000';
         when Fault_SyncParityOnWalk result = '0111':level<1:0>;
         when Fault_AsyncParity result = '011001';
         when Fault_AsyncExternal result = '010001';
         when Fault_Alignment result = '100001';
         when Fault_Debug result = '100010';
         when Fault_TLBConflict result = '110000';
         when Fault_Lockdown result = '110100';
         when Fault_Coproc result = '111010';
         otherwise Unreachable();
    return result;
}

 // LSInstructionSyndrome()
 // =======================
 // Returns the extended syndrome information for a second stage fault.
 // <10> - Syndrome valid bit. The syndrome is only valid for certain types of access instruction.
 // <9:8> - Access size.
 // <7> - Sign extended (for loads).
 // <6:2> - Transfer register.
 // <1> - Transfer register is 64-bit.
 // <0> - Instruction has acquire/release semantics.
 bits(11) LSInstructionSyndrome();

 // FaultSyndrome()
 // ===============
 // Creates an exception syndrome value for Abort and Watchpoint exceptions taken to
 // AArch32 Hyp mode or an Exception Level using AArch64.
 bits(25) FaultSyndrome(boolean d_side, FaultRecord fault)
{
     assert fault.type != Fault_None;

     bits(25) iss = Zeros();
     if d_side && IsSecondStage(fault) then
         iss<24:14> = LSInstructionSyndrome();
     if IsExternalAbort(fault) then iss<9> = fault.extflag;
     if d_side then
         iss<8> = if fault.acctype IN {AccType_DC, AccType_IC} then '1' else '0';
         iss<7> = if fault.s2fs1walk then '1' else '0';
         iss<6> = if fault.write then '1' else '0';
     iss<5:0> = EncodeLDFSC(fault.type, fault.level);

     return iss;
}

 // Helpers mostly identical to the ARM ARM
 // IsZero()
 // ========
 boolean IsZero(bits(N) x)
{
     return x == Zeros(N);
}

 // IsZeroBit()
 // ===========
 bit IsZeroBit(bits(N) x)
{
     return if IsZero(x) then '1' else '0';
}

 // IsOnes()
 // ========
 boolean IsOnes(bits(N) x)
{
     return x == Ones(N);
}

 // LSL()
 // =====
 bits(N) LSL(bits(N) x, integer shift)
{
     assert shift >= 0;
     if shift == 0 then
         result = x;
     else
         (result, -) = LSL_C(x, shift);
     return result;
}

 // LSL_C()
 // =======
 (bits(N), bit) LSL_C(bits(N) x, integer shift)
{
     assert shift > 0;
     extended_x = x : Zeros(shift);
     result = extended_x<N-1:0>;
     carry_out = extended_x<N>;
     return (result, carry_out);
}

 // LSR()
 // =====
 bits(N) LSR(bits(N) x, integer shift)
{
     assert shift >= 0;
     if shift == 0 then
         result = x;
     else
         (result, -) = LSR_C(x, shift);
     return result;
}

 // LSR_C()
 // =======
 (bits(N), bit) LSR_C(bits(N) x, integer shift)
{
     assert shift > 0; 
     extended_x = ZeroExtend(x, shift+N);
     result = extended_x<shift+N-1:shift>;
     carry_out = extended_x<shift-1>;
     return (result, carry_out);
}

 // ASR()
 // =====
 bits(N) ASR(bits(N) x, integer shift)
{
     assert shift >= 0;
     if shift == 0 then
         result = x;
     else
         (result, -) = ASR_C(x, shift);
     return result;
}

 // ASR_C()
 // =======
 (bits(N), bit) ASR_C(bits(N) x, integer shift)
{
     assert shift > 0;
     extended_x = SignExtend(x, shift+N);
     result = extended_x<shift+N-1:shift>;
     carry_out = extended_x<shift-1>;
     return (result, carry_out);
}

 // ROR()
 // =====
 bits(N) ROR(bits(N) x, integer shift)
{
     assert shift >= 0;
     if shift == 0 then
         result = x;
     else
         (result, -) = ROR_C(x, shift);
     return result;
}

 // ROR_C()
 // =======
 (bits(N), bit) ROR_C(bits(N) x, integer shift)
{
     assert shift != 0;
     m = shift MOD N;
     result = LSR(x,m) OR LSL(x,N-m);
     carry_out = result<N-1>;
     return (result, carry_out);
}

 // Replicate()
 // ===========

 bits(M*N) Replicate(bits(M) x, integer N);

 // Replicate()
 // ===========
 bits(N) Replicate(bits(M) x)
{
     assert N MOD M == 0;
     return Replicate(x, N DIV M);
}

 // RoundDown()
 // ===========

 integer RoundDown(real x);

 // RoundUp()
 // =========
 //
 integer RoundUp(real x);

 // RoundTowardsZero()
 // ==================
 integer RoundTowardsZero(real x)
{
     return if x == 0.0 then 0 else if x >= 0.0 then RoundDown(x) else RoundUp(x);
}

 // SignExtend()
 // ============
 bits(N) SignExtend(bits(M) x, integer N)
{
     assert N >= M;
     return Replicate(x<M-1>, N-M) : x;
}

 // SignExtend()
 // ============
 bits(N) SignExtend(bits(M) x)
{
     return SignExtend(x, N);
}

 // ZeroExtend()
 // ============
 bits(N) ZeroExtend(bits(M) x, integer N)
{
     assert N >= M;
     return Zeros(N-M) : x;
}

 // ZeroExtend()
 // ============
 bits(N) ZeroExtend(bits(M) x)
{
     return ZeroExtend(x, N);
}

 // Extend()
 // ========
 bits(N) Extend(bits(M) x, integer N, boolean unsigned)
{
     return if unsigned then ZeroExtend(x, N) else SignExtend(x, N);
}

 // Extend()
 // ========
 bits(N) Extend(bits(M) x, boolean unsigned)
{
     return Extend(x, N, unsigned);
}

 // Zeros()
 // =======
 bits(N) Zeros(integer N)
{
     return Replicate('0',N);
}

 // Zeros()
 // =======
 bits(N) Zeros()
{
     return Zeros(N);
}

 // Ones()
 // ======
 bits(N) Ones(integer N)
{
     return Replicate('1',N);
}

 // Ones()
 // ======
 bits(N) Ones()
{
     return Ones(N);
}

 // NOT()
 // =====
 //
 bits(N) NOT(bits(N) x);

 // UInt()
 // ======
 
 
