/**
 *  @file micro.hh
 *  @brief Microcode library
 *
 *  Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/
#ifndef MICRO_H
#define MICRO_H

#include <satdefs.hh>

/* <symb, is_signed?, width> */
typedef tuple<bool, ExprType, int> OpTriple;
inline const OpTriple make_op_triple (bool is_signed, ExprType exprType, int width) {
    return make_tuple <bool, ExprType, int> (is_signed, exprType, width);
}

/* triple helper getters */
inline bool triple_issigned( const OpTriple& triple )
{ return triple.get<0>(); }
inline ExprType triple_optype( const OpTriple& triple )
{ return triple.get<1>(); }
inline int triple_width( const OpTriple& triple )
{ return triple.get<2>(); }

struct OpTripleHash {
    long operator() (const OpTriple& k) const
    {
        const long prime = 31;

        long res = 1;
        res = prime * res + (k.get<0>() ? 1231 : 1237);
        res = prime * res + k.get<1>();
        res = prime * res + k.get<2>();
        return res;
    }
};

struct OpTripleEq {
    bool operator() (const OpTriple& x, const OpTriple& y) const
    {
        return
            x.get<0>() == y.get<0>() &&
            x.get<1>() == y.get<1>() &&
            x.get<2>() == y.get<2>()  ;
    }
};

class MuxDescriptor {
public:
    MuxDescriptor(unsigned width, DDVector& z, ADD cnd, ADD aux, DDVector& x, DDVector& y);

    inline unsigned width() const
    { return f_width; }
    inline const DDVector& z() const
    { return f_z; }
    inline ADD cnd() const
    { return f_cnd; }
    inline ADD aux() const
    { return f_aux; }
    inline const DDVector& x() const
    { return f_x; }
    inline const DDVector& y() const
    { return f_y; }

private:
    unsigned f_width;
    DDVector f_z;
    ADD f_cnd;
    ADD f_aux;
    DDVector f_x;
    DDVector f_y;
};

class MicroDescriptor {

public:
    MicroDescriptor(OpTriple triple, DDVector& z, DDVector &x);
    MicroDescriptor(OpTriple triple, DDVector& z, DDVector &x, DDVector &y);

    inline const OpTriple& triple() const
    { return f_triple; }

    inline const DDVector& z() const
    { return f_z; }
    inline const DDVector& x() const
    { return f_x; }
    inline const DDVector& y() const
    { return f_y; }

    inline bool is_relational() const
    { return f_z.size() == 1; }

    inline bool is_binary() const
    { return f_z.size() == f_x.size() &&
             f_z.size() == f_y.size(); }

    inline bool is_unary() const
    { return f_y.size() == 0; }

private:
    OpTriple f_triple;

    DDVector f_z;
    DDVector f_x;
    DDVector f_y;
};

// helpers
ostream& operator<<(ostream& os, OpTriple triple);
ostream& operator<<(ostream& os, MicroDescriptor& md);
ostream& operator<<(ostream& os, MuxDescriptor& md);

typedef vector<MicroDescriptor> MicroDescriptors;
typedef vector<MuxDescriptor> MuxDescriptors;
typedef unordered_map<Expr_ptr, MuxDescriptors> MuxMap;

#endif
