/**
 *  @file symbol.hh
 *  @brief Symbol interface
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

#ifndef SYMBOL_H
#define SYMBOL_H

typedef vector<FQExpr> FQExprVector;

class ISymbol;
typedef ISymbol* ISymbol_ptr;
typedef unordered_map<FQExpr, ISymbol_ptr, FQExprHash, FQExprEq> Symbols;

class ILiteral;
typedef ILiteral* ILiteral_ptr;
typedef unordered_map<FQExpr, ILiteral_ptr, FQExprHash, FQExprEq> Literals;

class IConstant;
typedef IConstant* IConstant_ptr;
typedef unordered_map<FQExpr, IConstant_ptr, FQExprHash, FQExprEq> Constants;

class IVariable;
typedef IVariable* IVariable_ptr;
typedef unordered_map<FQExpr, IVariable_ptr, FQExprHash, FQExprEq> Variables;

class ITemporary;
typedef ITemporary* ITemporary_ptr;
typedef unordered_map<FQExpr, ITemporary_ptr, FQExprHash, FQExprEq> Temporaries;

class IDefine;
typedef IDefine* IDefine_ptr;
typedef unordered_map<FQExpr, IDefine_ptr, FQExprHash, FQExprEq> Defines;

// -- primary decls  --------------------------------------------------------------
class ISymbol : IObject {
public:
    virtual const Expr_ptr ctx()  const =0;
    virtual const Expr_ptr expr() const =0;

    bool is_const() const;
    IConstant& as_const() const;

    bool is_literal() const;
    ILiteral& as_literal() const;

    bool is_variable() const;
    IVariable& as_variable() const;

    bool is_temporary() const;
    ITemporary& as_temporary() const;

    bool is_define() const;
    IDefine& as_define() const;
};

class ILiteral : public ISymbol {
public:
    virtual value_t value() const =0;
    virtual const Type_ptr type() const =0;
};

class IConstant : public ISymbol {
public:
    virtual value_t value() const =0;
    virtual const Type_ptr type() const =0;
};

class IVariable : public ISymbol {
public:
    // var types are used for enc building
    virtual const Type_ptr type() const =0;
};

class ITemporary : public IVariable {
public:
    virtual const Type_ptr type() const =0;
};

class IDefine : public ISymbol {
public:
    // defines have no type, it has to be inferred.
    virtual const Expr_ptr body() const =0;
};

#endif
