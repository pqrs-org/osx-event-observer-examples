cmake_minimum_required(VERSION 3.24 FATAL_ERROR)
project(vendor NONE)

include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/vendor/CPM.cmake")

CPMUsePackageLock("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/package-lock.cmake")

set(VENDOR_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor")
file(MAKE_DIRECTORY "${VENDOR_INCLUDE_DIR}")

include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/common.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cpm-cmake-package-lock/cmake/dependency.cmake")

copy_vendor_package(pqrs_osx_iokit_hid_device_report_monitor)
copy_vendor_package(pqrs_osx_iokit_hid_manager)
copy_vendor_package(pqrs_osx_iokit_hid_manager)
copy_vendor_package(pqrs_osx_iokit_hid_queue_value_monitor)
copy_vendor_package(pqrs_osx_iokit_hid_value)
copy_vendor_package(pqrs_weakify)
