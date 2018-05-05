# antlr.m4: Locate ANTLR3 build and runtime deps for autoconf-based projects.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Additional permission under section 7 of the GNU General Public
# License, version 3 ("GPLv3"):
#
# If you convey this file as part of a work that contains a
# configuration script generated by Autoconf, you may do so under
# terms of your choice.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
AC_DEFUN([AC_ANTLR3], [

    AC_ARG_WITH(
    [antlr-prefix],
    AC_HELP_STRING(
      [--with-antlr-prefix=PATH],
      [find the ANTLR3 headers and libraries in `PATH/include` and `PATH/lib`. By default, checks in /usr.]),
            antlr_prefix="$withval",
            antlr_prefix="/usr")

      AC_CHECK_PROG(ANTLR_JAVA, java, java, [])
      if test x$ANTLR_JAVA = "x"; then
         AC_MSG_ERROR([antlr3 requires java to run])
      fi

      AC_CHECK_PROG(ANTLR, antlr3, antlr, [])
      if test x$ANTLR = "x"; then
         AC_MSG_ERROR([antlr3 required to compile grammars])

      else
         save_CPPFLAGS=$CPPFLAGS
         save_LIBS=$LIBS

         AC_LANG_PUSH([C++])
         ANTLR_CPPFLAGS="-I$antlr_prefix/include"
         CPPFLAGS="$save_CFLAGS $ANTLR_CPPFLAGS"

         AC_CHECK_HEADER(antlr3.h,
         [],
         [AC_MSG_ERROR([ANTLR headers not found])])

         ANTLR_LIBS="-L$antlr_prefix/lib -lantlr3c"
         LIBS="$save_LIBS $ANTLR_LIBS"

         AC_MSG_CHECKING([whether linking with -lantlr3c in $antlr_prefix works])
         AC_LINK_IFELSE( [AC_LANG_PROGRAM(
                         [#include <antlr3.h>]
			 [pANTLR3_INPUT_STREAM input =
                         antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)
                         "Hello World", strlen("Hello World"), NULL);])],
                         [AC_MSG_RESULT([yes])], [AC_MSG_ERROR([no])])

         AC_LANG_POP([C++])
         CPPFLAGS=$save_CFLAGS
         LIBS=$save_LIBS
      fi

      AC_SUBST(ANTLR)
      AC_SUBST(ANTLR_CPPFLAGS)
      AC_SUBST(ANTLR_LIBS)
])
