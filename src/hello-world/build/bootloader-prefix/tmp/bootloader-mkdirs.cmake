# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/chucholoport/.espressif/v6.0/esp-idf/components/bootloader/subproject"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/tmp"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/src/bootloader-stamp"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/src"
  "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/chucholoport/upsrj/rtos-sim/src/hello-world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
