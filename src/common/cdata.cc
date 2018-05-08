/*
 * @file common/cdata.cc
 * @brief System wide definitions.
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

/* environment variables and paths */
const char *YASMV_MICROCODE_PATH ("YASMV_MICROCODE");

const char* TRACE_FMT_PLAIN ("plain");
const char* TRACE_FMT_JSON ("json");
const char* TRACE_FMT_XML  ("xml");
const char* TRACE_FMT_YASM ("yaml");

const char* TRACE_FMT_DEFAULT (TRACE_FMT_PLAIN);
