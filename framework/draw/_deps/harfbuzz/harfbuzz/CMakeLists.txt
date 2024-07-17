# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

declare_module(harfbuzz)
set(MODULE_DIR ${CMAKE_CURRENT_LIST_DIR}/harfbuzz)

set(MODULE_SRC
    ${MODULE_DIR}/src/harfbuzz.cc
)

set(MODULE_INCLUDE
    ${FREETYPE_INCLUDE_DIRS}
)

set(MODULE_DEF
    -DHAVE_FREETYPE
)

set(MODULE_NOT_LINK_GLOBAL ON)
set(MODULE_PCH_DISABLED ON)
set(MODULE_UNITY_DISABLED ON)
setup_module()

# target_no_warning(${MODULE} -Wimplicit-fallthrough=0)
# target_no_warning(${MODULE} -Wno-conversion)
# target_no_warning(${MODULE} -Wno-cast-function-type)
