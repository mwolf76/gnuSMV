/**
 *  @file variant.hh
 *  @brief Variant type implementation
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

#ifndef VARIANT_H
#define VARIANT_H
#include <common.hh>
#include <ctime>

typedef enum {
    BOTTOM,
    BOOLEAN,
    INTEGER,
    CLOCK,
    STRING,
} VariantType;

// Variant iface
class IVariant : public IObject {
public:
    virtual bool is_nil() const =0;

    virtual bool is_boolean() const =0;
    virtual bool as_boolean() const =0;

    virtual bool is_integer() const =0;
    virtual int as_integer() const =0;

    virtual bool is_clock() const =0;
    virtual clock_t as_clock() const =0;

    virtual bool is_string() const =0;
    virtual string as_string() const =0;
};


// Variant implementation
class Variant : public IVariant {
    friend ostream& operator<<(ostream& os, const Variant& variant);

public:
    // variant constructors
    Variant();

    Variant(const string &value);
    Variant(const char *value);

    Variant(bool value);
    Variant(int value);

    Variant(clock_t value);

    Variant(const Variant& v);

    // variant predicates & getters
    bool is_nil() const;

    bool is_boolean() const;
    bool as_boolean() const;

    bool is_integer() const;
    int as_integer() const;

    bool is_clock() const;
    clock_t as_clock() const;

    bool is_string() const;
    string as_string() const;

private:
    VariantType f_type;

    bool f_bool;
    int f_int;
    string f_str;
    clock_t f_clock;
};

extern Variant NilValue;

#endif
