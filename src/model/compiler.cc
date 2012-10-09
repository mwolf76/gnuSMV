/**
 *  @file compiler.cc
 *  @brief Boolean compiler
 *
 *  This module contains definitions and services that implement the
 *  booolean expressions compilation into a form which is suitable for
 *  the SAT analysis. Current implementation uses ADDs to perform
 *  expression manipulation and booleanization. Expressions are
 *  assumed to be type-safe, only boolean expressions on arithmetic
 *  predicates are supported. The final result of expression
 *  compilation must be a 0-1 ADD which is suitable for CNF clauses
 *  injection directly into the SAT solver. The compilation engine is
 *  implemented using a simple walker pattern: (a) on preorder, return
 *  true if the node has not yet been visited; (b) always do in-order
 *  (for binary nodes); (c) perform proper compilation in post-order
 *  hooks.
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

#include <common.hh>

#include <expr.hh>
#include <compiler.hh>

// comment this out
// #define DEBUG_BE_COMPILER

BECompiler::BECompiler()
    : f_map()
    , f_add_stack()
    , f_ctx_stack()
    , f_owner(ModelMgr::INSTANCE())
    , f_enc(EncodingMgr::INSTANCE())
{ DEBUG << "Created BECompiler @" << this << endl; }

BECompiler::~BECompiler()
{ DEBUG << "Destroying BECompiler @" << this << endl; }

ADD BECompiler::process(Expr_ptr ctx, Expr_ptr body, step_t time = 0)
{
    // remove previous results
    f_add_stack.clear();
    f_type_stack.clear();
    f_ctx_stack.clear();
    f_time_stack.clear();

    // walk body in given ctx
    f_ctx_stack.push_back(ctx);
    f_time_stack.push_back(time);

    DEBUG << "Compiling boolean expression "
          << "(time = " << time << ") "
          << ctx << "::" << body
          << endl;

    // invoke walker on the body of the expr to be processed
    (*this)(body);

    // sanity conditions
    assert(1 == f_add_stack.size());
    assert(1 == f_type_stack.size());
    assert(1 == f_ctx_stack.size());
    assert(1 == f_time_stack.size());

    // Just one add returned. This is ok, because the compiler is
    // supposed to return a boolean formula.
    ADD add = f_add_stack.back();
    return add;
}

void BECompiler::pre_hook()
{}
void BECompiler::post_hook()
{
    // TODO: MANDATORY CHECK THE RETURNED ADD!!!!
    // cout << "out -- " << endl;
    // ADD add = f_add_stack.back();
    // add.PrintMinterm();
    // cout << endl;
}

bool BECompiler::walk_next_preorder(const Expr_ptr expr)
{
    step_t curr_time = f_time_stack.back();
    f_time_stack.push_back(1 + curr_time);
    return true;
}
void BECompiler::walk_next_postorder(const Expr_ptr expr)
{
    f_time_stack.pop_back(); // reset time stack
}

bool BECompiler::walk_neg_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void BECompiler::walk_neg_postorder(const Expr_ptr expr)
{
    if (is_unary_monolithic(expr)) {
        const ADD top = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(top.Negate());
    }
    else if (is_unary_algebraic(expr)) {
        unsigned i, width = f_enc.width();
        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform algebraic NEG */
        assert(0); // not implemented
    }
    else assert(0); // unexpected
}

bool BECompiler::walk_not_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void BECompiler::walk_not_postorder(const Expr_ptr expr)
{
    if (is_unary_boolean(expr)) {
        const ADD top = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(top.Cmpl());
    }
    else if (is_unary_algebraic(expr)) {
        unsigned i, width = f_enc.width();
        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform algebraic NOT */
        assert(0); // not implemented
    }
}

bool BECompiler::walk_add_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_add_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_add_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Plus(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform arithmetic sum using positional algorithm */
        ADD carry = f_enc.zero();
        for (i = width; (i); -- i) {

            /* x[i] + y[i] + c */
            ADD tmp = (*lhs[i]).Plus(*rhs[i]).Plus(carry);
            carry = f_enc.base().LT(tmp); /* c > 0x10 */

            /* x[i] = (x[i] + y[i] + c) % base */ // TODO: detect overflow on MSB
            f_add_stack.push_back(tmp.Modulus(f_enc.base()));
        }
    }

    else assert(0); // unexpected
}

bool BECompiler::walk_sub_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_sub_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_sub_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Minus(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        assert(0); // not implemented
    }

    else assert(0); // unexpected
}

bool BECompiler::walk_div_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_div_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_div_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Divide(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        assert(0); // not implemented
    }

    else assert(0); // unexpected
}

bool BECompiler::walk_mul_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_mul_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_mul_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Times(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        assert(0); // not implemented
    }

    else assert(0); // unexpected
}

bool BECompiler::walk_mod_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_mod_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_mod_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Modulus(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        assert(0); // not implemented
    }

    else assert(0); // unexpected
}

bool BECompiler::walk_and_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_and_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_and_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Times(rhs)); /* 0, 1 logic uses arithmetic product */
    }

    else if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWTimes(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWTimes(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }

    else assert(0);
}

bool BECompiler::walk_or_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_or_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_or_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Or(rhs));
    }

    else if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWOr(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWTimes(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }
}

bool BECompiler::walk_xor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_xor_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_xor_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Xor(rhs));
    }

    else if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWXor(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWXor(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }
}

bool BECompiler::walk_xnor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_xnor_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_xnor_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Xnor(rhs));
    }

    else if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWXnor(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWXnor(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }
}

bool BECompiler::walk_implies_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_implies_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_implies_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Cmpl().Or(rhs));
    }

    else if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWCmpl().BWXOr(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWCmpl().BWXOr(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }
    else assert(0);
}

bool BECompiler::walk_iff_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_iff_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_iff_postorder(const Expr_ptr expr)
{
    /* just a fancy name for xnor :-) */
    walk_xnor_postorder(expr);
}

bool BECompiler::walk_lshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_lshift_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_lshift_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.BWCmpl().BWXOr(rhs)); /* bitwise monolithic arithmetic */
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, nothing fancy  here :-) */
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp = (*lhs[i]).BWCmpl().BWXOr(*rhs[i]);
            f_add_stack.push_back(tmp);
        }
    }
    else assert(0);

    const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
    const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
    #ifdef DEBUG_BE_COMPILER
    cout << "-- " << expr << endl;
    lhs.LShift(rhs).PrintMinterm();
    #endif
    f_add_stack.push_back(lhs.LShift(rhs));
}

bool BECompiler::walk_rshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_rshift_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_rshift_postorder(const Expr_ptr expr)
{
    const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
    const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
    #ifdef DEBUG_BE_COMPILER
    cout << "-- " << expr << endl;
    lhs.RShift(rhs).PrintMinterm();
    #endif
    f_add_stack.push_back(lhs.RShift(rhs));
}

bool BECompiler::walk_eq_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_eq_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_eq_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Equals(rhs));
    }

    else if (is_binary_monolithic(expr) ||
             is_binary_enumerative(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Equals(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, similar to xnor, only conjuct res */
        ADD tmp = f_enc.one();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp *= (*lhs[i]).Equals(*rhs[i]);
        }

        /* just one result */
        f_add_stack.push_back(tmp);

    }
    else assert(0);
}

bool BECompiler::walk_ne_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_ne_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_ne_postorder(const Expr_ptr expr)
{
    if (is_binary_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Equals(rhs).Cmpl());
    }

    else if (is_binary_monolithic(expr) ||
             is_binary_enumerative(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.Equals(rhs).Cmpl());
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* perform bw arithmetic, similar to xnor, only conjuct res */
        ADD tmp = f_enc.one();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp *= (*lhs[i]).Equals(*rhs[i]);
        }

        /* just one result */
        f_add_stack.push_back(tmp.Cmpl());

    }
    else assert(0);
}

bool BECompiler::walk_gt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_gt_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_gt_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(rhs.LT(lhs)); // simulate GT op
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* relationals, msb predicate first, if false inspect next digit ... */
        ADD tmp = f_enc.zero();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp += (*rhs[i]).LT(*lhs[i]); // CHECK MSB
        }

        /* just one result */
        f_add_stack.push_back(tmp);

    }
    else assert(0);
}

bool BECompiler::walk_ge_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_ge_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_ge_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(rhs.LEQ(lhs)); // simulate GEQ op
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* relationals, msb predicate first, if false inspect next digit ... */
        ADD tmp = f_enc.zero();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp += (*rhs[i]).LEQ(*lhs[i]); // CHECK MSB
        }

        /* just one result */
        f_add_stack.push_back(tmp);

    }
    else assert(0);
}

bool BECompiler::walk_lt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_lt_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_lt_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.LT(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* relationals, msb predicate first, if false inspect next digit ... */
        ADD tmp = f_enc.zero();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp += (*lhs[i]).LT(*rhs[i]); // CHECK MSB
        }

        /* just one result */
        f_add_stack.push_back(tmp);

    }
    else assert(0);
}

bool BECompiler::walk_le_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_le_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_le_postorder(const Expr_ptr expr)
{
    if (is_binary_monolithic(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        f_add_stack.push_back(lhs.LEQ(rhs));
    }

    else if (is_binary_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* relationals, msb predicate first, if false inspect next digit ... */
        ADD tmp = f_enc.zero();
        for (i = width; (i); -- i) {

            /* x[i] &  y[i] */
            ADD tmp += (*lhs[i]).LEQ(*rhs[i]); // CHECK MSB
        }

        /* just one result */
        f_add_stack.push_back(tmp);

    }
    else assert(0);
}

bool BECompiler::walk_ite_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_ite_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_ite_postorder(const Expr_ptr expr)
{
    if (is_ite_boolean(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD c = f_add_stack.back(); f_add_stack.pop_back();

        f_add_stack.push_back(c.Ite(lhs, rhs));
    }

    else if (is_ite_monolithic(expr) ||
             is_ite_enumerative(expr)) {
        const ADD rhs = f_add_stack.back(); f_add_stack.pop_back();
        const ADD lhs = f_add_stack.back(); f_add_stack.pop_back();

        f_add_stack.push_back(c.Ite(lhs, rhs));
    }

    else if (is_ite_algebraic(expr)) {
        unsigned i, width = f_enc.width();

        ADD* rhs[width];
        for (i = width; (i) ; -- i) {
            *rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        ADD* lhs[width];
        for (i = width; (i) ; -- i) {
            *lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
        }

        /* multiplex, easy as pie :-) */
        for (i = width; (i); -- i) {
            f_add_stack.push_back(c.Ite(*lhs[i], *rhs[i]));
        }
    }

    else assert(0); // unexpected
 }

bool BECompiler::walk_cond_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_cond_inorder(const Expr_ptr expr)
{ return true; }
void BECompiler::walk_cond_postorder(const Expr_ptr expr)
{ /* nop, ite will do all the work */ }

bool BECompiler::walk_dot_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool BECompiler::walk_dot_inorder(const Expr_ptr expr)
{
    // ADD tmp = f_add_stack.back();
    // Expr_ptr ctx = tmp->get_repr();
    // f_ctx_stack.push_back(ctx);
    return true;
}
void BECompiler::walk_dot_postorder(const Expr_ptr expr)
{
    ADD rhs_add;

    // { // RHS, no checks necessary/possible
    //     const ADD top = f_add_stack.back(); f_add_stack.pop_back();
    //     rhs_add = top;
    // }

    // { // LHS, must be an instance (by assertion, otherwise leaf would have fail)
    //     const ADD top = f_add_stack.back(); f_add_stack.pop_back();
    //     assert(f_tm.is_instance(top));
    // }

    // // propagate rhs add
    // f_add_stack.push_back(rhs_add);

    // // restore previous ctx
    // f_ctx_stack.pop_back();
}

inline void BECompiler::push_const_value(value_t value)
{
    if (0 == value) {
        f_add_stack.push_back(f_enc.zero());
    }
    else if (1 == value) {
        f_add_stack.push_back(f_enc.one());
    }
    else {
        f_add_stack.push_back(f_enc.constant(value));
    }
}

void BECompiler::walk_leaf(const Expr_ptr expr)
{
    /* cached? */
    if (! cache_miss(expr)) return;

    // symb resolution
    Model& model = static_cast <Model&> (*f_owner.model());
    Expr_ptr ctx = f_ctx_stack.back();
    step_t time = f_time_stack.back();

    // 0. explicit constants (e.g. 42)
    if (ExprMgr::INSTANCE().is_numeric(expr)) {
        value_t value = expr->value();
        push_const_value_algebraic(value); // this is problematic, because
        // it actually depends on the other operand type (finite range or algebraic?). shit.
    }

    else {
        ISymbol_ptr symb = model.fetch_symbol(ctx, expr);
        assert (NULL != symb);

        // 1. bool/integer constant leaves
        if (symb->is_const()) {
            value_t value = symb->as_const().value();
            push_const_value_boolean(value); // shit
        }

        // 2. variable
        else if (symb->is_variable()) {

            FQExpr key(ctx, expr, time);
            IEncoding_ptr enc;

            // if encoding for temporized variable is available reuse it
            ENCMap::iterator eye = f_encodings.find(key);
            if (eye != f_encodings.end()) {
                enc = (*eye).second;
            }

            else {
                // ... otherwise create and cache it
                enc = f_enc.make_encoding(symb->as_variable().type());
                register_encoding(key, enc); // TODO: move the
                                             // encoding register to a
                                             // dedicated
                                             // object. (maybe the
                                             // enc_mgr itself?)
            }

            assert (NULL != enc);

            // push either 1 or more ADDs depending on the encoding
            if (enc->is_boolean() ||
                enc->is_monolithic()) { // enums have monolithic encodings too
                f_add_stack.push_back(enc->dds()[0]);
            }
            else if (enc->is_algebraic()) {
                unsigned i;
                for (i = enc->width(); (i); -- i) {
                    f_add_stack.push_back(enc->dds()[i]);
                }
            }
            else assert(0); // unexpected
        }

        // 3. define? Simply compile it recursively, no checks necessary.
        else if (symb->is_define()) {
            (*this)(symb->as_define().body());
        }

        else assert(0); // unexpected
    }
}

/**
   Booleans:
   . binary: AND, OR, XOR, XNOR, IFF, IMPLIES, EQ, NE
   . unary : NOT, () ?

   Finite Range Integers (aka Monolithes)

   . binary: AND (bw), OR(bw), XOR(bw), XNOR(bw), IFF(bw),
   IMPLIES(bw), LT, LE, GT, GE, EQ, NE, PLUS, SUB, DIV, MUL, MOD

   . unary : ? (), : (), NEG, NOT(bw)

   Enums (strict subset of the above)
   . binary : LT, LE, GT, GE, EQ, NE
   . unary  : ? (), : (),

   Algebraic:

   . binary : AND(bw), OR(bw), XOR(bw), XNOR(bw), IFF(bw),
   IMPLIES(bw), LT, LE, GT, GE, EQ, NE, PLUS, SUB, DIV, MUL, MOD

   . unary  : NOT(bw), ? (), : (), NEG,
*/

bool BECompiler::is_binary_boolean(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    ExprMgr& em = f_owner.em();

    /* AND, OR, XOR, XNOR, IFF, IMPLIES */
    if (em.is_binary_logical(expr)) {
        FQExpr lhs(f_ctx_stack.back(), expr->lhs());
        FQExpr rhs(f_ctx_stack.back(), expr->rhs());

        return (tm.is_boolean(f_owner.type(lhs)) &&
                tm.is_boolean(f_owner.type(rhs)));
    }

    return false;
}

bool BECompiler::is_unary_boolean(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    ExprMgr& em = f_owner.em();

    /*  NOT, () ? */
    if (em.is_unary_logical(expr)) {
        FQExpr lhs(f_ctx_stack.back(), expr->lhs());
        return (tm.is_boolean(f_owner.type(lhs)));
    }

    return false;
}

bool BECompiler::is_binary_monolithic(const Expr_ptr expr)
{
    /* AND (bw), OR(bw), XOR(bw), XNOR(bw), IFF(bw),
       IMPLIES(bw), LT, LE, GT, GE, EQ, NE, PLUS, SUB, DIV, MUL, MOD */
    if ((em.is_binary_logical(expr)) ||
        (em.is_binary_arithmetical(expr)) ||
        (em.is_binary_relational(expr))) {

        FQExpr lhs(f_ctx_stack.back(), expr->lhs());
        FQExpr rhs(f_ctx_stack.back(), expr->rhs());

        return (tm.is_monolithic(f_owner.type(lhs)) &&
                tm.is_monolithic(f_owner.type(rhs)));
    }

    return false;
}

bool BECompiler::is_unary_monolithic(const Expr_ptr expr)
{
    /* unary : ? (), : (), NEG, NOT(bw) */
    if (em.is_unary_arithmetical(expr)) {
        FQExpr lhs(f_ctx_stack.back(), expr->lhs());

        return (tm.is_monolithic(f_owner.type(lhs)));
    }

    return false;
}

bool BECompiler::is_binary_enumerative(const Expr_ptr expr)
{
    /* AND (bw), OR(bw), XOR(bw), XNOR(bw), IFF(bw),
       IMPLIES(bw), LT, LE, GT, GE, EQ, NE, PLUS, SUB, DIV, MUL, MOD */
    if ((em.is_binary_arithmetical(expr)) ||
        (em.is_binary_relational(expr))) {

        FQExpr lhs(f_ctx_stack.back(), expr->lhs());
        FQExpr rhs(f_ctx_stack.back(), expr->rhs());

        return (tm.is_enumerative(f_owner.type(lhs)) &&
                tm.is_enumerative(f_owner.type(rhs)));
    }

    return false;
}

bool BECompiler::is_unary_enumerative(const Expr_ptr expr)
{
    /* unary : ? (), : (), NEG, NOT(bw) */
    if (em.is_unary_arithmetical(expr)) {
        FQExpr lhs(f_ctx_stack.back(), expr->lhs());

        return (tm.is_enumerative(f_owner.type(lhs)));
    }

    return false;
}


bool BECompiler::is_binary_algebraic(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    ExprMgr& em = f_owner.em();

    if ((em.is_binary_logical(expr)) ||
        (em.is_binary_arithmetical(expr)) ||
        (em.is_binary_relational(expr))) {

        FQExpr lhs(f_ctx_stack.back(), expr->lhs());
        FQExpr rhs(f_ctx_stack.back(), expr->rhs());

        return (tm.is_algebraic(f_owner.type(lhs)) &&
                tm.is_algebraic(f_owner.type(rhs)));
    }

    return false;
}

bool BECompiler::is_unary_algebraic(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    ExprMgr& em = f_owner.em();

    if ((em.is_unary_logical(expr)) ||
        (em.is_unary_arithmetical(expr))) {

        FQExpr lhs(f_ctx_stack.back(), expr->lhs());

        return (tm.is_algebraic(f_owner.type(lhs)));
    }

    return false;
}
