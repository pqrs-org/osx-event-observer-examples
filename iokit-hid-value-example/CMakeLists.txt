cmake_minimum_required (VERSION 3.9)
project (iokit-hid-value-example)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-O2)
add_compile_options(-fobjc-arc)
add_compile_options(-fmodules)
add_compile_options(-fcxx-modules)

include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/vendor/include)

add_executable(
  iokit-hid-value-example
  MACOSX_BUNDLE
  src/AppDelegate.mm
  src/AppDelegate.h
  src/IOKitHIDValueExample.mm
  src/IOKitHIDValueExample.h
  src/main.m
  Resources/MainMenu.xib
  Resources/Assets.xcassets
)

set_source_files_properties(
  Resources/MainMenu.xib
  Resources/Assets.xcassets
  PROPERTIES
  MACOSX_PACKAGE_LOCATION
  Resources
)

set_target_properties(
  iokit-hid-value-example
  PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST
  ${CMAKE_CURRENT_LIST_DIR}/Resources/Info.plist
)

set_target_properties(
  iokit-hid-value-example
  PROPERTIES
  XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
  "@loader_path @loader_path/../Frameworks"
)