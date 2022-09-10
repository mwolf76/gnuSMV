/**
 * @file sat/cnf_nocut.cc
 * @brief Engine interface implementation, CNFization algorithm #1 (No cut)
 * implementation.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 **/

#include <dd/dd_walker.hh>
#include <sat/sat.hh>

namespace sat {

    class CNFBuilderNoCut: public dd::ADDWalker {
    public:
        CNFBuilderNoCut(Engine& sat, step_t time,
                        group_t group = MAINGROUP)
            : f_sat(sat)
            , f_time(time)
            , f_group(group)
        {}

        ~CNFBuilderNoCut()
        {}

        void pre_hook()
        {
            assert(1 == f_recursion_stack.size());

            dd::add_activation_record curr { f_recursion_stack.back() };
            f_toplevel = const_cast<DdNode*>(curr.node);
        }

        void post_hook()
        {
            /* build and push clause toplevel */
            vec<Lit> ps;
            ps.push(mkLit(f_group, true));

            assert(NULL != f_toplevel);
        }

        inline bool is_unseen(const DdNode* node) const
        {
            return f_seen.end() == f_seen.find(const_cast<DdNode*>(node));
        }

        inline void mark(const DdNode* node)
        {
            f_seen.insert(const_cast<DdNode*>(node));
        }

        bool condition(const DdNode* node)
        {
            return cuddIsConstant(node) && !cuddV(node);
        }

        void action(const DdNode* node)
        {
            DdManager* dd_mgr { f_sat.enc().dd().getManager() };

            value_t value { Cudd_V(node) };
            assert(!value);

            vec<Lit> ps;
            ps.clear();
            if (MAINGROUP != f_group) {
                ps.push(mkLit(f_group, true));
            }

            for (auto i { f_recursion_stack.rbegin() }; f_recursion_stack.rend() != i; ++i) {
                const auto& eye { *i };

                if (cuddIsConstant(eye.node)) {
                    continue;
                }

                Lit lit {
                    mkLit(f_sat.find_dd_var(eye.node, f_time),
                          eye.polarity == dd::DD_NEGATIVE)
                };
                ps.push(lit);
            }

            f_sat.add_clause(ps);

            std::cerr
                << "no-cut push: "
                << ps
                << std::endl;
        }

    private:
        Engine& f_sat;
        boost::unordered_set<DdNode*> f_seen;

        DdNode* f_toplevel;

        step_t f_time;
        group_t f_group;
    };

    void Engine::cnf_push_no_cut(ADD add, step_t time, const group_t group)
    {
        CNFBuilderNoCut worker { *this, time, group };
        worker(add);
    }

}; // namespace sat
