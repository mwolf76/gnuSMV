/**
 *  @file enc.cc
 *  @brief Encoder module
 *
 *  This module contains definitions and services that implement an
 *  encoder for symbols. For each symbol a boolean encoding is
 *  maintained, the encoder takes care of ADD variables definitions
 *  and is provides mapback services as well.
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
#include <enc.hh>

// shared dctor
Encoding::~Encoding()
{}

// low-level service for bits allocation
ADD Encoding::make_bit()
{
    ADD res = f_mgr.bit();

    /* keep track of every bit in the encoding, this is user later,
       when evaluating the scalar value of a bit combination. */
    f_bits.push_back(res);

    return res;
}

// base service, has to be in superclass for visibility
ADD Encoding::make_monolithic_encoding(unsigned nbits)
{
    ADD res = make_bit();
    ADD two = f_mgr.constant(2);

    assert(0 < nbits);
    unsigned i = 1;

    while (i < nbits) {
        res *= two;
        res += make_bit();

        ++ i;
    }

    return res;
}

// boolean 1(1 bit) var
BooleanEncoding::BooleanEncoding()
    : Encoding()
{
    // single bit encoding
    f_dv.push_back(make_bit());
}

Expr_ptr BooleanEncoding::expr(int *assignment)
{
    ExprMgr& em = f_mgr.em();
    ADD eval = f_dv[0].Eval( assignment );
    assert (cuddIsConstant(eval.getRegularNode()));

    value_t res = Cudd_V(eval.getNode());
    return res == 0 ? em.make_false() : em.make_true();
}

ADD BooleanEncoding::bit()
{
    assert( 1 == f_dv.size() );
    return f_dv[0];
}

MonolithicEncoding::MonolithicEncoding()
    : Encoding()
{}

// algebraic encoding uses monolithic as a builing block
AlgebraicEncoding::AlgebraicEncoding(unsigned width, unsigned fract, bool is_signed, ADD *dds)
    : f_width(width)
    , f_fract(fract)
    , f_signed(is_signed)
    , f_temporary(NULL != dds)
{
    if (f_temporary) {
        assert( NULL != dds ); // obvious
        for (unsigned i = 0; i < width; ++ i) {
            f_dv.push_back(dds[i]);
        }
    }
    else {
        for (unsigned i = 0; i < width; ++ i) {
            f_dv.push_back( make_monolithic_encoding(NIBBLE_SIZE));
        }
    }
}

DDVector::const_iterator AlgebraicEncoding::bits_begin(unsigned k)
{
    assert(k < f_width);
    DDVector::const_iterator res = f_bits.begin();

    /* skip previous digits' bits */
    for (unsigned i = 0; i < k * NIBBLE_SIZE; ++ i) {
        ++ res;
    }

    return res;
}

DDVector::const_iterator AlgebraicEncoding::bits_end(unsigned k)
{
    assert(k < f_width);
    DDVector::const_iterator res = bits_begin(k);

    /* skip digits' bits */
    for (unsigned i = 0; i < NIBBLE_SIZE; ++ i) {
        ++ res;
    }

    return res;
}

Expr_ptr AlgebraicEncoding::expr(int *assignment)
{
    ExprMgr& em = f_mgr.em();
    unsigned i;

    value_t res = 0;

    i = 0; do {
        ADD eval = f_dv[i].Eval( assignment );
        assert (cuddIsConstant(eval.getRegularNode()));

        res += Cudd_V(eval.getNode());

        if (++ i < f_width) {
            res *= 0x10; // bleah!
        } else break;
    } while (true);

    return em.make_iconst(res);
}

EnumEncoding::EnumEncoding(const ExprSet& lits)
{
    unsigned nbits = range_repr_bits(lits.size());
    make_monolithic_encoding(nbits);

    value_t v;
    ExprSet::iterator eye;
    for (v = 0, eye= lits.begin(); eye != lits.end(); ++ eye, ++ v) {

        f_v2e_map[v] = *eye;
        f_e2v_map[*eye] = v;
    }
}

Expr_ptr EnumEncoding::expr(int *assignment)
{
    ADD eval = f_dv[0].Eval(assignment);
    assert (cuddIsConstant(eval.getNode()));

    value_t lindex = Cudd_V(eval.getNode());
    return f_v2e_map [lindex];
}

unsigned MonolithicEncoding::range_repr_bits (value_t range)
{
    unsigned res = 0;
    assert(0 < range);
    while (range) {
        ++ res;
        range /= 2;
    }

    return res;
}

ArrayEncoding::ArrayEncoding(Encodings elements)
    : f_elements(elements)
{
    assert(0 < elements.size());
    f_dv.reserve( elements.size() *
                  elements[0]->dv().size() ); // preallocate memory

    for (Encodings::iterator i = elements.begin(); i != elements.end(); ++ i) {
        DDVector& dds = (*i)->dv();
        f_dv.insert( f_dv.end(), dds.begin(), dds.end() );
    }
}

Expr_ptr ArrayEncoding::expr(int* assignment)
{
    assert( false ); // an array cannot be evaluated
}
