/**
 *  @file model_mgr.hh
 *  @brief Model module (ModelMgr class)
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

#ifndef MODEL_MGR_H
#define MODEL_MGR_H

#include <model/model.hh>
#include <model/model_resolver.hh>

#include <model/preprocessor/preprocessor.hh>

#include <model/type_checker/type_checker.hh>

#include <expr/expr_mgr.hh>
typedef boost::unordered_map<Expr_ptr, Expr_ptr> InputsMap;

#include <type/type_mgr.hh>

typedef boost::unordered_map<Expr_ptr, Module_ptr, PtrHash, PtrEq> ContextMap;
typedef boost::unordered_map<Expr_ptr, Expr_ptr> ParamMap;

typedef enum {
    MMGR_BUILD_CTX_MAP,
    MMGR_BUILD_PARAM_MAP,
    MMGR_TYPE_CHECK,
    MMGR_DONE
} analyzer_pass_t;

typedef class ModelMgr *ModelMgr_ptr;
class ModelMgr  {

public:
    inline Model& model()
    { return f_model; }

    inline Module& main()
    { return f_model.module( ExprMgr::INSTANCE().main()); }

    // command input support
    void set_input(Expr_ptr id, Expr_ptr value);
    Expr_ptr get_input(Expr_ptr id);

    inline Module& module(Expr_ptr module_name)
    { return f_model.module( module_name); }

    inline Resolver_ptr resolver()
    { return &f_resolver; }

    // this must be called before any type checking
    bool analyze();

    static ModelMgr& INSTANCE() {
        if (! f_instance)
            f_instance = new ModelMgr();

        return (*f_instance);
    }

    inline ExprMgr& em() const
    { return f_em; }

    inline TypeMgr& tm() const
    { return f_tm; }

    // delegated type inference method
    inline Type_ptr type(Expr_ptr body,
                         Expr_ptr ctx = ExprMgr::INSTANCE().make_empty())
    {
        assert( f_analyzed );
        return f_type_checker.type(body, ctx);
    }

    // delegated param binding method
    inline Expr_ptr preprocess(Expr_ptr body,
                               Expr_ptr ctx = ExprMgr::INSTANCE().make_empty()) {
        return f_preprocessor.process(body, ctx);
    }

    Module_ptr scope(Expr_ptr ctx);

    Expr_ptr rewrite_parameter( Expr_ptr expr );

protected:
    ModelMgr();
    ~ModelMgr();

    friend class ModelResolver;

    Symbols f_symbols;
    inline Symbols& symbols()
    { return f_symbols; }

private:
    static ModelMgr_ptr f_instance;

    /* local data */
    Model f_model;

    /* INPUTs */
    InputsMap f_inputs;

    // ref to expr manager
    ExprMgr& f_em;

    // ref to type manager
    TypeMgr& f_tm;

    // symb resolver
    ModelResolver f_resolver;

    // ref to preprocessor (used for defines expr substitution)
    Preprocessor& f_preprocessor;

    // ref to type_checker (used for model analysis)
    TypeChecker& f_type_checker;

    ContextMap f_context_map;
    ParamMap f_param_map;

    /* internals */
    bool analyze_aux( analyzer_pass_t pass );
    bool f_analyzed;
};

#endif
