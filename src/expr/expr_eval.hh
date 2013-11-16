/**
 *  @file expr_printer.hh
 *  @brief Expr printers
 *
 *  This module contains definitions and services that implement an
 *  optimized storage for expressions. Expressions are stored in a
 *  Directed Acyclic Graph (DAG) for data sharing.
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

#ifndef EXPR_PRINTER_H
#define EXPR_PRINTER_H
#include <expr_walker.hh>

typedef unordered_map<FQExpr_ptr, value_t, FQExprHash, FQExprEq> FQExprValueMap;
typedef pair<ExprValueMap::iterator, bool> FQExprValueMapHit;

/* shortcuts to to simplify manipulation of the internal type stack */
#define POP_EXPR(op)                              \
    const Expr_ptr op = f_expr_stack.back();      \
    f_expr_stack.pop_back()

#define PUSH_EXPR(tp)                             \
    f_expr_stack.push_back(tp)

class Evaluator : public ExprWalker {

    ExprStack f_expr_stack;
    ExprStack f_ctx_stack;
    TimeStack f_time_stack;

    // environment for evaluation
    Witness_ptr f_witness;

    // cache
    FQExprValueMap f_map;

public:
    Evaluator(); // defaults to std::cout
    virtual ~Evaluator();

    Expr_ptr process(Witness& witness, Expr_ptr ctx, Expr_ptr body, step_t time);

protected:
    OP_HOOKS;
    void walk_leaf(const Expr_ptr expr);

private:

    // services
    inline bool cache_miss(const Expr_ptr expr)
    {
        assert( 0 < f_ctx_stack.size());
        Expr_ptr ctx = f_ctx_stack.back();

        assert( 0 < f_time_stack.size());
        step_t step = f_time_stack.back();

        FQExpr key(ctx , expr, step);
        FQExprValueMap::iterator eye = f_map.find(key);

        if (eye != f_map.end()) {
            Expr_ptr res = (*eye).second;
            PUSH_EXPR(res);
            return false;
        }

        return true;
    }
};

#endif
