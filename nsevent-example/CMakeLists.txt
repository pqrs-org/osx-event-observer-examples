cmake_minimum_required (VERSION 3.9)
project(nsevent-example)
enable_language(Swift)

add_compile_options(-O2)

add_executable(
  nsevent-example
  MACOSX_BUNDLE
  src/AppDelegate.swift
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
  nsevent-example
  PROPERTIES
  MACOSX_BUNDLE_INFO_PLIST
  ${CMAKE_CURRENT_LIST_DIR}/Resources/Info.plist
)

set_target_properties(
  nsevent-example
  PROPERTIES
  XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
  "@loader_path @loader_path/../Frameworks"
)
