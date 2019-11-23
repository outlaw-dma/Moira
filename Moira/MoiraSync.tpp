// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------


int
Moira::syncIllegal(uint16_t opcode, int i)
{
    return 0;
}

template<Instr I, Size S> int Moira::syncRegShift(uint16_t opcode, int i) {
    printf("***** syncRegShift<%d,%d>(%d,%d)\n", I, S, opcode, i);
    return i;
}

template<Instr I, Size S> int Moira::syncImmShift(uint16_t opcode, int i) {
    printf("***** syncImmShift<%d,%d>(%d,%d)\n", I, S, opcode, i);
    return i;
}

template<Instr I, Mode M> int Moira::syncEaShift(uint16_t opcode, int i) {
    printf("***** syncEaShift<%d,%d>(%d,%d)\n", I, M, opcode, i);
    return i;
}