/**
 * @file opts.cc
 * @brief Command line options management
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/
#include "opts.hh"

// static initialization
OptsMgr_ptr OptsMgr::f_instance = NULL;

OptsMgr::OptsMgr()
    : f_desc("Program options")
    , f_pos()
    , f_vm()
    , f_help(false)
    , f_word_width(UINT_MAX)
{
    f_desc.add_options()

        (
         "help",
         "produce help message"
        )

        (
         "color",
         "enables colorized output in interactive shell"
        )

        (
         "word-width",
         options::value<unsigned>()->default_value(DEFAULT_WORD_WIDTH),
         "native word size in bits"
        )

        (
         "verbosity",
         options::value<unsigned>()->default_value(DEFAULT_VERBOSITY),
         "verbosity level"
        )

        (
         "model",
         options::value<string>(),
         "input model"
        )
        ;

    // arguments are models
    f_pos.add("model", -1);
}

void OptsMgr::parse_command_line(int argc, const char **argv)
{
    options::store(options::command_line_parser(argc, const_cast<char **>(argv)).
                options(f_desc).positional(f_pos).run(), f_vm);

    options::notify(f_vm);
    if (f_vm.count("help")) {
        f_help = true;
    }

    if (f_vm.count("color")) {
        f_color = true;
    }

    f_started = true;
}

unsigned OptsMgr::verbosity() const
{
    unsigned res = f_vm["verbosity"].as<unsigned>();
    return res;
}

bool OptsMgr::color() const
{
    return f_color;
}

void OptsMgr::set_word_width(unsigned value)
{
    TRACE
        << "Setting word width to "
        << value
        << endl;

    f_word_width = value;
}

unsigned OptsMgr::word_width() const
{
    return (UINT_MAX != f_word_width) ? f_word_width
        : f_vm["word-width"].as<unsigned>();
}

string OptsMgr::model() const
{
    string res = "";

    if (f_vm.count("model")) {
        res = f_vm["model"].as<string>();
    }

    return res;
}

bool OptsMgr::help() const
{
    return f_help;
}

string OptsMgr::usage() const
{
    ostringstream oss;
    oss << f_desc;
    return oss.str();
}


using namespace axter;
verbosity OptsMgr::get_verbosity_level_tolerance() {

    // FIX default
    if (!f_started) return log_often;

    switch (verbosity()) {
    case 0 : return log_always;
    case 1 : return log_often;
    case 2 : return log_regularly;
    case 3 : return log_rarely;
    default: return log_very_rarely;
    }
}
