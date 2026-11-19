/*
 * vm/instructions.h
 *
 */

#pragma once

#include "vm/instruction.h"

#define ARITHMETIC_INSTRUCTIONS \
    X(LDI, auto dest = &Procp.get_c()[r[0]]; typename Sint::clear tmp = int(n), \
            *dest++ = tmp) \
    X(LDSI, auto dest = &Procp.get_s()[r[0]]; \
            auto tmp = Sint::constant(int(n), Proc.P.my_num(), Procp.MC.get_alphai()), \
            *dest++ = tmp) \
    X(LDMS, auto dest = &Procp.get_s()[r[0]]; auto source = &Proc.machine.Mp.MS[n], \
            *dest++ = *source++) \
    X(STMS, auto source = &Procp.get_s()[r[0]]; auto dest = &Proc.machine.Mp.MS[n], \
            *dest++ = *source++) \
    X(LDMSI, Proc.machine.Mp.MS.indirect_read(instruction, Procp.get_s(), Proc.get_ci()),) \
    X(STMSI, Proc.machine.Mp.MS.indirect_write(instruction, Procp.get_s(), Proc.get_ci()),) \
    X(LDMCI, Proc.machine.Mp.MC.indirect_read(instruction, Procp.get_c(), Proc.get_ci()),) \
    X(STMCI, Proc.machine.Mp.MC.indirect_write(instruction, Procp.get_c(), Proc.get_ci()),) \
    X(MOVS, auto dest = &Procp.get_s()[r[0]]; auto source = &Procp.get_s()[r[1]], \
            *dest++ = *source++) \
    X(ADDS, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = &Procp.get_s()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(ADDM, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ + Sint::constant(*op2++, Proc.P.my_num(), Procp.MC.get_alphai())) \
    X(ADDSI, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]], \
            *dest++ = *op1++ + Sint::constant(int(n), Proc.P.my_num(), Procp.MC.get_alphai())) \
    X(ADDC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(ADDCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ + op2) \
    X(SUBS, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = &Procp.get_s()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(SUBSI, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = Sint::constant(int(n), Proc.P.my_num(), Procp.MC.get_alphai()), \
            *dest++ = *op1++ - op2) \
    X(SUBSFI, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = Sint::constant(int(n), Proc.P.my_num(), Procp.MC.get_alphai()), \
            *dest++ = op2 - *op1++) \
    X(SUBML, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ - Sint::constant(*op2++, Proc.P.my_num(), Procp.MC.get_alphai())) \
    X(SUBMR, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_s()[r[2]], \
            *dest++ = Sint::constant(*op1++, Proc.P.my_num(), Procp.MC.get_alphai()) - *op2++) \
    X(SUBC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(SUBCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ - op2) \
    X(SUBCFI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = op2 - *op1++) \
    X(PREFIXSUMS, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            Sint s, \
            s += *op1++; *dest++ = s) \
    X(PICKS, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1] + r[2]], \
            *dest++ = *op1; op1 += int(n)) \
    X(MULM, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]]; mulm_check<Sint>(), \
            *dest++ = *op1++ * *op2++) \
    X(MULC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(MULCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(MULSI, auto dest = &Procp.get_s()[r[0]]; auto op1 = &Procp.get_s()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(ANDC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ & *op2++) \
    X(XORC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ ^ *op2++) \
    X(ORC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ | *op2++) \
    X(ANDCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ & op2) \
    X(XORCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ ^ op2) \
    X(ORCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            typename Sint::clear op2 = int(n), \
            *dest++ = *op1++ | op2) \
    X(SHLC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ << *op2++) \
    X(SHRC, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]]; \
            auto op2 = &Procp.get_c()[r[2]], \
            *dest++ = *op1++ >> *op2++) \
    X(SHLCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]], \
            *dest++ = *op1++ << n) \
    X(SHRCI, auto dest = &Procp.get_c()[r[0]]; auto op1 = &Procp.get_c()[r[1]], \
            *dest++ = *op1++ >> n) \
    X(TRIPLE, auto a = &Procp.get_s()[r[0]]; auto b = &Procp.get_s()[r[1]]; \
            auto c = &Procp.get_s()[r[2]], \
            Procp.DataF.get_three(DATA_TRIPLE, *a++, *b++, *c++)) \
    X(BIT, auto dest = &Procp.get_s()[r[0]], \
            Procp.DataF.get_one(DATA_BIT, *dest++)) \
    X(RANDOMFULLS, auto dest = &Procp.get_s()[r[0]], \
            *dest++ = Procp.DataF.get_random()) \
    X(GLDSI, auto dest = &Proc2.get_s()[r[0]]; \
            auto tmp = Sgf2n::constant(int(n), Proc.P.my_num(), Proc2.MC.get_alphai()), \
            *dest++ = tmp) \
    X(GLDMS, auto dest = &Proc2.get_s()[r[0]]; auto source = &Proc.machine.M2.MS[n], \
            *dest++ = *source++) \
    X(GSTMS, auto source = &Proc2.get_s()[r[0]]; auto dest = &Proc.machine.M2.MS[n], \
            *dest++ = *source++) \
    X(GLDMSI, Proc.machine.M2.MS.indirect_read(instruction, Proc2.get_s(), Proc.get_ci()),) \
    X(GSTMSI, Proc.machine.M2.MS.indirect_write(instruction, Proc2.get_s(), Proc.get_ci()),) \
    X(GMOVS, auto dest = &Proc2.get_s()[r[0]]; auto source = &Proc2.get_s()[r[1]], \
            *dest++ = *source++) \
    X(GADDS, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = &Proc2.get_s()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(GADDM, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = &Proc2.get_c()[r[2]], \
            *dest++ = *op1++ + Sgf2n::constant(*op2++, Proc.P.my_num(), Proc2.MC.get_alphai())) \
    X(GADDSI, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]], \
            *dest++ = *op1++ + Sgf2n::constant(int(n), Proc.P.my_num(), Proc2.MC.get_alphai())) \
    X(GSUBS, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = &Proc2.get_s()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(GSUBSI, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = Sgf2n::constant(int(n), Proc.P.my_num(), Proc2.MC.get_alphai()), \
            *dest++ = *op1++ - op2) \
    X(GSUBSFI, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = Sgf2n::constant(int(n), Proc.P.my_num(), Proc2.MC.get_alphai()), \
            *dest++ = op2 - *op1++) \
    X(GSUBML, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = &Proc2.get_c()[r[2]], \
            *dest++ = *op1++ - Sgf2n::constant(*op2++, Proc.P.my_num(), Proc2.MC.get_alphai())) \
    X(GSUBMR, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_c()[r[1]]; \
            auto op2 = &Proc2.get_s()[r[2]], \
            *dest++ = Sgf2n::constant(*op1++, Proc.P.my_num(), Proc2.MC.get_alphai()) - *op2++) \
    X(GMULM, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            auto op2 = &Proc2.get_c()[r[2]]; mulm_check<Sgf2n>(), \
            *dest++ = *op1++ * *op2++) \
    X(GMULSI, auto dest = &Proc2.get_s()[r[0]]; auto op1 = &Proc2.get_s()[r[1]]; \
            typename Sgf2n::clear op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(GTRIPLE, auto a = &Proc2.get_s()[r[0]]; auto b = &Proc2.get_s()[r[1]]; \
            auto c = &Proc2.get_s()[r[2]], \
            Proc2.DataF.get_three(DATA_TRIPLE, *a++, *b++, *c++)) \
    X(GBIT, auto dest = &Proc2.get_s()[r[0]], \
            Proc2.DataF.get_one(DATA_BIT, *dest++)) \
    X(CONVINT, auto dest = &Procp.get_c()[r[0]]; auto source = &Proc.get_ci()[r[1]], \
            *dest++ = *source++) \
    X(GCONVINT, auto dest = &Proc2.get_c()[r[0]]; auto source = &Proc.get_ci()[r[1]], \
            *dest++ = *source++) \

#define REGINT_INSTRUCTIONS \
    X(LDMINT, auto dest = &Proc.get_ci()[r[0]]; auto source = &mi[n], \
            *dest++ = (*source).get(); source++) \
    X(STMINT, auto dest = &mi[n]; auto source = &Proc.get_ci()[r[0]], \
            *dest++ = *source++) \
    X(LDMINTI, mi.indirect_read(*this, Proc.get_ci(), Proc.get_ci()),) \
    X(STMINTI, mi.indirect_write(*this, Proc.get_ci(), Proc.get_ci()),) \
    X(MOVINT, auto dest = &Proc.get_ci()[r[0]]; auto source = &Ci[r[1]], \
            *dest++ = *source++) \
    X(PUSHINT, Proc.pushi(Ci[r[0]]),) \
    X(POPINT, auto dest = &Ci[r[0]], Proc.popi(*dest++)) \
    X(LDTN, auto dest = &Ci[r[0]], *dest++ = Proc.get_thread_num()) \
    X(LDARG, auto dest = &Ci[r[0]], *dest++ = Proc.get_arg()) \
    X(STARG, Proc.set_arg(Ci[r[0]]),) \
    X(LDINT, auto dest = &Proc.get_ci()[r[0]], \
            *dest++ = int(n)) \
    X(ADDINT, auto dest = &Proc.get_ci()[r[0]]; auto op1 = &Proc.get_ci()[r[1]]; \
            auto op2 = &Proc.get_ci()[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(SUBINT, auto dest = &Proc.get_ci()[r[0]]; auto op1 = &Proc.get_ci()[r[1]]; \
            auto op2 = &Proc.get_ci()[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(MULINT, auto dest = &Proc.get_ci()[r[0]]; auto op1 = &Proc.get_ci()[r[1]]; \
            auto op2 = &Proc.get_ci()[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(DIVINT, auto dest = &Proc.get_ci()[r[0]]; auto op1 = &Proc.get_ci()[r[1]]; \
            auto op2 = &Proc.get_ci()[r[2]], \
            if (*op2 == 0) throw DivisionByZero(); *dest++ = *op1++ / *op2++) \
    X(INCINT, incint(Proc.get_ci()),) \
    X(EQZC, auto dest = &Ci[r[0]]; auto source = &Ci[r[1]], *dest++ = *source++ == 0) \
    X(LTZC, auto dest = &Ci[r[0]]; auto source = &Ci[r[1]], *dest++ = *source++ < 0) \
    X(LTC, auto dest = &Ci[r[0]]; auto op1 = &Ci[r[1]]; auto op2 = &Ci[r[2]], \
            *dest++ = *op1++ < *op2++) \
    X(GTC, auto dest = &Ci[r[0]]; auto op1 = &Ci[r[1]]; auto op2 = &Ci[r[2]], \
            *dest++ = *op1++ > *op2++) \
    X(EQC, auto dest = &Ci[r[0]]; auto op1 = &Ci[r[1]]; auto op2 = &Ci[r[2]], \
            *dest++ = *op1++ == *op2++) \
    X(PRINTINT, print(Proc.out, &Proc.get_ci_ref(r[0])),) \
    X(PRINTFLOATPREC, Proc.out << setprecision(n),) \
    X(PRINTSTR, Proc.out << string((char*)&n,4) << flush,) \
    X(PRINTCHR, Proc.out << string((char*)&n,1) << flush,) \
    X(SHUFFLE, shuffle(Proc),) \
    X(BITDECINT, bitdecint(Proc),) \
    X(RAND, auto dest = &Ci[r[0]]; auto source = &Ci[r[1]], \
            *dest++ = Proc.shared_prng.get_uint() % (1 << (*source++).get())) \
    X(GROUPSUMINT, groupsumint(Ci),) \
    X(QS_PERM, qs_permutation(Ci),) \

#define CLEAR_GF2N_INSTRUCTIONS \
    X(GLDI, auto dest = &C2[r[0]]; Cgf2n tmp = int(n), \
            *dest++ = tmp) \
    X(GLDMC, auto dest = &C2[r[0]]; auto source = &M2C[n], \
            *dest++ = (*source).get(); source++) \
    X(GSTMC, auto dest = &M2C[n]; auto source = &C2[r[0]], \
            *dest++ = *source++) \
    X(GLDMCI, M2C.indirect_read(*this, C2, Proc.get_ci()),) \
    X(GSTMCI, M2C.indirect_write(*this, C2, Proc.get_ci()),) \
    X(GMOVC, auto dest = &C2[r[0]]; auto source = &C2[r[1]], \
            *dest++ = *source++) \
    X(GADDC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ + *op2++) \
    X(GADDCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ + op2) \
    X(GSUBC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ - *op2++) \
    X(GSUBCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ - op2) \
    X(GSUBCFI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = op2 - *op1++) \
    X(GMULC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ * *op2++) \
    X(GMULCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ * op2) \
    X(GANDC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ & *op2++) \
    X(GXORC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ ^ *op2++) \
    X(GORC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            auto op2 = &C2[r[2]], \
            *dest++ = *op1++ | *op2++) \
    X(GANDCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ & op2) \
    X(GXORCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ ^ op2) \
    X(GORCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]]; \
            Cgf2n op2 = int(n), \
            *dest++ = *op1++ | op2) \
    X(GNOTC, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]], \
            *dest++ = ~*op1++) \
    X(GSHLCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]], \
            *dest++ = *op1++ << n) \
    X(GSHRCI, auto dest = &C2[r[0]]; auto op1 = &C2[r[1]], \
            *dest++ = *op1++ >> n) \
    X(GPRINTREG, auto source = &C2[r[0]], \
            Proc.out << "Reg[" << r[0] << "] = " << *source++ \
            << " # " << string((char*)&n, 4) << endl) \
    X(GPRINTREGPLAIN, auto source = &C2[r[0]], \
            Proc.out << *source++ << flush) \
    X(GBITDEC, gbitdec(C2),) \
    X(GBITCOM, gbitcom(C2),) \
    X(GCONVGF2N, auto dest = &Proc.get_ci()[r[0]]; auto source = &C2[r[1]], \
            *dest++ = source->get_word(); source++) \
    X(GRAWOUTPUT, auto source = &C2[r[0]], \
            (*source++).output(Proc.get_public_output(), false)) \

#define REMAINING_INSTRUCTIONS \
    X(CONVMODP, throw NotImplemented(),) \
    X(LDMC, throw NotImplemented(),) \
    X(STMC, throw NotImplemented(),) \
    X(MOVC, throw NotImplemented(),) \
    X(DIVC, throw NotImplemented(),) \
    X(GDIVC, throw NotImplemented(),) \
    X(FLOORDIVC, throw NotImplemented(),) \
    X(MODC, throw NotImplemented(),) \
    X(LEGENDREC, throw NotImplemented(),) \
    X(DIGESTC, throw NotImplemented(),) \
    X(DIVCI, throw NotImplemented(),) \
    X(GDIVCI, throw NotImplemented(),) \
    X(INV2M, throw NotImplemented(),) \
    X(MODCI, throw NotImplemented(),) \
    X(SQUARE, throw NotImplemented(),) \
    X(GSQUARE, throw NotImplemented(),) \
    X(INV, throw NotImplemented(),) \
    X(GINV, throw NotImplemented(),) \
    X(RANDOMS, throw NotImplemented(),) \
    X(INPUTMASKREG, throw NotImplemented(),) \
    X(INPUTMASK, throw NotImplemented(),) \
    X(GINPUTMASK, throw NotImplemented(),) \
    X(INPUT, throw NotImplemented(),) \
    X(GINPUT, throw NotImplemented(),) \
    X(INPUTFIX, throw NotImplemented(),) \
    X(INPUTFLOAT, throw NotImplemented(),) \
    X(INPUTMIXED, throw NotImplemented(),) \
    X(INPUTMIXEDREG, throw NotImplemented(),) \
    X(RAWINPUT, throw NotImplemented(),) \
    X(GRAWINPUT, throw NotImplemented(),) \
    X(INPUTPERSONAL, throw NotImplemented(),) \
    X(NOTC, throw NotImplemented(),) \
    X(SHRSI, throw NotImplemented(),) \
    X(OPEN, throw NotImplemented(),) \
    X(GOPEN, throw NotImplemented(),) \
    X(MULS, throw NotImplemented(),) \
    X(GMULS, throw NotImplemented(),) \
    X(MULRS, throw NotImplemented(),) \
    X(GMULRS, throw NotImplemented(),) \
    X(MULBITS, throw NotImplemented(),) \
    X(DOTPRODS, throw NotImplemented(),) \
    X(GDOTPRODS, throw NotImplemented(),) \
    X(MATMULS, throw NotImplemented(),) \
    X(MATMULSM, throw NotImplemented(),) \
    X(CONV2DS, throw NotImplemented(),) \
    X(TRUNC_PR, throw NotImplemented(),) \
    X(CHECK, throw NotImplemented(),) \
    X(JMP, throw NotImplemented(),) \
    X(JMPI, throw NotImplemented(),) \
    X(JMPNZ, throw NotImplemented(),) \
    X(JMPEQZ, throw NotImplemented(),) \
    X(PRINTREG, throw NotImplemented(),) \
    X(PRINTREGPLAIN, throw NotImplemented(),) \
    X(CONDPRINTPLAIN, throw NotImplemented(),) \
    X(PRINTFLOATPLAIN, throw NotImplemented(),) \
    X(CONDPRINTSTR, throw NotImplemented(),) \
    X(REQBL, throw NotImplemented(),) \
    X(GREQBL, throw NotImplemented(),) \
    X(USE, throw NotImplemented(),) \
    X(USE_INP, throw NotImplemented(),) \
    X(USE_EDABIT, throw NotImplemented(),) \
    X(USE_MATMUL, throw NotImplemented(),) \
    X(USE_PREP, throw NotImplemented(),) \
    X(GUSE_PREP, throw NotImplemented(),) \
    X(TIME, throw NotImplemented(),) \
    X(START, throw NotImplemented(),) \
    X(STOP, throw NotImplemented(),) \
    X(RUN_TAPE, throw NotImplemented(),) \
    X(JOIN_TAPE, throw NotImplemented(),) \
    X(CRASH, throw NotImplemented(),) \
    X(STARTGRIND, throw NotImplemented(),) \
    X(STOPGRIND, throw NotImplemented(),) \
    X(NPLAYERS, throw NotImplemented(),) \
    X(THRESHOLD, throw NotImplemented(),) \
    X(PLAYERID, throw NotImplemented(),) \
    X(LISTEN, throw NotImplemented(),) \
    X(ACCEPTCLIENTCONNECTION, throw NotImplemented(),) \
    X(CLOSECLIENTCONNECTION, throw NotImplemented(),) \
    X(READSOCKETINT, throw NotImplemented(),) \
    X(READSOCKETC, throw NotImplemented(),) \
    X(READSOCKETS, throw NotImplemented(),) \
    X(WRITESOCKETINT, throw NotImplemented(),) \
    X(WRITESOCKETC, throw NotImplemented(),) \
    X(WRITESOCKETS, throw NotImplemented(),) \
    X(WRITESOCKETSHARE, throw NotImplemented(),) \
    X(WRITEFILESHARE, throw NotImplemented(),) \
    X(READFILESHARE, throw NotImplemented(),) \
    X(PUBINPUT, throw NotImplemented(),) \
    X(RAWOUTPUT, throw NotImplemented(),) \
    X(INTOUTPUT, throw NotImplemented(),) \
    X(FLOATOUTPUT, throw NotImplemented(),) \
    X(STARTPRIVATEOUTPUT, throw NotImplemented(),) \
    X(GSTARTPRIVATEOUTPUT, throw NotImplemented(),) \
    X(STOPPRIVATEOUTPUT, throw NotImplemented(),) \
    X(GSTOPPRIVATEOUTPUT, throw NotImplemented(),) \
    X(PREP, throw NotImplemented(),) \
    X(GPREP, throw NotImplemented(),) \
    X(CISC, throw NotImplemented(),) \
    X(SECSHUFFLE, throw NotImplemented(),) \
    X(GENSECSHUFFLE, throw NotImplemented(),) \
    X(APPLYSHUFFLE, throw NotImplemented(),) \
    X(DELSHUFFLE, throw NotImplemented(),) \
    X(ACTIVE, throw NotImplemented(),) \
    X(FIXINPUT, throw NotImplemented(),) \
    X(CONCATS, throw NotImplemented(),) \
    X(ZIPS, throw NotImplemented(),) \
    X(GMATMULS, throw NotImplemented(),) \
    X(GMATMULSM, throw NotImplemented(),) \
    X(PRINTREGPLAINS, throw NotImplemented(),) \
    X(GPRINTREGPLAINS, throw NotImplemented(),) \
    X(CALL_TAPE, throw NotImplemented(),) \
    X(CMDLINEARG, throw NotImplemented(),) \
    X(INITCLIENTCONNECTION, throw NotImplemented(),) \
    X(GWRITEFILESHARE, throw NotImplemented(),) \
    X(GREADFILESHARE, throw NotImplemented(),) \
    X(WRITEFILECLEAR, throw NotImplemented(),) \
    X(READFILECLEAR, throw NotImplemented(),) \

#define ALL_INSTRUCTIONS ARITHMETIC_INSTRUCTIONS REGINT_INSTRUCTIONS \
    CLEAR_GF2N_INSTRUCTIONS REMAINING_INSTRUCTIONS
