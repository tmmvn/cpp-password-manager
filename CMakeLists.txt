#  Copyright (C) 2010 Felix Geyer <debfx@fobos.de>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 2 or (at your option)
#  version 3 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
		"Choose the type of build, options are: None Debug Release RelWithDebInfo Debug Debugfull Profile MinSizeRel."
		FORCE)
ENDIF()
CMAKE_MINIMUM_REQUIRED(VERSION 3.28.1)
PROJECT(KeePassX)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
INCLUDE(CheckCCompilerFlag)
INCLUDE(CheckCXXCompilerFlag)
INCLUDE(CheckCXXSourceCompiles)
OPTION(WITH_TESTS "Enable building of unit tests" ON)
OPTION(WITH_GUI_TESTS "Enable building of GUI tests" ON)
OPTION(WITH_DEV_BUILD "Use only for development. Disables/warns about deprecated methods." ON)
SET(KEEPASSX_VERSION "2.0.3")
SET(KEEPASSX_VERSION_NUM "2.0.3")
IF("${CMAKE_C_COMPILER}" MATCHES "clang$" OR "${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
	SET(CMAKE_COMPILER_IS_CLANG 1)
ENDIF()
IF("${CMAKE_CXX_COMPILER}" MATCHES "clang(\\+\\+)?$" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	SET(CMAKE_COMPILER_IS_CLANGXX 1)
ENDIF()
MACRO(ADD_GCC_COMPILER_CXXFLAGS FLAGS)
	IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS}")
	ENDIF()
ENDMACRO(ADD_GCC_COMPILER_CXXFLAGS)
MACRO(ADD_GCC_COMPILER_CFLAGS FLAGS)
	IF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_CLANG)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAGS}")
	ENDIF()
ENDMACRO(ADD_GCC_COMPILER_CFLAGS)
MACRO(ADD_GCC_COMPILER_FLAGS FLAGS)
	ADD_GCC_COMPILER_CXXFLAGS("${FLAGS}")
	ADD_GCC_COMPILER_CFLAGS("${FLAGS}")
ENDMACRO(ADD_GCC_COMPILER_FLAGS)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS -DQT_NO_EXCEPTIONS -DQT_STRICT_ITERATORS -DQT_NO_CAST_TO_ASCII)
ADD_GCC_COMPILER_FLAGS("-fno-common -fstack-protector --param=ssp-buffer-size=4")
ADD_GCC_COMPILER_FLAGS("-Wall -Wextra -Wundef -Wpointer-arith -Wno-long-long")
ADD_GCC_COMPILER_FLAGS("-Wformat=2 -Wmissing-format-attribute")
ADD_GCC_COMPILER_FLAGS("-fvisibility=hidden")
ADD_GCC_COMPILER_CXXFLAGS("-fvisibility-inlines-hidden")
ADD_GCC_COMPILER_CXXFLAGS("-fno-exceptions -fno-rtti")
ADD_GCC_COMPILER_CXXFLAGS("-Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual")
ADD_GCC_COMPILER_CFLAGS("-Wchar-subscripts -Wwrite-strings")
STRING(TOLOWER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_LOWER)
IF(CMAKE_BUILD_TYPE_LOWER MATCHES (release|relwithdebinfo|minsizerel))
	ADD_GCC_COMPILER_FLAGS("-D_FORTIFY_SOURCE=2")
ENDIF()
CHECK_C_COMPILER_FLAG("-Werror=format-security -Werror=implicit-function-declaration" WERROR_C_AVAILABLE)
CHECK_CXX_COMPILER_FLAG("-Werror=format-security" WERROR_CXX_AVAILABLE)
IF(WERROR_C_AVAILABLE AND WERROR_CXX_AVAILABLE)
	ADD_GCC_COMPILER_FLAGS("-Werror=format-security")
	ADD_GCC_COMPILER_CFLAGS("-Werror=implicit-function-declaration")
ENDIF()
IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align")
ENDIF()
IF(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align")
ENDIF()
IF(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-add-needed -Wl,--as-needed -Wl,--no-undefined")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-add-needed -Wl,--as-needed")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,-z,relro")
ENDIF()
ADD_GCC_COMPILER_CXXFLAGS("-std=c++11")
IF(APPLE)
	ADD_GCC_COMPILER_CXXFLAGS("-stdlib=libc++")
ENDIF()
ADD_GCC_COMPILER_CFLAGS("-ansi")
IF(WITH_DEV_BUILD)
	ADD_DEFINITIONS(-DQT_DEPRECATED_WARNINGS -DGCRYPT_NO_DEPRECATED)
ENDIF()
IF(MINGW)
	SET(CMAKE_RC_COMPILER_INIT windres)
	ENABLE_LANGUAGE(RC)
	SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
ENDIF()
IF(APPLE OR MINGW)
	SET(PROGNAME KeePassX)
ELSE()
	SET(PROGNAME keepassx)
ENDIF()
IF(APPLE AND "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
	SET(CMAKE_INSTALL_PREFIX "/Applications")
ENDIF()
IF(MINGW)
	SET(BIN_INSTALL_DIR ".")
	SET(PLUGIN_INSTALL_DIR ".")
	SET(DATA_INSTALL_DIR "share")
ELSEIF(APPLE)
	SET(BIN_INSTALL_DIR ".")
	SET(PLUGIN_INSTALL_DIR ".")
	SET(DATA_INSTALL_DIR "${PROGNAME}.app/Contents/Resources")
ELSE()
	INCLUDE(GNUInstallDirs)
	SET(BIN_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
	SET(PLUGIN_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/keepassx")
	SET(DATA_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/keepassx")
ENDIF()
IF(WITH_TESTS)
	INCLUDE(ctest)
	ENABLE_TESTING()
ENDIF(WITH_TESTS)
SET(CMAKE_PREFIX_PATH "~/Qt/6.7.2/macos/")
FIND_PACKAGE(Qt6 REQUIRED COMPONENTS Core Concurrent Widgets Test LinguistTools)
SET(CMAKE_AUTOMOC ON)
# Debian sets the the build type to None for package builds.
# Make sure we don't enable asserts there.
SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS $<$<CONFIG:None>:QT_NO_DEBUG>)
FIND_PACKAGE(Gcrypt 1.6.0 REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
CHECK_CXX_SOURCE_COMPILES("
  #include <zlib.h>

  #if !defined(ZLIB_VERNUM) || (ZLIB_VERNUM < 0x1200)
      #error zlib 1.2.x or higher is required to use the gzip format
  #endif

  int main() { return 0; }" ZLIB_SUPPORTS_GZIP)
IF(NOT ZLIB_SUPPORTS_GZIP)
	MESSAGE(FATAL_ERROR "zlib 1.2.x or higher is required to use the gzip format")
ENDIF()
IF(UNIX)
	CHECK_CXX_SOURCE_COMPILES("#include <sys/prctl.h>
    int main() { prctl(PR_SET_DUMPABLE, 0); return 0; }"
		HAVE_PR_SET_DUMPABLE)
	CHECK_CXX_SOURCE_COMPILES("#include <sys/resource.h>
    int main() {
      struct rlimit limit;
      limit.rlim_cur = 0;
      limit.rlim_max = 0;
      setrlimit(RLIMIT_CORE, &limit);
      return 0;
    }" HAVE_RLIMIT_CORE)
	IF(APPLE)
		CHECK_CXX_SOURCE_COMPILES("#include <sys/types.h>
      #include <sys/ptrace.h>
      int main() { ptrace(PT_DENY_ATTACH, 0, 0, 0); return 0; }"
			HAVE_PT_DENY_ATTACH)
	ENDIF()
ENDIF()
INCLUDE_DIRECTORIES(SYSTEM ${GCRYPT_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
INCLUDE(FeatureSummary)
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(share)
ADD_SUBDIRECTORY(utils)
IF(WITH_TESTS)
	ADD_SUBDIRECTORY(tests)
ENDIF(WITH_TESTS)
IF(PRINT_SUMMARY)
	FEATURE_SUMMARY(WHAT ALL)
ENDIF()
