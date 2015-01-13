/**
 *  @file internals.cc
 *
 *  Copyright (C) 2011-2015 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
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
#include <compiler.hh>

/* private service of walk_leaf */
void Compiler::push_dds(Encoding_ptr enc, Type_ptr type)
{
    assert (NULL != enc);
    DDVector& dds = enc->dv();
    unsigned width = dds.size();
    assert( 0 < width );

    // push into type stack
    f_type_stack.push_back(type);

    /* booleans, monoliths are just one DD */
    if (type->is_monolithic())
        f_add_stack.push_back(dds[0]);

    /* algebraics, reversed list of encoding DDs */
    else if (type->is_algebraic()) {
        // type and enc width info has to match
        assert( type -> as_algebraic()-> width() == width );
        for (DDVector::reverse_iterator ri = dds.rbegin();
             ri != dds.rend(); ++ ri) {
            f_add_stack.push_back(*ri);
        }
    }

    /* array of algebraics, same as above, times nelems (!) */
    else if (type->is_array()) {

        // type and enc width info has to match
        assert( type -> as_array() -> of() -> as_algebraic()-> width() ==
                width / type -> as_array() -> nelems());

        for (DDVector::reverse_iterator ri = dds.rbegin();
             ri != dds.rend(); ++ ri) {
            f_add_stack.push_back(*ri);
        }
    }

    else assert( false ); // unexpected
}

/* unary ops */
void Compiler::register_microdescriptor( bool signedness, ExprType symb, unsigned width,
                                         DDVector& z, DDVector& x )
{
    MicroDescriptor md( make_op_triple( signedness, symb, width ), z, x);
    f_micro_descriptors.push_back(md);

    DEBUG
        << "Registered "
        << md
        << std::endl;
}

/* binary ops (both algebraic and relationals) */
void Compiler::register_microdescriptor( bool signedness, ExprType symb, unsigned width,
                                         DDVector& z, DDVector& x, DDVector &y )
{
    MicroDescriptor md( make_op_triple( signedness, symb, width ), z, x, y);
    f_micro_descriptors.push_back(md);

    DEBUG
        << "Registered "
        << md
        << std::endl;
}

void Compiler::register_muxdescriptor( Expr_ptr toplevel, unsigned width,
                                       DDVector& z, ADD cnd, ADD aux,
                                       DDVector& x, DDVector &y )
{
    /* verify if entry for toplevel already exists. If not, create it */
    {
        MuxMap::const_iterator mi = f_mux_map.find( toplevel );
        if (f_mux_map.end() == mi)
            f_mux_map.insert( std::make_pair< Expr_ptr, MuxDescriptors >
                              (toplevel, MuxDescriptors()));
    }

    MuxDescriptor md( width, z, cnd, aux, x, y);

    /* Entry for toplevel does exist for sure */
    {
        MuxMap::iterator mi = f_mux_map.find( toplevel );
        assert( f_mux_map.end() != mi );

        mi -> second.push_back( md );
    }

    DEBUG
        << "Registered "
        << md
        << std::endl;
}

/* post-processing for MUXes: for each descriptor, we need to conjunct
   `! AND ( prev_conditions ) AND cnd <-> aux` to the original
   formula */
void Compiler::post_process_muxes()
{
    for (MuxMap::const_iterator i = f_mux_map.begin(); f_mux_map.end() != i; ++ i) {

        Expr_ptr toplevel (i -> first);
        const MuxDescriptors& descriptors (i -> second);

        DRIVEL
            << "Processing MUX activation clauses for `"
            << toplevel << "`"
            << std::endl;

        ADD prev = f_enc.zero();
        for (MuxDescriptors::const_reverse_iterator j = descriptors.rbegin();
             descriptors.rend() != j; ++ j) {
            ADD act (prev.Cmpl().Times(j -> cnd()));
            PUSH_DD( act.Xnor(j -> aux()));
            prev = act;
        }
    }
}

/* auto id generator */
Expr_ptr Compiler::make_auto_id()
{
    ExprMgr& em = f_owner.em();
    std::ostringstream oss;
    oss << "__tmp" << f_temp_auto_index ++ ;
    return em.make_identifier(oss.str());
}

/* Builds a temporary expression out of a DD array. This is used by
   some algebraic algorithms. Uses arrays instead of DDVectors due to
   historic reasons. */
Expr_ptr Compiler::make_temporary_expr(ADD dds[], unsigned width)
{
    ExprMgr& em = f_owner.em();
    TypeMgr& tm = f_owner.tm();

    Expr_ptr expr(make_auto_id());

    /* Register temporary symbol into resolver (temporaries are global) */
    f_owner.resolver()->add_symbol(em.make_temp(), expr,
                                   new Variable(em.make_main(), expr,
                                                tm.find_unsigned(width),
                                                false, true));

    /* register encoding, using fqexpr */
    const FQExpr& key = FQExpr(expr);
    f_temp_encodings [ key ] = new AlgebraicEncoding(width, false, dds);

    return expr;
}

/* build an auto fresh ADD variable and register its encoding */
ADD Compiler::make_auto_dd()
{
    TypeMgr& tm = f_owner.tm();
    Type_ptr boolean(tm.find_boolean());

    BooleanEncoding_ptr be = reinterpret_cast<BooleanEncoding_ptr>
        (f_enc.make_encoding( boolean ));

    // register encoding, a FQExpr is needed for UCBI booking
    Expr_ptr aid = make_auto_id();
    Expr_ptr ctx = f_ctx_stack.back();
    step_t time = f_time_stack.back();
    FQExpr key (ctx, aid, time);
    f_enc.register_encoding( key, be);

    return be -> bits() [0];
}

/* build an auto DD vector of fresh ADD variables. */
void Compiler::make_auto_ddvect(DDVector& dv, unsigned width)
{
    assert(0 == dv.size());
    for (unsigned i = 0; i < width; ++ i )
        dv.push_back(make_auto_dd());
}

void Compiler::pre_node_hook(Expr_ptr expr)
{
    /* assemble memoization key */
    assert( 0 < f_ctx_stack.size() );
    Expr_ptr ctx = f_ctx_stack.back();

    assert( 0 < f_time_stack.size() );
    step_t time = f_time_stack.back();

    FQExpr key(ctx, expr, time);

    if (f_preprocess)
        DRIVEL
            << "Preprocessing " << key << "..."
            << std::endl;
    else
        DRIVEL
            << "Processing " << key << "..."
            << std::endl;
}

void Compiler::post_node_hook(Expr_ptr expr)
{
    /* no caching during preprocessing */
    if (f_preprocess)
        return;

    /* no cache when compiling types */
    if (f_owner.em().is_type(expr))
        return;

    /* assemble memoization key */
    assert( 0 < f_ctx_stack.size() );
    Expr_ptr ctx = f_ctx_stack.back();

    assert( 0 < f_time_stack.size() );
    step_t time = f_time_stack.back();

    FQExpr key(ctx, expr, time);

    assert( 0 < f_type_stack.size() );
    Type_ptr type = f_type_stack.back();

    /* collect dds */
    DDVector dv;
    unsigned i, width = type -> width();
    assert(width <= f_add_stack.size());

    ADDStack::reverse_iterator ri;
    for (i = 0, ri = f_add_stack.rbegin();
         i < width; ++ i, ++ ri) {
        dv.push_back(*ri);
    }
    assert (dv.size() == width);

    /* memoize result */
    f_cache.insert( std::make_pair<FQExpr, CompilationUnit> ( key,
            CompilationUnit( dv, f_micro_descriptors, f_mux_map)));

    unsigned res_sz (width);
    unsigned mcr_sz (f_micro_descriptors.size());
    unsigned mux_sz (f_mux_map.size());

    DRIVEL
        << "Cached " << key
        << ": "
        << res_sz << " DDs, "
        << mcr_sz << " Microcode descriptors, "
        << mux_sz << " Multiplexer descriptors."
        << std::endl;
}

bool Compiler::cache_miss(const Expr_ptr expr)
{
    Expr_ptr ctx (f_ctx_stack.back());

    FQExpr key(f_ctx_stack.back(), expr, f_time_stack.back());
    CompilationMap::iterator eye = f_cache.find(key);

    if (eye != f_cache.end()) {
        const Type_ptr type = f_owner.type(expr, ctx);
        DEBUG << "Cache hit for " << expr
              << ", type is " << type
              << std::endl;

        CompilationUnit& unit = (*eye).second;

        /* push cached DDs (reversed) */
        {
            const DDVector& dds (unit.dds());
            DDVector::const_reverse_iterator i;
            for (i = dds.rbegin(); i != dds.rend(); ++ i )
                f_add_stack.push_back(*i);
        }

        /* push cached microcode descriptors */
        {
            const MicroDescriptors& micros (unit.micro_descriptors());
            MicroDescriptors::const_iterator i;
            for (i = micros.begin(); micros.end() != i; ++ i)
                f_micro_descriptors.push_back(*i);
        }

        /* push cached multiplexer chains */
        {
            const MuxMap& muxes (unit.mux_map());
            MuxMap::const_iterator i;
            for (i = muxes.begin(); muxes.end() != i; ++ i)
                f_mux_map.insert(*i);
        }

        /* push cached type */
        f_type_stack.push_back(type);

        /* cache hit */
        return false;
    }

    /* cache miss */
    return true;
}

void Compiler::clear_internals()
{
    f_add_stack.clear();
    f_type_stack.clear();
    f_ctx_stack.clear();
    f_time_stack.clear();
    f_micro_descriptors.clear();
    f_mux_map.clear();
    f_toplevel_map.clear();
}

/* TODO: refactor pre and post hooks, they're pretty useless like this :-/ */
void Compiler::pre_hook()
{}
void Compiler::post_hook()
{}

