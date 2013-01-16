/**
 *  @file type.hh
 *  @brief Type system classes
 *
 *  This module contains definitions and services that implement type
 *  classes. YASMINE's types can be classified as Monolithic types
 *  (i.e. that can be represented using a single DD) or Algebraic types
 *  (i.e. that are represented using a vector of DDs). The type system
 *  is organized as follows:
 *
 *  MONOLITHIC types
 *  ================
 *  + Booleans (type keyword is 'boolean')
 *  + Enumeratives (e.g. { LOUIE, HUEWEY, DEWEY })
 *  TODO: add more

 *  ALGEBRAIC types
 *  ===============
 *  + Signed integers(N) (type keyword is 'signed int'), where N is the number
 *  of hexadecimal digits used in the representation.
 *
 *  + Unsigned integers(N) (type keyword is 'unsigned int'), where N
 *  has the same meaning as above.
 *
 *  + Signed fixed-point reals(N, F) (type keyword is 'signed fxd'),
 *  where N is the number of hexadecimal digits used in the
 *  representation of the integer part, F is the number of hexadecimal
 *  digits used in the representation of the fractional part.
 *
 * + Unsigned fixed-point reals(N, F) (type keyword is 'unsigned
 * fxd'), where N, F have the same meaning as above.
 *
 * Remark: constants (either integer or real) are *always* unsigned
 * and have the special reserved abstract types 'unsigned int(0)' (for
 * integer consts) and 'unsigned fxd(0,0)' (for reals consts).
 *
 * Type Aliases
 * ============
 *
 * A few type aliases are provided, to make YASMINE's type system look
 * closer to C99's. These type aliases are just synctactig sugar for
 * the type classes defined above, and their usage though recommended
 * is in no way mandatory.
 *
 * INTEGER type aliases:
 * uint8_t, int8_t
 * uint16_t, int16_t
 * uint32_t, int32_t
 * uint64_t, int64_t
 *
 * REAL type aliases:
 * ufxd0_8t, fxd0_8t,
 * ufxd12_4t, int12_4t,
 * ufxd24_8t, int24_8t,
 * ufxd48_16t, ufxd48_16t
 *
 * Explicit type conversions(casts)
 * ================================
 *
 * C-like Casts operator are provided to force conversions among the
 * various algebraic types.
 *
 * Implicit type conversions
 * =========================
 *
 * To mimic C operators behavior, implicit type conversions (disabled
 * by default) can be enabled so that when expressions of different
 * type are encountered during the compilation of an algebraic
 * operator, type conversions take place as a pre-processing. The
 * following rules apply:
 *
 * 1. When one of the two operands is fxd the other one is also converted to fxd.
 * 2. When one of the two operands is signed the other one is also converted to signed.
 * 3. The size of the result fits the largest size of the involved operands.
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
#ifndef TYPE_H
#define TYPE_H

#include <common.hh>
#include <cudd_mgr.hh>

#include <expr.hh>
#include <expr_mgr.hh>

/* Supported data types: boolean, integers (signed and unsigned),
   fixed-point, enums, module instances, arrays of all-of-the-above. */

/* REMARK types are *immutable* by design! */

typedef unsigned expected_t;
#define TP_BOOLEAN   0x1
#define TP_INT_CONST 0x2
#define TP_FXD_CONST 0x4
#define TP_UNSIGNED_INT  0x8
#define TP_SIGNED_INT    0x10
#define TP_UNSIGNED_FXD  0x20
#define TP_SIGNED_FXD 0x40
#define TP_ENUM 0x80
#define TP_INSTANCE  0x100
#define TP_LAST_TYPE 0x100

/* arrays use former bits to determine the type of elements */
#define TP_ARRAY 0x200

/* _ptr typdefs */
typedef class Type* Type_ptr;
typedef class BooleanType* BooleanType_ptr;
typedef class AlgebraicType* AlgebraicType_ptr;
typedef class SignedAlgebraicType* SignedAlgebraicType_ptr;
typedef class UnsignedAlgebraicType* UnsignedAlgebraicType_ptr;
typedef class SignedFixedAlgebraicType* SignedFixedAlgebraicType_ptr;
typedef class UnsignedFixedAlgebraicType* UnsignedFixedAlgebraicType_ptr;
typedef class EnumType* EnumType_ptr;
typedef class Instance* Instance_ptr;
typedef class ArrayType* ArrayType_ptr;

// ostream helper, uses FQExpr printer (see expr/expr.cc)
ostream& operator<<(ostream& os, Type_ptr type);

// ostream helper, uses FQExpr printer (see expr/expr.cc)
ostream& operator<<(ostream& os, const Type_ptr type);

class TypeMgr; // fwd

/** Basic Type class. Is.. nothing. */
class Type : public Object {
public:
    Expr_ptr repr() const
    { return f_repr; }

    virtual ~Type()
    {}

protected:
    Type(TypeMgr &owner)
        : f_owner(owner)
        , f_repr(NULL)
    {}

    TypeMgr& f_owner;
    Expr_ptr f_repr;
};


/** Boolean type */
class BooleanType : public Type {
protected:
    friend class TypeMgr; // ctors not public
    BooleanType(TypeMgr& owner);
};

/** Integer constants type (always unsigned) */
typedef class IntConstType* IntConstType_ptr;
class IntConstType : public Type {
protected:
    friend class TypeMgr; // ctors not public
    IntConstType(TypeMgr& owner);
};

/** Fixed-point real constants type (always unsigned) */
typedef class FxdConstType* FxdConstType_ptr;
class FxdConstType : public Type {
protected:
    friend class TypeMgr; // ctors not public
    FxdConstType(TypeMgr& owner);
};

/** Base class for all algebraic types */
typedef class AlgebraicType* AlgebraicType_ptr;
class AlgebraicType : public Type {
public:
    ADD *dds() const
    { return f_dds; }

protected:
    friend class TypeMgr; // ctors not public
    AlgebraicType(TypeMgr& owner, ADD *dds = NULL);

    // this is reserved for temp encodings, it's NULL for ordinary algebraics
    ADD *f_dds;
};

/** Signed integers */
typedef class SignedAlgebraicType* SignedAlgebraicType_ptr;
class SignedAlgebraicType : public AlgebraicType {
public:
    unsigned width() const
    { return f_width; }

 protected:
    friend class TypeMgr; // ctors not public
    SignedAlgebraicType(TypeMgr& owner, unsigned width, ADD *dds = NULL);

    unsigned f_width;
};

/** Unsigned integers */
typedef class UnsignedAlgebraicType* UnsignedAlgebraicType_ptr;
class UnsignedAlgebraicType : public AlgebraicType {
public:
    unsigned width() const
    { return f_width; }

protected:
    friend class TypeMgr; // ctors not public
    UnsignedAlgebraicType(TypeMgr& owner, unsigned width, ADD *dds = NULL);

    unsigned f_width;
};


/* Signed Fixed-point reals */
typedef class SignedFixedAlgebraicType* SignedFixedAlgebraicType_ptr;
class SignedFixedAlgebraicType : public AlgebraicType {
public:
    unsigned width() const
    { return f_width; }

    unsigned fract() const
    { return f_fract; }

protected:
    friend class TypeMgr; // ctors not public
    SignedFixedAlgebraicType(TypeMgr& owner, unsigned width, unsigned fract, ADD *dds = NULL);

    unsigned f_width;
    unsigned f_fract;
};

/* Unsigned Fixed-point reals */
typedef class UnsignedFixedAlgebraicType* UnsignedFixedAlgebraicType_ptr;
class UnsignedFixedAlgebraicType : public AlgebraicType {
public:
    unsigned width() const
    { return f_width; }

    unsigned fract() const
    { return f_fract; }

    /* in current implementation fixed is signed only */
    bool is_signed() const
    { return true; }

protected:
    friend class TypeMgr; // ctors not public
    UnsignedFixedAlgebraicType(TypeMgr& owner, unsigned width, unsigned fract, ADD *dds = NULL);

    unsigned f_width;
    unsigned f_fract;
};

/** Arrays */
typedef class ArrayType* ArrayType_ptr;
class ArrayType : public Type {
public:
    unsigned size() const
    { return f_size; }

    Type_ptr of() const
    { return f_of; }

protected:
    friend class TypeMgr; // ctors not public
    ArrayType(TypeMgr& owner, Type_ptr of, unsigned size);

    Type_ptr f_of;
    unsigned f_size;
};

/** Enumeratives */
typedef class EnumType* EnumType_ptr;
class EnumType : public Type {
protected:
    friend class TypeMgr; // ctors not public
    EnumType(TypeMgr& owner, ExprSet& literals);

public:
    const ExprSet& literals() const
    { return f_literals; }

private:
    ExprSet f_literals;
};

typedef class Instance* Instance_ptr;
class Instance : public Type {
protected:
    friend class TypeMgr; // ctors not public
    Instance(TypeMgr& owner, Expr_ptr identifier);

public:
    const Expr_ptr identifier() const
    { return f_identifier; }

private:
    const Expr_ptr f_identifier;
};

#endif
