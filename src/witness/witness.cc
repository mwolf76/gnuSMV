/**
 *  @file witness.cc
 *  @brief Witness module
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
#include <witness.hh>

TimeFrame::TimeFrame()
{}

TimeFrame::~TimeFrame()
{}

/* Retrieves value for expr, throws an exception if no value exists. */
Expr_ptr TimeFrame::value( FQExpr expr )
{
    FQExpr2ExprMap::iterator eye;

    eye = f_map.find( expr );
    assert (f_map.end() != eye); // TODO

    return (*eye).second;
}

/* Returns true iff expr has an assigned value within this time frame. */
bool TimeFrame::has_value( FQExpr expr )
{
    FQExpr2ExprMap::iterator eye;

    eye = f_map.find( expr );
    return (f_map.end() != eye);
}

/* Sets value for expr */
void TimeFrame::set_value( FQExpr fqexpr, Expr_ptr value )
{
    DRIVEL << fqexpr
           << " := "
           << value << endl;

    f_map.insert( make_pair< FQExpr, Expr_ptr >
                  (fqexpr, value));
}

Witness::Witness(string name)
    : f_name(name)
{
    DEBUG << "Created new witness: " << f_name << endl;
}

TimeFrame& Witness::new_frame()
{
    TimeFrame_ptr res = new TimeFrame();
    f_frames.push_back(*res);

    unsigned k = length() -1;
    DEBUG << "Added TimeFrame " << k
          << " to witness " << name()
          << endl;

    return * res;
}
