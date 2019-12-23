// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef STRING_WRITER_H
#define STRING_WRITER_H

#include "MoiraBase.h"

namespace moira {

//
// Wrapper structures controlling the output format
//

struct Int        { i32 raw;        Int(i32 v) : raw(v) { } };
struct UInt       { u32 raw;       UInt(u32 v) : raw(v) { } };
struct UInt8      { u8  raw;      UInt8(int v) : raw(v) { } };
struct UInt16     { u16 raw;     UInt16(int v) : raw(v) { } };
struct UInt32     { u32 raw;     UInt32(int v) : raw(v) { } };
struct Dn         { int raw;         Dn(int v) : raw(v) { } };
struct An         { int raw;         An(int v) : raw(v) { } };
struct Rn         { int raw;         Rn(int v) : raw(v) { } };
struct Imu        { u32 raw;        Imu(int v) : raw(v) { } };
struct Ims        { i32 raw;        Ims(int v) : raw(v) { } };
struct Imd        { u32 raw;        Imd(int v) : raw(v) { } };
struct Scale      { int raw;      Scale(int v) : raw(v) { } };
struct Align      { int raw;      Align(int v) : raw(v) { } };
struct RegList    { u16 raw;    RegList(u16 v) : raw(v) { } };
struct RegRegList { u16 raw; RegRegList(u16 v) : raw(v) { } };

template <Instr I>        struct Ins { };
template <Size S>         struct Sz  { };
template <Mode M, Size S> struct Ea  { u32 pc; u16 reg; u32 ext1; u32 ext2; u32 ext3; };

struct Finish     { };

class StrWriter
{
    char comment[32];
    char *base;
    char *ptr;
    bool hex;

public:

    StrWriter(char *p, bool h) : base(p), ptr(p), hex(h) {
        comment[0] = 0;
    };

    //
    // Printing instruction fragments
    //

    StrWriter& operator<<(const char *str);
    StrWriter& operator<<(int i);
    StrWriter& operator<<(Int i);
    StrWriter& operator<<(UInt u);
    StrWriter& operator<<(UInt8 value);
    StrWriter& operator<<(UInt16 value);
    StrWriter& operator<<(UInt32 value);
    StrWriter& operator<<(Dn dn);
    StrWriter& operator<<(An an);
    StrWriter& operator<<(Rn rn);
    StrWriter& operator<<(Imu im);
    StrWriter& operator<<(Ims im);
    StrWriter& operator<<(Imd im);
    StrWriter& operator<<(Scale s);
    StrWriter& operator<<(Align align);
    StrWriter& operator<<(RegRegList l);
    StrWriter& operator<<(RegList l);
    template <Instr I> StrWriter& operator<<(Ins<I> i);
    template <Size S> StrWriter& operator<<(Sz<S> sz);
    template <Mode M, Size S> StrWriter& operator<<(const Ea<M,S> &ea);
    StrWriter& operator<<(Finish finish);

private:

    template <Mode M, Size S> void briefExtension(const Ea<M,S> &ea);
    template <Mode M, Size S> void fullExtension(const Ea<M,S> &ea);
};

}
#endif
