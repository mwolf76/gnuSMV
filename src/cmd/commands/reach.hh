/**
 * @file reach.hh
 * @brief Command-interpreter subsystem related classes and definitions.
 *
 * This header file contains the handler inteface for the `reach`
 * command.
 *
 * Copyright (C) 2012-2018 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/

#ifndef REACH_CMD_H
#define REACH_CMD_H

#include <cmd/command.hh>
#include <algorithms/reach/reach.hh>

class Reach : public Command {
public:
    Reach(Interpreter& owner);
    virtual ~Reach();

    /** cmd params */
    void set_target(Expr_ptr target);

    /* guided reachability support: forward, backward and global guides */
    void add_forward_constraint(Expr_ptr constraint);
    void add_backward_constraint(Expr_ptr constraint);
    void add_global_constraint(Expr_ptr constraint);

    /* run() */
    utils::Variant virtual operator()();

private:
    std::ostream& f_out;

    /* the negation of invariant property to be verified */
    Expr_ptr f_target;

    ExprVector f_forward_constraints;
    ExprVector f_backward_constraints;
    ExprVector f_global_constraints;

    // -- helpers -------------------------------------------------------------
    bool check_requirements();
};

typedef Reach* Reach_ptr;

class ReachTopic : public CommandTopic {
public:
    ReachTopic(Interpreter& owner);
    virtual ~ReachTopic();

    void virtual usage();
};

#endif /* REACH_CMD_H */
