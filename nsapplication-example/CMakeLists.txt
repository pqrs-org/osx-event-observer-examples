cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for Mac OS X")

project(nsapplication-example)
enable_language(Swift)

add_compile_options(-O2)

add_executable(
  nsapplication-example
  MACOSX_BUNDLE
  src/AppDelegate.swift
  src/ExampleApplication.swift
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
  nsapplication-example
  PROPERTIES

  MACOSX_BUNDLE_INFO_PLIST
  ${CMAKE_CURRENT_LIST_DIR}/Resources/Info.plist

  XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME
  YES

  XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
  "@loader_path @loader_path/../Frameworks"
)
