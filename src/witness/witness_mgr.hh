/**
 *  @file witness_mgr.hh
 *  @brief Witness module (WitnessMgr class)
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

#ifndef WITNESS_MGR_H
#define WITNESS_MGR_H

#include <evaluator.hh>

#include <model.hh>
#include <model_mgr.hh>

#include <witness.hh>

typedef class WitnessMgr *WitnessMgr_ptr;
class WitnessMgr  {

public:
    static WitnessMgr& INSTANCE() {
        if (! f_instance) f_instance = new WitnessMgr();
        return (*f_instance);
    }

    inline ExprMgr& em() const
    { return f_em; }

    inline TypeMgr& tm() const
    { return f_tm; }

    // delegated method to the Evaluator functor
    inline const value_t eval( Witness&w, Expr_ptr ctx, Expr_ptr formula, step_t k)
    { return f_evaluator.process( w, ctx, formula, k); }

    // get a registered witness
    Witness& witness( Expr_ptr id );

    // register a new witness
    void register_witness( Expr_ptr id, Witness& w );

protected:
    WitnessMgr();
    ~WitnessMgr();

private:
    static WitnessMgr_ptr f_instance;

    // auto witness counter
    unsigned f_auto_index;

    // ref to expr manager
    ExprMgr& f_em;

    // ref to type manager
    TypeMgr& f_tm;

    Evaluator f_evaluator;
};

#endif
