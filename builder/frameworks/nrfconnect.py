# Copyright 2021-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""
The nRF Connect SDK contains the Nordic-specific source code additions to open source
projects (Zephyr RTOS and MCUboot). It must be combined with nrfxlib and the repositories
that use the same naming convention to build the provided samples and to use the additional
subsystems and libraries.

https://github.com/nrfconnect/sdk-nrf
"""

from os.path import join

from SCons.Script import Import, SConscript

Import("env")

SConscript(
    join(env.PioPlatform().get_package_dir("framework-nrfconnect"), "scripts",
         "platformio", "platformio-build.py"), exports="env")
