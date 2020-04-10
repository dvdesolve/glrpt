#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details:
#
#  http://www.gnu.org/copyleft/gpl.txt
#

# global settings
cmake_minimum_required(VERSION 3.12)
project(glrpt VERSION 2.5.0 LANGUAGES C)


# use FHS
include(GNUInstallDirs)

# default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# prevent agressive and unsafe optimizations
# (by default CMake uses -O3 level)
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

# use specific modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


# build project
add_subdirectory(src)
add_subdirectory(share)
