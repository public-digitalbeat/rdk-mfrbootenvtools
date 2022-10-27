# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the License);
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an AS IS BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# - Try to find TINYALSA
# Once done this will define
#  TINYALSA_FOUND - System has tinyalsa
#  TINYALSA_INCLUDE_DIRS - The tinyalsa include directories
#  TINYALSA_LIBRARIES - The libraries needed to use tinyalsa
#

find_package(PkgConfig)

find_library(TINYALSA_LIBRARIES NAMES tinyalsa)

set(TINYALSA_LIBRARIES ${TINYALSA_LIBRARIES} CACHE PATH "Path to tinyalsa library")

include(FindPackageHandleStandardArgs)

mark_as_advanced(TINYALSA_INCLUDE_DIRS TINYALSA_LIBRARIES)

